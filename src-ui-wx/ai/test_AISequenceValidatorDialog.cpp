/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Unit test for AISequenceValidatorDialog structure & configuration bindings

#include <catch2/catch_test_macros.hpp>
#include "AI/SequenceValidatorAI.h"

using namespace xLights::AI;

TEST_CASE("AISequenceValidatorDialog Configuration & Data Binding", "[AISequenceValidatorDialog]") {
    SECTION("Validates SequenceValidationConfig binding for Dialog UI") {
        SequenceValidationConfig config;
        config.totalDurationMs = 60000;
        config.activeEffectCount = 120;
        config.personaMode = PersonaReviewMode::LIGHT_SHOW_JOURNALIST;

        SequenceValidationResult result = SequenceValidatorAI::ValidateSequenceDiagnostics(config);
        REQUIRE(result.success == true);
        REQUIRE(result.scorecard.overallHealthScore > 0.0f);
        REQUIRE(!result.personaCritiqueBody.empty());
    }
}
