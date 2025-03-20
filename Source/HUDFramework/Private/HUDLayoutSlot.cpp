#include "HUDLayoutSlot.h"

#include "HUDLayoutExtension.h"
#include "HUDLayoutSubsystem.h"
#include "Blueprint/UserWidget.h"

FHUDLayoutSlot::FHUDLayoutSlot(const FGameplayTag& InSlotTag, const ULocalPlayer* InLocalPlayer, TSlotCallback&& InAddCallback, TSlotCallback&& InRemoveCallback)
	: SlotTag(InSlotTag)
	, PlayerContext(InLocalPlayer)
	, AddExtension(InAddCallback)
	, RemoveExtension(InRemoveCallback)
{
	
}

bool FHUDLayoutSlot::ExtensionPassesRequirements(const TSharedPtr<FHUDLayoutExtension>& Extension) const
{
	return SlotTag == Extension->SlotTag && Extension->PlayerContext == PlayerContext && Extension->WidgetClass.Get() != nullptr;
}

FHUDLayoutSlotHandle FHUDLayoutSlotHandle::EmptyHandle{};

void FHUDLayoutSlotHandle::Invalidate()
{
	Slot.Reset();
	Subsystem = nullptr;
}

FHUDLayoutSlotHandle::FHUDLayoutSlotHandle(UHUDLayoutSubsystem* InSubsystem, TSharedPtr<FHUDLayoutSlot> InSlot)
	: Subsystem(InSubsystem)
	, Slot(InSlot)
{
	
}
