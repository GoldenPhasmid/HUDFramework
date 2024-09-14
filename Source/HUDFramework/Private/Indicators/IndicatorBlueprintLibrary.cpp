#include "Indicators/IndicatorBlueprintLibrary.h"

#include "HUDFramework.h"
#include "Indicators/IndicatorManagerComponent.h"
#include "Indicators/IndicatorDescriptor.h"

#define GET_INDICATOR_MANAGER_OR_RETURN(WorldContext) \
	UIndicatorManagerComponent* IndicatorManager = UIndicatorManagerComponent::Get(WorldContext); \
	if (!IsValid(IndicatorManager)) \
	{ \
		UE_LOG(LogIndicators, Error, TEXT("%s: Failed to find indicator manager for context %s"), *FString(__FUNCTION__), *GetNameSafe(WorldContext)); \
		return; \
	}

void UIndicatorBlueprintLibrary::AddIndicator_Actor(const UIndicatorDescriptor* Descriptor, const AActor* OwnerActor)
{
	GET_INDICATOR_MANAGER_OR_RETURN(OwnerActor);

	IndicatorManager->AddIndicator(Descriptor, OwnerActor);
}

void UIndicatorBlueprintLibrary::AddIndicator_Component(const UIndicatorDescriptor* Descriptor, const USceneComponent* Component, FName SocketName)
{
	GET_INDICATOR_MANAGER_OR_RETURN(Component);

	IndicatorManager->AddIndicator(Descriptor, Component, SocketName);
}

void UIndicatorBlueprintLibrary::AddIndicator_ActorWithContext(const UIndicatorDescriptor* Descriptor, const AActor* OwnerActor, UObject* ContextObject)
{
	GET_INDICATOR_MANAGER_OR_RETURN(OwnerActor);

	IndicatorManager->AddIndicatorWithContext(Descriptor, OwnerActor, FHUDWidgetContextHandle::CreateContext<FIndicatorWidgetContext>(ContextObject, Descriptor));
}

void UIndicatorBlueprintLibrary::AddIndicator_ComponentWithContext(const UIndicatorDescriptor* Descriptor, const USceneComponent* Component, FName SocketName, UObject* ContextObject)
{
	GET_INDICATOR_MANAGER_OR_RETURN(Component);

	IndicatorManager->AddIndicatorWithContext(Descriptor, Component, SocketName, FHUDWidgetContextHandle::CreateContext<FIndicatorWidgetContext>(ContextObject, Descriptor));
}

void UIndicatorBlueprintLibrary::RemoveIndicator_Actor(const AActor* OwnerActor)
{
	GET_INDICATOR_MANAGER_OR_RETURN(OwnerActor);

	IndicatorManager->RemoveIndicators(OwnerActor);
}

void UIndicatorBlueprintLibrary::RemoveIndicator_Component(const USceneComponent* Component)
{
	GET_INDICATOR_MANAGER_OR_RETURN(Component);

	IndicatorManager->RemoveIndicators(Component);
}

#undef GET_INDICATOR_MANAGER_OR_RETURN