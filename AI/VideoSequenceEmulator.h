/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include "AI/AIConfigurationManager.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>
#include <nlohmann/json.hpp>

namespace xLights::AI {

struct VideoSourceInput {
    std::string filePath;
    std::string videoUrl;
    bool hasTimeRange = false;
    int timeStartMs = 0;
    int timeEndMs = 60000;
    std::string userPrompt;
    std::vector<std::string> targetPropFilters;
};

struct VideoVisualTrack {
    int timeMs = 0;
    int durationMs = 1000;
    std::string dominantColorHex = "#FF0000";
    std::string secondaryColorHex = "#FFFFFF";
    float brightness = 1.0f;
    std::string recognizedEffect = "Bars";
    std::string spatialDirection = "LeftToRight";
    std::string sourcePropCategory = "MegaTree";
    float energyLevel = 0.8f;
};

struct VideoAnalysisResult {
    std::string sourceTitle;
    int detectedDurationMs = 60000;
    float detectedBpm = 120.0f;
    std::vector<int> detectedBeatMarkers;
    std::vector<std::string> dominantPalette;
    std::vector<VideoVisualTrack> visualTracks;
    std::vector<std::string> propsDetectedInVideo;
    std::string executiveSummary;
    std::vector<std::string> observations;
    std::vector<std::string> adaptationChallenges;
    std::string rawJsonData;
};

struct AdaptationStrategy {
    std::string id;
    std::string name;
    std::string description;
    std::string tradeoffs;
    bool isRecommended = false;
};

struct EmulatedEffectCue {
    std::string targetPropName;
    std::string effectType;
    int layerIndex = 0;
    int startMs = 0;
    int endMs = 1000;
    std::string primaryColor = "#FF0000";
    std::string secondaryColor = "#00FFCC";
    std::string effectSettings;
    std::string rationale;
};

struct ConsultationQuestion {
    std::string id;
    std::string questionText;
    std::vector<std::string> options;
    int defaultOptionIndex = 0;
    std::string explanation;
};

struct VideoSequencePlan {
    VideoSourceInput sourceInput;
    VideoAnalysisResult analysis;
    std::string chosenStrategyId;
    std::vector<ConsultationQuestion> consultationQuestions;
    std::map<std::string, std::string> userAnswers;
    std::vector<EmulatedEffectCue> generatedCues;
    int totalCuesCount = 0;
    int targetPropsCount = 0;
    std::string statusMessage;
};

struct SequenceRefinementRequest {
    std::string tuningPrompt;
    float speedFactor = 1.0f;
    std::string colorShiftFrom;
    std::string colorShiftTo;
    float densityFactor = 1.0f;
    std::vector<std::string> targetPropSubset;
};

class VideoSequenceEmulator {
public:
    VideoSequenceEmulator();
    ~VideoSequenceEmulator() = default;

    VideoAnalysisResult AnalyzeVideoSource(const VideoSourceInput& input, const std::string& layoutXml);
    std::vector<AdaptationStrategy> SuggestStrategies(const VideoAnalysisResult& analysis, const std::string& layoutXml);
    std::vector<ConsultationQuestion> GenerateConsultationQuestions(const VideoAnalysisResult& analysis, const std::string& layoutXml);
    VideoSequencePlan GenerateEmulationPlan(
        const VideoSourceInput& input,
        const VideoAnalysisResult& analysis,
        const std::string& strategyId,
        const std::map<std::string, std::string>& userChoices,
        const std::string& layoutXml
    );
    std::string ExportPlanToXsqXml(const VideoSequencePlan& plan, int sequenceDurationMs = 0);
    VideoSequencePlan RefinePlan(const VideoSequencePlan& currentPlan, const SequenceRefinementRequest& refinement);

private:
    std::vector<std::string> ParseLayoutModelNames(const std::string& layoutXml) const;
    std::vector<std::string> ExtractColorsFromVisuals(const std::vector<VideoVisualTrack>& tracks) const;
    std::string MapVisualEffectToXLights(const std::string& recognizedEffect, const std::string& propCategory) const;
    std::string GenerateDefaultSettingsForEffect(const std::string& effectType, const VideoVisualTrack& track) const;

    mutable std::mutex m_mutex;
};

} // namespace xLights::AI
