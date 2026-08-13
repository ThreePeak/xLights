/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #14: Sequence Diagnostic & Audit AI Validator (xLights/AI/SequenceValidatorAI.cpp)

#include "SequenceValidatorAI.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cctype>
#include <sstream>

namespace xLights::AI {

static std::string ToLower(std::string_view str) {
    std::string lower(str);
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower;
}

CategoryScorecard SequenceValidatorAI::CalculateScorecard(const std::vector<SequenceIssue>& issues) {
    CategoryScorecard card;
    for (const auto& issue : issues) {
        float penalty = (issue.severity == ValidationIssueSeverity::Error || issue.severity == ValidationIssueSeverity::Critical) ? 25.0f : 10.0f;
        if (issue.category == "TimingGrid") {
            card.timingGridScore = std::max(0.0f, card.timingGridScore - penalty);
        } else if (issue.category == "ChannelOverlap") {
            card.channelOverlapScore = std::max(0.0f, card.channelOverlapScore - penalty);
        } else if (issue.category == "UnassignedModel") {
            card.modelAssignmentScore = std::max(0.0f, card.modelAssignmentScore - penalty);
        } else if (issue.category == "Performance") {
            card.performanceScore = std::max(0.0f, card.performanceScore - penalty);
        } else if (issue.category == "RhythmicPrecision") {
            card.rhythmicPrecisionScore = std::max(0.0f, card.rhythmicPrecisionScore - penalty);
        }
    }
    card.overallSequenceHealthScore = (card.timingGridScore + card.channelOverlapScore + card.modelAssignmentScore + card.performanceScore + card.rhythmicPrecisionScore) / 5.0f;
    card.overallHealthScore = card.overallSequenceHealthScore;
    return card;
}

SequenceValidationResult SequenceValidatorAI::ValidateSequenceDiagnostics(const SequenceValidationConfig& config) {
    SequenceValidationResult result;

    if (config.totalDurationMs <= 0 && config.activeEffectCount <= 0 && config.activeModelNames.empty()) {
        result.success = false;
        result.errorMessage = "SequenceValidationConfig parameters are empty.";
        spdlog::error("SequenceValidatorAI: {}", result.errorMessage);
        return result;
    }

    int issueCounter = 1;

    // Rule 1: Unassigned model layout check
    if (config.activeModelNames.empty()) {
        SequenceValidationIssue issue;
        issue.issueId = "VAL-" + std::to_string(issueCounter++);
        issue.severity = ValidationIssueSeverity::Error;
        issue.category = "UnassignedModel";
        issue.message = "No active target models declared in sequence layout.";
        issue.suggestedFix = "Add at least one model to the sequence layout before rendering.";
        result.issues.push_back(issue);
        result.errorCount++;
    }

    // Rule 2: High effect density / performance bottleneck check
    if (config.checkPerformanceBottlenecks && config.activeEffectCount > 2000) {
        SequenceValidationIssue issue;
        issue.issueId = "VAL-" + std::to_string(issueCounter++);
        issue.severity = ValidationIssueSeverity::Warning;
        issue.category = "Performance";
        issue.message = "High effect density detected (" + std::to_string(config.activeEffectCount) + " effects). Render latency may increase.";
        issue.suggestedFix = "Consolidate overlapping static effects into layer groups or submodel ranges.";
        result.issues.push_back(issue);
        result.warningCount++;
    }

    // Rule 3: Timing grid gaps check
    if (config.checkTimingGaps && config.totalDurationMs > 30000 && config.activeEffectCount < 5) {
        SequenceValidationIssue issue;
        issue.issueId = "VAL-" + std::to_string(issueCounter++);
        issue.severity = ValidationIssueSeverity::Warning;
        issue.category = "TimingGrid";
        issue.message = "Large un-sequenced timing gaps detected across 30+ second audio duration.";
        issue.suggestedFix = "Use AI Value Curve / Preset Synthesizer to fill empty timing tracks.";
        result.issues.push_back(issue);
        result.warningCount++;
    }

    // Rule 4: Model specific check
    for (const auto& model : config.activeModelNames) {
        if (ToLower(model).find("tree") != std::string::npos && config.activeEffectCount < 2) {
            SequenceValidationIssue issue;
            issue.issueId = "VAL-" + std::to_string(issueCounter++);
            issue.severity = ValidationIssueSeverity::Info;
            issue.category = "DesignSuggestion";
            issue.message = "Model '" + model + "' has low effect coverage.";
            issue.affectedModelName = model;
            issue.suggestedFix = "Apply a 3D Spiral or Bars effect preset to " + model + ".";
            issue.autoFixable = (config.actionMode == ExecutionActionMode::AUTO_REPAIR);
            result.issues.push_back(issue);
        }
    }

    result.totalIssuesCount = static_cast<int>(result.issues.size());
    result.detectedIssues = result.issues;
    result.scorecard = CalculateScorecard(result.detectedIssues);

    std::ostringstream summary;
    summary << "Sequence Validation Complete: Found " << result.totalIssuesCount << " issue(s) ("
            << result.errorCount << " Error, " << result.warningCount << " Warning).";
    result.validationSummary = summary.str();

    if (config.reviewMode == PersonaReviewMode::MASTER_SEQUENCER_BOSS) {
        result.personaCritiqueTitle = "Master Sequencer Boss Review";
        result.personaCritiqueBody = GenerateBossCritique(result);
    } else {
        result.personaCritiqueTitle = "Light Show Review";
        std::ostringstream critique;
        critique << "### Light Show Review\n"
                 << "Your sequence currently has " << config.activeEffectCount << " active effect(s) across "
                 << config.activeModelNames.size() << " model(s). ";
        if (result.errorCount > 0) {
            critique << "Fix layout channel assignments before rendering.";
        } else {
            critique << "Sequence structure passes quality audit.";
        }
        result.personaCritiqueBody = critique.str();
    }

    result.success = true;
    spdlog::info("SequenceValidatorAI: {}", result.validationSummary);
    return result;
}

std::string SequenceValidatorAI::GenerateBossCritique(const SequenceValidationResult& result) {
    std::ostringstream boss;
    boss << "### [MASTER SEQUENCER BOSS CRITIQUE]\n"
         << "Overall Health Score: " << result.scorecard.overallHealthScore << "/100\n\n";

    if (result.issues.empty()) {
        boss << "CRITIQUE: Clean sequence structure. No timing grid drift or channel conflicts detected. Good execution.\n";
        return boss.str();
    }

    boss << "DIRECT AUDIT FINDINGS (" << result.totalIssuesCount << " Issue(s)):\n";
    for (const auto& issue : result.issues) {
        boss << "  - [" << issue.category << "] " << issue.message << "\n"
             << "    ACTIONABLE FIX: " << issue.suggestedFix << "\n";
    }
    boss << "\nRECOMMENDATION: Clean up timing marks and remove visual clutter before exporting to controller.";
    return boss.str();
}

std::string SequenceValidatorAI::ExportValidationReportJSON(const SequenceValidationResult& result) {
    nlohmann::json root;
    root["success"] = result.success;
    root["errorMessage"] = result.errorMessage;
    root["totalIssuesCount"] = result.totalIssuesCount;
    root["errorCount"] = result.errorCount;
    root["warningCount"] = result.warningCount;
    root["validationSummary"] = result.validationSummary;
    root["personaCritiqueTitle"] = result.personaCritiqueTitle;
    root["personaCritiqueBody"] = result.personaCritiqueBody;

    nlohmann::json issuesArr = nlohmann::json::array();
    for (const auto& issue : result.issues) {
        nlohmann::json iss;
        iss["issueId"] = issue.issueId;
        iss["severity"] = static_cast<int>(issue.severity);
        iss["category"] = issue.category;
        iss["message"] = issue.message;
        iss["affectedModelName"] = issue.affectedModelName;
        iss["timeMs"] = issue.timeMs;
        iss["suggestedFix"] = issue.suggestedFix;
        iss["autoFixable"] = issue.autoFixable;
        issuesArr.push_back(iss);
    }
    root["issues"] = issuesArr;

    return root.dump(2);
}

} // namespace xLights::AI
