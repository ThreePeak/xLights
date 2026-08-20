#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// SubmodelDetector.h — Facade header for Feature #10 (SAM Submodel Detector)

#include "SubmodelDetectorAIGenerator.h"

namespace xLights::AI {

class SubmodelDetectorUtils {
public:
    // Apply geometric clustering (DBSCAN) to group node coordinates into spatial clusters
    static std::vector<std::vector<int>> ClusterNodesDBSCAN(
        const std::vector<std::pair<float, float>>& points,
        float eps = 0.15f,
        int minPts = 3);

    // Apply Radial K-Means clustering to group nodes into concentric rings / spokes
    static std::vector<std::vector<int>> ClusterNodesRadialKMeans(
        const std::vector<std::pair<float, float>>& points,
        int numClusters = 3);
};

class SubmodelDetector : public AISubsystemBase {
public:
    SubmodelDetector(ServiceManager* sm = nullptr) : AISubsystemBase(sm) {}
    virtual ~SubmodelDetector() override = default;

    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override {
        return SubmodelDetectorAIGenerator(m_serviceManager).InitializeAsync(callback);
    }

    virtual void Shutdown() override {
        m_isInitialized.store(false);
    }

    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "SubmodelDetector"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"sam_submodel_detection", "singing_face_viseme_segmentation", "submodel_xml_export"};
    }

    // Process image/render using ONNX SAM model to segment prop into submodels
    [[nodiscard]] static SubmodelDetectionResult DetectSubmodelsFromImage(const SubmodelDetectionConfig& config) {
        return SubmodelDetectorAIGenerator::DetectSubmodelsFromImage(config);
    }

    // Auto-detect structural submodels from a 2D node grid
    [[nodiscard]] static SubmodelDetectionResult DetectSubmodelsFromNodeGrid(
        const std::vector<std::vector<int>>& nodeGrid,
        int totalNodes,
        const std::string& propHint = "") {
        return SubmodelDetectorAIGenerator::DetectSubmodelsFromNodeGrid(nodeGrid, totalNodes, propHint);
    }

    // Export detected submodels into xLights <submodels> XML format with <node range="..."/> tags
    [[nodiscard]] static std::string ExportToSubmodelXML(const std::vector<DetectedSubmodel>& submodels) {
        return SubmodelDetectorAIGenerator::ExportToSubmodelXML(submodels);
    }

    [[nodiscard]] static std::string ExportToSubmodelXML(const std::vector<DetectedSubmodelGroup>& submodels) {
        return SubmodelDetectorAIGenerator::ExportToSubmodelXML(submodels);
    }
};

} // namespace xLights::AI
