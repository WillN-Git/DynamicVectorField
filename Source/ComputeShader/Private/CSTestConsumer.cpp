#include "CSTestConsumer.h"

#include "Kismet/GameplayStatics.h"
#include "ComputeShaderDeclarations/Public/CSTestDeclarations.h"

ACSTestConsumer::ACSTestConsumer()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));

	TimeStamp = 0;
}

void ACSTestConsumer::BeginPlay()
{
	Super::BeginPlay();
	FCSTestManager::Get()->BeginRendering();


	//Assuming that the static mesh is already using the material that we're targeting, we create an instance and assign it to it
	UMaterialInstanceDynamic* MID = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	MID->SetTextureParameterValue("InputTexture", (UTexture*)RenderTarget);
}

void ACSTestConsumer::BeginDestroy()
{
	Super::BeginDestroy();
	FCSTestManager::Get()->EndRendering();
}

void ACSTestConsumer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Update parameters
	FCSTestParams parameters(RenderTarget);
	TimeStamp++;
	parameters.Timestamp = TimeStamp;
	FCSTestManager::Get()->UpdateParameters(parameters);
}

void ACSTestConsumer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
