/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "SubmodelDetectorAIGenerator.h"

using namespace xLights::AI;

TEST_CASE("SubmodelDetectorAIGenerator: Singing face SAM submodel auto-detection", "[SubmodelDetectorAIGenerator]") {
    SECTION("Detects Outline, Eyes Open, and 8 Visemes for Singing Face prop") {
        SubmodelDetectionConfig config;
        config.propHint = "Singing Face 100 Pixel";
        config.totalNodes = 100;
        config.gridWidth = 10;
        config.gridHeight = 10;

        SubmodelDetectionResult result = SubmodelDetectorAIGenerator::DetectSubmodelsFromImage(config);
        REQUIRE(result.success == true);
        REQUIRE(result.faceTypeDetected == "Singing Face 8-Viseme");
        REQUIRE(result.submodels.size() >= 10); // Outline + Eyes Open + 8 Visemes
        REQUIRE(result.submodelGroups.size() >= 3); // Outline, Eyes, Mouth groups
        REQUIRE(!result.generatedSubmodelXML.empty());
        REQUIRE(result.generatedSubmodelXML.find("<submodel name=\"Outline\"") != std::string::npos);
        REQUIRE(result.generatedSubmodelXML.find("<node range=") != std::string::npos);
    }

    SECTION("Detects structural ring submodels for standard prop") {
        SubmodelDetectionConfig config;
        config.propHint = "Mega Tree";
        config.totalNodes = 90;

        SubmodelDetectionResult result = SubmodelDetectorAIGenerator::DetectSubmodelsFromImage(config);
        REQUIRE(result.success == true);
        REQUIRE(result.submodels.size() == 3); // 3 Ring submodels
        REQUIRE(result.generatedSubmodelXML.find("<submodel name=\"Outer_Ring\"") != std::string::npos);
        REQUIRE(result.generatedSubmodelXML.find("<submodel name=\"Inner_Core\"") != std::string::npos);
    }

    SECTION("Concentric point coordinate arrays are correctly categorized into Outer_Perimeter and Inner_Core submodels") {
        SubmodelDetectionConfig config;
        config.parentModelName = "ConcentricProp";
        config.totalNodes = 100;
        for (int i = 0; i < 100; ++i) {
            float r = (i < 50) ? 0.9f : 0.2f;
            float angle = (i % 50) * 2.0f * 3.14159f / 50.0f;
            config.pixelCoordinates.push_back({0.5f + r * std::cos(angle), 0.5f + r * std::sin(angle)});
        }

        SubmodelDetectionResult result = SubmodelDetectorAIGenerator::DetectSubmodelsFromImage(config);
        REQUIRE(result.success == true);
        REQUIRE(result.generatedSubmodelXML.find("<submodel name=\"Inner_Core\"") != std::string::npos);
    }

    SECTION("Detects linear spoke submodels Spoke_1 through Spoke_N") {
        SubmodelDetectionConfig config;
        config.propHint = "8 Spoke Star Spinner";
        config.totalNodes = 80;

        SubmodelDetectionResult result = SubmodelDetectorAIGenerator::DetectSubmodelsFromImage(config);
        REQUIRE(result.success == true);
        REQUIRE(result.submodels.size() == 8); // 8 Spoke submodels
        REQUIRE(result.generatedSubmodelXML.find("<submodel name=\"Spoke_1\"") != std::string::npos);
        REQUIRE(result.generatedSubmodelXML.find("<submodel name=\"Spoke_8\"") != std::string::npos);
    }

    SECTION("Handles invalid totalNodes gracefully") {
        SubmodelDetectionConfig config;
        config.totalNodes = 0;

        SubmodelDetectionResult result = SubmodelDetectorAIGenerator::DetectSubmodelsFromImage(config);
        REQUIRE(result.success == false);
        REQUIRE(!result.errorMessage.empty());
    }
}

TEST_CASE("SubmodelDetectorAIGenerator: XML Export format validation", "[SubmodelDetectorAIGenerator]") {
    SECTION("Generates valid <submodels> XML string with <node range=\"1-48\"/>") {
        std::vector<DetectedSubmodel> submodels;

        DetectedSubmodel sub1;
        sub1.name = "Outline";
        sub1.nodeRangeString = "1-48";
        submodels.push_back(sub1);

        DetectedSubmodel sub2;
        sub2.name = "Eyes Open";
        sub2.nodeRangeString = "49-75";
        submodels.push_back(sub2);

        std::string xml = SubmodelDetectorAIGenerator::ExportToSubmodelXML(submodels);
        REQUIRE(!xml.empty());
        REQUIRE(xml.find("<submodels>") != std::string::npos);
        REQUIRE(xml.find("<submodel name=\"Outline\">") != std::string::npos);
        REQUIRE(xml.find("<node range=\"1-48\"/>") != std::string::npos);
        REQUIRE(xml.find("<submodel name=\"Eyes Open\">") != std::string::npos);
        REQUIRE(xml.find("<node range=\"49-75\"/>") != std::string::npos);
    }
}
