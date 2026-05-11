// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ARPG_Project : ModuleRules
{
	public ARPG_Project(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"NetCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"GlobalEventSystem",
			"OnlineSubsystem", 
			"OnlineSubsystemUtils",
			"AnimGraphRuntime",
			// "OnlineSubsystemSteam"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks", "AnimGraphRuntime", "AnimGraphRuntime",
		});

		PublicIncludePaths.AddRange(new string[] {
			"ARPG_Project",
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
