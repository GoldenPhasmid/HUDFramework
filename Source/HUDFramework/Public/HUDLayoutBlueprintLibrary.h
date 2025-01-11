#pragma once

#include "CoreMinimal.h"
#include "HUDLayoutExtension.h"
#include "HUDWidgetContext.h"
#include "Components/Widget.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "HUDLayoutBlueprintLibrary.generated.h"

struct FGameplayTag;
class UUserWidget;
class UHUDPrimaryLayout;
class UCommonActivatableWidget;
struct FHUDWidgetContextHandle;
struct FHUDLayoutExtensionHandle;
struct FHUDLayoutSlotHandle;

// @todo: rename to HUDFrameworkBlueprintLibrary
UCLASS()
class HUDFRAMEWORK_API UHUDLayoutBlueprintLibrary: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/** @return widget tree description starting from the given widget up to the tree root. Description contains user widgets only */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "HUD")
	static FString ConstructWidgetTreeString(const UUserWidget* UserWidget);

	/** @return primary layout for a given player */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "HUD")
	static UHUDPrimaryLayout* GetPrimaryLayout(const APlayerController* PlayerController);

	/** Push widget to the layer for a given player */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD")
	static UCommonActivatableWidget* PushContentToLayer(const APlayerController* PlayerController, UPARAM(meta = (GameplayTagFilter = "HUD.Layer")) FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass, FHUDWidgetContextHandle WidgetContext);

	/** Push widget async to the layer for a given player */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD")
	static void PushStreamedContentToLayer(const APlayerController* PlayerController, UPARAM(meta = (GameplayTagFilter = "HUD.Layer")) FGameplayTag LayerTag, TSoftClassPtr<UCommonActivatableWidget> WidgetClass, FHUDWidgetContextHandle WidgetContext);

	/** Remove widget from the layer for a given player */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD")
	static void PopContentFromLayer(const APlayerController* PlayerController, UPARAM(meta = (GameplayTagFilter = "HUD.Layer")) FGameplayTag LayerTag, UCommonActivatableWidget* Widget);

	/** Remove widget from the primary layout */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD", meta = (DefaultToSelf = "Widget"))
	static void PopWidgetFromLayer(UCommonActivatableWidget* Widget);
	
	/**
	 * initialize widget with a given widget context
	 * will fail if widget has already been constructed
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD")
	static void InitializeWidgetFromContextHandle(UPARAM(Required) UUserWidget* UserWidget, const FHUDWidgetContextHandle& ContextHandle);

	/** 
	 * Register layout extension widget using @SlotTag to identify layout slots.
	 * Registered extension is automatically added to the newly added slots
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD")
	static FHUDLayoutExtensionHandle RegisterLayoutExtension(UPARAM(Required) const APlayerController* Player, UPARAM(meta = (GameplayTagFilter = "HUD.Slot")) FGameplayTag SlotTag, TSubclassOf<UUserWidget> WidgetClass);

	/** 
	 * Register layout extension widget using @SlotTag to identify layout slots. @WidgetContext is passed to the widget during initialization
	 * Registered extension is automatically added to the newly added slots
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD")
	static FHUDLayoutExtensionHandle RegisterLayoutExtensionWithContext(UPARAM(Required) const APlayerController* Player, UPARAM(meta = (GameplayTagFilter = "HUD.Slot")) FGameplayTag SlotTag, TSubclassOf<UUserWidget> WidgetClass, const FHUDWidgetContextHandle& WidgetContext);

	/** Unregister layout extension identified via a valid handle. Does nothing if handle is no longer valid */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD")
	static void UnregisterLayoutExtension(UPARAM(ref) FHUDLayoutExtensionHandle& Handle);

	/** @return widget context from an initialized user widget */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "HUD")
	static FHUDWidgetContextHandle GetWidgetContextHandle(const UUserWidget* UserWidget);

	// custom blueprint graph nodes
	
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "HUD", DisplayName = "Initialize Widget From Context", meta = (CustomStructureParam = "WidgetContext", BlueprintInternalUseOnly = "true"))
	static void K2_InitializeWidget(UPARAM(Required) UUserWidget* UserWidget, const int32& WidgetContext);

	UFUNCTION(BlueprintPure, CustomThunk, Category = "HUD", DisplayName = "Get Widget Context", meta = (CustomStructureParam = "WidgetContext", BlueprintInternalUseOnly = "true"))
	static void K2_GetWidgetContext(UPARAM(Required) UUserWidget* UserWidget, int32& WidgetContext);
	
	UFUNCTION(BlueprintPure, CustomThunk, Category = "HUD", DisplayName = "To Widget Context (Handle)", meta = (CustomStructureParam = "WidgetContext", BlueprintInternalUseOnly = "true"))
	static void K2_GetWidgetContextFromHandle(const FHUDWidgetContextHandle& ContextHandle, int32& WidgetContext);

	UFUNCTION(BlueprintPure, CustomThunk, Category = "HUD", DisplayName = "To Handle (Widget Context)", meta = (BlueprintAutocast, CustomStructureParam = "WidgetContext", BlueprintInternalUseOnly = "true"))
	static FHUDWidgetContextHandle Conv_WidgetContextToWidgetContextHandle(const int32& WidgetContext);
	
