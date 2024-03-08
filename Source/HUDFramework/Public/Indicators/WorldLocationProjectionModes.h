#pragma once

#include "IndicatorProjectionMode.h"

#include "WorldLocationProjectionModes.generated.h"

UCLASS(DisplayName = "Component Point")
class UIndicatorProjectionMode_ComponentPoint : public UIndicatorProjectionMode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Indicator|ProjectionMode")
	FVector2D ScreenSpaceOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Indicator|ProjectionMode")
	FVector WorldLocationOffset = FVector::ZeroVector;

	static FVector2D CalculateScreenPosition(const FSceneViewProjectionData& ProjectionData, const FVector& WorldLocation, const FVector2f& ScreenSize, const FVector2D& ScreenSpaceOffset);

	virtual void Project(const USceneComponent* Component, const FName& SocketName, const FLocalPlayerContext& PlayerContext, const FVector2f& ScreenSize, FIndicatorProjectionResult& Result) const override;
};

UCLASS(DisplayName = "Component Bounding Box")
class UIndicatorProjectionMode_ComponentBoundingBox : public UIndicatorProjectionMode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Indicator|ProjectionMode")
	FVector2D ScreenSpaceOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Indicator|ProjectionMode")
	FVector WorldLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Indicator|ProjectionMode", meta = (ClampMin = 0, ClampMax = 1))
	FVector BoundingBoxAnchor = FVector(0.5, 0.5, 0.5);

	UPROPERTY(EditAnywhere, Category = "Indicator|ProjectionMode")
	bool bUseOwnerBoundingBox = false;

	virtual void Project(const USceneComponent* Component, const FName& SocketName, const FLocalPlayerContext& PlayerContext, const FVector2f& ScreenSize, FIndicatorProjectionResult& Result) const override;
};
