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
#include <memory>
#include <functional>
#include <mutex>
#include <spdlog/spdlog.h>

namespace xLights::AI {

/**
 * @brief Abstract Command interface for all AI and layout operations.
 */
class AICommand {
public:
    explicit AICommand(std::string description) : m_description(std::move(description)) {}
    virtual ~AICommand() = default;

    virtual bool Execute() = 0;
    virtual bool Undo() = 0;
    virtual bool Redo() { return Execute(); }

    [[nodiscard]] const std::string& GetDescription() const { return m_description; }

protected:
    std::string m_description;
};

/**
 * @brief Lambda-based lightweight Command implementation for fast inline undo/redo.
 */
class LambdaAICommand : public AICommand {
public:
    LambdaAICommand(
        std::string description,
        std::function<bool()> executeFn,
        std::function<bool()> undoFn
    ) : AICommand(std::move(description)),
        m_executeFn(std::move(executeFn)),
        m_undoFn(std::move(undoFn)) {}

    bool Execute() override { return m_executeFn ? m_executeFn() : false; }
    bool Undo() override { return m_undoFn ? m_undoFn() : false; }

private:
    std::function<bool()> m_executeFn;
    std::function<bool()> m_undoFn;
};

/**
 * @brief Thread-safe Multi-level Undo/Redo Manager for AI and tool actions.
 */
class AICommandHistory {
public:
    explicit AICommandHistory(size_t maxHistoryDepth = 100)
        : m_maxDepth(maxHistoryDepth) {}

    ~AICommandHistory() = default;

    /// Executes and records a new command, invalidating any existing redo stack
    bool ExecuteCommand(std::unique_ptr<AICommand> command) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!command) return false;

        if (!command->Execute()) {
            spdlog::warn("AICommandHistory: Command '{}' execution failed.", command->GetDescription());
            return false;
        }

        // Truncate redo stack past current index
        if (m_currentIndex < m_history.size()) {
            m_history.erase(m_history.begin() + m_currentIndex, m_history.end());
        }

        m_history.push_back(std::move(command));
        if (m_history.size() > m_maxDepth) {
            m_history.erase(m_history.begin());
        } else {
            m_currentIndex++;
        }

        spdlog::info("AICommandHistory: Executed '{}' (History depth: {}/{})",
                     m_history.back()->GetDescription(), m_currentIndex, m_history.size());
        return true;
    }

    /// Undoes the last command
    bool Undo() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!CanUndo()) {
            spdlog::warn("AICommandHistory: Nothing to Undo.");
            return false;
        }

        m_currentIndex--;
        bool ok = m_history[m_currentIndex]->Undo();
        spdlog::info("AICommandHistory: Undid '{}'", m_history[m_currentIndex]->GetDescription());
        return ok;
    }

    /// Redoes the next command in the history stack
    bool Redo() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!CanRedo()) {
            spdlog::warn("AICommandHistory: Nothing to Redo.");
            return false;
        }

        bool ok = m_history[m_currentIndex]->Redo();
        spdlog::info("AICommandHistory: Redid '{}'", m_history[m_currentIndex]->GetDescription());
        m_currentIndex++;
        return ok;
    }

    [[nodiscard]] bool CanUndo() const { return m_currentIndex > 0; }
    [[nodiscard]] bool CanRedo() const { return m_currentIndex < m_history.size(); }

    [[nodiscard]] std::string GetUndoDescription() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return CanUndo() ? m_history[m_currentIndex - 1]->GetDescription() : "";
    }

    [[nodiscard]] std::string GetRedoDescription() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return CanRedo() ? m_history[m_currentIndex]->GetDescription() : "";
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_history.clear();
        m_currentIndex = 0;
    }

    [[nodiscard]] size_t GetHistoryCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_history.size();
    }

private:
    mutable std::mutex m_mutex;
    std::vector<std::unique_ptr<AICommand>> m_history;
    size_t m_currentIndex{0};
    size_t m_maxDepth{100};
};

} // namespace xLights::AI
