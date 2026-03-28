#pragma once

#include "GameplayTagContainer.h"
#include "HUDWidgetPool.h"
#include "Components/Widget.h"
#include "HUDIndicatorCanvasWidget.generated.h"

class SIndicatorCanvas;

UCLASS()
class HUDFRAMEWORK_API UHUDIndicatorCanvasWidget : public UWidget
{
	GENERATED_BODY()
	
public:
	UHUDIndicatorCanvasWidget(const FObjectInitializer& Initializer);
	
	// ~Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// ~End UVisual Interface

protected:
	// ~Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// ~End UWidget Interface

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Canvas")
	FGameplayTagContainer CategoryTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Canvas")
	FSlateBrush ArrowBrush;

protected:
	UPROPERTY(Transient)
	FHUDWidgetPool WidgetPool;
	
	TSharedPtr<SIndicatorCanvas> IndicatorCanvas;
};
