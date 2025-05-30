﻿// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#pragma once

#include "llama.h"
#include "LlamaModel.h"

#include "LlamaContext.generated.h"

UCLASS(BlueprintType)
class ULlamaContext : public UObject
{
	GENERATED_BODY()
	
public:
	ULlamaContext(llama_context *Ctx) : LlamaContext(Ctx) {}
	ULlamaContext() : LlamaContext(nullptr) {}

	virtual ~ULlamaContext() override;
	
	llama_context* GetLlamaContext() const
	{
		return LlamaContext;
	}
	
	void SetLlamaContext(llama_context *Context)
	{
		this->LlamaContext = Context;
	}

	TArray<llama_token> &GetEmbeds()
	{
		return Embeds;
	}
	
	TArray<int> &GetIOSizes()
	{
		return IOSizes;
	}

	FString GetPrefix()
	{
		return Prefix;
	}

	FString GetSuffix()
	{
		return Suffix;
	}

	FRWLock& GetLock()
	{
		return WriteLock;
	}

	//To stop current generation if needed
	bool stop = false;

	//Check if llama memory has already been destroyed
	bool isUnloaded = false;
	
	
private:
	llama_context *LlamaContext;
	
	/** List of embeds: every token, words, information treated by Llama */
	TArray<llama_token> Embeds = {};
	
	/** A list of the size of the blocks of information added in the context (tokens from user prompts or generated by Llama) */
	TArray<int> IOSizes = {};

	FString Prefix = FString();
	FString Suffix = FString();

	FRWLock WriteLock;
};


inline ULlamaContext::~ULlamaContext()
{
	stop = true;
	FRWScopeLock ContextLock = FRWScopeLock(WriteLock, SLT_Write);
	if (this && !isUnloaded)
	{
		isUnloaded = true;
		llama_free(LlamaContext);
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] A Context was unloaded !"));
	}
}
