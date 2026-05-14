#pragma once

#include "CoreMinimal.h"
#include <yaml-cpp/yaml.h>

class USimpleConstructionScript;
class USCS_Node;

class UNREALASSETYAML_API FSCSYamlSerializer
{
public:
	static void Export(const USimpleConstructionScript* SCS, YAML::Node& OutNode);
	static void ExportNode(const USCS_Node* Node, YAML::Node& OutNode);
};
