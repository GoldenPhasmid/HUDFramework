#include "Indicators/IndicatorCanvas.h"

#include "HUDFramework.h"
#include "Indicators/IndicatorDescriptor.h"
#include "Indicators/IndicatorManagerComponent.h"
#include "Indicators/IndicatorWidgetInterface.h"
#include "ViewModel/HUDWidgetContextSubsystem.h"

// Hope this namespace helps understand code better
namespace Private
{
	enum class EDirection : uint8
	{
		Left,
		Top,
		Right,
		Bottom,
		MAX
	};

	constexpr float ArrowRotations[int32(EDirection::MAX)] =
	{
		270.f,
		0.f,
		90.f,
		180.f
	};

	const FVector2D ArrowOffsets[int32(EDirection::MAX)] =
	{
		FVector2D(-1.f, 0.f),
		FVector2D(0.f, -1.f),
		FVector2D(1.f, 0.f),
		FVector2D(0.f, 1.f)
	};

	struct FSlotSizeAndOffset
	{
		explicit FSlotSizeAndOffset(const SIndicatorCanvas::FSlot& IndicatorSlot, bool bApplyScale = false);

		FVector2D Size = FVector2D::ZeroVector;
		FVector2D Offset = FVector2D::ZeroVector;
		FVector2D PaddingMin = FVector2D::ZeroVector;
		FVector2D PaddingMax = FVector2D::ZeroVector;
	};
}

int32 SIndicatorCanvasArrowWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 MaxLayerId = LayerId;
	
	if (ArrowBrush != nullptr)
	{
		const bool bShouldBeEnabled = ShouldBeEnabled(bParentEnabled);
		const ESlateDrawEffect DrawEffect = bShouldBeEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
		const FColor FinalColorAndOpacity = (InWidgetStyle.GetColorAndOpacityTint() * ArrowBrush->GetTint(InWidgetStyle)).ToFColor(true);
	
		FSlateDrawElement::MakeRotatedBox(
			OutDrawElements,
			MaxLayerId,
			AllottedGeometry.ToPaintGeometry(ArrowBrush->ImageSize, FSlateLayoutTransform()),
			ArrowBrush, DrawEffect,
			FMath::DegreesToRadians(Rotation),
			TOptional<FVector2D>(),
			FSlateDrawElement::RelativeToElement,
			FinalColorAndOpacity);
	
		MaxLayerId++;
	}
	
	return MaxLayerId;
}

FVector2D SIndicatorCanvasArrowWidget::ComputeDesiredSize(float) const
{
	if (ArrowBrush != nullptr)
	{
		return ArrowBrush->GetImageSize();
	}
	return FVector2D::ZeroVector;
}

void SIndicatorCanvasArrowWidget::Construct(const FArguments& InArgs, const FSlateBrush* InArrowBrush)
{
	this->ArrowBrush = InArrowBrush;
	SetCanTick(false);
}

void SIndicatorCanvas::Construct(const FArguments& InArgs, const FLocalPlayerContext& InLocalPlayerContext, const FGameplayTagContainer& InCategoryTags, const FSlateBrush* InArrowBrush)
{
	LocalPlayerContext = InLocalPlayerContext;
	CategoryTags = InCategoryTags;
	ArrowBrush = InArrowBrush;

	IndicatorPool.SetWorld(LocalPlayerContext.GetWorld());

	SetCanTick(false);
	SetVisibility(EVisibility::SelfHitTestInvisible);
	
	UpdateActiveTimer();
}

