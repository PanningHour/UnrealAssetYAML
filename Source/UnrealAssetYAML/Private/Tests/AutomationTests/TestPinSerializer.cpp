#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "PinYamlSerializer.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestPin_BasicFields,
	"UnrealAssetYAML.Pin.BasicFields",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestPin_BasicFields::RunTest(const FString& Parameters)
{
	UEdGraphNode* Node = NewObject<UEdGraphNode>(GetTransientPackage());
	UEdGraphPin* Pin = Node->CreatePin(EGPD_Input, TEXT("bool"), TEXT("bMyBool"));

	YAML::Node Out;
	FPinYamlSerializer::Export(Pin, Out);

	TestEqual(TEXT("name"), FString(Out["name"].as<std::string>().c_str()), FString(TEXT("bMyBool")));
	TestEqual(TEXT("dir"), FString(Out["dir"].as<std::string>().c_str()), FString(TEXT("in")));
	TestEqual(TEXT("type"), FString(Out["type"].as<std::string>().c_str()), FString(TEXT("bool")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestPin_DefaultValue,
	"UnrealAssetYAML.Pin.DefaultValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestPin_DefaultValue::RunTest(const FString& Parameters)
{
	UEdGraphNode* Node = NewObject<UEdGraphNode>(GetTransientPackage());
	UEdGraphPin* Pin = Node->CreatePin(EGPD_Input, TEXT("bool"), TEXT("bFlag"));
	Pin->DefaultValue = TEXT("true");

	YAML::Node Out;
	FPinYamlSerializer::Export(Pin, Out);

	TestEqual(TEXT("default"), FString(Out["default"].as<std::string>().c_str()), FString(TEXT("true")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestPin_LinkedTo_Format,
	"UnrealAssetYAML.Pin.LinkedToFormat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestPin_LinkedTo_Format::RunTest(const FString& Parameters)
{
	UEdGraphNode* NodeA = NewObject<UEdGraphNode>(GetTransientPackage());
	UEdGraphNode* NodeB = NewObject<UEdGraphNode>(GetTransientPackage());
	NodeB->NodeGuid = FGuid::NewGuid();

	UEdGraphPin* PinOut = NodeA->CreatePin(EGPD_Output, TEXT("exec"), TEXT("Then"));
	UEdGraphPin* PinIn  = NodeB->CreatePin(EGPD_Input,  TEXT("exec"), TEXT("Execute"));
	PinOut->LinkedTo.Add(PinIn);

	YAML::Node Out;
	FPinYamlSerializer::Export(PinOut, Out);

	TestTrue(TEXT("links is sequence"), Out["links"].IsSequence());
	TestEqual(TEXT("links count"), (int)Out["links"].size(), 1);

	FString LinkStr(Out["links"][0].as<std::string>().c_str());
	TestTrue(TEXT("links ends with .Execute"), LinkStr.EndsWith(TEXT(".Execute")));
	// format: TypeLabel_XXXXXXXX.PinName
	TestTrue(TEXT("links contains underscore"), LinkStr.Contains(TEXT("_")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestPin_SubPins,
	"UnrealAssetYAML.Pin.SubPins",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestPin_SubPins::RunTest(const FString& Parameters)
{
	UEdGraphNode* Node = NewObject<UEdGraphNode>(GetTransientPackage());
	UEdGraphPin* StructPin = Node->CreatePin(EGPD_Input, TEXT("struct"), TEXT("Location"));
	UEdGraphPin* SubX = Node->CreatePin(EGPD_Input, TEXT("float"), TEXT("Location_X"));
	UEdGraphPin* SubY = Node->CreatePin(EGPD_Input, TEXT("float"), TEXT("Location_Y"));
	StructPin->SubPins.Add(SubX);
	StructPin->SubPins.Add(SubY);

	YAML::Node Out;
	FPinYamlSerializer::Export(StructPin, Out);

	TestTrue(TEXT("sub_pins is sequence"), Out["sub_pins"].IsSequence());
	TestEqual(TEXT("sub_pins count"), (int)Out["sub_pins"].size(), 2);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
