#include "Indicators/IndicatorCanvasWidget.h"

#include "Indicators/IndicatorCanvas.h"

UIndicatorCanvasWidget::UIndicatorCanvasWidget(const FObjectInitializer& Initializer) : Super(Initializer)
{
	bIsVariable = true;
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UIndicatorCanvasWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	IndicatorCanvas.Reset();
	WidgetPool.ResetPool();
}

TSharedRef<SWidget> UIndicatorCanvasWidget::RebuildWidget()
{
#if WITH_EDITOR
	if (IsDesignTime())
	{
		return SNew(SBox);
	}
#endif

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	
	if (ensureAlwaysMsgf(LocalPlayer != nullptr, TEXT("%s: Attempt to rebuild with invalid LocalPlayer!"), *FString(__FUNCTION__)))
	{
		IndicatorCanvas = SNew(SIndicatorCanvas, FLocalPlayerContext(LocalPlayer), CategoryTags, &ArrowBrush);
		IndicatorCanvas->SetWidgetPool(&WidgetPool);
		return IndicatorCanvas.ToSharedRef();
	}

	// Return SBox because returning null widget is unsafe
	return SNew(SBox);
}
