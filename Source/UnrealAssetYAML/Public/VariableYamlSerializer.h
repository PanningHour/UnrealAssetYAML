#pragma once

#include "CoreMinimal.h"
#include <yaml-cpp/yaml.h>
#include "Engine/Blueprint.h"

class UNREALASSETYAML_API FVariableYamlSerializer
{
public:
	static void Export(const FBPVariableDescription& Var, YAML::Node& OutNode);
	static void ExportCDO(const UObject* CDO, YAML::Node& OutNode);
};
