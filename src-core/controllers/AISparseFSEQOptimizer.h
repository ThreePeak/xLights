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
#include <cstdint>

namespace xLights::AI {

struct FseqSparseChannelRange {
    uint32_t startChannel{1};
    uint32_t channelCount{510};
    std::string propName;
};

struct SparseFseqOptimizationRequest {
    std::string sequenceName{"Holiday_Show_2026"};
    std::string targetController{"ESP32_MegaTree_Remote"};
    uint32_t totalSequenceChannels{100000};
    uint32_t frameCount{3600}; // 90 seconds @ 40 FPS
    uint32_t fps{40};
    std::vector<FseqSparseChannelRange> assignedRanges;
    uint32_t clusterAlignmentBytes{4096}; // 4KB FAT32 cluster boundary
    bool enableDeltaCompression{true};
    bool generateSparseIndexTable{true};
};

struct SparseFseqOptimizationResult {
    bool success{false};
    std::string controllerName;
    size_t originalSizeBytes{0};
    size_t sparseOptimizedSizeBytes{0};
    double bandwidthSavingsPercent{0.0};
    size_t clusterPaddingBytes{0};
    double requiredSdBandwidthKbps{0.0};
    double estimatedReadLatencyMs{0.0};
    bool isUnderrunSafe{true};
    std::string outputFileName;
    std::string sha256Checksum;
    std::vector<std::string> optimizationSteps;

    std::string GenerateFormattedReport() const;
};

class AISparseFSEQOptimizer {
public:
    static SparseFseqOptimizationResult OptimizeFseqForController(const SparseFseqOptimizationRequest& request);
    static double EstimateSdSpiLatencyMs(size_t frameSizeBytes, uint32_t clusterAlignment);
};

} // namespace xLights::AI
