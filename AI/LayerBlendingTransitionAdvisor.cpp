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
