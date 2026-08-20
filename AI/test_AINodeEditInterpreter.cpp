/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AINodeEditInterpreter.h"
#include "AIConfigurationManager.h"
#include "CustomPropDesignerAI.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

using namespace xLights::AI;

static std::vector<PropNodeSuggestion> MakeCircleNodes(int count) {
    std::vector<PropNodeSuggestion> nodes;
    const float pi = 3.14159265f;
    for (int i = 0; i < count; ++i) {
        PropNodeSuggestion n;
        n.x = 50.0f + 40.0f * std::cos(2.0f * pi * i / count);
        n.y = 50.0f + 40.0f * std::sin(2.0f * pi * i / count);
        n.channelIndex = i * 3 + 1;
        n.label = "Node_" + std::to_string(i + 1);
        nodes.push_back(n);
    }
    return nodes;
}

static AIConfigSettings MakeTestConfig() {
    AIConfigSettings cfg;
    cfg.primaryModel = 0;
    cfg.temperature  = 0.7f;
    cfg.maxTokens    = 1024;
    return cfg;
}

int main() {
    std::cout << "[Unit Test] Running AINodeEditInterpreter verification..." << std::endl;

    // Test 1: InterpretPrompt confirmation message
    {
        AINodeEditInterpreter interp;
        auto nodes = MakeCircleNodes(16);
        auto cfg   = MakeTestConfig();

        auto plan = interp.InterpretPrompt("space all nodes evenly", nodes, cfg);
        assert(!plan.confirmationMessage.empty());
        std::cout << " -> Test 1 (Confirmation Message): PASSED" << std::endl;
    }

    // Test 2: RequiresUserApproval flag
    {
        AINodeEditInterpreter interp;
        auto nodes = MakeCircleNodes(8);
        auto cfg   = MakeTestConfig();

        auto plan = interp.InterpretPrompt("move everything left", nodes, cfg);
        assert(plan.requiresUserApproval == true);
        std::cout << " -> Test 2 (RequiresUserApproval Enforced): PASSED" << std::endl;
    }

    // Test 3: Space prompt operation code
    {
        AINodeEditInterpreter interp;
        auto nodes = MakeCircleNodes(20);
        auto cfg   = MakeTestConfig();

        auto plan = interp.InterpretPrompt("space all nodes evenly across the model", nodes, cfg);
        assert(plan.operationCode.find("EVEN_SPACE_ALL") != std::string::npos);
        std::cout << " -> Test 3 (EVEN_SPACE_ALL Opcode): PASSED" << std::endl;
    }

    // Test 4: Straighten leg operation code
    {
        AINodeEditInterpreter interp;
        auto nodes = MakeCircleNodes(12);
        auto cfg   = MakeTestConfig();

        auto plan = interp.InterpretPrompt("straighten each vertical leg", nodes, cfg);
        assert(plan.operationCode.find("STRAIGHTEN_LEG") != std::string::npos);
        std::cout << " -> Test 4 (STRAIGHTEN_LEG Opcode): PASSED" << std::endl;
    }

    // Test 5: Snap to grid operation code
    {
        AINodeEditInterpreter interp;
        auto nodes = MakeCircleNodes(10);
        auto cfg   = MakeTestConfig();

        auto plan = interp.InterpretPrompt("snap all nodes to the grid", nodes, cfg);
        assert(plan.operationCode.find("SNAP_GRID") != std::string::npos);
        std::cout << " -> Test 5 (SNAP_GRID Opcode): PASSED" << std::endl;
    }

    // Test 6: Unknown prompt fallback
    {
        AINodeEditInterpreter interp;
        auto nodes = MakeCircleNodes(5);
        auto cfg   = MakeTestConfig();

        auto plan = interp.InterpretPrompt("xyzzy quux frobnicate", nodes, cfg);
        assert(!plan.hasError);
        assert(!plan.confirmationMessage.empty());
        assert(!plan.operationCode.empty());
        std::cout << " -> Test 6 (Unknown Prompt Fallback): PASSED" << std::endl;
    }

    // Test 7: ExecutePlan EVEN_SPACE_ALL
    {
        AINodeEditInterpreter interp;
        auto nodes = MakeCircleNodes(16);
        auto cfg   = MakeTestConfig();

        auto plan    = interp.InterpretPrompt("space all nodes evenly", nodes, cfg);
        auto updated = interp.ExecutePlan(plan, nodes);
        assert(updated.size() == nodes.size());
        std::cout << " -> Test 7 (ExecutePlan EVEN_SPACE_ALL Preserves Count): PASSED" << std::endl;
    }

    // Test 8: ExecutePlan SNAP_GRID
    {
        AINodeEditInterpreter interp;
        std::vector<PropNodeSuggestion> nodes;
        PropNodeSuggestion n; n.x = 13.3f; n.y = 6.7f; n.channelIndex = 1; n.label = "N";
        nodes.push_back(n);

        NodeEditPlan plan;
        plan.operationCode      = "SNAP_GRID 5.0\n";
        plan.requiresUserApproval = true;

        auto updated = interp.ExecutePlan(plan, nodes);
        assert(std::abs(updated[0].x - 15.0f) < 1e-4f);
        assert(std::abs(updated[0].y - 5.0f) < 1e-4f);
        std::cout << " -> Test 8 (ExecutePlan SNAP_GRID): PASSED" << std::endl;
    }

    // Test 9: Compound prompt multi-operation
    {
        AINodeEditInterpreter interp;
        auto nodes = MakeCircleNodes(20);
        auto cfg   = MakeTestConfig();

        auto plan = interp.InterpretPrompt("straighten all legs and space all nodes evenly and snap to grid", nodes, cfg);
        assert(plan.operationCode.find("STRAIGHTEN_LEG") != std::string::npos);
        assert(plan.operationCode.find("EVEN_SPACE_ALL") != std::string::npos);
        assert(plan.operationCode.find("SNAP_GRID") != std::string::npos);
        std::cout << " -> Test 9 (Compound Prompt Multi-Op Extraction): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] AINodeEditInterpreter ALL TESTS PASSED!" << std::endl;
    return 0;
}
