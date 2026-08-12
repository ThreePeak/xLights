/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// STEP 2: ELECTRICAL CALCULATION ENGINE (xLights/AI/PowerInjectionAnalyzer.cpp)

#include "PowerInjectionAnalyzer.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace xLights::AI {

static float GetAWGResistancePerFoot(float awg) {
    if (awg <= 14.0f) return 0.002525f;
    if (awg <= 16.0f) return 0.004016f;
    if (awg <= 18.0f) return 0.006385f;
    if (awg <= 20.0f) return 0.01015f;
    return 0.01614f; // 22 AWG or smaller
}

PowerAnalysisResult PowerInjectionAnalyzer::AnalyzePowerDistribution(const PowerDistributionConfig& config) {
    PowerAnalysisResult result;
    result.modelName = config.modelName;

    if (config.totalPixelCount <= 0) {
        result.success = false;
        result.errorMessage = "totalPixelCount must be > 0";
        spdlog::error("PowerInjectionAnalyzer: {}", result.errorMessage);
        return result;
    }

    float baseVoltage = 12.0f;
    float currentPerPixelAt100 = 0.06f; // 12V WS2811 ~ 60mA per pixel @ 100% white
    if (config.voltage == PixelVoltage::V5) {
        baseVoltage = 5.0f;
        currentPerPixelAt100 = 0.05f; // 5V WS2812B ~ 50mA per pixel @ 100% white
    } else if (config.voltage == PixelVoltage::V24) {
        baseVoltage = 24.0f;
        currentPerPixelAt100 = 0.03f;
    }

    float cap = (config.maxPowerPercentage > 0.0f && config.maxPowerPercentage <= 1.0f)
                ? config.maxPowerPercentage : 1.0f;

    // Calculate maximum current draw scaled by power cap percentage
    float currentDrawPerPixel = currentPerPixelAt100 * cap;
    float totalPixelCurrent = config.totalPixelCount * currentDrawPerPixel;

    // Account for buck converters dropping 12V main power down to 5V for controller logic boards (ESP32)
    float logicBoardCurrent = config.usesBuckConverters ? 0.2f : 0.0f;
    result.estimatedMaxCurrentAmps = totalPixelCurrent + logicBoardCurrent;

    // Compute copper wire voltage drop across feedWireLengthFeet using AWG resistance tables
    float rPerFoot = GetAWGResistancePerFoot(config.wireGaugeAWG);
    float roundTripR = 2.0f * config.feedWireLengthFeet * rPerFoot; // Feed wire round trip (+ and -)
    float feedVoltageDrop = result.estimatedMaxCurrentAmps * roundTripR;
    result.calculatedVoltageDropPercent = (feedVoltageDrop / baseVoltage) * 100.0f;

    // Enforce safety rule: For 12V pixel strings, recommend power injection every 100 to 150 pixels
    int injectionInterval = (config.voltage == PixelVoltage::V5) ? 50 : 100;
    
    // Front feed fuse recommendation
    FuseRecommendation frontFuse;
    frontFuse.placementLocationNode = 1;
    float frontCurrent = std::min(result.estimatedMaxCurrentAmps, static_cast<float>(injectionInterval) * currentDrawPerPixel);
    frontFuse.recommendedAmperage = (frontCurrent <= 5.0f) ? 5.0f : ((frontCurrent <= 10.0f) ? 10.0f : 15.0f);
    frontFuse.fuseType = "In-line blade";
    frontFuse.notes = "Front feed line fuse to protect wire insulation";
    result.fuseSpecifications.push_back(frontFuse);

    // Calculate required injection tap points
    for (int node = injectionInterval + 1; node <= config.totalPixelCount; node += injectionInterval) {
        result.requiredInjectionNodeIndices.push_back(node);

        FuseRecommendation injFuse;
        injFuse.placementLocationNode = node;
        float tapCurrent = std::min(result.estimatedMaxCurrentAmps, static_cast<float>(injectionInterval) * currentDrawPerPixel);
        injFuse.recommendedAmperage = (tapCurrent <= 5.0f) ? 5.0f : ((tapCurrent <= 10.0f) ? 10.0f : 15.0f);
        injFuse.fuseType = "In-line blade";
        injFuse.notes = "Injection point inline blade fuse";
        result.fuseSpecifications.push_back(injFuse);
    }

    // Power supply recommendation
    float recommendedPSUAmps = std::ceil((result.estimatedMaxCurrentAmps * 1.25f) / 5.0f) * 5.0f;
    std::ostringstream psuStream;
    psuStream << static_cast<int>(baseVoltage) << "V " << static_cast<int>(recommendedPSUAmps) << "A PSU sufficient";
    result.powerSupplyRecommendation = psuStream.str();

    // Safety warning if voltage drop > 15% or un-injected nodes > interval
    if (result.calculatedVoltageDropPercent > 15.0f || !result.requiredInjectionNodeIndices.empty()) {
        std::ostringstream warn;
        warn << "WARNING: Feed wire voltage drop is " << std::fixed << std::setprecision(1)
             << result.calculatedVoltageDropPercent << "%. "
             << result.requiredInjectionNodeIndices.size() << " power injection tap(s) required to maintain 80% operating voltage.";
        result.safetyWarning = warn.str();
    }

    std::ostringstream summary;
    summary << "Power Distribution Analysis for '" << (config.modelName.empty() ? "Model" : config.modelName) << "': "
            << config.totalPixelCount << " pixels @ " << std::fixed << std::setprecision(1) << baseVoltage << "V. "
            << "Max current: " << std::setprecision(2) << result.estimatedMaxCurrentAmps << "A. "
            << "PSU: " << result.powerSupplyRecommendation << ". "
            << "Required injection points: " << result.requiredInjectionNodeIndices.size() << ".";
    result.recommendationSummary = summary.str();

    result.success = true;
    spdlog::info("PowerInjectionAnalyzer: {}", result.recommendationSummary);
    return result;
}

std::string PowerInjectionAnalyzer::ExportPowerReportJSON(const PowerAnalysisResult& result) {
    nlohmann::json root;
    root["success"] = result.success;
    root["modelName"] = result.modelName;
    root["errorMessage"] = result.errorMessage;
    root["estimatedMaxCurrentAmps"] = result.estimatedMaxCurrentAmps;
    root["calculatedVoltageDropPercent"] = result.calculatedVoltageDropPercent;
    root["requiredInjectionNodeIndices"] = result.requiredInjectionNodeIndices;
    root["powerSupplyRecommendation"] = result.powerSupplyRecommendation;
    root["safetyWarning"] = result.safetyWarning;
    root["recommendationSummary"] = result.recommendationSummary;

    nlohmann::json fuses = nlohmann::json::array();
    for (const auto& fuse : result.fuseSpecifications) {
        nlohmann::json f;
        f["placementLocationNode"] = fuse.placementLocationNode;
        f["recommendedAmperage"] = fuse.recommendedAmperage;
        f["fuseType"] = fuse.fuseType;
        f["notes"] = fuse.notes;
        fuses.push_back(f);
    }
    root["fuseSpecifications"] = fuses;

    return root.dump(2);
}

} // namespace xLights::AI
