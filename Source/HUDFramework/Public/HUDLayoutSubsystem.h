#pragma once

#include "CoreMinimal.h"
#include "HUDLayoutExtension.h"
#include "HUDLayoutSlot.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "HUDLayoutSubsystem.generated.h"

struct FHUDWidgetContextHandle;
struct FHUDLayoutExtensionRequest;
class FHUDLayoutExtension;
class FHUDLayoutSlot;
struct FHUDLayoutExtensionHandle;
struct FHUDLayoutSlotHandle;
class UHUDLayoutPolicy;

DECLARE_MULTICAST_DELEGATE_TwoParams(FSlotExtensionDelegate, UUserWidget* /** ExtensionWidget */, const FHUDLayoutExtension& /** LayoutExtension */);

UCLASS(Config = Game)
class HUDFRAMEWORK_API UHUDLayoutSubsystem: public UGameInstanceSubsystem
{
	GENERATED_BODY()

	using TSlotCallback = typename FHUDLayoutSlot::TSlotCallback;
public:

	static UHUDLayoutSubsystem* Get(const UObject* WorldContextObject);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

	UHUDLayoutPolicy* GetPolicy() const { return Policy; }

	/**
	 * Register layout slot using slot tag
	 * @SlotTag tag that describes the slot
	 * @AddCallback callback for adding extensions
	 * @RemoveCallback callback for removing extensions
	 */
	FHUDLayoutSlotHandle RegisterLayoutSlot(const FGameplayTag& SlotTag, const ULocalPlayer* LocalPlayer, TSlotCallback AddCallback, TSlotCallback RemoveCallback);
	
	/** Unregister layout slot by handle. Handle is invalidated */
	void UnregisterLayoutSlot(FHUDLayoutSlotHandle& Handle);

	/**
	 * Register layout extension using slot tag
	 * @SlotTag slot tag that indicates which slot to extend
	 * @WidgetClass widget to create
	 * @DataPayload additional payload data
	 */
	FHUDLayoutExtensionHandle RegisterLayoutExtension(const FGameplayTag& SlotTag, TSubclassOf<UUserWidget> WidgetClass, const ULocalPlayer* LocalPlayer);

	/**
	 * Register
	 * @SlotTag slot tag that indicates which slot to extend
	 * @WidgetClass widget to create
	 * @DataPayload additional payload data
	 * @Context Additional filtering for slots. Slots and extensions match only if their tags and contexts match
	 */
	FHUDLayoutExtensionHandle RegisterLayoutExtensionWithContext(const FGameplayTag& SlotTag, TSubclassOf<UUserWidget> WidgetClass, const ULocalPlayer* LocalPlayer, const FHUDWidgetContextHandle& Context);

	/** Unregister layout extension by handle. Handle is invalidated */
	void UnregisterLayoutExtension(FHUDLayoutExtensionHandle& Handle);

	FSlotExtensionDelegate OnExtensionAdded;
	FSlotExtensionDelegate OnExtensionRemoved;

protected:
	
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, DisplayName = "Register Layout Extension (Widget)")
	FHUDLayoutExtensionHandle K2_RegisterLayoutExtension(const APlayerController* Player, const FGameplayTag& SlotTag, TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, DisplayName = "Register Layout Extension (Widget With Context)")
	FHUDLayoutExtensionHandle K2_RegisterLayoutExtensionWithContext(const APlayerController* Player, const FGameplayTag& SlotTag, TSubclassOf<UUserWidget> WidgetClass, const FHUDWidgetContextHandle& Context);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, DisplayName = "Unregister Layout Extension")
	void K2_UnregisterLayoutExtension(UPARAM(ref) FHUDLayoutExtensionHandle& Handle);
	
	// @todo: RegisterLayoutSlot with blueprint delegates?
	// @todo: node to create custom widget context?
	
	virtual void OnLocalPlayerAdded(ULocalPlayer* LocalPlayer);
	virtual void OnLocalPlayerRemoved(ULocalPlayer* LocalPlayer);

	virtual void NotifySlotAdded(TSharedPtr<FHUDLayoutSlot> Slot);
	virtual void NotifySlotRemoved(TSharedPtr<FHUDLayoutSlot> Slot);

	void UpdateSlotExtensions(TSharedPtr<FHUDLayoutSlot> Slot, TSlotCallback& CallbackRef, FSlotExtensionDelegate& ExtensionDelegate);
	
	void ForEachSlot(FGameplayTag SlotTag, TFunctionRef<void( TSharedPtr<FHUDLayoutSlot> )> Func);
	void ForEachExtension(FGameplayTag SlotTag, TFunctionRef<void( TSharedPtr<FHUDLayoutExtension> )> Func);
	
	virtual void NotifyExtensionAdded(TSharedPtr<FHUDLayoutExtension> Extension);
	virtual void NotifyExtensionRemoved(TSharedPtr<FHUDLayoutExtension> Extension);

	static FHUDLayoutExtensionRequest CreateRequest(const FHUDLayoutExtensionHandle& Handle);

	UPROPERTY(Transient)
	TObjectPtr<UHUDLayoutPolicy> Policy = nullptr;
	
	using FLayoutSlotArray = TArray<TSharedPtr<FHUDLayoutSlot>>;
	TMap<FGameplayTag, FLayoutSlotArray> ActiveSlots;
	
	using FLayoutExtensionArray = TArray<TSharedPtr<FHUDLayoutExtension>>;
	TMap<FGameplayTag, FLayoutExtensionArray> ActiveExtensions;
};
