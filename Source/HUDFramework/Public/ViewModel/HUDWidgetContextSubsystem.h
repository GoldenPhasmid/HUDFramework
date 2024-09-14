#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HUDWidgetContext.h"
#include "HUDWidgetPool.h"

#include "HUDWidgetContextSubsystem.generated.h"

struct FUserWidgetPool;
struct FHUDWidgetPool;
class FHUDLayoutExtension;
class UMVVMView;
class UMVVMViewModelBase;
class UHUDViewModel;

UCLASS()
class HUDFRAMEWORK_API UHUDWidgetContextSubsystem: public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:

	/** @return widget context subsystem. Valid only for game worlds */
	static UHUDWidgetContextSubsystem* Get(const UObject* WorldContextObject);
	static UHUDWidgetContextSubsystem* Get(const UWorld* World);

	//~Begin USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End USubsystem interface
	
	/**
	 * Register and initialize user widget with provided widget context
	 * @param UserWidget user widget to initialize
	 * @param WidgetContext widget context
	 */
	void InitializeWidget(UUserWidget* UserWidget, const FHUDWidgetContextHandle& WidgetContext);

	/**
	 * Register and initialize widget, HUD widget pool version
	 * Explicitly handles widget context extension if it is already present on a widget
	 * @param WidgetPool reference to owning widget pool
	 * @param UserWidget user widget to initialize
	 * @param WidgetContext widget context
	 */
	void InitializeWidget_FromHUDWidgetPool(const FHUDWidgetPool& WidgetPool, UUserWidget* UserWidget, const FHUDWidgetContextHandle& WidgetContext);

	/**
	 * Register and initialize widget, UE widget pool version.
	 * @note Use only with unreal stuff where you can't replace original widget pool with a plugin version
	 * @param UserWidget user widget to initialize
	 * @param WidgetContext widget context
	 */
	void InitializeWidget_FromUserWidgetPool(UUserWidget* UserWidget, const FHUDWidgetContextHandle& WidgetContext);
	
	/**
	 * register user widget with provided widget context
	 * @param UserWidget user widget
	 * @param WidgetContext widget context
	 * @param bFromWidgetPool
	 * @return whether widget was successfully registered
	 */
	UE_DEPRECATED(5.4, "Will be made private.")
	bool CreateWidgetExtension(UUserWidget* UserWidget, const FHUDWidgetContextHandle& WidgetContext, bool bFromWidgetPool);
	
	/** @return widget context for given user widget. Widget should be registered with subsystem beforehand */
	FHUDWidgetContextHandle GetWidgetContext(const UUserWidget* UserWidget) const;

	/** @return whether widget has registered with widget context subsystem */
	bool IsWidgetRegistered(const UUserWidget* UserWidget) const;

	/**
	 * Create view model instance for given @View and assigns widget context if any
	 * @param UserWidget
	 * @param ContextWidget
	 * @return view model
	 */
	template <typename TViewModelClass>
	TViewModelClass* CreateViewModel(const UUserWidget* UserWidget, const UUserWidget* ContextWidget)
	{
		return CastChecked<TViewModelClass>(CreateViewModel(UserWidget, ContextWidget, TViewModelClass::StaticClass()), ECastCheckedType::NullAllowed);
	}

	/**
	 * Create view model instance for given @View and assigns widget context if any
	 * @param UserWidget widget to create view model for
	 * @param ContextWidget widget const provider
	 * @param ViewModelClass view model class to create
	 * @return view model
	 */
	UHUDViewModel* CreateViewModel(const UUserWidget* UserWidget, const UUserWidget* ContextWidget, TSubclassOf<UHUDViewModel> ViewModelClass);

	/** Release view model (either destroy or return it to subsystem) for given view */
	void ReleaseViewModel(const UUserWidget* UserWidget, UHUDViewModel* ViewModel);
	
protected:

	void TickModels(float DeltaTime);

	/** run widget context initialization for widget tree, starting with @UserWidget */
	void InitializeWidgetTree(UUserWidget* UserWidget);
	/** initialize single @UserWidget */
	void InitializeWidgetInternal(UUserWidget* UserWidget);

	/**
	 * @return true if @UserWidget is part of currently initializing widget tree aka is going to match one of @ActiveWidgetTrees
	 * Widget should not be initialized if it is already a part of initializing widget tree
	 */
	bool IsPartOfActiveWidgetTree(const UUserWidget* UserWidget) const;

	/** */
	const UUserWidget* GetActiveWidgetTreeForWidget(const UUserWidget* UserWidget) const;

private:
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UHUDViewModel>> TickableModels;

	/** List of roots of currently initializing widget trees */
	UPROPERTY(Transient)
	TArray<UUserWidget*> ActiveWidgetTrees;
};
