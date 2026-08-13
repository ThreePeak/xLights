#pragma once
#include <string>
#include <vector>
#include <mutex>

namespace xLights::AI {

struct PropNodeSuggestion {
    float x, y;
    int channelIndex;
    std::string label;
};

class CustomPropDesignerAI {
public:
    std::vector<PropNodeSuggestion> GenerateNodeLayout(const std::string& propDescription, int nodeCount);
    std::string ExportToXLightsModelXML(const std::vector<PropNodeSuggestion>& nodes, const std::string& modelName);
private:
    mutable std::mutex m_mutex;
};

} // namespace xLights::AI
