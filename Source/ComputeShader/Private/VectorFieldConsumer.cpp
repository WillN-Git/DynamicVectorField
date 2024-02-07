#include "VectorFieldConsumer.h"
#include "Kismet/GameplayStatics.h"
#include "ComputeShaderDeclarations/Public/VectorFieldDeclarations.h"

AVectorFieldConsumer::AVectorFieldConsumer()
{
	PrimaryActorTick.bCanEverTick = true;

	RootCmp = CreateDefaultSubobject<USceneComponent>(TEXT("RootCmp"));
	RootComponent = RootCmp;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootCmp);
}

void AVectorFieldConsumer::BeginPlay()
{
	Super::BeginPlay();
	FVectorFieldManager::Get()->BeginRendering();

	UMaterialInstanceDynamic* mid = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	mid->SetTextureParameterValue("InputTexture", (UTexture*)RenderTarget);
}

void AVectorFieldConsumer::BeginDestroy()
{
	Super::BeginDestroy();
	FVectorFieldManager::Get()->EndRendering();
}

void AVectorFieldConsumer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVectorFieldShaderParams params(RenderTarget);
	params.Color = Color;
	params.Constant = Constant;
	FVectorFieldManager::Get()->UpdateParameters(params);
}

void AVectorFieldConsumer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}