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
        xLights::AI::InferenceSessionConfig config;
        config.preferredProvider = xLights::AI::ExecutionProvider::DirectML;

        auto devices = xLights::AI::LocalInferenceEngine::EnumerateHardwareDevices();
        REQUIRE(devices.size() > 0);

        std::string providerName = xLights::AI::LocalInferenceEngine::ProviderToString(xLights::AI::ExecutionProvider::DirectML);
        REQUIRE(providerName == "DirectML");
    }
}
