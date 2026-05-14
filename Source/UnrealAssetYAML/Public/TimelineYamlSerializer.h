#pragma once

#include "CoreMinimal.h"
#include <yaml-cpp/yaml.h>

class UTimelineTemplate;

class UNREALASSETYAML_API FTimelineYamlSerializer
{
public:
	static void Export(const UTimelineTemplate* Timeline, YAML::Node& OutNode);
};
