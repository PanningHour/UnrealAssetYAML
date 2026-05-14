#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NodeYamlSerializer.h"
#include "MemberRefSerializer.h"
#include "PinYamlSerializer.h"
#include "EdGraph/EdGraphNode.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MathExpression.h"

#if WITH_DEV_AUTOMATION_TESTS

// NodeId = TypeLabel_<8charGUID>, pos serialized
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestNode_IdAndPos,
	"UnrealAssetYAML.Node.IdAndPos",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestNode_IdAndPos::RunTest(const FString& Parameters)
{
	UEdGraphNode* Node = NewObject<UEdGraphNode>(GetTransientPackage());
	Node->NodeGuid = FGuid::NewGuid();
	Node->NodePosX = 100;
	Node->NodePosY = 200;

	YAML::Node Out;
	FNodeYamlSerializer::Export(Node, Out);

	FString NodeId(Out["id"].as<std::string>().c_str());
	// format: ClassName_XXXXXXXX
	TestTrue(TEXT("id contains underscore"), NodeId.Contains(TEXT("_")));
	TestTrue(TEXT("id ends with 8 hex chars"),
		NodeId.Right(8).ToLower().Replace(TEXT("0"), TEXT("")).Replace(TEXT("1"), TEXT("")).
		Replace(TEXT("2"), TEXT("")).Replace(TEXT("3"), TEXT("")).Replace(TEXT("4"), TEXT("")).
		Replace(TEXT("5"), TEXT("")).Replace(TEXT("6"), TEXT("")).Replace(TEXT("7"), TEXT("")).
		Replace(TEXT("8"), TEXT("")).Replace(TEXT("9"), TEXT("")).Replace(TEXT("a"), TEXT("")).
		Replace(TEXT("b"), TEXT("")).Replace(TEXT("c"), TEXT("")).Replace(TEXT("d"), TEXT("")).
		Replace(TEXT("e"), TEXT("")).Replace(TEXT("f"), TEXT("")).IsEmpty());
	TestEqual(TEXT("pos[0]"), Out["pos"][0].as<int>(), 100);
	TestEqual(TEXT("pos[1]"), Out["pos"][1].as<int>(), 200);

	return true;
}

// K2Node_Event → type + event field
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestNode_Event,
	"UnrealAssetYAML.Node.Event",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestNode_Event::RunTest(const FString& Parameters)
{
	UK2Node_Event* Node = NewObject<UK2Node_Event>(GetTransientPackage());
	Node->NodeGuid = FGuid::NewGuid();
	Node->EventReference.SetExternalMember(FName("BeginPlay"), UObject::StaticClass());

	YAML::Node Out;
	FNodeYamlSerializer::Export(Node, Out);

	TestEqual(TEXT("type"), FString(Out["type"].as<std::string>().c_str()), FString(TEXT("K2Node_Event")));
	TestEqual(TEXT("event"), FString(Out["event"].as<std::string>().c_str()), FString(TEXT("BeginPlay")));

	return true;
}

// K2Node_CallFunction → type + function member name + member_ref
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestNode_CallFunction,
	"UnrealAssetYAML.Node.CallFunction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestNode_CallFunction::RunTest(const FString& Parameters)
{
	UK2Node_CallFunction* Node = NewObject<UK2Node_CallFunction>(GetTransientPackage());
	Node->NodeGuid = FGuid::NewGuid();
	Node->FunctionReference.SetExternalMember(FName("PrintString"), nullptr);

	YAML::Node Out;
	FNodeYamlSerializer::Export(Node, Out);

	TestEqual(TEXT("type"), FString(Out["type"].as<std::string>().c_str()), FString(TEXT("K2Node_CallFunction")));
	TestEqual(TEXT("function"), FString(Out["function"].as<std::string>().c_str()), FString(TEXT("PrintString")));
	TestTrue(TEXT("member_ref exists"), Out["member_ref"].IsDefined());

	return true;
}

// UK2Node_MathExpression → type + expression only (no pins)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestNode_MathExpression,
	"UnrealAssetYAML.Node.MathExpression",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestNode_MathExpression::RunTest(const FString& Parameters)
{
	UK2Node_MathExpression* Node = NewObject<UK2Node_MathExpression>(GetTransientPackage());
	Node->NodeGuid = FGuid::NewGuid();
	Node->Expression = TEXT("A + B * 2");

	YAML::Node Out;
	FNodeYamlSerializer::Export(Node, Out);

	TestEqual(TEXT("type"), FString(Out["type"].as<std::string>().c_str()), FString(TEXT("K2Node_MathExpression")));
	TestEqual(TEXT("expression"), FString(Out["expression"].as<std::string>().c_str()), FString(TEXT("A + B * 2")));
	TestFalse(TEXT("no pins key (ReconstructNode handles them)"), Out["pins"].IsDefined());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
