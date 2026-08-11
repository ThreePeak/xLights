#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <vector>

struct PixelStringPowerConfig {
    int pixelCount = 100;
    float nominalVoltage = 12.0f;       // 5V, 12V, 24V
    float pixelCurrentAmps = 0.03f;     // Max current per pixel at 100% white (e.g. 0.03A @ 12V)
    int wireGaugeAWG = 18;              // 18 AWG, 20 AWG, 22 AWG
    float leadWireLengthMeters = 3.0f;  // Wire length from controller to 1st pixel
    float pixelSpacingMeters = 0.10f;   // 10cm / 4 inch spacing
    float minOperatingVoltagePct = 0.80f; // Brownout threshold (default 80%)
};

struct VoltageDropPoint {
    int pixelIndex = 0;
    float voltage = 12.0f;
    float voltageDrop = 0.0f;
    bool isBrownout = false;
};

struct PowerInjectionPoint {
    int pixelIndex = 0;
    std::string injectionReason;
    float requiredCurrentAmps = 0.0f;
};

struct PowerAnalysisResult {
    bool success = false;
    float totalCurrentAmps = 0.0f;
    float totalPowerWatts = 0.0f;
    float minVoltage = 12.0f;
    int firstBrownoutPixel = -1;
    bool requiresInjection = false;
    std::vector<VoltageDropPoint> voltageProfile;
    std::vector<PowerInjectionPoint> recommendedInjections;
    std::string summaryReport;
};

class PowerInjectionAnalyzer {
public:
    PowerInjectionAnalyzer() = default;
    ~PowerInjectionAnalyzer() = default;

    // Resistance in Ohms/meter for AWG wire sizes
    static float WireResistancePerMeter(int awg);

    // Compute cumulative Ohm's law voltage drop along a pixel string
    static PowerAnalysisResult AnalyzeStringVoltageDrop(const PixelStringPowerConfig& config);

    // Calculate optimal power injection node locations along a string
    static std::vector<PowerInjectionPoint> CalculateOptimalInjections(const PixelStringPowerConfig& config);
};
