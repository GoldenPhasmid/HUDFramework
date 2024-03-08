#pragma once

#include "HUDWidgetContext.generated.h"

/**
 * Widget context data, passed from gameplay layer to UI layer during widget initialization
 * subclass to add your own widget context data
 * @todo: create empty base class FHUDWidgetContextBase
 */
USTRUCT(BlueprintType)
struct HUDFRAMEWORK_API FHUDWidgetContext
{
	GENERATED_BODY()

	FHUDWidgetContext() = default;
	FHUDWidgetContext(UObject* InContextObject)
		: ContextObject(InContextObject)
		, DataObject(nullptr)
	{}
	
	FHUDWidgetContext(UObject* InContextObject, const UObject* InDataObject)
		: ContextObject(InContextObject)
		, DataObject(InDataObject)
	{}
	
	UPROPERTY()
	TObjectPtr<UObject> ContextObject;

	UPROPERTY()
	TObjectPtr<const UObject> DataObject;
};

template<>
struct TStructOpsTypeTraits<FHUDWidgetContext> : public TStructOpsTypeTraitsBase2<FHUDWidgetContext>
{
	enum
	{
		WithCopy = true	
	};
};

template <typename T>
struct THUDWidgetContextTraits;

/**
 * Add your type to check context types (FHUDWidgetContextHandle::IsA)
 */
template <>
struct THUDWidgetContextTraits<FHUDWidgetContext>
{
	static constexpr FStringView Type = TEXTVIEW("FHUDWidgetContext");
};

USTRUCT(BlueprintType)
struct HUDFRAMEWORK_API FHUDWidgetContextHandle
{
	GENERATED_BODY()
	
	FHUDWidgetContextHandle() = default;
	
	FHUDWidgetContextHandle(const FHUDWidgetContextHandle& Other)
		: Data(Other.Data)
		, ContextType(Other.ContextType)
	{}

	template <typename TContextType = FHUDWidgetContext, TEMPLATE_REQUIRES(TIsDerivedFrom<TContextType, FHUDWidgetContext>::IsDerived)>
	explicit FHUDWidgetContextHandle(const TSharedRef<TContextType>& Context)
		: Data(Context)
		, ContextType(THUDWidgetContextTypeWrapper<TContextType>::Type)
	{}

	template <typename TContextType, typename ...TArgs>
	static FHUDWidgetContextHandle CreateContext(TArgs&&... Args)
	{
		return FHUDWidgetContextHandle{MakeShared<TContextType>(Forward<TArgs>(Args)...)};
	}

	bool IsValid() const
	{
		return Data.IsValid();
	}

	template <typename TContextType>
	bool IsA() const
	{
		static_assert(THUDWidgetContextTypeWrapper<TContextType>::value, "Type is undefined. Define THUDWidgetContextTraits with your Widget Context type.");
		
		return ContextType == FName(THUDWidgetContextTypeWrapper<TContextType>::Type);
	}

	FHUDWidgetContext& GetContext()
	{
		return *Data;
	}
	
	template <typename TContextType>
	TContextType& GetContext()
	{
		return *StaticCastSharedPtr<TContextType>(Data);
	}

	const FHUDWidgetContext& GetContext() const
	{
		return *Data;
	}

	template <typename TContextType>
	const TContextType& GetContext() const
	{
		return *StaticCastSharedPtr<TContextType>(Data);
	}

	/** Comparison operator */
	bool operator==(const FHUDWidgetContextHandle& Other) const
	{
		if (Data.IsValid() != Other.Data.IsValid())
		{
			return false;
		}
		
		if (Data.Get() != Other.Data.Get())
		{
			return false;
		}
		return true;
	}

	/** Comparison operator */
	bool operator!=(const FHUDWidgetContextHandle& Other) const
	{
		return !(*this == Other);
	}

private:

	template <typename T, typename U = void>
	struct THUDWidgetContextTypeWrapper : std::false_type
	{
		static constexpr FStringView Type = TEXTVIEW("Unknown");
	};

	template <typename T>
	struct THUDWidgetContextTypeWrapper<T, std::void_t<decltype(THUDWidgetContextTraits<T>::Type)>> : std::true_type
	{
		static constexpr FStringView Type = THUDWidgetContextTraits<T>::Type;
	};

	// @todo: custom serialization
	// @todo: Replace with FHUDWidgetContextBase
	TSharedPtr<FHUDWidgetContext> Data;
	FName ContextType;
};

template <>
struct TStructOpsTypeTraits<FHUDWidgetContextHandle>: public TStructOpsTypeTraitsBase2<FHUDWidgetContextHandle>
{
	enum
	{
		WithCopy = true, // Necessary so that TSharedPtr<FBeefWorldGenerationContext> Data is copied around
		WithIdenticalViaEquality = true,
	};
};
