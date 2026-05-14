#include "GraphYamlSerializer.h"
#include "NodeYamlSerializer.h"
#include "PinYamlSerializer.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MathExpression.h"
#include "K2Node_Variable.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"

// ---- Export ----

void FGraphYamlSerializer::Export(const UEdGraph* Graph, YAML::Node& OutNode)
{
	if (!Graph) return;

	YAML::Node NodesSeq(YAML::NodeType::Sequence);
	for (const UEdGraphNode* Node : Graph->Nodes)
	{
		YAML::Node NodeYaml;
		FNodeYamlSerializer::Export(Node, NodeYaml);

		// pins (skip for MathExpression — ReconstructNode handles them)
		if (!Node->IsA<UK2Node_MathExpression>())
		{
			YAML::Node PinsSeq(YAML::NodeType::Sequence);
			for (const UEdGraphPin* Pin : Node->Pins)
			{
				if (!Pin || Pin->bHidden) continue;
				YAML::Node PinYaml;
				FPinYamlSerializer::Export(Pin, PinYaml);
				PinsSeq.push_back(PinYaml);
			}
			NodeYaml["pins"] = PinsSeq;
		}

		NodesSeq.push_back(NodeYaml);
	}
	OutNode["nodes"] = NodesSeq;
}

// ---- Node class lookup ----

static UClass* FindNodeClass(const FString& TypeStr)
{
	static TMap<FString, UClass*> Map;
	if (Map.IsEmpty())
	{
		auto Add = [&](UClass* C) { Map.Add(C->GetName(), C); };
		Add(UEdGraphNode::StaticClass());
		Add(UK2Node_Event::StaticClass());
		Add(UK2Node_CallFunction::StaticClass());
		Add(UK2Node_MathExpression::StaticClass());
		Add(UK2Node_Variable::StaticClass());
		Add(UK2Node_VariableGet::StaticClass());
		Add(UK2Node_VariableSet::StaticClass());
	}
	UClass** Found = Map.Find(TypeStr);
	return Found ? *Found : UEdGraphNode::StaticClass();
}

// ---- Import (Pass 1~4) ----

struct FPendingLink
{
	FString FromNodeId;
	FString FromPinName;
	FString ToNodeId;
	FString ToPinName;
};

void FGraphYamlSerializer::Import(const YAML::Node& GraphYaml, UEdGraph* Graph)
{
	if (!Graph || !GraphYaml["nodes"]) return;

	TMap<FString, UEdGraphNode*> NodeMap;
	TArray<FPendingLink> PendingLinks;

	// Pass 1: 노드 skeleton 생성
	for (const auto& NodeYaml : GraphYaml["nodes"])
	{
		FString TypeStr(NodeYaml["type"].as<std::string>().c_str());
		FString NodeId(NodeYaml["id"].as<std::string>().c_str());

		UClass* NodeClass = FindNodeClass(TypeStr);

		UEdGraphNode* Node = NewObject<UEdGraphNode>(Graph, NodeClass);
		Node->NodePosX = NodeYaml["pos"][0].as<int>();
		Node->NodePosY = NodeYaml["pos"][1].as<int>();
		Node->NodeGuid = FGuid::NewGuid();
		Graph->AddNode(Node, false, false);

		NodeMap.Add(NodeId, Node);
	}

	// Pass 2: 핀 생성 + 링크 임시 저장
	for (const auto& NodeYaml : GraphYaml["nodes"])
	{
		FString NodeId(NodeYaml["id"].as<std::string>().c_str());
		UEdGraphNode* Node = NodeMap.FindRef(NodeId);
		if (!Node || !NodeYaml["pins"]) continue;

		for (const auto& PinYaml : NodeYaml["pins"])
		{
			FName PinName(PinYaml["name"].as<std::string>().c_str());
			FString DirStr(PinYaml["dir"].as<std::string>().c_str());
			EEdGraphPinDirection Dir = (DirStr == TEXT("in")) ? EGPD_Input : EGPD_Output;
			FName PinCat(PinYaml["type"].as<std::string>().c_str());

			UEdGraphPin* Pin = Node->CreatePin(Dir, PinCat, PinName);

			if (PinYaml["default"])
				Pin->DefaultValue = FString(PinYaml["default"].as<std::string>().c_str());

			if (PinYaml["links"])
			{
				for (const auto& LinkYaml : PinYaml["links"])
				{
					FString LinkStr(LinkYaml.as<std::string>().c_str());
					FString TargetNodeId, TargetPinName;
					LinkStr.Split(TEXT("."), &TargetNodeId, &TargetPinName);
					PendingLinks.Add({NodeId, PinName.ToString(), TargetNodeId, TargetPinName});
				}
			}
		}
	}

	// Pass 3: ReconstructNode (MathExpression 등 핀 재생성)
	for (auto& [NodeId, Node] : NodeMap)
	{
		Node->ReconstructNode();
	}

	// Pass 4: 핀 포인터 새로 조회 + 링크 연결
	for (const FPendingLink& Link : PendingLinks)
	{
		UEdGraphNode* FromNode = NodeMap.FindRef(Link.FromNodeId);
		UEdGraphNode* ToNode   = NodeMap.FindRef(Link.ToNodeId);
		if (!FromNode || !ToNode) continue;

		UEdGraphPin* FromPin = FromNode->FindPin(FName(*Link.FromPinName));
		UEdGraphPin* ToPin   = ToNode->FindPin(FName(*Link.ToPinName));
		if (!FromPin || !ToPin) continue;

		if (!FromPin->LinkedTo.Contains(ToPin))
			FromPin->LinkedTo.Add(ToPin);
	}
}
