using UnrealBuildTool;

public class HUDFrameworkGameFeatureActions : ModuleRules
{
    public HUDFrameworkGameFeatureActions(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
                "GameExperienceActions",
                "HUDFramework",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore", 
                "UMG",
                "ModularGameplay",
                "GameFeatures",
                "GameplayTags",
                "CommonUI",
            }
        );
    }
}