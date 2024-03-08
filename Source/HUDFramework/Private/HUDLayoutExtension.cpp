#include "HUDLayoutExtension.h"

FHUDLayoutExtensionHandle FHUDLayoutExtensionHandle::EmptyHandle;

FHUDLayoutExtension::FHUDLayoutExtension(const FGameplayTag& InSlotTag, TSubclassOf<UUserWidget> InWidgetClass, const ULocalPlayer* InLocalPlayer, const FHUDWidgetContextHandle& InContext)
	: SlotTag(InSlotTag)
	, PlayerContext(InLocalPlayer)
	, WidgetClass(InWidgetClass)
	, WidgetContext(InContext)
{
	
}

void FHUDLayoutExtensionHandle::Invalidate()
{
	Extension.Reset();
	Subsystem = nullptr;
}

FHUDLayoutExtensionHandle::FHUDLayoutExtensionHandle(UHUDLayoutSubsystem* InSubsystem, TSharedPtr<FHUDLayoutExtension> InExtension)
	: Subsystem(InSubsystem)
	, Extension(InExtension)
{
	
}
