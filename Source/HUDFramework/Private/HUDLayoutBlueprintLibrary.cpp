#include "HUDLayoutBlueprintLibrary.h"

#include "HUDLayoutExtension.h"
#include "HUDLayoutPolicy.h"
#include "HUDLayoutSlot.h"
#include "HUDLayoutSubsystem.h"
#include "HUDPrimaryLayout.h"
#include "ViewModel/HUDWidgetContextSubsystem.h"
#include "CommonActivatableWidget.h"
#include "Blueprint/BlueprintExceptionInfo.h"
#include "Blueprint/UserWidget.h"

#define LOCTEXT_NAMESPACE "HUDFramework"

FString UHUDLayoutBlueprintLibrary::ConstructWidgetTreeString(const UUserWidget* UserWidget)
{
	FString Result = GetNameSafe(UserWidget);

	const UWidget* CurrentWidget = UserWidget;
	while (CurrentWidget && CurrentWidget->GetOuter())
	{
		CurrentWidget = Cast<UWidget>(CurrentWidget->GetOuter()->GetOuter());
		if (const UUserWidget* CurrentUserWidget = Cast<UUserWidget>(CurrentWidget))
		{
			Result += TEXT(" -> ") + GetNameSafe(CurrentUserWidget);
		}
	}

	return Result;
}

UHUDPrimaryLayout* UHUDLayoutBlueprintLibrary::GetPrimaryLayout(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		return nullptr;
	}
	
	if (const UHUDLayoutSubsystem* LayoutSubsystem = PlayerController->GetGameInstance()->GetSubsystem<UHUDLayoutSubsystem>())
	{
		if (const UHUDLayoutPolicy* Policy = LayoutSubsystem->GetPolicy())
		{
			return Policy->GetPrimaryLayout(PlayerController->GetLocalPlayer());
		}
	}

	return nullptr;
}

UCommonActivatableWidget* UHUDLayoutBlueprintLibrary::PushContentToLayer(const APlayerController* PlayerController, FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass, FHUDWidgetContextHandle WidgetContext)
{
	if (!IsValid(PlayerController) || !LayerTag.IsValid())
	{
		return nullptr;
	}

	if (UHUDPrimaryLayout* PrimaryLayout = GetPrimaryLayout(PlayerController))
	{
		return PrimaryLayout->PushWidgetToLayer(LayerTag, WidgetClass, WidgetContext);
	}

	return nullptr;
}

void UHUDLayoutBlueprintLibrary::PushStreamedContentToLayer(const APlayerController* PlayerController, FGameplayTag LayerTag, TSoftClassPtr<UCommonActivatableWidget> WidgetClass, FHUDWidgetContextHandle WidgetContext)
{
	if (!IsValid(PlayerController) || !LayerTag.IsValid() || WidgetClass == nullptr)
	{
		return;
	}

	if (UHUDPrimaryLayout* PrimaryLayout = GetPrimaryLayout(PlayerController))
	{
		PrimaryLayout->PushWidgetToLayerAsync(LayerTag, WidgetClass, WidgetContext);
	}
}

void UHUDLayoutBlueprintLibrary::PopContentFromLayer(const APlayerController* PlayerController, FGameplayTag LayerTag, UCommonActivatableWidget* Widget)
{
	if (!IsValid(PlayerController) || !LayerTag.IsValid() || Widget == nullptr)
	{
		return;
	}

	if (UHUDPrimaryLayout* PrimaryLayout = UHUDLayoutBlueprintLibrary::GetPrimaryLayout(PlayerController))
	{
		PrimaryLayout->PopWidgetFromLayer(LayerTag, Widget);
	}
}

void UHUDLayoutBlueprintLibrary::PopWidgetFromLayer(UCommonActivatableWidget* Widget)
{
	if (Widget != nullptr)
	{
		if (UHUDPrimaryLayout* PrimaryLayout = UHUDLayoutBlueprintLibrary::GetPrimaryLayout(Widget->GetOwningPlayer()))
		{
			PrimaryLayout->PopWidget(Widget);
		}
	}
}

void UHUDLayoutBlueprintLibrary::SetWidgetContext_FromHandle(UUserWidget* UserWidget, const FHUDWidgetContextHandle& ContextHandle)
{
	if (UHUDWidgetContextSubsystem* Subsystem = UHUDWidgetContextSubsystem::Get(UserWidget))
	{
		Subsystem->InitializeWidget(UserWidget, ContextHandle);
	}
}

void UHUDLayoutBlueprintLibrary::InitializeWidgetFromHandle(UUserWidget* UserWidget, const FHUDWidgetContextHandle& ContextHandle)
{
	if (UHUDWidgetContextSubsystem* Subsystem = UHUDWidgetContextSubsystem::Get(UserWidget))
	{
		Subsystem->InitializeWidget(UserWidget, ContextHandle);
	}
}

