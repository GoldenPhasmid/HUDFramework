#pragma once

#include "GameplayTagContainer.h"

#include "HUDLayoutSlot.generated.h"

class ULocalPlayer;
struct FHUDLayoutExtensionHandle;
class FHUDLayoutExtension;
class UHUDLayoutSubsystem;

class FHUDLayoutSlot: public TSharedFromThis<FHUDLayoutSlot>
{
public:
	FGameplayTag SlotTag;
	TWeakObjectPtr<const ULocalPlayer> PlayerContext;

	using TSlotCallback = TFunction<UUserWidget*(const FHUDLayoutExtensionRequest&)>;
	TSlotCallback AddExtension;
	TSlotCallback RemoveExtension;

	FHUDLayoutSlot() = default;
	FHUDLayoutSlot(const FGameplayTag& InSlotTag, const ULocalPlayer* InLocalPlayer, TSlotCallback&& InAddCallback, TSlotCallback&& InRemoveCallback);
	
	bool ExtensionPassesRequirements(const TSharedPtr<FHUDLayoutExtension>& Extension) const;
};

USTRUCT(BlueprintType)
struct HUDFRAMEWORK_API FHUDLayoutSlotHandle
{
	GENERATED_BODY()
public:

	FHUDLayoutSlotHandle() = default;

	FORCEINLINE bool IsValid() const
	{
		return Slot.IsValid();
	}

	friend FORCEINLINE bool operator==(const FHUDLayoutSlotHandle& Lhs, const FHUDLayoutSlotHandle& Rhs)
	{
		return Lhs.Slot == Rhs.Slot;
	}
	
	friend FORCEINLINE bool operator!=(const FHUDLayoutSlotHandle& Lhs, const FHUDLayoutSlotHandle& Rhs)
	{
		return Lhs.Slot != Rhs.Slot;
	}
	
	friend FORCEINLINE uint32 GetTypeHash(const FHUDLayoutSlotHandle& Handle)
	{
		return PointerHash(Handle.Slot.Get());
	}

	static FHUDLayoutSlotHandle EmptyHandle;

private:

	void Invalidate();
	
	FHUDLayoutSlotHandle(UHUDLayoutSubsystem* InSubsystem, TSharedPtr<FHUDLayoutSlot> InSlot);

	TWeakObjectPtr<UHUDLayoutSubsystem> Subsystem;
	TSharedPtr<FHUDLayoutSlot> Slot;

	friend UHUDLayoutSubsystem;
	friend class UHUDLayoutBlueprintLibrary;
};

template<>
struct TStructOpsTypeTraits<FHUDLayoutSlotHandle> : public TStructOpsTypeTraitsBase2<FHUDLayoutSlotHandle>
{
	enum
	{
		WithCopy = true,  // This ensures the opaque type is copied correctly in BPs
		WithIdenticalViaEquality = true,
	};
};

