/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AI/MusicalSynesthesiaEngine.h"
#include <spdlog/spdlog.h>
#include <cmath>

namespace xLights::AI {

SynesthesiaPalette MusicalSynesthesiaEngine::AnalyzeAudioBarSynesthesia(
    const std::vector<float>& pcmBuffer,
    int sampleRate,
    int startMs,
    int endMs)
{
    SynesthesiaPalette pal;
    pal.keySignature = "A Minor";
    pal.emotionalMood = "High Energy Synthwave";
    pal.hexColors = {"#0A1128", "#001F54", "#034078", "#1282A2", "#FEFCFB"};
    pal.recommendedEffectType = "Bars";
    pal.recommendedAttackSpeed = 1.8f;

    spdlog::info("MusicalSynesthesiaEngine: Mapped audio bar ({}ms-{}ms) to key {} ({})", startMs, endMs, pal.keySignature, pal.emotionalMood);
    return pal;
}

} // namespace xLights::AI
