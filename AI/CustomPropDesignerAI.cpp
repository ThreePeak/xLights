#include "CustomPropDesignerAI.h"
#include "AI/AIConfigurationManager.h"
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <regex>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace xLights::AI {

static std::string ToLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower;
}

AIPropAnalysisResult CustomPropDesignerAI::AnalyzePrompt(const std::string& description, int fallbackNodeCount) {
    std::lock_guard<std::mutex> lock(m_mutex);
    AIPropAnalysisResult result;
    PropGenerationSpec& spec = result.spec;
    spec.totalNodes = (fallbackNodeCount > 0) ? fallbackNodeCount : 100;
    
    std::string text = ToLower(description);
    
    // 1. Detect Prop Category
    if (text.find("mini tree") != std::string::npos || text.find("minitree") != std::string::npos) {
        spec.propType = "Mini Tree";
        spec.arcDegrees = 240.0f;
        spec.heightInches = 30.0f;
        spec.baseDiameterInches = 18.0f;
        spec.topDiameterInches = 3.5f;
    } else if (text.find("mega tree") != std::string::npos || text.find("megatree") != std::string::npos || text.find("tree") != std::string::npos) {
        spec.propType = "Mega Tree";
        spec.arcDegrees = 270.0f;
        spec.heightInches = 120.0f;
        spec.baseDiameterInches = 60.0f;
        spec.topDiameterInches = 10.0f;
    } else if (text.find("star") != std::string::npos && text.find("tree") == std::string::npos) {
        spec.propType = "Star";
        spec.heightInches = 24.0f;
        spec.baseDiameterInches = 24.0f;
        spec.hasTopper = false;
    } else if (text.find("matrix") != std::string::npos || text.find("grid") != std::string::npos || text.find("panel") != std::string::npos) {
        spec.propType = "Matrix";
        spec.hasTopper = false;
    } else if (text.find("arch") != std::string::npos) {
        spec.propType = "Arch";
        spec.hasTopper = false;
    } else if (text.find("candy cane") != std::string::npos || text.find("cane") != std::string::npos) {
        spec.propType = "Candy Cane";
        spec.heightInches = 36.0f;
        spec.hasTopper = false;
    } else if (text.find("snowflake") != std::string::npos || text.find("snow") != std::string::npos) {
        spec.propType = "Snowflake";
        spec.heightInches = 24.0f;
        spec.hasTopper = false;
    } else if (text.find("wreath") != std::string::npos || text.find("ring") != std::string::npos) {
        spec.propType = "Wreath";
        spec.heightInches = 30.0f;
        spec.baseDiameterInches = 30.0f;
        spec.hasTopper = false;
    } else {
        spec.propType = "Custom";
    }
    
    // 2. Regex Extraction
    std::smatch m;
    
    // Body nodes: "80 nodes on the body" or "body: 80"
    std::regex bodyRegex(R"((\d+)\s*(?:nodes?\s*(?:on|for|in)?\s*(?:the)?\s*body|body\s*nodes?))");
    if (std::regex_search(text, m, bodyRegex)) {
        spec.bodyNodes = std::stoi(m[1].str());
    }
    
    // Star / Topper nodes: "20 for a top star" or "20 star nodes"
    std::regex starRegex(R"((\d+)\s*(?:nodes?\s*)?(?:for|on)?\s*(?:a|the)?\s*(?:top\s*)?star)");
    if (std::regex_search(text, m, starRegex)) {
        spec.topperNodes = std::stoi(m[1].str());
        spec.hasTopper = true;
        spec.topperType = "Star";
    } else if (text.find("star") != std::string::npos && spec.propType.find("Tree") != std::string::npos) {
        spec.hasTopper = true;
        spec.topperNodes = 20;
    } else if (text.find("no star") != std::string::npos || text.find("without star") != std::string::npos) {
        spec.hasTopper = false;
        spec.topperNodes = 0;
    }
    
    // Columns / Strands: "8 vertical columns" or "8 strands"
    std::regex colRegex(R"((\d+)\s*(?:vertical\s*)?(?:columns?|strands?|ribs?|lines?|strings?))");
    if (std::regex_search(text, m, colRegex)) {
        spec.columns = std::stoi(m[1].str());
    }
    
    // Nodes per column: "10 nodes each" or "10 nodes per strand"
    std::regex perColRegex(R"((\d+)\s*nodes?\s*(?:each|per\s*(?:column|strand|string|rib)))");
    if (std::regex_search(text, m, perColRegex)) {
        spec.nodesPerColumn = std::stoi(m[1].str());
    }
    
    // Arc / Degrees / Coverage: "240 degree" or "240 deg" or "240°"
    std::regex arcRegex(R"((\d+)\s*(?:degrees?|deg|°))");
    if (std::regex_search(text, m, arcRegex)) {
        spec.arcDegrees = std::stof(m[1].str());
    } else if (text.find("back left open") != std::string::npos || text.find("open back") != std::string::npos) {
        spec.arcDegrees = 240.0f;
    } else if (text.find("flat") != std::string::npos || text.find("half") != std::string::npos || text.find("180") != std::string::npos) {
        spec.arcDegrees = 180.0f;
    } else if (text.find("360") != std::string::npos || text.find("round") != std::string::npos || text.find("full") != std::string::npos) {
        spec.arcDegrees = 360.0f;
    }
    
    // Height: "30\" high" or "30 inch" or "30 in"
    std::regex heightRegex(R"((\d+(?:\.\d+)?)\s*(\"|inches|inch|in\b|feet|ft\b|\'|cm|m\b))");
    if (std::regex_search(text, m, heightRegex)) {
        float val = std::stof(m[1].str());
        std::string unit = m[2].str();
        if (unit == "ft" || unit == "feet" || unit == "'") {
            val *= 12.0f; // convert feet to inches
        } else if (unit == "cm") {
            val /= 2.54f;
        } else if (unit == "m") {
            val *= 39.37f;
        }
        spec.heightInches = val;
    }
    
    // Base width / diameter: "18\" wide" or "base 18 inch"
    std::regex widthRegex(R"((\d+(?:\.\d+)?)\s*(?:\"|in|inch|inches)\s*(?:wide|width|base|diameter))");
    if (std::regex_search(text, m, widthRegex)) {
        spec.baseDiameterInches = std::stof(m[1].str());
    } else {
        // Auto proportion base diameter ~ 60% of height for mini tree
        if (spec.propType == "Mini Tree") {
            spec.baseDiameterInches = spec.heightInches * 0.60f;
        }
    }
    
    // Wiring direction: "zigzag" / "zig-zag" / "up and down" / "top to bottom"
    if (text.find("zigzag") != std::string::npos || text.find("zig-zag") != std::string::npos || text.find("up and down") != std::string::npos) {
        spec.wiringDirection = "ZigZag";
    } else if (text.find("top to bottom") != std::string::npos || text.find("downward") != std::string::npos) {
        spec.wiringDirection = "TopToBottom";
    } else {
        spec.wiringDirection = "BottomToTop";
    }
    
    // Consistency reconciliation
    if (spec.columns > 0 && spec.nodesPerColumn > 0) {
        spec.bodyNodes = spec.columns * spec.nodesPerColumn;
    } else if (spec.columns > 0 && spec.bodyNodes > 0) {
        spec.nodesPerColumn = std::max(1, spec.bodyNodes / spec.columns);
    } else if (spec.nodesPerColumn > 0 && spec.bodyNodes > 0) {
        spec.columns = std::max(1, spec.bodyNodes / spec.nodesPerColumn);
    }
    
    if (spec.bodyNodes > 0 || spec.topperNodes > 0) {
        spec.totalNodes = spec.bodyNodes + (spec.hasTopper ? spec.topperNodes : 0);
    }
    
    // Calculate node pitch spacing
    if (spec.nodesPerColumn > 1 && spec.heightInches > 0.0f) {
        spec.calculatedPixelPitchInches = spec.heightInches / (spec.nodesPerColumn - 1);
    } else {
        spec.calculatedPixelPitchInches = 3.0f;
    }
    
    // 3. Build Executive Summary
    std::ostringstream summary;
    summary << "Will generate a " << std::fixed << std::setprecision(1) << spec.heightInches << "\" tall 3D " 
            << spec.propType << " with " << spec.bodyNodes << " body nodes arranged in "
            << spec.columns << " vertical columns (" << spec.nodesPerColumn << " nodes/col, ~" 
            << std::fixed << std::setprecision(1) << spec.calculatedPixelPitchInches << "\" pixel pitch)";
            
    if (spec.arcDegrees < 360.0f) {
        summary << " spanning a " << (int)spec.arcDegrees << "° cone arc with the rear open";
    } else {
        summary << " spanning a full 360° round cone";
    }
    
    if (spec.hasTopper && spec.topperNodes > 0) {
        summary << ", topped with a " << spec.topperNodes << "-node " << spec.starPoints << "-point " << spec.topperType 
                << " (Total: " << spec.totalNodes << " nodes, " << (spec.columns + 1) << " submodels).";
    } else {
        summary << " (Total: " << spec.totalNodes << " nodes, " << spec.columns << " submodels).";
    }
    result.executiveSummary = summary.str();
    
    // 4. Build Clarifications / Diagnostic Recommendations
    result.clarifications.clear();
    if (spec.propType == "Mini Tree" || spec.propType == "Mega Tree") {
        result.clarifications.push_back("Arc coverage set to " + std::to_string((int)spec.arcDegrees) + "° with back open (rear " + std::to_string((int)(360.0f - spec.arcDegrees)) + "° opening).");
        result.clarifications.push_back("Calculated pixel center-to-center pitch is ~" + std::to_string(spec.calculatedPixelPitchInches).substr(0, 3) + "\" (~" + std::to_string((int)(spec.calculatedPixelPitchInches * 25.4f)) + "mm).");
        result.clarifications.push_back("Base diameter proportioned to " + std::to_string(spec.baseDiameterInches).substr(0, 4) + "\" tapering to " + std::to_string(spec.topDiameterInches).substr(0, 3) + "\" top apex collar.");
        if (spec.hasTopper) {
            result.clarifications.push_back("Top star topper configured with " + std::to_string(spec.topperNodes) + " nodes across " + std::to_string(spec.starPoints) + " points (" + std::to_string((int)spec.topperDiameterInches) + "\" diameter).");
        }
        result.clarifications.push_back("Wiring direction: " + spec.wiringDirection + " (can be changed in the Tree parameter panel).");
    } else {
        result.clarifications.push_back("Analyzed as " + spec.propType + " with " + std::to_string(spec.totalNodes) + " nodes.");
    }
    
    // 5. Build Tags
    result.tags.push_back("#" + spec.propType);
    result.tags.push_back("#" + std::to_string(spec.totalNodes) + "Nodes");
    result.tags.push_back("#" + std::to_string(spec.columns) + "Strands");
    result.tags.push_back("#" + std::to_string((int)spec.arcDegrees) + "DegArc");
    result.tags.push_back("#" + std::to_string((int)spec.heightInches) + "InchHeight");
    if (spec.hasTopper) result.tags.push_back("#StarTopper");
    
    result.confidenceScore = 0.96f;
    result.isValid = true;
    
    spdlog::info("AIPropAnalyzer: {}", result.executiveSummary);
    return result;
}

