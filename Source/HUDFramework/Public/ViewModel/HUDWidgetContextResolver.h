#pragma once

#include "CoreMinimal.h"
#include "View/MVVMViewModelContextResolver.h"

#include "HUDWidgetContextResolver.generated.h"

class UMVVMView;
class UUserWidget;
class UHUDWidgetContextSubsystem;

UCLASS(Blueprintable)
class HUDFRAMEWORK_API UHUDWidgetContextResolver: public UMVVMViewModelContextResolver
{
	GENERATED_BODY()
public:

	//~Begin UMVVMViewModelContextResolver interface
	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
	virtual void DestroyInstance(const UObject* ViewModel, const UMVVMView* View) const override;
	//~End UMVVMViewModelContextResolver interface

protected:

	/**
	 * Go up the outer chain to find first registered user widget
	 * @return first registered user widget in outer chain
	 */
	const UUserWidget* FindRegisteredWidget(const UHUDWidgetContextSubsystem* Subsystem, const UUserWidget* UserWidget) const;
};
