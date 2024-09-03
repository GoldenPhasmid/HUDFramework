#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameExperienceActionBase.h"

#include "GameFeatureAction_AddHUDLayout.generated.h"


struct FHUDExtensionData;
class UCommonActivatableWidget;

USTRUCT()
struct FHUDLayoutEntry
{
	GENERATED_BODY()

	/** layout widget to spawn */
	UPROPERTY(EditAnywhere, meta = (Validate, AssetBundles = "Client"))
	TSoftClassPtr<UCommonActivatableWidget> LayoutClass;

	/** layer to push layout widget */
	UPROPERTY(EditAnywhere, meta = (Validate, Categories = "HUD.Layer"))
	FGameplayTag LayerTag;
};

USTRUCT()
struct FHUDLayoutExtensionEntry
{
	GENERATED_BODY()

	/** extension widget to spawn */
	UPROPERTY(EditAnywhere, meta = (Validate, AssetBundles = "Client"))
	TSoftClassPtr<UUserWidget> ExtensionWidgetClass;

	/** slot to connect extension with */
	UPROPERTY(EditAnywhere, meta = (Validate))
	FGameplayTag SlotTag;
};

UCLASS(meta = (DisplayName = "Add HUD Layout"))
class HUDFRAMEWORKGAMEFEATUREACTIONS_API UGameFeatureAction_AddHUDLayout: public UGameExperienceActionBase
{
	GENERATED_BODY()
public:

	//~Begin GameFeatureAction_WorldActionBase interface
	virtual TSharedPtr<FGameFeatureContextData> CreateContextData() const override;
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	virtual void RemoveFromWorld(const FGameFeatureStateChangeContext& ChangeContext) override;
#if WITH_EDITOR
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override;
#endif
	//~End GameFeatureAction_WorldActionBase interface
	
protected:
	void HandleActorExtension(AActor* Actor, FName Event, FGameFeatureStateChangeContext Context);
	void AddWidgets(const APlayerController* PlayerController, FHUDExtensionData& ExtensionData);
	void RemoveWidgets(const APlayerController* PlayerController, FHUDExtensionData& ExtensionData);

	UPROPERTY(EditAnywhere, meta = (TitleProperty = "{LayoutClass} -> {LayerTag}"))
	TArray<FHUDLayoutEntry> Layouts;

	UPROPERTY(EditAnywhere, meta = (TitleProperty = "{ExtensionWidgetClass} -> {SlotTag}"))
	TArray<FHUDLayoutExtensionEntry> Extensions;
};