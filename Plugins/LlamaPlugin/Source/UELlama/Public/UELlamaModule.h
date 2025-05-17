// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#pragma once
#include "Modules/ModuleManager.h"

class UELlamaModule : public IModuleInterface
{
public:

	UELlamaModule();

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:
	/**
	* Load static library
	*/
	void* LoadLibrary(const FString& name);

private:

	/** Llama DLL + LIB */
	void* LlamaLibrary;

	/** Whether the module has been initialized. */
	bool Initialized;
};
