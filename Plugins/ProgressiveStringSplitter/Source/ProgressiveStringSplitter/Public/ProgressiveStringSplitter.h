// Copyright 2023, Cheese Labs, All rights reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FProgressiveStringSplitterModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