void SIndicatorCanvas::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	FScopedArrowChildren ScopedArrowChildren(&ArrowChildren, ArrowBrush);

	if (bShowAnyIndicators)
	{
		TArray<const FSlot*> SortedSlots;
		// Reserve space for slots
		SortedSlots.Reserve(SlotChildren.Num());
		// Copy slot children
		for (int32 ChildIndex = 0; ChildIndex < SlotChildren.Num(); ChildIndex++)
		{
			SortedSlots.Add(&SlotChildren[ChildIndex]);
		}

		// Sort children with priority
		SortedSlots.StableSort([](const FSlot& A, const FSlot& B)
		{
			return A.GetPriority() == B.GetPriority() ? A.GetDepth() > B.GetDepth() : A.GetPriority() < B.GetPriority();
		});

		for (const FSlot*& Slot : SortedSlots)
		{
			const TSharedRef<FIndicatorDescriptorInstance> IndicatorInstance = Slot->GetIndicatorDescriptorInstance();

			// Skip indicator if it is not match requirements
			if (!ArrangedChildren.Accepts(Slot->GetWidget()->GetVisibility()) || Slot->ShouldSkipIndicator())
			{
				Slot->SetWasIndicatorClamped(false);
				continue;
			}

			const float IndicatorScale = Slot->GetIndicatorScale();

			bool bWasIndicatorClamped = false;
			FVector2D ClampedScreenPosition = FVector2D::ZeroVector;
			if (IndicatorInstance->Descriptor->bClampToScreen)
			{
				const uint8 ClampDirection =
					ClampIndicator(*Slot, AllottedGeometry.GetLocalSize(), ClampedScreenPosition);

				bWasIndicatorClamped = static_cast<Private::EDirection>(ClampDirection) != Private::EDirection::MAX;

				// Should we draw arrow
				if (bWasIndicatorClamped && IndicatorInstance->Descriptor->bShowClampToScreenArrow)
				{
					AddArrowWidget(
						AllottedGeometry,
						ArrangedChildren,
						ScopedArrowChildren,
						ClampDirection,
						ClampedScreenPosition,
						Slot->GetWidget()->GetDesiredSize() * IndicatorScale,
						IndicatorInstance->Descriptor->VerticalAlignment);
				}
			}
			
			Slot->SetWasIndicatorClamped(bWasIndicatorClamped);

			FVector2D ScreenPosition = bWasIndicatorClamped ? ClampedScreenPosition : Slot->GetScreenPosition();

			// Get params without scale because it will be applied by slate.
			Private::FSlotSizeAndOffset Params(*Slot);
			
			const FLayoutGeometry Geometry(FSlateLayoutTransform(IndicatorScale, ScreenPosition + Params.Offset * IndicatorScale), Params.Size);
			ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Slot->GetWidget(), Geometry));
		}
	}
}

int32 SIndicatorCanvas::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	CachedAllottedGeometry = AllottedGeometry;

	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	ArrangeChildren(AllottedGeometry, ArrangedChildren);

	int32 MaxLayerId = LayerId;

	const FPaintArgs NewArgs = Args.WithNewParent(this);
	const bool bShouldBeEnabled = ShouldBeEnabled(bParentEnabled);

	for (const FArrangedWidget& ArrangedChild : ArrangedChildren.GetInternalArray())
	{
		if (!IsChildWidgetCulled(MyCullingRect, ArrangedChild))
		{
			const int32 WidgetMaxLayerId = ArrangedChild.Widget->Paint(
				NewArgs, ArrangedChild.Geometry, MyCullingRect,OutDrawElements, LayerId, InWidgetStyle, bShouldBeEnabled);

			MaxLayerId = FMath::Max(MaxLayerId, WidgetMaxLayerId);
		}
	}

	return MaxLayerId;
}

void SIndicatorCanvas::ProjectIndicator(const TSharedRef<FIndicatorDescriptorInstance>& Instance, const FVector2f& ScreenSize, FIndicatorProjectionResult& Result)
{
	Instance->Descriptor->ProjectionMode->Project(
		Instance->Component,
		Instance->SocketName,
		LocalPlayerContext,
		ScreenSize,
		Result);
}

void SIndicatorCanvas::UpdateActiveTimer()
{
	const bool bNeedsTicks = SlotChildren.Num() > 0 || !IndicatorManager.IsValid();

	if (bNeedsTicks && !TickHandle.IsValid())
	{
		TickHandle = RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SIndicatorCanvas::UpdateCanvas));
	}
}

