#include "Indicators/WorldLocationProjectionModes.h"

#include "HUDFramework.h"

FVector2D UIndicatorProjectionMode_ComponentPoint::CalculateScreenPosition(const FSceneViewProjectionData& ProjectionData, const FVector& WorldLocation, const FVector2f& ScreenSize, const FVector2D& ScreenSpaceOffset)
{
	FVector2D OutScreenSpacePosition;
	const bool bInFrontOfCamera = ULocalPlayer::GetPixelPoint(ProjectionData, WorldLocation, OutScreenSpacePosition, &ScreenSize);
	
	OutScreenSpacePosition += FVector2D(ScreenSpaceOffset.X * (bInFrontOfCamera ? 1.0 : -1.0), ScreenSpaceOffset.Y);

	// Indicator is behind camera, calculate position on the edge of the screen manually. Needed for clamping.
	if (!bInFrontOfCamera)
	{
		const FVector2D CenterToPosition = (OutScreenSpacePosition - (FVector2D(ScreenSize) / 2.0)).GetSafeNormal();
		OutScreenSpacePosition = FVector2D(ScreenSize) / 2.0 + CenterToPosition * FVector2D(ScreenSize); 
	}
	
	return OutScreenSpacePosition;
}

void UIndicatorProjectionMode_ComponentPoint::Project(const USceneComponent* Component, const FName& SocketName, const FLocalPlayerContext& PlayerContext, const FVector2f& ScreenSize, FIndicatorProjectionResult& Result) const
{
	if (!Validate(Component, SocketName, PlayerContext))
	{
		return;
	}
	
	FSceneViewProjectionData ViewProjectionData;
	if (!GetProjectionData(PlayerContext, ViewProjectionData))
	{
		return;
	}

	FVector WorldLocation = SocketName.IsNone() ? Component->GetComponentLocation() : Component->GetSocketLocation(SocketName);
	WorldLocation += WorldLocationOffset;

	const FVector2D ScreenSpacePosition = CalculateScreenPosition(ViewProjectionData, WorldLocation, ScreenSize, ScreenSpaceOffset);
	
	Result.ScreenPositionWithDepth = FVector(ScreenSpacePosition.X, ScreenSpacePosition.Y, FVector::Dist(ViewProjectionData.ViewOrigin, WorldLocation));
	Result.bSuccess = true;
}

void UIndicatorProjectionMode_ComponentBoundingBox::Project(const USceneComponent* Component, const FName& SocketName, const FLocalPlayerContext& PlayerContext, const FVector2f& ScreenSize, FIndicatorProjectionResult& Result) const
{
	if (!Validate(Component, SocketName, PlayerContext))
	{
		return;
	}

	FSceneViewProjectionData ViewProjectionData;
	if (!GetProjectionData(PlayerContext, ViewProjectionData))
	{
		return;
	}

	const FBox IndicatorBox = bUseOwnerBoundingBox ?
							Component->GetOwner()->GetComponentsBoundingBox() :
							Component->Bounds.GetBox();

	FVector WorldLocation = FMath::Lerp(IndicatorBox.Min, IndicatorBox.Max, BoundingBoxAnchor);
	WorldLocation += WorldLocationOffset;

	const FVector2D ScreenSpacePosition = UIndicatorProjectionMode_ComponentPoint::CalculateScreenPosition(ViewProjectionData, WorldLocation, ScreenSize, ScreenSpaceOffset);

	Result.ScreenPositionWithDepth = FVector(ScreenSpacePosition.X, ScreenSpacePosition.Y, FVector::Dist(ViewProjectionData.ViewOrigin, WorldLocation));
	Result.bSuccess = true;
}
