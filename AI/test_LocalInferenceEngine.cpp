/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #15 Catch2 Unit Tests for LocalInferenceEngine

#include <catch2/catch_test_macros.hpp>
#include "LocalInferenceEngine.h"

using namespace xLights::AI;

TEST_CASE("LocalInferenceEngine: Hardware-Accelerated Local Inference Engine", "[LocalInferenceEngine]") {

    SECTION("Enumerates hardware execution devices") {
        auto devices = LocalInferenceEngine::EnumerateHardwareDevices();
        REQUIRE(!devices.empty());
        REQUIRE(devices[0].isAvailable == true);
        REQUIRE(!devices[0].deviceName.empty());
    }

    SECTION("Creates hardware inference session with INT8 quantization") {
        InferenceSessionConfig config;
        config.modelFilePath = "models/demucs_quantized.onnx";
        config.preferredProvider = ExecutionProvider::DirectML;
        config.enableQuantizationINT8 = true;

        InferenceSessionStatus status = LocalInferenceEngine::CreateInferenceSession(config);
        REQUIRE(status.isLoaded == true);
        REQUIRE(!status.activeProviderName.empty());
        REQUIRE(status.avgInferenceLatencyMs < 5.0f);
        REQUIRE(status.memoryAllocatedBytes > 0);
    }
}
