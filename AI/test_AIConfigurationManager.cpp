/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <catch2/catch_test_macros.hpp>
#include "AI/AIConfigurationManager.h"

TEST_CASE("AIConfigurationManager Thread-Safe Operations", "[AI][Config]") {
    SECTION("Get Default Settings") {
        auto settings = xLights::AI::AIConfigurationManager::Instance().GetSettings();

        REQUIRE(settings.cpuThreads == 4);
        REQUIRE(settings.vramCapMb == 4096);
        REQUIRE(settings.temperature == 0.70f);
        REQUIRE(settings.topP == 0.95f);
        REQUIRE(settings.maxTokens == 4096);
        REQUIRE(settings.ollamaEndpoint == "http://localhost:11434");
    }

    SECTION("Update Settings and Retrieve Active API Key") {
        xLights::AI::AIConfigSettings newConfig;
        newConfig.primaryModel = 1; // Anthropic Claude
        newConfig.anthropicKey = "sk-ant-test-key-12345";
        newConfig.temperature = 0.85f;

        xLights::AI::AIConfigurationManager::Instance().UpdateSettings(newConfig);

        auto activeKey = xLights::AI::AIConfigurationManager::Instance().GetActiveApiKey();
        REQUIRE(activeKey == "sk-ant-test-key-12345");

        auto retrieved = xLights::AI::AIConfigurationManager::Instance().GetSettings();
        REQUIRE(retrieved.temperature == 0.85f);
    }
}
