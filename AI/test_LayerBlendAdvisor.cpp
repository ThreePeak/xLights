/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LayerBlendAdvisor.h"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "[Unit Test] Running LayerBlendAdvisor detection rules verification..." << std::endl;

    // Test 1: Color Muddying Detection
    std::string warn, suggest;
    bool isMuddy = xLights::AI::LayerBlendAdvisor::CheckColorMuddying("#FF0000", "#00FFFF", "Normal", warn, suggest);
    assert(isMuddy);
    assert(warn == "COLOR_MUDDYING");
    assert(suggest == "Mask");
    std::cout << " -> Test 1 (Color Muddying Detection): PASSED" << std::endl;

    // Test 2: Spatial Occlusion Detection
    xLights::AI::LayerBlendIssue occIssue;
    bool isOccluded = xLights::AI::LayerBlendAdvisor::CheckSpatialOcclusion("ColorWash", "Fire", "Normal", 1.0f, occIssue);
    assert(isOccluded);
    assert(occIssue.issueType == "SPATIAL_OCCLUSION");
    assert(occIssue.suggestedBlendMode == "Additive");
    std::cout << " -> Test 2 (Spatial Occlusion Detection): PASSED" << std::endl;

    // Test 3: Strobe Overpowering Detection
    xLights::AI::LayerBlendIssue strobeIssue;
    bool isStrobeHarsh = xLights::AI::LayerBlendAdvisor::CheckStrobeOverpowering("Strobe", 25, "Normal", strobeIssue);
    assert(isStrobeHarsh);
    assert(strobeIssue.issueType == "STROBE_OVERPOWERING");
    assert(strobeIssue.suggestedBlendMode == "Additive");
    std::cout << " -> Test 3 (Strobe Overpowering Detection): PASSED" << std::endl;

    // Test 4: Combined Multi-Issue Detection
    auto issues = xLights::AI::LayerBlendAdvisor::DetectLayerBlendIssues(
        "ColorWash", "Fire", "Normal", 1.0f, "#FF0000", "#00FFFF", 0
    );
    assert(issues.size() >= 2);
    std::cout << " -> Test 4 (Combined Multi-Issue Detection): PASSED" << std::endl;

    std::cout << "[Unit Test] ALL DETECTOR TESTS PASSED!" << std::endl;
    return 0;
}
