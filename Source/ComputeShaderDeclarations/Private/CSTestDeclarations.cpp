#include "CSTestDeclarations.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "Modules/ModuleManager.h"
#include "DataDrivenShaderPlatformInfo.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 32

DEFINE_LOG_CATEGORY(LogCSTest);

///////////////////////////////////////////////////
// 
// Internal class that holds the parameters
// and connects the HLSL shader to the engine
// 
///////////////////////////////////////////////////
class FCSTest : public FGlobalShader
{
public:
	// Declare this class as a global shader
	DECLARE_GLOBAL_SHADER(FCSTest);

	// Tells the engine that this shader uses a structure for its parameters
	SHADER_USE_PARAMETER_STRUCT(FCSTest, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWTexture2D<float>, OutputTexture)
		SHADER_PARAMETER(FVector2f, Dimensions)
		SHADER_PARAMETER(UINT, Timestamp)
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static inline void ModifyCompilationEnvironmnent(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}
};

// This will tell the engine to create the shader and where the shader entry point is.
//					ShaderType			ShaderPath			ShaderMain  ShaderType
IMPLEMENT_GLOBAL_SHADER(FCSTest, "/CustomShaders/CSTest.usf", "MainCS", SF_Compute);


//Static members
FCSTestManager* FCSTestManager::Instance = nullptr;

//Begin the execution of the compute shader each frame
void FCSTestManager::BeginRendering()
{
	if (OnPostResolvedSceneColorHandle.IsValid())
		return;

	IRendererModule* rendererModule = FModuleManager::GetModulePtr<IRendererModule>(FName("Renderer"));

	if (rendererModule)
	{
		OnPostResolvedSceneColorHandle = rendererModule->GetResolvedSceneColorCallbacks()
														// Look at the function definition by ctrl+click on the GetResolvedSceneColorCallbacks
														// Then ctrl+click again on the delegate return type FOnResolvedSceneColor
														.AddRaw(this, &FCSTestManager::Execute_RenderThread);
	}
}

//Stop CS execution
void FCSTestManager::EndRendering()
{
	if (!OnPostResolvedSceneColorHandle.IsValid())
		return;

	IRendererModule* rendererModule = FModuleManager::GetModulePtr<IRendererModule>(FName("Renderer"));
	UE_LOG(LogCSTest, Log, TEXT("====================Try To remove renderer module."));
	if (rendererModule)
	{
		UE_LOG(LogCSTest, Log, TEXT("Removed from the module."));
		rendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}

	OnPostResolvedSceneColorHandle.Reset();
}

void FCSTestManager::UpdateParameters(FCSTestParams& Parameters)
{
	CachedParams = Parameters;
	IsCachedParamsAreValid = true;
	UE_LOG(LogCSTest, Log, TEXT("Update."));
}

void FCSTestManager::Execute_RenderThread(FRDGBuilder& GraphBuilder, const FSceneTextures& SceneTextures)
{
	UE_LOG(LogCSTest, Log, TEXT("1) Execute_RenderThread"));

	// If there's no cached parameters to use, skip
	// If no render target is supplied in the CachedParams, skip
	if (!(IsCachedParamsAreValid && CachedParams.RenderTarget))
	{
		UE_LOG(LogCSTest, Log, TEXT("IsCachedParamsAreValid ? %d"), IsCachedParamsAreValid);
		UE_LOG(LogCSTest, Log, TEXT("CachedParams.RenderTarget ? %d"), !!CachedParams.RenderTarget);
		return;
	}

	UE_LOG(LogCSTest, Log, TEXT("2) Execute_RenderThread"));
	// Render thread assertion
	check(IsInRenderingThread());
	UE_LOG(LogCSTest, Log, TEXT("3) Execute_RenderThread"));
	FRHICommandListImmediate& builderRHICmdList = GraphBuilder.RHICmdList;

	if (!ComputeShaderOutput.IsValid())
	{
		UE_LOG(LogCSTest, Error, TEXT("Not Valid."));
		FPooledRenderTargetDesc computeShaderOutputDesc(FPooledRenderTargetDesc::Create2DDesc(
			CachedParams.GetRenderTargetSize(),
			CachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI->GetFormat(),
			FClearValueBinding::None,
			TexCreate_None,
			TexCreate_ShaderResource | TexCreate_UAV,
			false
		));

		computeShaderOutputDesc.DebugName = TEXT("CSTest_Output_RenderTarget");
		GRenderTargetPool.FindFreeElement(builderRHICmdList, computeShaderOutputDesc, ComputeShaderOutput, TEXT("CSTest_Output_RenderTarget"));
	}

	UE_LOG(LogCSTest, Log, TEXT("End...."));

	FRHITexture* srcTexture = ComputeShaderOutput->GetRHI();
	FRHITexture* destTexture = CachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI;

	builderRHICmdList.Transition(FRHITransitionInfo(srcTexture, ERHIAccess::SRVMask));

	// Fill the shader parameters struct
	FCSTest::FParameters passParameters;
	passParameters.OutputTexture = builderRHICmdList.CreateUnorderedAccessView(srcTexture);
	passParameters.Dimensions = FVector2f(CachedParams.GetRenderTargetSize().X, CachedParams.GetRenderTargetSize().Y);
	passParameters.Timestamp = CachedParams.Timestamp;

	// Get a reference to our shader type from global shader map
	TShaderMapRef<FCSTest> cs(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	FComputeShaderUtils::Dispatch(builderRHICmdList, cs, passParameters,
		FIntVector(
			FMath::DivideAndRoundUp(CachedParams.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(CachedParams.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION),
			1
		)
	);

	builderRHICmdList.CopyTexture(srcTexture, destTexture, FRHICopyTextureInfo());

	GRenderTargetPool.FreeUnusedResource(ComputeShaderOutput);
}
