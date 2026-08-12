/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Unit tests for Feature #7: Camera-Based Auto Prop Mapper (Gray Code Detector)

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "models/GrayCodePropMapper.h"
#include <cstdint>

using namespace xLights::AI;

// -------------------------------------------------------------------------
// Test: BinaryToGray / GrayToBinary round-trip
// -------------------------------------------------------------------------
TEST_CASE("GrayCode: BinaryToGray and GrayToBinary are inverse operations", "[graycodepropmapper]") {
    for (uint32_t n = 0; n < 64; ++n) {
        uint32_t gray   = GrayCodePropMapper::BinaryToGray(n);
        uint32_t binary = GrayCodePropMapper::GrayToBinary(gray);
        REQUIRE(binary == n);
    }
}

TEST_CASE("GrayCode: Adjacent Gray Code values differ by exactly 1 bit", "[graycodepropmapper]") {
    for (uint32_t n = 0; n < 63; ++n) {
        uint32_t g1 = GrayCodePropMapper::BinaryToGray(n);
        uint32_t g2 = GrayCodePropMapper::BinaryToGray(n + 1);
        uint32_t diff = g1 ^ g2;
        // Exactly 1 bit should differ
        REQUIRE(diff != 0);
        REQUIRE((diff & (diff - 1)) == 0);
    }
}

// -------------------------------------------------------------------------
// Test: ComputeBitDepth
// -------------------------------------------------------------------------
TEST_CASE("GrayCode: ComputeBitDepth returns ceil(log2(N))", "[graycodepropmapper]") {
    REQUIRE(GrayCodePropMapper::ComputeBitDepth(1)   == 1);
    REQUIRE(GrayCodePropMapper::ComputeBitDepth(2)   == 1);
    REQUIRE(GrayCodePropMapper::ComputeBitDepth(3)   == 2);
    REQUIRE(GrayCodePropMapper::ComputeBitDepth(4)   == 2);
    REQUIRE(GrayCodePropMapper::ComputeBitDepth(5)   == 3);
    REQUIRE(GrayCodePropMapper::ComputeBitDepth(8)   == 3);
    REQUIRE(GrayCodePropMapper::ComputeBitDepth(100) == 7);
    REQUIRE(GrayCodePropMapper::ComputeBitDepth(256) == 8);
}

// -------------------------------------------------------------------------
// Test: GenerateGrayCodePatterns produces 2*K patterns
// -------------------------------------------------------------------------
TEST_CASE("GrayCode: GenerateGrayCodePatterns returns 2*K patterns", "[graycodepropmapper]") {
    int N = 16;
    int K = GrayCodePropMapper::ComputeBitDepth(N); // K=4
    auto patterns = GrayCodePropMapper::GenerateGrayCodePatterns(N);
    REQUIRE((int)patterns.size() == 2 * K);

    // Patterns must alternate normal/inverted
    for (int i = 0; i < (int)patterns.size(); i += 2) {
        REQUIRE(patterns[i].inverted == false);
        REQUIRE(patterns[i + 1].inverted == true);
    }
}

TEST_CASE("GrayCode: Normal and inverted patterns are bitwise complements", "[graycodepropmapper]") {
    int N = 8;
    auto patterns = GrayCodePropMapper::GenerateGrayCodePatterns(N);
    REQUIRE(!patterns.empty());

    for (int i = 0; i < (int)patterns.size(); i += 2) {
        const auto& normal   = patterns[i];
        const auto& inverted = patterns[i + 1];
        REQUIRE(normal.litNodes.size() == inverted.litNodes.size());
        for (size_t n = 0; n < normal.litNodes.size(); ++n) {
            REQUIRE(normal.litNodes[n] != inverted.litNodes[n]);
        }
    }
}

// -------------------------------------------------------------------------
// Test: ComputePatternBit — Bit_k = Frame_pattern > Frame_inverse_pattern
// -------------------------------------------------------------------------
TEST_CASE("GrayCode: ComputePatternBit returns true when pattern > inverse", "[graycodepropmapper]") {
    REQUIRE(GrayCodePropMapper::ComputePatternBit(200.0f, 50.0f)  == true);
    REQUIRE(GrayCodePropMapper::ComputePatternBit(50.0f, 200.0f)  == false);
    REQUIRE(GrayCodePropMapper::ComputePatternBit(100.0f, 100.0f) == false);
}

