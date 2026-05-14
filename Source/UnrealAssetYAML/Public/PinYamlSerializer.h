#pragma once

#include "CoreMinimal.h"
#include <yaml-cpp/yaml.h>

class UEdGraphPin;
class UEdGraphNode;

class UNREALASSETYAML_API FPinYamlSerializer
{
public:
	static void Export(const UEdGraphPin* Pin, YAML::Node& OutNode);
	static FString MakeNodeId(const UEdGraphNode* Node);
};
