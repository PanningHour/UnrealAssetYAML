#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "SCSYamlSerializer.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "Engine/Blueprint.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet2/KismetEditorUtilities.h"

#if WITH_DEV_AUTOMATION_TESTS

// GeneratedClass가 있는 진짜 Blueprint SCS 필요
static USimpleConstructionScript* MakeTestSCS()
{
	static int32 Counter = 0;
	FName BPName = *FString::Printf(TEXT("TestBP_SCS_%d"), Counter++);
	UBlueprint* BP = FKismetEditorUtilities::CreateBlueprint(
		AActor::StaticClass(), GetTransientPackage(), BPName,
		BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
	return BP ? BP->SimpleConstructionScript : nullptr;
}

// 루트 노드 1개 export → component_class, variable_name 확인
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestSCS_ExportRootNode,
	"UnrealAssetYAML.SCS.ExportRootNode",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestSCS_ExportRootNode::RunTest(const FString& Parameters)
{
	USimpleConstructionScript* SCS = MakeTestSCS();
	USCS_Node* RootNode = SCS->CreateNode(USceneComponent::StaticClass(), TEXT("SceneRoot"));
	SCS->AddNode(RootNode);

	YAML::Node Out;
	FSCSYamlSerializer::Export(SCS, Out);

	TestTrue(TEXT("nodes is sequence"), Out["nodes"].IsSequence());
	TestEqual(TEXT("root node count"), (int)Out["nodes"].size(), 1);

	const YAML::Node& Node0 = Out["nodes"][0];
	TestEqual(TEXT("component_class"), FString(Node0["component_class"].as<std::string>().c_str()), FString(TEXT("SceneComponent")));
	TestEqual(TEXT("variable_name"), FString(Node0["variable_name"].as<std::string>().c_str()), FString(TEXT("SceneRoot")));
	return true;
}

// 부모-자식 트리 → children 중첩 구조
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestSCS_ExportChildNodes,
	"UnrealAssetYAML.SCS.ExportChildNodes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestSCS_ExportChildNodes::RunTest(const FString& Parameters)
{
	USimpleConstructionScript* SCS = MakeTestSCS();
	USCS_Node* RootNode = SCS->CreateNode(USceneComponent::StaticClass(), TEXT("SceneRoot"));
	SCS->AddNode(RootNode);

	USCS_Node* ChildNode = SCS->CreateNode(UStaticMeshComponent::StaticClass(), TEXT("Mesh"));
	RootNode->AddChildNode(ChildNode);

	YAML::Node Out;
	FSCSYamlSerializer::Export(SCS, Out);

	TestEqual(TEXT("root count"), (int)Out["nodes"].size(), 1);

	const YAML::Node& Root = Out["nodes"][0];
	TestTrue(TEXT("children is sequence"), Root["children"].IsSequence());
	TestEqual(TEXT("child count"), (int)Root["children"].size(), 1);

	const YAML::Node& Child = Root["children"][0];
	TestEqual(TEXT("child class"), FString(Child["component_class"].as<std::string>().c_str()), FString(TEXT("StaticMeshComponent")));
	TestEqual(TEXT("child variable"), FString(Child["variable_name"].as<std::string>().c_str()), FString(TEXT("Mesh")));
	return true;
}

// 컴포넌트 프로퍼티 FProperty 리플렉션 → properties 맵 확인
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestSCS_ExportComponentProps,
	"UnrealAssetYAML.SCS.ExportComponentProps",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestSCS_ExportComponentProps::RunTest(const FString& Parameters)
{
	USimpleConstructionScript* SCS = MakeTestSCS();
	USCS_Node* Node = SCS->CreateNode(USceneComponent::StaticClass(), TEXT("SceneRoot"));
	SCS->AddNode(Node);

	// ComponentTemplate에 값 설정
	if (USceneComponent* Comp = Cast<USceneComponent>(Node->ComponentTemplate))
	{
		Comp->SetRelativeLocation(FVector(100.f, 0.f, 0.f));
	}

	YAML::Node Out;
	FSCSYamlSerializer::Export(SCS, Out);

	const YAML::Node& Node0 = Out["nodes"][0];
	TestTrue(TEXT("properties is map"), Node0["properties"].IsMap());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
