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

namespace xLights::AI {

/// Result from InsertNode — carries the updated node list and metadata so
/// the UI can prompt the user about ID renumbering before committing.
struct InsertNodeResult {
    std::vector<PropNodeSuggestion> updatedNodes;
    bool   renumberingRequired = false;
    int    insertedAtIndex     = -1;
    int    conflictingId       = -1;
    /// Ready-made suggestion string shown to the user: e.g.
    /// "Node ID 50 already exists. Shift nodes 50-99 → 51-100?"
    std::string aiSuggestion;
};

/// Pure C++20 stateless engine for all geometric node manipulation.
/// No wxWidgets, no globals — pass nodes by value, get transformed result back.
class PropModelEditEngine {
public:
    // ── Movement ────────────────────────────────────────────────────
    /// Translate a single node by (dx, dy, dz) in model-space units.
    void MoveNode(std::vector<PropNodeSuggestion>& nodes,
                  int nodeIndex, float dx, float dy, float dz);

    /// Translate every node whose index appears in selection.
    void MoveSelection(std::vector<PropNodeSuggestion>& nodes,
                       const std::vector<int>& selection,
                       float dx, float dy, float dz);

    /// Set absolute position of a single node.
    void SetNodePosition(std::vector<PropNodeSuggestion>& nodes,
                         int nodeIndex, float x, float y, float z);

    // ── Add / Remove ────────────────────────────────────────────────
    /// Insert a new node. Detects channel ID conflicts and fills in
    /// InsertNodeResult::renumberingRequired + aiSuggestion when needed.
    InsertNodeResult InsertNode(std::vector<PropNodeSuggestion> nodes,
                                float x, float y, float z,
                                int requestedChannelId);

    /// Remove the node at nodeIndex. Returns the modified list.
    std::vector<PropNodeSuggestion> RemoveNode(
        std::vector<PropNodeSuggestion> nodes, int nodeIndex);

    // ── ID Renumbering ──────────────────────────────────────────────
    /// Shift channelIndex of every node whose channelIndex >= fromChannelId
    /// by shiftBy (positive = shift up).
    std::vector<PropNodeSuggestion> RenumberNodesFrom(
        std::vector<PropNodeSuggestion> nodes,
        int fromChannelId, int shiftBy);

    // ── Geometry helpers ────────────────────────────────────────────
    /// Redistribute all nodes so they are evenly spaced along their
    /// current geometric bounding path.
    void EvenlySpaceNodes(std::vector<PropNodeSuggestion>& nodes);

    /// Straighten a contiguous leg (sequence of node indices) so they
    /// are collinear between their first and last point, evenly spaced.
    void StraightenLeg(std::vector<PropNodeSuggestion>& nodes,
                       int legStartIndex, int legEndIndex);

    /// Snap every node to the nearest gridSize-unit grid intersection.
    void SnapToGrid(std::vector<PropNodeSuggestion>& nodes, float gridSize);
};

} // namespace xLights::AI
