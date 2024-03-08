#include "ViewModel/HUDWidgetContextSubsystem.h"

#include "HUDFramework.h"
#include "HUDLayoutBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "View/MVVMView.h"

#include "HUDLayoutSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "View/MVVMViewClass.h"
#include "ViewModel/HUDViewModel.h"
#include "ViewModel/HUDWidgetContextExtension.h"
#include "ViewModel/HUDWidgetContextInterface.h"

DECLARE_CYCLE_STAT(TEXT("InitializeWidgetTree"),	STAT_HUD_Framework_InitializeWidgetTree, STATGROUP_HUD_Framework);
DECLARE_CYCLE_STAT(TEXT("InitializeWidget"),		STAT_HUD_Framework_InitializeWidget,	STATGROUP_HUD_Framework);
DECLARE_CYCLE_STAT(TEXT("CreateViewModel"),			STAT_HUD_Framework_CreateViewModel,		STATGROUP_HUD_Framework);
DECLARE_CYCLE_STAT(TEXT("ReleaseViewModel"),		STAT_HUD_Framework_ReleaseViewModel,	STATGROUP_HUD_Framework);
DECLARE_CYCLE_STAT(TEXT("TickViewModels"),			STAT_HUD_Framework_TickModels,			STATGROUP_HUD_Framework);

UHUDWidgetContextSubsystem* UHUDWidgetContextSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	return Get(World);
}

UHUDWidgetContextSubsystem* UHUDWidgetContextSubsystem::Get(const UWorld* World)
{
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UHUDWidgetContextSubsystem>();
		}
	}

	return nullptr;
}

void UHUDWidgetContextSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<UHUDLayoutSubsystem>();

	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnPreTick().AddUObject(this, &ThisClass::TickModels);
	}
}

void UHUDWidgetContextSubsystem::Deinitialize()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnPreTick().RemoveAll(this);
	}
	
	Super::Deinitialize();
}

bool UHUDWidgetContextSubsystem::RegisterWidget(UUserWidget* UserWidget, const FHUDWidgetContextHandle& WidgetContext)
{
	if (!IsValid(UserWidget))
	{
		return false;
	}

	// widget returned from widget pool can be either newly created or already constructed
	if (UHUDWidgetContextExtension* ContextExtension = UserWidget->GetExtension<UHUDWidgetContextExtension>())
	{
		ContextExtension->SetWidgetContext(WidgetContext);
	}
	else
	{
		ContextExtension = UserWidget->AddExtension<UHUDWidgetContextExtension>();
		ContextExtension->SetWidgetContext(WidgetContext);
	}

	return true;
}

void UHUDWidgetContextSubsystem::InitializeWidget_FromWidgetPool(const FHUDUserWidgetPool& WidgetPool, UUserWidget* UserWidget, const FHUDWidgetContextHandle& WidgetContext)
{
	if (!RegisterWidget(UserWidget, WidgetContext))
	{
		return;
	}

	// widget pool is a special case of widget initialization
	// widget already Constructed, has WidgetContextExtension and initialized view model
	// out target is to properly "deinitialize" ViewModel (in InitializeWidgetInternal) and "initialize" it back (again InitializeWidgetInternal)
	// Widgets that are used in widget pool shouldn't rely on Construct/Destruct calls anyway
	if (UHUDWidgetContextExtension* Extension = UserWidget->GetExtension<UHUDWidgetContextExtension>())
	{
		// force Deinitialize extension
		Extension->SetInitialized(false);
	}

	// initialize widget tree, unless widget is already a part of currently initializing widget tree
	if (!IsPartOfActiveWidgetTree(UserWidget))
	{
		InitializeWidgetTree(UserWidget);
	}
}

