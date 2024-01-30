#include "ComputeShaderDeclarations.h"
#include "Misc/Paths.h"
#include "GlobalShader.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_GAME_MODULE(FComputeShaderDeclaractionsModule, ComputeShaderDeclarations);

void FComputeShaderDeclaractionsModule::StartupModule()
{
	FString shaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders/Private"));
	AddShaderSourceDirectoryMapping("/CustomShaders", shaderDirectory);
}

void FComputeShaderDeclaractionsModule::ShutdownModule()
{
}
