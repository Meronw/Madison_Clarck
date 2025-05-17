// Copyright 2023 Isara Technologies SAS. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class UELlama : ModuleRules
{
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

	private string ThirdPartyPath
	{
		get { return Path.GetFullPath(Path.Combine(ModulePath, "../ThirdParty/")); }
	}

	public bool LoadLlama(ReadOnlyTargetRules Target)
	{
		
		bool isLibrarySupported = false;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			isLibrarySupported = true;

			string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "Win32";
			string LibrariesPath = Path.Combine(Path.Combine(Path.Combine(ThirdPartyPath, "llama", "lib"), "vs"), PlatformString);

			PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "llama.lib"));

			string[] dlls = { "llama.dll" };

			string BinariesPath = Path.Combine(Path.Combine(Path.Combine(ThirdPartyPath, "llama", "bin"), "vs"), PlatformString);
			foreach (string dll in dlls)
			{
				PublicDelayLoadDLLs.Add(dll);
				RuntimeDependencies.Add(Path.Combine(BinariesPath, dll), StagedFileType.NonUFS);
			}

		}

		if (isLibrarySupported)
		{
			// Include path
			PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "llama", "include"));
		}


		return isLibrarySupported;
	}

	public UELlama(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		OptimizeCode = CodeOptimization.Never;
		bEnableUndefinedIdentifierWarnings = false;
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore"
				// ... add other public dependencies that you statically link with here ...
			}
		);
		
		if (Target.Type == TargetRules.TargetType.Editor)
		{
			PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
		} 

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"MediaUtils",
				"RenderCore",
				"Projects"
			});

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"UELlama/Private"
			});


		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

		LoadLlama(Target);
	}
}
