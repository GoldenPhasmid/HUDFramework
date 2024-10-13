#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"

#include "HUDActivatableWidget.generated.h"

UCLASS(Abstract)
class HUDFRAMEWORK_API UHUDActivatableWidget: public UCommonActivatableWidget
{
	GENERATED_BODY()
public:

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

protected:
	
	UPROPERTY(EditDefaultsOnly, Category = Input)
	ECommonInputMode InputMode = ECommonInputMode::All;
};
