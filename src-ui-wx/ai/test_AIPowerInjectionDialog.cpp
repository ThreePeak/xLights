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
        xLights::AI::PowerInjectionConfig config;
        config.pixelCount = 300;
        config.supplyVoltage = 12.0f;
        config.wireGaugeAWG = 18;

        xLights::AI::PowerInjectionResult result = xLights::AI::PowerInjectionAnalyzer::CalculatePowerInjection(config);

        REQUIRE(result.totalCurrentAmps > 0.0f);
        REQUIRE(result.totalPowerWatts > 0.0f);
        REQUIRE(result.endOfLineVoltage < 12.0f);
    }

    SECTION("Flag Power Injection requirement for long runs") {
        xLights::AI::PowerInjectionConfig config;
        config.pixelCount = 600;
        config.supplyVoltage = 5.0f;
        config.wireGaugeAWG = 22;

        xLights::AI::PowerInjectionResult result = xLights::AI::PowerInjectionAnalyzer::CalculatePowerInjection(config);

        REQUIRE(result.requiresInjection == true);
    }
}
