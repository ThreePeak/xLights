/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/models/PixelAutoHealingAI.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running PixelAutoHealingAI verification..." << std::endl;

    // Test 1: Fault Detection & Neighbor Weight Computation
    {
        auto result = PixelAutoHealingAI::DiagnoseAndComputeHealing("MegaTree", 800, {42, 100});
        assert(result.deadPixelsDetected == 2);
        assert(result.diagnosedFaults.size() == 2);
        assert(result.diagnosedFaults[0].neighborNodeIndices.size() == 2);
        assert(result.diagnosedFaults[0].neighborWeightFactors.size() == 2);
        std::cout << " -> Test 1 (Fault Detection & Neighbor Weight Computation): PASSED" << std::endl;
    }

    // Test 2: Frame Buffer Real-Time Laplacian Remapping
    {
        auto result = PixelAutoHealingAI::DiagnoseAndComputeHealing("MegaTree", 10, {5});
        std::vector<uint8_t> frame(10 * 3, 100); // 10 pixels with R,G,B=100

        PixelAutoHealingAI::ApplyHealingToFrame(result, frame);

        // Node 4 and Node 6 (neighbors) should have boosted brightness (100 * 1.25 = 125)
        assert(frame[4 * 3] == 125);
        assert(frame[6 * 3] == 125);
        std::cout << " -> Test 2 (Frame Buffer Real-Time Laplacian Remapping): PASSED (Boosted to 125)" << std::endl;
    }

    // Test 3: Formatted Report & JSON Serialization
    {
        auto result = PixelAutoHealingAI::DiagnoseAndComputeHealing("MegaTree", 800, {42});
        std::string report = result.GenerateFormattedReport();
        assert(report.find("DEAD PIXEL AUTO-HEALING") != std::string::npos);

        auto j = result.ToJson();
        assert(j.contains("diagnosedFaults"));
        assert(j.contains("showQualityRecoveryScore"));
        std::cout << " -> Test 3 (Formatted Report & JSON Serialization): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] PixelAutoHealingAI ALL TESTS PASSED!" << std::endl;
    return 0;
}
