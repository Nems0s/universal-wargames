// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class space_wargames : ModuleRules
{
	public space_wargames(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        bUseRTTI = true;

        bEnableUndefinedIdentifierWarnings = false;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		string LogicPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../"));

        PublicIncludePaths.Add(Path.Combine(LogicPath, "jeu"));
        PublicIncludePaths.Add(Path.Combine(LogicPath, "joueur"));
        PublicIncludePaths.Add(Path.Combine(LogicPath, "Unite"));
        PublicIncludePaths.Add(Path.Combine(LogicPath, "configs"));

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
