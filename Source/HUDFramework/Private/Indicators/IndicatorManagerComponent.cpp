#include "Indicators/IndicatorManagerComponent.h"

#include "HUDFramework.h"
#include "Indicators/IndicatorDescriptor.h"

UIndicatorManagerComponent::UIndicatorManagerComponent(const FObjectInitializer& Initializer) : Super(Initializer)
{
}

void UIndicatorManagerComponent::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	UIndicatorManagerComponent* This = CastChecked<UIndicatorManagerComponent>(InThis);

	for (TSharedPtr<FIndicatorDescriptorInstance> Instance: This->IndicatorInstances)
	{
		if (Instance.IsValid())
		{
			Collector.AddReferencedObject(Instance->Descriptor, This);
		}
	}
}

void UIndicatorManagerComponent::OnUnregister()
{
	Super::OnUnregister();

	for (const TSharedPtr<FIndicatorDescriptorInstance>& Instance : IndicatorInstances)
	{
		OnIndicatorRemoved.Broadcast(Instance.ToSharedRef());
		// Dont remove indicators in range-based for
	}

	IndicatorInstances.Empty();

	OnIndicatorAdded.Clear();
	OnIndicatorRemoved.Clear();
}

UIndicatorManagerComponent* UIndicatorManagerComponent::Get(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}
	
	const UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	const AGameStateBase* GameState = World->GetGameState();
	return IsValid(GameState) ? GameState->FindComponentByClass<UIndicatorManagerComponent>(): nullptr;
}

void UIndicatorManagerComponent::AddIndicator(const UIndicatorDescriptor* Descriptor, const AActor* OwnerActor)
{
	// create context anyway with Descriptor as a data object
	AddIndicatorWithContext(Descriptor, OwnerActor, FHUDWidgetContextHandle::CreateContext<FHUDWidgetContext>(nullptr, Descriptor));
}

void UIndicatorManagerComponent::AddIndicator(const UIndicatorDescriptor* Descriptor, const USceneComponent* Component, FName SocketName)
{
	// create context anyway with Descriptor as a data object
	AddIndicatorWithContext(Descriptor, Component, SocketName, FHUDWidgetContextHandle::CreateContext<FHUDWidgetContext>(nullptr, Descriptor));
}

void UIndicatorManagerComponent::AddIndicatorWithContext(const UIndicatorDescriptor* Descriptor, const AActor* OwnerActor, const FHUDWidgetContextHandle& WidgetContext)
{
	if (!IsValid(OwnerActor) || Descriptor == nullptr)
	{
		UE_LOG(LogIndicators, Error, TEXT("%s: Failed to add indicator with [%s] descriptor for [%s] actor"), *FString(__FUNCTION__), *GetNameSafe(Descriptor), *GetNameSafe(OwnerActor));
		return;
	}

	const TSharedPtr<FIndicatorDescriptorInstance> NewInstance = MakeShared<FIndicatorDescriptorInstance>(Descriptor, OwnerActor->GetRootComponent(), NAME_None, WidgetContext);
	AddIndicatorInternal(NewInstance);
}


void UIndicatorManagerComponent::AddIndicatorWithContext(const UIndicatorDescriptor* Descriptor, const USceneComponent* Component, FName SocketName, const FHUDWidgetContextHandle& WidgetContext)
{
	if (!IsValid(Component) || Descriptor == nullptr)
	{
		UE_LOG(LogIndicators, Error, TEXT("%s: Failed to add indicator with [%s] descriptor for [%s] component"), *FString(__FUNCTION__), *GetNameSafe(Descriptor), *GetNameSafe(Component));
		return;
	}

	const TSharedPtr<FIndicatorDescriptorInstance> NewInstance = MakeShared<FIndicatorDescriptorInstance>(Descriptor, Component, SocketName, WidgetContext);
	AddIndicatorInternal(NewInstance);
}

void UIndicatorManagerComponent::AddIndicatorInternal(TSharedPtr<FIndicatorDescriptorInstance> Instance)
{
	check(!IndicatorInstances.Contains(Instance));
	
	IndicatorInstances.Add(Instance);
	OnIndicatorAdded.Broadcast(Instance.ToSharedRef());
}

void UIndicatorManagerComponent::RemoveIndicators(const AActor* OwnerActor)
{
	for (auto It = IndicatorInstances.CreateIterator(); It; ++It)
	{
		TSharedPtr<FIndicatorDescriptorInstance> Instance = *It;
		if (!IsValid(Instance->Component) || OwnerActor == Instance->Component->GetOwner())
		{
			It.RemoveCurrent();
			OnIndicatorRemoved.Broadcast(Instance.ToSharedRef());
		}
	}
}

void UIndicatorManagerComponent::RemoveIndicators(const USceneComponent* Component)
{
	for (auto It = IndicatorInstances.CreateIterator(); It; ++It)
    {
    	TSharedPtr<FIndicatorDescriptorInstance> Instance = *It;
    	if (!IsValid(Instance->Component) || Instance->Component == Component)
    	{
    		It.RemoveCurrent();
    		OnIndicatorRemoved.Broadcast(Instance.ToSharedRef());
    	}
    }
}
