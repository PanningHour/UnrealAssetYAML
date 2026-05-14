#pragma once

#include "CoreMinimal.h"
#include <yaml-cpp/yaml.h>
#include "Engine/MemberReference.h"

class UNREALASSETYAML_API FMemberRefSerializer
{
public:
	static void Export(const FMemberReference& Ref, YAML::Node& OutNode);
};
