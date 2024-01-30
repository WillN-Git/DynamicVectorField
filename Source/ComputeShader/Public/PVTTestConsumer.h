#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PVTTestConsumer.generated.h"


UCLASS()
class COMPUTESHADER_API APVTTestConsumer : public APawn
{
	GENERATED_BODY()

private:

public:
	UPROPERTY()
	USceneComponent* Root;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PVTTestConsumer")
	class UTextureRenderTarget2D* RenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PVTTestConsumer")
	int32 Period;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PVTTestConsumer")
	FLinearColor Color;

public:
	// Sets default values for this pawn's properties
	APVTTestConsumer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};