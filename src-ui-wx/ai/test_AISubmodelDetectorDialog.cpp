/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <catch2/catch_test_macros.hpp>
#include "AI/SubmodelDetector.h"

TEST_CASE("AI Submodel Vision Detector Backend Validation Tests", "[AI][Submodel]") {
    SECTION("SAM Vision ONNX & DBSCAN Spatial Clustering") {
        xLights::AI::SubmodelDetectionConfig config;
        config.totalNodes = 50;
        config.gridWidth = 10;
        config.gridHeight = 5;

        xLights::AI::SubmodelDetectionResult result = xLights::AI::SubmodelDetector::DetectSubmodelsFromImage(config);

        REQUIRE(result.success == true);
        REQUIRE(!result.submodels.empty());
    }
}
