/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LayerBlendingTransitionAdvisor.h"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <sstream>

namespace xLights::AI {

nlohmann::json LayerBlendAnalysis::ToJson() const {
    nlohmann::json j;
    j["stack_description"] = stackDescription;
    j["color_harmonic_balance"] = colorHarmonicBalance;
    j["recommendations"] = nlohmann::json::array();
    for (const auto& rec : recommendations) {
        nlohmann::json rj;
        rj["top_layer_index"] = rec.topLayerIndex;
        rj["bottom_layer_index"] = rec.bottomLayerIndex;
        rj["top_effect"] = rec.topEffectName;
        rj["bottom_effect"] = rec.bottomEffectName;
        rj["recommended_blend_mode"] = rec.recommendedBlendMode;
        rj["recommended_opacity"] = rec.recommendedOpacity;
        rj["transition_type"] = rec.transitionType;
        rj["transition_duration_ms"] = rec.transitionDurationMS;
        rj["reasoning"] = rec.reasoning;
        j["recommendations"].push_back(rj);
    }
    return j;
}

LayerBlendingTransitionAdvisor::LayerBlendingTransitionAdvisor(ServiceManager* sm)
    : AISubsystemBase(sm) {}

std::future<bool> LayerBlendingTransitionAdvisor::InitializeAsync(StatusCallback callback) {
    return std::async(std::launch::async, [this, callback]() {
        if (callback) callback("Initializing LayerBlendingTransitionAdvisor...", 0.0f);
        m_isInitialized.store(true);
        if (callback) callback("LayerBlendingTransitionAdvisor initialized.", 100.0f);
        return true;
    });
}

void LayerBlendingTransitionAdvisor::Shutdown() {
    m_isInitialized.store(false);
}

LayerBlendRecommendation LayerBlendingTransitionAdvisor::RecommendBlendMode(
    const std::string& topEffect,
    const std::string& bottomEffect,
    const std::vector<std::string>& topColors,
    const std::vector<std::string>& bottomColors)
{
    LayerBlendRecommendation rec;
    rec.topEffectName = topEffect;
    rec.bottomEffectName = bottomEffect;

    std::string topLower = topEffect;
    std::string botLower = bottomEffect;
    std::transform(topLower.begin(), topLower.end(), topLower.begin(), ::tolower);
    std::transform(botLower.begin(), botLower.end(), botLower.begin(), ::tolower);

    if (topLower.find("twinkle") != std::string::npos || topLower.find("shimmer") != std::string::npos || topLower.find("strobe") != std::string::npos) {
        rec.recommendedBlendMode = "Additive";
        rec.recommendedOpacity = 0.85f;
        rec.transitionType = "Fade";
        rec.transitionDurationMS = 400;
        rec.reasoning = "Additive blend preserves base background colors while allowing top sparkles/strobes to shine through at full luminance.";
    } else if (topLower.find("meteors") != std::string::npos || topLower.find("fireworks") != std::string::npos) {
        rec.recommendedBlendMode = "Additive";
        rec.recommendedOpacity = 1.0f;
        rec.transitionType = "Crossfade";
        rec.transitionDurationMS = 500;
        rec.reasoning = "Additive blend highlights meteor trails and fireworks bursts over background textures without dimming background elements.";
    } else if (topLower.find("morph") != std::string::npos || topLower.find("wipe") != std::string::npos) {
        rec.recommendedBlendMode = "Layered";
        rec.recommendedOpacity = 0.9f;
        rec.transitionType = "Wipe";
        rec.transitionDurationMS = 750;
        rec.reasoning = "Layered blend creates crisp geometric transitions with directional wipes.";
    } else if (topLower.find("text") != std::string::npos || topLower.find("picture") != std::string::npos) {
        rec.recommendedBlendMode = "Mask";
        rec.recommendedOpacity = 1.0f;
        rec.transitionType = "Fade";
        rec.transitionDurationMS = 300;
        rec.reasoning = "Mask mode uses text/picture silhouettes as aperture masks over background pattern effects.";
    } else {
        rec.recommendedBlendMode = "Normal";
        rec.recommendedOpacity = 0.8f;
        rec.transitionType = "Crossfade";
        rec.transitionDurationMS = 500;
        rec.reasoning = "Standard alpha composite crossfade ensures balanced visual harmony.";
    }

    return rec;
}

static void HexToHSV(const std::string& hexStr, float& outH, float& outS, float& outV) {
    std::string hex = hexStr;
    if (!hex.empty() && hex[0] == '#') hex.erase(0, 1);
    if (hex.length() < 6) { outH = 0; outS = 0; outV = 0; return; }

    unsigned int rInt = 0, gInt = 0, bInt = 0;
    std::stringstream ss;
    ss << std::hex << hex.substr(0, 2); ss >> rInt; ss.clear();
    ss << std::hex << hex.substr(2, 2); ss >> gInt; ss.clear();
    ss << std::hex << hex.substr(4, 2); ss >> bInt;

    float r = rInt / 255.0f;
    float g = gInt / 255.0f;
    float b = bInt / 255.0f;

    float maxC = std::max({r, g, b});
    float minC = std::min({r, g, b});
    float delta = maxC - minC;

    outV = maxC;
    outS = (maxC > 0.0f) ? (delta / maxC) : 0.0f;

    if (delta < 0.00001f) {
        outH = 0.0f;
    } else {
        if (maxC == r) {
            outH = 60.0f * (fmod(((g - b) / delta), 6.0f));
        } else if (maxC == g) {
            outH = 60.0f * (((b - r) / delta) + 2.0f);
        } else {
            outH = 60.0f * (((r - g) / delta) + 4.0f);
        }
        if (outH < 0.0f) outH += 360.0f;
    }
}

bool LayerBlendingTransitionAdvisor::CheckColorMuddying(
    const std::string& topColorHex,
    const std::string& bottomColorHex,
    const std::string& currentBlendMode,
    std::string& outWarning,
    std::string& outSuggestedBlendMode)
{
    if (currentBlendMode != "Normal" && currentBlendMode != "normal") {
        return false;
    }

    float h1 = 0.0f, s1 = 0.0f, v1 = 0.0f;
    float h2 = 0.0f, s2 = 0.0f, v2 = 0.0f;
    HexToHSV(topColorHex, h1, s1, v1);
    HexToHSV(bottomColorHex, h2, s2, v2);

    float hueDiff = std::abs(h1 - h2);
    if (hueDiff > 180.0f) hueDiff = 360.0f - hueDiff;

    // Complementary colors (hue distance 135 deg - 180 deg) with high saturation (>0.7) in Normal mode causes muddy colors
    if (s1 > 0.7f && s2 > 0.7f && hueDiff >= 135.0f && hueDiff <= 180.0f) {
        outWarning = "COLOR_MUDDYING";
        outSuggestedBlendMode = "Mask";
        return true;
    }

    return false;
}

LayerBlendAnalysis LayerBlendingTransitionAdvisor::AnalyzeLayerStack(const std::vector<std::string>& effectStack) {
    LayerBlendAnalysis analysis;
    analysis.stackDescription = "Multi-layer effect stack analysis (" + std::to_string(effectStack.size()) + " layers)";

    if (effectStack.size() < 2) {
        analysis.colorHarmonicBalance = 1.0f;
        return analysis;
    }

    for (size_t i = 1; i < effectStack.size(); ++i) {
        LayerBlendRecommendation rec = RecommendBlendMode(effectStack[i], effectStack[i - 1]);
        rec.topLayerIndex = static_cast<int>(i);
        rec.bottomLayerIndex = static_cast<int>(i - 1);
        analysis.recommendations.push_back(rec);
    }

    analysis.colorHarmonicBalance = 0.92f;
    return analysis;
}

std::optional<LayerBlendAnalysis> LayerBlendingTransitionAdvisor::ParseJSONToBlendAnalysis(const std::string& jsonPayload) {
    try {
        nlohmann::json j = nlohmann::json::parse(jsonPayload);
        LayerBlendAnalysis analysis;

        if (j.contains("stack_description") && j["stack_description"].is_string()) {
            analysis.stackDescription = j["stack_description"].get<std::string>();
        }

        if (j.contains("color_harmonic_balance") && j["color_harmonic_balance"].is_number()) {
            analysis.colorHarmonicBalance = j["color_harmonic_balance"].get<float>();
        }

        if (j.contains("recommendations") && j["recommendations"].is_array()) {
            for (const auto& rj : j["recommendations"]) {
                LayerBlendRecommendation rec;
                if (rj.contains("top_layer_index")) rec.topLayerIndex = rj["top_layer_index"].get<int>();
                if (rj.contains("bottom_layer_index")) rec.bottomLayerIndex = rj["bottom_layer_index"].get<int>();
                if (rj.contains("top_effect")) rec.topEffectName = rj["top_effect"].get<std::string>();
                if (rj.contains("bottom_effect")) rec.bottomEffectName = rj["bottom_effect"].get<std::string>();
                if (rj.contains("recommended_blend_mode")) rec.recommendedBlendMode = rj["recommended_blend_mode"].get<std::string>();
                if (rj.contains("recommended_opacity")) rec.recommendedOpacity = rj["recommended_opacity"].get<float>();
                if (rj.contains("transition_type")) rec.transitionType = rj["transition_type"].get<std::string>();
                if (rj.contains("transition_duration_ms")) rec.transitionDurationMS = rj["transition_duration_ms"].get<int>();
                if (rj.contains("reasoning")) rec.reasoning = rj["reasoning"].get<std::string>();
                analysis.recommendations.push_back(rec);
            }
        }

        return analysis;
    } catch (const std::exception& e) {
        spdlog::error("[LayerBlendingTransitionAdvisor] Failed to parse blend analysis JSON: {}", e.what());
        return std::nullopt;
    }
}

} // namespace xLights::AI
