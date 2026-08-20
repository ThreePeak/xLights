/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/layout/AudienceViewingOptimizerAI.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running AudienceViewingOptimizerAI verification..." << std::endl;

    // Test 1: ProjectModelToScreen Perspective Math
    {
        LayoutModelDescriptor model{"MegaTree", 0.0f, 0.0f, 25.0f, 12.0f, 20.0f, 0.0f};
        SpectatorVantageImage vantage{"car.jpg", "Car Driver", 35.0f, 3.8f, 0.0f, 65.0f};
        float sx, sy, sw, sh;
        AudienceViewingOptimizerAI::ProjectModelToScreen(model, vantage, sx, sy, sw, sh);

        assert(sx > 0.0f && sx < 1000.0f);
        assert(sw > 0.0f);
        assert(sh > 0.0f);
        std::cout << " -> Test 1 (ProjectModelToScreen Perspective Math): PASSED (Screen Box: "
                  << sx << ", " << sy << " [" << sw << "x" << sh << "])" << std::endl;
    }

    // Test 2: CalculateOcclusion Front vs Back
    {
        LayoutModelDescriptor front{"Arch 2", 0.0f, 0.0f, 10.0f, 6.0f, 4.0f, 0.0f};
        LayoutModelDescriptor back{"MegaTree", 0.0f, 0.0f, 25.0f, 12.0f, 20.0f, 0.0f};
        SpectatorVantageImage vantage{"car.jpg", "Car Driver", 35.0f, 3.8f, 0.0f, 65.0f};

        float occ = AudienceViewingOptimizerAI::CalculateOcclusion(front, back, vantage);
        assert(occ > 0.0f);
        std::cout << " -> Test 2 (CalculateOcclusion Front vs Back): PASSED (Occlusion: " << occ << "%)" << std::endl;
    }

    // Test 3: EvaluateSightlines Pipeline
    {
        auto result = AudienceViewingOptimizerAI::EvaluateSightlines({}, {});
        assert(result.overallShowVisibilityIndex > 0.0f);
        assert(!result.propRatings.empty());
        assert(!result.vantagePoints.empty());
        std::cout << " -> Test 3 (EvaluateSightlines Pipeline): PASSED (Overall Visibility: "
                  << result.overallShowVisibilityIndex << "%)" << std::endl;
    }

    // Test 4: Formatted Report & JSON Serialization
    {
        auto result = AudienceViewingOptimizerAI::EvaluateSightlines({}, {});
        std::string report = result.GenerateFormattedReportText();
        assert(report.find("AI AUDIENCE SIGHTLINE") != std::string::npos);
        assert(report.find("Calibrated Spectator Vantage Points") != std::string::npos);

        auto j = result.ToJson();
        assert(j.contains("overallShowVisibilityIndex"));
        assert(j.contains("vantagePoints"));
        assert(j.contains("propRatings"));
        std::cout << " -> Test 4 (Formatted Report & JSON Serialization): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] AudienceViewingOptimizerAI ALL TESTS PASSED!" << std::endl;
    return 0;
}
