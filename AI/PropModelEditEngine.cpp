/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AI/PropModelEditEngine.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace xLights::AI {

// ── Movement ─────────────────────────────────────────────────────────────────

void PropModelEditEngine::MoveNode(std::vector<PropNodeSuggestion>& nodes,
                                   int nodeIndex, float dx, float dy, float dz) {
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(nodes.size())) return;
    nodes[nodeIndex].x += dx;
    nodes[nodeIndex].y += dy;
    // z stored as label suffix for now — extend PropNodeSuggestion if 3D needed
    spdlog::debug("PropModelEditEngine: MoveNode {} by ({:.2f},{:.2f},{:.2f})", nodeIndex, dx, dy, dz);
}

void PropModelEditEngine::MoveSelection(std::vector<PropNodeSuggestion>& nodes,
                                        const std::vector<int>& selection,
                                        float dx, float dy, float dz) {
    for (int idx : selection) {
        MoveNode(nodes, idx, dx, dy, dz);
    }
}

void PropModelEditEngine::SetNodePosition(std::vector<PropNodeSuggestion>& nodes,
                                          int nodeIndex, float x, float y, float /*z*/) {
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(nodes.size())) return;
    nodes[nodeIndex].x = x;
    nodes[nodeIndex].y = y;
}

// ── Add / Remove ──────────────────────────────────────────────────────────────

InsertNodeResult PropModelEditEngine::InsertNode(std::vector<PropNodeSuggestion> nodes,
                                                 float x, float y, float /*z*/,
                                                 int requestedChannelId) {
    InsertNodeResult result;

    // Check for channel ID conflict
    auto it = std::find_if(nodes.begin(), nodes.end(),
                           [requestedChannelId](const PropNodeSuggestion& n) {
                               return n.channelIndex == requestedChannelId;
                           });

    if (it != nodes.end()) {
        result.renumberingRequired = true;
        result.conflictingId       = requestedChannelId;
        // Count how many nodes have IDs >= requestedChannelId
        int count = static_cast<int>(std::count_if(nodes.begin(), nodes.end(),
            [requestedChannelId](const PropNodeSuggestion& n) {
                return n.channelIndex >= requestedChannelId;
            }));
        int maxId = 0;
        for (const auto& n : nodes) maxId = std::max(maxId, n.channelIndex);

        result.aiSuggestion = "Node ID " + std::to_string(requestedChannelId) +
                              " already exists. Shift nodes " +
                              std::to_string(requestedChannelId) + "-" +
                              std::to_string(maxId) + " → " +
                              std::to_string(requestedChannelId + 1) + "-" +
                              std::to_string(maxId + 1) + "?";
        spdlog::info("PropModelEditEngine: InsertNode conflict at ID {} — {}", requestedChannelId, result.aiSuggestion);
    }

    PropNodeSuggestion newNode;
    newNode.x            = x;
    newNode.y            = y;
    newNode.channelIndex = requestedChannelId;
    newNode.label        = "Node_" + std::to_string(requestedChannelId);

    // Insert at position such that nodes remain sorted by channelIndex
    auto insertPos = std::lower_bound(nodes.begin(), nodes.end(), newNode,
        [](const PropNodeSuggestion& a, const PropNodeSuggestion& b) {
            return a.channelIndex < b.channelIndex;
        });
    result.insertedAtIndex = static_cast<int>(std::distance(nodes.begin(), insertPos));
    nodes.insert(insertPos, newNode);
    result.updatedNodes = std::move(nodes);
    return result;
}

std::vector<PropNodeSuggestion> PropModelEditEngine::RemoveNode(
        std::vector<PropNodeSuggestion> nodes, int nodeIndex) {
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(nodes.size())) return nodes;
    spdlog::info("PropModelEditEngine: RemoveNode at index {} (ID {})",
                 nodeIndex, nodes[nodeIndex].channelIndex);
    nodes.erase(nodes.begin() + nodeIndex);
    return nodes;
}

// ── ID Renumbering ────────────────────────────────────────────────────────────

std::vector<PropNodeSuggestion> PropModelEditEngine::RenumberNodesFrom(
        std::vector<PropNodeSuggestion> nodes, int fromChannelId, int shiftBy) {
    for (auto& node : nodes) {
        if (node.channelIndex >= fromChannelId) {
            node.channelIndex += shiftBy;
            // Update label to reflect new ID
            node.label = "Node_" + std::to_string(node.channelIndex);
        }
    }
    spdlog::info("PropModelEditEngine: RenumberNodesFrom ID {} by +{}", fromChannelId, shiftBy);
    return nodes;
}

// ── Geometry Helpers ──────────────────────────────────────────────────────────

void PropModelEditEngine::EvenlySpaceNodes(std::vector<PropNodeSuggestion>& nodes) {
    if (nodes.size() < 2) return;

    // Compute total path length
    float totalLen = 0.0f;
    std::vector<float> cumLen(nodes.size(), 0.0f);
    for (size_t i = 1; i < nodes.size(); ++i) {
        float dx = nodes[i].x - nodes[i-1].x;
        float dy = nodes[i].y - nodes[i-1].y;
        cumLen[i] = cumLen[i-1] + std::sqrt(dx*dx + dy*dy);
    }
    totalLen = cumLen.back();
    if (totalLen < 1e-6f) return;

    float step = totalLen / static_cast<float>(nodes.size() - 1);

    std::vector<PropNodeSuggestion> original = nodes;
    for (size_t i = 1; i + 1 < nodes.size(); ++i) {
        float targetLen = step * static_cast<float>(i);
        // Find segment
        size_t seg = 1;
        while (seg < cumLen.size() - 1 && cumLen[seg] < targetLen) ++seg;
        float segFrac = (targetLen - cumLen[seg-1]) / (cumLen[seg] - cumLen[seg-1] + 1e-9f);
        nodes[i].x = original[seg-1].x + segFrac * (original[seg].x - original[seg-1].x);
        nodes[i].y = original[seg-1].y + segFrac * (original[seg].y - original[seg-1].y);
    }
    spdlog::info("PropModelEditEngine: EvenlySpaceNodes across {} nodes, totalLen={:.2f}", nodes.size(), totalLen);
}

void PropModelEditEngine::StraightenLeg(std::vector<PropNodeSuggestion>& nodes,
                                        int legStartIndex, int legEndIndex) {
    if (legStartIndex < 0 || legEndIndex >= static_cast<int>(nodes.size()) ||
        legStartIndex >= legEndIndex) return;

    float x0 = nodes[legStartIndex].x, y0 = nodes[legStartIndex].y;
    float x1 = nodes[legEndIndex].x,   y1 = nodes[legEndIndex].y;
    int count = legEndIndex - legStartIndex;

    for (int i = legStartIndex + 1; i < legEndIndex; ++i) {
        float t = static_cast<float>(i - legStartIndex) / static_cast<float>(count);
        nodes[i].x = x0 + t * (x1 - x0);
        nodes[i].y = y0 + t * (y1 - y0);
    }
    spdlog::info("PropModelEditEngine: StraightenLeg [{},{}]", legStartIndex, legEndIndex);
}

void PropModelEditEngine::SnapToGrid(std::vector<PropNodeSuggestion>& nodes, float gridSize) {
    if (gridSize < 1e-6f) return;
    for (auto& node : nodes) {
        node.x = std::round(node.x / gridSize) * gridSize;
        node.y = std::round(node.y / gridSize) * gridSize;
    }
    spdlog::debug("PropModelEditEngine: SnapToGrid gridSize={:.2f}", gridSize);
}

} // namespace xLights::AI
