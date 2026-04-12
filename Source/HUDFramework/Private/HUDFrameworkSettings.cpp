#include "HUDFrameworkSettings.h"

UHUDFrameworkSettings::UHUDFrameworkSettings(const FObjectInitializer& Initializer): Super(Initializer)
{
	CategoryName = TEXT("Game");
	PolicyClass = FSoftClassPath{TEXT("/HUDFramework/BP_DefaultLayoutPolicy.BP_DefaultLayoutPolicy_C")};
}
