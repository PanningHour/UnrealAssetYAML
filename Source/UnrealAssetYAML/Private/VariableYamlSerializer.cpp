#include "VariableYamlSerializer.h"
#include "Engine/Blueprint.h"
#include "EdGraphSchema_K2.h"

void FVariableYamlSerializer::Export(const FBPVariableDescription& Var, YAML::Node& OutNode)
{
	OutNode["name"] = TCHAR_TO_UTF8(*Var.VarName.ToString());
	OutNode["type"] = TCHAR_TO_UTF8(*Var.VarType.PinCategory.ToString());

	if (!Var.DefaultValue.IsEmpty())
		OutNode["default"] = TCHAR_TO_UTF8(*Var.DefaultValue);

	// PropertyFlags → flags 배열 (알려진 플래그만)
	YAML::Node FlagsSeq(YAML::NodeType::Sequence);
	if (Var.PropertyFlags & CPF_Transient)             FlagsSeq.push_back(std::string("Transient"));
	if (Var.PropertyFlags & CPF_DisableEditOnInstance) FlagsSeq.push_back(std::string("DisableEditOnInstance"));
	if (Var.PropertyFlags & CPF_BlueprintReadOnly)     FlagsSeq.push_back(std::string("BlueprintReadOnly"));
	OutNode["flags"] = FlagsSeq;
}

void FVariableYamlSerializer::ExportCDO(const UObject* CDO, YAML::Node& OutNode)
{
	OutNode = YAML::Node(YAML::NodeType::Map);
	if (!CDO) return;

	UClass* Class = CDO->GetClass();
	for (TFieldIterator<FProperty> It(Class, EFieldIteratorFlags::ExcludeSuper); It; ++It)
	{
		FProperty* Prop = *It;
		if (Prop->HasAnyPropertyFlags(CPF_Transient | CPF_SkipSerialization)) continue;
		if (!Prop->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible)) continue;

		FString ValueStr;
		const void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(CDO);
		Prop->ExportTextItem_Direct(ValueStr, ValuePtr, nullptr, nullptr, PPF_None);
		OutNode[std::string(TCHAR_TO_UTF8(*Prop->GetName()))] = std::string(TCHAR_TO_UTF8(*ValueStr));
	}
}
