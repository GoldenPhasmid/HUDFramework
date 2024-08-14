#include "K2Node_WidgetContext.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "HUDLayoutBlueprintLibrary.h"
#include "HUDWidgetContext.h"

void UK2Node_WidgetContext::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	Super::GetMenuActions(ActionRegistrar);

	UClass* Action = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(Action))
	{
		auto CustomizeNode = [](UEdGraphNode* NewNode, bool bTemplateNode, FName FunctionName)
		{
			UK2Node_WidgetContext* Node = CastChecked<UK2Node_WidgetContext>(NewNode);
			const UFunction* Function = UHUDLayoutBlueprintLibrary::StaticClass()->FindFunctionByName(FunctionName);
			check(Function);
			
			Node->SetFromFunction(Function);
		};

		UBlueprintNodeSpawner* InitializeWidgetNode = UBlueprintNodeSpawner::Create(Action);
		InitializeWidgetNode->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(CustomizeNode, GET_MEMBER_NAME_CHECKED(UHUDLayoutBlueprintLibrary, K2_InitializeWidget));
		ActionRegistrar.AddBlueprintAction(InitializeWidgetNode);

		UBlueprintNodeSpawner* GetWidgetContextNode = UBlueprintNodeSpawner::Create(Action);
		GetWidgetContextNode->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(CustomizeNode, GET_MEMBER_NAME_CHECKED(UHUDLayoutBlueprintLibrary, K2_GetWidgetContext));
		ActionRegistrar.AddBlueprintAction(GetWidgetContextNode);

		UBlueprintNodeSpawner* GetWidgetContextFromHandleNode = UBlueprintNodeSpawner::Create(Action);
		GetWidgetContextFromHandleNode->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(CustomizeNode, GET_MEMBER_NAME_CHECKED(UHUDLayoutBlueprintLibrary, K2_GetWidgetContextFromHandle));
		ActionRegistrar.AddBlueprintAction(GetWidgetContextFromHandleNode);

		UBlueprintNodeSpawner* ToWidgetContextHandleNode = UBlueprintNodeSpawner::Create(Action);
		ToWidgetContextHandleNode->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(CustomizeNode, GET_MEMBER_NAME_CHECKED(UHUDLayoutBlueprintLibrary, Conv_WidgetContextToWidgetContextHandle));
		ActionRegistrar.AddBlueprintAction(ToWidgetContextHandleNode);
	}
}

void UK2Node_WidgetContext::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	Super::ValidateNodeDuringCompilation(MessageLog);

	const UEdGraphPin* WidgetContextPin = FindPinChecked(TEXT("WidgetContext"));
	if (WidgetContextPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
	{
		MessageLog.Error(TEXT("@@: WidgetContext pin cannot be a wildcard."), this);
	}
	else if (!IsWidgetContextPin(*WidgetContextPin))
	{
		MessageLog.Error(*FString::Printf(TEXT("@@: WidgetContext pin is not derived from %s"), *FHUDWidgetContextProxy::StaticStruct()->GetStructCPPName()), this);
	}
}

bool UK2Node_WidgetContext::IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const
{
	if (Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason))
	{
		return true;
	}
	
	const UEdGraphPin* WidgetContextPin = FindPinChecked(TEXT("WidgetContext"));
	if (MyPin == WidgetContextPin && MyPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
	{
		if (OtherPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Struct)
		{
			OutReason = TEXT("WidgetContext must be a struct.");
			return true;
		}
		if (!IsWidgetContextPin(*OtherPin))
		{
			OutReason = FString::Printf(TEXT("WidgetContext must derive from %s type."), *FHUDWidgetContextProxy::StaticStruct()->GetStructCPPName());
			return true;
		}
	}

	return false;
}

bool UK2Node_WidgetContext::IsWidgetContextPin(const UEdGraphPin& Pin)
{
	if (Pin.PinType.PinCategory == UEdGraphSchema_K2::PC_Struct)
	{
		if (UScriptStruct* StructType = Cast<UScriptStruct>(Pin.PinType.PinSubCategoryObject.Get()))
		{
			if (StructType->IsChildOf(FHUDWidgetContextProxy::StaticStruct()))
			{
				return true;
			}
		}
	}

	return false;
}
