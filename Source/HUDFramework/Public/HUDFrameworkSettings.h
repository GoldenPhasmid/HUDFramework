#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "HUDFrameworkSettings.generated.h"

class UHUDLayoutPolicy;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "HUD Framework"))
class HUDFRAMEWORK_API UHUDFrameworkSettings: public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UHUDFrameworkSettings(const FObjectInitializer& Initializer);

	UPROPERTY(EditDefaultsOnly, Config, meta = (Validate))
	TSoftClassPtr<UHUDLayoutPolicy> PolicyClass;
	
};
