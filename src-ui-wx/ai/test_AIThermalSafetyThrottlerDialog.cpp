/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIThermalSafetyThrottlerDialog.h"
#include <iostream>
#include <cassert>

using namespace xLights;

int main() {
    std::cout << "[Unit Test] Running AIThermalSafetyThrottlerDialog verification..." << std::endl;

    // Headless test of dialog data backend
    ThermalSimulationParameters params;
    params.mode = ThrottlerOperationMode::MODE_3_AUTO_MICRO_DIMMING;
    params.enableAutoMicroDimmingOverride = true;

    auto result = ThermalSafetyThrottlerAI::RunSimulation(150, 50, {"MegaTree", "Arch 3"}, params);
    assert(result.totalIncidentsDetected >= 1);
    assert(result.totalCurvesAutoApplied >= 1);
    assert(!result.GenerateFormattedReportText().empty());

    std::cout << " -> Dialog Model Backend & Multi-Mode Pipeline: PASSED" << std::endl;
    std::cout << "[Unit Test] AIThermalSafetyThrottlerDialog ALL TESTS PASSED!" << std::endl;
    return 0;
}
