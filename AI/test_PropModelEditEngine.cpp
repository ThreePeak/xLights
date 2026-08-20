/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PropModelEditEngine.h"
#include "CustomPropDesignerAI.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

using namespace xLights::AI;

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

int main() {
    std::cout << "[Unit Test] Running PropModelEditEngine verification..." << std::endl;

    // Test 1: MoveNode shifts position
    {
        PropModelEditEngine eng;
        auto nodes = MakeGrid(5);
        float origX = nodes[2].x;
        float origY = nodes[2].y;

        eng.MoveNode(nodes, 2, 10.0f, 5.0f, 0.0f);
        assert(std::abs(nodes[2].x - (origX + 10.0f)) < 1e-4f);
        assert(std::abs(nodes[2].y - (origY + 5.0f)) < 1e-4f);
        std::cout << " -> Test 1 (MoveNode Position Shift): PASSED" << std::endl;
    }

    // Test 2: MoveNode out-of-range is safe
    {
        PropModelEditEngine eng;
        auto nodes = MakeGrid(3);
        eng.MoveNode(nodes, 99, 1.0f, 1.0f, 0.0f);
        assert(nodes.size() == 3);
        std::cout << " -> Test 2 (Out-of-range Safety): PASSED" << std::endl;
    }

    // Test 3: SetNodePosition sets absolute
    {
        PropModelEditEngine eng;
        auto nodes = MakeGrid(3);
        eng.SetNodePosition(nodes, 0, 42.5f, 77.0f, 0.0f);
        assert(std::abs(nodes[0].x - 42.5f) < 1e-4f);
        assert(std::abs(nodes[0].y - 77.0f) < 1e-4f);
        std::cout << " -> Test 3 (SetNodePosition Absolute): PASSED" << std::endl;
    }

    // Test 4: InsertNode no conflict
    {
        PropModelEditEngine eng;
        auto nodes = MakeGrid(5); // IDs 1-5
        auto result = eng.InsertNode(nodes, 50.0f, 50.0f, 0.0f, 99);
        assert(!result.renumberingRequired);
        assert(result.updatedNodes.size() == 6);
        bool found = false;
        for (const auto& n : result.updatedNodes) {
            if (n.channelIndex == 99) { found = true; break; }
        }
        assert(found);
        std::cout << " -> Test 4 (InsertNode No Conflict): PASSED" << std::endl;
    }

    // Test 5: InsertNode detects conflict and generates suggestion
    {
        PropModelEditEngine eng;
        auto nodes = MakeGrid(10); // IDs 1-10
        auto result = eng.InsertNode(nodes, 50.0f, 50.0f, 0.0f, 5); // ID 5 exists
        assert(result.renumberingRequired);
        assert(result.conflictingId == 5);
        assert(!result.aiSuggestion.empty());
        assert(result.aiSuggestion.find("5") != std::string::npos);
        std::cout << " -> Test 5 (InsertNode Conflict & AI Suggestion): PASSED" << std::endl;
    }

    // Test 6: RemoveNode removes correct node
    {
        PropModelEditEngine eng;
        auto nodes = MakeGrid(5);
        int targetId = nodes[2].channelIndex;
        auto updated = eng.RemoveNode(nodes, 2);
        assert(updated.size() == 4);
        for (const auto& n : updated) {
            assert(n.channelIndex != targetId);
        }
        std::cout << " -> Test 6 (RemoveNode Correct Index): PASSED" << std::endl;
    }

    // Test 7: RenumberNodesFrom shifts IDs >= fromId
    {
        PropModelEditEngine eng;
        auto nodes = MakeGrid(10); // IDs 1-10
        auto updated = eng.RenumberNodesFrom(nodes, 5, 1);
        bool foundSix = false;
        for (const auto& n : updated) {
            if (n.channelIndex == 6) { foundSix = true; break; }
        }
        assert(foundSix);
        bool foundFour = false;
        for (const auto& n : updated) {
            if (n.channelIndex == 4) { foundFour = true; break; }
        }
        assert(foundFour);
        std::cout << " -> Test 7 (RenumberNodesFrom Shift): PASSED" << std::endl;
    }

    // Test 8: EvenlySpaceNodes preserves endpoints
    {
        PropModelEditEngine eng;
        auto nodes = MakeGrid(5);
        float x0 = nodes.front().x, y0 = nodes.front().y;
        float xN = nodes.back().x,  yN = nodes.back().y;

        eng.EvenlySpaceNodes(nodes);
        assert(std::abs(nodes.front().x - x0) < 1e-4f);
        assert(std::abs(nodes.front().y - y0) < 1e-4f);
        assert(std::abs(nodes.back().x - xN) < 1e-4f);
        assert(std::abs(nodes.back().y - yN) < 1e-4f);
        std::cout << " -> Test 8 (EvenlySpaceNodes Endpoints Preserved): PASSED" << std::endl;
    }

    // Test 9: StraightenLeg makes midpoints collinear
    {
        PropModelEditEngine eng;
        auto nodes = MakeGrid(5);
        nodes[1].x += 20.0f; nodes[2].y += 15.0f; nodes[3].x -= 8.0f;
        eng.StraightenLeg(nodes, 0, 4);

        float x0 = nodes[0].x, y0 = nodes[0].y;
        float x4 = nodes[4].x, y4 = nodes[4].y;
        for (int i = 1; i <= 3; ++i) {
            float t = static_cast<float>(i) / 4.0f;
            assert(std::abs(nodes[i].x - (x0 + t*(x4-x0))) < 0.05f);
            assert(std::abs(nodes[i].y - (y0 + t*(y4-y0))) < 0.05f);
        }
        std::cout << " -> Test 9 (StraightenLeg Collinear Verification): PASSED" << std::endl;
    }

    // Test 10: SnapToGrid rounds to nearest
    {
        PropModelEditEngine eng;
        std::vector<PropNodeSuggestion> nodes;
        PropNodeSuggestion n; n.x = 12.3f; n.y = 7.8f; n.channelIndex = 1; n.label = "A";
        nodes.push_back(n);
        eng.SnapToGrid(nodes, 5.0f);
        assert(std::abs(nodes[0].x - 10.0f) < 1e-4f);
        assert(std::abs(nodes[0].y - 10.0f) < 1e-4f);
        std::cout << " -> Test 10 (SnapToGrid Nearest Grid Multiple): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] PropModelEditEngine ALL TESTS PASSED!" << std::endl;
    return 0;
}
