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
        config.minimumConfidenceThreshold = 0.5f;

        xLights::AI::SourceChannelSpec src;
        src.channelName = "Vendor_Tree";
        src.propTypeHint = "Tree";
        src.nodeCount = 100;
        config.sourceChannels.push_back(src);

        xLights::AI::TargetModelSpec tgt;
        tgt.modelName = "My_Tree";
        tgt.modelType = "Tree";
        tgt.nodeCount = 100;
        config.targetModels.push_back(tgt);

        xLights::AI::ModelMappingResult result = xLights::AI::ModelMappingAIGenerator::GenerateModelMapping(config);

        REQUIRE(result.success == true);
        REQUIRE(result.mappedChannelsCount > 0);
    }
}
