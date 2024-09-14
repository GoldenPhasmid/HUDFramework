#pragma once

#include "CoreMinimal.h"
#include "Extensions/UserWidgetExtension.h"
#include "HUDWidgetContext.h"

#include "HUDWidgetContextExtension.generated.h"

struct FHUDWidgetContextHandle;

UCLASS()
class HUDFRAMEWORK_API UHUDWidgetContextExtension: public UUserWidgetExtension
{
	GENERATED_BODY()
public:

	//~Begin UserWidgetExtension interface
	virtual void Construct() override;
	virtual void Destruct() override;
	//~End UserWidgetExtension interface

	FORCEINLINE FHUDWidgetContextHandle GetWidgetContext() const
	{
		return WidgetContext;
	}
	
	FORCEINLINE	void SetWidgetContext(const FHUDWidgetContextHandle& InWidgetContext)
	{
		WidgetContext = InWidgetContext;
	}
	
	FORCEINLINE bool IsInitialized() const
	{
		return bInitialized;
	}

	FORCEINLINE void SetInitialized(bool bNewInitialized)
	{
		bInitialized = bNewInitialized;
	}

	FORCEINLINE bool FromWidgetPool() const
	{
		return bFromWidgetPool;
	}

	FORCEINLINE void SetFromWidgetPool(bool bValue)
	{
		bFromWidgetPool = bValue;
	}

protected:

	UPROPERTY()
	FHUDWidgetContextHandle WidgetContext;

	UPROPERTY()
	bool bInitialized = false;

	UPROPERTY()
	bool bFromWidgetPool = false;
};
