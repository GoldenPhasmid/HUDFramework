#pragma once

#include "HUDWidgetContext.h"
#include "Components/GameStateComponent.h"
#include "HUDIndicatorManagerComponent.generated.h"

struct FHUDWidgetContextHandle;
class UHUDIndicatorDescriptor;

USTRUCT(BlueprintType)
struct HUDFRAMEWORK_API FIndicatorDescriptorInstance
{
	GENERATED_BODY()

	FIndicatorDescriptorInstance() = default;
	FIndicatorDescriptorInstance(const UHUDIndicatorDescriptor* InDescriptor, const USceneComponent* InComponent, FName InSocketName, const FHUDWidgetContextHandle& InWidgetContext)
		: Descriptor(InDescriptor)
		, Component(InComponent)
		, SocketName(InSocketName)
		, WidgetContext(InWidgetContext)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<const UHUDIndicatorDescriptor> Descriptor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<const USceneComponent> Component;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FHUDWidgetContextHandle WidgetContext;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FIndicatorDelegate, const TSharedRef<FIndicatorDescriptorInstance>&);

UCLASS(BlueprintType)
class HUDFRAMEWORK_API UHUDIndicatorManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()
	
public:
	UHUDIndicatorManagerComponent(const FObjectInitializer& Initializer);

	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	
	// ~Begin ActorComponent Interface
	virtual void OnUnregister() override;
	// ~End ActorComponent Interface

	UFUNCTION(BlueprintCallable, Category = "Components", DisplayName = "Get Indicator Manager Component", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static UHUDIndicatorManagerComponent* Get(const UObject* WorldContextObject);

	FIndicatorDelegate OnIndicatorAdded;
	FIndicatorDelegate OnIndicatorRemoved;
	
	void AddIndicator(const UHUDIndicatorDescriptor* Descriptor, const AActor* OwnerActor);
	void AddIndicator(const UHUDIndicatorDescriptor* Descriptor, const USceneComponent* Component, FName SocketName = NAME_None);
	
	void AddIndicatorWithContext(const UHUDIndicatorDescriptor* Descriptor, const AActor* OwnerActor, const FHUDWidgetContextHandle& WidgetContext);
	void AddIndicatorWithContext(const UHUDIndicatorDescriptor* Descriptor, const USceneComponent* Component, FName SocketName, const FHUDWidgetContextHandle& WidgetContext);

	void RemoveIndicators(const AActor* OwnerActor);
	void RemoveIndicators(const USceneComponent* Component);
	
	FORCEINLINE const TSet<TSharedPtr<FIndicatorDescriptorInstance>>& GetIndicators() const
	{
		return IndicatorInstances;
	}

protected:

	void AddIndicatorInternal(TSharedPtr<FIndicatorDescriptorInstance> Instance);
	
	TSet<TSharedPtr<FIndicatorDescriptorInstance>> IndicatorInstances;
};
