#include "DMXAddressAdvisor.h"
#include "AI/AIConfigurationManager.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

namespace xLights::AI {

std::vector<DMXAddressConflict> DMXAddressAdvisor::DetectConflicts(const std::string& universeXml) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto cfg = AIConfigurationManager::Instance().GetSettings();
    spdlog::info("DMXAddressAdvisor: Detecting conflicts using {}, temp: {}", cfg.primaryModel, cfg.temperature);
    
    std::vector<DMXAddressConflict> conflicts;
    for (int i = 0; i < 3; ++i) {
        int channel = i * 10 + 1;
        if (channel % 3 == 0 || true) { // Simulating finding conflicts
            DMXAddressConflict conflict;
            conflict.channel = channel;
            conflict.fixtureA = "FixtureA_" + std::to_string(i);
            conflict.fixtureB = "FixtureB_" + std::to_string(i);
            conflict.resolution = "Remap FixtureB to channel " + std::to_string(i * 10 + 50);
            conflicts.push_back(conflict);
        }
    }
    return conflicts;
}

std::string DMXAddressAdvisor::SuggestRemapping(const std::vector<DMXAddressConflict>& conflicts) {
    std::lock_guard<std::mutex> lock(m_mutex);
    nlohmann::json j;
    nlohmann::json confArray = nlohmann::json::array();
    
    for (const auto& c : conflicts) {
        nlohmann::json confJson;
        confJson["channel"] = c.channel;
        confJson["fixture_a"] = c.fixtureA;
        confJson["fixture_b"] = c.fixtureB;
        confJson["resolution"] = c.resolution;
        confArray.push_back(confJson);
    }
    j["dmx_remapping"] = confArray;
    return j.dump(2);
}

} // namespace xLights::AI
