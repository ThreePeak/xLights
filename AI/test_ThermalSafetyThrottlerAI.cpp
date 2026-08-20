/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/controllers/ThermalSafetyThrottlerAI.h"
#include <iostream>
#include <cassert>

using namespace xLights;

int main() {
    std::cout << "[Unit Test] Running ThermalSafetyThrottlerAI verification..." << std::endl;

    // Test 1: Timecode Formatting
    {
        std::string tc = ThermalSafetyThrottlerAI::FormatTimecode(83450);
        assert(tc == "00:01:23.450");
        std::cout << " -> Test 1 (FormatTimecode HH:MM:SS.mmm): PASSED" << std::endl;
    }

    // Test 2: Mode 1 (Incident Audit)
    {
        ThermalSimulationParameters p;
        p.mode = ThrottlerOperationMode::MODE_1_INCIDENT_AUDIT;
        p.enableAutoMicroDimmingOverride = false;

        auto res = ThermalSafetyThrottlerAI::RunSimulation(100, 50, {}, p);
        assert(res.totalIncidentsDetected >= 1);
        assert(res.appliedMicroDimmings.empty()); // None auto-applied in Mode 1
        assert(!res.incidents[0].timeCodeRange.empty());
        assert(!res.incidents[0].propName.empty());
        std::cout << " -> Test 2 (Mode 1 Incident Audit & Timecodes): PASSED" << std::endl;
    }

    // Test 3: Mode 2 (Remediation Advisor with Named Props)
    {
        ThermalSimulationParameters p;
        p.mode = ThrottlerOperationMode::MODE_2_REMEDIATION_ADVISOR;
        p.enableAutoMicroDimmingOverride = false;

        auto res = ThermalSafetyThrottlerAI::RunSimulation(100, 50, {"MegaTree", "Arch 3"}, p);
        assert(!res.incidents.empty());
        assert(!res.incidents[0].suggestedFix.empty());
        assert(res.incidents[0].suggestedDimmingCurveData.find("Type=Custom") != std::string::npos);
        std::cout << " -> Test 3 (Mode 2 Actionable Remediation Suggestions): PASSED" << std::endl;
    }

    // Test 4: Mode 3 (Auto Micro-Dimming Intentional Override)
    {
        ThermalSimulationParameters p;
        p.mode = ThrottlerOperationMode::MODE_3_AUTO_MICRO_DIMMING;
        p.enableAutoMicroDimmingOverride = true; // Explicit intentional toggle

        auto res = ThermalSafetyThrottlerAI::RunSimulation(100, 50, {}, p);
        assert(res.totalCurvesAutoApplied >= 1);
        assert(!res.appliedMicroDimmings.empty());
        assert(res.appliedMicroDimmings[0].reductionPercent > 0.0f);
        assert(res.safetyCompliancePassed);

        // Edit applied curve
        ThermalSafetyThrottlerAI::UpdateAppliedCurve(res, 1, "Type=Custom;CustomData=0:1.0|0.5:0.5|1.0:1.0");
        assert(res.appliedMicroDimmings[0].isManuallyOverridden);

        // Revert applied curve
        ThermalSafetyThrottlerAI::RevertAppliedCurve(res, 1);
        assert(res.appliedMicroDimmings.empty() || res.appliedMicroDimmings[0].recordId != 1);
        std::cout << " -> Test 4 (Mode 3 Auto Micro-Dimming & Audit Revert): PASSED" << std::endl;
    }

    // Test 5: Report Formatting & JSON
    {
        ThermalSimulationParameters p;
        p.mode = ThrottlerOperationMode::MODE_3_AUTO_MICRO_DIMMING;
        p.enableAutoMicroDimmingOverride = true;
        auto res = ThermalSafetyThrottlerAI::RunSimulation(100, 50, {}, p);

        std::string report = res.GenerateFormattedReportText();
        assert(report.find("xLights AI Thermal & Current Load Safety Throttler Report") != std::string::npos);
        assert(report.find("Mode 3") != std::string::npos);

        auto json = res.ToJson();
        assert(json.contains("incidents"));
        assert(json.contains("applied_dimmings"));
        std::cout << " -> Test 5 (Formatted Report & JSON Serialization): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] ThermalSafetyThrottlerAI ALL TESTS PASSED!" << std::endl;
    return 0;
}
