#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/DynamicEntryBoxBase.h"
#include "HUDLayoutExtension.h"

#include "HUDLayoutSlotWidget.generated.h"

struct FHUDWidgetContextHandle;
struct FHUDLayoutSlotHandle;
struct FHUDLayoutExtensionRequest;

UCLASS()
class HUDFRAMEWORK_API UHUDLayoutSlotWidget: public UDynamicEntryBoxBase
{
	GENERATED_BODY()
	
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FConfigureWidget, UUserWidget*, Widget, const FHUDWidgetContextHandle&, WidgetContext);
public:

	UHUDLayoutSlotWidget(const FObjectInitializer& Initializer);

	//~Begin UWidget interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~End UWidget interface
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (Validate))
	FGameplayTag SlotTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (IsBindableEvent = "true"))
	FConfigureWidget ConfigureWidget;
	
protected:

	/** Register slot */
	void RegisterSlot();
	/** Register slot */
	void UnregisterSlot();

	/** Callback when extension is added to this layout slot */
	virtual UUserWidget* AddExtension(const FHUDLayoutExtensionRequest& Request);
	/** Callback when extension is removed to this layout slot */
	virtual UUserWidget* RemoveExtension(const FHUDLayoutExtensionRequest& Request);

	UPROPERTY(Transient)
	TArray<UUserWidget*> PendingWidgets;
	
	UPROPERTY(Transient)
	TArray<FHUDLayoutSlotHandle> Handles;
	
	UPROPERTY(Transient)
	TMap<FHUDLayoutExtensionHandle, TObjectPtr<UUserWidget>> ActiveExtensions;
};
