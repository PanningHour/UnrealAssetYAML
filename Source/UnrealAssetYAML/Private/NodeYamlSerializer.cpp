#include "NodeYamlSerializer.h"
#include "MemberRefSerializer.h"
#include "PinYamlSerializer.h"
#include "EdGraph/EdGraphNode.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MathExpression.h"

void FNodeYamlSerializer::Export(const UEdGraphNode* Node, YAML::Node& OutNode)
{
	if (!Node) return;

	OutNode["id"] = TCHAR_TO_UTF8(*FPinYamlSerializer::MakeNodeId(Node));
	OutNode["type"] = TCHAR_TO_UTF8(*Node->GetClass()->GetName());

	YAML::Node PosNode(YAML::NodeType::Sequence);
	PosNode.push_back(Node->NodePosX);
	PosNode.push_back(Node->NodePosY);
	OutNode["pos"] = PosNode;

	if (const UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node))
	{
		OutNode["event"] = TCHAR_TO_UTF8(*EventNode->EventReference.GetMemberName().ToString());
		return;
	}

	if (const UK2Node_MathExpression* MathNode = Cast<UK2Node_MathExpression>(Node))
	{
		OutNode["expression"] = TCHAR_TO_UTF8(*MathNode->Expression);
		// pins excluded — ReconstructNode regenerates them on import
		return;
	}

	if (const UK2Node_CallFunction* CallNode = Cast<UK2Node_CallFunction>(Node))
	{
		OutNode["function"] = TCHAR_TO_UTF8(*CallNode->FunctionReference.GetMemberName().ToString());
		YAML::Node MemberRef;
		FMemberRefSerializer::Export(CallNode->FunctionReference, MemberRef);
		OutNode["member_ref"] = MemberRef;
		return;
	}
}
