/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "AutoPropMapper.h"
#include <cmath>
#include <iostream>

using namespace xLights::AI;

TEST_CASE("AutoPropMapper: Gray Code pattern generation bit depth and count", "[AutoPropMapper]") {
    SECTION("Generates 2*N patterns for N-bit resolution") {
        int pixelCount = 16; // log2(16) = 4 bits -> 8 patterns
        auto patterns = AutoPropMapper::GenerateGrayCodePatterns(pixelCount);
        REQUIRE(patterns.size() == 8);

        for (const auto& pat : patterns) {
            REQUIRE(pat.size() == 16);
        }
    }

    SECTION("Normal and inverse patterns are complementary") {
        int pixelCount = 8; // 3 bits -> 6 patterns
        auto patterns = AutoPropMapper::GenerateGrayCodePatterns(pixelCount);
        REQUIRE(patterns.size() == 6);

        for (size_t k = 0; k < 3; ++k) {
            const auto& normal = patterns[2 * k];
            const auto& inverse = patterns[2 * k + 1];
            for (size_t i = 0; i < 8; ++i) {
                REQUIRE(normal[i] == !inverse[i]);
            }
        }
    }
}

TEST_CASE("AutoPropMapper: ProcessCameraMapping decodes pixels correctly", "[AutoPropMapper]") {
    SECTION("Decodes synthetic 16-pixel custom prop") {
        PropMappingConfig config;
        config.modelTargetName = "TestStarProp";
        config.totalPixelCount = 16;
        config.cameraDeviceId = 0;
        config.patternHoldTimeMs = 50;

        PropMappingResult result = AutoPropMapper::ProcessCameraMapping(config);
        REQUIRE(result.success == true);
        REQUIRE(result.modelName == "TestStarProp");
        REQUIRE(result.mappedPixels.size() == 16);
        REQUIRE(!result.generatedCustomModelXML.empty());

        for (size_t i = 0; i < result.mappedPixels.size(); ++i) {
            REQUIRE(result.mappedPixels[i].pixelIndex == static_cast<int>(i));
            REQUIRE(result.mappedPixels[i].confidence > 0.90f);
        }
    }

    SECTION("Handles invalid pixel count gracefully") {
        PropMappingConfig config;
        config.totalPixelCount = 0;

        PropMappingResult result = AutoPropMapper::ProcessCameraMapping(config);
        REQUIRE(result.success == false);
        REQUIRE(!result.errorMessage.empty());
    }
}

TEST_CASE("AutoPropMapper: ExportToCustomModelXML structure validation", "[AutoPropMapper]") {
    SECTION("Generates valid XML with custommodel node and matrix string") {
        std::vector<MappedPixelNode> nodes;
        for (int i = 0; i < 4; ++i) {
            MappedPixelNode node;
            node.pixelIndex = i;
            node.normalizedX = (i % 2 == 0) ? 0.0f : 1.0f;
            node.normalizedY = (i < 2) ? 0.0f : 1.0f;
            node.confidence = 1.0f;
            nodes.push_back(node);
        }

        std::string xml = AutoPropMapper::ExportToCustomModelXML("MatrixProp", nodes, 2, 2);
        REQUIRE(!xml.empty());
        REQUIRE(xml.find("<custommodel") != std::string::npos);
        REQUIRE(xml.find("name=\"MatrixProp\"") != std::string::npos);
        REQUIRE(xml.find("parm1=\"2\"") != std::string::npos);
        REQUIRE(xml.find("parm2=\"2\"") != std::string::npos);
        REQUIRE(xml.find("CustomModel=") != std::string::npos);
    }
}
