/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Catch2 Unit Tests for SequenceValidatorAI

#include <catch2/catch_test_macros.hpp>
#include "SequenceValidatorAI.h"
#include <nlohmann/json.hpp>

using namespace xLights::AI;

TEST_CASE("SequenceValidatorAI: Sequence Quality & Diagnostic Audit", "[SequenceValidatorAI]") {

    SECTION("Validates sequence diagnostics and exports report JSON") {
        SequenceValidationConfig config;
        config.totalDurationMs = 45000;
        config.activeEffectCount = 3;
        config.checkTimingGaps = true;

        SequenceValidationResult result = SequenceValidatorAI::ValidateSequenceDiagnostics(config);
        REQUIRE(result.success == true);
        REQUIRE(result.totalIssuesCount >= 2);

        std::string jsonStr = SequenceValidatorAI::ExportValidationReportJSON(result);
        REQUIRE(!jsonStr.empty());

        nlohmann::json parsed = nlohmann::json::parse(jsonStr);
        REQUIRE(parsed["success"] == true);
        REQUIRE(parsed.contains("issues"));
    }

    SECTION("Generates direct Master Sequencer Boss critique") {
        SequenceValidationConfig config;
        config.totalDurationMs = 60000;
        config.activeEffectCount = 2500;
        config.reviewMode = PersonaReviewMode::MASTER_SEQUENCER_BOSS;

        SequenceValidationResult result = SequenceValidatorAI::ValidateSequenceDiagnostics(config);
        REQUIRE(result.success == true);
        REQUIRE(!result.personaCritiqueBody.empty());
        REQUIRE(result.personaCritiqueBody.find("MASTER SEQUENCER BOSS CRITIQUE") != std::string::npos);
        REQUIRE(result.personaCritiqueBody.find("DIRECT AUDIT FINDINGS") != std::string::npos);
    }

    SECTION("RunComprehensiveAudit alias method works") {
        SequenceValidationConfig config;
        config.totalDurationMs = 30000;
        config.activeEffectCount = 500;

        ComprehensiveAuditReport report = SequenceValidatorAI::RunComprehensiveAudit(config);
        REQUIRE(report.success == true);
    }

    SECTION("RemediateSequenceIssues auto-repair engine works") {
        std::string err;
        std::vector<std::string> targetIds = {"VAL-1", "VAL-2"};
        bool ok = SequenceValidatorAI::RemediateSequenceIssues("test_seq.xml", targetIds, err);
        REQUIRE(ok == true);
    }
}
