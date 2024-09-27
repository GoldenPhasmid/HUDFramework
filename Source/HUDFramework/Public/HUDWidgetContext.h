#pragma once

#include "CoreMinimal.h"

#include "HUDWidgetContext.generated.h"

struct FHUDWidgetContextBase;
struct FHUDWidgetContext;
struct FHUDWidgetContextHandle;
using FHUDWidgetContextProxy = FHUDWidgetContextBase;

/**
 * Base class for any widget context data
 * Passed from gameplay layer to UI layer during widget initialization
 * Subclass to add your own widget context data
 * @see @FHUDWidgetContextHandle, @UHUDWidgetContextSubsystem, @InitializeWidget
 */
USTRUCT(BlueprintType)
struct HUDFRAMEWORK_API FHUDWidgetContextBase
{
	GENERATED_BODY()
};

/**
 * Widget context data, passed from gameplay layer to UI layer during widget initialization
 * subclass to add your own widget context data
 */
USTRUCT(BlueprintType)
struct HUDFRAMEWORK_API FHUDWidgetContext: public FHUDWidgetContextBase
{
	GENERATED_BODY()

	FHUDWidgetContext() = default;
	explicit FHUDWidgetContext(UObject* InContextObject)
		: ContextObject(InContextObject)
		, DataObject(nullptr)
	{}
	
	FHUDWidgetContext(UObject* InContextObject, const UObject* InDataObject)
		: ContextObject(InContextObject)
		, DataObject(InDataObject)
	{}
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> ContextObject;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<const UObject> DataObject;
};

USTRUCT(BlueprintType)
struct HUDFRAMEWORK_API FHUDWidgetContextHandle
{
	GENERATED_BODY()
	
	FHUDWidgetContextHandle() = default;
	
	FHUDWidgetContextHandle(const FHUDWidgetContextHandle& Other)
		: ContextData(Other.ContextData)
		, ContextType(Other.ContextType)
	{}

	template <typename TContextType = FHUDWidgetContextProxy, TEMPLATE_REQUIRES(TIsDerivedFrom<TContextType, FHUDWidgetContextProxy>::IsDerived)>
	explicit FHUDWidgetContextHandle(const TSharedRef<TContextType>& Context)
		: ContextData(Context)
		, ContextType(TBaseStructure<TContextType>::Get())
	{}

	/** constructor for internal use only */
	FHUDWidgetContextHandle(const UScriptStruct* ScriptStruct, const void* StructMemory);
	
	template <typename TContextType, typename ...TArgs>
	static FHUDWidgetContextHandle CreateContext(TArgs&&... Args)
	{
		return FHUDWidgetContextHandle{MakeShared<TContextType>(Forward<TArgs>(Args)...)};
	}

	bool IsValid() const
	{
		return ContextData.IsValid() && ContextType.IsValid();
	}

	void Invalidate()
	{
		ContextData.Reset();
		ContextType = nullptr;
	}

	/** */
	template <typename TContextType, TEMPLATE_REQUIRES(TIsDerivedFrom<TContextType, FHUDWidgetContextProxy>::IsDerived)>
	FORCEINLINE bool IsA() const
	{
		return IsA(TBaseStructure<TContextType>::Get());
	}

	template <typename TContextType, TEMPLATE_REQUIRES(TIsDerivedFrom<TContextType, FHUDWidgetContextProxy>::IsDerived)>
	FORCEINLINE bool IsDerivedFrom() const
	{
		return IsDerivedFrom(TBaseStructure<TContextType>::Get());
	}

	FORCEINLINE bool IsA(const UScriptStruct* ScriptStruct) const
	{
		return ContextType.Get() == ScriptStruct;
	}

	FORCEINLINE bool IsDerivedFrom(const UScriptStruct* ScriptStruct) const
	{
		return ContextType.Get()->IsChildOf(ScriptStruct);
	}

	FORCEINLINE const UScriptStruct* GetContextType() const
	{
		return ContextType.Get();
	}
	
	template <typename TContextType>
	const TContextType& GetContext() const
	{
		check(IsDerivedFrom<TContextType>());
		return *StaticCastSharedPtr<TContextType>(ContextData);
	}

	template <typename TContextType>
	TContextType& GetContext()
	{
		check(IsDerivedFrom<TContextType>());
		return *StaticCastSharedPtr<TContextType>(ContextData);
	}

	FHUDWidgetContextProxy& GetContext()
	{
		return *ContextData;
	}
	
	const FHUDWidgetContextProxy& GetContext() const
	{
		return *ContextData;
	}

	/** Comparison operator */
	bool operator==(const FHUDWidgetContextHandle& Other) const
	{
		if (ContextData.IsValid() != Other.ContextData.IsValid())
		{
			return false;
		}
		
		if (ContextData.Get() != Other.ContextData.Get())
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
	
	// @todo: custom serialization
	TSharedPtr<FHUDWidgetContextProxy> ContextData;
	TWeakObjectPtr<const UScriptStruct> ContextType;
};

template <>
struct TStructOpsTypeTraits<FHUDWidgetContextHandle>: public TStructOpsTypeTraitsBase2<FHUDWidgetContextHandle>
{
	enum
	{
		WithCopy = true, // Necessary so that TSharedPtr data is copied around
		WithIdenticalViaEquality = true,
	};
};

/**
 * Widget Context Container
 * Can hold one or more widget contexts of the same type
 */
USTRUCT(BlueprintType)
struct HUDFRAMEWORK_API FHUDWidgetContextContainer
{
	GENERATED_BODY()

	template<typename TContextType, typename TPred, TEMPLATE_REQUIRES(TIsDerivedFrom<TContextType, FHUDWidgetContextProxy>::Value)>
	void ForEachContext(TPred&& Callable) const
	{
		for (const FHUDWidgetContextHandle& Handle : ContextHandles)
		{
			if (Handle.IsA<TContextType>())
			{
				Invoke(Callable, Handle.GetContext<TContextType>());
			}
		}
	}

	UPROPERTY()
	TArray<FHUDWidgetContextHandle> ContextHandles;
};
