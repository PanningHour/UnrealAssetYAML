#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "TimelineYamlSerializer.h"
#include "Engine/TimelineTemplate.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"

#if WITH_DEV_AUTOMATION_TESTS

// Timeline 기본 프로퍼티: length, loop, auto_play
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestTimeline_BasicProperties,
	"UnrealAssetYAML.Timeline.BasicProperties",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestTimeline_BasicProperties::RunTest(const FString& Parameters)
{
	UTimelineTemplate* TL = NewObject<UTimelineTemplate>(GetTransientPackage());
	TL->TimelineLength = 5.0f;
	TL->bLoop = true;
	TL->bAutoPlay = false;

	YAML::Node Out;
	FTimelineYamlSerializer::Export(TL, Out);

	TestTrue(TEXT("length exists"), Out["length"].IsDefined());
	TestEqual(TEXT("length"), Out["length"].as<float>(), 5.0f);
	TestEqual(TEXT("loop"), Out["loop"].as<bool>(), true);
	TestEqual(TEXT("auto_play"), Out["auto_play"].as<bool>(), false);
	return true;
}

// FloatTrack 추가 → float_tracks 배열 + track name
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestTimeline_FloatTrack,
	"UnrealAssetYAML.Timeline.FloatTrack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestTimeline_FloatTrack::RunTest(const FString& Parameters)
{
	UTimelineTemplate* TL = NewObject<UTimelineTemplate>(GetTransientPackage());

	FTTFloatTrack Track;
	Track.SetTrackName(TEXT("Alpha"), TL);
	Track.CurveFloat = NewObject<UCurveFloat>(GetTransientPackage());
	TL->FloatTracks.Add(Track);

	YAML::Node Out;
	FTimelineYamlSerializer::Export(TL, Out);

	TestTrue(TEXT("float_tracks is sequence"), Out["float_tracks"].IsSequence());
	TestEqual(TEXT("float_tracks count"), (int)Out["float_tracks"].size(), 1);
	TestEqual(TEXT("track name"),
		FString(Out["float_tracks"][0]["name"].as<std::string>().c_str()),
		FString(TEXT("Alpha")));
	return true;
}

// CurveFloat 키 데이터 보존
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestTimeline_CurveData,
	"UnrealAssetYAML.Timeline.CurveData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestTimeline_CurveData::RunTest(const FString& Parameters)
{
	UTimelineTemplate* TL = NewObject<UTimelineTemplate>(GetTransientPackage());

	UCurveFloat* Curve = NewObject<UCurveFloat>(GetTransientPackage());
	Curve->FloatCurve.AddKey(0.0f, 0.0f);
	Curve->FloatCurve.AddKey(1.0f, 1.0f);

	FTTFloatTrack Track;
	Track.SetTrackName(TEXT("Fade"), TL);
	Track.CurveFloat = Curve;
	TL->FloatTracks.Add(Track);

	YAML::Node Out;
	FTimelineYamlSerializer::Export(TL, Out);

	const YAML::Node& Keys = Out["float_tracks"][0]["keys"];
	TestTrue(TEXT("keys is sequence"), Keys.IsSequence());
	TestEqual(TEXT("key count"), (int)Keys.size(), 2);
	TestEqual(TEXT("key[0] time"), Keys[0]["time"].as<float>(), 0.0f);
	TestEqual(TEXT("key[0] value"), Keys[0]["value"].as<float>(), 0.0f);
	TestEqual(TEXT("key[1] time"), Keys[1]["time"].as<float>(), 1.0f);
	TestEqual(TEXT("key[1] value"), Keys[1]["value"].as<float>(), 1.0f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
