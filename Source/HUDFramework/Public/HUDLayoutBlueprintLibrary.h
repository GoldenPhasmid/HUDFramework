#pragma once

#include "CoreMinimal.h"
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

	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "HUD")
	static FString ConstructWidgetTreeString(const UUserWidget* UserWidget);

	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "HUD")
	static UHUDPrimaryLayout* GetPrimaryLayout(const APlayerController* PlayerController);

	/** Push widget to the layer for given player */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD")
	static UCommonActivatableWidget* PushContentToLayer(const APlayerController* PlayerController, UPARAM(meta = (Categories = "HUD.Layer")) FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass);

	/** Push widget async to the layer for given player */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD")
	static void PushStreamedContentToLayer(const APlayerController* PlayerController, UPARAM(meta = (Categories = "HUD.Layer")) FGameplayTag LayerTag, TSoftClassPtr<UCommonActivatableWidget> WidgetClass);

	/** Remove widget from the layer for given player */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "HUD")
	static void PopContentFromLayer(const APlayerController* PlayerController, UPARAM(meta = (Categories = "HUD.Layer")) FGameplayTag LayerTag, UCommonActivatableWidget* Widget);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, DisplayName = "Set Widget Context")
	static void SetWidgetContext(UUserWidget* UserWidget, UObject* Context, const UObject* DataPayload);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, DisplayName = "Set Widget Context (Handle)")
	static void SetWidgetContext_FromHandle(UUserWidget* UserWidget, const FHUDWidgetContextHandle& ContextHandle);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, DisplayName = "Initialize Widget")
	static void InitializeWidget(UUserWidget* UserWidget, UObject* Context, const UObject* DataPayload);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, DisplayName = "Initialize Widget (Handle)")
	static void InitializeWidget_FromHandle(UUserWidget* UserWidget, const FHUDWidgetContextHandle& ContextHandle);

	UFUNCTION(BlueprintPure, BlueprintCosmetic, DisplayName = "Get Widget Context")
	static FHUDWidgetContextHandle GetWidgetContext(const UUserWidget* UserWidget);

	// @todo: node that creates custom widget context
	
protected:

	// internal functions, blueprint usage only

	UFUNCTION(BlueprintPure, DisplayName = "Is Valid (Widget Context)")
	static bool IsValid_WidgetContext(const FHUDWidgetContextHandle& WidgetContext);
	
	UFUNCTION(BlueprintPure, DisplayName = "Get Context Object (Widget Context)")
	static UObject* GetContextObject_WidgetContext(const FHUDWidgetContextHandle& WidgetContext);

	UFUNCTION(BlueprintPure, DisplayName = "Get Data Object (Widget Context)")
	static const UObject* GetDataObject_WidgetContext(const FHUDWidgetContextHandle& WidgetContext);
	
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
	UFUNCTION(BlueprintCallable, DisplayName = "Unregister (Extension Handle)")
	static void Unregister_ExtensionHandle(UPARAM(ref) FHUDLayoutExtensionHandle& Handle);
	
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
