#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "GraphYamlSerializer.h"
#include "PinYamlSerializer.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "K2Node_MathExpression.h"

#if WITH_DEV_AUTOMATION_TESTS

// Export: 노드 수 확인
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestGraph_ExportNodeCount,
	"UnrealAssetYAML.Graph.ExportNodeCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestGraph_ExportNodeCount::RunTest(const FString& Parameters)
{
	UEdGraph* Graph = NewObject<UEdGraph>(GetTransientPackage());

	UEdGraphNode* NodeA = NewObject<UEdGraphNode>(Graph);
	NodeA->NodeGuid = FGuid::NewGuid();
	Graph->AddNode(NodeA, false, false);

	UEdGraphNode* NodeB = NewObject<UEdGraphNode>(Graph);
	NodeB->NodeGuid = FGuid::NewGuid();
	Graph->AddNode(NodeB, false, false);

	YAML::Node Out;
	FGraphYamlSerializer::Export(Graph, Out);

	TestTrue(TEXT("nodes is sequence"), Out["nodes"].IsSequence());
	TestEqual(TEXT("nodes count"), (int)Out["nodes"].size(), 2);
	return true;
}

// Export: 핀 연결 → links 직렬화
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestGraph_ExportPinLinks,
	"UnrealAssetYAML.Graph.ExportPinLinks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestGraph_ExportPinLinks::RunTest(const FString& Parameters)
{
	UEdGraph* Graph = NewObject<UEdGraph>(GetTransientPackage());

	UEdGraphNode* NodeA = NewObject<UEdGraphNode>(Graph);
	NodeA->NodeGuid = FGuid::NewGuid();
	Graph->AddNode(NodeA, false, false);
	UEdGraphPin* OutPin = NodeA->CreatePin(EGPD_Output, TEXT("exec"), TEXT("Then"));

	UEdGraphNode* NodeB = NewObject<UEdGraphNode>(Graph);
	NodeB->NodeGuid = FGuid::NewGuid();
	Graph->AddNode(NodeB, false, false);
	UEdGraphPin* InPin = NodeB->CreatePin(EGPD_Input, TEXT("exec"), TEXT("Execute"));

	OutPin->LinkedTo.Add(InPin);
	InPin->LinkedTo.Add(OutPin);

	YAML::Node Out;
	FGraphYamlSerializer::Export(Graph, Out);

	// NodeA의 Then 핀에 links가 있어야 함
	bool bFoundLinks = false;
	for (const auto& NodeYaml : Out["nodes"])
	{
		if (NodeYaml["pins"] && NodeYaml["pins"].IsSequence())
		{
			for (const auto& PinYaml : NodeYaml["pins"])
			{
				if (PinYaml["links"] && PinYaml["links"].IsSequence() && PinYaml["links"].size() > 0)
					bFoundLinks = true;
			}
		}
	}
	TestTrue(TEXT("at least one pin has links"), bFoundLinks);
	return true;
}

// Import: 노드 수 확인 (Pass 1)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestGraph_ImportNodeCount,
	"UnrealAssetYAML.Graph.ImportNodeCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestGraph_ImportNodeCount::RunTest(const FString& Parameters)
{
	const char* Yaml = R"(
nodes:
  - id: EdGraphNode_aabbccdd
    type: EdGraphNode
    pos: [0, 0]
    pins: []
  - id: EdGraphNode_11223344
    type: EdGraphNode
    pos: [200, 0]
    pins: []
)";
	YAML::Node GraphYaml = YAML::Load(Yaml);
	UEdGraph* Graph = NewObject<UEdGraph>(GetTransientPackage());

	FGraphYamlSerializer::Import(GraphYaml, Graph);

	TestEqual(TEXT("imported node count"), Graph->Nodes.Num(), 2);
	return true;
}

// Import: 핀 연결 복원 (Pass 1~4)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestGraph_ImportPinLinks,
	"UnrealAssetYAML.Graph.ImportPinLinks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestGraph_ImportPinLinks::RunTest(const FString& Parameters)
{
	const char* Yaml = R"(
nodes:
  - id: EdGraphNode_aabbccdd
    type: EdGraphNode
    pos: [0, 0]
    pins:
      - name: Then
        dir: out
        type: exec
        links: [EdGraphNode_11223344.Execute]
  - id: EdGraphNode_11223344
    type: EdGraphNode
    pos: [200, 0]
    pins:
      - name: Execute
        dir: in
        type: exec
        links: [EdGraphNode_aabbccdd.Then]
)";
	YAML::Node GraphYaml = YAML::Load(Yaml);
	UEdGraph* Graph = NewObject<UEdGraph>(GetTransientPackage());

	FGraphYamlSerializer::Import(GraphYaml, Graph);

	TestEqual(TEXT("node count"), Graph->Nodes.Num(), 2);

	// NodeA의 Then 핀 → NodeB의 Execute 핀 연결 확인
	bool bLinked = false;
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		UEdGraphPin* ThenPin = Node->FindPin(TEXT("Then"), EGPD_Output);
		if (ThenPin && ThenPin->LinkedTo.Num() > 0)
			bLinked = true;
	}
	TestTrue(TEXT("Then pin linked"), bLinked);
	return true;
}

// MathExpression round-trip: expression 보존 확인
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestGraph_MathExpressionRoundTrip,
	"UnrealAssetYAML.Graph.MathExpressionRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestGraph_MathExpressionRoundTrip::RunTest(const FString& Parameters)
{
	UEdGraph* Graph = NewObject<UEdGraph>(GetTransientPackage());

	UK2Node_MathExpression* MathNode = NewObject<UK2Node_MathExpression>(Graph);
	MathNode->NodeGuid = FGuid::NewGuid();
	MathNode->Expression = TEXT("X * 2 + Y");
	Graph->AddNode(MathNode, false, false);

	YAML::Node Out;
	FGraphYamlSerializer::Export(Graph, Out);

	// expression 필드 확인
	bool bFoundExpression = false;
	for (const auto& NodeYaml : Out["nodes"])
	{
		if (NodeYaml["expression"])
		{
			FString Expr(NodeYaml["expression"].as<std::string>().c_str());
			TestEqual(TEXT("expression preserved"), Expr, FString(TEXT("X * 2 + Y")));
			bFoundExpression = true;
		}
	}
	TestTrue(TEXT("expression node found"), bFoundExpression);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
