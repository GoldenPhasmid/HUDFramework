#pragma once

#include "IndicatorWidgetInterface.generated.h"

class USceneComponent;
class UIndicatorDescriptor;

UINTERFACE(BlueprintType)
class HUDFRAMEWORK_API UIndicatorWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class IIndicatorWidgetInterface
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Indicator")
	void SetIndicator(const UIndicatorDescriptor* Descriptor, const USceneComponent* OwnerComponent);

	UFUNCTION(BlueprintNativeEvent, Category = "Indicator")
	void ResetIndicator();
};
