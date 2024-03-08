// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUDUserWidgetPool.h"

FHUDUserWidgetPool::FHUDUserWidgetPool(UWidget& InOwningWidget)
	: OwningWidget(&InOwningWidget)
{}

FHUDUserWidgetPool::~FHUDUserWidgetPool()
{
	ResetPool();
}

void FHUDUserWidgetPool::SetWorld(UWorld* InOwningWorld)
{
	OwningWorld = InOwningWorld;
}

void FHUDUserWidgetPool::SetDefaultPlayerController(APlayerController* InDefaultPlayerController)
{
	DefaultPlayerController = InDefaultPlayerController;
}

void FHUDUserWidgetPool::RebuildWidgets()
{
	for (UUserWidget* Widget : ActiveWidgets)
	{
		CachedSlateByWidgetObject.Add(Widget, Widget->TakeWidget());
	}
}

void FHUDUserWidgetPool::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObjects<UUserWidget>(ActiveWidgets, OwningWidget.Get());
	Collector.AddReferencedObjects<UUserWidget>(InactiveWidgets, OwningWidget.Get());
}

void FHUDUserWidgetPool::Release(UUserWidget* Widget, bool bReleaseSlate)
{
	if (Widget != nullptr)
	{
		const int32 ActiveWidgetIdx = ActiveWidgets.Find(Widget);
		if (ActiveWidgetIdx != INDEX_NONE)
		{
			InactiveWidgets.Push(Widget);
			ActiveWidgets.RemoveAt(ActiveWidgetIdx);

			if (bReleaseSlate)
			{
				CachedSlateByWidgetObject.Remove(Widget);
			}
		}
	}
}

void FHUDUserWidgetPool::Release(TArray<UUserWidget*> Widgets, bool bReleaseSlate)
{
	for (UUserWidget* Widget : Widgets)
	{
		Release(Widget, bReleaseSlate);
	}
}

void FHUDUserWidgetPool::ReleaseAll(bool bReleaseSlate)
{
	InactiveWidgets.Append(ActiveWidgets);
	ActiveWidgets.Empty();

	if (bReleaseSlate)
	{
		CachedSlateByWidgetObject.Reset();
	}
}

void FHUDUserWidgetPool::ResetPool()
{
	InactiveWidgets.Reset();
	ActiveWidgets.Reset();
	CachedSlateByWidgetObject.Reset();
}

void FHUDUserWidgetPool::ReleaseInactiveSlateResources()
{
	for (UUserWidget* InactiveWidget : InactiveWidgets)
	{
		CachedSlateByWidgetObject.Remove(InactiveWidget);
	}
}

void FHUDUserWidgetPool::ReleaseAllSlateResources()
{
	CachedSlateByWidgetObject.Reset();
}

