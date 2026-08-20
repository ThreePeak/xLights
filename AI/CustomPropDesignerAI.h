#pragma once
#include <string>
#include <vector>
#include <mutex>

namespace xLights::AI {

struct PropNodeSuggestion {
    float x = 0.0f;           // 0..100 normalized or model space coordinates
    float y = 0.0f;           // 0..100 normalized or model space coordinates
    float z = 0.0f;           // 0..100 depth coordinate
    int channelIndex = 1;     // 1-based pixel / channel index
    std::string label;        // e.g. "Col1_Node1", "Star_1"
    std::string group;        // e.g. "Tree_Strand_1", "Top_Star", "Body"
};

struct PropGenerationSpec {
    std::string propType = "Mini Tree"; // "Mini Tree", "Mega Tree", "Star", "Matrix", "Arch", "Candy Cane", "Snowflake", "Wreath", "Custom"
    int totalNodes = 100;
    int bodyNodes = 80;
    int topperNodes = 20;
    int columns = 8;
    int nodesPerColumn = 10;
    float arcDegrees = 240.0f;
    float heightInches = 30.0f;
    float baseDiameterInches = 18.0f;
    float topDiameterInches = 3.5f;
    float calculatedPixelPitchInches = 3.1f;
    
    // Topper configuration
    bool hasTopper = true;
    std::string topperType = "Star"; // "Star", "Snowflake", "Ball"
    int starPoints = 5;
    float topperDiameterInches = 10.0f;
    float topperHeightOffsetInches = 2.0f;
    
    // Wiring configuration
    std::string wiringDirection = "BottomToTop"; // "BottomToTop", "TopToBottom", "ZigZag"
    int startChannel = 1;
};

struct AIPropAnalysisResult {
    PropGenerationSpec spec;
    std::string executiveSummary;
    std::vector<std::string> clarifications;
    std::vector<std::string> tags;
    float confidenceScore = 0.95f;
    bool isValid = true;
};

class CustomPropDesignerAI {
public:
    AIPropAnalysisResult AnalyzePrompt(const std::string& description, int fallbackNodeCount = 100);
    std::vector<PropNodeSuggestion> GenerateFromSpec(const PropGenerationSpec& spec);
    std::vector<PropNodeSuggestion> GenerateNodeLayout(const std::string& propDescription, int nodeCount);
    std::string ExportToXLightsModelXML(const std::vector<PropNodeSuggestion>& nodes, const std::string& modelName, const PropGenerationSpec& spec = {});

private:
    std::vector<PropNodeSuggestion> GenerateTreeLayout(const PropGenerationSpec& spec);
    std::vector<PropNodeSuggestion> GenerateStarLayout(int nodeCount, float cx, float cy, float cz, float radius, int points = 5, int startChannel = 1, const std::string& group = "Star");
    std::vector<PropNodeSuggestion> GenerateMatrixLayout(const PropGenerationSpec& spec);
    std::vector<PropNodeSuggestion> GenerateArchLayout(const PropGenerationSpec& spec);
    std::vector<PropNodeSuggestion> GenerateCandyCaneLayout(const PropGenerationSpec& spec);
    std::vector<PropNodeSuggestion> GenerateSnowflakeLayout(const PropGenerationSpec& spec);
    std::vector<PropNodeSuggestion> GenerateWreathLayout(const PropGenerationSpec& spec);
    std::vector<PropNodeSuggestion> GenerateCircleLayout(int nodeCount, int startChannel = 1);

    mutable std::mutex m_mutex;
};

} // namespace xLights::AI
