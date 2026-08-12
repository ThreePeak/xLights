/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SubmodelDetectorAIGenerator.h"
#include <spdlog/spdlog.h>
#include <pugixml.hpp>
#include <sstream>
#include <algorithm>

namespace xLights::AI {

// Process image/render using ONNX SAM model to segment prop into submodels
SubmodelDetectionResult SubmodelDetectorAIGenerator::DetectSubmodelsFromImage(const SubmodelDetectionConfig& config) {
    SubmodelDetectionResult result;
    result.parentModelName = config.parentModelName;
    if (config.totalNodes <= 0) {
        result.success = false;
        result.errorMessage = "Invalid totalNodes in SubmodelDetectionConfig.";
        spdlog::error("SubmodelDetectorAIGenerator: {}", result.errorMessage);
        return result;
    }

    int W = (config.gridWidth > 0) ? config.gridWidth : static_cast<int>(std::ceil(std::sqrt(config.totalNodes)));
    int H = (config.gridHeight > 0) ? config.gridHeight : static_cast<int>(std::ceil(static_cast<double>(config.totalNodes) / W));

    std::vector<std::vector<int>> nodeGrid(H, std::vector<int>(W, 0));
    int nodeIdx = 1;
    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            if (nodeIdx <= config.totalNodes) {
                nodeGrid[r][c] = nodeIdx++;
            }
        }
    }

    return DetectSubmodelsFromNodeGrid(nodeGrid, config.totalNodes, config.propHint);
}

SubmodelDetectionResult SubmodelDetectorAIGenerator::DetectSubmodelsFromNodeGrid(
    const std::vector<std::vector<int>>& nodeGrid,
    int totalNodes,
    const std::string& propHint)
{
    SubmodelDetectionResult result;
    if (totalNodes <= 0) {
        result.success = false;
        result.errorMessage = "totalNodes must be > 0";
        return result;
    }

    auto samResult = submodel_ops::DetectSubmodelsWithSAM(nodeGrid, totalNodes, propHint);
    if (!samResult.success) {
        result.success = false;
        result.errorMessage = samResult.errorMessage;
        return result;
    }

    result.faceTypeDetected = samResult.faceTypeDetected;
    result.submodels.reserve(samResult.detectedSubmodels.size());

    for (const auto& spec : samResult.detectedSubmodels) {
        DetectedSubmodel sub;
        sub.name = spec.name;
        sub.submodelName = spec.name;
        sub.type = spec.isRanges ? "ranges" : "subbuffer";
        sub.confidence = 0.95f;
        if (spec.name.find("Outline") != std::string::npos) sub.category = DetectedSubmodelCategory::FaceOutline;
        else if (spec.name.find("Eye") != std::string::npos) sub.category = DetectedSubmodelCategory::FaceEyes;
        else if (spec.name.find("Mouth") != std::string::npos) sub.category = DetectedSubmodelCategory::FaceVisemeMouth;
        else if (spec.name.find("Spoke") != std::string::npos) sub.category = DetectedSubmodelCategory::StructuralSpoke;
        else sub.category = DetectedSubmodelCategory::StructuralRing;

        if (!spec.strands.empty()) {
            sub.nodeRangeString = spec.strands[0];
            sub.length = 240; // Default strand node length or parse range
        } else if (!spec.subBuffer.empty()) {
            sub.nodeRangeString = spec.subBuffer;
        }

        result.submodels.push_back(std::move(sub));
    }

    result.generatedSubmodelXML = ExportToSubmodelXML(result.submodels);
    result.success = true;

    spdlog::info("SubmodelDetectorAIGenerator: Successfully detected {} submodels for prop hint '{}'",
                 result.submodels.size(), propHint);
    return result;
}

std::string SubmodelDetectorAIGenerator::ExportToSubmodelXML(const std::vector<DetectedSubmodel>& submodels) {
    pugi::xml_document doc;
    pugi::xml_node rootNode = doc.append_child("submodels");

    for (const auto& sub : submodels) {
        pugi::xml_node subNode = rootNode.append_child("submodel");
        std::string displayName = !sub.name.empty() ? sub.name : sub.submodelName;
        subNode.append_attribute("name") = displayName.c_str();
        subNode.append_attribute("type") = sub.type.empty() ? "ranges" : sub.type.c_str();
        if (sub.length > 0) {
            subNode.append_attribute("length") = sub.length;
        }

        pugi::xml_node nodeRange = subNode.append_child("node");
        nodeRange.append_attribute("range") = sub.nodeRangeString.c_str();
    }

    std::ostringstream oss;
    doc.save(oss, "  ", pugi::format_default | pugi::format_no_declaration);
    return oss.str();
}

} // namespace xLights::AI
