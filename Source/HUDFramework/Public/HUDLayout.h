#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Layout/HUDActivatableWidget.h"

#include "HUDLayout.generated.h"

/**
 * UHUDLayout
 * Widget used to lay out player's HUD and dynamically filled during gameplay.
 * Try use mostly UHUDLayoutExtension widgets in WidgetTree and don't add any logic in it
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class HUDFRAMEWORK_API UHUDLayout: public UHUDActivatableWidget
{
	GENERATED_BODY()
public:
	
};
