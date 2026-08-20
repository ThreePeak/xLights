/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/controllers/AIESP32TelemetryMonitor.h"
#include <spdlog/spdlog.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>

namespace xLights::AI {

AIESP32TelemetryMonitor::AIESP32TelemetryMonitor(double nominalVolts, double maxAmps, double maxTempC)
    : m_nominalVoltage(nominalVolts), m_maxAmps(maxAmps), m_maxTempCelsius(maxTempC) {
}

void AIESP32TelemetryMonitor::Clear() {
    m_samples.clear();
}

void AIESP32TelemetryMonitor::IngestSample(const TelemetrySample& sample) {
    m_samples.push_back(sample);
    if (m_samples.size() > 500) {
        m_samples.pop_front();
    }
}

std::string AIESP32TelemetryMonitor::GetIncidentTypeName(TelemetryIncidentType type) {
    switch (type) {
        case TelemetryIncidentType::VOLTAGE_SAG_BROWNOUT: return "Voltage Sag (Brownout Imminent)";
        case TelemetryIncidentType::CURRENT_OVERLOAD_FUSE: return "Current Overload (Fuse Trip Imminent)";
        case TelemetryIncidentType::CPU_OVERHEAT: return "ESP32 CPU Thermal Overheat";
        case TelemetryIncidentType::WIFI_LINK_DEGRADATION: return "WiFi RSSI Degradation";
        default: return "Unknown Incident";
    }
}

ESP32TelemetryAuditReport AIESP32TelemetryMonitor::EvaluateSafety(const std::string& controllerName) {
    ESP32TelemetryAuditReport report;
    report.controllerName = controllerName;
    report.totalSamples = static_cast<int>(m_samples.size());

    if (m_samples.empty()) {
        return report;
    }

    report.minVoltageObserved = 999.0;
    report.maxCurrentObserved = 0.0;
    report.maxCpuTempObserved = 0.0;
    long long totalRssi = 0;

    for (const auto& s : m_samples) {
        report.minVoltageObserved = std::min(report.minVoltageObserved, s.voltageRailVolts);
        report.maxCurrentObserved = std::max(report.maxCurrentObserved, s.currentAmps);
        report.maxCpuTempObserved = std::max(report.maxCpuTempObserved, s.cpuTempCelsius);
        totalRssi += s.wifiRssiDbm;
    }
    report.avgRssiDbm = static_cast<int>(totalRssi / m_samples.size());

    // Predictive derivative analysis over the last 10 samples
    if (m_samples.size() >= 5) {
        size_t n = m_samples.size();
        const auto& s_old = m_samples[n - 5];
        const auto& s_now = m_samples[n - 1];

        double dtSec = (s_now.timestampMs > s_old.timestampMs) ? (s_now.timestampMs - s_old.timestampMs) / 1000.0 : 1.0;
        if (dtSec > 0.0) {
            double dV_dt = (s_now.voltageRailVolts - s_old.voltageRailVolts) / dtSec;
            double dI_dt = (s_now.currentAmps - s_old.currentAmps) / dtSec;
            double dT_dt = (s_now.cpuTempCelsius - s_old.cpuTempCelsius) / dtSec;

            // Voltage sag check (brownout below 85% of nominal)
            double brownoutThreshold = m_nominalVoltage * 0.85;
            if (s_now.voltageRailVolts < (m_nominalVoltage * 0.92) || dV_dt < -0.15) {
                PredictiveThrottlingIncident inc;
                inc.timestampMs = s_now.timestampMs;
                inc.type = TelemetryIncidentType::VOLTAGE_SAG_BROWNOUT;
                inc.observedValue = s_now.voltageRailVolts;
                inc.safeThreshold = brownoutThreshold;
                inc.timeToFailureSeconds = (dV_dt < 0.0) ? std::max(1.0, (s_now.voltageRailVolts - brownoutThreshold) / (-dV_dt)) : 10.0;
                inc.recommendedThrottlePercent = 20.0;
                inc.explanation = "Rail voltage dropped to " + std::to_string(s_now.voltageRailVolts).substr(0,4) + "V (Rate: " + std::to_string(dV_dt).substr(0,4) + "V/s).";
                inc.remediationAction = "Send DDP brightness scale down -20% to prevent ESP32 MCU reset.";
                report.incidents.push_back(inc);
                report.suggestedGlobalThrottlePercent = std::max(report.suggestedGlobalThrottlePercent, 20.0);
            }

            // Current overload check
            if (s_now.currentAmps > (m_maxAmps * 0.88) || dI_dt > 1.5) {
                PredictiveThrottlingIncident inc;
                inc.timestampMs = s_now.timestampMs;
                inc.type = TelemetryIncidentType::CURRENT_OVERLOAD_FUSE;
                inc.observedValue = s_now.currentAmps;
                inc.safeThreshold = m_maxAmps;
                inc.timeToFailureSeconds = (dI_dt > 0.0) ? std::max(1.0, (m_maxAmps - s_now.currentAmps) / dI_dt) : 8.0;
                inc.recommendedThrottlePercent = 25.0;
                inc.explanation = "Current surged to " + std::to_string(s_now.currentAmps).substr(0,4) + "A (Rate: +" + std::to_string(dI_dt).substr(0,4) + "A/s).";
                inc.remediationAction = "Apply fast micro-dimming curve (-25%) to avoid blowing 30A fuse.";
                report.incidents.push_back(inc);
                report.suggestedGlobalThrottlePercent = std::max(report.suggestedGlobalThrottlePercent, 25.0);
            }

            // Temperature check
            if (s_now.cpuTempCelsius > (m_maxTempCelsius * 0.90) || dT_dt > 0.5) {
                PredictiveThrottlingIncident inc;
                inc.timestampMs = s_now.timestampMs;
                inc.type = TelemetryIncidentType::CPU_OVERHEAT;
                inc.observedValue = s_now.cpuTempCelsius;
                inc.safeThreshold = m_maxTempCelsius;
                inc.timeToFailureSeconds = (dT_dt > 0.0) ? std::max(1.0, (m_maxTempCelsius - s_now.cpuTempCelsius) / dT_dt) : 20.0;
                inc.recommendedThrottlePercent = 15.0;
                inc.explanation = "ESP32 junction temperature reached " + std::to_string(s_now.cpuTempCelsius).substr(0,4) + " °C.";
                inc.remediationAction = "Throttle frame rate or reduce pixel white intensity.";
                report.incidents.push_back(inc);
            }
        }
    }

    report.safetyShutdownImminent = (report.suggestedGlobalThrottlePercent >= 25.0);
    return report;
}

ESP32TelemetryAuditReport AIESP32TelemetryMonitor::SimulateLiveShowTelemetry(int sampleCount, bool injectOverloadFault) {
    Clear();
    for (int i = 0; i < sampleCount; ++i) {
        TelemetrySample s;
        s.timestampMs = i * 250; // 4 samples per sec
        s.currentFps = 40.0;
        s.wifiRssiDbm = -52 - (i % 6);

        if (injectOverloadFault && i > (sampleCount / 2)) {
            // Overload ramp
            double factor = static_cast<double>(i - (sampleCount / 2)) / static_cast<double>(sampleCount / 2);
            s.voltageRailVolts = m_nominalVoltage - (factor * 2.8); // Drop down to 9.2V
            s.currentAmps = (m_maxAmps * 0.7) + (factor * (m_maxAmps * 0.45)); // Surge to 34.5A
            s.cpuTempCelsius = 48.0 + (factor * 32.0); // Reach 80C
        } else {
            // Normal operation
            s.voltageRailVolts = m_nominalVoltage - 0.2 + (std::sin(i * 0.2) * 0.1);
            s.currentAmps = (m_maxAmps * 0.5) + (std::cos(i * 0.1) * 3.0);
            s.cpuTempCelsius = 46.0 + (std::sin(i * 0.05) * 4.0);
        }
        IngestSample(s);
    }
    return EvaluateSafety("QuinLED_DigOcta_Roofline");
}

std::string ESP32TelemetryAuditReport::GenerateFormattedReport() const {
    std::ostringstream ss;
    ss << "=================================================================\n";
    ss << "   xLights AI Real-Time ESP32 Telemetry & Predictive Safety     \n";
    ss << "=================================================================\n\n";
    ss << "Controller Name:        " << controllerName << "\n";
    ss << "Telemetry Samples:      " << totalSamples << " frames\n";
    ss << "Min Observed Voltage:   " << std::fixed << std::setprecision(2) << minVoltageObserved << " V\n";
    ss << "Peak Observed Current:  " << std::fixed << std::setprecision(2) << maxCurrentObserved << " A\n";
    ss << "Max CPU Temperature:    " << std::fixed << std::setprecision(1) << maxCpuTempObserved << " °C\n";
    ss << "Average WiFi RSSI:      " << avgRssiDbm << " dBm\n";
    ss << "Suggested Throttle:     -" << std::fixed << std::setprecision(0) << suggestedGlobalThrottlePercent << " %\n";
    ss << "Safety Shutdown Status: " << (safetyShutdownImminent ? "CRITICAL (Throttling Active)" : "NORMAL (Healthy)") << "\n\n";
    
    if (incidents.empty()) {
        ss << "Status: All telemetry parameters within safe operating thresholds.\n";
    } else {
        ss << "Predictive Incident Alerts:\n";
        ss << "-----------------------------------------------------------------\n";
        for (const auto& inc : incidents) {
            ss << "[!] " << AIESP32TelemetryMonitor::GetIncidentTypeName(inc.type) << "\n";
            ss << "    Time to Failure: ~" << std::fixed << std::setprecision(1) << inc.timeToFailureSeconds << " seconds\n";
            ss << "    Diagnosis:       " << inc.explanation << "\n";
            ss << "    Remediation:     " << inc.remediationAction << "\n\n";
        }
    }
    return ss.str();
}

} // namespace xLights::AI
