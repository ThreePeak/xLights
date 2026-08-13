/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <catch2/catch_test_macros.hpp>
#include "AI/GrayCodePixelMapper.h"

TEST_CASE("AI 3D Pixel Map Camera Solver Backend Validation Tests", "[AI][Camera]") {
    SECTION("10-bit Gray Code Frame Capture & 3D Point Cloud Solver") {
        xLights::AI::GrayCodeCaptureConfig config;
        config.cameraIndex = 0;
        config.patternBits = 10;

        xLights::AI::GrayCodeCaptureResult result = xLights::AI::GrayCodePixelMapper::Solve3DPointCloud(config);

        REQUIRE(result.solvedPixelsCount > 0);
        REQUIRE(result.reconstructionErrorRMS < 0.05f);
    }
}
