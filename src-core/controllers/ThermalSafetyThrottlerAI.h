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
#include <memory>
#include <nlohmann/json.hpp>

namespace xLights {

enum class ThermalSeverity {
    NORMAL = 0,
    WARNING,
    CRITICAL_OVERCURRENT,
    THERMAL_SHUTDOWN_RISK
};

enum class ThrottlerOperationMode {
    MODE_1_INCIDENT_AUDIT = 0,
    MODE_2_REMEDIATION_ADVISOR,
    MODE_3_AUTO_MICRO_DIMMING
};

struct ThermalIncidentReport {
    uint32_t incidentId{0};
    uint64_t startTimeMs{0};
    uint64_t endTimeMs{0};
    std::string timeCodeRange;      // e.g. "00:01:23.450 - 00:01:28.100"
    std::string propName;           // e.g. "MegaTree", "Arch 3"
    std::string controllerPort;     // e.g. "Controller 1 - Port 4"
    float peakAmps{0.0f};
    float maxAllowedAmps{30.0f};
    float peakWatts{0.0f};
    float ratedPsuWatts{350.0f};
    float peakTempC{25.0f};
    float maxTempC{70.0f};
    ThermalSeverity severity{ThermalSeverity::NORMAL};
    std::string incidentDescription;
    std::string suggestedFix;       // Mode 2 specific remediation
    std::string suggestedDimmingCurveData; // e.g. "Type=Custom;CustomData=0:1.0|0.5:0.75|1.0:1.0"

    nlohmann::json ToJson() const;
};

struct AppliedMicroDimmingRecord {
    uint32_t recordId{0};
    uint64_t startTimeMs{0};
    uint64_t endTimeMs{0};
    std::string timeCodeRange;
    std::string propName;
    std::string controllerPort;
    float originalPeakWatts{0.0f};
    float throttledPeakWatts{0.0f};
    float reductionPercent{0.0f};
    std::string appliedCurvePayload; // e.g. "Type=Custom;CustomData=0:1.0|0.5:0.75|1.0:1.0"
    bool isManuallyOverridden{false};

    nlohmann::json ToJson() const;
};

struct ThermalSimulationParameters {
    ThrottlerOperationMode mode{ThrottlerOperationMode::MODE_1_INCIDENT_AUDIT};
    float psuRatedWattage{350.0f};          // Watts
    float maxContinuousAmps{30.0f};         // Amps
    float maxSafeTemperatureC{70.0f};       // Celsius
    float ambientTemperatureC{20.0f};       // Celsius
    float pixelVoltageVolts{12.0f};         // 5V, 12V, 24V
    float fullWhiteAmpPerPixel{0.06f};      // 60mA per RGB pixel
    bool enableAutoMicroDimmingOverride{false}; // Strict intentional toggle for Mode 3
    float targetSafetyMarginPercent{20.0f}; // 80% continuous rating rule
};

struct ThermalSimulationResult {
    ThrottlerOperationMode executedMode{ThrottlerOperationMode::MODE_1_INCIDENT_AUDIT};
    std::vector<ThermalIncidentReport> incidents;
    std::vector<AppliedMicroDimmingRecord> appliedMicroDimmings;
    float maxShowAmps{0.0f};
    float maxShowWatts{0.0f};
    float maxShowTempC{20.0f};
    int totalIncidentsDetected{0};
    int totalCurvesAutoApplied{0};
    bool safetyCompliancePassed{false};

    std::string GenerateFormattedReportText() const;
    nlohmann::json ToJson() const;
};

class ThermalSafetyThrottlerAI {
public:
    ThermalSafetyThrottlerAI() = default;
    ~ThermalSafetyThrottlerAI() = default;

    /// Runs electrical, wattage, and Joule-heating thermal simulation across sequence frames
    static ThermalSimulationResult RunSimulation(
        int totalFrames,
        int frameIntervalMs,
        const std::vector<std::string>& activeProps,
        const ThermalSimulationParameters& params
    );

    /// Formats milliseconds into standard SMPTE timecode (HH:MM:SS.mmm)
    static std::string FormatTimecode(uint64_t timeMs);

    /// Generates targeted micro-dimming curve string for an overload duration
    static std::string GenerateDimmingCurvePayload(float requiredReductionFraction);

    /// Manually edits/adjusts an applied micro-dimming curve record
    static void UpdateAppliedCurve(
        ThermalSimulationResult& result,
        uint32_t recordId,
        const std::string& newCurvePayload
    );

    /// Reverts/rolls back an individual applied micro-dimming curve
    static void RevertAppliedCurve(
        ThermalSimulationResult& result,
        uint32_t recordId
    );
};

} // namespace xLights
