#include "GameFeatureAction_AddHUDLayout.h"

#include "CommonActivatableWidget.h"
#include "GameFeaturesSubsystemSettings.h"
#include "HUDLayoutBlueprintLibrary.h"
#include "HUDLayoutExtension.h"
#include "HUDLayoutSubsystem.h"
#include "HUDPrimaryLayout.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/HUD.h"

struct FHUDExtensionData
{
	TArray<TPair<UCommonActivatableWidget*, FGameplayTag>> LayoutWidgets;
	TArray<FHUDLayoutExtensionHandle> ExtensionHandles;
};

struct FHUDLayoutContextData: public FGameFeatureContextData
{
	TMap<AActor*, FHUDExtensionData> ActiveExtensions;
};

TSharedPtr<FGameFeatureContextData> UGameFeatureAction_AddHUDLayout::CreateContextData() const
{
	return MakeShared<FHUDLayoutContextData>();
}

void UGameFeatureAction_AddHUDLayout::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	Super::AddToWorld(WorldContext, ChangeContext);

	FPerContextData& ContextData = ActiveContexts.FindOrAdd(ChangeContext);
	check(ContextData.ExtensionRequests.IsEmpty());

	const UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	if (UGameFrameworkComponentManager* Manager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
	{
		auto Delegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension, ChangeContext);
		auto Handle = Manager->AddExtensionHandler(AHUD::StaticClass(), Delegate);
		ContextData.ExtensionRequests.Add(Handle);
	}
}

void UGameFeatureAction_AddHUDLayout::RemoveFromWorld(const FGameFeatureStateChangeContext& ChangeContext)
{
	FPerContextData& ContextData = ActiveContexts.FindChecked(ChangeContext);

	ContextData.ExtensionRequests.Empty();
	FHUDLayoutContextData& LayoutData = ContextData.GetContextData<FHUDLayoutContextData>();
	for (auto& [Actor, ActorExtensions]: LayoutData.ActiveExtensions)
	{
		const AHUD* HUD = CastChecked<AHUD>(Actor);
		RemoveWidgets(HUD->GetOwningPlayerController(), ActorExtensions);
	}
	LayoutData.ActiveExtensions.Empty();

	Super::RemoveFromWorld(ChangeContext);
}

#if WITH_EDITOR
void UGameFeatureAction_AddHUDLayout::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	for (const FHUDLayoutEntry& Layout: Layouts)
	{
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, Layout.LayoutClass.ToSoftObjectPath().GetAssetPath());
	}

	for (const FHUDLayoutExtensionEntry& Extension: Extensions)
	{
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, Extension.ExtensionWidgetClass.ToSoftObjectPath().GetAssetPath());
	}
}
#endif

void UGameFeatureAction_AddHUDLayout::HandleActorExtension(AActor* Actor, FName Event, FGameFeatureStateChangeContext Context)
{
	check(Actor);
	FHUDLayoutContextData& ContextData = ActiveContexts.FindChecked(Context).GetContextData<FHUDLayoutContextData>();

	const AHUD* HUD = CastChecked<AHUD>(Actor);
	APlayerController* PlayerController = HUD->GetOwningPlayerController();
	
	if (Event == UGameFrameworkComponentManager::NAME_ExtensionAdded || Event == UGameFrameworkComponentManager::NAME_GameActorReady)
	{
		FHUDExtensionData& ExtensionData = ContextData.ActiveExtensions.Add(Actor);
		AddWidgets(PlayerController, ExtensionData);
	}
	else if (Event == UGameFrameworkComponentManager::NAME_ReceiverRemoved || Event == UGameFrameworkComponentManager::NAME_ExtensionRemoved)
	{
		FHUDExtensionData& ExtensionData = ContextData.ActiveExtensions.FindChecked(Actor);
		RemoveWidgets(PlayerController, ExtensionData);
	}
}

void UGameFeatureAction_AddHUDLayout::AddWidgets(const APlayerController* PlayerController, FHUDExtensionData& ExtensionData)
{
	check(PlayerController);
	UHUDLayoutSubsystem* LayoutSubsystem = PlayerController->GetGameInstance()->GetSubsystem<UHUDLayoutSubsystem>();
	check(LayoutSubsystem);

	if (UHUDPrimaryLayout* PrimaryLayout = UHUDLayoutBlueprintLibrary::GetPrimaryLayout(PlayerController))
	{
		for (const FHUDLayoutEntry& Layout: Layouts)
		{
			check(Layout.LayoutClass.Get());
			ExtensionData.LayoutWidgets.Emplace(PrimaryLayout->PushWidgetToLayer<UCommonActivatableWidget>(
				Layout.LayerTag, Layout.LayoutClass.Get()), Layout.LayerTag
			);
		}
	}


	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	for (const FHUDLayoutExtensionEntry& Extension: Extensions)
	{
		check(Extension.ExtensionWidgetClass.Get());
		ExtensionData.ExtensionHandles.Add(LayoutSubsystem->RegisterLayoutExtension(
			Extension.SlotTag, Extension.ExtensionWidgetClass.Get(), LocalPlayer
		));
	}
}

void UGameFeatureAction_AddHUDLayout::RemoveWidgets(const APlayerController* PlayerController, FHUDExtensionData& ExtensionData)
{
	check(PlayerController);
	UHUDLayoutSubsystem* LayoutSubsystem = PlayerController->GetGameInstance()->GetSubsystem<UHUDLayoutSubsystem>();
	check(LayoutSubsystem);

	if (UHUDPrimaryLayout* PrimaryLayout = UHUDLayoutBlueprintLibrary::GetPrimaryLayout(PlayerController))
	{
		for (const auto& [Widget, LayerTag]: ExtensionData.LayoutWidgets)
		{
			PrimaryLayout->PopWidgetFromLayer(LayerTag, Widget);
		}
	}
	ExtensionData.LayoutWidgets.Empty();
	
	for (FHUDLayoutExtensionHandle& Handle: ExtensionData.ExtensionHandles)
	{
		LayoutSubsystem->UnregisterLayoutExtension(Handle);
	}
	ExtensionData.ExtensionHandles.Empty();
}

