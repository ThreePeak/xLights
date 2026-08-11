/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "controllers/PowerInjectionAnalyzer.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>
#include <sstream>

float PowerInjectionAnalyzer::WireResistancePerMeter(int awg) {
    switch (awg) {
        case 14: return 0.00828f;
        case 16: return 0.01317f;
        case 18: return 0.02095f;
        case 20: return 0.03330f;
        case 22: return 0.05290f;
        case 24: return 0.08420f;
        default: return 0.02095f; // Default 18 AWG
    }
}

PowerAnalysisResult PowerInjectionAnalyzer::AnalyzeStringVoltageDrop(const PixelStringPowerConfig& config) {
    PowerAnalysisResult result;
    if (config.pixelCount <= 0 || config.nominalVoltage <= 0.0f) {
        result.summaryReport = "Invalid string configuration.";
        return result;
    }

    float wireRPerMeter = WireResistancePerMeter(config.wireGaugeAWG);
    float leadR = config.leadWireLengthMeters * wireRPerMeter * 2.0f; // Round trip (+V and GND)
    float pixelR = config.pixelSpacingMeters * wireRPerMeter * 2.0f;

    result.totalCurrentAmps = config.pixelCount * config.pixelCurrentAmps;
    result.totalPowerWatts = result.totalCurrentAmps * config.nominalVoltage;

    float minAllowedVoltage = config.nominalVoltage * config.minOperatingVoltagePct;
    float currentVoltage = config.nominalVoltage;
    result.minVoltage = config.nominalVoltage;

    result.voltageProfile.reserve(config.pixelCount);

    // Initial lead wire drop
    float leadDrop = result.totalCurrentAmps * leadR;
    currentVoltage -= leadDrop;

    for (int i = 1; i <= config.pixelCount; ++i) {
        // Amperage downstream of pixel i
        float remainingAmps = (config.pixelCount - i + 1) * config.pixelCurrentAmps;
        float stepDrop = remainingAmps * pixelR;
        currentVoltage -= stepDrop;

        VoltageDropPoint point;
        point.pixelIndex = i;
        point.voltage = std::max(0.0f, currentVoltage);
        point.voltageDrop = config.nominalVoltage - point.voltage;
        point.isBrownout = (point.voltage < minAllowedVoltage);

        if (point.isBrownout && result.firstBrownoutPixel == -1) {
            result.firstBrownoutPixel = i;
            result.requiresInjection = true;
        }

        if (point.voltage < result.minVoltage) {
            result.minVoltage = point.voltage;
        }

        result.voltageProfile.push_back(point);
    }

    if (result.requiresInjection) {
        result.recommendedInjections = CalculateOptimalInjections(config);
    }

    std::ostringstream ss;
    ss << "Pixel String Power Analysis:\n"
       << "  - Pixel Count: " << config.pixelCount << "\n"
       << "  - Total Current: " << result.totalCurrentAmps << " Amps (" << result.totalPowerWatts << " Watts)\n"
       << "  - Minimum End Voltage: " << result.minVoltage << "V (Nominal: " << config.nominalVoltage << "V)\n";

    if (result.requiresInjection) {
        ss << "  - STATUS: POWER INJECTION REQUIRED!\n"
           << "  - First Brownout at Pixel #" << result.firstBrownoutPixel << "\n"
           << "  - Recommended Injection Points: " << result.recommendedInjections.size() << "\n";
    } else {
        ss << "  - STATUS: OK (No power injection required)\n";
    }

    result.summaryReport = ss.str();
    result.success = true;

    spdlog::info("PowerInjectionAnalyzer: Analyzed {} pixels. Total current: {:.2f}A, Min voltage: {:.2f}V",
                 config.pixelCount, result.totalCurrentAmps, result.minVoltage);

    return result;
}

std::vector<PowerInjectionPoint> PowerInjectionAnalyzer::CalculateOptimalInjections(const PixelStringPowerConfig& config) {
    std::vector<PowerInjectionPoint> injections;
    float minAllowedVoltage = config.nominalVoltage * config.minOperatingVoltagePct;
    float wireRPerMeter = WireResistancePerMeter(config.wireGaugeAWG);
    float pixelR = config.pixelSpacingMeters * wireRPerMeter * 2.0f;

    // Calculate maximum pixels before voltage drop exceeds threshold
    int safePixelsPerSection = 0;
    float v = config.nominalVoltage;
    for (int p = 1; p <= config.pixelCount; ++p) {
        float remainingAmps = (config.pixelCount - p + 1) * config.pixelCurrentAmps;
        v -= remainingAmps * pixelR;
        if (v < minAllowedVoltage) {
            safePixelsPerSection = std::max(1, p - 1);
            break;
        }
    }

    if (safePixelsPerSection == 0) safePixelsPerSection = 50; // Fallback section length

    for (int idx = safePixelsPerSection; idx < config.pixelCount; idx += safePixelsPerSection) {
        PowerInjectionPoint pt;
        pt.pixelIndex = idx;
        pt.injectionReason = "Voltage drop below brownout threshold";
        pt.requiredCurrentAmps = (float)safePixelsPerSection * config.pixelCurrentAmps;
        injections.push_back(pt);
    }

    return injections;
}
