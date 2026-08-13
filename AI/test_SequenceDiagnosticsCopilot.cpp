/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #13 & #14 Catch2 Unit Tests for SequenceDiagnosticsCopilot

#include <catch2/catch_test_macros.hpp>
#include "SequenceDiagnosticsCopilot.h"
#include <nlohmann/json.hpp>

using namespace xLights::AI;

TEST_CASE("SequenceDiagnosticsCopilot: Audit & Assistant Copilot", "[SequenceDiagnosticsCopilot]") {

    SECTION("AuditSequenceDiagnostics detects empty model list and timing gaps") {
        SequenceAuditConfig config;
        config.totalDurationMs = 60000;
        config.activeEffectCount = 2;
        config.checkTimingGaps = true;

        SequenceAuditResult result = SequenceDiagnosticsCopilot::AuditSequenceDiagnostics(config);
        REQUIRE(result.success == true);
        REQUIRE(result.totalIssuesCount >= 2);
        REQUIRE(result.errorCount >= 1); // UnassignedModel error

        std::string jsonStr = SequenceDiagnosticsCopilot::ExportAuditReportJSON(result);
        REQUIRE(!jsonStr.empty());

        nlohmann::json parsed = nlohmann::json::parse(jsonStr);
        REQUIRE(parsed["success"] == true);
        REQUIRE(parsed.contains("issues"));
        REQUIRE(parsed["issues"].is_array());
    }

    SECTION("QueryAssistantCopilot generates markdown recommendations and shortcuts") {
        AssistantCopilotQuery query;
        query.userQuery = "How do I make smooth transitions on MegaTree?";
        query.selectedModelName = "MegaTree";

        AssistantCopilotResponse response = SequenceDiagnosticsCopilot::QueryAssistantCopilot(query);
        REQUIRE(response.success == true);
        REQUIRE(!response.replyText.empty());
        REQUIRE(!response.recommendedShortcuts.empty());
        REQUIRE(!response.suggestedEffectPreset.empty());
    }

    SECTION("Handles empty query gracefully") {
        AssistantCopilotQuery query;
        query.userQuery = "";

        AssistantCopilotResponse response = SequenceDiagnosticsCopilot::QueryAssistantCopilot(query);
        REQUIRE(response.success == false);
        REQUIRE(!response.errorMessage.empty());
    }
}
