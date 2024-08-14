#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_CallFunction.h"
#include "EdGraph/EdGraphNode.h"

#include "K2Node_WidgetContext.generated.h"

/**
 * Node customization for InitializeWidget(), GetWidgetContext()
 */
UCLASS()
class UK2Node_WidgetContext: public UK2Node_CallFunction
{
	GENERATED_BODY()
public:

	//~Begin UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;
	virtual bool IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const override;
	//~End UK2Node interface

	static bool IsWidgetContextPin(const UEdGraphPin& Pin);
};
