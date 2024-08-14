#include "HUDWidgetContext.h"

FHUDWidgetContextHandle::FHUDWidgetContextHandle(const UScriptStruct* ScriptStruct, const void* StructMemory)
{
	check(ScriptStruct);

	ContextType = ScriptStruct;
	void* ContextMemory = FMemory::Malloc(FMath::Max(1, ContextType->GetStructureSize()), ContextType->GetMinAlignment());
	ContextType->InitializeStruct(ContextMemory);
	if (StructMemory)
	{
		ContextType->CopyScriptStruct(ContextMemory, StructMemory);
	}
	
	ContextData = MakeShareable<FHUDWidgetContextProxy>(static_cast<FHUDWidgetContextProxy*>(ContextMemory));
}
