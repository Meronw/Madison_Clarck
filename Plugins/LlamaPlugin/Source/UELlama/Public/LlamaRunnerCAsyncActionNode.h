// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LlamaRunner.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Misc/ScopeRWLock.h"
#include "Async/AsyncWork.h"

#include "LlamaRunnerCAsyncActionNode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncCTaskOutput, FString, Answer);

/** A Class that to implement the get ai answer with callback node in an async way. */
UCLASS()
class ULlamaRunnerCAsyncActionNode : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintAssignable)
	FAsyncCTaskOutput FinishedWork;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), category = "LlamaIntegration")
	static ULlamaRunnerCAsyncActionNode* GetAIAnswerWithCallbackAsync(ULlamaContext* Context, FString Prompt, const FLlamaRequestCallDelegate Callback, int AnswerLength = 50, FLlamaParams Params = FLlamaParams());

	virtual void Activate() override;

	friend class BP_GetAIAnswerCAsyncTask;

private:
	ULlamaContext *Context;
	FString Prompt;
	FLlamaRequestCallDelegate Callback;
	int AnswerLength;
	FLlamaParams Params;
};

//===================================================================================

/** A Class that implements an Async Task. Allows the user to execute some work on another thread. */
class BP_GetAIAnswerCAsyncTask : public FNonAbandonableTask
{
public:
	BP_GetAIAnswerCAsyncTask(ULlamaRunnerCAsyncActionNode* BP_TaskInstance);

	~BP_GetAIAnswerCAsyncTask();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(ExampleAutoDeleteAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	TWeakObjectPtr<ULlamaRunnerCAsyncActionNode> CallingObject;

	void DoWork();

	FString Answer;
};
