// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LlamaRunner.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Misc/ScopeRWLock.h"
#include "Async/AsyncWork.h"

#include "LlamaRunnerAsyncActionNode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTaskOutput, FString, Answer);

/** A Class that to implement the get ai answer node in an async way. */
UCLASS()
class ULlamaRunnerAsyncActionNode : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintAssignable)
	FAsyncTaskOutput FinishedWork;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), category = "LlamaIntegration")
	static ULlamaRunnerAsyncActionNode* GetAIAnswerAsync(ULlamaContext* Context, FString Prompt, int AnswerLength = 50, FLlamaParams Params = FLlamaParams());

	virtual void Activate() override;

	friend class BP_GetAIAnswerAsyncTask;

private:
	ULlamaContext *Context;
	FString Prompt;
	int AnswerLength;
	FLlamaParams Params;
};

//===================================================================================
/** A Class that implements an Async Task. Allows the user to execute some work on another thread. */
class BP_GetAIAnswerAsyncTask : public FNonAbandonableTask
{
public:
	BP_GetAIAnswerAsyncTask(ULlamaRunnerAsyncActionNode* BP_TaskInstance);

	~BP_GetAIAnswerAsyncTask();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(ExampleAutoDeleteAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	TWeakObjectPtr<ULlamaRunnerAsyncActionNode> CallingObject;

	void DoWork();

	FString Answer;
};
