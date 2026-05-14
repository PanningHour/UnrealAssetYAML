#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "YamlAssetRegistry.h"
#include "Engine/Texture2D.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestRegistry_BasicDispatch,
	"UnrealAssetYAML.Registry.BasicDispatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestRegistry_BasicDispatch::RunTest(const FString& Parameters)
{
	FYamlAssetRegistry::UnregisterAll();

	bool bExportCalled = false;
	FYamlAssetRegistry::Register<UTexture2D>(
		[&](const UTexture2D* Asset, FYamlExportContext& Ctx) { bExportCalled = true; },
		[&](FYamlImportContext& Ctx, UTexture2D* Asset) {}
	);

	UTexture2D* Obj = NewObject<UTexture2D>(GetTransientPackage());
	YAML::Node OutNode;
	bool bResult = FYamlAssetRegistry::TryExport(Obj, OutNode);

	TestTrue(TEXT("TryExport returns true"), bResult);
	TestTrue(TEXT("Export handler called"), bExportCalled);

	FYamlAssetRegistry::UnregisterAll();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestRegistry_UnregisteredReturnsFalse,
	"UnrealAssetYAML.Registry.UnregisteredReturnsFalse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestRegistry_UnregisteredReturnsFalse::RunTest(const FString& Parameters)
{
	FYamlAssetRegistry::UnregisterAll();

	UTexture2D* Obj = NewObject<UTexture2D>(GetTransientPackage());
	YAML::Node OutNode;
	bool bResult = FYamlAssetRegistry::TryExport(Obj, OutNode);

	TestFalse(TEXT("Unregistered returns false"), bResult);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestRegistry_ParentFallback,
	"UnrealAssetYAML.Registry.ParentFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestRegistry_ParentFallback::RunTest(const FString& Parameters)
{
	FYamlAssetRegistry::UnregisterAll();

	bool bExportCalled = false;
	FYamlAssetRegistry::Register<UObject>(
		[&](const UObject* Asset, FYamlExportContext& Ctx) { bExportCalled = true; },
		[&](FYamlImportContext& Ctx, UObject* Asset) {}
	);

	// UTexture2D 핸들러 없음 → UObject fallback
	UTexture2D* Tex = NewObject<UTexture2D>(GetTransientPackage());
	YAML::Node OutNode;
	bool bResult = FYamlAssetRegistry::TryExport(Tex, OutNode);

	TestTrue(TEXT("Parent fallback returns true"), bResult);
	TestTrue(TEXT("Parent handler called"), bExportCalled);

	FYamlAssetRegistry::UnregisterAll();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestRegistry_ReplaceSemantics,
	"UnrealAssetYAML.Registry.ReplaceSemantics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestRegistry_ReplaceSemantics::RunTest(const FString& Parameters)
{
	FYamlAssetRegistry::UnregisterAll();

	bool bParentCalled = false;
	bool bChildCalled = false;

	FYamlAssetRegistry::Register<UObject>(
		[&](const UObject* Asset, FYamlExportContext& Ctx) { bParentCalled = true; },
		[&](FYamlImportContext& Ctx, UObject* Asset) {}
	);
	FYamlAssetRegistry::Register<UTexture2D>(
		[&](const UTexture2D* Asset, FYamlExportContext& Ctx) { bChildCalled = true; },
		[&](FYamlImportContext& Ctx, UTexture2D* Asset) {}
	);

	UTexture2D* Tex = NewObject<UTexture2D>(GetTransientPackage());
	YAML::Node OutNode;
	FYamlAssetRegistry::TryExport(Tex, OutNode);

	TestTrue(TEXT("Child handler called"), bChildCalled);
	TestFalse(TEXT("Parent handler NOT called (replace semantics)"), bParentCalled);

	FYamlAssetRegistry::UnregisterAll();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestRegistry_CallSuper,
	"UnrealAssetYAML.Registry.CallSuper",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestRegistry_CallSuper::RunTest(const FString& Parameters)
{
	FYamlAssetRegistry::UnregisterAll();

	TArray<FString> CallOrder;

	FYamlAssetRegistry::Register<UObject>(
		[&](const UObject* Asset, FYamlExportContext& Ctx) { CallOrder.Add(TEXT("UObject")); },
		[&](FYamlImportContext& Ctx, UObject* Asset) {}
	);
	FYamlAssetRegistry::Register<UTexture2D>(
		[&](const UTexture2D* Asset, FYamlExportContext& Ctx)
		{
			CallOrder.Add(TEXT("UTexture2D"));
			Ctx.CallSuper(Asset);
		},
		[&](FYamlImportContext& Ctx, UTexture2D* Asset) {}
	);

	UTexture2D* Tex = NewObject<UTexture2D>(GetTransientPackage());
	YAML::Node OutNode;
	FYamlAssetRegistry::TryExport(Tex, OutNode);

	TestEqual(TEXT("Two handlers called"), CallOrder.Num(), 2);
	if (CallOrder.Num() == 2)
	{
		TestEqual(TEXT("First: UTexture2D"), CallOrder[0], FString(TEXT("UTexture2D")));
		TestEqual(TEXT("Second: UObject via CallSuper"), CallOrder[1], FString(TEXT("UObject")));
	}

	FYamlAssetRegistry::UnregisterAll();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
