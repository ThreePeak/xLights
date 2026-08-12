#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AISubsystemBase.h"
#include "../src-core/models/SubModelOps.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <future>

namespace xLights::AI {

enum class DetectedSubmodelCategory {
    StructuralRing,
    StructuralSpoke,
    FaceOutline,
    FaceEyes,
    FaceVisemeMouth,
    CustomCluster,
    RADIAL_SPINNER,
    SINGING_MOUTH,
    OUTER_PERIMETER,
    CUSTOM_CLUSTER,
    INNER_CORE
};

struct SubmodelDetectionConfig {
    std::string parentModelName; // Target xLights model name
    std::string imagePath;
    std::string propHint; // e.g., "Singing Face", "Tree", "Star", "Arch"
    int totalNodes = 0;
    int gridWidth = 0;
    int gridHeight = 0;
    std::vector<std::pair<float, float>> pixelCoordinates; // Explicit 2D pixel coordinates (x, y)
};

struct DetectedSubmodel {
    std::string name;
    std::string submodelName;    // Alias for name
    std::string type = "ranges"; // "ranges" or "subbuffer"
    int length = 0;              // Total node count in this submodel
    std::string nodeRangeString; // e.g., "1-48"
    std::vector<int> nodeIndices;
    std::string submodelType;   // e.g., "Outline", "Eyes Open", "Eyes Closed", "Mouth AI", "Ring"
    DetectedSubmodelCategory category = DetectedSubmodelCategory::StructuralRing;
    float confidence = 0.0f;
};

struct DetectedSubmodelGroup {
    std::string groupName;
    DetectedSubmodelCategory category = DetectedSubmodelCategory::StructuralRing;
    std::vector<DetectedSubmodel> submodels;
    int totalNodesInGroup = 0;
    float confidence = 0.0f;
};

struct SubmodelDetectionResult {
    bool success = false;
    std::string errorMessage;
    std::string parentModelName; // Target parent model name
    std::string faceTypeDetected;
    std::vector<DetectedSubmodel> submodels;
    std::vector<DetectedSubmodelGroup> submodelGroups;
    std::vector<DetectedSubmodelGroup> detectedSubmodels; // Alias for submodelGroups
    std::string generatedSubmodelXML; // XML string containing <submodel> and <node range="1-48"/> elements
};

/**
 * @brief Feature #10: Automated Submodel & Face Detector (SAM Integration)
 * Auto-segments custom prop node grids into structural submodels (Rings, Spokes, Clusters)
 * and Singing Face components (Outline, Eyes Open, Eyes Closed, and 8 Mouth viseme states).
 */
class SubmodelDetectorAIGenerator : public AISubsystemBase {
public:
    SubmodelDetectorAIGenerator(ServiceManager* sm = nullptr) : AISubsystemBase(sm) {}
    virtual ~SubmodelDetectorAIGenerator() override = default;

    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override {
        return std::async(std::launch::async, [this, callback]() {
            if (callback) callback("Initializing SubmodelDetectorAIGenerator...", 0.0f);
            m_isInitialized.store(true);
            if (callback) callback("SubmodelDetectorAIGenerator initialized.", 100.0f);
            return true;
        });
    }

    virtual void Shutdown() override {
        m_isInitialized.store(false);
    }

    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "SubmodelDetectorAIGenerator"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"sam_submodel_detection", "singing_face_viseme_segmentation", "submodel_xml_export"};
    }

    // Process image/render using ONNX SAM model to segment prop into submodels
    [[nodiscard]] static SubmodelDetectionResult DetectSubmodelsFromImage(const SubmodelDetectionConfig& config);

    // Auto-detect structural submodels from a 2D node grid
    [[nodiscard]] static SubmodelDetectionResult DetectSubmodelsFromNodeGrid(
        const std::vector<std::vector<int>>& nodeGrid,
        int totalNodes,
        const std::string& propHint = "");

    // Export detected submodels into xLights <submodels> XML format with <node range="..."/> tags
    [[nodiscard]] static std::string ExportToSubmodelXML(const std::vector<DetectedSubmodel>& submodels);
    [[nodiscard]] static std::string ExportToSubmodelXML(const std::vector<DetectedSubmodelGroup>& submodels);
};

} // namespace xLights::AI
