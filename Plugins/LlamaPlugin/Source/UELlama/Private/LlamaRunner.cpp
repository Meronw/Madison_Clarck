// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#include "LlamaRunner.h"
#include <string>
#include <vector>

#include "LlamaModel.h"
#include "LlamaSettings.h"

static std::string llama_token_to_str(const struct llama_context * ctx, llama_token token)
{
	std::vector<char> result(8, 0);
	const int n_tokens = llama_token_to_piece(ctx, token, result.data(), result.size());
	if (n_tokens < 0) {
		result.resize(-n_tokens);
		int check = llama_token_to_piece(ctx, token, result.data(), result.size());
		GGML_ASSERT(check == -n_tokens);
	} else {
		result.resize(n_tokens);
	}

	return std::string(result.data(), result.size());
}

bool ULlamaRunner::PrepareEmbeds(ULlamaContext* Context, FString& Prompt)
{
	llama_context *LlamaContext = Context->GetLlamaContext();
	
	if (LlamaContext == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Impossible to prepare prompt: valid context missing !"));
		return false;
	} 
		
	Prompt = Context->GetPrefix() + " " + Prompt + " " +Context->GetSuffix();
	
	TArray<llama_token> InputEmbeds;
	std::string Utf8String = TCHAR_TO_UTF8(*Prompt);
	InputEmbeds.SetNum(Utf8String.length() + 1);
	
	const int n = llama_tokenize(LlamaContext, Utf8String.c_str(), InputEmbeds.GetData(), InputEmbeds.Num(), true);
	if (n >= llama_n_ctx(LlamaContext) - 4)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Impossible to prepare prompt: input too long ! Please increase context size in the plugin parameters or make your prompt smaller. "));
		return false;
	}

	if (n < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] An error happened when trying to preapre prompt. "));
		return false;
	}

	// Assure that input can be added to context. if not, remove some old context information
	if (Context->GetEmbeds().Num() + n >= llama_n_ctx(LlamaContext) - 4)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Tokenization: Truncating embeds"));
		int Sum = 0;
		int i;

		//Check how many elements must be removed
		for (i = 0; i < Context->GetIOSizes().Num(); i++)
		{
			Sum +=  Context->GetIOSizes()[i];
			if (Sum >= n) break;
		}

		//Update sizes and remove elements from embeds
		Context->GetIOSizes().RemoveAt(0, i);
		Context->GetEmbeds().RemoveAt(0, Sum);
	}
	
	InputEmbeds.SetNum(n);
	Context->GetIOSizes().Add(n);

	TArray<llama_token> TempArray;

	for (int32 i = 0; i < InputEmbeds.Num(); i++)
	{
		if (Context->stop)
		{
			return false;
		}
		
		TempArray.Reset();
		TempArray.Add(InputEmbeds[i]);
		llama_eval(LlamaContext, TempArray.GetData(), 1, Context->GetEmbeds().Num() + i, SETTINGS->NThreadToUse);
		TempArray.Empty();
	}
	Context->GetEmbeds().Append(InputEmbeds);
	return true;
}

