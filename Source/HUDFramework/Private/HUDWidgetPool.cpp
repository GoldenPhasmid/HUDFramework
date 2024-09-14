// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUDWidgetPool.h"

FHUDWidgetPool::FHUDWidgetPool(UWidget& InOwningWidget)
	: OwningWidget(&InOwningWidget)
{}

FHUDWidgetPool::~FHUDWidgetPool()
{
	ResetPool();
}

void FHUDWidgetPool::SetWorld(UWorld* InOwningWorld)
{
	OwningWorld = InOwningWorld;
}

void FHUDWidgetPool::SetDefaultPlayerController(APlayerController* InDefaultPlayerController)
{
	DefaultPlayerController = InDefaultPlayerController;
}

void FHUDWidgetPool::RebuildWidgets()
{
	for (UUserWidget* Widget : ActiveWidgets)
	{
		CachedSlateByWidgetObject.Add(Widget, Widget->TakeWidget());
	}
}

void FHUDWidgetPool::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObjects<UUserWidget>(ActiveWidgets, OwningWidget.Get());
	Collector.AddReferencedObjects<UUserWidget>(InactiveWidgets, OwningWidget.Get());
}

void FHUDWidgetPool::Release(UUserWidget* Widget)
{
	if (Widget != nullptr)
	{
		const int32 ActiveWidgetIdx = ActiveWidgets.Find(Widget);
		if (ActiveWidgetIdx != INDEX_NONE)
		{
			InactiveWidgets.Push(Widget);
			ActiveWidgets.RemoveAt(ActiveWidgetIdx);

			CachedSlateByWidgetObject.Remove(Widget);
		}
	}
}

void FHUDWidgetPool::Release(TArray<UUserWidget*> Widgets)
{
	for (int32 Index = Widgets.Num() - 1; Index >= 0; --Index)
	{
		Release(Widgets[Index]);
	}
}

void FHUDWidgetPool::ReleaseAll()
{
	InactiveWidgets.Append(ActiveWidgets);
	ActiveWidgets.Empty();

	CachedSlateByWidgetObject.Reset();
}

void FHUDWidgetPool::ResetPool()
{
	InactiveWidgets.Reset();
	ActiveWidgets.Reset();
	CachedSlateByWidgetObject.Reset();
}

void FHUDWidgetPool::ReleaseInactiveSlateResources()
{
	for (UUserWidget* InactiveWidget : InactiveWidgets)
	{
		CachedSlateByWidgetObject.Remove(InactiveWidget);
	}
}

void FHUDWidgetPool::ReleaseAllSlateResources()
{
	CachedSlateByWidgetObject.Reset();
}

