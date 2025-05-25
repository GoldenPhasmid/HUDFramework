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
	// might be called in widget design time
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return Get(World);
	}

	return nullptr;
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

bool UHUDWidgetContextSubsystem::CreateWidgetExtension(UUserWidget* UserWidget, const FHUDWidgetContextHandle& WidgetContext, bool bFromWidgetPool)
{
	if (!IsValid(UserWidget))
	{
		return false;
	}

	// widget returned from widget pool can be either newly created or already constructed
	if (UHUDWidgetContextExtension* ContextExtension = UserWidget->GetExtension<UHUDWidgetContextExtension>())
	{
		// widget pool widgets should stay in widget pool
		check(ContextExtension->FromWidgetPool() == bFromWidgetPool);
		ContextExtension->SetWidgetContext(WidgetContext);
	}
	else
	{
		ContextExtension = UserWidget->AddExtension<UHUDWidgetContextExtension>();
		ContextExtension->SetFromWidgetPool(bFromWidgetPool);
		ContextExtension->SetWidgetContext(WidgetContext);
	}

	return true;
}

void UHUDWidgetContextSubsystem::InitializeWidget_FromHUDWidgetPool(const FHUDWidgetPool& WidgetPool, UUserWidget* UserWidget, const FHUDWidgetContextHandle& WidgetContext)
{
	constexpr bool bFromWidgetPool = true;
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	if (!CreateWidgetExtension(UserWidget, WidgetContext, bFromWidgetPool))
PRAGMA_ENABLE_DEPRECATION_WARNINGS
	{
		return;
	}

	if (UserWidget->IsConstructed())
	{
		// already constructed widgets are not supported, even in widget pool case
		const FString WidgetTree = UHUDLayoutBlueprintLibrary::ConstructWidgetTreeString(UserWidget);
		UE_LOG(LogHUDFramework, Error, TEXT("%s: Trying to initialize widget that is already constructed. ")
									   TEXT("Either initialization is late or calling code is using widget pool.\nWidget Tree: %s"),
			*FString(__FUNCTION__), *WidgetTree);
		return;
	}

	// widget pool is a special case of widget initialization
	// WidgetContextExtension might already been created and initialized, same for viewmodel sources
	// out target is to properly "deinitialize" ViewModel resources and "initialize" them again (@see InitializeWidgetInternal)
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

void UHUDWidgetContextSubsystem::InitializeWidget_FromUserWidgetPool(UUserWidget* UserWidget, const FHUDWidgetContextHandle& WidgetContext)
{
	static FHUDWidgetPool Dummy{};
	InitializeWidget_FromHUDWidgetPool(Dummy, UserWidget, WidgetContext);
}

void UHUDWidgetContextSubsystem::InitializeWidget(UUserWidget* UserWidget, const FHUDWidgetContextHandle& WidgetContext)
{
	if (!IsValid(UserWidget))
	{
		return;
	}

	if (UserWidget->IsConstructed())
	{
		// already constructed widgets are not supported, even in widget pool case
		const FString WidgetTree = UHUDLayoutBlueprintLibrary::ConstructWidgetTreeString(UserWidget);
		UE_LOG(LogHUDFramework, Error, TEXT("%s: Trying to initialize widget that is already constructed. ")
									   TEXT("Either initialization is late or calling code is using widget pool.\nWidget Tree: %s"),
			*FString(__FUNCTION__), *WidgetTree);
		return;
	}

	bool bFromWidgetPool = false;
	const UUserWidget* RootWidget = GetActiveWidgetTreeForWidget(UserWidget);
	// We determine whether initializing widget is a part of another widget that uses widget pool
	// If yes, we should promote widget pool status to the newly created extension and skip checks that are valid for normally created widgets
	if (RootWidget != nullptr)
	{
		const UHUDWidgetContextExtension* RootExtension = RootWidget->GetExtension<UHUDWidgetContextExtension>();
		// widget should be already initialized
		check(RootExtension && RootExtension->IsInitialized());
		bFromWidgetPool = RootExtension->FromWidgetPool();
	}

	if (bFromWidgetPool)
	{
		if (UHUDWidgetContextExtension* Extension = UserWidget->GetExtension<UHUDWidgetContextExtension>())
		{
			// force Deinitialize extension
			// view binding sources are checked and deinitialized later in InitializeWidgetInternal
			Extension->SetInitialized(false);
		}
	}
	else
	{
		// widget is either a root widget or created as a part of widget tree initialization
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
	}


	// store widget context as part of widget context extension
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	const bool bResult = CreateWidgetExtension(UserWidget, WidgetContext, bFromWidgetPool);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	check(bResult);

	// initialize widget tree of original widget, unless it is already a part of currently initializing widget tree
	if (RootWidget == nullptr)
	{
		InitializeWidgetTree(UserWidget);
	}
}

bool UHUDWidgetContextSubsystem::IsPartOfActiveWidgetTree(const UUserWidget* UserWidget) const
{
	return GetActiveWidgetTreeForWidget(UserWidget) != nullptr;
}

const UUserWidget* UHUDWidgetContextSubsystem::GetActiveWidgetTreeForWidget(const UUserWidget* UserWidget) const
{
	auto Pred = [this](const UWidget* Widget)
	{
		if (const UUserWidget* UserWidget = Cast<UUserWidget>(Widget); UserWidget && ActiveWidgetTrees.Contains(UserWidget))
		{
			return true;
		}

		return false;
	};

	// this is a costly check that goes through widget outer chain and verifies that any of the parent widget doesn't match one of activating widget trees
	const UWidget* FoundWidget = ForEachParentWidget(UserWidget, Pred);
	return CastChecked<UUserWidget>(FoundWidget, ECastCheckedType::NullAllowed);
}

void UHUDWidgetContextSubsystem::InitializeWidgetTree(UUserWidget* UserWidget)
{
	check(UserWidget);
	SCOPE_CYCLE_COUNTER(STAT_HUD_Framework_InitializeWidgetTree);
	
	TArray<UUserWidget*, TInlineAllocator<80>> WidgetsToInitialize;
	WidgetsToInitialize.Add(UserWidget);
	
	check(ActiveWidgetTrees.Contains(UserWidget) == false);
	// add root widget to the list of active widget trees
	// Prevent another widget to initialize when if it is already going to be initialized as a part of an active widget tree
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
			// widget was created and initialized during InitializeWidgetTree call
			// if extension is initialized, it means CurrentWidget and its widget tree are initialized as well. Skip it
			UE_LOG(LogHUDFramework, Verbose, TEXT("Widget [%s] is already initialized. Skipping widget tree initialization."), *GetNameSafe(CurrentWidget));
			return;	
		}

		InitializeWidgetInternal(CurrentWidget, Extension);
		
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

void UHUDWidgetContextSubsystem::InitializeWidgetInternal(UUserWidget* UserWidget, UHUDWidgetContextExtension* Extension)
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

	if (Extension != nullptr)
	{
		check(!Extension->IsInitialized());
		Extension->SetInitialized(true);
		
		if (UserWidget->Implements<UHUDWidgetContextInterface>())
		{
			check(IsWidgetRegistered(UserWidget));
		
			const FHUDWidgetContextHandle WidgetContext = Extension->GetWidgetContext();
			IHUDWidgetContextInterface::Execute_InitializeWidgetTree(UserWidget, WidgetContext);
		}
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
