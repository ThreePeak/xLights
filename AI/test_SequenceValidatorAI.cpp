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

    SECTION("Generates Light Show Journalist review") {
        CategoryScorecard card;
        card.overallHealthScore = 95.0f;
        std::vector<SequenceIssue> issues;
        std::string review = SequenceValidatorAI::GenerateJournalistReview(card, issues);
        REQUIRE(!review.empty());
        REQUIRE(review.find("LIGHT SHOW CHRONICLE REVIEW") != std::string::npos);
    }

    SECTION("Fine-grained pre-run toggles and TargetScopeFilter") {
        SequenceValidationConfig config;
        config.totalDurationMs = 60000;
        config.activeEffectCount = 10;
        config.checkTimingGaps = false;
        config.checkPerformanceBottlenecks = false;

        config.scopeFilter.startMs = 1000;
        config.scopeFilter.endMs = 50000;
        config.scopeFilter.targetPropNames = {"MegaTree", "Roofline"};
        config.scopeFilter.ignorePropNames = {"BackgroundMatrix"};

        SequenceValidationResult result = SequenceValidatorAI::ValidateSequenceDiagnostics(config);
        REQUIRE(result.success == true);
        REQUIRE(result.passedAudit == true);
    }

    SECTION("Calculates CategoryScorecard including HardwareSafety") {
        std::vector<SequenceIssue> issues;
        SequenceIssue i1;
        i1.category = "HardwareSafety";
        i1.severity = ValidationIssueSeverity::Warning;
        issues.push_back(i1);

        CategoryScorecard card = SequenceValidatorAI::CalculateScorecard(issues);
        REQUIRE(card.hardwareSafetyScore < 100.0f);
        REQUIRE(card.overallHealthScore > 0.0f);
    }

    SECTION("AutoRemediateSequence XML repair engine works") {
        std::string rawXml = "<seq overlap=\"true\"><color>255,255,255</color></seq>";
        std::vector<SequenceIssue> issues;
        SequenceIssue i1;
        i1.category = "ChannelOverlap";
        i1.autoFixable = true;
        issues.push_back(i1);
        SequenceIssue i2;
        i2.category = "HardwareSafety";
        i2.autoFixable = true;
        issues.push_back(i2);

        std::string remediated = SequenceValidatorAI::AutoRemediateSequence(rawXml, issues);
        REQUIRE(remediated.find("overlap=\"false\"") != std::string::npos);
        REQUIRE(remediated.find("200,200,200") != std::string::npos);
    }

    SECTION("GenerateBossCritique overload with CategoryScorecard and issues works") {
        CategoryScorecard card;
        card.overallHealthScore = 80.0f;
        std::vector<SequenceIssue> issues;
        SequenceIssue i1;
        i1.category = "TimingGrid";
        i1.message = "Timing drift detected";
        i1.suggestedFix = "Quantize timing grid";
        issues.push_back(i1);

        std::string critique = SequenceValidatorAI::GenerateBossCritique(card, issues);
        REQUIRE(!critique.empty());
        REQUIRE(critique.find("MASTER SEQUENCER BOSS CRITIQUE") != std::string::npos);
        REQUIRE(critique.find("Timing drift detected") != std::string::npos);
    }
}