EActiveTimerReturnType SIndicatorCanvas::UpdateCanvas(double InCurrentTime, float InDeltaTime)
{
	if (!CachedAllottedGeometry.IsSet())
	{
		return EActiveTimerReturnType::Continue;
	}

	if (!WidgetContextSubsystem.IsValid())
	{
		WidgetContextSubsystem = UHUDWidgetContextSubsystem::Get(LocalPlayerContext.GetWorld());
	}

	if (!IndicatorManager.IsValid())
	{
		IndicatorManager = UIndicatorManagerComponent::Get(LocalPlayerContext.GetWorld());

		// New indicator manager found.
		if (IndicatorManager.IsValid())
		{
			OnIndicatorManagerChanged();
		}
		else
		{
			check(SlotChildren.Num() == 0);
			return EActiveTimerReturnType::Continue;
		}
	}

	if (LocalPlayerContext.IsValid())
	{
		SetShowAnyIndicators(true);

		if (UpdateIndicators())
		{
			Invalidate(EInvalidateWidgetReason::Paint);
		}
	}
	else
	{
		SetShowAnyIndicators(false);
	}

	if (SlotChildren.Num() == 0)
	{
		TickHandle.Reset();
		return EActiveTimerReturnType::Stop;
	}

	return EActiveTimerReturnType::Continue;
}

void SIndicatorCanvas::HandleIndicatorAdded(const TSharedRef<FIndicatorDescriptorInstance>& IndicatorInstance)
{
	// Skip this indicator if this is not our category
	if (!CategoryTags.HasTagExact(IndicatorInstance->Descriptor->CategoryTag))
	{
		return;
	}
	
	// Dont check on validity because of meta = (Validate)
	TSoftClassPtr<UUserWidget> IndicatorWidgetClass = IndicatorInstance->Descriptor->IndicatorWidgetClass;

	// Make weak pointer. Indicator can be removed during loading
	TWeakPtr<FIndicatorDescriptorInstance> WeakInstance = IndicatorInstance;
	
	AsyncLoad(IndicatorWidgetClass, [this, WeakInstance, IndicatorWidgetClass]()
	{
		if (const TSharedPtr<FIndicatorDescriptorInstance> SharedInstance = WeakInstance.Pin())
		{
			UUserWidget* IndicatorWidget = IndicatorPool.GetOrCreateInstance(
				TSubclassOf<UUserWidget>(IndicatorWidgetClass.Get()),
			[this, SharedInstance](UUserWidget* UserWidget)
			{
				if (WidgetContextSubsystem.IsValid())
				{
					WidgetContextSubsystem->InitializeWidget_FromWidgetPool(IndicatorPool, UserWidget, SharedInstance->WidgetContext);
				}
			});

			if (IndicatorWidget->Implements<UIndicatorWidgetInterface>())
			{
				IIndicatorWidgetInterface::Execute_SetIndicator(IndicatorWidget, SharedInstance->Descriptor, SharedInstance->Component);
			}

			AddIndicatorSlot(SharedInstance.ToSharedRef(), IndicatorWidget)
			[
				SNew(SBox)
				[
					IndicatorWidget->TakeWidget()
				]
			];
		}
	});
	StartAsyncLoading();
}


void SIndicatorCanvas::HandleIndicatorRemoved(const TSharedRef<FIndicatorDescriptorInstance>& IndicatorInstance)
{
	// Skip this indicator if this is not our category
	if (!CategoryTags.HasTagExact(IndicatorInstance->Descriptor->CategoryTag))
	{
		return;
	}
	
	for (int32 Index = 0; Index < SlotChildren.Num(); Index++)
	{
		if (SlotChildren[Index].GetIndicatorDescriptorInstance() == IndicatorInstance)
		{
			const TWeakObjectPtr<UUserWidget> IndicatorWidget = SlotChildren[Index].GetUserWidget();
			if (IndicatorWidget.IsValid())
			{
				if (IndicatorWidget->GetClass()->ImplementsInterface(UIndicatorWidgetInterface::StaticClass()))
				{
					IIndicatorWidgetInterface::Execute_ResetIndicator(IndicatorWidget.Get());
				}

				IndicatorPool.Release(IndicatorWidget.Get());
			}
			else
			{
				UE_LOG(LogIndicators, Error, TEXT("%s: Indicator widget was destroyed before slot removal!"), *FString(__FUNCTION__));
			}
			
			RemoveIndicatorSlot(Index);
			break;
		}
	}
}

