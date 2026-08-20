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
#include <mutex>
#include <chrono>
#include <nlohmann/json.hpp>

namespace xLights::AI {

struct GranularSnapshotItem {
    std::string id;
    std::string category; ///< "MODEL", "EFFECT", "TIMING", "COLOR_PALETTE", "LAYOUT"
    std::string name;
    std::string serializedPayload;
    bool isSelectedForExport{true};
};

struct AISnapshotNode {
    uint32_t snapshotId{0};
    std::string label{"Initial State"};
    std::string timestampStr;
    std::string sourceSubsystem{"Core"}; ///< e.g. "AudioChoreographerAI", "CustomPropDesigner", "Manual"
    std::string fullStateXml;
    std::vector<GranularSnapshotItem> granularItems;
    size_t memorySizeBytes{0};
};

class AISnapshotHistoryManager {
public:
    static AISnapshotHistoryManager& Instance();

    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    void SetMaxHistoryCapacity(size_t maxCapacity);
    size_t GetMaxHistoryCapacity() const;

    uint32_t PushSnapshot(const std::string& label, const std::string& fullXml,
                          const std::string& subsystem = "Core",
                          const std::vector<GranularSnapshotItem>& items = {});

    bool JumpToSnapshot(uint32_t snapshotId, std::string& outXml);
    uint32_t GetCurrentActiveSnapshotId() const;

    const std::vector<AISnapshotNode>& GetAllSnapshots() const;
    void ClearHistory();

    // Granular Export & Import
    std::string ExportPackage(uint32_t snapshotId, const std::vector<std::string>& selectedItemIds) const;
    bool ImportPackage(const std::string& packageJsonStr, std::string& outMergedXml);

private:
    AISnapshotHistoryManager();
    ~AISnapshotHistoryManager() = default;

    mutable std::mutex m_mutex;
    bool m_enabled{true};
    size_t m_maxCapacity{50};
    uint32_t m_nextSnapshotId{1};
    uint32_t m_currentActiveId{0};
    std::vector<AISnapshotNode> m_historyTimeline;
};

} // namespace xLights::AI
