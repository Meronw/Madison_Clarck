// Copyright 2023, Cheese Labs, All rights reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ProgressiveStringSplitterBPLibrary.generated.h"

#define MIN_SEGMENT_CJK  5
#define MIN_SEGMENT_LATIN 3

#define LANG_CJK   1
#define LANG_LATIN 0
#define LANG_ND   -1

USTRUCT(BlueprintType)
struct FRegexMatchResult
{
	GENERATED_USTRUCT_BODY()

	/* Index zero holds full match. First index has result for 1st capturing group, second index for 2nd capturing group etc.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progressive String Splitter")
	TMap<int32, FString> Results;
};

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/
UCLASS(BlueprintType, Category = "Progressive String Splitter")
class UProgressiveStringSplitterBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UProgressiveStringSplitterBPLibrary(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Progressive String Splitter")
	static UProgressiveStringSplitterBPLibrary* CreateSplitter();

	UFUNCTION(BlueprintCallable, Category = "Progressive String Splitter")
	void ResetSplitter();

	UFUNCTION(BlueprintCallable, Category = "Progressive String Splitter")
	TArray<FString> Split(const FString& Progressive);

	UFUNCTION(BlueprintCallable, Category = "Progressive String Splitter")
	TArray<FString> SplitCJK(const FString& Progressive);

	UFUNCTION(BlueprintCallable, Category = "Progressive String Splitter")
	TArray<FString> SplitLatin(const FString& Progressive);

	UFUNCTION(BlueprintCallable, Category = "Progressive String Splitter")
	FString WindUp(const FString& FinalString);

	/*Indicates whether a pattern can be matched at least once in the input string*/
	UFUNCTION(BlueprintCallable, Category = "Progressive String Splitter")
	static bool RegexCanMatch(const FString& pattern, const FString& input);

	/*Searches the input string for substrings that match the pattern and returns all results*/
	UFUNCTION(BlueprintCallable, Category = "Progressive String Splitter")
	static TArray<FString> RegexMatch(const FString& pattern, const FString& input);

	/*Searches the input string for substrings that match the pattern and returns all results. Apart from full matches, results contain partial matches for capturing groups as well*/
	UFUNCTION(BlueprintCallable, Category = "Progressive String Splitter")
	static TArray<FRegexMatchResult> RegexMatchExtended(const FString& pattern, const FString& input);

private:
	FString ProcessedString;
};
