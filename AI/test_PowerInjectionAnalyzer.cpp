/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #9 Catch2 Unit Tests for PowerInjectionAnalyzer

#include <catch2/catch_test_macros.hpp>
#include "PowerInjectionAnalyzer.h"
#include <nlohmann/json.hpp>

using namespace xLights::AI;

TEST_CASE("PowerInjectionAnalyzer: Electrical Constraints & Load Balancing", "[PowerInjectionAnalyzer]") {

    SECTION("A 1200-pixel Mega Tree split across outputs on 12V supply generates required injection points and fuse specs") {
        PowerDistributionConfig config;
        config.modelName = "MegaTree_1200";
        config.voltage = PixelVoltage::V12;
        config.totalPixelCount = 1200;
        config.feedWireLengthFeet = 30.0f;
        config.wireGaugeAWG = 18.0f;
        config.maxPowerPercentage = 0.50f; // 50% power cap
        config.usesBuckConverters = true;  // ESP32 logic buck converter

        PowerAnalysisResult result = PowerInjectionAnalyzer::AnalyzePowerDistribution(config);
        REQUIRE(result.success == true);
        REQUIRE(result.modelName == "MegaTree_1200");
        REQUIRE(result.estimatedMaxCurrentAmps > 30.0f);
        REQUIRE(!result.requiredInjectionNodeIndices.empty());
        REQUIRE(!result.fuseSpecifications.empty());
        REQUIRE(result.fuseSpecifications[0].placementLocationNode == 1);
        REQUIRE(!result.powerSupplyRecommendation.empty());
    }

    SECTION("ExportPowerReportJSON returns valid structured JSON") {
        PowerDistributionConfig config;
        config.modelName = "Arch_String";
        config.voltage = PixelVoltage::V12;
        config.totalPixelCount = 300;
        config.feedWireLengthFeet = 15.0f;

        PowerAnalysisResult result = PowerInjectionAnalyzer::AnalyzePowerDistribution(config);
        REQUIRE(result.success == true);

        std::string jsonStr = PowerInjectionAnalyzer::ExportPowerReportJSON(result);
        REQUIRE(!jsonStr.empty());

        nlohmann::json parsed = nlohmann::json::parse(jsonStr);
        REQUIRE(parsed["success"] == true);
        REQUIRE(parsed["modelName"] == "Arch_String");
        REQUIRE(parsed.contains("requiredInjectionNodeIndices"));
        REQUIRE(parsed.contains("fuseSpecifications"));
        REQUIRE(parsed["fuseSpecifications"].is_array());
    }

    SECTION("Handles invalid totalPixelCount gracefully") {
        PowerDistributionConfig config;
        config.totalPixelCount = 0;

        PowerAnalysisResult result = PowerInjectionAnalyzer::AnalyzePowerDistribution(config);
        REQUIRE(result.success == false);
        REQUIRE(!result.errorMessage.empty());
    }
}
