#include "PinYamlSerializer.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

FString FPinYamlSerializer::MakeNodeId(const UEdGraphNode* Node)
{
	if (!Node) return TEXT("");
	FString TypeLabel = Node->GetClass()->GetName();
	FString GuidStr = Node->NodeGuid.ToString(EGuidFormats::Digits).Left(8).ToLower();
	return FString::Printf(TEXT("%s_%s"), *TypeLabel, *GuidStr);
}

void FPinYamlSerializer::Export(const UEdGraphPin* Pin, YAML::Node& OutNode)
{
	if (!Pin) return;

	OutNode["name"] = TCHAR_TO_UTF8(*Pin->PinName.ToString());
	OutNode["dir"] = (Pin->Direction == EGPD_Input) ? "in" : "out";
	OutNode["type"] = TCHAR_TO_UTF8(*Pin->PinType.PinCategory.ToString());

	if (!Pin->DefaultValue.IsEmpty())
	{
		OutNode["default"] = TCHAR_TO_UTF8(*Pin->DefaultValue);
	}

	if (Pin->LinkedTo.Num() > 0)
	{
		YAML::Node Links(YAML::NodeType::Sequence);
		for (const UEdGraphPin* Linked : Pin->LinkedTo)
		{
			if (!Linked || !Linked->GetOwningNodeUnchecked()) continue;
			FString NodeId = MakeNodeId(Linked->GetOwningNodeUnchecked());
			FString LinkStr = NodeId + TEXT(".") + Linked->PinName.ToString();
			Links.push_back(TCHAR_TO_UTF8(*LinkStr));
		}
		OutNode["links"] = Links;
	}

	if (Pin->SubPins.Num() > 0)
	{
		YAML::Node SubPinsNode(YAML::NodeType::Sequence);
		for (const UEdGraphPin* Sub : Pin->SubPins)
		{
			YAML::Node SubOut;
			Export(Sub, SubOut);
			SubPinsNode.push_back(SubOut);
		}
		OutNode["sub_pins"] = SubPinsNode;
	}
}
