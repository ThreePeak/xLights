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
#include <deque>
#include <cstdint>

namespace xLights::AI {

enum class TelemetryIncidentType {
    VOLTAGE_SAG_BROWNOUT,
    CURRENT_OVERLOAD_FUSE,
    CPU_OVERHEAT,
    WIFI_LINK_DEGRADATION
};

struct TelemetrySample {
    uint32_t timestampMs{0};
    double voltageRailVolts{12.0};
    double currentAmps{5.0};
    double cpuTempCelsius{45.0};
    int wifiRssiDbm{-55};
    double currentFps{40.0};
};

struct PredictiveThrottlingIncident {
    uint32_t timestampMs{0};
    TelemetryIncidentType type{TelemetryIncidentType::VOLTAGE_SAG_BROWNOUT};
    double timeToFailureSeconds{15.0};
    double observedValue{0.0};
    double safeThreshold{0.0};
    double recommendedThrottlePercent{15.0}; // Dimming reduction (e.g. -15%)
    std::string explanation;
    std::string remediationAction;
};

struct ESP32TelemetryAuditReport {
    std::string controllerName{"ESP32_DigOcta_Roofline"};
    int totalSamples{0};
    double minVoltageObserved{12.0};
    double maxCurrentObserved{0.0};
    double maxCpuTempObserved{0.0};
    int avgRssiDbm{-55};
    double suggestedGlobalThrottlePercent{0.0};
    bool safetyShutdownImminent{false};
    std::vector<PredictiveThrottlingIncident> incidents;

    std::string GenerateFormattedReport() const;
};

class AIESP32TelemetryMonitor {
public:
    AIESP32TelemetryMonitor(double nominalVolts = 12.0, double maxAmps = 30.0, double maxTempC = 75.0);
    ~AIESP32TelemetryMonitor() = default;

    void SetNominalVoltage(double volts) { m_nominalVoltage = volts; }
    void SetMaxAmps(double amps) { m_maxAmps = amps; }
    void SetMaxTempCelsius(double tempC) { m_maxTempCelsius = tempC; }

    void IngestSample(const TelemetrySample& sample);
    void Clear();

    ESP32TelemetryAuditReport EvaluateSafety(const std::string& controllerName = "ESP32_Controller");
    ESP32TelemetryAuditReport SimulateLiveShowTelemetry(int sampleCount, bool injectOverloadFault);

    static std::string GetIncidentTypeName(TelemetryIncidentType type);

private:
    double m_nominalVoltage{12.0};
    double m_maxAmps{30.0};
    double m_maxTempCelsius{75.0};
    std::deque<TelemetrySample> m_samples;
};

} // namespace xLights::AI
