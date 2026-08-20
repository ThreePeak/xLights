/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

namespace xLights::AI {

enum class AudioStemType {
    KICK_DRUM = 0,       ///< 40Hz-120Hz sub-bass transient hits
    SNARE_CLAP = 1,      ///< 1kHz-3kHz mid-frequency punch
    VOCAL_LEAD = 2,      ///< 300Hz-3.5kHz vocal harmonics
    HIHAT_CYMBAL = 3,    ///< 6kHz-16kHz high-frequency shimmer
    FULL_MIX_BEAT = 4    ///< Aggregate musical downbeats
};

enum class ChoreographyOutputMode {
    GENERATE_TIMING_MARK_TRACK = 0, ///< Emits native .xtiming Mark Track for alignment
    GENERATE_PROP_EFFECTS = 1       ///< Inserts specified or AI-chosen effects on target props
};

struct StemOnsetMark {
    uint32_t timestampMs{0};
    float energyIntensity{1.0f}; ///< 0.0 to 1.0 peak energy
    std::string label{"Kick"};
};

struct GeneratedChoreographyResult {
    bool success{true};
    std::string audioStemName{"Kick Drum"};
    size_t detectedHitCount{0};
    std::vector<StemOnsetMark> onsets;
    std::string generatedTimingTrackXml;
    std::string generatedEffectsXml;
    std::string deltaRefinementSummary;

    [[nodiscard]] nlohmann::json ToJson() const;
    [[nodiscard]] std::string GenerateFormattedReport() const;
};

struct ChoreographyParameters {
    AudioStemType stemType{AudioStemType::KICK_DRUM};
    ChoreographyOutputMode outputMode{ChoreographyOutputMode::GENERATE_PROP_EFFECTS};
    std::string targetPropName{"MegaTree"};
    std::string desiredEffectType{"Bars"}; ///< "Bars", "Strobe", "Butterfly", "Shockwave", "AI_AUTO"
    float sensitivityThreshold{0.65f};     ///< 0.1 to 1.0
    std::string naturalLanguagePrompt;     ///< User prompt e.g. "Trigger red & white shockwave on Arches every kick"
    bool nonDestructiveDeltaMode{true};    ///< Protects existing effect layers from being wiped
};

class AudioChoreographerAI {
public:
    static GeneratedChoreographyResult AnalyzeAndChoreograph(
        const std::vector<float>& audioSamples,
        uint32_t sampleRateHz,
        const ChoreographyParameters& params
    );

    static std::string ApplyDeltaRefinement(
        const std::string& currentSequenceXml,
        const std::string& deltaPrompt,
        const GeneratedChoreographyResult& choreoResult
    );
};

} // namespace xLights::AI
