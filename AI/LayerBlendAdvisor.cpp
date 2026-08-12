/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LayerBlendAdvisor.h"
#include "spdlog/spdlog.h"
#include <algorithm>

namespace xLights::AI {

std::vector<LayerBlendIssue> LayerBlendAdvisor::DetectLayerBlendIssues(
    const std::string& topEffect,
    const std::string& bottomEffect,
    const std::string& currentBlendMode,
    float currentOpacity,
    const std::string& topColorHex,
    const std::string& bottomColorHex,
    int strobeFrequency)
{
    std::vector<LayerBlendIssue> issues;

    // Rule 1: Check Color Muddying
    std::string warnMsg, suggestMode;
    if (CheckColorMuddying(topColorHex, bottomColorHex, currentBlendMode, warnMsg, suggestMode)) {
        LayerBlendIssue issue;
        issue.issueType = "COLOR_MUDDYING";
        issue.severity = "WARNING";
        issue.description = "Complementary saturated colors in Normal blend mode cause visual muddying / brown tint on LED pixels.";
        issue.suggestedBlendMode = suggestMode;
        issue.suggestedOpacity = 0.85f;
        issues.push_back(issue);
    }

    // Rule 2: Check Spatial Occlusion
    LayerBlendIssue occIssue;
    if (CheckSpatialOcclusion(topEffect, bottomEffect, currentBlendMode, currentOpacity, occIssue)) {
        issues.push_back(occIssue);
    }

    // Rule 3: Check Strobe Overpowering
    LayerBlendIssue strobeIssue;
    if (CheckStrobeOverpowering(topEffect, strobeFrequency, currentBlendMode, strobeIssue)) {
        issues.push_back(strobeIssue);
    }

    return issues;
}

bool LayerBlendAdvisor::CheckSpatialOcclusion(
    const std::string& topEffect,
    const std::string& bottomEffect,
    const std::string& currentBlendMode,
    float currentOpacity,
    LayerBlendIssue& outIssue)
{
    std::string topLower = topEffect;
    std::string botLower = bottomEffect;
    std::transform(topLower.begin(), topLower.end(), topLower.begin(), ::tolower);
    std::transform(botLower.begin(), botLower.end(), botLower.begin(), ::tolower);

    bool topOpaque = (topLower.find("colorwash") != std::string::npos || topLower.find("bars") != std::string::npos || topLower.find("picture") != std::string::npos);
    bool botDetailed = (botLower.find("fire") != std::string::npos || botLower.find("plasma") != std::string::npos || botLower.find("meteors") != std::string::npos || botLower.find("wave") != std::string::npos);

    if (topOpaque && botDetailed && currentBlendMode == "Normal" && currentOpacity >= 0.9f) {
        outIssue.issueType = "SPATIAL_OCCLUSION";
        outIssue.severity = "WARNING";
        outIssue.description = "Opaque top effect '" + topEffect + "' completely obscures detailed background effect '" + bottomEffect + "'.";
        outIssue.suggestedBlendMode = "Additive";
        outIssue.suggestedOpacity = 0.75f;
        return true;
    }

    return false;
}

bool LayerBlendAdvisor::CheckStrobeOverpowering(
    const std::string& topEffect,
    int strobeFrequency,
    const std::string& currentBlendMode,
    LayerBlendIssue& outIssue)
{
    std::string topLower = topEffect;
    std::transform(topLower.begin(), topLower.end(), topLower.begin(), ::tolower);

    if ((topLower.find("strobe") != std::string::npos || strobeFrequency > 20) && currentBlendMode == "Normal") {
        outIssue.issueType = "STROBE_OVERPOWERING";
        outIssue.severity = "SUGGESTION";
        outIssue.description = "High frequency strobe in Normal mode produces harsh clipping. Switch to Additive blend.";
        outIssue.suggestedBlendMode = "Additive";
        outIssue.suggestedOpacity = 0.8f;
        return true;
    }

    return false;
}

} // namespace xLights::AI
