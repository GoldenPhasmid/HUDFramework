#include "HUDLayoutSlotWidget.h"

#include "HUDLayoutExtension.h"
#include "HUDLayoutSubsystem.h"
#include "ViewModel/HUDWidgetContextSubsystem.h"

UHUDLayoutSlotWidget::UHUDLayoutSlotWidget(const FObjectInitializer& Initializer): Super(Initializer)
{
	
}

void UHUDLayoutSlotWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		UnregisterSlot();
		ResetInternal();
	}
	
	Super::ReleaseSlateResources(bReleaseChildren);
}

TSharedRef<SWidget> UHUDLayoutSlotWidget::RebuildWidget()
{
	if (IsDesignTime())
	{
		return
		SNew(SOverlay)
		+ SOverlay::Slot()
		.Padding(5.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Justification(ETextJustify::Center)
			.Text_Lambda([this]()
			{
				return FText::Format(FText::FromString("Extension Point\n{0}"), FText::FromString(SlotTag.ToString()));
			})
		];
	}
	else
	{
		RegisterSlot();

		TSharedRef<SWidget> Widget = Super::RebuildWidget();

		check(MyPanelWidget.IsValid());
		for (UUserWidget* PendingWidget: PendingWidgets)
		{
			AddEntryChild(*PendingWidget);
		}
		PendingWidgets.Reset();

		return Widget;
	}
	
}

void UHUDLayoutSlotWidget::RegisterSlot()
{
	if (Handles.Num() > 0)
	{
		UnregisterSlot();
	}

	if (UHUDLayoutSubsystem* LayoutSubsystem = GetGameInstance()->GetSubsystem<UHUDLayoutSubsystem>())
	{
		// register for owning local player
		FHUDLayoutSlotHandle Handle = LayoutSubsystem->RegisterLayoutSlot(SlotTag,
			GetOwningLocalPlayer(),
			[this](const FHUDLayoutExtensionRequest& Request) { return AddExtension(Request);	},
			[this](const FHUDLayoutExtensionRequest& Request) { return RemoveExtension(Request); }
		);
		Handles.Add(Handle);
	}
}

void UHUDLayoutSlotWidget::UnregisterSlot()
{
	if (Handles.Num() == 0)
	{
		return;
	}
	
	if (UHUDLayoutSubsystem* LayoutSubsystem = GetGameInstance()->GetSubsystem<UHUDLayoutSubsystem>())
	{
		for (FHUDLayoutSlotHandle& Handle: Handles)
		{
			LayoutSubsystem->UnregisterLayoutSlot(Handle);
		}
		Handles.Reset();
	}
}

UUserWidget* UHUDLayoutSlotWidget::AddExtension(const FHUDLayoutExtensionRequest& Request)
{
	// not using widget pool, because it constructs widget even before adding it to panel widget
	UUserWidget* Widget = CreateWidget<UUserWidget>(this, Request.WidgetClass);
	// initialize widget with widget context using specified subsystem. It is done so we can add this widget as a child right away
	if (UHUDWidgetContextSubsystem* Subsystem = UHUDWidgetContextSubsystem::Get(this))
	{
		Subsystem->InitializeWidget(Widget, Request.WidgetContext);
	}
	if (MyPanelWidget.IsValid())
	{
		AddEntryChild(*Widget);
	}
	else
	{
		PendingWidgets.Add(Widget);
	}
	
	if (ConfigureWidget.IsBound())
	{
		const UObject* DataObject = nullptr;
		if (Request.WidgetContext.IsValid())
		{
			DataObject = Request.WidgetContext.GetContext<FHUDWidgetContext>().DataObject;
		}
		ConfigureWidget.Execute(Widget, DataObject);
	}
	ActiveExtensions.Add(Request.Handle, Widget);

	return Widget;
}

UUserWidget* UHUDLayoutSlotWidget::RemoveExtension(const FHUDLayoutExtensionRequest& Request)
{
	if (UUserWidget* Widget = ActiveExtensions.FindRef(Request.Handle))
	{
		RemoveEntryInternal(Widget);
		ActiveExtensions.Remove(Request.Handle);

		return Widget;
	}

	return nullptr;
}
