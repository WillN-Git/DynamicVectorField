#include "VectorFieldDeclarations.h"
#include "GlobalShader.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "Modules/ModuleManager.h"
#include "ShaderParameterStruct.h"
#include "DataDrivenShaderPlatformInfo.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 8


class FVectorFieldShader : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FVectorFieldShader);

	SHADER_USE_PARAMETER_STRUCT(FVectorFieldShader, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER_UAV(RWTexture3D<FVector4f>, OutputTexture)
		SHADER_PARAMETER(FVector3f, InDimensions)
		SHADER_PARAMETER(FVector4f, InColor)
		SHADER_PARAMETER(float, C)
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	};

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), NUM_THREADS_PER_GROUP_DIMENSION);
	};
};

IMPLEMENT_GLOBAL_SHADER(FVectorFieldShader, "/CustomShaders/VFShaderTest.usf", "MainCS", SF_Compute);


FVectorFieldManager* FVectorFieldManager::Instance = nullptr;

void FVectorFieldManager::BeginRendering()
{
	if (OnPostResolvedSceneColorHandle.IsValid())
		return;

	IRendererModule* rendererModule = FModuleManager::GetModulePtr<IRendererModule>(FName("Renderer"));

	if (rendererModule)
	{
		OnPostResolvedSceneColorHandle = rendererModule->GetResolvedSceneColorCallbacks()
														.AddRaw(this, &FVectorFieldManager::Execute_RenderThread);
	}
}

void FVectorFieldManager::EndRendering()
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

void FVectorFieldManager::UpdateParameters(FVectorFieldShaderParams& InParams)
{
	CachedParams = InParams;
	IsCachedParamsValid = true;
}

void FVectorFieldManager::Execute_RenderThread(FRDGBuilder& GraphBuilder, const FSceneTextures& SceneTextures)
{
	if (!(IsCachedParamsValid && CachedParams.RenderTarget))
		return;

	check(IsInRenderingThread());

	FRHICommandListImmediate& builderRHICmdList = GraphBuilder.RHICmdList;

	if (!ComputeShaderOutput.IsValid())
	{
		FPooledRenderTargetDesc shaderOutputDesc(FPooledRenderTargetDesc::CreateVolumeDesc(
			CachedParams.GetRenderTargetSize().X,
			CachedParams.GetRenderTargetSize().Y,
			CachedParams.GetRenderTargetSize().Z,
			CachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI->GetDesc().Format,
			FClearValueBinding::None,
			TexCreate_None,
			TexCreate_ShaderResource | TexCreate_UAV,
			false
		));

		shaderOutputDesc.DebugName = TEXT("VectorField_Output_RenderTarget");
		GRenderTargetPool.FindFreeElement(builderRHICmdList, shaderOutputDesc, ComputeShaderOutput, TEXT("VectorField_Output_RenderTarget"));
	}

	FRHITexture* srcTexture = ComputeShaderOutput->GetRHI();
	FRHITexture* destTexture = CachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI;

	builderRHICmdList.Transition(FRHITransitionInfo(srcTexture, ERHIAccess::SRVMask));

	FVectorFieldShader::FParameters passParameters;
	passParameters.OutputTexture = builderRHICmdList.CreateUnorderedAccessView(srcTexture);
	passParameters.InDimensions = FVector3f(CachedParams.GetRenderTargetSize());
	passParameters.InColor = CachedParams.Color;
	passParameters.C = CachedParams.Constant;

	TShaderMapRef<FVectorFieldShader> vfShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	FComputeShaderUtils::Dispatch(builderRHICmdList, vfShader, passParameters,
		FIntVector(
			FMath::DivideAndRoundUp(CachedParams.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(CachedParams.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(CachedParams.GetRenderTargetSize().Z, NUM_THREADS_PER_GROUP_DIMENSION)
		)
	);

	builderRHICmdList.CopyTexture(srcTexture, destTexture, FRHICopyTextureInfo());

	GRenderTargetPool.FreeUnusedResource(ComputeShaderOutput);
}