void UHUDWidgetContextSubsystem::InitializeWidget(UUserWidget* UserWidget, const FHUDWidgetContextHandle& WidgetContext)
{
	if (!IsValid(UserWidget))
	{
		return;
	}
	
	if (UserWidget->IsConstructed())
	{
		// already constructed widgets are not supported
		const FString WidgetTree = UHUDLayoutBlueprintLibrary::ConstructWidgetTreeString(UserWidget);
		UE_LOG(LogHUDFramework, Error, TEXT("%s: Trying to initialize widget that is already constructed. ")
									   TEXT("Either initialization is late or calling code is using widget pool.\nWidget Tree: %s"),
			*FString(__FUNCTION__), *WidgetTree);
		return;
	}
	
	if (UserWidget->GetExtension<UHUDWidgetContextExtension>() != nullptr)
	{
		// user widget has already been registered. Widgets are either initialized right away or registered as part of parent widget tree initialization
		const FString WidgetTree = UHUDLayoutBlueprintLibrary::ConstructWidgetTreeString(UserWidget);
		UE_LOG(LogHUDFramework, Error, TEXT("%s: Trying to directly initialize already registered widget. ")
									   TEXT("Possible scenario - widget was mistakenly initialized directly by InitializeWidget call, and then initialized as part of widget tree initialization.\n")
									   TEXT("Widget cannot be registered or initialized twice. \nWidget Tree: %s"), *FString(__FUNCTION__), *WidgetTree);
		return;
	}

	const UMVVMView* View = UserWidget->GetExtension<UMVVMView>();
	if (View && View->AreSourcesInitialized())
	{
		// view models are already initialized at this point
		const FString WidgetTree = UHUDLayoutBlueprintLibrary::ConstructWidgetTreeString(UserWidget);
		UE_LOG(LogHUDFramework, Error, TEXT("%s: Trying to initialize widget with already initialized sources. ")
									   TEXT("Either initialization is late or calling code is using widget pool.\nWidget Tree: %s"),
			*FString(__FUNCTION__), *WidgetTree);
		return;
	}

	// store widget context as part of widget context extension
	const bool bResult = RegisterWidget(UserWidget, WidgetContext);
	check(bResult);

	// initialize widget tree of original widget, unless it is already a part of currently initializing widget tree
	if (!IsPartOfActiveWidgetTree(UserWidget))
	{
		InitializeWidgetTree(UserWidget);
	}
#if WITH_EDITOR && 0
	else
	{
		const FString WidgetTree = UHUDLayoutBlueprintLibrary::ConstructWidgetTreeString(UserWidget);
		UE_LOG(LogHUDFramework, Error,  TEXT("%s Trying to initialize widget that is a part of another widget tree, that is being initialized.\n")
										TEXT("This happens if InitializeWidget is called in InitializeWidgetTree execution graph. Consider calling RegisterWidget instead.\n")
										TEXT("Widget Tree: %s"), *FString(__FUNCTION__), *WidgetTree);
	}
#endif
}

bool UHUDWidgetContextSubsystem::IsPartOfActiveWidgetTree(const UUserWidget* UserWidget) const
{
	// this is a costly check that goes through widget outer chain and verifies that any of the parent widget doesn't match one of activating widget trees
	auto Pred = [this](const UWidget* Widget)
	{
		if (const UUserWidget* UserWidget = Cast<UUserWidget>(Widget); UserWidget && ActiveWidgetTrees.Contains(UserWidget))
		{
			return true;
		}

		return false;
	};

	const UWidget* FoundWidget = ForEachParentWidget(UserWidget, Pred);
	return FoundWidget != nullptr;
}

void UHUDWidgetContextSubsystem::InitializeWidgetTree(UUserWidget* UserWidget)
{
	check(UserWidget);
	SCOPE_CYCLE_COUNTER(STAT_HUD_Framework_InitializeWidgetTree);
	
	TArray<UUserWidget*> WidgetsToInitialize;
	WidgetsToInitialize.Add(UserWidget);
	
	check(ActiveWidgetTrees.Contains(UserWidget) == false);
	// set widget tree root using guard value.
	// Prevent another widget to initialize while being a part of other initializing widget tree
	ActiveWidgetTrees.Add(UserWidget);
	ON_SCOPE_EXIT
	{
		check(ActiveWidgetTrees.Contains(UserWidget) == true);
		ActiveWidgetTrees.Remove(UserWidget);	
	};
	
	for (int32 Index = 0; Index < WidgetsToInitialize.Num(); ++Index)
	{
		UUserWidget* CurrentWidget = WidgetsToInitialize[Index];
		check(CurrentWidget);
		
		UHUDWidgetContextExtension* Extension = CurrentWidget->GetExtension<UHUDWidgetContextExtension>();
		if (Extension != nullptr && Extension->IsInitialized())
		{
			const FString WidgetTree = UHUDLayoutBlueprintLibrary::ConstructWidgetTreeString(CurrentWidget);
			UE_LOG(LogHUDFramework, Error, TEXT("%s Trying to double initialize widget extension.\nWidget Tree: %s"), *FString(__FUNCTION__), *WidgetTree);
			continue;	
		}
		
		InitializeWidgetInternal(CurrentWidget);
		
		const UWidgetTree* WidgetTree = CurrentWidget->WidgetTree;
		check(WidgetTree);

		WidgetTree->ForEachWidget([this, &WidgetsToInitialize](UWidget* Widget)
		{
			if (UUserWidget* UserWidget = Cast<UUserWidget>(Widget))
			{
				WidgetsToInitialize.Add(UserWidget);
			}
		});
	}
}

