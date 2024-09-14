#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"

#include "HUDActivatableWidget.generated.h"

UCLASS(Abstract)
class HUDFRAMEWORK_API UHUDActivatableWidget: public UCommonActivatableWidget
{
	GENERATED_BODY()
public:

	virtual TSharedRef<SWidget> RebuildWidget() override;
};
