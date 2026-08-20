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
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

namespace xLights::AI {

enum class PixelFaultType {
    DEAD_DARK = 0,         ///< Pixel completely unresponsive / no light emitted
    STUCK_COLOR = 1,       ///< Pixel permanently stuck on a specific channel (e.g. 100% Green)
    FLICKERING = 2,        ///< Intermittent data signal corruption
    COLOR_CHANNEL_LOST = 3 ///< Red, Green, or Blue sub-die failed
};

struct DeadPixelDiagnosis {
    int nodeIndex{0};
    int strandIndex{0};
    std::string propName{"MegaTree"};
    PixelFaultType faultType{PixelFaultType::DEAD_DARK};
    float confidenceScore{0.95f}; ///< 0.0 to 1.0 vision detection confidence
    std::vector<int> neighborNodeIndices; ///< Adjacent working pixels used for spatial healing
    std::vector<float> neighborWeightFactors; ///< Blending weights for Laplacian interpolation
};

struct AutoHealingCalibrationResult {
    bool analysisSuccess{true};
    std::string propName{"MegaTree"};
    size_t totalNodesEvaluated{800};
    size_t deadPixelsDetected{3};
    std::vector<DeadPixelDiagnosis> diagnosedFaults;
    float showQualityRecoveryScore{98.5f}; ///< Estimated perceived visual restoration %

    [[nodiscard]] nlohmann::json ToJson() const;
    [[nodiscard]] std::string GenerateFormattedReport() const;
};

class PixelAutoHealingAI {
public:
    /**
     * @brief Analyzes a camera test image / frame buffer to diagnose faulty pixels and calculate spatial interpolation weights.
     */
    static AutoHealingCalibrationResult DiagnoseAndComputeHealing(
        const std::string& propName,
        size_t totalNodes,
        const std::vector<int>& simulatedFaultyNodes = {}
    );

    /**
     * @brief Real-time runtime color remapping for a frame buffer.
     */
    static void ApplyHealingToFrame(
        const AutoHealingCalibrationResult& calibration,
        std::vector<uint8_t>& rgbPixelBuffer
    );
};

} // namespace xLights::AI