FString ULlamaRunner::PredictNextToken(ULlamaContext* Context, bool& EndReached, FLlamaParams Params)
{
    llama_context *LlamaContext = Context->GetLlamaContext();
    
    if (LlamaContext == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Impossible to apply prediction: valid context missing !"));
        return FString();
    }
    
    const auto LlamaDefaultParams = llama_context_default_params();
	
    FString Prediction;
	FLlamaParams Parameters = Params;

    TArray<llama_token> LastNTokens;
    LastNTokens.Init(0, LlamaDefaultParams.n_ctx);

    llama_token id;
    
    auto logits = llama_get_logits(LlamaContext);
    auto n_vocab = llama_n_vocab(LlamaContext);

    TArray<llama_token_data> Candidates;
    Candidates.Reserve(n_vocab);

    for (llama_token token_id = 0; token_id < n_vocab; token_id++)
    {
        Candidates.Add(llama_token_data{token_id, logits[token_id], 0.0f});
    }

    llama_token_data_array candidates_p;
    candidates_p.data = Candidates.GetData();
    candidates_p.size = Candidates.Num();
    candidates_p.sorted = false;

	float NLLogit = logits[llama_token_nl(LlamaContext)];

    int last_n_repeat = std::min(std::min((int)LastNTokens.Num(), Parameters.RepeatLastN), LlamaDefaultParams.n_ctx);

    llama_sample_frequency_and_presence_penalties(LlamaContext,
                                                  &candidates_p,
                                                  LastNTokens.GetData() + LastNTokens.Num() -
                                                  last_n_repeat,
                                                  Parameters.RepeatPenalty,
                                                  Parameters.AlphaFrequency,
                                                  Parameters.AlphaPresence);
	if (!Parameters.PenalizeNl)
	{
		
		logits[llama_token_nl(LlamaContext)] = NLLogit;
	}

	if (Parameters.Temp <= 0)
	{
		// Greedy sampling
		id = llama_sample_token_greedy(LlamaContext, &candidates_p);
	}
	else
	{
		if (Parameters.Mirostat == 1)
		{
			static float MirostatMu = 2.0f * Parameters.MirostatTau;
			llama_sample_temperature(LlamaContext, &candidates_p, Parameters.Temp);
			id = llama_sample_token_mirostat(LlamaContext, &candidates_p, Parameters.MirostatTau, Parameters.MirostatEta, Parameters.MirostatM, &MirostatMu);
		}
		else if (Parameters.Mirostat == 2)
		{
			static float MirostatMu = 2.0f * Parameters.MirostatTau;
			llama_sample_temperature(LlamaContext, &candidates_p, Parameters.Temp);
			id = llama_sample_token_mirostat_v2(LlamaContext, &candidates_p, Parameters.MirostatTau, Parameters.MirostatEta, &MirostatMu);
		}
		else
		{
			// Temperature sampling
			llama_sample_top_k(LlamaContext, &candidates_p, Parameters.TopK, 1);
			llama_sample_tail_free(LlamaContext, &candidates_p, Parameters.TfsZ, 1);
			llama_sample_typical(LlamaContext, &candidates_p, Parameters.TypicalP, 1);
			llama_sample_top_p(LlamaContext, &candidates_p, Parameters.TopP, 1);
			llama_sample_temperature(LlamaContext, &candidates_p, Parameters.Temp);
			id = llama_sample_token(LlamaContext, &candidates_p);
		}
	}
	
    if (LastNTokens.Num() > 0)
    {
        LastNTokens.RemoveAt(0);
    }

    LastNTokens.Add(id);

    TArray<llama_token> newEmbds;
    newEmbds.Add(id);
    
    Context->GetEmbeds().Append(newEmbds);
	
	std::string res = llama_token_to_str(LlamaContext, id);
	FString Utf8 = UTF8_TO_TCHAR(res.c_str());

	if (id == llama_token_eos(LlamaContext))
	{
		EndReached = true;
	} else
	{
		Prediction += Utf8;
	}

	llama_eval(LlamaContext,  newEmbds.GetData(), 1, (int)  Context->GetEmbeds().Num(), SETTINGS->NThreadToUse);
	return Prediction;
}

