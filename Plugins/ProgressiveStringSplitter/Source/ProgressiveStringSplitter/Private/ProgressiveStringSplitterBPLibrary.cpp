// Copyright 2023, Cheese Labs, All rights reserved.

#include "ProgressiveStringSplitterBPLibrary.h"
#include "ProgressiveStringSplitter.h"
#include "Internationalization/Regex.h"

UProgressiveStringSplitterBPLibrary::UProgressiveStringSplitterBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	ProcessedString = TEXT("");
}

UProgressiveStringSplitterBPLibrary* UProgressiveStringSplitterBPLibrary::CreateSplitter()
{
	return NewObject<UProgressiveStringSplitterBPLibrary>();
}

void UProgressiveStringSplitterBPLibrary::ResetSplitter()
{
	ProcessedString = TEXT("");
}

TArray<FString> UProgressiveStringSplitterBPLibrary::Split(const FString& Progressive)
{
	TArray<FString> ret;
	FString str = Progressive.RightChop(ProcessedString.Len());

	int32 cjk_count = RegexMatch(TEXT("\\p{Han}|\\p{Hiragana}|\\p{Katakana}|\\p{Hangul}"), str).Num();
	int32 latin_count = RegexMatch(TEXT("\\p{Latin}+\\s*"), str).Num();
	if (cjk_count < MIN_SEGMENT_CJK && latin_count < MIN_SEGMENT_LATIN) return ret;
	if (cjk_count > latin_count) ret = SplitCJK(Progressive);
	else ret = SplitLatin(Progressive);

	return ret;
}

TArray<FString> UProgressiveStringSplitterBPLibrary::SplitCJK(const FString& Progressive)
{
	TArray<FString> ret;

	// Prepare string for current loop
	FString str = Progressive.RightChop(ProcessedString.Len());
	int len = str.Len();

	// Move leading whitespace characters (if any) to Processed
	FString trimmed = str.TrimStart();
	int trimmed_len = trimmed.Len();
	if (trimmed_len != len)
	{
		ProcessedString += str.Left(len - trimmed_len);
		str = trimmed;
		len = str.Len();
	}

	if (len < MIN_SEGMENT_CJK) return ret;

	bool didSplit;
	do
	{
		didSplit = false;
		for (int i = 0; i < len; i++)
		{
			FString c = str.Mid(i, 1);
			//              .                      ;                      !                      ?            newline
			if (c == TEXT("\u3002") || c == TEXT("\uff1b") || c == TEXT("\uff01") || c == TEXT("\uff1f") || c == TEXT("\n"))
			{
				FString seg = str.Left(i + 1);
				didSplit = true;
				ProcessedString += seg;
				str.RightChopInline(i + 1);
				if (seg.TrimStartAndEnd().Len() != 0) ret.Push(FString(seg));
				break;
			}
		}
	} while (didSplit);

	return ret;
}

TArray<FString> UProgressiveStringSplitterBPLibrary::SplitLatin(const FString& Progressive)
{
	TArray<FString> ret;

	// Prepare string for current loop
	FString str = Progressive.RightChop(ProcessedString.Len());
	int len = str.Len();

	// Move leading whitespace characters (if any) to ProcessedOut
	FString trimmed = str.TrimStart();
	int trimmed_len = trimmed.Len();
	if (trimmed_len != len)
	{
		ProcessedString += str.Left(len - trimmed_len);
		str = trimmed;
		len = str.Len();
	}

	bool didSplit;
	do
	{
		TArray<FRegexMatchResult> words = RegexMatchExtended(TEXT("(\\X+?)(\\s|\\.|\\?|!|;|\\n)\\s*"), str);
		int wcnt = words.Num();
		FString matched;

		didSplit = false;
		for (int i = 0; i < wcnt; i++)
		{
			FString raw = words[i].Results[0];
			FString word = words[i].Results[1];
			FString tail = words[i].Results[2].Replace(TEXT(" "), TEXT(""));
			FString tail_trimmed = tail.TrimStartAndEnd();
			matched += raw;
			//                         .                            ;                            !                            ?               newline
			if ((tail_trimmed == TEXT(".") || tail_trimmed == TEXT(";") || tail_trimmed == TEXT("!") || tail_trimmed == TEXT("?") || tail.Len() != tail_trimmed.Len()))
			{
				// Although a word ends with a dot, following conditions implied it may not be a sentence, continue with no split
				if (tail_trimmed == ".")
				{
					// filter out abbr. For words before a dot, 
					// 1. One letter no matter the case. Example: James O. Smith
					if (RegexCanMatch(TEXT("^\\p{Lu}$"), word)) continue;
					// 2. Start with an upper case, following with lower case, and len <= 4. Example: Setp. 25th
					if (RegexCanMatch(TEXT("^\\p{Lu}\\p{Ll}{0,3}$"), word) && word.Len() <= 4) continue;

					// filter ordered list: all digits, at the begining of a sentence.
					if (RegexCanMatch(TEXT("^\\p{N}+$"), word) && i == 0) continue;
				}

				// filter short sentence
				if (i < MIN_SEGMENT_LATIN) continue;

				// found a valid sentence
				didSplit = true;
				ProcessedString += matched;
				str.RightChopInline(matched.Len());
				ret.Push(matched.TrimStartAndEnd());
				break;
			}
		}
	} while (didSplit);
	return ret;
}

FString UProgressiveStringSplitterBPLibrary::WindUp(const FString& FinalString)
{
	int32 len = ProcessedString.Len();
	ProcessedString = TEXT("");
	return FinalString.RightChop(len).TrimStartAndEnd();
}

bool UProgressiveStringSplitterBPLibrary::RegexCanMatch(const FString& pattern, const FString& input)
{
	const FRegexPattern frp = FRegexPattern(pattern);
	FRegexMatcher frm = FRegexMatcher(frp, input);
	bool isMatch = frm.FindNext();
	return isMatch;
}

TArray<FString> UProgressiveStringSplitterBPLibrary::RegexMatch(const FString& pattern, const FString& input)
{
	TArray<FString> results;

	const FRegexPattern frp = FRegexPattern(pattern);
	FRegexMatcher frm = FRegexMatcher(frp, input);

	while (frm.FindNext())
	{
		FString match = frm.GetCaptureGroup(0);
		results.Add(match);
	}

	return results;
}

TArray<FRegexMatchResult> UProgressiveStringSplitterBPLibrary::RegexMatchExtended(const FString& pattern, const FString& input)
{
	TArray<FRegexMatchResult> results;

	const FRegexPattern frp = FRegexPattern(pattern);
	FRegexMatcher frm = FRegexMatcher(frp, input);

	while (frm.FindNext())
	{
		FRegexMatchResult extResult;

		for (int32 i = 0; i <= frm.GetMatchEnding(); i++)
		{
			FString group = frm.GetCaptureGroup(i);
			if (group.Len() > 0)
			{
				extResult.Results.Add(i, frm.GetCaptureGroup(i));
			}
		}
		results.Add(extResult);
	}

	return results;
}