void UHUDWidgetContextSubsystem::InitializeWidgetInternal(UUserWidget* UserWidget)
{
	SCOPE_CYCLE_COUNTER(STAT_HUD_Framework_InitializeWidget);

	check(UserWidget);
	if (UMVVMView* View = UserWidget->GetExtension<UMVVMView>())
	{
		if (View->AreSourcesInitialized())
		{
			// uninitialize sources to trigger view model destruction
			// UserWidget may or may not be initialized
			View->UninitializeSources();
		}

		// this is necessary for view bindings to work
		// Basically BindingLibrary first initialized in Construct, so it means sources and bindings were meant to be initialized AFTER Construct
		// However, we're initializing bindings BEFORE construct, so that we have up and running view model in Construct
		const UMVVMViewClass* ViewClass = View->GetViewClass();
		FMVVMCompiledBindingLibrary& BindingLibrary = const_cast<FMVVMCompiledBindingLibrary&>(ViewClass->GetBindingLibrary());
		if (!BindingLibrary.IsLoaded())
		{
			BindingLibrary.Load();
		}

		// essentially create view model and initialize view bindings
		View->InitializeSources();
	}

	if (UHUDWidgetContextExtension* Extension = UserWidget->GetExtension<UHUDWidgetContextExtension>())
	{
		check(!Extension->IsInitialized());
		if (UserWidget->Implements<UHUDWidgetContextInterface>())
		{
			check(IsWidgetRegistered(UserWidget));
		
			const FHUDWidgetContextHandle WidgetContext = Extension->GetWidgetContext();
			IHUDWidgetContextInterface::Execute_InitializeWidgetTree(UserWidget, WidgetContext);
		}

		Extension->SetInitialized(true);
	}
}

FHUDWidgetContextHandle UHUDWidgetContextSubsystem::GetWidgetContext(const UUserWidget* UserWidget) const
{
	if (UserWidget == nullptr || !IsWidgetRegistered(UserWidget))
	{
		UE_LOG(LogHUDFramework, Warning, TEXT("%s: Failed to find widget context for user widget [%s]"), *FString(__FUNCTION__), *GetNameSafe(UserWidget));
		return FHUDWidgetContextHandle{};
	}
	
	return UserWidget->GetExtension<UHUDWidgetContextExtension>()->GetWidgetContext();
}

bool UHUDWidgetContextSubsystem::IsWidgetRegistered(const UUserWidget* UserWidget) const
{
	return UserWidget != nullptr && UserWidget->GetExtension<UHUDWidgetContextExtension>() != nullptr;
}

UHUDViewModel* UHUDWidgetContextSubsystem::CreateViewModel(const UUserWidget* UserWidget, const UUserWidget* ContextWidget, TSubclassOf<UHUDViewModel> ViewModelClass)
{
	SCOPE_CYCLE_COUNTER(STAT_HUD_Framework_CreateViewModel);
	
	// @todo: view model pooling
	UHUDViewModel* ViewModel = NewObject<UHUDViewModel>(this, ViewModelClass);
	FHUDWidgetContextHandle WidgetContext = GetWidgetContext(UserWidget);

#if WITH_EDITOR
	// validate context requirements in editor to avoid crashes
	if (ViewModel->RequiresContext() && !WidgetContext.IsValid())
	{
		const FString WidgetTree = UHUDLayoutBlueprintLibrary::ConstructWidgetTreeString(UserWidget);
		UE_LOG(LogHUDFramework, Error, TEXT("%s: Widget requires a context but is missing one. View model is unitialized and would not be used to avoid crashes. Widget Tree: %s"), *FString(__FUNCTION__), *WidgetTree);
		return ViewModel;
	}
#endif

	ViewModel->InitializeWithContext(UserWidget, WidgetContext);
	if (ViewModel->IsAllowedToTick())
	{
		TickableModels.Add(ViewModel);
	}
	
	return ViewModel;
}

void UHUDWidgetContextSubsystem::ReleaseViewModel(const UUserWidget* UserWidget, UHUDViewModel* ViewModel)
{
	SCOPE_CYCLE_COUNTER(STAT_HUD_Framework_ReleaseViewModel);
	
	FHUDWidgetContextHandle WidgetContext = GetWidgetContext(UserWidget);

#if WITH_EDITOR
	// validate context requirements in editor to avoid crashes
	if (ViewModel->RequiresContext() && !WidgetContext.IsValid())
	{
		const FString WidgetTree = UHUDLayoutBlueprintLibrary::ConstructWidgetTreeString(UserWidget);
		UE_LOG(LogHUDFramework, Error, TEXT("%s: Widget requires a context but is missing one. View model is unitialized and would not be used to avoid crashes. Widget Tree: %s"), *FString(__FUNCTION__), *WidgetTree);

		// mark viewmodel as garbage either way
		ViewModel->MarkAsGarbage();
		return;
	}
#endif
	
	ViewModel->Deinitialize(UserWidget);
	if (ViewModel->IsAllowedToTick())
	{
		TickableModels.Remove(ViewModel);
	}

	// @todo: view model pooling
	ViewModel->MarkAsGarbage();
}

void UHUDWidgetContextSubsystem::TickModels(float DeltaTime)
{
	if (TickableModels.IsEmpty())
	{
		return;
	}
	
	SCOPE_CYCLE_COUNTER(STAT_HUD_Framework_TickModels);
	SCOPED_NAMED_EVENT(UHUDViewModelSubsystem_Tick, FColor::Turquoise)

	for (UHUDViewModel* TickableModel: TickableModels)
	{
		if (TickableModel->IsTickable())
		{
			TickableModel->Tick(DeltaTime);
		}
	}
}