SIndicatorCanvas::FScopedWidgetSlotArguments SIndicatorCanvas::AddIndicatorSlot(const TSharedRef<FIndicatorDescriptorInstance>& IndicatorInstance, UUserWidget* IndicatorWidget)
{
	TWeakPtr<SIndicatorCanvas> WeakCanvas = SharedThis(this);
	return FScopedWidgetSlotArguments(MakeUnique<FSlot>(IndicatorInstance, IndicatorWidget), SlotChildren, INDEX_NONE,
		[WeakCanvas](const FSlot*, int32)
		{
			if (TSharedPtr<SIndicatorCanvas> Canvas = WeakCanvas.Pin())
			{
				Canvas->UpdateActiveTimer();
			}
		});
}

void SIndicatorCanvas::RemoveIndicatorSlot(int32 Index)
{
	SlotChildren.RemoveAt(Index);

	UpdateActiveTimer();
}

void SIndicatorCanvas::SetShowAnyIndicators(bool InValue)
{
	if (bShowAnyIndicators == InValue)
	{
		return;
	}

	bShowAnyIndicators = InValue;

	if (!bShowAnyIndicators)
	{
		for (int32 ChildIndex = 0; ChildIndex < AllChildren.Num(); ChildIndex++)
		{
			AllChildren.GetChildAt(ChildIndex)->SetVisibility(EVisibility::Collapsed);
		}
		
	}	
}

void SIndicatorCanvas::OnIndicatorManagerChanged()
{
	// World may have changed
	IndicatorPool.SetWorld(LocalPlayerContext.GetWorld());

	IndicatorManager->OnIndicatorAdded.AddSP(this, &SIndicatorCanvas::HandleIndicatorAdded);
	IndicatorManager->OnIndicatorRemoved.AddSP(this, &SIndicatorCanvas::HandleIndicatorRemoved);
	for (const TSharedPtr<FIndicatorDescriptorInstance>& Instance : IndicatorManager->GetIndicators())
	{
		HandleIndicatorAdded(Instance.ToSharedRef());
	}
}

bool SIndicatorCanvas::UpdateIndicators()
{
	bool bWasIndicatorsChanged = false;

	const FGeometry AllottedGeometry = CachedAllottedGeometry.GetValue();

	for (int32 ChildIndex = 0; ChildIndex < SlotChildren.Num(); ChildIndex++)
	{
		FSlot& Slot = SlotChildren[ChildIndex];
		const TSharedPtr<FIndicatorDescriptorInstance> Indicator = Slot.GetIndicatorDescriptorInstance();

		if (Slot.WasUserWidgetManuallyCollapsed())
		{
			continue;
		}
		
		if (Slot.WasIndicatorClampedStatusChanged())
		{
			Slot.ClearIndicatorClampedStatusChangedFlag();
			bWasIndicatorsChanged = true;
		}

		FIndicatorProjectionResult Result;
		ProjectIndicator(Indicator.ToSharedRef(), AllottedGeometry.GetLocalSize(), Result);
		
		Slot.SetHasValidScreenPosition(Result.bSuccess);

		if (Slot.HasValidScreenPosition())
		{
			Slot.SetScreenPosition(FVector2D(Result.ScreenPositionWithDepth));
			Slot.SetDepth(Result.ScreenPositionWithDepth.Z);
			Slot.SetPriority(Indicator->Descriptor->Priority);
		}
		
		bWasIndicatorsChanged |= Slot.IsDirty();
		Slot.ClearDirtyFlag();
	}
	
	return bWasIndicatorsChanged;
}