std::vector<PropNodeSuggestion> CustomPropDesignerAI::GenerateFromSpec(const PropGenerationSpec& spec) {
    if (spec.propType == "Mini Tree" || spec.propType == "Mega Tree") {
        return GenerateTreeLayout(spec);
    } else if (spec.propType == "Star") {
        return GenerateStarLayout(spec.totalNodes, 50.0f, 50.0f, 0.0f, 25.0f, spec.starPoints, spec.startChannel, "Star");
    } else if (spec.propType == "Matrix") {
        return GenerateMatrixLayout(spec);
    } else if (spec.propType == "Arch") {
        return GenerateArchLayout(spec);
    } else if (spec.propType == "Candy Cane") {
        return GenerateCandyCaneLayout(spec);
    } else if (spec.propType == "Snowflake") {
        return GenerateSnowflakeLayout(spec);
    } else if (spec.propType == "Wreath") {
        return GenerateWreathLayout(spec);
    }
    
    return GenerateCircleLayout(spec.totalNodes, spec.startChannel);
}

std::vector<PropNodeSuggestion> CustomPropDesignerAI::GenerateTreeLayout(const PropGenerationSpec& spec) {
    std::vector<PropNodeSuggestion> nodes;
    int channelIdx = spec.startChannel;
    
    int cols = std::max(2, spec.columns);
    int nodesPerCol = std::max(2, spec.nodesPerColumn);
    float arcDeg = (spec.arcDegrees > 0.0f) ? spec.arcDegrees : 240.0f;
    
    float rBase = 22.0f;
    float rTop = 3.5f;
    float baseY = 12.0f;
    float treeHeight = spec.hasTopper ? 56.0f : 74.0f;
    
    float startAngleDeg = -arcDeg / 2.0f;
    float angleStepDeg = (cols > 1) ? (arcDeg / (cols - 1)) : 0.0f;
    if (std::abs(arcDeg - 360.0f) < 1.0f) {
        angleStepDeg = 360.0f / cols;
    }
    
    // Generate Tree Body Columns
    for (int c = 0; c < cols; ++c) {
        float angleDeg = startAngleDeg + c * angleStepDeg;
        float angleRad = angleDeg * (float)M_PI / 180.0f;
        std::string strandGroup = "Tree_Strand_" + std::to_string(c + 1);
        
        bool isReversed = (spec.wiringDirection == "TopToBottom") || 
                          (spec.wiringDirection == "ZigZag" && (c % 2 == 1));
        
        for (int r = 0; r < nodesPerCol; ++r) {
            int actualRow = isReversed ? (nodesPerCol - 1 - r) : r;
            float t = (float)actualRow / (float)(nodesPerCol - 1); // 0 = base, 1 = top
            float radius = rBase * (1.0f - t) + rTop * t;
            
            PropNodeSuggestion node;
            node.x = 50.0f + radius * std::sin(angleRad);
            node.z = radius * std::cos(angleRad); // 3D depth
            node.y = baseY + t * treeHeight;
            node.channelIndex = channelIdx++;
            node.label = "Col" + std::to_string(c + 1) + "_N" + std::to_string(actualRow + 1);
            node.group = strandGroup;
            nodes.push_back(node);
        }
    }
    
    // Generate Top Star Topper
    if (spec.hasTopper && spec.topperNodes > 0) {
        float starCenterY = baseY + treeHeight + 10.0f;
        auto starNodes = GenerateStarLayout(spec.topperNodes, 50.0f, starCenterY, 0.0f, 9.0f, spec.starPoints, channelIdx, "Top_Star");
        for (auto& sn : starNodes) {
            nodes.push_back(sn);
        }
    }
    
    return nodes;
}