// -------------------------------------------------------------------------
// Test: RunMappingSession with synthetic captures
// -------------------------------------------------------------------------
TEST_CASE("GrayCode: RunMappingSession succeeds with synthetic camera frames", "[graycodepropmapper]") {
    const int N = 8;
    const int W = 4;
    const int H = 2;
    int K = GrayCodePropMapper::ComputeBitDepth(N);

    // Build synthetic normal + inverted captures (128x64 grayscale, alternating ON/OFF halves)
    int camW = 128, camH = 64;
    std::vector<CameraFrameCapture> captures;
    for (int k = 0; k < K; ++k) {
        uint32_t mask = 1u << (K - 1 - k);

        CameraFrameCapture normal;
        normal.patternIndex = k;
        normal.isInverted = false;
        normal.width = camW;
        normal.height = camH;
        normal.pixels.resize(camW * camH, 0);

        CameraFrameCapture inv;
        inv.patternIndex = k;
        inv.isInverted = true;
        inv.width = camW;
        inv.height = camH;
        inv.pixels.resize(camW * camH, 0);

        // Set pixel brightness based on Gray Code bit for each column
        for (int n = 0; n < N; ++n) {
            int x = n * camW / N;
            int x_end = (n + 1) * camW / N;
            uint32_t gray = GrayCodePropMapper::BinaryToGray((uint32_t)n);
            bool bit = (gray & mask) != 0;
            for (int row = 0; row < camH; ++row) {
                for (int col = x; col < x_end && col < camW; ++col) {
                    normal.pixels[row * camW + col] = bit ? 220 : 30;
                    inv.pixels[row * camW + col]    = bit ? 30 : 220;
                }
            }
        }

        captures.push_back(normal);
        captures.push_back(inv);
    }

    auto result = GrayCodePropMapper::RunMappingSession(N, W, H, captures);
    REQUIRE(result.success == true);
    REQUIRE(result.totalNodes == N);
    REQUIRE(result.decodedNodes == N);
    REQUIRE(!result.customModelXML.empty());
    REQUIRE(result.customModelXML.find("<custommodel") != std::string::npos);
    REQUIRE(result.customModelXML.find("GrayCodePropMapper") != std::string::npos);
}

// -------------------------------------------------------------------------
// Test: ExportToCustomModelXML produces valid XML
// -------------------------------------------------------------------------
TEST_CASE("GrayCode: ExportToCustomModelXML produces valid xLights Custom Model XML", "[graycodepropmapper]") {
    GrayCodeMapResult result;
    result.success = true;
    result.totalNodes = 4;
    result.decodedNodes = 4;
    result.modelWidth = 2;
    result.modelHeight = 2;

    result.pixels = {
        {0, 0, 0, 0, 0, 0.9f, true},
        {1, 1, 0, 1, 0, 0.9f, true},
        {2, 0, 1, 0, 1, 0.9f, true},
        {3, 1, 1, 1, 1, 0.9f, true},
    };

    std::string xml = GrayCodePropMapper::ExportToCustomModelXML(result, "TestProp");
    REQUIRE(xml.find("TestProp") != std::string::npos);
    REQUIRE(xml.find("parm1=\"2\"") != std::string::npos);
    REQUIRE(xml.find("parm2=\"2\"") != std::string::npos);
    REQUIRE(xml.find("CustomModel=") != std::string::npos);
    REQUIRE(xml.find("AI_Generated=\"true\"") != std::string::npos);
}

// -------------------------------------------------------------------------
// Test: ExportToCSV produces header and correct row count
// -------------------------------------------------------------------------
TEST_CASE("GrayCode: ExportToCSV produces correct CSV output", "[graycodepropmapper]") {
    GrayCodeMapResult result;
    result.totalNodes = 3;
    result.decodedNodes = 3;
    result.modelWidth = 3;
    result.modelHeight = 1;
    result.pixels = {
        {0, 10, 5, 0, 0, 0.8f, true},
        {1, 40, 5, 1, 0, 0.9f, true},
        {2, 70, 5, 2, 0, 0.7f, true},
    };

    std::string csv = GrayCodePropMapper::ExportToCSV(result);
    REQUIRE(csv.find("NodeIndex,CamX,CamY") != std::string::npos);
    // Count data rows (3 nodes + header)
    int newlines = 0;
    for (char c : csv) if (c == '\n') ++newlines;
    REQUIRE(newlines == 4); // header + 3 rows
}

// -------------------------------------------------------------------------
// Test: Error path — invalid dimensions
// -------------------------------------------------------------------------
TEST_CASE("GrayCode: RunMappingSession returns error for invalid dimensions", "[graycodepropmapper]") {
    auto result = GrayCodePropMapper::RunMappingSession(0, 0, 0, {});
    REQUIRE(result.success == false);
    REQUIRE(!result.errorMessage.empty());
}
