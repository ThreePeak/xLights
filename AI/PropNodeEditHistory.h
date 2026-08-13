/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <string>
#include <vector>
#include <deque>

#include "AI/CustomPropDesignerAI.h"

namespace xLights::AI {

/// One reversible edit — stores the full node list before and after.
struct PropEditCommand {
    std::string description;
    std::vector<PropNodeSuggestion> beforeState;
    std::vector<PropNodeSuggestion> afterState;
};

/// Thread-compatible (caller must serialize) 100-step undo/redo stack.
class PropNodeEditHistory {
public:
    static constexpr int kMaxHistory = 100;

    void Clear();

    /// Push a new edit. Clears the redo stack.
    void PushEdit(std::string description,
                  std::vector<PropNodeSuggestion> before,
                  std::vector<PropNodeSuggestion> after);

    bool CanUndo() const { return !m_undoStack.empty(); }
    bool CanRedo() const { return !m_redoStack.empty(); }

    /// Step one undo. Returns the before-state and sets descOut to the command description.
    /// Caller must check CanUndo() first.
    std::vector<PropNodeSuggestion> Undo(std::string& descOut);

    /// Step one redo. Returns the after-state and sets descOut.
    /// Caller must check CanRedo() first.
    std::vector<PropNodeSuggestion> Redo(std::string& descOut);

    /// Human-readable label for the next undo step (e.g. "Move Node 12").
    std::string GetUndoDescription() const;
    /// Human-readable label for the next redo step.
    std::string GetRedoDescription() const;

    int UndoDepth()  const { return static_cast<int>(m_undoStack.size()); }
    int RedoDepth()  const { return static_cast<int>(m_redoStack.size()); }

private:
    std::deque<PropEditCommand> m_undoStack;
    std::deque<PropEditCommand> m_redoStack;
};

} // namespace xLights::AI
