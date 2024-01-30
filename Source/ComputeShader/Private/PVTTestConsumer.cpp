#include "PVTTestConsumer.h"
#include "Kismet/GameplayStatics.h"
#include "ComputeShaderDeclarations/Public/PVTTestDeclarations.h"

APVTTestConsumer::APVTTestConsumer()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));

	Period = 0;
}

void APVTTestConsumer::BeginPlay()
{
	Super::BeginPlay();
	FPVTTestManager::Get()->BeginRendering();


	//Assuming that the static mesh is already using the material that we're targeting, we create an instance and assign it to it
	UMaterialInstanceDynamic* MID = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	MID->SetTextureParameterValue("InputTexture", (UTexture*)RenderTarget);
}

void APVTTestConsumer::BeginDestroy()
{
	Super::BeginDestroy();
	FPVTTestManager::Get()->EndRendering();
}

void APVTTestConsumer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UE_LOG(LogTemp, Log, TEXT("%s"), *Color.ToString());

	//Update parameters
	FPVTTestParams parameters(RenderTarget);
	parameters.Period = Period;
	parameters.Color = Color;
	FPVTTestManager::Get()->UpdateParameters(parameters);
}

void APVTTestConsumer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
