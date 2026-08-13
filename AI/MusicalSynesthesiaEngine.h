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

namespace xLights::AI {

struct SynesthesiaPalette {
    std::string keySignature;               // e.g. "C Minor", "G Major"
    std::string emotionalMood;              // "Melancholic", "Energetic", "Triumphant"
    std::vector<std::string> hexColors;    // e.g. ["#001F3F", "#7FDBFF", "#F012BE"]
    std::string recommendedEffectType;     // "Shimmer", "Bars", "Marquee", "SingleStrand"
    float recommendedAttackSpeed = 1.0f;
};

class MusicalSynesthesiaEngine {
public:
    MusicalSynesthesiaEngine() = default;
    ~MusicalSynesthesiaEngine() = default;

    [[nodiscard]] static SynesthesiaPalette AnalyzeAudioBarSynesthesia(
        const std::vector<float>& pcmBuffer,
        int sampleRate,
        int startMs,
        int endMs
    );
};

} // namespace xLights::AI
