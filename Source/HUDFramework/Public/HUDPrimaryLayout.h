#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "HUDWidgetContext.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include "HUDPrimaryLayout.generated.h"

struct FHUDWidgetContextHandle;
class UCommonActivatableWidgetContainerBase;

/**
 * Primary layout (root widget) of the whole game.
 * Represents and manages a number of HUD layers which user code pushes content to. Each layer should contain some sort
 * of HUD layout (see @UHUDLayout) or represent simple push/pop functionality.
 * Users can extend HUD layout with HUD extensions using HUD slots (see @UHUDLayoutSubsystem)
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class HUDFRAMEWORK_API UHUDPrimaryLayout: public UCommonUserWidget
{
	GENERATED_BODY()
public:

	// @todo: PushUniqueWidgetToLayer - does nothing if widget of class is already present in the container, instead brings it up to the front
	// @todo: PushUniqueWidgetToLayerAsync - does nothing if widget of class is already present in the container, instead brings it up to the front
	
	/**
	 * Adds activatable widget of @WidgetClass to the layer referenced by @LayerTag. Performs async load for given @WidgetClass
	 */
	template <typename TActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerAsync(FGameplayTag LayerTag, TSoftClassPtr<TActivatableWidget> WidgetClass)
	{
		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		TSharedPtr<FStreamableHandle> StreamableHandle = StreamableManager.RequestAsyncLoad(WidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
			[this, LayerTag, WidgetClass]()
		{
			PushWidgetToLayer<TActivatableWidget>(LayerTag, WidgetClass.Get());
		}));

		return StreamableHandle;
	}

	/**
	 * Adds activatable widget of @WidgetClass to the layer referenced by @LayerTag. Performs async load for given @WidgetClass
	 */
	template <typename TActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerAsync(FGameplayTag LayerTag, TSoftClassPtr<TActivatableWidget> WidgetClass, const FHUDWidgetContextHandle& WidgetContext)
	{
		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		TSharedPtr<FStreamableHandle> StreamableHandle = StreamableManager.RequestAsyncLoad(WidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
			[this, LayerTag, WidgetClass, WidgetContext]()
		{
			PushWidgetToLayer<TActivatableWidget>(LayerTag, WidgetClass.Get(), WidgetContext);
		}));

		return StreamableHandle;
	}
	
	/**
	 * Adds activatable widget of @WidgetClass to the layer referenced by @LayerTag. Performs async load for given @WidgetClass
	 * Calls @InitFunc with valid widget if load succeeded and widget created, calls with nullptr if async load was cancelled
	 */
	template <typename TActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerAsync(FGameplayTag LayerTag, TSoftClassPtr<TActivatableWidget> WidgetClass, TFunction<void(TActivatableWidget*)> InitFunc)
	{
		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		TSharedPtr<FStreamableHandle> StreamableHandle = StreamableManager.RequestAsyncLoad(WidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
			[this, LayerTag, WidgetClass, InitFunc]()
		{
			PushWidgetToLayer<TActivatableWidget>(LayerTag, WidgetClass.Get(), [InitFunc](TActivatableWidget& Widget) { InitFunc(&Widget); });
		}));

		StreamableHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this, [this, InitFunc]()
		{
			InitFunc(nullptr);
		}));

		return StreamableHandle;
	}

	/** Adds activatable widget of @WidgetClass to the layer referenced by @LayerTag */
	template <typename TActivatableWidget>
	TActivatableWidget* PushWidgetToLayer(FGameplayTag LayerTag, TSubclassOf<TActivatableWidget> WidgetClass)
	{
		check(bAddWidgetGuard == false);
		if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerTag))
		{
			TGuardValue Guard{bAddWidgetGuard, true};
			return Layer->AddWidget<TActivatableWidget>(WidgetClass);
		}

		return nullptr;
	}

	/** Adds activatable widget of @WidgetClass to the layer referenced by @LayerTag. Overload that calls @InitFunc after creating a widget */
	template <typename TActivatableWidget>
	TActivatableWidget* PushWidgetToLayer(FGameplayTag LayerTag, TSubclassOf<TActivatableWidget> WidgetClass, TFunction<void(TActivatableWidget&)> InitFunc)
	{
		check(bAddWidgetGuard == false);
		if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerTag))
		{
			TGuardValue Guard{bAddWidgetGuard, true};
			return Layer->AddWidget<TActivatableWidget>(WidgetClass, InitFunc);
		}
		
		return nullptr;
	}

	template <typename TActivatableWidget>
	TActivatableWidget* PushWidgetToLayer(FGameplayTag LayerTag, TSubclassOf<TActivatableWidget> WidgetClass, const FHUDWidgetContextHandle& WidgetContext)
	{
		check(bAddWidgetGuard == false);
		if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerTag))
		{
			TGuardValue Guard{bAddWidgetGuard, true};
			ActiveContext = WidgetContext;
			
			TActivatableWidget* Widget = Layer->AddWidget<TActivatableWidget>(WidgetClass);
			// activatable widget should 'consume' widget context
			check(!ActiveContext.IsValid());

			return Widget;
		}
		
		return nullptr;
	}

	/** finds if widget exists on any layers and removes it from residing layer */
	void PopWidget(UCommonActivatableWidget* Widget);

	/** Removes widget from residing layer */
	void PopWidgetFromLayer(FGameplayTag LayerTag, UCommonActivatableWidget* Widget);
	
	/** @return layer widget for given @LayerTag */
	UFUNCTION(BlueprintPure, Category = "Layer")
	UCommonActivatableWidgetContainerBase* GetLayerWidget(UPARAM(meta = (Categories = "HUD.Layer")) FGameplayTag LayerTag) const;

	/** Register layer for this primary layout */
	UFUNCTION(BlueprintCallable, Category = "Layer")
	void RegisterLayer(UPARAM(meta = (Categories = "HUD.Layer")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget);

	UFUNCTION(BlueprintCallable, Category = "Layer")
	void UnregisterLayer(UPARAM(meta = (Categories = "HUD.Layer")) FGameplayTag LayerTag);

	void InitActivatableWidget(UCommonActivatableWidget& NewWidget);
	
protected:

	/** Context for a widget currently being added to the layer */
	FHUDWidgetContextHandle ActiveContext;
	bool bAddWidgetGuard;

	/** Registered layers for primary layout */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> ActiveLayers;
};
