#pragma once

#include "CoreMinimal.h"
#include <yaml-cpp/yaml.h>

class UEdGraphNode;

class UNREALASSETYAML_API FNodeYamlSerializer
{
public:
	static void Export(const UEdGraphNode* Node, YAML::Node& OutNode);
};
