#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VectorFieldConsumer.generated.h"


UCLASS()
class COMPUTESHADER_API AVectorFieldConsumer : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY()
	USceneComponent* RootCmp;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VectorFieldConsumer")
	class UTextureRenderTargetVolume* RenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VectorFieldConsumer")
	FLinearColor Color;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VectorFieldConsumer")
	float Constant;

public:
	AVectorFieldConsumer();

protected:
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};