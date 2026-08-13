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

    SECTION("API Key Resolution Across Providers") {
        xLights::AI::AIConfigSettings cfg;
        cfg.openAIKey = "sk-openai-123";
        cfg.anthropicKey = "sk-ant-456";
        cfg.geminiKey = "AIzaSy-789";
        cfg.deepSeekKey = "sk-ds-abc";

        cfg.primaryModel = 0; // OpenAI
        xLights::AI::AIConfigurationManager::Instance().UpdateSettings(cfg);
        REQUIRE(xLights::AI::AIConfigurationManager::Instance().GetActiveApiKey() == "sk-openai-123");

        cfg.primaryModel = 1; // Anthropic
        xLights::AI::AIConfigurationManager::Instance().UpdateSettings(cfg);
        REQUIRE(xLights::AI::AIConfigurationManager::Instance().GetActiveApiKey() == "sk-ant-456");

        cfg.primaryModel = 2; // Gemini
        xLights::AI::AIConfigurationManager::Instance().UpdateSettings(cfg);
        REQUIRE(xLights::AI::AIConfigurationManager::Instance().GetActiveApiKey() == "AIzaSy-789");

        cfg.primaryModel = 3; // DeepSeek
        xLights::AI::AIConfigurationManager::Instance().UpdateSettings(cfg);
        REQUIRE(xLights::AI::AIConfigurationManager::Instance().GetActiveApiKey() == "sk-ds-abc");
    }

    SECTION("Hyperparameter Bounds & Local Engine Settings") {
        xLights::AI::AIConfigSettings cfg;
        cfg.temperature = 1.85f;
        cfg.topP = 0.99f;
        cfg.maxTokens = 8192;
        cfg.quantizationPrecision = 1; // FP16
        cfg.cpuThreads = 8;

        xLights::AI::AIConfigurationManager::Instance().UpdateSettings(cfg);
        auto retrieved = xLights::AI::AIConfigurationManager::Instance().GetSettings();

        REQUIRE(retrieved.temperature == 1.85f);
        REQUIRE(retrieved.topP == 0.99f);
        REQUIRE(retrieved.maxTokens == 8192);
        REQUIRE(retrieved.quantizationPrecision == 1);
        REQUIRE(retrieved.cpuThreads == 8);
    }
}