uint8 SIndicatorCanvas::ClampIndicator(const FSlot& IndicatorSlot, const FVector2D& ScreenSize, FVector2D& OutClampedScreenPosition) const
{
	Private::EDirection ClampDirection = Private::EDirection::MAX;
	const Private::FSlotSizeAndOffset Params(IndicatorSlot, true);

	const FVector2D ArrowImageSize = ArrowBrush->GetImageSize();
	const FIntPoint FixedPadding = MinClampPadding + FIntPoint(ArrowImageSize.X, ArrowImageSize.Y);
	const FIntPoint RectMin = FIntPoint(Params.PaddingMin.X, Params.PaddingMin.Y) + FixedPadding;
	const FIntPoint RectMax = FIntPoint(ScreenSize.X - Params.PaddingMax.X, ScreenSize.Y - Params.PaddingMax.Y) - FixedPadding;
	const FIntRect ClampRect = FIntRect(RectMin, RectMax);
	const FVector Center = FVector(ScreenSize * 0.5f, 0.f);

	const FVector2D CurrentScreenPosition = IndicatorSlot.GetScreenPosition();

	if (!ClampRect.Contains(FIntPoint(CurrentScreenPosition.X, CurrentScreenPosition.Y)))
	{
		const FPlane Planes[] =
		{
			FPlane(FVector(1.f, 0.f, 0.f), ClampRect.Min.X), //Left
			FPlane(FVector(0.f, 1.f, 0.f), ClampRect.Min.Y), // Top
			FPlane(FVector(-1.f, 0.f, 0.f), -ClampRect.Max.X), //Right
			FPlane(FVector(0.f, -1.f, 0.f), -ClampRect.Max.Y) //Bottom
		};
		
		FVector OutIntersectionPoint = FVector::ZeroVector;
		for (int32 Index = 0; Index < static_cast<int32>(Private::EDirection::MAX); ++Index)
		{
			bool bSuccess = FMath::SegmentPlaneIntersection(
				Center,
				FVector(CurrentScreenPosition, 0.f),
				Planes[Index],
				OutIntersectionPoint);

			bSuccess &= FIntRect(RectMin - 1, RectMax + 1).Contains(FIntPoint(OutIntersectionPoint.X, OutIntersectionPoint.Y));
			
			if (bSuccess)
			{
				ClampDirection = static_cast<Private::EDirection>(Index);
				OutClampedScreenPosition = FVector2D(OutIntersectionPoint);
			}
		}
	}
	
	return static_cast<uint8>(ClampDirection);
}

SIndicatorCanvas::FScopedArrowChildren::~FScopedArrowChildren()
{
	int32 InactiveArrows = 0;
	// Disable all arrows that dont use
	while (Index < ArrowChildren.Num())
	{
		// Check max number of inactive arrows reached.
		if (InactiveArrows == MAX_INACTIVE_ARROWS)
		{
			// Remove arrow slot. Dont increment index in removal operation.
			ArrowChildren.RemoveAt(Index);
			continue;
		}

		//Deactivate arrow widget and increment Index.
		ArrowChildren.GetChildAt(Index++)->SetVisibility(EVisibility::Collapsed);
		InactiveArrows++;
	}
}

TSharedRef<SIndicatorCanvasArrowWidget> SIndicatorCanvas::FScopedArrowChildren::GetOrCreateArrowChild()
{
	if (Index < ArrowChildren.Num())
	{
		return StaticCastSharedRef<SIndicatorCanvasArrowWidget>(ArrowChildren.GetChildAt(Index++));
	}

	if (Index == ArrowChildren.Num())
	{
		TSharedRef<SIndicatorCanvasArrowWidget> NewArrowWidget = SNew(SIndicatorCanvasArrowWidget, ArrowBrush);
		NewArrowWidget->SetVisibility(EVisibility::Collapsed);

		ArrowChildren.AddSlot(MoveTemp(FArrowSlot::FSlotArguments(MakeUnique<FArrowSlot>())
		[
			NewArrowWidget
		]));

		Index++;

		return NewArrowWidget;
	}

	// Should never happen
	checkf(false, TEXT("%s: Index out of range!"), *FString(__FUNCTION__));
	return TSharedPtr<SIndicatorCanvasArrowWidget>().ToSharedRef();
}

