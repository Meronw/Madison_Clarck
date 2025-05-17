// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

#include "..\Public\LlamaRunnerAsyncActionNode.h"

#include "LlamaRunner.h"

ULlamaRunnerAsyncActionNode* ULlamaRunnerAsyncActionNode::GetAIAnswerAsync(ULlamaContext* Context, FString Prompt, int AnswerLength, FLlamaParams Params)
{
	ULlamaRunnerAsyncActionNode* Node = NewObject<ULlamaRunnerAsyncActionNode>();
	
	Node->Context = Context;
	Node->Prompt = Prompt;
	Node->AnswerLength = AnswerLength;
	Node->Params = Params;

	return Node;
}

void ULlamaRunnerAsyncActionNode::Activate()
{
	(new FAutoDeleteAsyncTask<BP_GetAIAnswerAsyncTask>(this))->StartBackgroundTask();
}

//==============================================================
BP_GetAIAnswerAsyncTask::BP_GetAIAnswerAsyncTask(ULlamaRunnerAsyncActionNode* BP_TaskInstance)
{
	CallingObject = TWeakObjectPtr<ULlamaRunnerAsyncActionNode>(BP_TaskInstance);
}

BP_GetAIAnswerAsyncTask::~BP_GetAIAnswerAsyncTask()
{

	if (CallingObject.IsValid())
	{
		ULlamaRunnerAsyncActionNode* ValidCallingObject = CallingObject.Get();
        	if (ValidCallingObject && ValidCallingObject->FinishedWork.IsBound()) {
        		ValidCallingObject->FinishedWork.Broadcast(Answer);
        	}
            
        	if (ValidCallingObject)
        	{
        		ValidCallingObject->SetReadyToDestroy();
        	}
	}
}

void BP_GetAIAnswerAsyncTask::DoWork()
{
	if (CallingObject.IsValid())
	{
		ULlamaRunnerAsyncActionNode* ValidCallingObject = CallingObject.Get();
		if (ValidCallingObject)
		{
			FString Res = ULlamaRunner::GetAIAnswer(ValidCallingObject->Context, ValidCallingObject->Prompt, ValidCallingObject->AnswerLength, ValidCallingObject->Params);
			Answer = Res;
		}
	}
}