std::vector<PropNodeSuggestion> CustomPropDesignerAI::GenerateStarLayout(int nodeCount, float cx, float cy, float cz, float radius, int points, int startChannel, const std::string& group) {
    std::vector<PropNodeSuggestion> nodes;
    if (nodeCount <= 0) return nodes;
    
    int numPoints = (points >= 3) ? points : 5;
    float rOuter = radius;
    float rInner = radius * 0.42f;
    
    // Construct 2*numPoints star vertices (alternating outer and inner)
    struct Pt2D { float x, y; };
    std::vector<Pt2D> vertices;
    for (int i = 0; i < numPoints * 2; ++i) {
        float angDeg = 90.0f + i * (360.0f / (numPoints * 2));
        float angRad = angDeg * (float)M_PI / 180.0f;
        float r = (i % 2 == 0) ? rOuter : rInner;
        vertices.push_back({ cx + r * std::cos(angRad), cy + r * std::sin(angRad) });
    }
    
    // Distribute nodeCount evenly along the outer edges
    int numEdges = (int)vertices.size();
    for (int i = 0; i < nodeCount; ++i) {
        float edgeProgress = (float)i / (float)nodeCount * (float)numEdges;
        int edgeIdx = (int)std::floor(edgeProgress) % numEdges;
        float t = edgeProgress - std::floor(edgeProgress);
        
        Pt2D p1 = vertices[edgeIdx];
        Pt2D p2 = vertices[(edgeIdx + 1) % numEdges];
        
        PropNodeSuggestion node;
        node.x = p1.x + (p2.x - p1.x) * t;
        node.y = p1.y + (p2.y - p1.y) * t;
        node.z = cz;
        node.channelIndex = startChannel + i;
        node.label = "Star_" + std::to_string(i + 1);
        node.group = group;
        nodes.push_back(node);
    }
    
    return nodes;
}

