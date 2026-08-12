#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #11: AI Power Injection & Load Balancer Subsystem

#include "AISubsystemBase.h"
#include <string>
#include <vector>
#include <future>

enum class PixelVoltage {
    V5,
    V12,
    V24
};

struct PowerInjectionConfig {
    PixelVoltage voltageType = PixelVoltage::V12; // Pixel supply voltage type
    float supplyVoltage = 12.0f;           // Supply voltage (e.g. 5.0V or 12.0V)
    float maxCurrentPerPixelAmps = 0.05f;  // Current per pixel in Amps (e.g. 0.05A = 50mA @ 100% white)
    float wireGaugeAWG = 18.0f;            // e.g., 14, 16, 18 AWG
    float wireLengthFeet = 25.0f;          // Length of main wire run in feet
    int totalPixels = 100;                 // Number of pixels in string
    float minRequiredVoltage = 9.6f;       // Minimum required operating voltage (e.g. 80% threshold = 9.6V)
};

struct PowerInjectionTap {
    int pixelIndex = 0;                    // Pixel index (1-based) where power tap is inserted
    float calculatedVoltage = 0.0f;        // Voltage at this tap
    float calculatedCurrentAmps = 0.0f;    // Current draw at this tap
    float recommendedAmperage = 0.0f;      // e.g., 5A, 10A, 15A fuse rating
    std::string tapType;                   // "Front", "Mid-String", "End-String"
    std::string recommendedWireAWG;        // Recommended wire AWG e.g. "18 AWG"
};

struct PowerInjectionResult {
    bool success = false;
    std::string errorMessage;
    float totalCurrentAmps = 0.0f;
    float totalPowerWatts = 0.0f;
    float recommendedAmperage = 0.0f;      // e.g., 5A, 10A, 15A total power supply rating recommendation
    float endVoltageNoInjection = 0.0f;
    int requiredTapsCount = 0;
    std::vector<PowerInjectionTap> injectionTaps;
    std::string recommendationSummary;
};

using PowerDistributionConfig = PowerInjectionConfig;
using PowerAnalysisResult = PowerInjectionResult;

class PowerInjectionAIGenerator : public AISubsystemBase {
public:
    PowerInjectionAIGenerator(ServiceManager* sm = nullptr) : AISubsystemBase(sm) {}
    virtual ~PowerInjectionAIGenerator() override = default;

    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override {
        return std::async(std::launch::async, [this, callback]() {
            if (callback) callback("Initializing PowerInjectionAIGenerator...", 0.0f);
            m_isInitialized.store(true);
            if (callback) callback("PowerInjectionAIGenerator initialized.", 100.0f);
            return true;
        });
    }

    virtual void Shutdown() override {
        m_isInitialized.store(false);
    }

    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "PowerInjectionAIGenerator"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"power_injection_calculation", "ohms_law_voltage_drop", "load_balancing_recommendation"};
    }

    // Calculates Ohm's Law voltage drop and power injection tap points across pixel strings
    [[nodiscard]] static PowerInjectionResult CalculatePowerInjection(const PowerInjectionConfig& config);
    [[nodiscard]] static PowerAnalysisResult AnalyzePowerDistribution(const PowerDistributionConfig& config);
};

} // namespace xLights::AI
