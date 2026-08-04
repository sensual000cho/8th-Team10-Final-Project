// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CreatePotion : ModuleRules
{
	public CreatePotion(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayTags",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"CreatePotion",
			"CreatePotion/Variant_Platforming",
			"CreatePotion/Variant_Platforming/Animation",
			"CreatePotion/Variant_Combat",
			"CreatePotion/Variant_Combat/AI",
			"CreatePotion/Variant_Combat/Animation",
			"CreatePotion/Variant_Combat/Gameplay",
			"CreatePotion/Variant_Combat/Interfaces",
			"CreatePotion/Variant_Combat/UI",
			"CreatePotion/Variant_SideScrolling",
			"CreatePotion/Variant_SideScrolling/AI",
			"CreatePotion/Variant_SideScrolling/Gameplay",
			"CreatePotion/Variant_SideScrolling/Interfaces",
			"CreatePotion/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
