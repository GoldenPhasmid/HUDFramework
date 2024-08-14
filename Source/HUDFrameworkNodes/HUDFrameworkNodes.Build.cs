using UnrealBuildTool;

public class HUDFrameworkNodes : ModuleRules
{
    public HUDFrameworkNodes(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "BlueprintGraph",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "HUDFramework",
                "Kismet",
                "KismetCompiler",
                "UnrealEd",
            }
        );
    }
}