#pragma once

#include "llama.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoreMinimal.h"
#include "Misc/ScopeRWLock.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

#include "LlamaModel.generated.h"


UCLASS(BlueprintType)
class ULlamaModel : public UObject
{
	GENERATED_BODY()

public:
	ULlamaModel() : LlamaModel(nullptr)
	{
		AddToRoot();
		Instance = this;
	}

	virtual ~ULlamaModel() override;

	/**
	 * Loads a Llama model into memory.
	 * @param ModelPath - The path of the model stored locally.
	 * @return The Loaded model
	 */
	UFUNCTION(BlueprintCallable, Category = "LlamaIntegration")
	static ULlamaModel* LoadModel(const FString& ModelPath);

	/**
	 * Unloads a Llama model from memory.
	 */
	UFUNCTION(BlueprintCallable, Category = "LlamaIntegration")
	static void FreeModel();

	/**
	 * Returns the model called on the last LoadModel call.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "LlamaIntegration")
	static ULlamaModel* GetInstance()
	{
		return Instance;
	}

	llama_model* GetLlamaModel() const
	{
		return LlamaModel;
	}

	static FRWLock& GetLock()
	{
		return WriteLock;
	}

	UFUNCTION()
	void OnEndPIE(bool bIsSimulating)
	{
		FreeModel();
	}

private:
	//Only one model is used during execution
	static ULlamaModel* Instance;

	static FRWLock WriteLock;
	llama_model* LlamaModel;

	//Check if llama memory has already been destroyed
	inline static bool isUnloaded;

#if WITH_EDITOR
	FDelegateHandle EndPIEdelegate = FEditorDelegates::EndPIE.AddUObject(this, &ULlamaModel::OnEndPIE);
#endif
};
