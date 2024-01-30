#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"

//DEFINE_LOG_CATEGORY_EXTERN(LogPVTTest, Log, All);


struct FPVTTestParams
{
private:
	FIntPoint CachedRenderTargetSize;

public:
	UTextureRenderTarget2D* RenderTarget;
	uint32 Period;
	FLinearColor Color;

public:
	FPVTTestParams() = default;
	FPVTTestParams(UTextureRenderTarget2D* IORenderTarget)
		: RenderTarget(IORenderTarget)
	{
		CachedRenderTargetSize = RenderTarget ? FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY) : FIntPoint::ZeroValue;
	}

	FIntPoint GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}
};


class COMPUTESHADERDECLARATIONS_API FPVTTestManager
{
private:
	static FPVTTestManager* Instance;

	FDelegateHandle OnPostResolvedSceneColorHandle;

	FPVTTestParams CachedParams;

	volatile bool IsCachedParamsValid;

	TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;

public:
	static FPVTTestManager* Get()
	{
		if (!Instance)
			Instance = new FPVTTestManager();

		return Instance;
	};

	void BeginRendering();

	void EndRendering();

	void UpdateParameters(FPVTTestParams& DrawParameters);

	FPVTTestManager() = default;

	void Execute_RenderThread(FRDGBuilder& GraphBuilder, const FSceneTextures& SceneTextures);
};