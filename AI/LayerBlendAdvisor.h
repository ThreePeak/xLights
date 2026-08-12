#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LayerBlendingTransitionAdvisor.h"
#include <string>
#include <vector>
#include <optional>

namespace xLights::AI {

struct LayerBlendIssue {
    std::string issueType;            // "COLOR_MUDDYING", "SPATIAL_OCCLUSION", "STROBE_OVERPOWERING"
    std::string severity;             // "WARNING", "SUGGESTION", "INFO"
    std::string description;
    std::string suggestedBlendMode;   // "Mask", "True 3D Mask", "Additive", "Layered", "Alpha"
    float suggestedOpacity = 1.0f;
};

struct LayerBlendSpec {
    int layerIndex = 0;
    std::string effectName;
    std::string currentBlendMode = "Normal";
    float currentOpacity = 1.0f;
    std::vector<std::string> hexColors;
    int strobeFrequency = 0;
};

/**
 * @brief LayerBlendAdvisor provides detection rules for color muddying,
 * spatial occlusion, and strobe overpowering across multi-layer effect stacks.
 */
class LayerBlendAdvisor : public LayerBlendingTransitionAdvisor {
public:
    LayerBlendAdvisor(ServiceManager* sm = nullptr) : LayerBlendingTransitionAdvisor(sm) {}
    virtual ~LayerBlendAdvisor() override = default;

    /**
     * @brief Analyzes multi-layer effect stack using structured LayerBlendSpec inputs.
     */
    static std::vector<LayerBlendRecommendation> AnalyzeLayerStack(const std::vector<LayerBlendSpec>& layers);

    /**
     * @brief Detects all blend issues (color muddying, spatial occlusion, strobe overpowering) between top and bottom layers.
     */
    static std::vector<LayerBlendIssue> DetectLayerBlendIssues(
        const std::string& topEffect,
        const std::string& bottomEffect,
        const std::string& currentBlendMode = "Normal",
        float currentOpacity = 1.0f,
        const std::string& topColorHex = "#FF0000",
        const std::string& bottomColorHex = "#00FFFF",
        int strobeFrequency = 0
    );

    /**
     * @brief Evaluates spatial occlusion risk where an opaque top effect masks a detailed bottom effect.
     */
    static bool CheckSpatialOcclusion(
        const std::string& topEffect,
        const std::string& bottomEffect,
        const std::string& currentBlendMode,
        float currentOpacity,
        LayerBlendIssue& outIssue
    );

    /**
     * @brief Evaluates strobe overpowering risk.
     */
    static bool CheckStrobeOverpowering(
        const std::string& topEffect,
        int strobeFrequency,
        const std::string& currentBlendMode,
        LayerBlendIssue& outIssue
    );
};

} // namespace xLights::AI
