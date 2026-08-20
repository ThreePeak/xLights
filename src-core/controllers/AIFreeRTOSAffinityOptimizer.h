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

enum class FreeRTOSCorePinning {
    CORE_0_PROTOCOL_STACK,
    CORE_1_PIXEL_PIPELINE,
    ANY_CORE_AFFINITY
};

struct FreeRTOSTaskConfig {
    std::string taskName;
    FreeRTOSCorePinning coreAffinity{FreeRTOSCorePinning::CORE_0_PROTOCOL_STACK};
    int priority{5}; // 1 = lowest, 24 = highest (configMAX_PRIORITIES-1)
    size_t stackSizeBytes{4096};
    int queueDepth{8};
    std::string role;
    std::string riskAnalysis;
};

struct FreeRTOSOptimizationRequest {
    std::string controllerName{"ESP32_DigOcta_Controller"};
    bool isDualCore{true};
    int numPixelPorts{4};
    int pixelsPerPort{600};
    bool enableDdpUdpListener{true};
    bool enableSdCardFppPlayback{true};
    bool enableWebOtaServer{true};
    bool enableSyslogDiagnostics{true};
};

struct FreeRTOSOptimizationResult {
    bool success{false};
    std::string controllerName;
    std::string chipCoreType;
    size_t totalTaskStackRamBytes{0};
    double estimatedWdtMarginPercent{85.0};
    bool interruptConflictSafe{true};
    std::vector<FreeRTOSTaskConfig> tasks;
    std::string platformioIniFlags;
    std::string freertosCppSkeleton;
    std::vector<std::string> optimizationSteps;

    std::string GenerateFormattedReport() const;
};

class AIFreeRTOSAffinityOptimizer {
public:
    static FreeRTOSOptimizationResult OptimizeAffinity(const FreeRTOSOptimizationRequest& request);
    static std::string GetCorePinningName(FreeRTOSCorePinning core);
};

} // namespace xLights::AI
