// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#include "..\Public\LlamaRunnerCAsyncActionNode.h"

#include "LlamaRunner.h"

ULlamaRunnerCAsyncActionNode* ULlamaRunnerCAsyncActionNode::GetAIAnswerWithCallbackAsync(ULlamaContext* Context, FString Prompt, const FLlamaRequestCallDelegate Callback, int AnswerLength, FLlamaParams Params)
{
	ULlamaRunnerCAsyncActionNode* Node = NewObject<ULlamaRunnerCAsyncActionNode>();
	
	Node->Context = Context;
	Node->Prompt = Prompt;
	Node->Callback = Callback;
	Node->AnswerLength = AnswerLength;
	Node->Params = Params;

	return Node;
}

void ULlamaRunnerCAsyncActionNode::Activate()
{
	(new FAutoDeleteAsyncTask<BP_GetAIAnswerCAsyncTask>(this))->StartBackgroundTask();
}

//==============================================================
BP_GetAIAnswerCAsyncTask::BP_GetAIAnswerCAsyncTask(ULlamaRunnerCAsyncActionNode* BP_TaskInstance)
{
	CallingObject = TWeakObjectPtr<ULlamaRunnerCAsyncActionNode>(BP_TaskInstance);
}

BP_GetAIAnswerCAsyncTask::~BP_GetAIAnswerCAsyncTask()
{

	if (CallingObject.IsValid())
	{
		ULlamaRunnerCAsyncActionNode* ValidCallingObject = CallingObject.Get();
        	if (ValidCallingObject && ValidCallingObject->FinishedWork.IsBound()) {
        		ValidCallingObject->FinishedWork.Broadcast(Answer);
        	}
            
        	if (ValidCallingObject)
        	{
        		ValidCallingObject->SetReadyToDestroy();
        	}
	}
}

void BP_GetAIAnswerCAsyncTask::DoWork()
{
	if (CallingObject.IsValid())
	{
		ULlamaRunnerCAsyncActionNode* ValidCallingObject = CallingObject.Get();
		if (ValidCallingObject)
		{
			FString Res = ULlamaRunner::GetAIAnswerWithCallback(ValidCallingObject->Context, ValidCallingObject->Prompt, ValidCallingObject->Callback, ValidCallingObject->AnswerLength, ValidCallingObject->Params);
			Answer = Res;
		}
	}
}
