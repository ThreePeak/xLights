/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AI/PropNodeEditHistory.h"
#include <spdlog/spdlog.h>

namespace xLights::AI {

void PropNodeEditHistory::Clear() {
    m_undoStack.clear();
    m_redoStack.clear();
}

void PropNodeEditHistory::PushEdit(std::string description,
                                   std::vector<PropNodeSuggestion> before,
                                   std::vector<PropNodeSuggestion> after) {
    // Clear redo — new edit invalidates the future branch
    m_redoStack.clear();

    PropEditCommand cmd;
    cmd.description  = std::move(description);
    cmd.beforeState  = std::move(before);
    cmd.afterState   = std::move(after);
    m_undoStack.push_back(std::move(cmd));

    // Cap undo depth
    while (static_cast<int>(m_undoStack.size()) > kMaxHistory) {
        m_undoStack.pop_front();
    }

    spdlog::debug("PropNodeEditHistory: push '{}' — undo depth={}, redo cleared",
                  m_undoStack.back().description, m_undoStack.size());
}

std::vector<PropNodeSuggestion> PropNodeEditHistory::Undo(std::string& descOut) {
    if (m_undoStack.empty()) {
        descOut = "";
        return {};
    }
    PropEditCommand cmd = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    descOut = cmd.description;
    auto before = cmd.beforeState;

    // Push to redo
    m_redoStack.push_back(std::move(cmd));

    spdlog::info("PropNodeEditHistory: undo '{}' — undo depth={}", descOut, m_undoStack.size());
    return before;
}

std::vector<PropNodeSuggestion> PropNodeEditHistory::Redo(std::string& descOut) {
    if (m_redoStack.empty()) {
        descOut = "";
        return {};
    }
    PropEditCommand cmd = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    descOut = cmd.description;
    auto after = cmd.afterState;

    // Push back to undo
    m_undoStack.push_back(std::move(cmd));

    spdlog::info("PropNodeEditHistory: redo '{}' — undo depth={}", descOut, m_undoStack.size());
    return after;
}

std::string PropNodeEditHistory::GetUndoDescription() const {
    if (m_undoStack.empty()) return "";
    return m_undoStack.back().description;
}

std::string PropNodeEditHistory::GetRedoDescription() const {
    if (m_redoStack.empty()) return "";
    return m_redoStack.back().description;
}

} // namespace xLights::AI
