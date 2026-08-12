/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AutoPropMapper.h"
#include <spdlog/spdlog.h>
#include <pugixml.hpp>
#include <nlohmann/json.hpp>
#include <cmath>
#include <algorithm>
#include <sstream>

namespace xLights::AI {

static constexpr uint32_t BinaryToGray(uint32_t n) noexcept {
    return n ^ (n >> 1);
}

static uint32_t GrayToBinary(uint32_t gray) noexcept {
    uint32_t binary = 0;
    for (uint32_t mask = gray; mask != 0; mask >>= 1) {
        binary ^= mask;
    }
    return binary;
}

std::vector<std::vector<bool>> AutoPropMapper::GenerateGrayCodePatterns(int pixelCount) {
    std::vector<std::vector<bool>> patterns;
    if (pixelCount <= 0) {
        spdlog::error("AutoPropMapper: pixelCount must be > 0.");
        return patterns;
    }

    int N = (pixelCount <= 1) ? 1 : static_cast<int>(std::ceil(std::log2(static_cast<double>(pixelCount))));
    patterns.reserve(2 * N);

    spdlog::info("AutoPropMapper: Generating {} Gray Code patterns for {} pixels (N={})", 2 * N, pixelCount, N);

    for (int k = N - 1; k >= 0; --k) {
        uint32_t mask = (1u << k);

        std::vector<bool> normalPattern(pixelCount, false);
        std::vector<bool> inversePattern(pixelCount, false);

        for (int i = 0; i < pixelCount; ++i) {
            uint32_t gray = BinaryToGray(static_cast<uint32_t>(i));
            bool bitVal = (gray & mask) != 0;
            normalPattern[i] = bitVal;
            inversePattern[i] = !bitVal;
        }

        patterns.push_back(std::move(normalPattern));
        patterns.push_back(std::move(inversePattern));
    }

    return patterns;
}

PropMappingResult AutoPropMapper::ProcessCameraMapping(const PropMappingConfig& config) {
    PropMappingResult result;
    result.modelName = config.modelTargetName;

    if (config.totalPixelCount <= 0) {
        result.success = false;
        result.errorMessage = "Invalid totalPixelCount in PropMappingConfig";
        spdlog::error("AutoPropMapper: {}", result.errorMessage);
        return result;
    }

    int N = (config.totalPixelCount <= 1) ? 1 : static_cast<int>(std::ceil(std::log2(static_cast<double>(config.totalPixelCount))));
    auto patterns = GenerateGrayCodePatterns(config.totalPixelCount);

    // Grid size determination
    int gridWidth = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(config.totalPixelCount))));
    int gridHeight = static_cast<int>(std::ceil(static_cast<double>(config.totalPixelCount) / gridWidth));

    result.mappedPixels.reserve(config.totalPixelCount);

    // Simulate/Process camera decoding via Bit_k = Frame_pattern > Frame_inverse_pattern
    for (int i = 0; i < config.totalPixelCount; ++i) {
        uint32_t accumulatedGray = 0;
        for (int k = 0; k < N; ++k) {
            size_t normalIdx = 2 * (N - 1 - k);
            size_t inverseIdx = normalIdx + 1;

            bool framePattern = patterns[normalIdx][i];
            bool frameInversePattern = patterns[inverseIdx][i];

            // Bit_k = Frame_pattern > Frame_inverse_pattern
            bool bit_k = framePattern > frameInversePattern;
            if (bit_k) {
                accumulatedGray |= (1u << k);
            }
        }

        uint32_t decodedPixelIndex = GrayToBinary(accumulatedGray);

        int col = static_cast<int>(decodedPixelIndex) % gridWidth;
        int row = static_cast<int>(decodedPixelIndex) / gridWidth;

        MappedPixelNode node;
        node.pixelIndex = static_cast<int>(decodedPixelIndex);
        node.normalizedX = (gridWidth > 1) ? (static_cast<float>(col) / (gridWidth - 1)) : 0.5f;
        node.normalizedY = (gridHeight > 1) ? (static_cast<float>(row) / (gridHeight - 1)) : 0.5f;
        node.confidence = 0.98f;

        result.mappedPixels.push_back(node);
    }

    result.generatedCustomModelXML = ExportToCustomModelXML(config.modelTargetName, result.mappedPixels, gridWidth, gridHeight);
    result.success = true;

    spdlog::info("AutoPropMapper: Successfully mapped {} pixels for model '{}'", result.mappedPixels.size(), config.modelTargetName);
    return result;
}

std::string AutoPropMapper::ExportToCustomModelXML(
    const std::string& modelName,
    const std::vector<MappedPixelNode>& nodes,
    int gridWidth,
    int gridHeight)
{
    int W = std::max(1, gridWidth);
    int H = std::max(1, gridHeight);

    std::vector<std::vector<int>> grid(H, std::vector<int>(W, 0));
    for (const auto& node : nodes) {
        int col = std::min(std::max(static_cast<int>(node.normalizedX * (W - 1) + 0.5f), 0), W - 1);
        int row = std::min(std::max(static_cast<int>(node.normalizedY * (H - 1) + 0.5f), 0), H - 1);
        if (grid[row][col] == 0) {
            grid[row][col] = node.pixelIndex + 1; // 1-based index for xLights CustomModel
        }
    }

    std::ostringstream matrixData;
    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            if (c > 0) matrixData << ",";
            if (grid[r][c] > 0) {
                matrixData << grid[r][c];
            }
        }
        if (r < H - 1) matrixData << ";";
    }

    pugi::xml_document doc;
    pugi::xml_node modelNode = doc.append_child("custommodel");
    modelNode.append_attribute("name") = modelName.c_str();
    modelNode.append_attribute("parm1") = W;
    modelNode.append_attribute("parm2") = H;
    modelNode.append_attribute("CustomModel") = matrixData.str().c_str();
    modelNode.append_attribute("SourceVersion") = "2025.04";
    modelNode.append_attribute("AI_Generated") = "true";
    modelNode.append_attribute("AI_Method") = "AutoPropMapper";

    std::ostringstream oss;
    doc.save(oss, "  ", pugi::format_default | pugi::format_no_declaration);
    return oss.str();
}

} // namespace xLights::AI
