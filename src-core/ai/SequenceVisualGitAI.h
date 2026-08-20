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
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

namespace xLights::AI {

enum class DiffChangeType {
    UNCHANGED = 0,
    ADDED = 1,        ///< Green: Effect exists in Branch B but not in Base A
    DELETED = 2,      ///< Red: Effect exists in Base A but removed in Branch B
    MODIFIED = 3,     ///< Yellow: Effect parameters or color altered
    CONFLICT = 4      ///< Orange/Red: Overlapping conflicting edits on the same model/timing
};

enum class MergeStrategy {
    KEEP_BASE = 0,         ///< Accept version from Base Sequence A
    KEEP_INCOMING = 1,     ///< Accept version from Branch Sequence B
    SPLIT_SUB_LAYERS = 2,  ///< Non-destructive: Place both on separate stacked effect layers
    INTELLIGENT_BLEND = 3  ///< AI-calculated crossfade transition between the two effects
};

struct EffectDiffItem {
    std::string effectId;
    std::string modelName{"MegaTree"};
    int layerIndex{0};
    uint32_t startMs{0};
    uint32_t endMs{2000};
    std::string effectType{"Bars"};
    DiffChangeType changeType{DiffChangeType::UNCHANGED};
    std::string baseValue;
    std::string incomingValue;
    MergeStrategy chosenStrategy{MergeStrategy::SPLIT_SUB_LAYERS};
};

struct SequenceDiffReport {
    bool analysisSuccess{true};
    std::string baseSequenceName{"Show_Master_v1.xsq"};
    std::string incomingSequenceName{"Collaborator_Branch.xsq"};
    size_t totalEffectsCompared{0};
    size_t additionsCount{0};
    size_t deletionsCount{0};
    size_t modificationsCount{0};
    size_t conflictsCount{0};
    std::vector<EffectDiffItem> diffItems;

    [[nodiscard]] nlohmann::json ToJson() const;
    [[nodiscard]] std::string GenerateFormattedReport() const;
};

class SequenceVisualGitAI {
public:
    static SequenceDiffReport CompareSequences(
        const std::string& baseSequenceXml,
        const std::string& incomingSequenceXml,
        const std::string& baseName = "BaseSequence.xsq",
        const std::string& incomingName = "IncomingBranch.xsq"
    );

    static std::string ResolveAndMergeSequence(
        const SequenceDiffReport& diffReport,
        const std::string& baseSequenceXml
    );
};

} // namespace xLights::AI
