/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "AI/PropNodeEditHistory.h"
#include "AI/CustomPropDesignerAI.h"

using namespace xLights::AI;

// ── Helpers ───────────────────────────────────────────────────────────────────

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

// ── Tests ─────────────────────────────────────────────────────────────────────

TEST_CASE("PropNodeEditHistory: initial state", "[PropNodeEditHistory]") {
    PropNodeEditHistory h;
    REQUIRE_FALSE(h.CanUndo());
    REQUIRE_FALSE(h.CanRedo());
    REQUIRE(h.UndoDepth() == 0);
    REQUIRE(h.RedoDepth() == 0);
    REQUIRE(h.GetUndoDescription().empty());
    REQUIRE(h.GetRedoDescription().empty());
}

TEST_CASE("PropNodeEditHistory: push single edit and undo", "[PropNodeEditHistory]") {
    PropNodeEditHistory h;
    auto before = MakeNodes(5);
    auto after  = MakeNodes(10);

    h.PushEdit("Generate Layout", before, after);
    REQUIRE(h.CanUndo());
    REQUIRE_FALSE(h.CanRedo());
    REQUIRE(h.UndoDepth() == 1);
    REQUIRE(h.GetUndoDescription() == "Generate Layout");

    std::string desc;
    auto restored = h.Undo(desc);
    REQUIRE(desc == "Generate Layout");
    REQUIRE(restored.size() == 5);          // should match 'before'
    REQUIRE_FALSE(h.CanUndo());
    REQUIRE(h.CanRedo());
    REQUIRE(h.GetRedoDescription() == "Generate Layout");
}

TEST_CASE("PropNodeEditHistory: redo after undo", "[PropNodeEditHistory]") {
    PropNodeEditHistory h;
    h.PushEdit("Step 1", MakeNodes(3), MakeNodes(6));

    std::string desc;
    h.Undo(desc);
    auto redone = h.Redo(desc);
    REQUIRE(desc == "Step 1");
    REQUIRE(redone.size() == 6);
    REQUIRE(h.CanUndo());
    REQUIRE_FALSE(h.CanRedo());
}

TEST_CASE("PropNodeEditHistory: new push clears redo", "[PropNodeEditHistory]") {
    PropNodeEditHistory h;
    h.PushEdit("A", MakeNodes(2), MakeNodes(4));
    std::string desc;
    h.Undo(desc);                       // redo stack now has A
    REQUIRE(h.CanRedo());

    h.PushEdit("B", MakeNodes(4), MakeNodes(8));  // should clear redo
    REQUIRE_FALSE(h.CanRedo());
    REQUIRE(h.CanUndo());
    REQUIRE(h.GetUndoDescription() == "B");
}

TEST_CASE("PropNodeEditHistory: multi-step undo 5 times", "[PropNodeEditHistory]") {
    PropNodeEditHistory h;
    for (int i = 1; i <= 5; ++i) {
        h.PushEdit("Edit " + std::to_string(i), MakeNodes(i), MakeNodes(i + 1));
    }
    REQUIRE(h.UndoDepth() == 5);

    std::string desc;
    for (int i = 5; i >= 1; --i) {
        REQUIRE(h.CanUndo());
        auto s = h.Undo(desc);
        REQUIRE(desc == "Edit " + std::to_string(i));
        REQUIRE(s.size() == static_cast<size_t>(i));
    }
    REQUIRE_FALSE(h.CanUndo());
    REQUIRE(h.RedoDepth() == 5);
}

TEST_CASE("PropNodeEditHistory: clear resets all", "[PropNodeEditHistory]") {
    PropNodeEditHistory h;
    h.PushEdit("X", MakeNodes(1), MakeNodes(2));
    h.Clear();
    REQUIRE_FALSE(h.CanUndo());
    REQUIRE_FALSE(h.CanRedo());
}

TEST_CASE("PropNodeEditHistory: undo on empty returns empty", "[PropNodeEditHistory]") {
    PropNodeEditHistory h;
    std::string desc;
    auto result = h.Undo(desc);
    REQUIRE(result.empty());
    REQUIRE(desc.empty());
}
