#pragma once

#include "AsyncMixin.h"
#include "GameplayTagContainer.h"
#include "HUDWidgetPool.h"
#include "IndicatorDescriptor.h"
#include "IndicatorManagerComponent.h"
#include "IndicatorProjectionMode.h"
#include "Blueprint/UserWidgetPool.h"

class UHUDWidgetContextSubsystem;
class UIndicatorManagerComponent;
struct FIndicatorDescriptorInstance;


class SIndicatorCanvasArrowWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SIndicatorCanvasArrowWidget) {}
	SLATE_END_ARGS()

	SIndicatorCanvasArrowWidget() {}

	// ~Begin SWidget Interface
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	// ~End SWidget Interface

	void Construct(const FArguments& InArgs, const FSlateBrush* InArrowBrush);

	FORCEINLINE void SetRotation(float InRotation)
	{
		Rotation = FMath::Fmod(InRotation, 360.f);
	}

private:
	const FSlateBrush* ArrowBrush = nullptr;
	float Rotation = 0.f;
};

#define INDICATOR_SLOT_SETTER_IMPL(VariableName, VariableNewValue) \
	if (VariableName != VariableNewValue) \
	{ \
		VariableName = VariableNewValue; \
		MarkDirty(); \
	}

class HUDFRAMEWORK_API SIndicatorCanvas : public SPanel, public FAsyncMixin
{
public:

	// Slot for indicator
	class FSlot : public TSlotBase<FSlot>
	{
	public:
		
		FSlot(const TSharedRef<FIndicatorDescriptorInstance>& InIndicatorDescriptorInstance, UUserWidget* InUserWidget)
			: TSlotBase<FSlot>(),
			  IndicatorDescriptorInstance(InIndicatorDescriptorInstance),
			  IndicatorUserWidget(InUserWidget),
			  bHasValidScreenPosition(false),
			  bWasIndicatorClamped(false),
			  bDirty(true),
			  bWasIndicatorClampedStatusChanged(false)
		{
		}

		SLATE_SLOT_BEGIN_ARGS(FSlot, TSlotBase<FSlot>)
		SLATE_END_ARGS()
		using TSlotBase<FSlot>::Construct;

		// ~Begin Getters && Setters
		FORCEINLINE TSharedRef<FIndicatorDescriptorInstance> GetIndicatorDescriptorInstance() const
		{
			return IndicatorDescriptorInstance.ToSharedRef();
		}

		FORCEINLINE TWeakObjectPtr<UUserWidget> GetUserWidget() const
		{
			return IndicatorUserWidget;
		}

		FORCEINLINE_DEBUGGABLE bool ShouldSkipIndicator() const
		{
			bool bSkip = false;
			bSkip |= WasUserWidgetManuallyCollapsed() | FMath::IsNearlyZero(GetIndicatorScale()) | !IsValid(IndicatorDescriptorInstance->Component);

			if (!bSkip && !IndicatorDescriptorInstance->Descriptor->bDisplayIndicatorWhenComponentCanNotRender)
			{
				bSkip |= !IndicatorDescriptorInstance->Component->CanEverRender();
			}
			return bSkip;
		}
		
		FORCEINLINE const FVector2D& GetScreenPosition() const { return ScreenPosition; }
		FORCEINLINE void SetScreenPosition(const FVector2D& InValue) { INDICATOR_SLOT_SETTER_IMPL(ScreenPosition, InValue); }
		FORCEINLINE double GetDepth() const { return Depth; }
		FORCEINLINE void SetDepth(double InValue) { INDICATOR_SLOT_SETTER_IMPL(Depth, InValue); }
		FORCEINLINE int32 GetPriority() const { return Priority; }
		FORCEINLINE void SetPriority(int32 InValue) { INDICATOR_SLOT_SETTER_IMPL(Priority, InValue); }
		
		FORCEINLINE bool HasValidScreenPosition() const { return bHasValidScreenPosition; }
		FORCEINLINE void SetHasValidScreenPosition(bool InValue)
		{
			INDICATOR_SLOT_SETTER_IMPL(bHasValidScreenPosition, InValue);
			
			GetWidget()->SetVisibility(bHasValidScreenPosition ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed);
		}
		
		FORCEINLINE bool WasIndicatorClamped() const { return bWasIndicatorClamped; }
		FORCEINLINE void SetWasIndicatorClamped(bool InValue) const
		{
			if (bWasIndicatorClamped != InValue)
			{
				bWasIndicatorClamped = InValue;
				bWasIndicatorClampedStatusChanged = true;
			}
		}
		
		// Flags
		FORCEINLINE bool IsDirty() const { return bDirty; }
		FORCEINLINE void ClearDirtyFlag() { bDirty = false; }
		FORCEINLINE bool WasIndicatorClampedStatusChanged() const { return bWasIndicatorClampedStatusChanged; }
		FORCEINLINE void ClearIndicatorClampedStatusChangedFlag() const { bWasIndicatorClampedStatusChanged = false; }
		// ~End Getters && Setters

		FORCEINLINE bool WasUserWidgetManuallyCollapsed() const
		{
			return GetUserWidget()->GetVisibility() == ESlateVisibility::Collapsed;
		}

		FORCEINLINE float GetIndicatorScale() const
		{
			const UIndicatorDescriptor* Descriptor = IndicatorDescriptorInstance->Descriptor;
			check(Descriptor != nullptr);

			if (Descriptor->bEnableScaling && Descriptor->ScaleCurve != nullptr)
			{
				return Descriptor->ScaleCurve->GetFloatValue(GetDepth());
			}
			return 1.f;
		}

	private:
		FORCEINLINE void MarkDirty() { bDirty = true; }
		
