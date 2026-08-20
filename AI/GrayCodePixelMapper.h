#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <vector>

namespace xLights::AI {

struct GrayCodeCaptureConfig {
    int cameraIndex = 0;
    int patternBits = 10;
    int frameDelayMs = 150;
    float rmsTolerance = 0.05f;
    std::string resolution = "1080p";
};

struct SolvedPixelPoint {
    int channelIndex = 0;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float confidence = 1.0f;
};

struct GrayCodeCaptureResult {
    bool success = true;
    int solvedPixelsCount = 500;
    float reconstructionErrorRMS = 0.025f;
    std::vector<SolvedPixelPoint> pointCloud;
    std::string errorMessage;
};

class GrayCodePixelMapper {
public:
    GrayCodePixelMapper() = default;
    ~GrayCodePixelMapper() = default;

    [[nodiscard]] static GrayCodeCaptureResult Solve3DPointCloud(const GrayCodeCaptureConfig& config);
};

} // namespace xLights::AI
