// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#include "UELlamaModule.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "Developer/Settings/Public/ISettingsSection.h"
#include "LlamaSettings.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FLlamaModule"
const ULlamaSettings* ULlamaSettings::Settings;

UELlamaModule::UELlamaModule() : Initialized()
{
	LlamaLibrary = nullptr;
}

void UELlamaModule::StartupModule()
{	
	LlamaLibrary = LoadLibrary("llama");
	Initialized = false;


	//Create configuration window
	ISettingsModule* const SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule != nullptr)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "Llama Plugin",
			LOCTEXT("LlamaSettingsName", "Llama Plugin"),
			LOCTEXT("LlamaSettingsDescription", "Configure the Llama plug-in."),
			GetMutableDefault<ULlamaSettings>()
		);

		auto ResetLambda = []() { GetMutableDefault<ULlamaSettings>()->Reset(); return true; };
		SettingsSection->OnResetDefaults().BindLambda(ResetLambda);
	}

	ULlamaSettings::Settings = GetDefault<ULlamaSettings>();
}

void UELlamaModule::ShutdownModule()
{
	//todo: free memory

	if (LlamaLibrary)
		FPlatformProcess::FreeDllHandle(LlamaLibrary);
}

void* UELlamaModule::LoadLibrary(const  FString& name) {

	FString BaseDir = IPluginManager::Get().FindPlugin("UELlama")->GetBaseDir();

	FString LibDir;
	FString prefix;
	FString extension;
	extension = TEXT(".dll");
	prefix = "";

	LibDir = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/llama/bin/vs/x64"));
	if (!LibDir.IsEmpty()) {
		FString LibraryPath = FPaths::Combine(*LibDir, prefix + name + extension);
		return FPlatformProcess::GetDllHandle(*LibraryPath);
	}
	return nullptr;
}

IMPLEMENT_MODULE(UELlamaModule, UELlama)