		TSharedPtr<FIndicatorDescriptorInstance> IndicatorDescriptorInstance;
		// Save User Widget here to release it from widget pool on removal.
		TWeakObjectPtr<UUserWidget> IndicatorUserWidget;
		FVector2D ScreenPosition = FVector2D::ZeroVector;
		double Depth = 0.;
		int32 Priority = 0;
		uint8 bHasValidScreenPosition : 1;
		mutable uint8 bWasIndicatorClamped : 1; // Saved during const ArrangeChildren operation
		// Flags
		uint8 bDirty : 1;
		mutable uint8 bWasIndicatorClampedStatusChanged : 1; // Saved during const ArrangeChildren operation
	};

	//Slot for arrow
	class FArrowSlot : public TSlotBase<FArrowSlot>
	{
	};

	SLATE_BEGIN_ARGS(SIndicatorCanvas)
	{
		_Visibility = EVisibility::HitTestInvisible;
	}
		SLATE_SLOT_ARGUMENT(SIndicatorCanvas::FSlot, Slots)
	SLATE_END_ARGS()

	SIndicatorCanvas() : SlotChildren(this),
	                     ArrowChildren(this),
	                     AllChildren(this)
	{
		AllChildren.AddChildren(SlotChildren);
		AllChildren.AddChildren(ArrowChildren);
	}

	void Construct(const FArguments& InArgs, const FLocalPlayerContext& InLocalPlayerContext, const FGameplayTagContainer& InCategoryTags, const FSlateBrush* InArrowBrush);

	// ~Begin SWidget Interface
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; };
	virtual FChildren* GetChildren() override { return &AllChildren; };
	// ~End SWidget Interface

	FIntPoint MinClampPadding = FIntPoint(10, 10);

protected:
	virtual void ProjectIndicator(const TSharedRef<FIndicatorDescriptorInstance>& Instance, const FVector2f& ScreenSize, FIndicatorProjectionResult& Result);

	void UpdateActiveTimer();
	EActiveTimerReturnType UpdateCanvas(double InCurrentTime, float InDeltaTime);
	
private:
	void HandleIndicatorAdded(const TSharedRef<FIndicatorDescriptorInstance>& IndicatorInstance);
	void HandleIndicatorRemoved(const TSharedRef<FIndicatorDescriptorInstance>& IndicatorInstance);
	
	using FScopedWidgetSlotArguments = TPanelChildren<FSlot>::FScopedWidgetSlotArguments;
	FScopedWidgetSlotArguments AddIndicatorSlot(const TSharedRef<FIndicatorDescriptorInstance>& IndicatorInstance, UUserWidget* IndicatorWidget);
	void RemoveIndicatorSlot(int32 Index);

	void SetShowAnyIndicators(bool InValue);
	
	void OnIndicatorManagerChanged();

	/** Returns true if one or more indicators have been changed. */
	bool UpdateIndicators();

	uint8 ClampIndicator(const FSlot& IndicatorSlot, const FVector2D& ScreenSize, OUT FVector2D& OutClampedScreenPosition) const;
	
	/* Scoped array for correct arrow slot managing. Can be used in const functions. */
	struct FScopedArrowChildren
	{
		// Pass pointer on const value because arrow processing happens during ArrangeChildren const operation.
		FScopedArrowChildren(const TPanelChildren<FArrowSlot>* ArrowChildren, const FSlateBrush* ArrowBrush) :
			ArrowChildren(*(const_cast<TPanelChildren<FArrowSlot>*>(ArrowChildren))),
			ArrowBrush(ArrowBrush)
		{
		}
		
		~FScopedArrowChildren();

		// Gets next arrow widget from children if exists, otherwise adds new slot to children and returns its widget.
		TSharedRef<SIndicatorCanvasArrowWidget> GetOrCreateArrowChild();

	private:
		// Maximum inactive arrow slots.
		static constexpr int32 MAX_INACTIVE_ARROWS = 3;

		// Index of next arrow that will be returned
		int32 Index = 0;
		// Ref to canvas arrow children
		TPanelChildren<FArrowSlot>& ArrowChildren;
		// Slate brush for new arrow widgets
		const FSlateBrush* ArrowBrush = nullptr;

		FScopedArrowChildren(const FScopedArrowChildren&) = delete;
		FScopedArrowChildren(const FScopedArrowChildren&&) = delete;
		FScopedArrowChildren& operator=(const FScopedArrowChildren&) = delete;
	};
	
	void AddArrowWidget(const FGeometry& AllottedGeometry, FArrangedChildren& ToArrangedChildren, FScopedArrowChildren& FromScopedChildren, uint8 InArrowDirection,  const FVector2D& IndicatorPosition, const FVector2D& IndicatorSize, EVerticalAlignment IndicatorVAlignment) const;

protected:
	mutable TOptional<FGeometry> CachedAllottedGeometry;

private:
	TPanelChildren<FSlot> SlotChildren;
	TPanelChildren<FArrowSlot> ArrowChildren;
	FCombinedChildren AllChildren;
	
	FHUDWidgetPool IndicatorPool;

	FLocalPlayerContext LocalPlayerContext;

	const FSlateBrush* ArrowBrush = nullptr;

	FGameplayTagContainer CategoryTags;
	
	bool bShowAnyIndicators = false;

	TWeakObjectPtr<UHUDWidgetContextSubsystem> WidgetContextSubsystem;
	TWeakObjectPtr<UIndicatorManagerComponent> IndicatorManager;

	TSharedPtr<FActiveTimerHandle> TickHandle;
};

#undef INDICATOR_SLOT_SETTER_IMPL
