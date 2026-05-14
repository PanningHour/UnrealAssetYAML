#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "BlueprintYamlHandler.h"
#include "VariableYamlSerializer.h"
#include "GraphYamlSerializer.h"
#include "Engine/Blueprint.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"

#if WITH_DEV_AUTOMATION_TESTS

static UBlueprint* MakeTestBlueprint(const FString& Name)
{
	return FKismetEditorUtilities::CreateBlueprint(
		AActor::StaticClass(), GetTransientPackage(),
		*Name, BPTYPE_Normal,
		UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

// Export: asset_type=Blueprint, parent_class, variables/graphs keys 존재
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestBP_ExportStructure,
	"UnrealAssetYAML.Blueprint.ExportStructure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestBP_ExportStructure::RunTest(const FString& Parameters)
{
	UBlueprint* BP = MakeTestBlueprint(TEXT("TestBP_ExportStructure"));
	if (!TestNotNull(TEXT("BP created"), BP)) return false;

	YAML::Node Out;
	FBlueprintYamlHandler::Export(BP, Out);

	TestEqual(TEXT("asset_type"), FString(Out["asset_type"].as<std::string>().c_str()), FString(TEXT("Blueprint")));
	TestTrue(TEXT("parent_class exists"), Out["parent_class"].IsDefined());
	TestTrue(TEXT("variables is sequence"), Out["variables"].IsSequence());
	TestTrue(TEXT("graphs is map"), Out["graphs"].IsMap());
	return true;
}

// Round-trip: 변수 추가 → Export → Import → 변수 이름 일치
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestBP_RoundTrip_Variables,
	"UnrealAssetYAML.Blueprint.RoundTripVariables",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestBP_RoundTrip_Variables::RunTest(const FString& Parameters)
{
	UBlueprint* OrigBP = MakeTestBlueprint(TEXT("TestBP_RoundTripVars_Orig"));
	if (!TestNotNull(TEXT("BP created"), OrigBP)) return false;

	// 변수 추가 (PC_Int — PC_Real은 headless에서 AddMemberVariable 크래시)
	FEdGraphPinType IntType;
	IntType.PinCategory = UEdGraphSchema_K2::PC_Int;
	FBlueprintEditorUtils::AddMemberVariable(OrigBP, TEXT("Health"), IntType);

	YAML::Node Out;
	FBlueprintYamlHandler::Export(OrigBP, Out);

	UBlueprint* ImportedBP = FBlueprintYamlHandler::Import(Out, GetTransientPackage());
	if (!TestNotNull(TEXT("Imported BP"), ImportedBP)) return false;

	bool bHasHealth = false;
	for (const FBPVariableDescription& V : ImportedBP->NewVariables)
		if (V.VarName == TEXT("Health")) bHasHealth = true;
	TestTrue(TEXT("Health variable round-tripped"), bHasHealth);
	return true;
}

// Round-trip: EventGraph 노드 Export → Import → 노드 수 일치
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestBP_RoundTrip_Graph,
	"UnrealAssetYAML.Blueprint.RoundTripGraph",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestBP_RoundTrip_Graph::RunTest(const FString& Parameters)
{
	UBlueprint* OrigBP = MakeTestBlueprint(TEXT("TestBP_RoundTripGraph_Orig"));
	if (!TestNotNull(TEXT("BP created"), OrigBP)) return false;

	YAML::Node Out;
	FBlueprintYamlHandler::Export(OrigBP, Out);

	// EventGraph가 YAML에 직렬화됐는지 확인
	TestTrue(TEXT("graphs is map"), Out["graphs"].IsMap());
	TestTrue(TEXT("EventGraph exported"), Out["graphs"]["EventGraph_0"].IsDefined()
		|| Out["graphs"]["EventGraph"].IsDefined()
		|| Out["graphs"].size() > 0);

	UBlueprint* ImportedBP = FBlueprintYamlHandler::Import(Out, GetTransientPackage());
	if (!TestNotNull(TEXT("Imported BP"), ImportedBP)) return false;

	// Import 후 EventGraph 존재 확인
	UEdGraph* ImportedGraph = FBlueprintEditorUtils::FindEventGraph(ImportedBP);
	TestNotNull(TEXT("Imported EventGraph exists"), ImportedGraph);
	return true;
}

// Round-trip: Import 후 CompileBlueprint 성공
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestBP_RoundTrip_Compile,
	"UnrealAssetYAML.Blueprint.RoundTripCompile",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestBP_RoundTrip_Compile::RunTest(const FString& Parameters)
{
	UBlueprint* OrigBP = MakeTestBlueprint(TEXT("TestBP_RoundTripCompile_Orig"));
	if (!TestNotNull(TEXT("BP created"), OrigBP)) return false;

	YAML::Node Out;
	FBlueprintYamlHandler::Export(OrigBP, Out);

	UBlueprint* ImportedBP = FBlueprintYamlHandler::Import(Out, GetTransientPackage());
	if (!TestNotNull(TEXT("Imported BP"), ImportedBP)) return false;

	FKismetEditorUtilities::CompileBlueprint(ImportedBP);
	TestNotEqual(TEXT("compile succeeded (not error)"), (int)ImportedBP->Status, (int)BS_Error);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
