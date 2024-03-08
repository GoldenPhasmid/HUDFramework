#include "HUDLayoutPolicy.h"

#include "HUDFramework.h"
#include "Blueprint/UserWidget.h"
#include "HUDPrimaryLayout.h"

void FHUDPrimaryLayoutInstance::AddToViewport()
{
	if (bAddedToViewport)
	{
		return;
	}

	check(LocalPlayer && PrimaryLayout);
	UE_LOG(LogHUDFramework, Log, TEXT("Adding primary layout [%s] to local player [%s]"), *GetNameSafe(PrimaryLayout), *GetNameSafe(LocalPlayer));

	PrimaryLayout->SetPlayerContext(FLocalPlayerContext{LocalPlayer});
	PrimaryLayout->AddToPlayerScreen(1000);
	
	bAddedToViewport = true;
}

void FHUDPrimaryLayoutInstance::RemoveFromViewport()
{
	if (!bAddedToViewport)
	{
		return;
	}
	
	TWeakPtr<SWidget> SlateWidget = PrimaryLayout->GetCachedWidget();
	if (SlateWidget.IsValid())
	{
		UE_LOG(LogHUDFramework, Log, TEXT("Removing primary layout [%s] from local player [%s]"), *GetNameSafe(PrimaryLayout), *GetNameSafe(LocalPlayer));
		PrimaryLayout->RemoveFromParent();

		if (SlateWidget.IsValid())
		{
			UE_LOG(LogHUDFramework, Fatal, TEXT("Widget memory leak"));
		}
	}
}

UHUDPrimaryLayout* UHUDLayoutPolicy::GetPrimaryLayout(const ULocalPlayer* LocalPlayer) const
{
	if (const FHUDPrimaryLayoutInstance* Instance = ActiveLayouts.FindByKey(LocalPlayer))
	{
		return Instance->PrimaryLayout;
	}

	return nullptr;
}

void UHUDLayoutPolicy::NotifyPlayerAdded(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer)
	{
		if (IsValid(LocalPlayer->PlayerController))
		{
			AddPrimaryLayout(LocalPlayer);
		}
		LocalPlayer->OnPlayerControllerChanged().AddUObject(this, &ThisClass::OnPlayerControllerChanged, LocalPlayer);
	}
}

void UHUDLayoutPolicy::NotifyPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer)
	{
		LocalPlayer->OnPlayerControllerChanged().RemoveAll(this);
		RemovePrimaryLayout(LocalPlayer);
	}
}

void UHUDLayoutPolicy::OnPlayerControllerChanged(APlayerController* PlayerController, ULocalPlayer* LocalPlayer)
{
	if (!IsValid(LocalPlayer))
	{
		return;
	}
	
	RemovePrimaryLayout(LocalPlayer);
	if (IsValid(PlayerController))
	{
		AddPrimaryLayout(PlayerController->GetLocalPlayer());
	}
}

UWorld* UHUDLayoutPolicy::GetWorld() const
{
	return GetOuter()->GetWorld();
}

void UHUDLayoutPolicy::AddPrimaryLayout(ULocalPlayer* LocalPlayer)
{
	if (FHUDPrimaryLayoutInstance* LayoutInstance = ActiveLayouts.FindByKey(LocalPlayer))
	{
		UE_LOG(LogHUDFramework, Warning, TEXT("%s: local player [%s] readded to layout policy."), *FString(__FUNCTION__), *GetNameSafe(LocalPlayer));
		LayoutInstance->AddToViewport();
	}
	else
	{
		if (UHUDPrimaryLayout* LayoutWidget = CreatePrimaryLayout(LocalPlayer))
		{
			FHUDPrimaryLayoutInstance& Instance = ActiveLayouts.AddDefaulted_GetRef();
			Instance.LocalPlayer = LocalPlayer;
			Instance.PrimaryLayout = LayoutWidget;

			Instance.AddToViewport();
			OnPrimaryLayoutAdded(LocalPlayer, LayoutWidget);
		}
		else
		{
			UE_LOG(LogHUDFramework, Error, TEXT("%s: failed to create primary layout for player %s"), *FString(__FUNCTION__), *GetNameSafe(LocalPlayer));	
		}
	}
}

void UHUDLayoutPolicy::RemovePrimaryLayout(ULocalPlayer* LocalPlayer)
{
	int32 LayoutIndex = ActiveLayouts.IndexOfByKey(LocalPlayer);
	if (LayoutIndex != INDEX_NONE)
	{
		ActiveLayouts[LayoutIndex].RemoveFromViewport();
		ActiveLayouts.RemoveAtSwap(LayoutIndex);
	}
	else
	{
		UE_LOG(LogHUDFramework, Log, TEXT("%s: local player [%s] missing primary layout instance, probably because of nullptr player controller"), *FString(__FUNCTION__), *GetNameSafe(LocalPlayer));
	}
}

void UHUDLayoutPolicy::OnPrimaryLayoutAdded(ULocalPlayer* LocalPlayer, UHUDPrimaryLayout* Layout)
{
#if WITH_EDITOR
	if (GIsEditor && LocalPlayer->IsPrimaryPlayer())
	{
		// So our controller will work in PIE without needing to click in the viewport
		FSlateApplication::Get().SetUserFocusToGameViewport(0);
	}
#endif
}

void UHUDLayoutPolicy::OnPrimaryLayoutRemoved(ULocalPlayer* LocalPlayer, UHUDPrimaryLayout* Layout)
{
}

UHUDPrimaryLayout* UHUDLayoutPolicy::CreatePrimaryLayout(ULocalPlayer* LocalPlayer) const
{
	if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
	{
		if (TSubclassOf<UHUDPrimaryLayout> LayoutClass = PrimaryLayoutClass.LoadSynchronous())
		{
			UHUDPrimaryLayout* PrimaryLayout = CreateWidget<UHUDPrimaryLayout>(PlayerController, LayoutClass);
			return PrimaryLayout;
		}
	}

	return nullptr;
}
