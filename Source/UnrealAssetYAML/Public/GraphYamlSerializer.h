#pragma once

#include "CoreMinimal.h"
#include <yaml-cpp/yaml.h>

class UEdGraph;

class UNREALASSETYAML_API FGraphYamlSerializer
{
public:
	static void Export(const UEdGraph* Graph, YAML::Node& OutNode);
	static void Import(const YAML::Node& GraphYaml, UEdGraph* Graph);
};
