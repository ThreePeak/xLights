/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/ai/AISnapshotHistoryManager.h"
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>

namespace xLights::AI {

AISnapshotHistoryManager& AISnapshotHistoryManager::Instance() {
    static AISnapshotHistoryManager instance;
    return instance;
}

AISnapshotHistoryManager::AISnapshotHistoryManager() {
    // Initial baseline state
    PushSnapshot("Initial Show Baseline", "<show_layout/>", "Core");
}

void AISnapshotHistoryManager::SetEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_enabled = enabled;
    spdlog::info("AISnapshotHistoryManager: Enabled state set to {}", m_enabled);
}

bool AISnapshotHistoryManager::IsEnabled() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_enabled;
}

void AISnapshotHistoryManager::SetMaxHistoryCapacity(size_t maxCapacity) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_maxCapacity = std::max(size_t(5), maxCapacity);
    while (m_historyTimeline.size() > m_maxCapacity) {
        m_historyTimeline.erase(m_historyTimeline.begin());
    }
}

size_t AISnapshotHistoryManager::GetMaxHistoryCapacity() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_maxCapacity;
}

uint32_t AISnapshotHistoryManager::PushSnapshot(
    const std::string& label,
    const std::string& fullXml,
    const std::string& subsystem,
    const std::vector<GranularSnapshotItem>& items
) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_enabled) return 0;

    AISnapshotNode node;
    node.snapshotId = m_nextSnapshotId++;
    node.label = label;
    node.sourceSubsystem = subsystem;
    node.fullStateXml = fullXml;
    node.granularItems = items;
    node.memorySizeBytes = fullXml.size();

    // Timestamp
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm timeInfo{};
#if defined(_WIN32)
    localtime_s(&timeInfo, &in_time_t);
#else
    localtime_r(&in_time_t, &timeInfo);
#endif
    std::stringstream ss;
    ss << std::put_time(&timeInfo, "%H:%M:%S");
    node.timestampStr = ss.str();

    m_historyTimeline.push_back(node);
    m_currentActiveId = node.snapshotId;

    while (m_historyTimeline.size() > m_maxCapacity) {
        m_historyTimeline.erase(m_historyTimeline.begin());
    }

    spdlog::info("AISnapshotHistoryManager: Recorded snapshot #{} '{}' (Subsystem: {}, Size: {} bytes)",
                 node.snapshotId, node.label, node.sourceSubsystem, node.memorySizeBytes);
    return node.snapshotId;
}

bool AISnapshotHistoryManager::JumpToSnapshot(uint32_t snapshotId, std::string& outXml) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& node : m_historyTimeline) {
        if (node.snapshotId == snapshotId) {
            m_currentActiveId = snapshotId;
            outXml = node.fullStateXml;
            spdlog::info("AISnapshotHistoryManager: Jumped to historical snapshot #{} '{}'",
                         node.snapshotId, node.label);
            return true;
        }
    }
    return false;
}

uint32_t AISnapshotHistoryManager::GetCurrentActiveSnapshotId() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentActiveId;
}

const std::vector<AISnapshotNode>& AISnapshotHistoryManager::GetAllSnapshots() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_historyTimeline;
}

void AISnapshotHistoryManager::ClearHistory() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_historyTimeline.clear();
    m_nextSnapshotId = 1;
    m_currentActiveId = 0;
}

std::string AISnapshotHistoryManager::ExportPackage(
    uint32_t snapshotId,
    const std::vector<std::string>& selectedItemIds
) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& node : m_historyTimeline) {
        if (node.snapshotId == snapshotId) {
            nlohmann::json pkg;
            pkg["version"] = "2026.1";
            pkg["snapshotId"] = node.snapshotId;
            pkg["label"] = node.label;
            pkg["timestamp"] = node.timestampStr;
            pkg["sourceSubsystem"] = node.sourceSubsystem;
            pkg["fullStateXml"] = node.fullStateXml;

            nlohmann::json itemsJson = nlohmann::json::array();
            for (const auto& item : node.granularItems) {
                bool include = selectedItemIds.empty();
                for (const auto& selId : selectedItemIds) {
                    if (selId == item.id) { include = true; break; }
                }
                if (include) {
                    itemsJson.push_back({
                        {"id", item.id},
                        {"category", item.category},
                        {"name", item.name},
                        {"serializedPayload", item.serializedPayload}
                    });
                }
            }
            pkg["granularItems"] = itemsJson;
            return pkg.dump(2);
        }
    }
    return "{}";
}

bool AISnapshotHistoryManager::ImportPackage(const std::string& packageJsonStr, std::string& outMergedXml) {
    try {
        auto pkg = nlohmann::json::parse(packageJsonStr);
        if (pkg.contains("fullStateXml")) {
            outMergedXml = pkg["fullStateXml"].get<std::string>();
            std::string label = pkg.value("label", "Imported Snapshot");
            std::string sub = pkg.value("sourceSubsystem", "Import");
            PushSnapshot("Imported: " + label, outMergedXml, sub);
            return true;
        }
    } catch (...) {
        return false;
    }
    return false;
}

} // namespace xLights::AI
