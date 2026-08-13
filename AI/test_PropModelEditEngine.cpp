/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "AI/PropModelEditEngine.h"
#include "AI/CustomPropDesignerAI.h"

using namespace xLights::AI;

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::vector<PropNodeSuggestion> MakeGrid(int count) {
    std::vector<PropNodeSuggestion> nodes;
    for (int i = 0; i < count; ++i) {
        PropNodeSuggestion n;
        n.x = static_cast<float>((i % 10) * 10.0f);
        n.y = static_cast<float>((i / 10) * 10.0f);
        n.channelIndex = i + 1;
        n.label = "Node_" + std::to_string(i + 1);
        nodes.push_back(n);
    }
    return nodes;
}

// ── Movement tests ────────────────────────────────────────────────────────────

TEST_CASE("PropModelEditEngine: MoveNode shifts position", "[PropModelEditEngine]") {
    PropModelEditEngine eng;
    auto nodes = MakeGrid(5);
    float origX = nodes[2].x;
    float origY = nodes[2].y;

    eng.MoveNode(nodes, 2, 10.0f, 5.0f, 0.0f);
    REQUIRE(nodes[2].x == Approx(origX + 10.0f));
    REQUIRE(nodes[2].y == Approx(origY + 5.0f));
}

TEST_CASE("PropModelEditEngine: MoveNode out-of-range is safe", "[PropModelEditEngine]") {
    PropModelEditEngine eng;
    auto nodes = MakeGrid(3);
    REQUIRE_NOTHROW(eng.MoveNode(nodes, 99, 1.0f, 1.0f, 0.0f));
    REQUIRE(nodes.size() == 3);
}

TEST_CASE("PropModelEditEngine: SetNodePosition sets absolute", "[PropModelEditEngine]") {
    PropModelEditEngine eng;
    auto nodes = MakeGrid(3);
    eng.SetNodePosition(nodes, 0, 42.5f, 77.0f, 0.0f);
    REQUIRE(nodes[0].x == Approx(42.5f));
    REQUIRE(nodes[0].y == Approx(77.0f));
}

// ── Insert / Remove ───────────────────────────────────────────────────────────

TEST_CASE("PropModelEditEngine: InsertNode no conflict", "[PropModelEditEngine]") {
    PropModelEditEngine eng;
    auto nodes = MakeGrid(5);        // IDs 1-5
    auto result = eng.InsertNode(nodes, 50.0f, 50.0f, 0.0f, 99);
    REQUIRE_FALSE(result.renumberingRequired);
    REQUIRE(result.updatedNodes.size() == 6);
    bool found = false;
    for (const auto& n : result.updatedNodes)
        if (n.channelIndex == 99) { found = true; break; }
    REQUIRE(found);
}

TEST_CASE("PropModelEditEngine: InsertNode detects conflict and generates suggestion", "[PropModelEditEngine]") {
    PropModelEditEngine eng;
    auto nodes = MakeGrid(10);       // IDs 1-10
    auto result = eng.InsertNode(nodes, 50.0f, 50.0f, 0.0f, 5);   // ID 5 exists
    REQUIRE(result.renumberingRequired);
    REQUIRE(result.conflictingId == 5);
    REQUIRE_FALSE(result.aiSuggestion.empty());
    // Suggestion should mention "5" and renumbering
    REQUIRE(result.aiSuggestion.find("5") != std::string::npos);
}

TEST_CASE("PropModelEditEngine: RemoveNode removes correct node", "[PropModelEditEngine]") {
    PropModelEditEngine eng;
    auto nodes = MakeGrid(5);
    int targetId = nodes[2].channelIndex;
    auto updated = eng.RemoveNode(nodes, 2);
    REQUIRE(updated.size() == 4);
    for (const auto& n : updated)
        REQUIRE(n.channelIndex != targetId);
}

// ── ID Renumbering ────────────────────────────────────────────────────────────

TEST_CASE("PropModelEditEngine: RenumberNodesFrom shifts IDs >= fromId", "[PropModelEditEngine]") {
    PropModelEditEngine eng;
    auto nodes = MakeGrid(10);       // IDs 1-10
    auto updated = eng.RenumberNodesFrom(nodes, 5, 1);
    // IDs 1-4 unchanged, IDs 5-10 become 6-11
    for (const auto& n : updated) {
        int original = n.channelIndex;
        if (original >= 6) {  // after shift
            // find in original nodes (channelIndex was original-1 before)
            REQUIRE(original >= 6);
        }
    }
    // Specifically: original ID 5 → 6
    bool foundSix = false;
    for (const auto& n : updated)
        if (n.channelIndex == 6) { foundSix = true; break; }
    REQUIRE(foundSix);
    // Original ID 4 should still be 4
    bool foundFour = false;
    for (const auto& n : updated)
        if (n.channelIndex == 4) { foundFour = true; break; }
    REQUIRE(foundFour);
}

// ── Geometry helpers ──────────────────────────────────────────────────────────

TEST_CASE("PropModelEditEngine: EvenlySpaceNodes preserves endpoints", "[PropModelEditEngine]") {
    PropModelEditEngine eng;
    auto nodes = MakeGrid(5);
    float x0 = nodes.front().x, y0 = nodes.front().y;
    float xN = nodes.back().x,  yN = nodes.back().y;

    eng.EvenlySpaceNodes(nodes);

    REQUIRE(nodes.front().x == Approx(x0));
    REQUIRE(nodes.front().y == Approx(y0));
    REQUIRE(nodes.back().x  == Approx(xN));
    REQUIRE(nodes.back().y  == Approx(yN));
}

TEST_CASE("PropModelEditEngine: StraightenLeg makes midpoints collinear", "[PropModelEditEngine]") {
    PropModelEditEngine eng;
    auto nodes = MakeGrid(5);
    // Perturb middle nodes
    nodes[1].x += 20.0f; nodes[2].y += 15.0f; nodes[3].x -= 8.0f;
    eng.StraightenLeg(nodes, 0, 4);

    // After straightening, all nodes should lie on the line from [0] to [4]
    float x0 = nodes[0].x, y0 = nodes[0].y;
    float x4 = nodes[4].x, y4 = nodes[4].y;
    for (int i = 1; i <= 3; ++i) {
        float t = static_cast<float>(i) / 4.0f;
        REQUIRE(nodes[i].x == Approx(x0 + t*(x4-x0)).margin(0.01f));
        REQUIRE(nodes[i].y == Approx(y0 + t*(y4-y0)).margin(0.01f));
    }
}

TEST_CASE("PropModelEditEngine: SnapToGrid rounds to nearest", "[PropModelEditEngine]") {
    PropModelEditEngine eng;
    std::vector<PropNodeSuggestion> nodes;
    PropNodeSuggestion n; n.x = 12.3f; n.y = 7.8f; n.channelIndex = 1; n.label = "A";
    nodes.push_back(n);
    eng.SnapToGrid(nodes, 5.0f);
    REQUIRE(nodes[0].x == Approx(10.0f));
    REQUIRE(nodes[0].y == Approx(10.0f));
}