std::vector<PropNodeSuggestion> CustomPropDesignerAI::GenerateMatrixLayout(const PropGenerationSpec& spec) {
    std::vector<PropNodeSuggestion> nodes;
    int width = spec.columns > 0 ? spec.columns : 10;
    int height = spec.nodesPerColumn > 0 ? spec.nodesPerColumn : (spec.totalNodes / width);
    width = std::max(1, width);
    height = std::max(1, height);
    
    float marginX = 15.0f, marginY = 15.0f;
    float stepX = (70.0f) / std::max(1, width - 1);
    float stepY = (70.0f) / std::max(1, height - 1);
    
    int ch = spec.startChannel;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int actualX = (spec.wiringDirection == "ZigZag" && (y % 2 == 1)) ? (width - 1 - x) : x;
            PropNodeSuggestion node;
            node.x = marginX + actualX * stepX;
            node.y = marginY + y * stepY;
            node.z = 0.0f;
            node.channelIndex = ch++;
            node.label = "R" + std::to_string(y + 1) + "C" + std::to_string(actualX + 1);
            node.group = "Matrix_Row_" + std::to_string(y + 1);
            nodes.push_back(node);
        }
    }
    return nodes;
}

std::vector<PropNodeSuggestion> CustomPropDesignerAI::GenerateArchLayout(const PropGenerationSpec& spec) {
    std::vector<PropNodeSuggestion> nodes;
    int arches = spec.columns > 0 ? spec.columns : 1;
    int nodesPerArch = spec.nodesPerColumn > 0 ? spec.nodesPerColumn : (spec.totalNodes / arches);
    arches = std::max(1, arches);
    nodesPerArch = std::max(2, nodesPerArch);
    
    int ch = spec.startChannel;
    for (int a = 0; a < arches; ++a) {
        float r = 25.0f + a * 6.0f;
        std::string archGroup = "Arch_" + std::to_string(a + 1);
        for (int i = 0; i < nodesPerArch; ++i) {
            float ang = (float)M_PI * (1.0f - (float)i / (float)(nodesPerArch - 1));
            PropNodeSuggestion node;
            node.x = 50.0f + r * std::cos(ang);
            node.y = 20.0f + r * std::sin(ang);
            node.z = (float)a * 4.0f;
            node.channelIndex = ch++;
            node.label = "Arch" + std::to_string(a + 1) + "_N" + std::to_string(i + 1);
            node.group = archGroup;
            nodes.push_back(node);
        }
    }
    return nodes;
}

