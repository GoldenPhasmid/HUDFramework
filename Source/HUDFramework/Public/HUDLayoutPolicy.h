#pragma once

#include "CoreMinimal.h"
#include "HUDPrimaryLayout.h"
#include "UObject/Object.h"

#include "HUDLayoutPolicy.generated.h"

class ULocalPlayer;
class UHUDPrimaryLayout;

USTRUCT()
struct FHUDPrimaryLayoutInstance
{
	GENERATED_BODY()

	FHUDPrimaryLayoutInstance() = default;
	FHUDPrimaryLayoutInstance(ULocalPlayer* InLocalPLayer, UHUDPrimaryLayout* InPrimaryLayout, bool bInViewport)
		: LocalPlayer(InLocalPLayer)
		, PrimaryLayout(InPrimaryLayout)
		, bAddedToViewport(bInViewport)
	{ }

	void AddToViewport();
	void RemoveFromViewport();
	
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UHUDPrimaryLayout> PrimaryLayout = nullptr;

	UPROPERTY(Transient)
	bool bAddedToViewport = false;

	friend bool operator==(const FHUDPrimaryLayoutInstance& Lhs, const FHUDPrimaryLayoutInstance& Rhs)
	{
		return Lhs.LocalPlayer == Rhs.LocalPlayer;
	}
	
	friend bool operator==(const FHUDPrimaryLayoutInstance& Layout, const ULocalPlayer* LocalPLayer)
	{
		return Layout.LocalPlayer.Get() == LocalPLayer;
	}
};

UCLASS(Abstract, BlueprintType, Blueprintable)
class HUDFRAMEWORK_API UHUDLayoutPolicy: public UObject
{
	GENERATED_BODY()
public:

	/** @return primary layout for given local player */
	UHUDPrimaryLayout* GetPrimaryLayout(const ULocalPlayer* LocalPlayer) const;
	
	virtual void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);
	virtual void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);

protected:

	void OnPlayerControllerChanged(APlayerController* PlayerController, ULocalPlayer* LocalPlayer);

	virtual UWorld* GetWorld() const override;
	
	void AddPrimaryLayout(ULocalPlayer* LocalPlayer);
	void RemovePrimaryLayout(ULocalPlayer* LocalPlayer);

	virtual void OnPrimaryLayoutAdded(ULocalPlayer* LocalPlayer, UHUDPrimaryLayout* Layout);
	virtual void OnPrimaryLayoutRemoved(ULocalPlayer* LocalPlayer, UHUDPrimaryLayout* Layout);

	UHUDPrimaryLayout* CreatePrimaryLayout(ULocalPlayer* LocalPlayer) const;

	/** Primary layout class to create */
	UPROPERTY(EditAnywhere, meta = (Validate))
	TSoftClassPtr<UHUDPrimaryLayout> PrimaryLayoutClass;

	/** Currently active primary layouts, one per local player */
	UPROPERTY(Transient)
	TArray<FHUDPrimaryLayoutInstance> ActiveLayouts;
};