protected:
	
	DECLARE_FUNCTION(execK2_InitializeWidget);
	DECLARE_FUNCTION(execK2_GetWidgetContext);
	DECLARE_FUNCTION(execK2_GetWidgetContextFromHandle);
	DECLARE_FUNCTION(execConv_WidgetContextToWidgetContextHandle);
	
	// internal functions, blueprint usage only

	UFUNCTION(BlueprintPure, DisplayName = "Is Valid (Widget Context)")
	static bool IsValid_WidgetContext(const FHUDWidgetContextHandle& WidgetContext);

	UFUNCTION(BlueprintPure, DisplayName = "Is Derived From (Widget Context)")
	static bool IsDerivedFrom_WidgetContext(const FHUDWidgetContextHandle& WidgetContext, UScriptStruct* ContextType);
	
	/** @return whether given layout slot handle is valid */
	UFUNCTION(BlueprintPure, DisplayName = "Is Valid (Slot Handle)")
	static bool IsValid_SlotHandle(UPARAM(ref) FHUDLayoutSlotHandle& Handle);

	/** @return unregisters given layout slot from layout subsystem */
	UFUNCTION(BlueprintCallable, DisplayName = "Unregister (Slot Handle)")
	static void Unregister_SlotHandle(UPARAM(ref) FHUDLayoutSlotHandle& Handle);

	/** @return whether given layout extension handle is valid */
	UFUNCTION(BlueprintPure, DisplayName = "Is Valid (Extension Handle)")
	static bool IsValid_ExtensionHandle(UPARAM(ref) FHUDLayoutExtensionHandle& Handle);

	/** @return unregisters given layout extension from layout subsystem */
	UFUNCTION(BlueprintCallable, DisplayName = "Unregister (Extension Handle)", meta = (DeprecatedFunction, DeprecationMessage = "Use UnregisterLayoutExtension instead."))
	static void Unregister_ExtensionHandle(UPARAM(ref) FHUDLayoutExtensionHandle& Handle);

	UE_DEPRECATED(5.4, "")
	UFUNCTION(BlueprintPure, DisplayName = "Get Context Object (Widget Context)", meta = (DeprecatedFunction, DeprecationMessage = "Use HUDWidgetContext explicitly."))
	static UObject* GetContextObject_WidgetContext(const FHUDWidgetContextHandle& WidgetContext);
	
	UE_DEPRECATED(5.4, "")
	UFUNCTION(BlueprintPure, DisplayName = "Get Data Object (Widget Context)", meta = (DeprecatedFunction, DeprecationMessage = "Use HUDWidgetContext explicitly."))
	static const UObject* GetDataObject_WidgetContext(const FHUDWidgetContextHandle& WidgetContext);

	UE_DEPRECATED(5.4, "")
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD", meta = (DeprecatedFunction, DeprecationMessage = "Use InitializeWidgetFromContext instead."))
	static void InitializeWidget(UPARAM(Required) UUserWidget* UserWidget, UObject* Context, const UObject* DataPayload);
	
	UE_DEPRECATED(5.4, "")
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD", meta = (DeprecatedFunction, DeprecationMessage = "Use InitializeWidgetFromContext instead."))
	static void SetWidgetContext(UPARAM(Required) UUserWidget* UserWidget, UObject* Context, const UObject* DataPayload);
	
	UE_DEPRECATED(5.4, "")
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD", DisplayName = "Set Widget Context (Handle)", meta = (DeprecatedFunction, DeprecationMessage = "Use InitializeWidgetFromHandle instead."))
	static void SetWidgetContext_FromHandle(UPARAM(Required) UUserWidget* UserWidget, const FHUDWidgetContextHandle& ContextHandle);

	UE_DEPRECATED(5.4, "")
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD", meta = (DeprecatedFunction, DeprecationMessage = "Use InitializeWidgetFromContextHandle instead."))
	static void InitializeWidget_FromHandle(UPARAM(Required) UUserWidget* UserWidget, const FHUDWidgetContextHandle& ContextHandle);

	UE_DEPRECATED(5.4, "")
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD", meta = (DeprecatedFunction, DeprecationMessage = "Use InitializeWidgetFromContextHandle instead."))
	static void InitializeWidgetFromHandle(UPARAM(Required) UUserWidget* UserWidget, const FHUDWidgetContextHandle& ContextHandle);

	
};

template <typename TPred>
const UWidget* ForEachParentWidget(const UWidget* CurrentWidget, TPred&& Pred)
{
	if (Pred(CurrentWidget))
	{
		return CurrentWidget;
	}
	
	while (CurrentWidget && CurrentWidget->GetOuter()->IsA<UWidgetTree>())
	{
		CurrentWidget = Cast<UWidget>(CurrentWidget->GetOuter()->GetOuter());
		if (Pred(CurrentWidget))
		{
			return CurrentWidget;
		}
	}

	return nullptr;
}