std::vector<PropNodeSuggestion> CustomPropDesignerAI::GenerateCandyCaneLayout(const PropGenerationSpec& spec) {
    std::vector<PropNodeSuggestion> nodes;
    int total = spec.totalNodes > 0 ? spec.totalNodes : 100;
    int straightNodes = total * 0.65f;
    int hookNodes = total - straightNodes;
    
    int ch = spec.startChannel;
    for (int i = 0; i < straightNodes; ++i) {
        PropNodeSuggestion node;
        node.x = 42.0f;
        node.y = 15.0f + (float)i / (float)straightNodes * 50.0f;
        node.z = 0.0f;
        node.channelIndex = ch++;
        node.label = "Stem_" + std::to_string(i + 1);
        node.group = "Cane_Stem";
        nodes.push_back(node);
    }
    float hookRadius = 12.0f;
    float hookCenterY = 65.0f;
    for (int i = 0; i < hookNodes; ++i) {
        float ang = (float)M_PI * (1.0f - (float)i / (float)hookNodes);
        PropNodeSuggestion node;
        node.x = 42.0f + hookRadius * (1.0f - std::cos(ang));
        node.y = hookCenterY + hookRadius * std::sin(ang);
        node.z = 0.0f;
        node.channelIndex = ch++;
        node.label = "Hook_" + std::to_string(i + 1);
        node.group = "Cane_Hook";
        nodes.push_back(node);
    }
    return nodes;
}

