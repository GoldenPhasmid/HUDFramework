#pragma once

#include "HUDIndicatorWidgetInterface.generated.h"

class USceneComponent;
class UHUDIndicatorDescriptor;

UINTERFACE(BlueprintType)
class HUDFRAMEWORK_API UHUDIndicatorWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class IHUDIndicatorWidgetInterface
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Indicator")
	void SetIndicator(const UHUDIndicatorDescriptor* Descriptor, const USceneComponent* OwnerComponent);

	UFUNCTION(BlueprintNativeEvent, Category = "Indicator")
	void ResetIndicator();
};
