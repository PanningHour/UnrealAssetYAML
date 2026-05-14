using UnrealBuildTool;
using System.IO;

public class UnrealAssetYAML : ModuleRules
{
	public UnrealAssetYAML(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"BlueprintGraph",
			"KismetCompiler",
		});

		string ThirdPartyPath = Path.Combine(ModuleDirectory, "..", "ThirdParty", "yaml-cpp");
		PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "include"));
		PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "lib", "Win64", "yaml-cpp.lib"));
	}
}
