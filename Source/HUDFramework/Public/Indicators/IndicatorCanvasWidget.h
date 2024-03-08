#pragma once

#include "GameplayTagContainer.h"
#include "Components/Widget.h"
#include "IndicatorCanvasWidget.generated.h"

class SIndicatorCanvas;

UCLASS()
class HUDFRAMEWORK_API UIndicatorCanvasWidget : public UWidget
{
	GENERATED_BODY()
	
public:
	UIndicatorCanvasWidget(const FObjectInitializer& Initializer);
	
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
	TSharedPtr<SIndicatorCanvas> IndicatorCanvas;
};
