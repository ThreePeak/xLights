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

#include "AI/CustomPropDesignerAI.h"
#include "AI/AIConfigurationManager.h"

namespace xLights::AI {

/// A pending edit plan returned by AINodeEditInterpreter::InterpretPrompt.
/// Contains only a description — no mutations have occurred yet.
/// ExecutePlan() fires only after explicit user confirmation.
struct NodeEditPlan {
    /// Human-readable summary shown to the user for confirmation.
    /// e.g. "I will straighten 6 legs and evenly space 50 nodes across
    ///       the model, ensuring 12 cm gaps between each node."
    std::string confirmationMessage;

    /// Serialized list of operations (internal format).
    /// Opaque to the UI — parsed back in ExecutePlan().
    std::string operationCode;

    /// Always true — the AI never executes without user confirmation.
    bool requiresUserApproval = true;

    /// Set to true when the LLM call itself failed.
    bool hasError  = false;
    std::string errorMessage;
};

/// Interprets natural language prompts into NodeEditPlans and executes them.
/// Pure C++20, no wxWidgets.
class AINodeEditInterpreter {
public:
    /// Send the user prompt + current node layout to the active LLM.
    /// Returns a NodeEditPlan whose confirmationMessage is shown to the user.
    /// NEVER mutates nodes — always requires explicit ExecutePlan() call.
    NodeEditPlan InterpretPrompt(const std::string& userPrompt,
                                 const std::vector<PropNodeSuggestion>& currentNodes,
                                 const AIConfigSettings& cfg);

    /// Apply the operations described in plan.operationCode to nodes.
    /// Call this ONLY after the user has confirmed the plan.
    std::vector<PropNodeSuggestion> ExecutePlan(
        const NodeEditPlan& plan,
        std::vector<PropNodeSuggestion> nodes);

private:
    /// Build the structured system prompt sent to the LLM.
    std::string BuildSystemPrompt(const std::vector<PropNodeSuggestion>& nodes) const;

    /// Parse the LLM response into a NodeEditPlan.
    NodeEditPlan ParseLLMResponse(const std::string& response) const;

    /// Execute a single operation token (e.g. "MOVE_NODE 5 dx=10 dy=0 dz=0").
    void ApplyOperation(const std::string& op,
                        std::vector<PropNodeSuggestion>& nodes) const;
};

} // namespace xLights::AI
