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

using SubmodelDetector = SubmodelDetectorAIGenerator;

} // namespace xLights::AI
