#include "HUDLayoutBlueprintLibrary.h"

#include "HUDLayoutExtension.h"
#include "HUDLayoutPolicy.h"
#include "HUDLayoutSlot.h"
#include "HUDLayoutSubsystem.h"
#include "HUDPrimaryLayout.h"
#include "ViewModel/HUDWidgetContextSubsystem.h"
#include "CommonActivatableWidget.h"
#include "Blueprint/UserWidget.h"

FString UHUDLayoutBlueprintLibrary::ConstructWidgetTreeString(const UUserWidget* UserWidget)
{
	FString Result = GetNameSafe(UserWidget);

	const UWidget* CurrentWidget = UserWidget;
	while (CurrentWidget && CurrentWidget->GetOuter())
	{
		CurrentWidget = Cast<UWidget>(CurrentWidget->GetOuter()->GetOuter());
		if (const UUserWidget* CurrentUserWidget = Cast<UUserWidget>(CurrentWidget))
		{
			Result += TEXT(" -> ") + GetNameSafe(CurrentUserWidget);
		}
	}

	return Result;
}

UHUDPrimaryLayout* UHUDLayoutBlueprintLibrary::GetPrimaryLayout(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		return nullptr;
	}
	
	if (const UHUDLayoutSubsystem* LayoutSubsystem = PlayerController->GetGameInstance()->GetSubsystem<UHUDLayoutSubsystem>())
	{
		if (const UHUDLayoutPolicy* Policy = LayoutSubsystem->GetPolicy())
		{
			return Policy->GetPrimaryLayout(PlayerController->GetLocalPlayer());
		}
	}

	return nullptr;
}

UCommonActivatableWidget* UHUDLayoutBlueprintLibrary::PushContentToLayer(const APlayerController* PlayerController, FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!IsValid(PlayerController) || !LayerTag.IsValid())
	{
		return nullptr;
	}

	if (UHUDPrimaryLayout* PrimaryLayout =  UHUDLayoutBlueprintLibrary::GetPrimaryLayout(PlayerController))
	{
		return PrimaryLayout->PushWidgetToLayer(LayerTag, WidgetClass);
	}

	return nullptr;
}

void UHUDLayoutBlueprintLibrary::PushStreamedContentToLayer(const APlayerController* PlayerController, FGameplayTag LayerTag, TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
{
	if (!IsValid(PlayerController) || !LayerTag.IsValid() || WidgetClass == nullptr)
	{
		return;
	}

	if (UHUDPrimaryLayout* PrimaryLayout =  UHUDLayoutBlueprintLibrary::GetPrimaryLayout(PlayerController))
	{
		PrimaryLayout->PushWidgetToLayerAsync(LayerTag, WidgetClass);
	}
}

void UHUDLayoutBlueprintLibrary::PopContentFromLayer(const APlayerController* PlayerController, FGameplayTag LayerTag, UCommonActivatableWidget* Widget)
{
	if (!IsValid(PlayerController) || !LayerTag.IsValid() || Widget == nullptr)
	{
		return;
	}

	if (UHUDPrimaryLayout* PrimaryLayout = UHUDLayoutBlueprintLibrary::GetPrimaryLayout(PlayerController))
	{
		PrimaryLayout->PopWidgetFromLayer(LayerTag, Widget);
	}
}

void UHUDLayoutBlueprintLibrary::SetWidgetContext(UUserWidget* UserWidget, UObject* Context, const UObject* DataPayload)
{
	if (UHUDWidgetContextSubsystem* Subsystem = UHUDWidgetContextSubsystem::Get(UserWidget))
	{
		Subsystem->RegisterWidget(UserWidget, FHUDWidgetContextHandle::CreateContext<FHUDWidgetContext>(Context, DataPayload));
	}
}

void UHUDLayoutBlueprintLibrary::SetWidgetContext_FromHandle(UUserWidget* UserWidget, const FHUDWidgetContextHandle& ContextHandle)
{
	if (UHUDWidgetContextSubsystem* Subsystem = UHUDWidgetContextSubsystem::Get(UserWidget))
	{
		Subsystem->RegisterWidget(UserWidget, ContextHandle);
	}
}

void UHUDLayoutBlueprintLibrary::InitializeWidget(UUserWidget* UserWidget, UObject* Context, const UObject* DataPayload)
{
	if (UHUDWidgetContextSubsystem* Subsystem = UHUDWidgetContextSubsystem::Get(UserWidget))
	{
		Subsystem->InitializeWidget(UserWidget, FHUDWidgetContextHandle::CreateContext<FHUDWidgetContext>(Context, DataPayload));
	}
}

void UHUDLayoutBlueprintLibrary::InitializeWidget_FromHandle(UUserWidget* UserWidget, const FHUDWidgetContextHandle& ContextHandle)
{
	if (UHUDWidgetContextSubsystem* Subsystem = UHUDWidgetContextSubsystem::Get(UserWidget))
	{
		Subsystem->InitializeWidget(UserWidget, ContextHandle);
	}
}

FHUDWidgetContextHandle UHUDLayoutBlueprintLibrary::GetWidgetContext(const UUserWidget* UserWidget)
{
	if (UHUDWidgetContextSubsystem* Subsystem = UHUDWidgetContextSubsystem::Get(UserWidget))
	{
		return Subsystem->GetWidgetContext(UserWidget);
	}

	return {};
}

bool UHUDLayoutBlueprintLibrary::IsValid_WidgetContext(const FHUDWidgetContextHandle& WidgetContext)
{
	return WidgetContext.IsValid();
}

UObject* UHUDLayoutBlueprintLibrary::GetContextObject_WidgetContext(const FHUDWidgetContextHandle& WidgetContext)
{
	if (!WidgetContext.IsValid())
	{
		return nullptr;
	}

	const FHUDWidgetContext& Context = WidgetContext.GetContext<FHUDWidgetContext>();
	return Context.ContextObject;
}

const UObject* UHUDLayoutBlueprintLibrary::GetDataObject_WidgetContext(const FHUDWidgetContextHandle& WidgetContext)
{
	if (!WidgetContext.IsValid())
	{
		return nullptr;
	}

	const FHUDWidgetContext& Context = WidgetContext.GetContext<FHUDWidgetContext>();
	return Context.DataObject;
}

bool UHUDLayoutBlueprintLibrary::IsValid_SlotHandle(FHUDLayoutSlotHandle& Handle)
{
	return Handle.IsValid();
}

void UHUDLayoutBlueprintLibrary::Unregister_SlotHandle(FHUDLayoutSlotHandle& Handle)
{
	if (Handle.IsValid() && Handle.Subsystem.IsValid())
	{
		UHUDLayoutSubsystem* Subsystem = Handle.Subsystem.Get();
		Subsystem->UnregisterLayoutSlot(Handle);
	}
}

bool UHUDLayoutBlueprintLibrary::IsValid_ExtensionHandle(FHUDLayoutExtensionHandle& Handle)
{
	return Handle.IsValid();
}

void UHUDLayoutBlueprintLibrary::Unregister_ExtensionHandle(FHUDLayoutExtensionHandle& Handle)
{
	if (Handle.IsValid() && Handle.Subsystem.IsValid())
	{
		UHUDLayoutSubsystem* Subsystem = Handle.Subsystem.Get();
		Subsystem->UnregisterLayoutExtension(Handle);
	}
}
