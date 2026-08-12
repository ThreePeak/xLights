/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Catch2 unit test suite for Feature #11 PowerInjectionAIGenerator

#include <catch2/catch_test_macros.hpp>
#include "PowerInjectionAIGenerator.h"

using namespace xLights::AI;

TEST_CASE("PowerInjectionAIGenerator: Voltage Drop & Tap Point Calculations", "[PowerInjectionAIGenerator]") {

    SECTION("Calculates total current, power, and tap points for 12V string") {
        PowerInjectionConfig config;
        config.supplyVoltage = 12.0f;
        config.maxCurrentPerPixelAmps = 0.05f; // 50mA
        config.wireGaugeAWG = 18.0f;
        config.wireLengthFeet = 30.0f;
        config.totalPixels = 100;
        config.minRequiredVoltage = 9.6f;

        PowerInjectionResult result = PowerInjectionAIGenerator::CalculatePowerInjection(config);
        REQUIRE(result.success == true);
        REQUIRE(result.totalCurrentAmps == 5.0f); // 100 * 0.05A = 5.0A
        REQUIRE(result.totalPowerWatts == 60.0f); // 5.0A * 12V = 60W
        REQUIRE(!result.injectionTaps.empty());
        REQUIRE(result.injectionTaps[0].tapType == "Front");
        REQUIRE(!result.recommendationSummary.empty());
    }

    SECTION("Handles invalid totalPixels gracefully") {
        PowerInjectionConfig config;
        config.totalPixels = 0;

        PowerInjectionResult result = PowerInjectionAIGenerator::CalculatePowerInjection(config);
        REQUIRE(result.success == false);
        REQUIRE(!result.errorMessage.empty());
    }
}
