#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "../src-core/ai/AISubsystemBase.h"
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <memory>
#include <nlohmann/json.hpp>

namespace xLights::AI {

struct LayerBlendRecommendation {
    int topLayerIndex = 1;
    int bottomLayerIndex = 0;
    std::string topEffectName;
    std::string bottomEffectName;
    std::string recommendedBlendMode = "Normal"; // "Normal", "Additive", "Subtractive", "Layered", "Alpha", "Mask", "Unmask", "True 3D Mask"
    float recommendedOpacity = 1.0f;            // 0.0f to 1.0f
    std::string transitionType = "Fade";         // "Fade", "Crossfade", "Wipe", "Morph", "None"
    int transitionDurationMS = 500;             // Transition duration in ms
    std::string reasoning;                      // AI explanation of blend dynamics
};

struct LayerBlendAnalysis {
    std::string stackDescription;
    std::vector<LayerBlendRecommendation> recommendations;
    float colorHarmonicBalance = 1.0f;           // Harmonic contrast score 0.0f - 1.0f

    [[nodiscard]] nlohmann::json ToJson() const;
};

/**
 * @brief Intelligent Layer Blending & Transition Advisor.
 * Analyzes multi-layer effect stacks and evaluates visual color contrast, spatial occlusion,
 * and blend modes ("Additive", "Subtractive", "Layered", "Mask") to recommend optimal transitions.
 */
class LayerBlendingTransitionAdvisor : public AISubsystemBase {
public:
    LayerBlendingTransitionAdvisor(ServiceManager* sm = nullptr);
    virtual ~LayerBlendingTransitionAdvisor() override = default;

    // AISubsystemBase lifecycle implementation
    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override;
    virtual void Shutdown() override;
    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "LayerBlendingTransitionAdvisor"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"layer_blend_advisor", "transition_recommendation", "color_harmonic_analysis"};
    }

    /**
     * @brief Evaluates two adjacent effects and recommends the optimal blend mode and transition curve.
     */
    [[nodiscard]] static LayerBlendRecommendation RecommendBlendMode(
        const std::string& topEffect,
        const std::string& bottomEffect,
        const std::vector<std::string>& topColors = {},
        const std::vector<std::string>& bottomColors = {}
    );

    /**
     * @brief Analyzes a full multi-layer effect stack and returns layer blend recommendations.
     */
    [[nodiscard]] static LayerBlendAnalysis AnalyzeLayerStack(const std::vector<std::string>& effectStack);

    /**
     * @brief Parses structured LLM JSON responses into a LayerBlendAnalysis.
     */
    [[nodiscard]] static std::optional<LayerBlendAnalysis> ParseJSONToBlendAnalysis(const std::string& jsonPayload);
};

} // namespace xLights::AI
