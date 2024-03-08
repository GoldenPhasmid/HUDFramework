#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"

#include "HUDLayout.generated.h"

/**
 * HUDLayout
 * Widget used to lay out player's HUD and dynamically filled during gameplay.
 * Try use mostly UHUDExtensionPointWidget's in WidgetTree and don't add any logic in it
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class HUDFRAMEWORK_API UHUDLayout: public UCommonActivatableWidget
{
	GENERATED_BODY()
public:

	UHUDLayout(const FObjectInitializer& Initializer);
	
};
