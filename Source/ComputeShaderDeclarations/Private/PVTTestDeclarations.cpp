#include "PVTTestDeclarations.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "Modules/ModuleManager.h"
#include "DataDrivenShaderPlatformInfo.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 8

//DEFINE_LOG_CATEGORY(LogPVTTest);

class FPVTTest : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FPVTTest);

	SHADER_USE_PARAMETER_STRUCT(FPVTTest, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER_UAV(RWTexture2D<FVector4f>, OutputTexture)
		SHADER_PARAMETER(FVector2f, Dimensions)
		SHADER_PARAMETER(UINT, Period)
		SHADER_PARAMETER(FVector4f, Color)
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}
};

IMPLEMENT_GLOBAL_SHADER(FPVTTest, "/CustomShaders/PVTTest.usf", "MainCS", SF_Compute);


FPVTTestManager* FPVTTestManager::Instance = nullptr;

void FPVTTestManager::BeginRendering()
{
	if (OnPostResolvedSceneColorHandle.IsValid())
		return;

	IRendererModule* rendererModule = FModuleManager::GetModulePtr<IRendererModule>(FName("Renderer"));

	if (rendererModule)
	{
		OnPostResolvedSceneColorHandle = rendererModule->GetResolvedSceneColorCallbacks()
														.AddRaw(this, &FPVTTestManager::Execute_RenderThread);
	}
}

void FPVTTestManager::EndRendering()
{
	if (OnPostResolvedSceneColorHandle.IsValid())
		return;

	IRendererModule* rendererModule = FModuleManager::GetModulePtr<IRendererModule>(FName("Renderer"));

	if (rendererModule)
	{
		rendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}

	OnPostResolvedSceneColorHandle.Reset();
}

void FPVTTestManager::UpdateParameters(FPVTTestParams& DrawParameters)
{
	CachedParams = DrawParameters;
	IsCachedParamsValid = true;
}

void FPVTTestManager::Execute_RenderThread(FRDGBuilder& GraphBuilder, const FSceneTextures& SceneTextures)
{
	if (!(IsCachedParamsValid && CachedParams.RenderTarget))
		return;

	check(IsInRenderingThread());

	FRHICommandListImmediate& builderRHICmdList = GraphBuilder.RHICmdList;

	if (!ComputeShaderOutput.IsValid())
	{
		FPooledRenderTargetDesc computeShaderOutputDesc(FPooledRenderTargetDesc::Create2DDesc(
			CachedParams.GetRenderTargetSize(),
			CachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI->GetFormat(),
			FClearValueBinding::None,
			TexCreate_None,
			TexCreate_ShaderResource | TexCreate_UAV,
			false
		));

		computeShaderOutputDesc.DebugName = TEXT("PVTTest_Output_RenderTarget");
		GRenderTargetPool.FindFreeElement(builderRHICmdList, computeShaderOutputDesc, ComputeShaderOutput, TEXT("PVTTest_Output_RenderTarget"));
	}

	FRHITexture* srcTexture = ComputeShaderOutput->GetRHI();
	FRHITexture* destTexture = CachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI;

	builderRHICmdList.Transition(FRHITransitionInfo(srcTexture, ERHIAccess::SRVMask));

	FPVTTest::FParameters passParameters;
	passParameters.OutputTexture = builderRHICmdList.CreateUnorderedAccessView(srcTexture);
	passParameters.Dimensions = FVector2f(CachedParams.GetRenderTargetSize().X, CachedParams.GetRenderTargetSize().Y);
	passParameters.Period = CachedParams.Period;
	passParameters.Color = CachedParams.Color;

	TShaderMapRef<FPVTTest> pvt(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	FComputeShaderUtils::Dispatch(builderRHICmdList, pvt, passParameters,
		FIntVector(
			FMath::DivideAndRoundUp(CachedParams.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(CachedParams.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION),
			1
		)
	);

	builderRHICmdList.CopyTexture(srcTexture, destTexture, FRHICopyTextureInfo());

	GRenderTargetPool.FreeUnusedResource(ComputeShaderOutput);
}