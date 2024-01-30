#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CSTestConsumer.generated.h"

UCLASS()
class COMPUTESHADER_API ACSTestConsumer : public APawn
{
	GENERATED_BODY()

private:
	uint32 TimeStamp;

public:
	UPROPERTY()
	USceneComponent* Root;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
	class UTextureRenderTarget2D* RenderTarget;

public:
	// Sets default values for this pawn's properties
	ACSTestConsumer();

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
