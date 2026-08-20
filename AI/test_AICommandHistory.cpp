/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/ai/AICommandHistory.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running AICommandHistory verification..." << std::endl;

    AICommandHistory history(10);
    int value = 0;

    // Test 1: Execute Command
    {
        auto cmd1 = std::make_unique<LambdaAICommand>(
            "Add 10",
            [&value]() { value += 10; return true; },
            [&value]() { value -= 10; return true; }
        );
        bool ok = history.ExecuteCommand(std::move(cmd1));
        assert(ok);
        assert(value == 10);
        assert(history.CanUndo());
        assert(!history.CanRedo());
        assert(history.GetUndoDescription() == "Add 10");
        std::cout << " -> Test 1 (Execute Command): PASSED" << std::endl;
    }

    // Test 2: Undo Command
    {
        bool ok = history.Undo();
        assert(ok);
        assert(value == 0);
        assert(!history.CanUndo());
        assert(history.CanRedo());
        assert(history.GetRedoDescription() == "Add 10");
        std::cout << " -> Test 2 (Undo Command): PASSED" << std::endl;
    }

    // Test 3: Redo Command
    {
        bool ok = history.Redo();
        assert(ok);
        assert(value == 10);
        assert(history.CanUndo());
        assert(!history.CanRedo());
        std::cout << " -> Test 3 (Redo Command): PASSED" << std::endl;
    }

    // Test 4: Redo Branch Truncation
    {
        // Undo back to 0
        history.Undo();
        assert(value == 0);

        // Execute new branch: Multiply by 5
        auto cmd2 = std::make_unique<LambdaAICommand>(
            "Set to 50",
            [&value]() { value = 50; return true; },
            [&value]() { value = 0; return true; }
        );
        history.ExecuteCommand(std::move(cmd2));
        assert(value == 50);
        assert(!history.CanRedo()); // Redo stack truncated
        assert(history.GetUndoDescription() == "Set to 50");
        std::cout << " -> Test 4 (Redo Stack Branch Truncation): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] AICommandHistory ALL TESTS PASSED!" << std::endl;
    return 0;
}
