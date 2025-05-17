// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#include "LlamaContextHandler.h"

#include "LlamaSettings.h"

TArray<ULlamaContext*> ULlamaContextHandler::Contexts = TArray<ULlamaContext*>();

ULlamaContext* ULlamaContextHandler::NewContextFromModel(ULlamaModel* Model)
{
	auto LlamaDefaultParams = llama_context_default_params();
	LlamaDefaultParams.n_ctx  = abs(SETTINGS->ContextSize);

	if (Model != nullptr && Model->GetInstance() != nullptr)
	{
		llama_context *loadedCtx = llama_new_context_with_model(ULlamaModel::GetInstance()->GetLlamaModel(), LlamaDefaultParams);

		if (loadedCtx == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Error while trying to create a new context !"));
			return nullptr;
		} 
		
		ULlamaContext* NewContext = NewObject<ULlamaContext>();
		NewContext->SetLlamaContext(loadedCtx);
		Contexts.Add(NewContext);

		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] A new context was made with size %d !"), llama_n_ctx(loadedCtx));
		return NewContext;
		
	}

	UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Error while trying to create a new context: missing model !"));
	return nullptr;
}

void ULlamaContextHandler::FreeContext(ULlamaContext* Context)
{
	if (Context != nullptr && Context->GetLlamaContext() != nullptr && !Context->isUnloaded)
	{
		Context->stop = true;
		Context->isUnloaded = true;
		FRWScopeLock ContextLock = FRWScopeLock(Context->GetLock(), SLT_Write);
		llama_free(Context->GetLlamaContext());
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] A Context was unloaded !"));
	}
}

void ULlamaContextHandler::SetPrefix(ULlamaContext* Context, FString PromptPrefix)
{
	if (Context)
	{
		Context->GetPrefix() = PromptPrefix;
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Impossible to set a prefix: valid context missing !"));
	}
	
}

void ULlamaContextHandler::SetSuffix(ULlamaContext* Context, FString PromptSuffix)
{
	if (Context)
	{
		Context->GetSuffix() = PromptSuffix;
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Impossible to set a suffix: valid context missing !"));
	}
}


