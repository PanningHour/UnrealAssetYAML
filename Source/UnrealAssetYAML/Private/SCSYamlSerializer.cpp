#include "SCSYamlSerializer.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "Components/ActorComponent.h"

void FSCSYamlSerializer::ExportNode(const USCS_Node* Node, YAML::Node& OutNode)
{
	if (!Node) return;

	FString ClassName = Node->ComponentClass ? Node->ComponentClass->GetName() : TEXT("ActorComponent");
	OutNode["component_class"] = std::string(TCHAR_TO_UTF8(*ClassName));
	OutNode["variable_name"]   = std::string(TCHAR_TO_UTF8(*Node->GetVariableName().ToString()));

	// FProperty 리플렉션으로 ComponentTemplate 프로퍼티 직렬화
	YAML::Node PropsNode(YAML::NodeType::Map);
	if (UActorComponent* Template = Node->ComponentTemplate)
	{
		UClass* CompClass = Template->GetClass();
		for (TFieldIterator<FProperty> It(CompClass, EFieldIteratorFlags::ExcludeSuper); It; ++It)
		{
			FProperty* Prop = *It;
			if (Prop->HasAnyPropertyFlags(CPF_Transient | CPF_SkipSerialization)) continue;
			if (!Prop->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible)) continue;

			// CDO 기본값과 다른 경우만 저장
			UObject* CDO = CompClass->GetDefaultObject();
			if (Prop->Identical_InContainer(Template, CDO)) continue;

			FString ValueStr;
			const void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Template);
			Prop->ExportTextItem_Direct(ValueStr, ValuePtr, nullptr, nullptr, PPF_None);
			PropsNode[std::string(TCHAR_TO_UTF8(*Prop->GetName()))] = std::string(TCHAR_TO_UTF8(*ValueStr));
		}
	}
	OutNode["properties"] = PropsNode;

	// 자식 노드 재귀
	YAML::Node ChildrenSeq(YAML::NodeType::Sequence);
	for (const USCS_Node* Child : Node->GetChildNodes())
	{
		YAML::Node ChildYaml;
		ExportNode(Child, ChildYaml);
		ChildrenSeq.push_back(ChildYaml);
	}
	OutNode["children"] = ChildrenSeq;
}

void FSCSYamlSerializer::Export(const USimpleConstructionScript* SCS, YAML::Node& OutNode)
{
	if (!SCS) return;

	YAML::Node NodesSeq(YAML::NodeType::Sequence);
	for (const USCS_Node* RootNode : SCS->GetRootNodes())
	{
		YAML::Node NodeYaml;
		ExportNode(RootNode, NodeYaml);
		NodesSeq.push_back(NodeYaml);
	}
	OutNode["nodes"] = NodesSeq;
}
