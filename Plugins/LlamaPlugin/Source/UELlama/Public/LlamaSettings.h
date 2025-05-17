// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#pragma once

#include "LlamaSettings.generated.h"

#define SETTINGS ULlamaSettings::Settings

UCLASS(config = Engine, defaultconfig)
class ULlamaSettings : public UObject
{
	
	GENERATED_BODY()
	
public:

	/** The max size of a context used in the program */
	UPROPERTY(config, EditAnywhere, Category = ContextConfiguration)
	int ContextSize;

	/** The number of CPU threads to use when Llama evaluates tokens */
	UPROPERTY(config, EditAnywhere, Category = ContextConfiguration)
	int NThreadToUse;

	void Reset();

	/** General settings of the plugin retrieved from configuration window */
	static ULlamaSettings const* Settings;

	ULlamaSettings(const FObjectInitializer& ObjectInitializer);
	
};
