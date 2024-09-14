#include "HUDLayoutSubsystem.h"

#include "HUDFramework.h"
#include "HUDLayoutExtension.h"
#include "HUDLayoutPolicy.h"
#include "HUDFrameworkSettings.h"
#include "HUDLayoutSlot.h"

UHUDLayoutSubsystem* UHUDLayoutSubsystem::Get(const UObject* WorldContextObject)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UHUDLayoutSubsystem>();
		}
	}
	return nullptr;
}

bool UHUDLayoutSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UGameInstance* GameInstance = CastChecked<UGameInstance>(Outer);
	if (GameInstance->IsDedicatedServerInstance())
	{
		return false;
	}

	// allow to replace subsystem with one of derived classes
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(GetClass(), DerivedClasses, true);

	for (UClass* SubsystemClass: DerivedClasses)
	{
		USubsystem* Subsystem = SubsystemClass->GetDefaultObject<USubsystem>();
		if (Subsystem->ShouldCreateSubsystem(Outer))
		{
			return false;
		}
	}
	
	return Super::ShouldCreateSubsystem(Outer);
}

void UHUDLayoutSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UHUDFrameworkSettings* Settings = GetDefault<UHUDFrameworkSettings>();
	if (Policy == nullptr && !Settings->PolicyClass.IsNull())
	{
		TSubclassOf<UHUDLayoutPolicy> PolicyClass = Settings->PolicyClass.LoadSynchronous();
		Policy = NewObject<UHUDLayoutPolicy>(this, PolicyClass);
	}
	else
	{
		UE_LOG(LogHUDFramework, Warning, TEXT("%s: Initialized without policy class"), *FString(__FUNCTION__));
	}

	UGameInstance* GameInstance = CastChecked<UGameInstance>(GetOuter());
	GameInstance->OnLocalPlayerAddedEvent.AddUObject(this, &ThisClass::OnLocalPlayerAdded);
	GameInstance->OnLocalPlayerRemovedEvent.AddUObject(this, &ThisClass::OnLocalPlayerRemoved);
}

void UHUDLayoutSubsystem::Deinitialize()
{
	Policy = nullptr;

	UGameInstance* GameInstance = CastChecked<UGameInstance>(GetOuter());
	GameInstance->OnLocalPlayerAddedEvent.RemoveAll(this);
	GameInstance->OnLocalPlayerRemovedEvent.RemoveAll(this);
	
	Super::Deinitialize();
}

void UHUDLayoutSubsystem::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	UHUDLayoutSubsystem* LayoutSubsystem = CastChecked<UHUDLayoutSubsystem>(InThis);

#if 0
	for (auto& [Tag, ExtensionArray]: LayoutSubsystem->ActiveExtensions)
	{
		for (const TSharedPtr<FHUDLayoutExtension>& Extension: ExtensionArray)
		{
			if (Extension->DataPayload != nullptr)
			{
				Collector.AddReferencedObject(Extension->DataPayload);
			}
		}
	}
#endif
}

FHUDLayoutSlotHandle UHUDLayoutSubsystem::RegisterLayoutSlot(const FGameplayTag& SlotTag, const ULocalPlayer* LocalPlayer, TSlotCallback AddCallback, TSlotCallback RemoveCallback)
{
	if (!SlotTag.IsValid())
	{
		UE_LOG(LogHUDFramework, Error, TEXT("%s: invalid slot tag [%s]"), *FString(__FUNCTION__), *SlotTag.ToString());
		return FHUDLayoutSlotHandle::EmptyHandle;
	}

	FLayoutSlotArray& Array = ActiveSlots.FindOrAdd(SlotTag);

	TSharedPtr<FHUDLayoutSlot> Slot = Array.Add_GetRef(MakeShared<FHUDLayoutSlot>(SlotTag, LocalPlayer, MoveTemp(AddCallback), MoveTemp(RemoveCallback)));
	NotifySlotAdded(Slot);
	
	return FHUDLayoutSlotHandle{this, Slot};
}

void UHUDLayoutSubsystem::UnregisterLayoutSlot(FHUDLayoutSlotHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}
	check(Handle.Subsystem == this);

	TSharedPtr<FHUDLayoutSlot> Slot = Handle.Slot;
	check(Slot.IsValid());

	// invalidate handle referencing slot instance
	Handle.Invalidate();

	// remove slot instance from slot array
	FLayoutSlotArray& Array = ActiveSlots.FindChecked(Slot->SlotTag);
	Array.Remove(Slot);

	NotifySlotRemoved(Slot);
}

