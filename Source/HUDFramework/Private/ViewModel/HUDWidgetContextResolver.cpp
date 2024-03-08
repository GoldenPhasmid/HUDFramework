#include "ViewModel/HUDWidgetContextResolver.h"

#include "Blueprint/UserWidget.h"
#include "View/MVVMView.h"
#include "ViewModel/HUDViewModel.h"
#include "ViewModel/HUDWidgetContextSubsystem.h"
#include "HUDLayoutBlueprintLibrary.h"

UObject* UHUDWidgetContextResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	UHUDWidgetContextSubsystem* VMSubsystem = UHUDWidgetContextSubsystem::Get(UserWidget);
	if (VMSubsystem == nullptr)
	{
		return nullptr;
	}

	UClass* ModelType = const_cast<UClass*>(ExpectedType);
	
	const UUserWidget* ContextWidget = FindRegisteredWidget(VMSubsystem, UserWidget);
	// it is ok to be unable to find registered view, CreateViewModel should handle it
	return VMSubsystem->CreateViewModel(UserWidget, ContextWidget, TSubclassOf<UHUDViewModel>{ModelType});
}

void UHUDWidgetContextResolver::DestroyInstance(const UObject* ViewModel, const UMVVMView* View) const
{
	const UUserWidget* UserWidget = View->GetTypedOuter<UUserWidget>();
	UHUDWidgetContextSubsystem* VMSubsystem = UHUDWidgetContextSubsystem::Get(UserWidget);
	if (VMSubsystem == nullptr)
	{
		return;
	}

	UHUDViewModel* HUDViewModel = const_cast<UHUDViewModel*>(CastChecked<UHUDViewModel>(ViewModel));
	VMSubsystem->ReleaseViewModel(UserWidget, HUDViewModel);
}

const UUserWidget* UHUDWidgetContextResolver::FindRegisteredWidget(const UHUDWidgetContextSubsystem* Subsystem, const UUserWidget* UserWidget) const
{
	auto Pred = [&Subsystem](const UWidget* Widget)
	{
		if (const UUserWidget* UserWidget = Cast<UUserWidget>(Widget); UserWidget && Subsystem->IsWidgetRegistered(UserWidget))
		{
			return true;
		}

		return false;
	};

	const UWidget* FoundWidget = ForEachParentWidget(UserWidget, Pred);
	return CastChecked<UUserWidget>(FoundWidget, ECastCheckedType::NullAllowed);
}