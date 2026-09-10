/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <catch2/catch_test_macros.hpp>
#include "AI/PowerInjectionAnalyzer.h"

TEST_CASE("AI Power Injection Inspector Backend Validation Tests", "[AI][Power]") {
    SECTION("Calculate Power Injection for 12V 300 Pixel Run") {
        xLights::AI::PowerDistributionConfig config;
        config.totalPixelCount = 300;
        config.voltage = xLights::AI::PixelVoltage::V12;
        config.wireGaugeAWG = 18.0f;

        xLights::AI::PowerAnalysisResult result = xLights::AI::PowerInjectionAnalyzer::AnalyzePowerDistribution(config);

        REQUIRE(result.estimatedMaxCurrentAmps > 0.0f);
        REQUIRE(result.calculatedVoltageDropPercent >= 0.0f);
        REQUIRE(!result.requiredInjectionNodeIndices.empty());
    }

    SECTION("Flag Power Injection requirement for long runs") {
        xLights::AI::PowerDistributionConfig config;
        config.totalPixelCount = 600;
        config.voltage = xLights::AI::PixelVoltage::V5;
        config.wireGaugeAWG = 22.0f;

        xLights::AI::PowerAnalysisResult result = xLights::AI::PowerInjectionAnalyzer::AnalyzePowerDistribution(config);

        REQUIRE(!result.requiredInjectionNodeIndices.empty());
    }
}
