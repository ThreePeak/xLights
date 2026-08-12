/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #10 / #8 Catch2 Unit Tests for SubmodelDetector

#include <catch2/catch_test_macros.hpp>
#include "SubmodelDetector.h"
#include <cmath>

using namespace xLights::AI;

TEST_CASE("SubmodelDetector: SAM Vision Submodel Detection Tests", "[SubmodelDetector]") {

    SECTION("Detects submodels for singing face prop hint") {
        SubmodelDetectionConfig config;
        config.parentModelName = "SingingFace1";
        config.propHint = "Singing Face";
        config.totalNodes = 240;

        SubmodelDetectionResult result = SubmodelDetector::DetectSubmodelsFromImage(config);
        REQUIRE(result.success == true);
        REQUIRE(result.faceTypeDetected == "Singing Face 8-Viseme");
        REQUIRE(!result.submodels.empty());
        REQUIRE(result.generatedSubmodelXML.find("<submodels>") != std::string::npos);
        REQUIRE(result.generatedSubmodelXML.find("Outline") != std::string::npos);
    }

    SECTION("ClusterNodesDBSCAN clusters spatial point coordinates into submodel groups") {
        std::vector<std::pair<float, float>> points = {
            {0.1f, 0.1f}, {0.12f, 0.11f}, {0.11f, 0.09f},
            {0.9f, 0.9f}, {0.91f, 0.89f}, {0.89f, 0.92f}
        };
        auto clusters = SubmodelDetectorUtils::ClusterNodesDBSCAN(points, 0.2f, 2);
        REQUIRE(clusters.size() == 2);
    }

    SECTION("ClusterNodesRadialKMeans clusters points into concentric rings") {
        std::vector<std::pair<float, float>> points;
        for (int i = 0; i < 30; ++i) {
            float r = (i < 10) ? 0.1f : ((i < 20) ? 0.4f : 0.8f);
            float angle = i * 0.2f;
            points.push_back({0.5f + r * std::cos(angle), 0.5f + r * std::sin(angle)});
        }

        auto rings = SubmodelDetectorUtils::ClusterNodesRadialKMeans(points, 3);
        REQUIRE(rings.size() == 3);
    }

    SECTION("ExportToSubmodelXML generates canonical xLights XML") {
        std::vector<DetectedSubmodel> submodels;
        DetectedSubmodel s1;
        s1.name = "Outer_Ring";
        s1.type = "ranges";
        s1.length = 48;
        s1.nodeRangeString = "1-48";
        submodels.push_back(s1);

        std::string xml = SubmodelDetector::ExportToSubmodelXML(submodels);
        REQUIRE(xml.find("<submodels>") != std::string::npos);
        REQUIRE(xml.find("<submodel name=\"Outer_Ring\" type=\"ranges\" length=\"48\">") != std::string::npos);
        REQUIRE(xml.find("<node range=\"1-48\"/>") != std::string::npos);
    }
}