FHUDLayoutExtensionHandle UHUDLayoutBlueprintLibrary::RegisterLayoutExtension(const APlayerController* Player, const FGameplayTag SlotTag, TSubclassOf<UUserWidget> WidgetClass)
{
	if (UHUDLayoutSubsystem* LayoutSubsystem = UHUDLayoutSubsystem::Get(Player))
	{
		return LayoutSubsystem->RegisterLayoutExtension(SlotTag, WidgetClass, Player->GetLocalPlayer());
	}

	return {};
}

FHUDLayoutExtensionHandle UHUDLayoutBlueprintLibrary::RegisterLayoutExtensionWithContext(
	const APlayerController* Player, const FGameplayTag SlotTag, TSubclassOf<UUserWidget> WidgetClass,
	const FHUDWidgetContextHandle& WidgetContext)
{
	if (UHUDLayoutSubsystem* LayoutSubsystem = UHUDLayoutSubsystem::Get(Player))
	{
		return LayoutSubsystem->RegisterLayoutExtensionWithContext(SlotTag, WidgetClass, Player->GetLocalPlayer(), WidgetContext);
	}

	return {};
}

void UHUDLayoutBlueprintLibrary::UnregisterLayoutExtension(FHUDLayoutExtensionHandle& Handle)
{
	if (UHUDLayoutSubsystem* LayoutSubsystem = Handle.GetSubsystem())
	{
		LayoutSubsystem->UnregisterLayoutExtension(Handle);
	}
}

FHUDWidgetContextHandle UHUDLayoutBlueprintLibrary::GetWidgetContextHandle(const UUserWidget* UserWidget)
{
	if (UHUDWidgetContextSubsystem* Subsystem = UHUDWidgetContextSubsystem::Get(UserWidget))
	{
		return Subsystem->GetWidgetContext(UserWidget);
	}

	return {};
}

void UHUDLayoutBlueprintLibrary::K2_InitializeWidget(UUserWidget* UserWidget, const int32& WidgetContext)
{
	checkNoEntry();
}

void UHUDLayoutBlueprintLibrary::K2_GetWidgetContext(UUserWidget* UserWidget, int32& WidgetContext)
{
	checkNoEntry();
}

void UHUDLayoutBlueprintLibrary::K2_GetWidgetContextFromHandle(const FHUDWidgetContextHandle& ContextHandle, int32& WidgetContext)
{
	checkNoEntry();
}

FHUDWidgetContextHandle UHUDLayoutBlueprintLibrary::Conv_WidgetContextToWidgetContextHandle(const int32& WidgetContext)
{
	checkNoEntry();
	return {};
}

DEFINE_FUNCTION(UHUDLayoutBlueprintLibrary::execK2_InitializeWidget)
{
	P_GET_OBJECT(UUserWidget, UserWidget);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const void* ContextData = Stack.MostRecentPropertyAddress;
	const FStructProperty* ContextProperty = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH
	P_NATIVE_BEGIN
	
	if (ContextData == nullptr || ContextProperty == nullptr)
	{
		FBlueprintExceptionInfo Exception{EBlueprintExceptionType::AbortExecution, LOCTEXT("K2_InitializeWidget_InvalidMemory", "")};
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, Exception);
		return;
	}
	
	UScriptStruct* ContextType = Cast<UScriptStruct>(ContextProperty->Struct);
	if (!ContextType->IsChildOf(FHUDWidgetContextProxy::StaticStruct()))
	{
		FBlueprintExceptionInfo Exception{EBlueprintExceptionType::AbortExecution, LOCTEXT("GetWidgetContextFromHandle_InvalidType", "")};
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, Exception);
		return;
	}
	
	const FHUDWidgetContextHandle ContextHandle{ContextType, ContextData};
	InitializeWidgetFromHandle(UserWidget, ContextHandle);
	P_NATIVE_END
}

DEFINE_FUNCTION(UHUDLayoutBlueprintLibrary::execConv_WidgetContextToWidgetContextHandle)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const void* ContextData = Stack.MostRecentPropertyAddress;
	const FStructProperty* ContextProperty = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH
	P_NATIVE_BEGIN
	if (ContextData == nullptr || ContextProperty == nullptr)
	{
		FBlueprintExceptionInfo Exception{EBlueprintExceptionType::AbortExecution, LOCTEXT("K2_InitializeWidget_InvalidMemory", "")};
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, Exception);
		return;
	}
	
	UScriptStruct* ContextType = Cast<UScriptStruct>(ContextProperty->Struct);
	if (!ContextType->IsChildOf(FHUDWidgetContextProxy::StaticStruct()))
	{
		FBlueprintExceptionInfo Exception{EBlueprintExceptionType::AbortExecution, LOCTEXT("GetWidgetContextFromHandle_InvalidType", "")};
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, Exception);
		return;
	}

	*(FHUDWidgetContextHandle*)RESULT_PARAM = FHUDWidgetContextHandle{ContextType, ContextData};
	P_NATIVE_END
}

