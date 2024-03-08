#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "IndicatorBlueprintLibrary.generated.h"

class AActor;
class USceneComponent;
class UIndicatorDescriptor;

UCLASS()
class HUDFRAMEWORK_API UIndicatorBlueprintLibrary: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
protected:

	UFUNCTION(BlueprintCallable, DisplayName = "Add Indicator (Actor)")
	static void AddIndicator_Actor(const UIndicatorDescriptor* Descriptor, const AActor* OwnerActor);

	UFUNCTION(BlueprintCallable, DisplayName = "Add Indicator (Component")
	static void AddIndicator_Component(const UIndicatorDescriptor* Descriptor, const USceneComponent* Component, FName SocketName = NAME_None);

	UFUNCTION(BlueprintCallable, DisplayName = "Add Indicator With Context (Actor)")
	static void AddIndicator_ActorWithContext(const UIndicatorDescriptor* Descriptor, const AActor* OwnerActor, UObject* ContextObject = nullptr);

	UFUNCTION(BlueprintCallable, DisplayName = "Add Indicator With Context (Component)")
	static void AddIndicator_ComponentWithContext(const UIndicatorDescriptor* Descriptor, const USceneComponent* Component, FName SocketName = NAME_None, UObject* ContextObject = nullptr);

	UFUNCTION(BlueprintCallable, DisplayName = "Remove Indicator By Actor")
	static void RemoveIndicator_Actor(const AActor* OwnerActor);

	UFUNCTION(BlueprintCallable, DisplayName = "Remove Indicator By Component")
	static void RemoveIndicator_Component(const USceneComponent* Component);
};


