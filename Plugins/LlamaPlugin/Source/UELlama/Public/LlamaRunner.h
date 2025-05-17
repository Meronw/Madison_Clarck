// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#pragma once

#include "LlamaContext.h"
#include "Misc/ScopeRWLock.h"

#include "LlamaRunner.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FLlamaRequestCallDelegate, FString, Answer);

USTRUCT(BlueprintType)
struct FLlamaParams
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	float Temp = 0.80f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	float TopP = 0.95f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	float RepeatPenalty = 1.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	int32 TopK = 40;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	float TfsZ = 1.00f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	float TypicalP = 1.00f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	int32 RepeatLastN = 64;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	float AlphaPresence = 0.00f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	float AlphaFrequency = 0.00f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	int32 Mirostat = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	float MirostatTau = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	float MirostatEta = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	int MirostatM = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LlamaIntegration Advanced Params")
	bool PenalizeNl = true;
};


UCLASS()
class ULlamaRunner : public UObject
{
	GENERATED_BODY() 
	
public:

	/**
	 * Uses a Llama model to interpret a user request and returns the result of the request
	 * @param Context - The context to use
	 * @param Prompt - The prompt submitted by the user
	 * @param AnswerLength - The specified character limit for the response (the response may be truncated mid-sentence)
	 * @param Params - Advanced parameters to customize responses quality 
	 * @return The AI's response to the user's prompt
	 */
	UFUNCTION(BlueprintCallable, Category="LlamaIntegration")
	static FString GetAIAnswer(ULlamaContext* Context, FString Prompt, int AnswerLength = 50, FLlamaParams Params = FLlamaParams());

	/**
	 * Uses a Llama model to interpret a user request and returns the result of the request. 
	 * Implements a callback to treat the response while it is still generating.
	 * @param Context - The context to use
	 * @param Prompt - The prompt submitted by the user
	 * @param Callback - The Event that will be called when a new word is generated
	 * @param AnswerLength - The specified character limit for the response (the response may be truncated mid-sentence)
	 * @return The AI's response to the user's prompt.
	 */
	UFUNCTION(BlueprintCallable, Category="LlamaIntegration")
	static FString GetAIAnswerWithCallback(ULlamaContext* Context, FString Prompt, const FLlamaRequestCallDelegate& Callback, int AnswerLength = 50, FLlamaParams Params = FLlamaParams());

	/**
	 * Tokenizes and interprets the user's input prompt, preparing the answer for evaluation.
	 * This function is used within the "GetAIAnswer" process.
	 * @param Context - The context to use
	 * @param Prompt - The prompt submitted by the user
	 * @return Whether Llama could tokenize the answer or not.
	 */
	static bool PrepareEmbeds(ULlamaContext* Context, FString& Prompt);

	/**
	 * Evaluates a token and return the translation of the token in a human-readable language
	 * @param Context - The context to use
	 * @param EndReached - Whether Llama has finished to answer or not
	 * @param Params - Advanced parameters to customize responses quality 
	 * @return The translation of the token in a human-readable language.
	 */
	static FString PredictNextToken(ULlamaContext* Context, bool& EndReached, FLlamaParams Params);
	
};