DEFINE_FUNCTION(UHUDLayoutBlueprintLibrary::execK2_GetWidgetContext)
{
	P_GET_OBJECT(UUserWidget, UserWidget);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	void* OutData = Stack.MostRecentPropertyAddress;
	const FStructProperty* ContextProperty = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH
	P_NATIVE_BEGIN
	if (OutData == nullptr || ContextProperty == nullptr)
	{
		FBlueprintExceptionInfo Exception{EBlueprintExceptionType::AbortExecution, LOCTEXT("GetWidgetContextFromHandle_InvalidMemory", "")};
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, Exception);
		return;
	}

	FHUDWidgetContextHandle ContextHandle = GetWidgetContextHandle(UserWidget);
	const UScriptStruct* ContextType = Cast<UScriptStruct>(ContextProperty->Struct);
	if (!ContextHandle.IsDerivedFrom(ContextType))
	{
		FBlueprintExceptionInfo Exception{EBlueprintExceptionType::AbortExecution, LOCTEXT("GetWidgetContextFromHandle_InvalidType", "")};
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, Exception);
		return;
	}
	
	ContextType->CopyScriptStruct(OutData, &ContextHandle.GetContext());
	P_NATIVE_END
}

DEFINE_FUNCTION(UHUDLayoutBlueprintLibrary::execK2_GetWidgetContextFromHandle)
{
	P_GET_STRUCT_REF(FHUDWidgetContextHandle, ContextHandle);

	if (!ContextHandle.IsValid())
	{
		return;
	}

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	void* OutContextData = Stack.MostRecentPropertyAddress;
	const FStructProperty* OutContextProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	
	P_FINISH
	P_NATIVE_BEGIN
	if (OutContextData == nullptr || OutContextProperty == nullptr)
	{
		FBlueprintExceptionInfo Exception{EBlueprintExceptionType::AbortExecution, LOCTEXT("GetWidgetContextFromHandle_Invalid", "")};
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, Exception);
		return;
	}
	
	const UScriptStruct* ContextType = Cast<UScriptStruct>(OutContextProperty->Struct);
	if (!ContextHandle.IsDerivedFrom(ContextType))
	{
		FBlueprintExceptionInfo Exception{EBlueprintExceptionType::AbortExecution, LOCTEXT("GetWidgetContextFromHandle_InvalidType", "")};
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, Exception);
		return;
	}

	ContextType->CopyScriptStruct(OutContextData, &ContextHandle.GetContext());
	P_NATIVE_END
}

bool UHUDLayoutBlueprintLibrary::IsValid_WidgetContext(const FHUDWidgetContextHandle& WidgetContext)
{
	return WidgetContext.IsValid();
}

bool UHUDLayoutBlueprintLibrary::IsDerivedFrom_WidgetContext(const FHUDWidgetContextHandle& WidgetContext, UScriptStruct* ContextType)
{
	return WidgetContext.IsDerivedFrom(ContextType);
}

UObject* UHUDLayoutBlueprintLibrary::GetContextObject_WidgetContext(const FHUDWidgetContextHandle& WidgetContext)
{
	if (!WidgetContext.IsValid())
	{
		return nullptr;
	}

	const FHUDWidgetContext& Context = WidgetContext.GetContext<FHUDWidgetContext>();
	return Context.ContextObject;
}

bool UHUDLayoutBlueprintLibrary::IsValid_SlotHandle(FHUDLayoutSlotHandle& Handle)
{
	return Handle.IsValid();
}

void UHUDLayoutBlueprintLibrary::Unregister_SlotHandle(FHUDLayoutSlotHandle& Handle)
{
	if (Handle.IsValid() && Handle.Subsystem.IsValid())
	{
		UHUDLayoutSubsystem* Subsystem = Handle.Subsystem.Get();
		Subsystem->UnregisterLayoutSlot(Handle);
	}
}

bool UHUDLayoutBlueprintLibrary::IsValid_ExtensionHandle(FHUDLayoutExtensionHandle& Handle)
{
	return Handle.IsValid();
}

void UHUDLayoutBlueprintLibrary::Unregister_ExtensionHandle(FHUDLayoutExtensionHandle& Handle)
{
	if (Handle.IsValid() && Handle.Subsystem.IsValid())
	{
		UHUDLayoutSubsystem* Subsystem = Handle.Subsystem.Get();
		Subsystem->UnregisterLayoutExtension(Handle);
	}
}

#undef LOCTEXT_NAMESPACE