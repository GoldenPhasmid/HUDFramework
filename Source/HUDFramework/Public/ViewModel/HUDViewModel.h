#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"

#include "HUDViewModel.generated.h"

struct FHUDWidgetContextHandle;

UCLASS(Abstract, Blueprintable, BlueprintType)
class HUDFRAMEWORK_API UHUDViewModel: public UMVVMViewModelBase
{
	GENERATED_BODY()
public:

	UHUDViewModel();

	/** */
	virtual void InitializeWithContext(const UUserWidget* UserWidget, const FHUDWidgetContextHandle& ContextHandle) {}

	/** */
	virtual void Deinitialize(const UUserWidget* UserWidget) {}

	/** Tick condition */
	virtual bool IsTickable() const { return false; }
	/** Tick functionality for view model */
	virtual void Tick(float DeltaTime) {}
	
	FORCEINLINE bool IsAllowedToTick() const
	{
		return bAllowedToTick;
	}

#if WITH_EDITOR
	FORCEINLINE bool RequiresContext() const
	{
		return bRequiresContext;
	}
#endif

protected:
	
	uint8 bAllowedToTick: 1;
	uint8 bRequiresContext: 1;
};
