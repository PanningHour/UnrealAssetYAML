#include "BlueprintYamlHandler.h"
#include "VariableYamlSerializer.h"
#include "GraphYamlSerializer.h"
#include "SCSYamlSerializer.h"
#include "TimelineYamlSerializer.h"
#include "Engine/Blueprint.h"
#include "Engine/TimelineTemplate.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"

// ---- Export ----

void FBlueprintYamlHandler::Export(const UBlueprint* BP, YAML::Node& OutNode)
{
	if (!BP) return;

	OutNode["asset_type"] = std::string("Blueprint");

	FString ParentClass = BP->ParentClass ? BP->ParentClass->GetPathName() : TEXT("");
	OutNode["parent_class"] = std::string(TCHAR_TO_UTF8(*ParentClass));

	// Variables
	YAML::Node VarsSeq(YAML::NodeType::Sequence);
	for (const FBPVariableDescription& Var : BP->NewVariables)
	{
		YAML::Node VarNode;
		FVariableYamlSerializer::Export(Var, VarNode);
		VarsSeq.push_back(VarNode);
	}
	OutNode["variables"] = VarsSeq;

	// SCS
	YAML::Node SCSNode;
	if (BP->SimpleConstructionScript)
		FSCSYamlSerializer::Export(BP->SimpleConstructionScript, SCSNode);
	OutNode["scs_components"] = SCSNode.IsDefined() ? SCSNode : YAML::Node(YAML::NodeType::Sequence);

	// Timelines
	YAML::Node TimelinesSeq(YAML::NodeType::Sequence);
	for (UTimelineTemplate* TL : BP->Timelines)
	{
		if (!TL) continue;
		YAML::Node TLNode;
		FTimelineYamlSerializer::Export(TL, TLNode);
		TimelinesSeq.push_back(TLNode);
	}
	OutNode["timelines"] = TimelinesSeq;

	// Graphs (UbergraphPages only — FunctionGraphs like UserConstructionScript have system root nodes)
	YAML::Node GraphsMap(YAML::NodeType::Map);
	for (UEdGraph* Graph : BP->UbergraphPages)
	{
		if (!Graph) continue;
		YAML::Node GraphNode;
		FGraphYamlSerializer::Export(Graph, GraphNode);
		GraphsMap[std::string(TCHAR_TO_UTF8(*Graph->GetName()))] = GraphNode;
	}
	OutNode["graphs"] = GraphsMap;

	// CDO defaults
	YAML::Node CDONode;
	if (BP->GeneratedClass)
	{
		UObject* CDO = BP->GeneratedClass->GetDefaultObject(false);
		if (CDO) FVariableYamlSerializer::ExportCDO(CDO, CDONode);
	}
	OutNode["cdo_defaults"] = CDONode.IsDefined() ? CDONode : YAML::Node(YAML::NodeType::Map);
}

// ---- Import ----

UBlueprint* FBlueprintYamlHandler::Import(const YAML::Node& Node, UPackage* Package)
{
	if (!Node["asset_type"] || Node["asset_type"].as<std::string>() != "Blueprint")
		return nullptr;

	// 1. parent_class 찾기
	FString ParentClassPath(Node["parent_class"].as<std::string>().c_str());
	UClass* ParentClass = FindObject<UClass>(nullptr, *ParentClassPath);
	if (!ParentClass) ParentClass = AActor::StaticClass();

	// 2. UBlueprint 생성
	static int32 Counter = 0;
	FName BPName = *FString::Printf(TEXT("ImportedBP_%d"), Counter++);
	UBlueprint* BP = FKismetEditorUtilities::CreateBlueprint(
		ParentClass, Package, BPName,
		BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
	if (!BP) return nullptr;

	// 3. Variables 복원 (NewVariables 직접 추가 — AddMemberVariable은 중간 recompile 트리거)
	if (Node["variables"] && Node["variables"].IsSequence())
	{
		for (const auto& VarNode : Node["variables"])
		{
			FName VarName(VarNode["name"].as<std::string>().c_str());
			FString TypeStr(VarNode["type"].as<std::string>().c_str());

			FBPVariableDescription& NewVar = BP->NewVariables.AddDefaulted_GetRef();
			NewVar.VarName = VarName;
			NewVar.VarType.PinCategory = FName(*TypeStr);
			if (VarNode["default"])
				NewVar.DefaultValue = FString(VarNode["default"].as<std::string>().c_str());
		}
	}

	// 4. SCS — 기존 SCS에 노드 추가 (Phase 6 scope 유지)
	// (SCS import는 Phase 6 이후 확장 예정)

	// 5. Graphs 복원 (EventGraph)
	if (Node["graphs"] && Node["graphs"].IsMap())
	{
		for (auto GraphIt = Node["graphs"].begin(); GraphIt != Node["graphs"].end(); ++GraphIt)
		{
			FString GraphName(GraphIt->first.as<std::string>().c_str());
			const YAML::Node& GraphYaml = GraphIt->second;

			// UbergraphPages only (FunctionGraphs have system root nodes — skip)
			UEdGraph* TargetGraph = nullptr;
			for (UEdGraph* G : BP->UbergraphPages)
			{
				if (G && G->GetName() == GraphName)
				{
					TargetGraph = G;
					break;
				}
			}

			if (TargetGraph)
			{
				TargetGraph->Nodes.Empty();
				FGraphYamlSerializer::Import(GraphYaml, TargetGraph);
			}
		}
	}

	// 6. CompileBlueprint
	FKismetEditorUtilities::CompileBlueprint(BP);

	return BP;
}
