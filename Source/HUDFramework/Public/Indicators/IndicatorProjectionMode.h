#pragma once

#include "IndicatorProjectionMode.generated.h"

struct HUDFRAMEWORK_API FIndicatorProjectionResult
{
	/** Screen position with depth in format { ScreenPos.X, ScreenPos.Y, Depth }*/
	FVector ScreenPositionWithDepth = FVector::ZeroVector;
	
	bool bSuccess = false;
};

UCLASS(Abstract, BlueprintType, DefaultToInstanced, EditInlineNew)
class HUDFRAMEWORK_API UIndicatorProjectionMode : public UObject
{
	GENERATED_BODY()
	
public:
	/**
	 * Override this function to define how position of indicator calculates in this projection mode.
	 * @param Component Component owner of indicator
	 * @param SocketName Socket of the @Component
	 * @param PlayerContext Context of local player
	 * @param ScreenSize Size that was allotted for indicators screen
	 * @param Result Out calculated result. NOTE: OutResult.bSuccess must be true only if position calculated successfully.
	 */
	virtual void Project(const USceneComponent* Component, const FName& SocketName, const FLocalPlayerContext& PlayerContext, const FVector2f& ScreenSize, FIndicatorProjectionResult& Result) const {}

protected:
	/**
	 * Method for validate data passed to Project(...) function. Simply call it at the beginning of function.
	 * @param Component Component owner of indicator
	 * @param SocketName Socket of the @Component
	 * @param PlayerContext Context of local player
	 * @return true if data validated successfully.
	 */
	static bool Validate(const USceneComponent* Component, const FName& SocketName, const FLocalPlayerContext& PlayerContext)
	{
		if (ensureAlwaysMsgf(IsValid(Component), TEXT("%s: Data validation failed! Invalid Component!"), *FString(__FUNCTION__)))
		{
			if (ensureAlwaysMsgf(PlayerContext.IsValid(), TEXT("%s: Data validation failed! Invalid player!"), *FString(__FUNCTION__)))
			{
				return ensureAlwaysMsgf(SocketName.IsNone() ? true : Component->DoesSocketExist(SocketName), TEXT("%s: Socket [%s] doesn`t exist on Component [%s]"),
				*FString(__FUNCTION__), *SocketName.ToString(), *GetNameSafe(Component));
			}
		}
		return false;
	}

	/** Helper function to get projection data from local player. */
	static bool GetProjectionData(const FLocalPlayerContext& PlayerContext, FSceneViewProjectionData& OutData)
	{
		const ULocalPlayer* LocalPlayer = PlayerContext.GetLocalPlayer();
		
		const bool bSuccess = PlayerContext.IsValid() && LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, OutData);

		return ensureAlwaysMsgf(bSuccess, TEXT("%s: Unable to get projection data!"), *FString(__FUNCTION__));
	}
};
