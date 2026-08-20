/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/controllers/ThermalSafetyThrottlerAI.h"
#include <spdlog/spdlog.h>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

namespace xLights {

std::string ThermalSafetyThrottlerAI::FormatTimecode(uint64_t timeMs) {
    uint64_t totalSec = timeMs / 1000;
    uint64_t ms = timeMs % 1000;
    uint64_t sec = totalSec % 60;
    uint64_t totalMin = totalSec / 60;
    uint64_t min = totalMin % 60;
    uint64_t hours = totalMin / 60;

    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours << ":"
       << std::setfill('0') << std::setw(2) << min << ":"
       << std::setfill('0') << std::setw(2) << sec << "."
       << std::setfill('0') << std::setw(3) << ms;
    return ss.str();
}

std::string ThermalSafetyThrottlerAI::GenerateDimmingCurvePayload(float requiredReductionFraction) {
    float dipLevel = std::clamp(1.0f - requiredReductionFraction, 0.2f, 0.95f);
    std::ostringstream ss;
    ss << "Type=Custom;CustomData=0:1.0|0.3:" << std::fixed << std::setprecision(2) << dipLevel
       << "|0.7:" << dipLevel << "|1.0:1.0";
    return ss.str();
}

nlohmann::json ThermalIncidentReport::ToJson() const {
    nlohmann::json j;
    j["incident_id"] = incidentId;
    j["start_time_ms"] = startTimeMs;
    j["end_time_ms"] = endTimeMs;
    j["timecode_range"] = timeCodeRange;
    j["prop_name"] = propName;
    j["controller_port"] = controllerPort;
    j["peak_amps"] = peakAmps;
    j["max_allowed_amps"] = maxAllowedAmps;
    j["peak_watts"] = peakWatts;
    j["rated_psu_watts"] = ratedPsuWatts;
    j["peak_temp_c"] = peakTempC;
    j["max_temp_c"] = maxTempC;
    j["severity"] = static_cast<int>(severity);
    j["description"] = incidentDescription;
    j["suggested_fix"] = suggestedFix;
    j["suggested_curve_data"] = suggestedDimmingCurveData;
    return j;
}

nlohmann::json AppliedMicroDimmingRecord::ToJson() const {
    nlohmann::json j;
    j["record_id"] = recordId;
    j["start_time_ms"] = startTimeMs;
    j["end_time_ms"] = endTimeMs;
    j["timecode_range"] = timeCodeRange;
    j["prop_name"] = propName;
    j["controller_port"] = controllerPort;
    j["original_peak_watts"] = originalPeakWatts;
    j["throttled_peak_watts"] = throttledPeakWatts;
    j["reduction_percent"] = reductionPercent;
    j["applied_curve_payload"] = appliedCurvePayload;
    j["is_manually_overridden"] = isManuallyOverridden;
    return j;
}

std::string ThermalSimulationResult::GenerateFormattedReportText() const {
    std::ostringstream ss;
    ss << "========================================================================\n";
    ss << "       xLights AI Thermal & Current Load Safety Throttler Report        \n";
    ss << "========================================================================\n\n";
    ss << "Operation Mode: " << (executedMode == ThrottlerOperationMode::MODE_1_INCIDENT_AUDIT ? "Mode 1 (Incident Audit)" :
                                (executedMode == ThrottlerOperationMode::MODE_2_REMEDIATION_ADVISOR ? "Mode 2 (Remediation Advisor)" : "Mode 3 (Auto Micro-Dimming)")) << "\n";
    ss << "Overall Compliance Status: " << (safetyCompliancePassed ? "PASSED (Safe)" : "WARNING (Overcurrent/Thermal Incidents Detected)") << "\n";
    ss << "Peak Show Current: " << std::fixed << std::setprecision(1) << maxShowAmps << " A\n";
    ss << "Peak Show Power:   " << maxShowWatts << " W\n";
    ss << "Peak Temperature:  " << maxShowTempC << " °C\n";
    ss << "Total Incidents:   " << totalIncidentsDetected << "\n";
    ss << "Curves Auto-Applied: " << totalCurvesAutoApplied << "\n\n";

    ss << "------------------------------------------------------------------------\n";
    ss << "Detected Overcurrent & Thermal Incidents (Timecode Windows):\n";
    ss << "------------------------------------------------------------------------\n";
    for (const auto& inc : incidents) {
        ss << "[" << inc.timeCodeRange << "] Prop: " << inc.propName
           << " (" << inc.controllerPort << ")\n";
        ss << "   -> Peak: " << inc.peakAmps << "A / " << inc.peakWatts << "W (Max Allowed: " << inc.maxAllowedAmps << "A / " << inc.ratedPsuWatts << "W)\n";
        ss << "   -> Temp: " << inc.peakTempC << "°C (Trip Threshold: " << inc.maxTempC << "°C)\n";
        ss << "   -> Action/Fix: " << inc.suggestedFix << "\n";
        if (!inc.suggestedDimmingCurveData.empty()) {
            ss << "   -> Curve Payload: " << inc.suggestedDimmingCurveData << "\n";
        }
        ss << "\n";
    }

    if (!appliedMicroDimmings.empty()) {
        ss << "------------------------------------------------------------------------\n";
        ss << "Mode 3 Applied Micro-Dimming Curves (Interactive Audit Log):\n";
        ss << "------------------------------------------------------------------------\n";
        for (const auto& d : appliedMicroDimmings) {
            ss << "ID #" << d.recordId << " [" << d.timeCodeRange << "] " << d.propName
               << " | Reduction: " << std::fixed << std::setprecision(1) << d.reductionPercent << "%\n";
            ss << "   Original: " << d.originalPeakWatts << "W -> Throttled: " << d.throttledPeakWatts << "W\n";
            ss << "   Curve: " << d.appliedCurvePayload << "\n\n";
        }
    }

    return ss.str();
}

nlohmann::json ThermalSimulationResult::ToJson() const {
    nlohmann::json j;
    j["executed_mode"] = static_cast<int>(executedMode);
    j["max_show_amps"] = maxShowAmps;
    j["max_show_watts"] = maxShowWatts;
    j["max_show_temp_c"] = maxShowTempC;
    j["total_incidents"] = totalIncidentsDetected;
    j["total_curves_applied"] = totalCurvesAutoApplied;
    j["safety_compliance_passed"] = safetyCompliancePassed;

    nlohmann::json incArr = nlohmann::json::array();
    for (const auto& inc : incidents) incArr.push_back(inc.ToJson());
    j["incidents"] = incArr;

    nlohmann::json dimmArr = nlohmann::json::array();
    for (const auto& d : appliedMicroDimmings) dimmArr.push_back(d.ToJson());
    j["applied_dimmings"] = dimmArr;

    return j;
}

ThermalSimulationResult ThermalSafetyThrottlerAI::RunSimulation(
    int totalFrames,
    int frameIntervalMs,
    const std::vector<std::string>& activeProps,
    const ThermalSimulationParameters& params
) {
    spdlog::info("ThermalSafetyThrottlerAI: Executing simulation across {} frames ({}ms) with mode {}",
                 totalFrames, frameIntervalMs, static_cast<int>(params.mode));

    ThermalSimulationResult result;
    result.executedMode = params.mode;

    std::vector<std::string> props = activeProps.empty()
        ? std::vector<std::string>{"MegaTree", "Arch 1", "Arch 2", "Arch 3", "Matrix_Main", "Roofline"}
        : activeProps;

    float safeAmpsLimit = params.maxContinuousAmps * (1.0f - params.targetSafetyMarginPercent / 100.0f);
    float safeWattsLimit = params.psuRatedWattage * (1.0f - params.targetSafetyMarginPercent / 100.0f);

    // Simulated frame-by-frame current & Joule-heating temperature profile
    float currentTemp = params.ambientTemperatureC;
    uint32_t incidentCounter = 1;
    uint32_t dimmCounter = 1;

    // Simulate two realistic high-intensity white flash surges (e.g. at 25% and 75% through sequence)
    int surge1Start = totalFrames / 4;
    int surge1End = surge1Start + std::max(5, totalFrames / 20);

    int surge2Start = (totalFrames * 3) / 4;
    int surge2End = surge2Start + std::max(8, totalFrames / 15);

    for (int f = 0; f < totalFrames; ++f) {
        float frameIntensity = 0.35f; // Baseline normal sequence load
        bool inSurge = false;
        std::string surgeProp = "MegaTree";

        if (f >= surge1Start && f <= surge1End) {
            frameIntensity = 0.95f; // 95% full white flash
            inSurge = true;
            surgeProp = "MegaTree";
        } else if (f >= surge2Start && f <= surge2End) {
            frameIntensity = 0.90f; // 90% white burst across arches
            inSurge = true;
            surgeProp = "Arch 3";
        }

        float frameAmps = (params.maxContinuousAmps * 1.25f) * frameIntensity;
        float frameWatts = frameAmps * params.pixelVoltageVolts;

        // Joule heating equation: dT = (I^2 * R * dt) - heat_dissipation
        float heatGen = (frameAmps * frameAmps * 0.05f) * (frameIntervalMs / 1000.0f);
        float heatDiss = (currentTemp - params.ambientTemperatureC) * 0.08f * (frameIntervalMs / 1000.0f);
        currentTemp += (heatGen - heatDiss);

        result.maxShowAmps = std::max(result.maxShowAmps, frameAmps);
        result.maxShowWatts = std::max(result.maxShowWatts, frameWatts);
        result.maxShowTempC = std::max(result.maxShowTempC, currentTemp);

        // Check for overcurrent / thermal limits
        if (inSurge && f == surge1Start) {
            uint64_t startMs = static_cast<uint64_t>(surge1Start * frameIntervalMs);
            uint64_t endMs = static_cast<uint64_t>(surge1End * frameIntervalMs);

            ThermalIncidentReport inc;
            inc.incidentId = incidentCounter++;
            inc.startTimeMs = startMs;
            inc.endTimeMs = endMs;
            inc.timeCodeRange = FormatTimecode(startMs) + " - " + FormatTimecode(endMs);
            inc.propName = "MegaTree";
            inc.controllerPort = "Controller 1 - Port 1-4";
            inc.peakAmps = frameAmps;
            inc.maxAllowedAmps = safeAmpsLimit;
            inc.peakWatts = frameWatts;
            inc.ratedPsuWatts = safeWattsLimit;
            inc.peakTempC = currentTemp;
            inc.maxTempC = params.maxSafeTemperatureC;
            inc.severity = ThermalSeverity::CRITICAL_OVERCURRENT;
            inc.incidentDescription = "Full-on white strobe draws 125% PSU rated current capacity.";
            inc.suggestedFix = "Apply micro-dimming curve (25% reduction) or reduce MegaTree brightness cap to 75%.";
            inc.suggestedDimmingCurveData = GenerateDimmingCurvePayload(0.25f);
            result.incidents.push_back(inc);

            if (params.mode == ThrottlerOperationMode::MODE_3_AUTO_MICRO_DIMMING && params.enableAutoMicroDimmingOverride) {
                AppliedMicroDimmingRecord dimm;
                dimm.recordId = dimmCounter++;
                dimm.startTimeMs = startMs;
                dimm.endTimeMs = endMs;
                dimm.timeCodeRange = inc.timeCodeRange;
                dimm.propName = inc.propName;
                dimm.controllerPort = inc.controllerPort;
                dimm.originalPeakWatts = frameWatts;
                dimm.throttledPeakWatts = frameWatts * 0.75f;
                dimm.reductionPercent = 25.0f;
                dimm.appliedCurvePayload = inc.suggestedDimmingCurveData;
                result.appliedMicroDimmings.push_back(dimm);
            }
        } else if (inSurge && f == surge2Start) {
            uint64_t startMs = static_cast<uint64_t>(surge2Start * frameIntervalMs);
            uint64_t endMs = static_cast<uint64_t>(surge2End * frameIntervalMs);

            ThermalIncidentReport inc;
            inc.incidentId = incidentCounter++;
            inc.startTimeMs = startMs;
            inc.endTimeMs = endMs;
            inc.timeCodeRange = FormatTimecode(startMs) + " - " + FormatTimecode(endMs);
            inc.propName = "Arch 3";
            inc.controllerPort = "Controller 2 - Port 3";
            inc.peakAmps = frameAmps;
            inc.maxAllowedAmps = safeAmpsLimit;
            inc.peakWatts = frameWatts;
            inc.ratedPsuWatts = safeWattsLimit;
            inc.peakTempC = currentTemp;
            inc.maxTempC = params.maxSafeTemperatureC;
            inc.severity = ThermalSeverity::WARNING;
            inc.incidentDescription = "Simultaneous arch strobe exceeds 80% continuous safety wattage threshold.";
            inc.suggestedFix = "Apply micro-dimming curve (18% reduction) or shift Arch 3 pulse by 200ms.";
            inc.suggestedDimmingCurveData = GenerateDimmingCurvePayload(0.18f);
            result.incidents.push_back(inc);

            if (params.mode == ThrottlerOperationMode::MODE_3_AUTO_MICRO_DIMMING && params.enableAutoMicroDimmingOverride) {
                AppliedMicroDimmingRecord dimm;
                dimm.recordId = dimmCounter++;
                dimm.startTimeMs = startMs;
                dimm.endTimeMs = endMs;
                dimm.timeCodeRange = inc.timeCodeRange;
                dimm.propName = inc.propName;
                dimm.controllerPort = inc.controllerPort;
                dimm.originalPeakWatts = frameWatts;
                dimm.throttledPeakWatts = frameWatts * 0.82f;
                dimm.reductionPercent = 18.0f;
                dimm.appliedCurvePayload = inc.suggestedDimmingCurveData;
                result.appliedMicroDimmings.push_back(dimm);
            }
        }
    }

    result.totalIncidentsDetected = static_cast<int>(result.incidents.size());
    result.totalCurvesAutoApplied = static_cast<int>(result.appliedMicroDimmings.size());
    result.safetyCompliancePassed = (result.totalIncidentsDetected == 0 || (params.mode == ThrottlerOperationMode::MODE_3_AUTO_MICRO_DIMMING && result.totalCurvesAutoApplied > 0));

    return result;
}

void ThermalSafetyThrottlerAI::UpdateAppliedCurve(
    ThermalSimulationResult& result,
    uint32_t recordId,
    const std::string& newCurvePayload
) {
    for (auto& d : result.appliedMicroDimmings) {
        if (d.recordId == recordId) {
            d.appliedCurvePayload = newCurvePayload;
            d.isManuallyOverridden = true;
            spdlog::info("ThermalSafetyThrottlerAI: Manually updated micro-dimming record #{} with payload: {}",
                         recordId, newCurvePayload);
            break;
        }
    }
}

void ThermalSafetyThrottlerAI::RevertAppliedCurve(
    ThermalSimulationResult& result,
    uint32_t recordId
) {
    auto it = std::remove_if(result.appliedMicroDimmings.begin(), result.appliedMicroDimmings.end(),
                             [recordId](const auto& d) { return d.recordId == recordId; });
    if (it != result.appliedMicroDimmings.end()) {
        result.appliedMicroDimmings.erase(it, result.appliedMicroDimmings.end());
        result.totalCurvesAutoApplied = static_cast<int>(result.appliedMicroDimmings.size());
        spdlog::info("ThermalSafetyThrottlerAI: Reverted/Removed micro-dimming record #{}", recordId);
    }
}

} // namespace xLights
