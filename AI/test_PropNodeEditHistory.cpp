/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PropNodeEditHistory.h"
#include "CustomPropDesignerAI.h"
#include <iostream>
#include <vector>
#include <cassert>

using namespace xLights::AI;

static std::vector<PropNodeSuggestion> MakeNodes(int count) {
    std::vector<PropNodeSuggestion> nodes;
    for (int i = 0; i < count; ++i) {
        PropNodeSuggestion n;
        n.x = static_cast<float>(i * 10);
        n.y = static_cast<float>(i * 5);
        n.channelIndex = i + 1;
        n.label = "Node_" + std::to_string(i + 1);
        nodes.push_back(n);
    }
    return nodes;
}

int main() {
    std::cout << "[Unit Test] Running PropNodeEditHistory verification..." << std::endl;

    // Test 1: Initial state
    {
        PropNodeEditHistory h;
        assert(!h.CanUndo());
        assert(!h.CanRedo());
        assert(h.UndoDepth() == 0);
        assert(h.RedoDepth() == 0);
        assert(h.GetUndoDescription().empty());
        assert(h.GetRedoDescription().empty());
        std::cout << " -> Test 1 (Initial State): PASSED" << std::endl;
    }

    // Test 2: Push single edit, undo, redo
    {
        PropNodeEditHistory h;
        auto before = MakeNodes(5);
        auto after  = MakeNodes(10);

        h.PushEdit("Generate Layout", before, after);
        assert(h.CanUndo());
        assert(!h.CanRedo());
        assert(h.UndoDepth() == 1);
        assert(h.GetUndoDescription() == "Generate Layout");

        std::string desc;
        auto restored = h.Undo(desc);
        assert(desc == "Generate Layout");
        assert(restored.size() == 5);
        assert(!h.CanUndo());
        assert(h.CanRedo());
        assert(h.GetRedoDescription() == "Generate Layout");

        std::string redoDesc;
        auto redone = h.Redo(redoDesc);
        assert(redoDesc == "Generate Layout");
        assert(redone.size() == 10);
        assert(h.CanUndo());
        assert(!h.CanRedo());
        std::cout << " -> Test 2 (Push, Undo, Redo Single Edit): PASSED" << std::endl;
    }

    // Test 3: Multiple edits in sequence
    {
        PropNodeEditHistory h;
        h.PushEdit("Step 1", MakeNodes(2), MakeNodes(4));
        h.PushEdit("Step 2", MakeNodes(4), MakeNodes(6));
        h.PushEdit("Step 3", MakeNodes(6), MakeNodes(8));

        assert(h.UndoDepth() == 3);
        assert(h.GetUndoDescription() == "Step 3");

        std::string desc;
        h.Undo(desc);
        assert(desc == "Step 3");
        assert(h.GetUndoDescription() == "Step 2");

        h.Undo(desc);
        assert(desc == "Step 2");
        assert(h.GetUndoDescription() == "Step 1");

        // Push new branch clears redo
        h.PushEdit("Step 2b", MakeNodes(4), MakeNodes(12));
        assert(h.UndoDepth() == 2);
        assert(!h.CanRedo());
        std::cout << " -> Test 3 (Multiple Undo & Branch Clearing): PASSED" << std::endl;
    }

    // Test 4: Capacity limit (kMaxHistory = 100)
    {
        PropNodeEditHistory h;
        for (int i = 0; i < 120; ++i) {
            h.PushEdit("Edit " + std::to_string(i), MakeNodes(i), MakeNodes(i + 1));
        }
        assert(h.UndoDepth() == PropNodeEditHistory::kMaxHistory);
        std::cout << " -> Test 4 (Capacity Bounds: 100 max entries): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] PropNodeEditHistory ALL TESTS PASSED!" << std::endl;
    return 0;
}
