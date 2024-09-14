#include "HUDPrimaryLayout.h"

#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "HUDFramework.h"
#include "ViewModel/HUDWidgetContextSubsystem.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UHUDPrimaryLayout::PopWidget(UCommonActivatableWidget* Widget)
{
	if (Widget == nullptr)
	{
		return;
	}
	
	for (auto& [LayerTag, Layer]: ActiveLayers)
	{
		Layer->RemoveWidget(*Widget);
	}
}

void UHUDPrimaryLayout::PopWidgetFromLayer(FGameplayTag LayerTag, UCommonActivatableWidget* Widget)
{
	if (Widget != nullptr)
	{
		if (UCommonActivatableWidgetContainerBase* Layer = ActiveLayers.FindRef(LayerTag))
		{
			Layer->RemoveWidget(*Widget);
		}
	}
}

UCommonActivatableWidgetContainerBase* UHUDPrimaryLayout::GetLayerWidget(FGameplayTag LayerTag) const
{
	// FindRef returns default value when value is not found
	UCommonActivatableWidgetContainerBase* Layer = ActiveLayers.FindRef(LayerTag);
	if (Layer == nullptr)
	{
		UE_LOG(LogHUDFramework, Error, TEXT("%s: Failed to find layer widget for [%s] layer tag"), *FString(__FUNCTION__), *LayerTag.ToString());
	}
	
	return Layer;
}

void UHUDPrimaryLayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if (IsDesignTime())
	{
		UE_LOG(LogHUDFramework, Error, TEXT("%s: trying to register layer at design time"), *FString(__FUNCTION__));
		return;
	}

	if (LayerWidget == nullptr)
	{
		UE_LOG(LogHUDFramework, Error, TEXT("%s: trying to nullptr layer for [%s] layer tag"), *FString(__FUNCTION__), *LayerTag.ToString());
	}
	
	if (!LayerTag.IsValid())
	{
		UE_LOG(LogHUDFramework, Error, TEXT("%s: trying to register layer [%s] with invalid layer tag"), *FString(__FUNCTION__), *GetNameSafe(LayerWidget));
		return;
	}
	
	if (ActiveLayers.Contains(LayerTag))
	{
		UE_LOG(LogHUDFramework, Error, TEXT("%s: Layer with [%s] layer tag already exists"), *FString(__FUNCTION__), *LayerTag.ToString());
		return;
	}

	ActiveLayers.Add(LayerTag, LayerWidget);
}

void UHUDPrimaryLayout::UnregisterLayer(FGameplayTag LayerTag)
{
	if (UCommonActivatableWidgetContainerBase* Layer = ActiveLayers.FindRef(LayerTag))
	{
		Layer->ClearWidgets();
	}
	
	ActiveLayers.Remove(LayerTag);
}

void UHUDPrimaryLayout::InitActivatableWidget(UCommonActivatableWidget* NewWidget)
{
	check(bAddWidgetGuard == true);

	if (ActiveContext.IsValid())
	{
		if (UHUDWidgetContextSubsystem* Subsystem = UHUDWidgetContextSubsystem::Get(this))
		{
			Subsystem->InitializeWidget_FromUserWidgetPool(NewWidget, ActiveContext);
		}
		ActiveContext.Invalidate();
	}

}
