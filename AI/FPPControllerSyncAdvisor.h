#pragma once
#include <string>
#include <vector>
#include <mutex>

namespace xLights::AI {

struct FPPSyncSuggestion {
    std::string controllerIP;
    std::string universe;
    int suggestedStartChannel = 1;
    std::string rationale;
};

class FPPControllerSyncAdvisor {
public:
    std::vector<FPPSyncSuggestion> AnalyzeControllerLayout(const std::string& showXml);
    std::string GenerateFPPJsonManifest(const std::vector<FPPSyncSuggestion>& suggestions);
private:
    mutable std::mutex m_mutex;
};

} // namespace xLights::AI
