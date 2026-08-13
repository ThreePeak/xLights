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
#include <atomic>
#include <future>
#include <memory>

namespace xLights::AI {

enum class DiagnosticBadgeType {
    NONE,
    POWER_DROP_WARNING,
    CHANNEL_OVERLAP_ERROR,
    RHYTHM_MISALIGNMENT_INFO
};

struct TelemetryBadge {
    std::string propName;
    int rowId = -1;
    int timeMs = 0;
    DiagnosticBadgeType type = DiagnosticBadgeType::NONE;
    std::string tooltipMessage;
};

struct TelemetryReport {
    bool isDirty = false;
    std::vector<TelemetryBadge> badges;
};

class PredictiveDiagnosticWorker {
public:
    PredictiveDiagnosticWorker() = default;
    ~PredictiveDiagnosticWorker() = default;

    static TelemetryReport RunBackgroundDiagnosticScan(
        const std::string& rawXml,
        int durationMs,
        const std::vector<std::string>& modelNames
    );
};

} // namespace xLights::AI
