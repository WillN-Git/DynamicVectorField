#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCSTest, Log, All);

struct FCSTestParams
{
private:
	FIntPoint CachedRenderTargetSize;

public:
	UTextureRenderTarget2D* RenderTarget;
	uint32 Timestamp;

public:
	FCSTestParams() = default;
	FCSTestParams(UTextureRenderTarget2D* IORenderTarget)
		: RenderTarget(IORenderTarget)
	{
		CachedRenderTargetSize = RenderTarget ? FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY) : FIntPoint::ZeroValue;
	}


	FIntPoint GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}
};


class COMPUTESHADERDECLARATIONS_API FCSTestManager
{

private:
	//The singleton instance
	static FCSTestManager* Instance;

	//The delegate handle to our function that will be executed each frame by the renderer
	FDelegateHandle OnPostResolvedSceneColorHandle;

	//Cached Shader Manager Parameters
	FCSTestParams CachedParams;

	//Whether we have cached parameters to pass to the shader or not
	volatile bool IsCachedParamsAreValid;

	//Reference to a pooled render target where the shader will write its output
	TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;

public:
	//Get the instance
	static FCSTestManager* Get()
	{
		if (!Instance)
			Instance = new FCSTestManager();
		return Instance;
	};

	// Call this when you want to hook onto the renderer and start executing the compute shader. The shader will be dispatched once per frame.
	void BeginRendering();

	// Stops compute shader execution
	void EndRendering();

	// Call this whenever you have new parameters to share.
	void UpdateParameters(FCSTestParams& DrawParameters);
	//Private constructor to prevent client from instanciating
	FCSTestManager() = default;

	void Execute_RenderThread(FRDGBuilder& builder, const FSceneTextures& SceneTextures);
};