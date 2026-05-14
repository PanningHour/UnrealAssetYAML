#include "YamlAssetRegistry.h"

TMap<UClass*, TUniquePtr<IYamlHandler>>& FYamlAssetRegistry::GetHandlers()
{
	static TMap<UClass*, TUniquePtr<IYamlHandler>> Handlers;
	return Handlers;
}

bool FYamlAssetRegistry::TryExport(const UObject* Asset, YAML::Node& OutNode)
{
	if (!Asset) return false;
	FYamlExportContext Ctx{OutNode};
	return ExportFromClass(Asset, Asset->GetClass(), Ctx);
}

bool FYamlAssetRegistry::ExportFromClass(const UObject* Asset, UClass* StartClass, FYamlExportContext& Ctx)
{
	auto& Handlers = GetHandlers();
	for (UClass* Class = StartClass; Class; Class = Class->GetSuperClass())
	{
		if (TUniquePtr<IYamlHandler>* Handler = Handlers.Find(Class))
		{
			Ctx.DispatchedClass = Class;
			(*Handler)->Export(Asset, Ctx);
			return true;
		}
	}
	return false;
}

UObject* FYamlAssetRegistry::TryImport(const YAML::Node& Node, UPackage* Package)
{
	return nullptr;
}

void FYamlAssetRegistry::UnregisterAll()
{
	GetHandlers().Empty();
}

void FYamlExportContext::CallSuper(const UObject* Asset)
{
	if (!DispatchedClass) return;
	UClass* SuperStart = DispatchedClass->GetSuperClass();
	if (!SuperStart) return;
	FYamlAssetRegistry::ExportFromClass(Asset, SuperStart, *this);
}

void FYamlImportContext::CallSuper(UObject* Asset)
{
	// Phase 2에서 구현
}
