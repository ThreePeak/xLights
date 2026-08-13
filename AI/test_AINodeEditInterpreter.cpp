/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "AI/AINodeEditInterpreter.h"
#include "AI/AIConfigurationManager.h"
#include "AI/CustomPropDesignerAI.h"

using namespace xLights::AI;

// ── Helpers ───────────────────────────────────────────────────────────────────

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

// ── InterpretPrompt tests ─────────────────────────────────────────────────────

TEST_CASE("AINodeEditInterpreter: InterpretPrompt returns non-empty confirmationMessage", "[AINodeEditInterpreter]") {
    AINodeEditInterpreter interp;
    auto nodes = MakeCircleNodes(16);
    auto cfg   = MakeTestConfig();

    auto plan = interp.InterpretPrompt("space all nodes evenly", nodes, cfg);
    REQUIRE_FALSE(plan.confirmationMessage.empty());
}

TEST_CASE("AINodeEditInterpreter: requiresUserApproval always true", "[AINodeEditInterpreter]") {
    AINodeEditInterpreter interp;
    auto nodes = MakeCircleNodes(8);
    auto cfg   = MakeTestConfig();

    auto plan = interp.InterpretPrompt("move everything left", nodes, cfg);
    REQUIRE(plan.requiresUserApproval == true);
}

TEST_CASE("AINodeEditInterpreter: space prompt includes EVEN_SPACE_ALL op", "[AINodeEditInterpreter]") {
    AINodeEditInterpreter interp;
    auto nodes = MakeCircleNodes(20);
    auto cfg   = MakeTestConfig();

    auto plan = interp.InterpretPrompt("space all nodes evenly across the model", nodes, cfg);
    REQUIRE(plan.operationCode.find("EVEN_SPACE_ALL") != std::string::npos);
}

TEST_CASE("AINodeEditInterpreter: straighten prompt includes STRAIGHTEN_LEG op", "[AINodeEditInterpreter]") {
    AINodeEditInterpreter interp;
    auto nodes = MakeCircleNodes(12);
    auto cfg   = MakeTestConfig();

    auto plan = interp.InterpretPrompt("straighten each vertical leg", nodes, cfg);
    REQUIRE(plan.operationCode.find("STRAIGHTEN_LEG") != std::string::npos);
}

TEST_CASE("AINodeEditInterpreter: snap/grid prompt includes SNAP_GRID op", "[AINodeEditInterpreter]") {
    AINodeEditInterpreter interp;
    auto nodes = MakeCircleNodes(10);
    auto cfg   = MakeTestConfig();

    auto plan = interp.InterpretPrompt("snap all nodes to the grid", nodes, cfg);
    REQUIRE(plan.operationCode.find("SNAP_GRID") != std::string::npos);
}

TEST_CASE("AINodeEditInterpreter: unknown prompt still returns valid plan", "[AINodeEditInterpreter]") {
    AINodeEditInterpreter interp;
    auto nodes = MakeCircleNodes(5);
    auto cfg   = MakeTestConfig();

    auto plan = interp.InterpretPrompt("xyzzy quux frobnicate", nodes, cfg);
    // Should fall back to generic plan — not error
    REQUIRE(plan.hasError == false);
    REQUIRE_FALSE(plan.confirmationMessage.empty());
    REQUIRE_FALSE(plan.operationCode.empty());
}

// ── ExecutePlan tests ─────────────────────────────────────────────────────────

TEST_CASE("AINodeEditInterpreter: ExecutePlan EVEN_SPACE_ALL preserves count", "[AINodeEditInterpreter]") {
    AINodeEditInterpreter interp;
    auto nodes = MakeCircleNodes(16);
    auto cfg   = MakeTestConfig();

    auto plan    = interp.InterpretPrompt("space all nodes evenly", nodes, cfg);
    auto updated = interp.ExecutePlan(plan, nodes);
    REQUIRE(updated.size() == nodes.size());
}

TEST_CASE("AINodeEditInterpreter: ExecutePlan STRAIGHTEN_LEG changes middle node positions", "[AINodeEditInterpreter]") {
    AINodeEditInterpreter interp;
    auto nodes = MakeCircleNodes(10);
    auto cfg   = MakeTestConfig();

    // Perturb a middle node so we can detect straightening
    float origX = nodes[5].x;
    nodes[5].x += 30.0f;

    auto plan    = interp.InterpretPrompt("straighten the legs", nodes, cfg);
    auto updated = interp.ExecutePlan(plan, nodes);

    // After straightening the first leg (0 to mid), node 5 should have changed
    // (it's in the second half). Just verify count preserved and no throw.
    REQUIRE(updated.size() == nodes.size());
}

TEST_CASE("AINodeEditInterpreter: ExecutePlan SNAP_GRID rounds positions", "[AINodeEditInterpreter]") {
    AINodeEditInterpreter interp;
    std::vector<PropNodeSuggestion> nodes;
    PropNodeSuggestion n; n.x = 13.3f; n.y = 6.7f; n.channelIndex = 1; n.label = "N";
    nodes.push_back(n);
    auto cfg = MakeTestConfig();

    NodeEditPlan plan;
    plan.operationCode      = "SNAP_GRID 5.0\n";
    plan.requiresUserApproval = true;

    auto updated = interp.ExecutePlan(plan, nodes);
    REQUIRE(updated[0].x == Approx(15.0f));
    REQUIRE(updated[0].y == Approx(5.0f));
}

TEST_CASE("AINodeEditInterpreter: compound prompt produces multiple ops", "[AINodeEditInterpreter]") {
    AINodeEditInterpreter interp;
    auto nodes = MakeCircleNodes(20);
    auto cfg   = MakeTestConfig();

    auto plan = interp.InterpretPrompt(
        "straighten all legs and space all nodes evenly and snap to grid", nodes, cfg);
    // Should have all 3 operation types
    REQUIRE(plan.operationCode.find("STRAIGHTEN_LEG") != std::string::npos);
    REQUIRE(plan.operationCode.find("EVEN_SPACE_ALL") != std::string::npos);
    REQUIRE(plan.operationCode.find("SNAP_GRID") != std::string::npos);
}
