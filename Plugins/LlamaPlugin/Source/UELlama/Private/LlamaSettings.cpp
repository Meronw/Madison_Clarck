// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#include "LlamaSettings.h"

ULlamaSettings::ULlamaSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Reset();
}

void ULlamaSettings::Reset()
{
	ContextSize = 4096;
	NThreadToUse = 4;
}