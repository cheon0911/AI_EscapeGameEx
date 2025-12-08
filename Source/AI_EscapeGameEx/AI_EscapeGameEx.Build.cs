// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AI_EscapeGameEx : ModuleRules
{
	public AI_EscapeGameEx(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
            "Niagara",
            "EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"AI_EscapeGameEx",
			"AI_EscapeGameEx/Variant_Platforming",
			"AI_EscapeGameEx/Variant_Platforming/Animation",
			"AI_EscapeGameEx/Variant_Combat",
			"AI_EscapeGameEx/Variant_Combat/AI",
			"AI_EscapeGameEx/Variant_Combat/Animation",
			"AI_EscapeGameEx/Variant_Combat/Gameplay",
			"AI_EscapeGameEx/Variant_Combat/Interfaces",
			"AI_EscapeGameEx/Variant_Combat/UI",
			"AI_EscapeGameEx/Variant_SideScrolling",
			"AI_EscapeGameEx/Variant_SideScrolling/AI",
			"AI_EscapeGameEx/Variant_SideScrolling/Gameplay",
			"AI_EscapeGameEx/Variant_SideScrolling/Interfaces",
			"AI_EscapeGameEx/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