FString ULlamaRunner::GetAIAnswer(ULlamaContext* Context, FString Prompt, int AnswerLength, FLlamaParams Params)
{
	FString Answer = FString();
	
	if (Context == nullptr || Context->GetLlamaContext() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Impossible to answer prompt: valid context missing !"));
		return Answer;
	}
	
	FRWScopeLock ModelLock(ULlamaModel::GetLock(), SLT_ReadOnly);
	FRWScopeLock ContextLock(Context->GetLock(), SLT_Write);
	
	llama_context *LlamaContext = Context->GetLlamaContext();

	UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Preparing answer..."));

	if (AnswerLength >= llama_n_ctx(LlamaContext) - 4)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Impossible to answer prompt: output too long !"));
		return Answer;
	}
	
	bool Prepared = PrepareEmbeds(Context, Prompt);

	if (Prepared)
	{

		if (AnswerLength + Context->GetEmbeds().Num()  >= llama_n_ctx(LlamaContext) - 4)
		{
			int Sum = 0;
			int i;

			//Check how many elements must be removed
			for (i = 0; i < Context->GetIOSizes().Num(); i++)
			{
				Sum +=  Context->GetIOSizes()[i];
				if (Sum >= AnswerLength) break;
			}

			//Update sizes and remove elements from embeds
			Context->GetIOSizes().RemoveAt(0, i);
			Context->GetEmbeds().RemoveAt(0, Sum);
		}
		
		int i = 0;
		bool stop = i >= AnswerLength;
		while (!stop && !Context->stop) {
			bool EndReached = false;
			FString Prediction = PredictNextToken(Context, EndReached, Params);
			Answer += Prediction;
			//UE_LOG(LogTemp, Warning, TEXT("[LLama Integration TEMP] Result: %s"), *Answer);
			i++;
			stop = EndReached || (i >= AnswerLength);
		}

		Context->GetIOSizes().Add(i);
	}
	
	return Answer;
}


FString ULlamaRunner::GetAIAnswerWithCallback(ULlamaContext* Context, FString Prompt, const FLlamaRequestCallDelegate& Callback,  int AnswerLength, FLlamaParams Params)
{
	FString Answer = FString();
	
	if (Context == nullptr || Context->GetLlamaContext() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Impossible to answer prompt: valid context missing !"));
		return Answer;
	}

	FRWScopeLock ModelLock(ULlamaModel::GetLock(), SLT_ReadOnly);
	FRWScopeLock ContextLock(Context->GetLock(), SLT_Write);
	
	llama_context *LlamaContext = Context->GetLlamaContext();

	UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Preparing answer..."));

	
	if (AnswerLength >= llama_n_ctx(LlamaContext) - 4)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Impossible to answer prompt: output too long ! Please increase context size in the plugin parameters or make your answer size smaller."));
		return Answer;
	}
	
	bool Prepared = PrepareEmbeds(Context, Prompt);

	if (Prepared)
	{

		if (AnswerLength + Context->GetEmbeds().Num()  >= llama_n_ctx(LlamaContext) - 4)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LLama Integration] Interpretation: Truncating embeds"));
			int Sum = 0;
			int i;

			//Check how many elements must be removed
			for (i = 0; i < Context->GetIOSizes().Num(); i++)
			{
				Sum +=  Context->GetIOSizes()[i];
				if (Sum >= AnswerLength) break;
			}

			//Update sizes and remove elements from embeds
			Context->GetIOSizes().RemoveAt(0, i);
			Context->GetEmbeds().RemoveAt(0, Sum);
		}
		
		int i = 0;
		bool stop = i >= AnswerLength;
		while (!stop && !Context->stop) {
			bool EndReached = false;
			FString Prediction = PredictNextToken(Context, EndReached, Params);
			Answer += Prediction;
			//UE_LOG(LogTemp, Warning, TEXT("[LLama Integration TEMP] Result: %s"), *Answer);
			#if WITH_EDITOR
				FFunctionGraphTask::CreateAndDispatchWhenReady([Callback, Answer]()
				   {
						Callback.ExecuteIfBound(Answer);
				   }, TStatId(), nullptr, ENamedThreads::GameThread);
			#else
				Callback.ExecuteIfBound(Answer);
			#endif
			
			i++;
			stop = EndReached || (i >= AnswerLength);
		}

		Context->GetIOSizes().Add(i);
	}
	
	return Answer;
}
