/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #4 Catch2 Unit Tests for ModelMappingAIGenerator

#include <catch2/catch_test_macros.hpp>
#include "ModelMappingAIGenerator.h"
#include <nlohmann/json.hpp>

using namespace xLights::AI;

TEST_CASE("ModelMappingAIGenerator: 4-Pass LLM Model Mapping Engine", "[ModelMappingAIGenerator]") {

    SECTION("Pass 1: Primary Model Name & Type Matching") {
        ModelMappingConfig config;
        
        SourceChannelSpec s1;
        s1.channelName = "Vendor_MegaTree_String1";
        s1.propTypeHint = "Tree";
        s1.nodeCount = 100;

        TargetModelSpec t1;
        t1.modelName = "MegaTree";
        t1.modelType = "Tree";
        t1.nodeCount = 100;

        config.sourceChannels.push_back(s1);
        config.targetModels.push_back(t1);

        ModelMappingResult result = ModelMappingAIGenerator::GenerateModelMapping(config);
        REQUIRE(result.success == true);
        REQUIRE(result.mappedChannelsCount == 1);
        REQUIRE(result.mappings[0].matchedPass == MappingPassStage::PrimaryModelMatching);
        REQUIRE(result.mappings[0].targetModelName == "MegaTree");
    }

    SECTION("Pass 2: Submodel Alignment (Eyes & Mouth)") {
        ModelMappingConfig config;

        SourceChannelSpec s1;
        s1.channelName = "SingingFace_Eyes";
        s1.nodeCount = 20;

        TargetModelSpec t1;
        t1.modelName = "SantaFace";
        t1.modelType = "Singing Face";
        t1.nodeCount = 100;
        t1.submodels = {"Eyes", "Mouth", "Outline"};

        config.sourceChannels.push_back(s1);
        config.targetModels.push_back(t1);

        ModelMappingResult result = ModelMappingAIGenerator::GenerateModelMapping(config);
        REQUIRE(result.success == true);
        REQUIRE(result.mappedChannelsCount == 1);
        REQUIRE(result.mappings[0].matchedPass == MappingPassStage::SubmodelChannelAlignment);
        REQUIRE(result.mappings[0].targetSubmodelName == "Eyes");
    }

    SECTION("Pass 3 & Pass 4 Fallback Engine & JSON Export") {
        ModelMappingConfig config;

        SourceChannelSpec s1;
        s1.channelName = "Custom_Strand_Set";
        s1.strandCount = 4;
        s1.nodeCount = 200;

        TargetModelSpec t1;
        t1.modelName = "Arch_Group";
        t1.strandCount = 4;
        t1.nodeCount = 200;

        config.sourceChannels.push_back(s1);
        config.targetModels.push_back(t1);

        ModelMappingResult result = ModelMappingAIGenerator::GenerateModelMapping(config);
        REQUIRE(result.success == true);
        REQUIRE(result.mappedChannelsCount == 1);

        std::string jsonStr = ModelMappingAIGenerator::ExportMappingReportJSON(result);
        REQUIRE(!jsonStr.empty());

        nlohmann::json parsed = nlohmann::json::parse(jsonStr);
        REQUIRE(parsed["success"] == true);
        REQUIRE(parsed.contains("mappings"));
        REQUIRE(parsed["mappings"].is_array());
    }
}
