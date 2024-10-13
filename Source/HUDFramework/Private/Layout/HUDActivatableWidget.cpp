#include "Layout/HUDActivatableWidget.h"

#include "HUDLayoutBlueprintLibrary.h"
#include "HUDPrimaryLayout.h"

TOptional<FUIInputConfig> UHUDActivatableWidget::GetDesiredInputConfig() const
{
	// default behavior is to receive all input
	return FUIInputConfig{InputMode, EMouseCaptureMode::CaptureDuringMouseDown, false};
}

TSharedRef<SWidget> UHUDActivatableWidget::RebuildWidget()
{
	if (!IsDesignTime())
	{
		const APlayerController* OwningPlayer = GetOwningPlayer();
		check(OwningPlayer);

		if (UHUDPrimaryLayout* PrimaryLayout = UHUDLayoutBlueprintLibrary::GetPrimaryLayout(OwningPlayer))
		{
			PrimaryLayout->InitActivatableWidget(*this);
		}
	}
	
	return Super::RebuildWidget();
}
