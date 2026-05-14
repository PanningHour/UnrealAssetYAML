#include "MemberRefSerializer.h"

void FMemberRefSerializer::Export(const FMemberReference& Ref, YAML::Node& OutNode)
{
	OutNode["member_name"] = TCHAR_TO_UTF8(*Ref.GetMemberName().ToString());
	OutNode["guid"] = TCHAR_TO_UTF8(*Ref.GetMemberGuid().ToString(EGuidFormats::Digits).ToLower());
	OutNode["self_context"] = Ref.IsSelfContext();

	if (UClass* ParentClass = Ref.GetMemberParentClass())
	{
		OutNode["parent_class"] = TCHAR_TO_UTF8(*ParentClass->GetPathName());
	}
}
