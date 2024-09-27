#include "Layout/HUDActivatableWidget.h"

#include "HUDLayoutBlueprintLibrary.h"
#include "HUDPrimaryLayout.h"

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
