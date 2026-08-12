/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LayerBlendingTransitionAdvisor.h"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "[Unit Test] Running LayerBlendingTransitionAdvisor verification..." << std::endl;

    // Test 1: Single Blend Recommendation
    auto rec1 = xLights::AI::LayerBlendingTransitionAdvisor::RecommendBlendMode("Twinkle", "ColorWash");
    assert(rec1.recommendedBlendMode == "Additive");
    assert(rec1.transitionType == "Fade");
    std::cout << " -> Test 1 (Single Blend Recommendation): PASSED" << std::endl;

    // Test 2: Multi-Layer Stack Analysis
    std::vector<std::string> stack = {"ColorWash", "Meteors", "Strobe"};
    auto analysis = xLights::AI::LayerBlendingTransitionAdvisor::AnalyzeLayerStack(stack);
    assert(analysis.recommendations.size() == 2);
    assert(analysis.recommendations[0].topEffectName == "Meteors");
    assert(analysis.recommendations[0].recommendedBlendMode == "Additive");
    assert(analysis.recommendations[1].topEffectName == "Strobe");
    assert(analysis.recommendations[1].recommendedBlendMode == "Additive");
    std::cout << " -> Test 2 (Multi-Layer Stack Analysis): PASSED" << std::endl;

    // Test 3: JSON Parsing
    std::string jsonPayload = R"({
        "stack_description": "Fire over ocean wave stack",
        "color_harmonic_balance": 0.95,
        "recommendations": [
            {
                "top_layer_index": 1,
                "bottom_layer_index": 0,
                "top_effect": "Twinkle",
                "bottom_effect": "Fire",
                "recommended_blend_mode": "Additive",
                "recommended_opacity": 0.85,
                "transition_type": "Fade",
                "transition_duration_ms": 400,
                "reasoning": "Additive blend preserves fire glow"
            }
        ]
    })";

    auto parsedOpt = xLights::AI::LayerBlendingTransitionAdvisor::ParseJSONToBlendAnalysis(jsonPayload);
    assert(parsedOpt.has_value());
    const auto& parsed = parsedOpt.value();
    assert(parsed.recommendations.size() == 1);
    assert(parsed.recommendations[0].recommendedBlendMode == "Additive");
    assert(parsed.recommendations[0].topEffectName == "Twinkle");
    std::cout << " -> Test 3 (JSON Parsing): PASSED" << std::endl;

    // Test 4: CheckColorMuddying Validation
    std::string warn, suggest;
    bool isMuddy = xLights::AI::LayerBlendingTransitionAdvisor::CheckColorMuddying("#FF0000", "#00FFFF", "Normal", warn, suggest);
    assert(isMuddy);
    assert(warn == "COLOR_MUDDYING");
    assert(suggest == "Mask");
    std::cout << " -> Test 4 (CheckColorMuddying Validation): PASSED" << std::endl;

    std::cout << "[Unit Test] ALL TESTS PASSED!" << std::endl;
    return 0;
}
