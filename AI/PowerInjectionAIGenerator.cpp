/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #11: AI Power Injection & Load Balancer Implementation

#include "PowerInjectionAIGenerator.h"
#include <spdlog/spdlog.h>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace xLights::AI {

static float GetWireResistancePerFoot(float awg) {
    if (awg <= 14.0f) return 0.002525f;
    if (awg <= 16.0f) return 0.004016f;
    if (awg <= 18.0f) return 0.006385f;
    if (awg <= 20.0f) return 0.01015f;
    return 0.01614f; // 22 AWG or smaller
}

PowerInjectionResult PowerInjectionAIGenerator::CalculatePowerInjection(const PowerInjectionConfig& config) {
    PowerInjectionResult result;
    result.modelName = config.modelName;
    float vSupply = config.supplyVoltage;
    if (config.voltageType == PixelVoltage::V5) vSupply = 5.0f;
    else if (config.voltageType == PixelVoltage::V24) vSupply = 24.0f;

    if (config.totalPixels <= 0 || vSupply <= 0.0f) {
        result.success = false;
        result.errorMessage = "Invalid totalPixels or supplyVoltage in PowerInjectionConfig.";
        spdlog::error("PowerInjectionAIGenerator: {}", result.errorMessage);
        return result;
    }

    float rPerFoot = GetWireResistancePerFoot(config.wireGaugeAWG);
    float totalCurrent = config.totalPixels * config.maxCurrentPerPixelAmps;
    float totalPower = totalCurrent * config.supplyVoltage;

    result.totalCurrentAmps = totalCurrent;
    result.totalPowerWatts = totalPower;
    result.recommendedAmperage = (totalCurrent <= 5.0f) ? 5.0f : ((totalCurrent <= 10.0f) ? 10.0f : ((totalCurrent <= 15.0f) ? 15.0f : 20.0f));

    // Calculate voltage drop along string without injection
    // Voltage drop V_drop = 2 * (Wire_Length_Per_Node * R_per_foot) * I_accumulated
    float wireLengthPerNode = (config.wireLengthFeet > 0.0f) ? (config.wireLengthFeet / config.totalPixels) : 0.2f;
    float currentV = config.supplyVoltage;
    float currentR = 2.0f * wireLengthPerNode * rPerFoot; // Round-trip (+ and -)

    std::vector<float> voltageAtPixel(config.totalPixels, config.supplyVoltage);
    float remainingCurrent = totalCurrent;

    for (int p = 0; p < config.totalPixels; ++p) {
        currentV -= remainingCurrent * currentR;
        voltageAtPixel[p] = std::max(0.0f, currentV);
        remainingCurrent -= config.maxCurrentPerPixelAmps;
    }

    result.endVoltageNoInjection = voltageAtPixel.back();

    // Determine injection tap locations where voltage drops below minRequiredVoltage
    PowerInjectionTap frontTap;
    frontTap.pixelIndex = 1;
    frontTap.calculatedVoltage = config.supplyVoltage;
    frontTap.calculatedCurrentAmps = totalCurrent;
    frontTap.recommendedAmperage = (totalCurrent <= 5.0f) ? 5.0f : ((totalCurrent <= 10.0f) ? 10.0f : 15.0f);
    frontTap.tapType = "Front";
    frontTap.recommendedWireAWG = (config.wireGaugeAWG <= 14.0f) ? "14 AWG" : ((config.wireGaugeAWG <= 16.0f) ? "16 AWG" : "18 AWG");
    frontTap.wireGauge = frontTap.recommendedWireAWG;
    frontTap.fuse.recommendedAmperage = frontTap.recommendedAmperage;
    frontTap.fuse.fuseType = "ATC/ATO";
    frontTap.fuse.notes = "Front power feed inline fuse";
    result.injectionTaps.push_back(frontTap);

    int tapCount = 1;
    for (int p = 0; p < config.totalPixels; ++p) {
        if (voltageAtPixel[p] < config.minRequiredVoltage) {
            // Need injection tap at pixel p + 1
            PowerInjectionTap tap;
            tap.pixelIndex = p + 1;
            tap.calculatedVoltage = config.minRequiredVoltage;
            tap.calculatedCurrentAmps = (config.totalPixels - p) * config.maxCurrentPerPixelAmps;
            tap.recommendedAmperage = (tap.calculatedCurrentAmps <= 5.0f) ? 5.0f : ((tap.calculatedCurrentAmps <= 10.0f) ? 10.0f : 15.0f);
            tap.tapType = (p == config.totalPixels - 1) ? "End-String" : "Mid-String";
            tap.recommendedWireAWG = frontTap.recommendedWireAWG;
            tap.wireGauge = frontTap.recommendedWireAWG;
            tap.fuse.recommendedAmperage = tap.recommendedAmperage;
            tap.fuse.fuseType = "ATC/ATO";
            tap.fuse.notes = "Injection lead inline fuse";
            result.injectionTaps.push_back(tap);
            tapCount++;
            // Reset voltage after tap
            for (int k = p; k < config.totalPixels; ++k) {
                voltageAtPixel[k] = config.supplyVoltage;
            }
        }
    }

    result.requiredTapsCount = tapCount;

    std::ostringstream summary;
    summary << "Power Injection Analysis: " << config.totalPixels << " pixels @ " << std::fixed << std::setprecision(1)
            << config.supplyVoltage << "V draws " << std::setprecision(2) << totalCurrent << "A (" << totalPower << "W). "
            << "End voltage without injection: " << result.endVoltageNoInjection << "V. "
            << "Requires " << tapCount << " power injection tap(s) using " << frontTap.recommendedWireAWG << " wire.";
    result.recommendationSummary = summary.str();

    result.success = true;
    spdlog::info("PowerInjectionAIGenerator: {}", result.recommendationSummary);
    return result;
}

PowerAnalysisResult PowerInjectionAIGenerator::AnalyzePowerDistribution(const PowerDistributionConfig& config) {
    return CalculatePowerInjection(config);
}

} // namespace xLights::AI
