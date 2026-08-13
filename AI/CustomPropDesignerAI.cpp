#include "CustomPropDesignerAI.h"
#include "AI/AIConfigurationManager.h"
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <numbers>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace xLights::AI {

std::vector<PropNodeSuggestion> CustomPropDesignerAI::GenerateNodeLayout(const std::string& propDescription, int nodeCount) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto cfg = AIConfigurationManager::Instance().GetSettings();
    spdlog::info("CustomPropDesignerAI: generating {} nodes, model={}", nodeCount, cfg.primaryModel);
    
    std::vector<PropNodeSuggestion> nodes;
    for (int i = 0; i < nodeCount; ++i) {
        PropNodeSuggestion node;
        node.x = 50.0f + 40.0f * std::cos(2.0f * M_PI * i / nodeCount);
        node.y = 50.0f + 40.0f * std::sin(2.0f * M_PI * i / nodeCount);
        node.channelIndex = i * 3 + 1;
        node.label = "Node_" + std::to_string(i + 1);
        nodes.push_back(node);
    }
    return nodes;
}

std::string CustomPropDesignerAI::ExportToXLightsModelXML(const std::vector<PropNodeSuggestion>& nodes, const std::string& modelName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::ostringstream oss;
    oss << "<custommodel name=\"" << modelName << "\" CustomModel=\"";
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        oss << nodes[i].channelIndex;
        if (i < nodes.size() - 1) {
            oss << ",";
        }
    }
    
    oss << "\" LayoutGroup=\"Default\" parm1=\"" << nodes.size() 
        << "\" parm2=\"1\" StringType=\"RGB Nodes\" Transparency=\"0\" />";
        
    return oss.str();
}

} // namespace xLights::AI
