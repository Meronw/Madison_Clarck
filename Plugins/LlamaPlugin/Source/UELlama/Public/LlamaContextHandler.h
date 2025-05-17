// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#pragma once
#include "LlamaContext.h"
#include "LlamaModel.h"

#include "LlamaContextHandler.generated.h"

UCLASS()
class ULlamaContextHandler : public UObject
{

	GENERATED_BODY()
	
public:

	/**
	 * Creates a new context by using an existing model.
	 * @param Model - The model to use 
	 * @return The newly created model
	 */
	UFUNCTION(BlueprintCallable, Category="LlamaIntegration")
	static ULlamaContext* NewContextFromModel(ULlamaModel* Model);

	/**
	 * Unloads an existing context from memory.
	 * @param Context - The context to unload
	 */
	UFUNCTION(BlueprintCallable, Category="LlamaIntegration")
	static void FreeContext(ULlamaContext* Context);

	/**
	 * Adds a prefix to the user's prompt: a text that will be inserted before the prompt every request on the same context.
	 * @param Context - The context to use
	 * @param PromptPrefix - The prefix to add
	 */
	UFUNCTION(BlueprintCallable, Category="LlamaIntegration")
	static void SetPrefix(ULlamaContext* Context, FString PromptPrefix);

	/**
	 * Adds a suffix to the user's prompt: a text that will be inserted after the prompt every request on the same context.
	 * @param Context - The context to use
	 * @param PromptSuffix - The suffix to add
	 */
	UFUNCTION(BlueprintCallable, Category="LlamaIntegration")
	static void SetSuffix(ULlamaContext* Context, FString PromptSuffix);

	/** A list of every context loaded at some point in memory */
	static TArray<ULlamaContext*> Contexts;
	
};
