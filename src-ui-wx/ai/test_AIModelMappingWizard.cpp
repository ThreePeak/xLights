/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <catch2/catch_test_macros.hpp>
#include "AI/ModelMappingAIGenerator.h"

TEST_CASE("AI Vendor Model Mapping Backend Validation Tests", "[AI][Mapping]") {
    SECTION("Run 4-Pass LLM Fuzzy Channel Alignment") {
        xLights::AI::ModelMappingConfig config;
        config.vendorSequencePath = "vendor_test.xsq";
        config.confidenceThreshold = 85.0f;

        xLights::AI::ModelMappingResult result = xLights::AI::ModelMappingAIGenerator::GenerateModelMappings(config);

        REQUIRE(result.isSuccess == true);
        REQUIRE(result.totalChannelsMapped > 0);
        REQUIRE(result.averageConfidenceScore > 80.0f);
    }
}
