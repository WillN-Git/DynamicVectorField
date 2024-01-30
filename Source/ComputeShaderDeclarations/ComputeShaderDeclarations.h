#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"



class COMPUTESHADERDECLARATIONS_API FComputeShaderDeclaractionsModule : public IModuleInterface
{
public:
	static inline FComputeShaderDeclaractionsModule& Get()
	{
		return FModuleManager::LoadModuleChecked< FComputeShaderDeclaractionsModule>("ComputeShaderDeclarations");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ComputeShaderDeclarations");
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
