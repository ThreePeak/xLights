/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AI/AINodeEditInterpreter.h"
#include "AI/PropModelEditEngine.h"
#include <spdlog/spdlog.h>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace xLights::AI {

// ── Public API ────────────────────────────────────────────────────────────────

NodeEditPlan AINodeEditInterpreter::InterpretPrompt(
        const std::string& userPrompt,
        const std::vector<PropNodeSuggestion>& currentNodes,
        const AIConfigSettings& cfg) {

    spdlog::info("AINodeEditInterpreter: InterpretPrompt — model={}, nodes={}, prompt=\"{}\"",
                 cfg.primaryModel, currentNodes.size(), userPrompt);

    // Build the structured system prompt
    std::string systemPrompt = BuildSystemPrompt(currentNodes);

    // Compose the full LLM request body
    // In a real implementation this would call the active provider's HTTP API
    // using cfg.openAIKey / cfg.ollamaEndpoint etc.  For now we produce a
    // well-structured plan based on keyword analysis of the prompt so the UI
    // flow (confirmation → execute) is exercised end-to-end.
    std::string lowerPrompt = userPrompt;
    std::transform(lowerPrompt.begin(), lowerPrompt.end(), lowerPrompt.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    NodeEditPlan plan;
    plan.requiresUserApproval = true;

    std::ostringstream opStream;
    std::string confirmParts;

    // ── Keyword → operation mapping ───────────────────────────────────────────
    bool opAdded = false;

    if (lowerPrompt.find("straighten") != std::string::npos ||
        lowerPrompt.find("straight") != std::string::npos) {
        // Straighten each half as a "leg" split at midpoint
        int mid = static_cast<int>(currentNodes.size()) / 2;
        if (mid > 1) {
            opStream << "STRAIGHTEN_LEG 0 " << (mid - 1) << "\n";
            opStream << "STRAIGHTEN_LEG " << mid << " " << (static_cast<int>(currentNodes.size()) - 1) << "\n";
            confirmParts += "straighten all legs";
            opAdded = true;
        }
    }

    if (lowerPrompt.find("even") != std::string::npos ||
        lowerPrompt.find("space") != std::string::npos ||
        lowerPrompt.find("spread") != std::string::npos) {
        opStream << "EVEN_SPACE_ALL\n";
        if (!confirmParts.empty()) confirmParts += " and ";
        confirmParts += "evenly space all " + std::to_string(currentNodes.size()) + " nodes";
        opAdded = true;
    }

    if (lowerPrompt.find("snap") != std::string::npos ||
        lowerPrompt.find("grid") != std::string::npos) {
        opStream << "SNAP_GRID 5.0\n";
        if (!confirmParts.empty()) confirmParts += " and ";
        confirmParts += "snap all nodes to a 5-unit grid";
        opAdded = true;
    }

    if (!opAdded) {
        // Generic fallback — surface that AI received the prompt
        opStream << "EVEN_SPACE_ALL\n";
        confirmParts = "apply general layout optimisation to all " +
                       std::to_string(currentNodes.size()) + " nodes";
    }

    plan.operationCode = opStream.str();
    plan.confirmationMessage =
        "I will " + confirmParts + " on this " +
        std::to_string(currentNodes.size()) + "-node model. "
        "Confirm to proceed, or Cancel to leave the model unchanged.";

    spdlog::info("AINodeEditInterpreter: plan confirmation=\"{}\"", plan.confirmationMessage);
    return plan;
}

std::vector<PropNodeSuggestion> AINodeEditInterpreter::ExecutePlan(
        const NodeEditPlan& plan,
        std::vector<PropNodeSuggestion> nodes) {

    if (plan.hasError || plan.operationCode.empty()) return nodes;

    spdlog::info("AINodeEditInterpreter: ExecutePlan — {} bytes of ops", plan.operationCode.size());

    std::istringstream stream(plan.operationCode);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty()) {
            ApplyOperation(line, nodes);
        }
    }
    return nodes;
}

// ── Private helpers ───────────────────────────────────────────────────────────

std::string AINodeEditInterpreter::BuildSystemPrompt(
        const std::vector<PropNodeSuggestion>& nodes) const {
    std::ostringstream ss;
    ss << "You are an expert xLights prop model editor. "
          "The current model has " << nodes.size() << " nodes. "
          "Node positions (x, y, channelId):\n";
    for (const auto& n : nodes) {
        ss << "  " << n.label << ": x=" << n.x << " y=" << n.y
           << " ch=" << n.channelIndex << "\n";
    }
    ss << "Return a list of operation tokens only. Supported tokens:\n"
          "  MOVE_NODE <index> <dx> <dy>\n"
          "  SET_NODE_POS <index> <x> <y>\n"
          "  EVEN_SPACE_ALL\n"
          "  STRAIGHTEN_LEG <startIdx> <endIdx>\n"
          "  SNAP_GRID <gridSize>\n";
    return ss.str();
}

NodeEditPlan AINodeEditInterpreter::ParseLLMResponse(const std::string& response) const {
    NodeEditPlan plan;
    plan.requiresUserApproval = true;
    plan.operationCode = response;
    plan.confirmationMessage = "Execute the following AI-generated node edits: " + response;
    return plan;
}

void AINodeEditInterpreter::ApplyOperation(const std::string& op,
                                           std::vector<PropNodeSuggestion>& nodes) const {
    PropModelEditEngine engine;
    std::istringstream ss(op);
    std::string token;
    ss >> token;

    if (token == "EVEN_SPACE_ALL") {
        engine.EvenlySpaceNodes(nodes);

    } else if (token == "STRAIGHTEN_LEG") {
        int start = 0, end = 0;
        ss >> start >> end;
        engine.StraightenLeg(nodes, start, end);

    } else if (token == "SNAP_GRID") {
        float gridSize = 5.0f;
        ss >> gridSize;
        engine.SnapToGrid(nodes, gridSize);

    } else if (token == "MOVE_NODE") {
        int index = 0; float dx = 0, dy = 0;
        ss >> index >> dx >> dy;
        engine.MoveNode(nodes, index, dx, dy, 0.0f);

    } else if (token == "SET_NODE_POS") {
        int index = 0; float x = 0, y = 0;
        ss >> index >> x >> y;
        engine.SetNodePosition(nodes, index, x, y, 0.0f);

    } else {
        spdlog::warn("AINodeEditInterpreter: Unknown operation token '{}'", token);
    }
}

} // namespace xLights::AI
