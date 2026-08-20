/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/media/ShowNarrativeComposerAI.h"
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>

namespace xLights::AI {

std::string GeneratedNarrativeResult::ExportXTimingXml(const std::string& trackName) const {
    std::ostringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    ss << "<timing name=\"" << trackName << "\" version=\"1.0\">\n";
    ss << "  <timingList>\n";

    for (const auto& mark : timingMarks) {
        ss << "    <mark start=\"" << mark.startMs << "\" end=\"" << mark.endMs
           << "\" label=\"" << mark.word << "\" viseme=\"" << mark.phonemeVisemeHint << "\" />\n";
    }

    ss << "  </timingList>\n";
    ss << "</timing>\n";
    return ss.str();
}

nlohmann::json GeneratedNarrativeResult::ToJson() const {
    nlohmann::json marksJson = nlohmann::json::array();
    for (const auto& m : timingMarks) {
        marksJson.push_back({
            {"startMs", m.startMs},
            {"endMs", m.endMs},
            {"word", m.word},
            {"phonemeVisemeHint", m.phonemeVisemeHint},
            {"audioIntensity", m.audioIntensity}
        });
    }

    return {
        {"success", success},
        {"scriptText", scriptText},
        {"audioFilePath", audioFilePath},
        {"totalDurationMs", totalDurationMs},
        {"timingMarks", marksJson},
        {"recommendedLightingCues", recommendedLightingCues}
    };
}

std::string ShowNarrativeComposerAI::ComposeScript(const NarrativePromptParameters& params) {
    if (!params.customDetailsPrompt.empty()) {
        return params.customDetailsPrompt;
    }

    switch (params.style) {
        case NarrativeStyle::FESTIVE_CHRISTMAS_STORY:
            return "Ho ho ho! Merry Christmas everyone! Welcome to " + params.showTitle +
                   ". Tune your radio, sit back, and get ready for a magical night of lights with " + params.familyOrCityName + "!";
        case NarrativeStyle::RADIO_DJ_INTRO:
            return "You are tuned in LIVE to the biggest holiday light show on the block! This is " +
                   params.showTitle + "! Crank up the volume and let the spectacle begin!";
        case NarrativeStyle::HUMOROUS_FAMILY_WELCOME:
            return "Welcome neighbors and visitors! The electric meter is spinning at warp speed, and " +
                   params.familyOrCityName + " welcomes you to our crazy light show!";
        case NarrativeStyle::COUNTDOWN_ANNOUNCEMENT:
            return "Ten, nine, eight, seven, six, five, four, three, two, one... Launch show!";
        case NarrativeStyle::HALLOWEEN_SPOOKY_TALE:
            return "Enter if you dare! The spirits have awakened at " + params.showTitle +
                   ". Keep your eyes open and prepare for a frighteningly good time!";
    }
    return "Welcome to the show!";
}

std::vector<NarrativeTimingMark> ShowNarrativeComposerAI::GenerateAlignedTimings(const std::string& script, float paceMultiplier) {
    std::vector<NarrativeTimingMark> marks;
    std::istringstream ss(script);
    std::string word;

    uint32_t currentMs = 500; // 500ms lead-in
    float baseWordDurationMs = 320.0f / std::max(0.5f, paceMultiplier);

    while (ss >> word) {
        // Strip basic punctuation for phoneme mapping
        std::string clean = word;
        while (!clean.empty() && (clean.back() == '.' || clean.back() == '!' || clean.back() == ',' || clean.back() == '?')) {
            clean.pop_back();
        }

        NarrativeTimingMark mark;
        mark.startMs = currentMs;
        mark.endMs = currentMs + static_cast<uint32_t>(baseWordDurationMs);
        mark.word = word;

        // Phoneme viseme hint heuristic
        char first = clean.empty() ? 'A' : static_cast<char>(std::toupper(clean.front()));
        if (first == 'M' || first == 'B' || first == 'P') mark.phonemeVisemeHint = "MBP";
        else if (first == 'O' || first == 'U' || first == 'W') mark.phonemeVisemeHint = "O";
        else if (first == 'E' || first == 'I') mark.phonemeVisemeHint = "E";
        else if (first == 'L') mark.phonemeVisemeHint = "L";
        else mark.phonemeVisemeHint = "AI";

        mark.audioIntensity = 0.9f;
        marks.push_back(mark);

        currentMs = mark.endMs + 80; // 80ms word gap
    }

    return marks;
}

GeneratedNarrativeResult ShowNarrativeComposerAI::GenerateNarrative(const NarrativePromptParameters& params) {
    GeneratedNarrativeResult result;
    result.scriptText = ComposeScript(params);
    result.timingMarks = GenerateAlignedTimings(result.scriptText, params.speechPaceMultiplier);

    if (!result.timingMarks.empty()) {
        result.totalDurationMs = result.timingMarks.back().endMs + 500;
    } else {
        result.totalDurationMs = 5000;
    }

    result.recommendedLightingCues = {
        "00:00:00.500 - Warm White Shimmer Fade-In across Matrix & Trees",
        "00:00:04.200 - Singing Tree Mouth Lip-Sync track mapped to 'AI Voiceover Narrative'",
        "00:00:10.000 - High-Energy Color Strobe Explosion synchronized with Climax"
    };

    spdlog::info("ShowNarrativeComposerAI: Generated narrative script ({} words, duration {}ms)",
                 result.timingMarks.size(), result.totalDurationMs);
    return result;
}

} // namespace xLights::AI
