#include "Indicators/HUDIndicatorBlueprintLibrary.h"

#include "HUDFramework.h"
#include "Indicators/HUDIndicatorManagerComponent.h"
#include "Indicators/HUDIndicatorDescriptor.h"

#define GET_INDICATOR_MANAGER_OR_RETURN(WorldContext) \
	UHUDIndicatorManagerComponent* IndicatorManager = UHUDIndicatorManagerComponent::Get(WorldContext); \
	if (!IsValid(IndicatorManager)) \
	{ \
		UE_LOG(LogIndicators, Error, TEXT("%s: Failed to find indicator manager for context %s"), *FString(__FUNCTION__), *GetNameSafe(WorldContext)); \
		return; \
	}

void UHUDIndicatorBlueprintLibrary::AddIndicator_Actor(const UHUDIndicatorDescriptor* Descriptor, const AActor* OwnerActor)
{
	GET_INDICATOR_MANAGER_OR_RETURN(OwnerActor);

	IndicatorManager->AddIndicator(Descriptor, OwnerActor);
}

void UHUDIndicatorBlueprintLibrary::AddIndicator_Component(const UHUDIndicatorDescriptor* Descriptor, const USceneComponent* Component, FName SocketName)
{
	GET_INDICATOR_MANAGER_OR_RETURN(Component);

	IndicatorManager->AddIndicator(Descriptor, Component, SocketName);
}

void UHUDIndicatorBlueprintLibrary::AddIndicator_ActorWithContext(const UHUDIndicatorDescriptor* Descriptor, const AActor* OwnerActor, UObject* ContextObject)
{
	GET_INDICATOR_MANAGER_OR_RETURN(OwnerActor);

	IndicatorManager->AddIndicatorWithContext(Descriptor, OwnerActor, FHUDWidgetContextHandle::CreateContext<FIndicatorWidgetContext>(ContextObject, Descriptor));
}

void UHUDIndicatorBlueprintLibrary::AddIndicator_ComponentWithContext(const UHUDIndicatorDescriptor* Descriptor, const USceneComponent* Component, FName SocketName, UObject* ContextObject)
{
	GET_INDICATOR_MANAGER_OR_RETURN(Component);

	IndicatorManager->AddIndicatorWithContext(Descriptor, Component, SocketName, FHUDWidgetContextHandle::CreateContext<FIndicatorWidgetContext>(ContextObject, Descriptor));
}

void UHUDIndicatorBlueprintLibrary::RemoveIndicator_Actor(const AActor* OwnerActor)
{
	GET_INDICATOR_MANAGER_OR_RETURN(OwnerActor);

	IndicatorManager->RemoveIndicators(OwnerActor);
}

void UHUDIndicatorBlueprintLibrary::RemoveIndicator_Component(const USceneComponent* Component)
{
	GET_INDICATOR_MANAGER_OR_RETURN(Component);

	IndicatorManager->RemoveIndicators(Component);
}

#undef GET_INDICATOR_MANAGER_OR_RETURN