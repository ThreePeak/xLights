#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// STEP 1: C++ STRUCT & CLASS DECLARATION (xLights/AI/PowerInjectionAnalyzer.h)

#include "AISubsystemBase.h"
#include <string>
#include <vector>
#include <future>

namespace xLights::AI {

#ifndef PIXEL_VOLTAGE_ENUM_DEFINED
#define PIXEL_VOLTAGE_ENUM_DEFINED
enum class PixelVoltage {
    V5,
    V12,
    V24
};
#endif

struct FuseRecommendation {
    int placementLocationNode = 0;             // Node index where fuse is required
    float recommendedAmperage = 5.0f;         // e.g., 5A, 10A, 15A
    std::string fuseType = "In-line blade";    // "In-line blade", "PCB mounted", "Waterproof holder"
    std::string notes;
};

struct PowerDistributionConfig {
    std::string modelName;                     // Target xLights model name e.g. "MegaTree"
    PixelVoltage voltage = PixelVoltage::V12;  // Supply voltage type (5V, 12V, 24V)
    int totalPixelCount = 100;                 // Total pixel node count
    float feedWireLengthFeet = 25.0f;          // Distance from PSU to injection point
    float wireGaugeAWG = 18.0f;                // e.g. 14, 16, 18 AWG
    float maxPowerPercentage = 1.0f;           // e.g., 0.30 to 0.50 (30% to 50% power cap)
    bool usesBuckConverters = false;           // 12V to 5V logic step-down for controller logic (ESP32)
};

struct PowerAnalysisResult {
    bool success = false;
    std::string errorMessage;
    std::string modelName;
    float estimatedMaxCurrentAmps = 0.0f;
    float calculatedVoltageDropPercent = 0.0f;
    std::vector<int> requiredInjectionNodeIndices;
    std::vector<FuseRecommendation> fuseSpecifications;
    std::string powerSupplyRecommendation;    // e.g., "12V 30A PSU sufficient"
    std::string safetyWarning;
    std::string recommendationSummary;
};

class PowerInjectionAnalyzer : public AISubsystemBase {
public:
    PowerInjectionAnalyzer(ServiceManager* sm = nullptr) : AISubsystemBase(sm) {}
    virtual ~PowerInjectionAnalyzer() override = default;

    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override {
        return std::async(std::launch::async, [this, callback]() {
            if (callback) callback("Initializing PowerInjectionAnalyzer...", 0.0f);
            m_isInitialized.store(true);
            if (callback) callback("PowerInjectionAnalyzer initialized.", 100.0f);
            return true;
        });
    }

    virtual void Shutdown() override {
        m_isInitialized.store(false);
    }

    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "PowerInjectionAnalyzer"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"power_injection_analysis", "electrical_constraint_calculation", "fuse_recommendation", "json_report_export"};
    }

    // Analyze electrical constraints and compute required power injection points and fuse specs
    [[nodiscard]] static PowerAnalysisResult AnalyzePowerDistribution(const PowerDistributionConfig& config);

    // Export power distribution report into a markdown/JSON layout summary
    [[nodiscard]] static std::string ExportPowerReportJSON(const PowerAnalysisResult& result);
};

} // namespace xLights::AI
