#pragma once

#include "CoreMinimal.h"
#include <yaml-cpp/yaml.h>

class UBlueprint;
class UPackage;

class UNREALASSETYAML_API FBlueprintYamlHandler
{
public:
	static void Export(const UBlueprint* BP, YAML::Node& OutNode);
	static UBlueprint* Import(const YAML::Node& Node, UPackage* Package);
};
