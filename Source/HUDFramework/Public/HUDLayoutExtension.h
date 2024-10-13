#pragma once

#include "HUDWidgetContext.h"
#include "GameplayTagContainer.h"

#include "HUDLayoutExtension.generated.h"

class ULocalPlayer;
class UHUDLayoutSubsystem;
struct FHUDLayoutExtensionHandle;

class FHUDLayoutExtension: public TSharedFromThis<FHUDLayoutExtension>
{
public:
	FGameplayTag SlotTag;
	TWeakObjectPtr<const ULocalPlayer> PlayerContext;
	TSubclassOf<UUserWidget> WidgetClass;
	/** kept alive in UHUDLayoutSubsystem::AddReferencedObjects */
	FHUDWidgetContextHandle WidgetContext;
	
	FHUDLayoutExtension() = default;
	FHUDLayoutExtension(const FGameplayTag& InSlotTag, TSubclassOf<UUserWidget> InWidgetClass, const ULocalPlayer* InLocalPlayer, const FHUDWidgetContextHandle& InContext);
};

USTRUCT(BlueprintType)
struct HUDFRAMEWORK_API FHUDLayoutExtensionHandle
{
	GENERATED_BODY()
	
	FHUDLayoutExtensionHandle() = default;
	
	FORCEINLINE bool IsValid() const { return Extension.IsValid(); }
	FORCEINLINE  UHUDLayoutSubsystem* GetSubsystem() const { return Subsystem.Get(); }
	
	friend FORCEINLINE bool operator==(const FHUDLayoutExtensionHandle& Lhs, const FHUDLayoutExtensionHandle& Rhs)
	{
		return Lhs.Extension == Rhs.Extension;
	}

	friend FORCEINLINE bool operator!=(const FHUDLayoutExtensionHandle& Lhs, const FHUDLayoutExtensionHandle& Rhs)
	{
		return Lhs.Extension != Rhs.Extension;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FHUDLayoutExtensionHandle& Handle)
	{
		return PointerHash(Handle.Extension.Get());
	}

	static FHUDLayoutExtensionHandle EmptyHandle;
	
private:

	void Invalidate();
	
	FHUDLayoutExtensionHandle(UHUDLayoutSubsystem* InSubsystem, TSharedPtr<FHUDLayoutExtension> InExtension);

	TWeakObjectPtr<UHUDLayoutSubsystem> Subsystem;
	TSharedPtr<FHUDLayoutExtension> Extension;

	friend class UHUDLayoutSubsystem;
	friend class UHUDLayoutBlueprintLibrary;
};

template<>
struct TStructOpsTypeTraits<FHUDLayoutExtensionHandle> : public TStructOpsTypeTraitsBase2<FHUDLayoutExtensionHandle>
{
	enum
	{
		WithCopy = true,  // This ensures the opaque type is copied correctly in BPs
		WithIdenticalViaEquality = true,
	};
};

USTRUCT(BlueprintType)
struct HUDFRAMEWORK_API FHUDLayoutExtensionRequest
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FHUDLayoutExtensionHandle Handle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag SlotTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY()
	FHUDWidgetContextHandle WidgetContext;
};