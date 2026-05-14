#pragma once

#include "CoreMinimal.h"
#include <yaml-cpp/yaml.h>

class IYamlHandler;

struct UNREALASSETYAML_API FYamlExportContext
{
	YAML::Node& OutNode;
	UClass* DispatchedClass = nullptr;

	void CallSuper(const UObject* Asset);
};

struct UNREALASSETYAML_API FYamlImportContext
{
	const YAML::Node& InNode;
	UPackage* Package = nullptr;
	UClass* DispatchedClass = nullptr;

	void CallSuper(UObject* Asset);
};

class UNREALASSETYAML_API FYamlAssetRegistry
{
public:
	template<typename T>
	static void Register(
		TFunction<void(const T*, FYamlExportContext&)> ExportFn,
		TFunction<void(FYamlImportContext&, T*)> ImportFn);

	static bool TryExport(const UObject* Asset, YAML::Node& OutNode);
	static UObject* TryImport(const YAML::Node& Node, UPackage* Package);

	static bool ExportFromClass(const UObject* Asset, UClass* StartClass, FYamlExportContext& Ctx);

	static void UnregisterAll();

private:
	static TMap<UClass*, TUniquePtr<IYamlHandler>>& GetHandlers();
};

// ---- Internal ----

class IYamlHandler
{
public:
	virtual ~IYamlHandler() = default;
	virtual void Export(const UObject* Asset, FYamlExportContext& Ctx) const = 0;
	virtual void Import(FYamlImportContext& Ctx, UObject* Asset) const = 0;
};

template<typename T>
class TContextYamlHandler final : public IYamlHandler
{
	TFunction<void(const T*, FYamlExportContext&)> ExportFn;
	TFunction<void(FYamlImportContext&, T*)> ImportFn;
public:
	TContextYamlHandler(
		TFunction<void(const T*, FYamlExportContext&)> InExport,
		TFunction<void(FYamlImportContext&, T*)> InImport)
		: ExportFn(MoveTemp(InExport)), ImportFn(MoveTemp(InImport)) {}

	void Export(const UObject* Asset, FYamlExportContext& Ctx) const override
	{
		ExportFn(static_cast<const T*>(Asset), Ctx);
	}
	void Import(FYamlImportContext& Ctx, UObject* Asset) const override
	{
		ImportFn(Ctx, static_cast<T*>(Asset));
	}
};

template<typename T>
inline void FYamlAssetRegistry::Register(
	TFunction<void(const T*, FYamlExportContext&)> ExportFn,
	TFunction<void(FYamlImportContext&, T*)> ImportFn)
{
	GetHandlers().Add(T::StaticClass(),
		MakeUnique<TContextYamlHandler<T>>(MoveTemp(ExportFn), MoveTemp(ImportFn)));
}
