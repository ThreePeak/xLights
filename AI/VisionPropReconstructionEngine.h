/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <string>
#include <vector>

namespace xLights::AI {

struct ReconstructedPropMesh {
    std::string propName;
    std::string modelType; // "PolyLine", "Matrix", "Custom"
    std::vector<float> pointCloudX;
    std::vector<float> pointCloudY;
    std::vector<float> pointCloudZ;
    std::string generatedModelXml;
};

struct VisionReconstructionResult {
    bool success = false;
    int detectedPropsCount = 0;
    std::vector<ReconstructedPropMesh> props;
    std::string fullLayoutXml;
    std::string errorMessage;
};

class VisionPropReconstructionEngine {
public:
    VisionPropReconstructionEngine() = default;
    ~VisionPropReconstructionEngine() = default;

    [[nodiscard]] static VisionReconstructionResult Reconstruct3DLayoutFromVideo(
        const std::string& videoPath,
        float depthConfidenceThreshold = 0.80f
    );
};

} // namespace xLights::AI