std::vector<PropNodeSuggestion> CustomPropDesignerAI::GenerateSnowflakeLayout(const PropGenerationSpec& spec) {
    std::vector<PropNodeSuggestion> nodes;
    int arms = spec.columns > 0 ? spec.columns : 6;
    int nodesPerArm = std::max(1, spec.totalNodes / arms);
    int ch = spec.startChannel;
    
    for (int a = 0; a < arms; ++a) {
        float angRad = a * ((360.0f / arms) * (float)M_PI / 180.0f);
        std::string armGroup = "Arm_" + std::to_string(a + 1);
        for (int i = 0; i < nodesPerArm; ++i) {
            float dist = 6.0f + (float)i / (float)nodesPerArm * 35.0f;
            PropNodeSuggestion node;
            node.x = 50.0f + dist * std::cos(angRad);
            node.y = 50.0f + dist * std::sin(angRad);
            node.z = 0.0f;
            node.channelIndex = ch++;
            node.label = "Arm" + std::to_string(a + 1) + "_N" + std::to_string(i + 1);
            node.group = armGroup;
            nodes.push_back(node);
        }
    }
    return nodes;
}

std::vector<PropNodeSuggestion> CustomPropDesignerAI::GenerateWreathLayout(const PropGenerationSpec& spec) {
    std::vector<PropNodeSuggestion> nodes;
    float radius = 35.0f;
    int ch = spec.startChannel;
    for (int i = 0; i < spec.totalNodes; ++i) {
        float ang = 2.0f * (float)M_PI * (float)i / (float)spec.totalNodes;
        PropNodeSuggestion node;
        node.x = 50.0f + radius * std::cos(ang);
        node.y = 50.0f + radius * std::sin(ang);
        node.z = 0.0f;
        node.channelIndex = ch++;
        node.label = "Ring_" + std::to_string(i + 1);
        node.group = "Wreath_Ring";
        nodes.push_back(node);
    }
    return nodes;
}

std::vector<PropNodeSuggestion> CustomPropDesignerAI::GenerateCircleLayout(int nodeCount, int startChannel) {
    std::vector<PropNodeSuggestion> nodes;
    for (int i = 0; i < nodeCount; ++i) {
        PropNodeSuggestion node;
        node.x = 50.0f + 40.0f * std::cos(2.0f * (float)M_PI * i / nodeCount);
        node.y = 50.0f + 40.0f * std::sin(2.0f * (float)M_PI * i / nodeCount);
        node.z = 0.0f;
        node.channelIndex = startChannel + i;
        node.label = "Node_" + std::to_string(i + 1);
        node.group = "Default";
        nodes.push_back(node);
    }
    return nodes;
}

std::vector<PropNodeSuggestion> CustomPropDesignerAI::GenerateNodeLayout(const std::string& propDescription, int nodeCount) {
    AIPropAnalysisResult res = AnalyzePrompt(propDescription, nodeCount);
    return GenerateFromSpec(res.spec);
}

std::string CustomPropDesignerAI::ExportToXLightsModelXML(const std::vector<PropNodeSuggestion>& nodes, const std::string& modelName, const PropGenerationSpec& spec) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::ostringstream oss;
    std::string safeName = modelName.empty() ? "CustomModel" : modelName;
    
    // Group analysis for submodel export
    std::map<std::string, std::vector<int>> groups;
    for (const auto& n : nodes) {
        if (!n.group.empty()) {
            groups[n.group].push_back(n.channelIndex);
        }
    }
    
    oss << "<custommodel name=\"" << safeName << "\" CustomModel=\"";
    for (size_t i = 0; i < nodes.size(); ++i) {
        oss << nodes[i].channelIndex;
        if (i < nodes.size() - 1) {
            oss << ",";
        }
    }
    oss << "\" LayoutGroup=\"Default\" parm1=\"" << nodes.size() 
        << "\" parm2=\"1\" StringType=\"RGB Nodes\" Transparency=\"0\" "
        << "HeightInches=\"" << spec.heightInches << "\" "
        << "WidthInches=\"" << spec.baseDiameterInches << "\" "
        << "ArcDegrees=\"" << spec.arcDegrees << "\">\n";
        
    // Add SubModels
    for (const auto& [grpName, chList] : groups) {
        oss << "  <subModel name=\"" << grpName << "\" layout=\"horizontal\" type=\"ranges\" lineRanges=\"";
        if (!chList.empty()) {
            oss << chList.front() << "-" << chList.back();
        }
        oss << "\" />\n";
    }
    
    oss << "</custommodel>";
    return oss.str();
}

} // namespace xLights::AI
