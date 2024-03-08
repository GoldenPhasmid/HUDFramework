using UnrealBuildTool;

public class HUDFramework : ModuleRules
{
	public HUDFramework(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CommonUI",
				"UMG",
				"Slate",
				"SlateCore",
				"DeveloperSettings",
				"ModularGameplay",
				"AsyncMixin",
				"ModelViewViewModel",
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"GameplayTags",
			}
		);
	}
}
