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
#include <memory>
#include <nlohmann/json.hpp>

namespace xLights::AI {

enum class NarrativeStyle {
    FESTIVE_CHRISTMAS_STORY = 0,   ///< Whimsical holiday story (Santa, Reindeer)
    RADIO_DJ_INTRO = 1,           ///< High-energy broadcast show intro
    HUMOROUS_FAMILY_WELCOME = 2,  ///< Comedy/family-friendly greeting
    COUNTDOWN_ANNOUNCEMENT = 3,   ///< Dramatic 10-second show launch countdown
    HALLOWEEN_SPOOKY_TALE = 4     ///< Ghostly theatrical narrative
};

enum class VoiceCharacter {
    SANTA_CLAUS = 0,              ///< Warm, deep baritone with festive laughter
    FRIENDLY_ELF = 1,             ///< High-energy, playful, upbeat voice
    BROADCAST_RADIO_DJ = 2,       ///< Polished, booming radio host voice
    CHILD_STORYTELLER = 3,        ///< Gentle, magical wonder narrative voice
    CINEMATIC_EPIC_TRAILER = 4    ///< Deep dramatic movie trailer voice
};

struct NarrativeTimingMark {
    uint32_t startMs{0};
    uint32_t endMs{0};
    std::string word;
    std::string phonemeVisemeHint; ///< e.g. "AI", "O", "MBP" for singing face sync
    float audioIntensity{1.0f};
};

struct GeneratedNarrativeResult {
    bool success{true};
    std::string scriptText;
    std::string audioFilePath{"narrative_output.wav"};
    uint32_t totalDurationMs{15000};
    std::vector<NarrativeTimingMark> timingMarks;
    std::vector<std::string> recommendedLightingCues;

    [[nodiscard]] std::string ExportXTimingXml(const std::string& trackName = "AI Voiceover Narrative") const;
    [[nodiscard]] nlohmann::json ToJson() const;
};

struct NarrativePromptParameters {
    std::string showTitle{"Smith Family Lights Spectacular"};
    std::string familyOrCityName{"The Smiths"};
    NarrativeStyle style{NarrativeStyle::FESTIVE_CHRISTMAS_STORY};
    VoiceCharacter voice{VoiceCharacter::SANTA_CLAUS};
    float speechPaceMultiplier{1.0f}; ///< 0.75x to 1.5x
    int targetDurationSeconds{15};
    bool generateSingingFaceVisemes{true};
    std::string customDetailsPrompt;
};

class ShowNarrativeComposerAI {
public:
    static GeneratedNarrativeResult GenerateNarrative(const NarrativePromptParameters& params);
    static std::string ComposeScript(const NarrativePromptParameters& params);
    static std::vector<NarrativeTimingMark> GenerateAlignedTimings(const std::string& script, float paceMultiplier);
};

} // namespace xLights::AI
