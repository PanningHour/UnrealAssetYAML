#include "TimelineYamlSerializer.h"
#include "Engine/TimelineTemplate.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"
#include "Curves/RichCurve.h"

static void ExportRichCurveKeys(const FRichCurve& Curve, YAML::Node& OutKeys)
{
	OutKeys = YAML::Node(YAML::NodeType::Sequence);
	for (auto It = Curve.GetKeyHandleIterator(); It; ++It)
	{
		const FRichCurveKey& Key = Curve.GetKey(*It);
		YAML::Node KeyNode;
		KeyNode["time"]  = Key.Time;
		KeyNode["value"] = Key.Value;
		OutKeys.push_back(KeyNode);
	}
}

void FTimelineYamlSerializer::Export(const UTimelineTemplate* Timeline, YAML::Node& OutNode)
{
	if (!Timeline) return;

	OutNode["length"]    = Timeline->TimelineLength;
	OutNode["loop"]      = (bool)Timeline->bLoop;
	OutNode["auto_play"] = (bool)Timeline->bAutoPlay;

	// Float tracks
	YAML::Node FloatTracksSeq(YAML::NodeType::Sequence);
	for (const FTTFloatTrack& Track : Timeline->FloatTracks)
	{
		YAML::Node TrackNode;
		TrackNode["name"] = std::string(TCHAR_TO_UTF8(*Track.GetTrackName().ToString()));
		if (Track.CurveFloat)
		{
			YAML::Node Keys;
			ExportRichCurveKeys(Track.CurveFloat->FloatCurve, Keys);
			TrackNode["keys"] = Keys;
		}
		FloatTracksSeq.push_back(TrackNode);
	}
	OutNode["float_tracks"] = FloatTracksSeq;

	// Vector tracks
	YAML::Node VectorTracksSeq(YAML::NodeType::Sequence);
	for (const FTTVectorTrack& Track : Timeline->VectorTracks)
	{
		YAML::Node TrackNode;
		TrackNode["name"] = std::string(TCHAR_TO_UTF8(*Track.GetTrackName().ToString()));
		VectorTracksSeq.push_back(TrackNode);
	}
	OutNode["vector_tracks"] = VectorTracksSeq;

	// Event tracks
	YAML::Node EventTracksSeq(YAML::NodeType::Sequence);
	for (const FTTEventTrack& Track : Timeline->EventTracks)
	{
		YAML::Node TrackNode;
		TrackNode["name"] = std::string(TCHAR_TO_UTF8(*Track.GetTrackName().ToString()));
		EventTracksSeq.push_back(TrackNode);
	}
	OutNode["event_tracks"] = EventTracksSeq;
}
