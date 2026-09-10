/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <catch2/catch_test_macros.hpp>
#include "AI/AudioStemExtractor.h"

TEST_CASE("AI Audio Stem Extractor Backend Validation Tests", "[AI][Audio]") {
    SECTION("Neural Audio Stem Separation & Timing Grid Generation") {
        xLights::AI::AudioStemConfig config;
        config.extractVocals = true;
        config.extractDrums = true;
        config.extractBass = true;
        config.generateBpmTimings = true;

        xLights::AI::AudioStemResult result = xLights::AI::AudioStemExtractor::ExtractStems("test_audio.wav", config);

        REQUIRE(result.success == true);
        REQUIRE(result.stemFiles.size() >= 3);
        REQUIRE(result.detectedBpm > 0.0f);
    }
}