FHUDLayoutExtensionHandle UHUDLayoutSubsystem::RegisterLayoutExtension(const FGameplayTag& SlotTag, TSubclassOf<UUserWidget> WidgetClass, const ULocalPlayer* LocalPlayer)
{
	return RegisterLayoutExtensionWithContext(SlotTag, WidgetClass, LocalPlayer, FHUDWidgetContextHandle{});
}

FHUDLayoutExtensionHandle UHUDLayoutSubsystem::RegisterLayoutExtensionWithContext(const FGameplayTag& SlotTag, TSubclassOf<UUserWidget> WidgetClass, const ULocalPlayer* LocalPlayer, const FHUDWidgetContextHandle& Context)
{
	if (!SlotTag.IsValid())
	{
		UE_LOG(LogHUDFramework, Error, TEXT("%s: invalid slot tag [%s] for extension"), *FString(__FUNCTION__), *SlotTag.ToString());
		return FHUDLayoutExtensionHandle::EmptyHandle;
	}

	if (WidgetClass.Get() == nullptr)
	{
		UE_LOG(LogHUDFramework, Error, TEXT("%s invalid widget for slot extension tag [%s]"), *FString(__FUNCTION__), *SlotTag.ToString());
		return FHUDLayoutExtensionHandle::EmptyHandle;
	}
	
	FLayoutExtensionArray& Array = ActiveExtensions.FindOrAdd(SlotTag);

	TSharedPtr<FHUDLayoutExtension> Extension = Array.Add_GetRef(MakeShared<FHUDLayoutExtension>(SlotTag, WidgetClass, LocalPlayer, Context));
	NotifyExtensionAdded(Extension);

	return FHUDLayoutExtensionHandle{this, Extension};
}

void UHUDLayoutSubsystem::UnregisterLayoutExtension(FHUDLayoutExtensionHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}
	check(Handle.Subsystem == this);

	TSharedPtr<FHUDLayoutExtension> Extension = Handle.Extension;
	check(Extension.IsValid());

	// invalidate handle referencing extension
	Handle.Invalidate();

	// remove extension from slot array
	FLayoutExtensionArray& Array = ActiveExtensions.FindChecked(Extension->SlotTag);
	Array.Remove(Extension);
	
	NotifyExtensionRemoved(Extension); 
}

void UHUDLayoutSubsystem::NotifySlotAdded(TSharedPtr<FHUDLayoutSlot> Slot)
{
	UE_LOG(LogHUDFramework, Verbose, TEXT("Slot [%s] added for [%s] local player"), *Slot->SlotTag.ToString(), *GetNameSafe(Slot->PlayerContext.Get()));

	UpdateSlotExtensions(Slot, Slot->AddExtension, OnExtensionAdded);
}

void UHUDLayoutSubsystem::NotifySlotRemoved(TSharedPtr<FHUDLayoutSlot> Slot)
{
	UE_LOG(LogHUDFramework, Verbose, TEXT("Slot [%s] removed for [%s] context"), *Slot->SlotTag.ToString(), *GetNameSafe(Slot->PlayerContext.Get()));

	UpdateSlotExtensions(Slot, Slot->RemoveExtension, OnExtensionRemoved);
}

void UHUDLayoutSubsystem::UpdateSlotExtensions(TSharedPtr<FHUDLayoutSlot> Slot, TSlotCallback& CallbackRef, FSlotExtensionDelegate& ExtensionDelegate)
{
	ForEachExtension(Slot->SlotTag, [&Slot, &CallbackRef, &ExtensionDelegate, this](TSharedPtr<FHUDLayoutExtension> Extension)
	{
		if (Slot->ExtensionPassesRequirements(Extension))
		{
			FHUDLayoutExtensionRequest Request = CreateRequest(FHUDLayoutExtensionHandle{this, Extension});
			UUserWidget* UserWidget = Invoke(CallbackRef, Request);

			ExtensionDelegate.Broadcast(UserWidget, *Extension);
		}
	});
}

void UHUDLayoutSubsystem::ForEachSlot(FGameplayTag SlotTag, TFunctionRef<void( TSharedPtr<FHUDLayoutSlot> )> Func)
{
	while (SlotTag.IsValid())
	{
		if (const FLayoutSlotArray* ArrayPtr = ActiveSlots.Find(SlotTag))
		{
			FLayoutSlotArray ArrayCopy(*ArrayPtr);
			for (const TSharedPtr<FHUDLayoutSlot>& Slot: ArrayCopy)
			{
				Func(Slot);
			}
		}

		//@todo: full/partial match rules
		SlotTag = SlotTag.RequestDirectParent();
	}
}

