// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#include "LlamaModel.h"

#include "LlamaContextHandler.h"
#include "LlamaSettings.h"

ULlamaModel *ULlamaModel::Instance = nullptr;
FRWLock ULlamaModel::WriteLock;

ULlamaModel::~ULlamaModel()
{
	if (!isUnloaded)
		FreeModel();
}

ULlamaModel *ULlamaModel::LoadModel(const FString& ModelPath)
{
	isUnloaded = false;
	const char *Path = TCHAR_TO_UTF8(*ModelPath);
	auto LlamaDefaultParams = llama_context_default_params();
	LlamaDefaultParams.n_ctx = abs(SETTINGS->ContextSize);

	llama_model *LoadedModel = llama_load_model_from_file(Path, LlamaDefaultParams);

	ULlamaModel *LlamaModel = NewObject<ULlamaModel>();

	if (LoadedModel)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] The Model was loaded !"));
	}
		
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Impossible to load a new model !"));
		isUnloaded = true;
	}
	
	LlamaModel->LlamaModel = LoadedModel;
	
	return LlamaModel;
}

void ULlamaModel::FreeModel()
{
	
	if (Instance != nullptr && Instance->LlamaModel != nullptr && !isUnloaded)
	{
		isUnloaded = true;
		// A contest cannot exist without a model. Free all contexts before model
		 for (auto* Context : ULlamaContextHandler::Contexts)
		 {
		 	
		 	if (Context && !Context->isUnloaded)
		 	{
		 		Context->stop = true;
		 		ULlamaContextHandler::FreeContext(Context);
		 	}
		 	
		 }

		ULlamaContextHandler::Contexts = {};
		
		FRWScopeLock Lock(WriteLock, SLT_Write);
		llama_free_model(Instance->LlamaModel);
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] The Model was unloaded !"));
		
	}
	else if (!isUnloaded)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Couldn't unload model, model has not been initialized !"));
	}
}