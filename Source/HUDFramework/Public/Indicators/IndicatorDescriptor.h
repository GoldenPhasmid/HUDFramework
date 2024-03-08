#pragma once

#include "GameplayTagContainer.h"
#include "IndicatorDescriptor.generated.h"

class UIndicatorProjectionMode;

/*
 * Data asset that describes common behavior for all indicators of certain type
 */
UCLASS(BlueprintType)
class HUDFRAMEWORK_API UIndicatorDescriptor : public UDataAsset
{
	GENERATED_BODY()
	
public:
	/* Defines which indicator canvas widget should manage group of indicators with this descriptor */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "indicator", meta = (Validate))
	FGameplayTag CategoryTag;
	
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Indicator", meta = (Validate))
	TObjectPtr<UIndicatorProjectionMode> ProjectionMode;

	/* Widget that will be created for this indicator */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator", meta = (Validate))
	TSoftClassPtr<UUserWidget> IndicatorWidgetClass;

	/* Defines paint priority. Indicator with priority = 0 will paint above indicator with priority = 1 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator", meta = (ClampMin = 0))
	int32 Priority = 0;

	/* Should indicator display even if Component can not render? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator", AdvancedDisplay)
	bool bDisplayIndicatorWhenComponentCanNotRender = false;

	/* Enables scaling based on depth for this indicator. See UIndicatorProjectionMode */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Scaling", meta = (InlineEditConditionToggle))
	bool bEnableScaling = false;

	/* Curve for indicator scaling. Time - Depth, Value - Scale coefficient. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Scaling", meta = (EditCondition = "bEnableScaling"))
	TObjectPtr<UCurveFloat> ScaleCurve;

	/* Should indicator project on edge of the screen? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Clamp")
	bool bClampToScreen = false;

	/* Should clamped indicator draw arrow? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Clamp", meta = (EditCondition = "bClampToScreen == true"))
	bool bShowClampToScreenArrow = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Alignment")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Center;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Alignment")
	TEnumAsByte<EVerticalAlignment> VerticalAlignment = VAlign_Center;
};
