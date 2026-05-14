#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "VariableYamlSerializer.h"
#include "Engine/Blueprint.h"
#include "EdGraphSchema_K2.h"

#if WITH_DEV_AUTOMATION_TESTS

static FBPVariableDescription MakeVar(FName Name, FName PinCategory, const FString& Default = TEXT(""))
{
	FBPVariableDescription Var;
	Var.VarName = Name;
	Var.VarType.PinCategory = PinCategory;
	Var.DefaultValue = Default;
	return Var;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestVariable_Int,
	"UnrealAssetYAML.Variable.Int",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestVariable_Int::RunTest(const FString& Parameters)
{
	FBPVariableDescription Var = MakeVar(TEXT("Health"), UEdGraphSchema_K2::PC_Int, TEXT("100"));

	YAML::Node Out;
	FVariableYamlSerializer::Export(Var, Out);

	TestEqual(TEXT("name"), FString(Out["name"].as<std::string>().c_str()), FString(TEXT("Health")));
	TestEqual(TEXT("type"), FString(Out["type"].as<std::string>().c_str()), FString(TEXT("int")));
	TestEqual(TEXT("default"), FString(Out["default"].as<std::string>().c_str()), FString(TEXT("100")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestVariable_Bool,
	"UnrealAssetYAML.Variable.Bool",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestVariable_Bool::RunTest(const FString& Parameters)
{
	FBPVariableDescription Var = MakeVar(TEXT("bIsAlive"), UEdGraphSchema_K2::PC_Boolean, TEXT("true"));

	YAML::Node Out;
	FVariableYamlSerializer::Export(Var, Out);

	TestEqual(TEXT("name"), FString(Out["name"].as<std::string>().c_str()), FString(TEXT("bIsAlive")));
	TestEqual(TEXT("type"), FString(Out["type"].as<std::string>().c_str()), FString(TEXT("bool")));
	TestEqual(TEXT("default"), FString(Out["default"].as<std::string>().c_str()), FString(TEXT("true")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestVariable_Float,
	"UnrealAssetYAML.Variable.Float",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestVariable_Float::RunTest(const FString& Parameters)
{
	FBPVariableDescription Var = MakeVar(TEXT("Speed"), UEdGraphSchema_K2::PC_Real, TEXT("600.0"));

	YAML::Node Out;
	FVariableYamlSerializer::Export(Var, Out);

	TestEqual(TEXT("name"), FString(Out["name"].as<std::string>().c_str()), FString(TEXT("Speed")));
	TestEqual(TEXT("default"), FString(Out["default"].as<std::string>().c_str()), FString(TEXT("600.0")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestVariable_String,
	"UnrealAssetYAML.Variable.String",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestVariable_String::RunTest(const FString& Parameters)
{
	FBPVariableDescription Var = MakeVar(TEXT("PlayerName"), UEdGraphSchema_K2::PC_String, TEXT("Hero"));

	YAML::Node Out;
	FVariableYamlSerializer::Export(Var, Out);

	TestEqual(TEXT("name"), FString(Out["name"].as<std::string>().c_str()), FString(TEXT("PlayerName")));
	TestEqual(TEXT("type"), FString(Out["type"].as<std::string>().c_str()), FString(TEXT("string")));
	TestEqual(TEXT("default"), FString(Out["default"].as<std::string>().c_str()), FString(TEXT("Hero")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestVariable_TransientFlagged,
	"UnrealAssetYAML.Variable.TransientFlagged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestVariable_TransientFlagged::RunTest(const FString& Parameters)
{
	FBPVariableDescription Var = MakeVar(TEXT("TempVar"), UEdGraphSchema_K2::PC_Int, TEXT("0"));
	Var.PropertyFlags |= CPF_Transient;

	YAML::Node Out;
	FVariableYamlSerializer::Export(Var, Out);

	// Transient 변수는 flags에 transient 표시
	TestTrue(TEXT("flags key exists"), Out["flags"].IsDefined());
	bool bTransientInFlags = false;
	if (Out["flags"].IsSequence())
	{
		for (const auto& Flag : Out["flags"])
		{
			if (FString(Flag.as<std::string>().c_str()) == TEXT("Transient"))
				bTransientInFlags = true;
		}
	}
	TestTrue(TEXT("Transient flag recorded"), bTransientInFlags);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestCDO_Basic,
	"UnrealAssetYAML.Variable.CDOBasic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestCDO_Basic::RunTest(const FString& Parameters)
{
	// UObject CDO: 직렬화할 user-facing 프로퍼티 없음 → 빈 map
	UObject* CDO = UObject::StaticClass()->GetDefaultObject();

	YAML::Node Out;
	FVariableYamlSerializer::ExportCDO(CDO, Out);

	// 결과는 Map 노드여야 함 (비어있어도 OK)
	TestTrue(TEXT("CDO result is map"), Out.IsMap());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
