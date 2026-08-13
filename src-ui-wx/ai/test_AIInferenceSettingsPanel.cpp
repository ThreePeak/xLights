/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <catch2/catch_test_macros.hpp>
#include "AI/LocalInferenceEngine.h"

TEST_CASE("AI Hardware Acceleration Preferences Backend Validation Tests", "[AI][Inference]") {
    SECTION("Local Hardware Acceleration Execution Provider Selection") {
        xLights::AI::InferenceConfig config;
        config.provider = xLights::AI::ExecutionProviderBackend::DIRECTML;
        config.vramMemoryCapMB = 4096;

        bool initialized = xLights::AI::LocalInferenceEngine::InitializeEngine(config);
        REQUIRE(initialized == true);

        xLights::AI::ExecutionProviderBackend active = xLights::AI::LocalInferenceEngine::GetActiveBackend();
        REQUIRE(active == xLights::AI::ExecutionProviderBackend::DIRECTML);
    }
}
