#pragma once


#include "CoreMinimal.h"
#include "Engine/TextureRenderTargetVolume.h"


struct FVectorFieldShaderParams
{
private:
	FIntVector CachedRenderTargetSize;

public:
	UTextureRenderTargetVolume* RenderTarget;
	FLinearColor Color;
	float Constant;
	
public:
	FVectorFieldShaderParams() = default;
	FVectorFieldShaderParams(UTextureRenderTargetVolume* InRenderTarget)
		: RenderTarget(InRenderTarget)
	{
		CachedRenderTargetSize = RenderTarget ? FIntVector(RenderTarget->SizeX, RenderTarget->SizeY, RenderTarget->SizeZ) : FIntVector::ZeroValue;
	};

	FIntVector GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	};
};



class COMPUTESHADERDECLARATIONS_API FVectorFieldManager
{
private:
	static FVectorFieldManager* Instance;

	FDelegateHandle OnPostResolvedSceneColorHandle;

	FVectorFieldShaderParams CachedParams;

	volatile bool IsCachedParamsValid;

	TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;

public:
	static FVectorFieldManager* Get()
	{
		if (!Instance)
			Instance = new FVectorFieldManager();

		return Instance;
	};

	void BeginRendering();

	void EndRendering();

	void UpdateParameters(FVectorFieldShaderParams& InParams);

	FVectorFieldManager() = default;

	void Execute_RenderThread(FRDGBuilder& GraphBuilder, const FSceneTextures& SceneTextures);
};