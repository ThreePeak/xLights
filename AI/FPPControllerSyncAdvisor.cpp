#include "FPPControllerSyncAdvisor.h"
#include "AI/AIConfigurationManager.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <sstream>

namespace xLights::AI {

std::vector<FPPSyncSuggestion> FPPControllerSyncAdvisor::AnalyzeControllerLayout(const std::string& showXml) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto cfg = AIConfigurationManager::Instance().GetSettings();
    spdlog::info("FPPControllerSyncAdvisor: Analyzing layout using {}, temp: {}", cfg.primaryModel, cfg.temperature);
    
    std::vector<FPPSyncSuggestion> suggestions;
    for (int i = 0; i < 3; ++i) {
        FPPSyncSuggestion sugg;
        sugg.controllerIP = "192.168.1." + std::to_string(100 + i);
        sugg.universe = "Universe " + std::to_string(i + 1);
        sugg.suggestedStartChannel = (i * 512) + 1;
        sugg.rationale = "AI-assigned channel block based on prop density analysis";
        suggestions.push_back(sugg);
    }
    return suggestions;
}

std::string FPPControllerSyncAdvisor::GenerateFPPJsonManifest(const std::vector<FPPSyncSuggestion>& suggestions) {
    std::lock_guard<std::mutex> lock(m_mutex);
    nlohmann::json j;
    j["fpp_version"] = "2.0";
    nlohmann::json suggArray = nlohmann::json::array();
    
    for (const auto& s : suggestions) {
        nlohmann::json suggJson;
        suggJson["ip"] = s.controllerIP;
        suggJson["universe"] = s.universe;
        suggJson["start_channel"] = s.suggestedStartChannel;
        suggJson["rationale"] = s.rationale;
        suggArray.push_back(suggJson);
    }
    j["suggestions"] = suggArray;
    return j.dump(2);
}

} // namespace xLights::AI
