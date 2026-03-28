#pragma once

#include "CoreMinimal.h"
#include "HUDWidgetContext.h"
#include "HUDIndicatorDescriptor.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "HUDIndicatorBlueprintLibrary.generated.h"

class AActor;
class USceneComponent;
class UHUDIndicatorDescriptor;

USTRUCT(BlueprintType)
struct FIndicatorWidgetContext: public FHUDWidgetContextBase
{
	GENERATED_BODY()

	FIndicatorWidgetContext() = default;
	FIndicatorWidgetContext(UObject* InContextObject, const UHUDIndicatorDescriptor* InDescriptor)
		: ContextObject(InContextObject)
		, Descriptor(InDescriptor)
	{}
	
	UPROPERTY(BlueprintReadWrite)
	UObject* ContextObject = nullptr;

	UPROPERTY(BlueprintReadWrite)
	const UHUDIndicatorDescriptor* Descriptor = nullptr;
};

UCLASS()
class HUDFRAMEWORK_API UHUDIndicatorBlueprintLibrary: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
protected:

	UFUNCTION(BlueprintCallable, DisplayName = "Add Indicator (Actor)")
	static void AddIndicator_Actor(const UHUDIndicatorDescriptor* Descriptor, const AActor* OwnerActor);

	UFUNCTION(BlueprintCallable, DisplayName = "Add Indicator (Component")
	static void AddIndicator_Component(const UHUDIndicatorDescriptor* Descriptor, const USceneComponent* Component, FName SocketName = NAME_None);

	UFUNCTION(BlueprintCallable, DisplayName = "Add Indicator With Context (Actor)")
	static void AddIndicator_ActorWithContext(const UHUDIndicatorDescriptor* Descriptor, const AActor* OwnerActor, UObject* ContextObject = nullptr);

	UFUNCTION(BlueprintCallable, DisplayName = "Add Indicator With Context (Component)")
	static void AddIndicator_ComponentWithContext(const UHUDIndicatorDescriptor* Descriptor, const USceneComponent* Component, FName SocketName = NAME_None, UObject* ContextObject = nullptr);

	UFUNCTION(BlueprintCallable, DisplayName = "Remove Indicator By Actor")
	static void RemoveIndicator_Actor(const AActor* OwnerActor);

	UFUNCTION(BlueprintCallable, DisplayName = "Remove Indicator By Component")
	static void RemoveIndicator_Component(const USceneComponent* Component);
};


