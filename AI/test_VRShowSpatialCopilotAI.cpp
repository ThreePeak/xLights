/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/layout/VRShowSpatialCopilotAI.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running VRShowSpatialCopilotAI verification..." << std::endl;

    // Test 1: Spatial Clearance Audit
    {
        auto session = VRShowSpatialCopilotAI::RunSpatialClearanceAudit();
        assert(!session.detectedObstacles.empty());
        assert(!session.clearanceReports.empty());
        assert(session.clearanceReports[0].minimumClearanceFeet > 0.0f);
        std::cout << " -> Test 1 (Spatial Clearance Audit): PASSED (Clearance: "
                  << session.clearanceReports[0].minimumClearanceFeet << " ft)" << std::endl;
    }

    // Test 2: Conversational Spatial Query Parsing
    {
        auto session = VRShowSpatialCopilotAI::RunSpatialClearanceAudit();
        std::string reply = VRShowSpatialCopilotAI::ProcessConversationalSpatialQuery("Is the MegaTree clearing the oak tree?", session);
        assert(!reply.empty());
        assert(reply.find("MegaTree") != std::string::npos);
        assert(reply.find("2.8 feet") != std::string::npos);
        std::cout << " -> Test 2 (Conversational Spatial Query Parsing): PASSED" << std::endl;
    }

    // Test 3: Formatted Report & JSON Serialization
    {
        auto session = VRShowSpatialCopilotAI::RunSpatialClearanceAudit();
        std::string report = session.GenerateFormattedReport();
        assert(report.find("VR/AR SPATIAL CLEARANCE") != std::string::npos);

        auto j = session.ToJson();
        assert(j.contains("detectedObstacles"));
        assert(j.contains("clearanceReports"));
        std::cout << " -> Test 3 (Formatted Report & JSON Serialization): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] VRShowSpatialCopilotAI ALL TESTS PASSED!" << std::endl;
    return 0;
}
