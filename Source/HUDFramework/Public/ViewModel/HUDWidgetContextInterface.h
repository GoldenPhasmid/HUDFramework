#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "HUDWidgetContextInterface.generated.h"

struct FHUDWidgetContextHandle;

UINTERFACE(BlueprintType)
class HUDFRAMEWORK_API UHUDWidgetContextInterface: public UInterface
{
	GENERATED_BODY()
};

// @todo: come up with better name for this interface
class HUDFRAMEWORK_API IHUDWidgetContextInterface
{
	GENERATED_BODY()
public:

	/**
	 * Called after OnInitialized but before PreConstruct and definitely before Construct
	 * It is guaranteed for view models to be initialized when this called, use it to pass/transform your widget context to your children
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Widget Context")
	void InitializeWidgetTree(const FHUDWidgetContextHandle& WidgetContext);
	
};