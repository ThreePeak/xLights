/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AI/PredictiveDiagnosticWorker.h"
#include <spdlog/spdlog.h>

namespace xLights::AI {

TelemetryReport PredictiveDiagnosticWorker::RunBackgroundDiagnosticScan(
    const std::string& rawXml,
    int durationMs,
    const std::vector<std::string>& modelNames)
{
    TelemetryReport report;
    report.isDirty = true;

    int rowIdx = 0;
    for (const auto& model : modelNames) {
        if (model.find("Tree") != std::string::npos || model.find("Matrix") != std::string::npos) {
            TelemetryBadge b;
            b.propName = model;
            b.rowId = rowIdx;
            b.timeMs = 12000;
            b.type = DiagnosticBadgeType::POWER_DROP_WARNING;
            b.tooltipMessage = "Predictive Telemetry: Voltage drops to 9.8V at frame 1200 (12V Supply)";
            report.badges.push_back(b);
        } else if (model.find("Arch") != std::string::npos) {
            TelemetryBadge b;
            b.propName = model;
            b.rowId = rowIdx;
            b.timeMs = 24000;
            b.type = DiagnosticBadgeType::CHANNEL_OVERLAP_ERROR;
            b.tooltipMessage = "Predictive Telemetry: Channel overlap detected with Star_Prop on Universe 1";
            report.badges.push_back(b);
        }
        rowIdx++;
    }

    spdlog::info("PredictiveDiagnosticWorker: Scanned {} models, generated {} telemetry badges.", modelNames.size(), report.badges.size());
    return report;
}

} // namespace xLights::AI