void SIndicatorCanvas::AddArrowWidget(const FGeometry& AllottedGeometry, FArrangedChildren& ToArrangedChildren, FScopedArrowChildren& FromScopedChildren, uint8 InArrowDirection, const FVector2D& IndicatorPosition, const FVector2D& IndicatorSize, EVerticalAlignment IndicatorVAlignment) const
{
	const Private::EDirection ArrowDirection = static_cast<Private::EDirection>(InArrowDirection);
	const FVector2D ArrowOffsetDirection = Private::ArrowOffsets[int32(ArrowDirection)];
	const float ArrowRotation = Private::ArrowRotations[int32(ArrowDirection)];
	const FVector2D ArrowWidgetSize = ArrowBrush->GetImageSize();

	// Calculating arrow position
	const FVector2D WidgetOffset = (IndicatorSize + ArrowWidgetSize) * 0.5f * ArrowOffsetDirection;
	const FVector2D ArrowCenteringOffset = -(ArrowWidgetSize * 0.5f);
	FVector2D ArrowAlignmentOffset = FVector2D::ZeroVector;
	switch (IndicatorVAlignment)
	{
	case VAlign_Top:
		ArrowAlignmentOffset = IndicatorSize * FVector2D(0.f, 0.5f);
		break;
	case VAlign_Bottom:
		ArrowAlignmentOffset = IndicatorSize * FVector2D(0.f, -0.5f);
		break;
	default: break;
	}
	const FVector2D FinalOffset = WidgetOffset + ArrowAlignmentOffset + ArrowCenteringOffset;
	const FVector2D FinalPosition = IndicatorPosition + FinalOffset;

	// Grab arrow from scoped children
	const TSharedRef<SIndicatorCanvasArrowWidget> ArrowWidget = FromScopedChildren.GetOrCreateArrowChild();
	ArrowWidget->SetRotation(ArrowRotation);
	ArrowWidget->SetVisibility(EVisibility::HitTestInvisible);

	const FLayoutGeometry Geometry(FSlateLayoutTransform(1.f, FinalPosition), ArrowWidgetSize);
	ToArrangedChildren.AddWidget(AllottedGeometry.MakeChild(ArrowWidget, Geometry));
}

Private::FSlotSizeAndOffset::FSlotSizeAndOffset(const SIndicatorCanvas::FSlot& IndicatorSlot, bool bApplyScale)
{
	const float Scale = bApplyScale ? IndicatorSlot.GetIndicatorScale() : 1.f;
	
	// Grab box from FSlot. Conversation from TSharedRef, object must be valid
	Size = IndicatorSlot.GetWidget()->GetDesiredSize() * Scale;
	const UIndicatorDescriptor* Descriptor = IndicatorSlot.GetIndicatorDescriptorInstance()->Descriptor;

	switch (Descriptor->HorizontalAlignment)
	{
	case HAlign_Left:
		Offset.X = 0.f;
		PaddingMin.X = 0.f;
		PaddingMax.X = Size.X;
		break;

	case HAlign_Fill:
	case HAlign_Center:
		Offset.X = -Size.X / 2.f;
		PaddingMin.X = Size.X / 2.f;
		PaddingMax.X = PaddingMin.X;
		break;
		
	case HAlign_Right:
		Offset.X = -Size.X;
		PaddingMin.X = Size.X;
		PaddingMax.X = 0.f;
		break;
		
	default: checkNoEntry();
	}

	switch (Descriptor->VerticalAlignment)
	{
	case VAlign_Top:
		Offset.Y = 0.f;
		PaddingMin.Y = 0.f;
		PaddingMax.Y = Size.Y;
		break;

	case VAlign_Fill:
	case VAlign_Center:
		Offset.Y = -Size.Y / 2.f;
		PaddingMin.Y = Size.Y / 2.f;
		PaddingMax.Y = PaddingMin.Y;
		break;
		
	case VAlign_Bottom:
		Offset.Y = -Size.Y;
		PaddingMin.Y = Size.Y;
		PaddingMax.Y = 0.f;
		break;
		
	default: checkNoEntry();
	}
}