void UHUDLayoutSubsystem::ForEachExtension(FGameplayTag SlotTag, TFunctionRef<void( TSharedPtr<FHUDLayoutExtension> )> Func)
{
	while (SlotTag.IsValid())
	{
		if (const FLayoutExtensionArray* ArrayPtr = ActiveExtensions.Find(SlotTag))
		{
			FLayoutExtensionArray ArrayCopy(*ArrayPtr);
			for (const TSharedPtr<FHUDLayoutExtension>& Extension: ArrayCopy)
			{
				Func(Extension);
			}
		}

		//@todo: full/partial match rules
		SlotTag = SlotTag.RequestDirectParent();
	}
}

void UHUDLayoutSubsystem::NotifyExtensionAdded(TSharedPtr<FHUDLayoutExtension> Extension)
{
	UE_LOG(LogHUDFramework, Verbose, TEXT("Extension added for slot [%s] with [%s] local player"), *Extension->SlotTag.ToString(), *GetNameSafe(Extension->PlayerContext.Get()));

	FHUDLayoutExtensionRequest Request = CreateRequest(FHUDLayoutExtensionHandle{this, Extension});
	ForEachSlot(Extension->SlotTag, [&Request, &Extension, this](TSharedPtr<FHUDLayoutSlot> Slot)
	{
		if (Slot->ExtensionPassesRequirements(Extension))
		{
			UUserWidget* UserWidget = Slot->AddExtension(Request);
			OnExtensionAdded.Broadcast(UserWidget, *Extension);
		}
	});
}

void UHUDLayoutSubsystem::NotifyExtensionRemoved(TSharedPtr<FHUDLayoutExtension> Extension)
{
	UE_LOG(LogHUDFramework, Verbose, TEXT("Extension removed for slot [%s] with [%s] local player"), *Extension->SlotTag.ToString(), *GetNameSafe(Extension->PlayerContext.Get()));

	FHUDLayoutExtensionRequest Request = CreateRequest(FHUDLayoutExtensionHandle{this, Extension});
	ForEachSlot(Extension->SlotTag, [&Request, &Extension, this](TSharedPtr<FHUDLayoutSlot> Slot)
	{
		if (Slot->ExtensionPassesRequirements(Extension))
		{
			UUserWidget* UserWidget = Slot->RemoveExtension(Request);
			OnExtensionRemoved.Broadcast(UserWidget, *Extension);
		}
	});
}

FHUDLayoutExtensionRequest UHUDLayoutSubsystem::CreateRequest(const FHUDLayoutExtensionHandle& Handle)
{
	check(Handle.IsValid());
	
	FHUDLayoutExtensionRequest Request;
	Request.Handle = Handle;
	Request.SlotTag = Handle.Extension->SlotTag;
	Request.WidgetClass = Handle.Extension->WidgetClass;
	Request.WidgetContext = Handle.Extension->WidgetContext;

	return Request;
}

FHUDLayoutExtensionHandle UHUDLayoutSubsystem::K2_RegisterLayoutExtension(const APlayerController* Player, const FGameplayTag& SlotTag, TSubclassOf<UUserWidget> WidgetClass)
{
	return RegisterLayoutExtension(SlotTag, WidgetClass, Player->GetLocalPlayer());
}

FHUDLayoutExtensionHandle UHUDLayoutSubsystem::K2_RegisterLayoutExtensionWithContext(const APlayerController* Player, const FGameplayTag& SlotTag, TSubclassOf<UUserWidget> WidgetClass, const FHUDWidgetContextHandle& Context)
{
	return RegisterLayoutExtensionWithContext(SlotTag, WidgetClass, Player->GetLocalPlayer(), Context);
}

void UHUDLayoutSubsystem::K2_UnregisterLayoutExtension(FHUDLayoutExtensionHandle& Handle)
{
	UnregisterLayoutExtension(Handle);
}

void UHUDLayoutSubsystem::OnLocalPlayerAdded(ULocalPlayer* LocalPlayer)
{
	if (Policy != nullptr)
	{
		Policy->NotifyPlayerAdded(LocalPlayer);
	}
}

void UHUDLayoutSubsystem::OnLocalPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	if (Policy != nullptr)
	{
		Policy->NotifyPlayerRemoved(LocalPlayer);
	}
}
