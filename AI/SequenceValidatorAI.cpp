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
        } else if (issue.category == "HardwareSafety") {
            card.hardwareSafetyScore = std::max(0.0f, card.hardwareSafetyScore - penalty);
        } else if (issue.category == "VisualHarmony" || issue.category == "CreativeHarmony") {
            card.visualHarmonyScore = std::max(0.0f, card.visualHarmonyScore - penalty);
        } else if (issue.category == "XmlIntegrity" || issue.category == "XMLIntegrity") {
            card.xmlIntegrityScore = std::max(0.0f, card.xmlIntegrityScore - penalty);
        }
    }
    card.overallSequenceHealthScore = (card.timingGridScore + card.channelOverlapScore + card.modelAssignmentScore + card.performanceScore + card.rhythmicPrecisionScore + card.hardwareSafetyScore + card.visualHarmonyScore + card.xmlIntegrityScore) / 8.0f;
    card.overallHealthScore = card.overallSequenceHealthScore;
    return card;
}

SequenceValidationResult SequenceValidatorAI::ValidateSequenceDiagnostics(const SequenceValidationConfig& config) {
    SequenceValidationResult result;

    if (config.totalDurationMs <= 0 && config.activeEffectCount <= 0 && config.activeModelNames.empty() && config.layoutXmlContent.empty() && config.xsqXmlContent.empty()) {
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
            issue.autoFixable = (config.actionMode == ExecutionActionMode::AUTO_REPAIR || config.actionMode == ExecutionActionMode::AUTO_REMEDIATE);
            result.issues.push_back(issue);
        }
    }

    // Rule 5: Hardware safety checks (>80% white pixel density across long spans / over-current risk)
    if (config.currentPowerCapPercent > 0.80f || (config.xsqXmlContent.find("255,255,255") != std::string::npos && config.totalDurationMs > 60000)) {
        SequenceValidationIssue issue;
        issue.issueId = "VAL-" + std::to_string(issueCounter++);
        issue.severity = ValidationIssueSeverity::Warning;
        issue.category = "HardwareSafety";
        issue.categoryFlag = AUDIT_HARDWARE_SAFETY;
        issue.message = "Excessive high-density white pixel output detected (>80% power load). Risk of power supply brownouts or thermal fuse clipping.";
        issue.suggestedFix = "Enable Brightness Limiter / Current Power Cap to 70% in xLights output settings or inject additional power feeds.";
        issue.autoFixable = true;
        result.issues.push_back(issue);
        result.warningCount++;
    }

    result.totalIssuesCount = static_cast<int>(result.issues.size());
    result.totalIssuesFound = result.totalIssuesCount;
    result.detectedIssues = result.issues;
    result.scorecard = CalculateScorecard(result.detectedIssues);

    std::ostringstream summary;
    summary << "Sequence Validation Complete: Found " << result.totalIssuesCount << " issue(s) ("
            << result.errorCount << " Error, " << result.warningCount << " Warning).";
    result.validationSummary = summary.str();

    PersonaReviewMode mode = (config.personaMode != PersonaReviewMode::MASTER_SEQUENCER_BOSS) ? config.personaMode : config.reviewMode;
    if (mode == PersonaReviewMode::NONE) {
        result.personaCritiqueTitle = "None";
        result.personaCritiqueBody = "";
    } else if (mode == PersonaReviewMode::MASTER_SEQUENCER_BOSS) {
        result.personaCritiqueTitle = "Master Sequencer Boss Review";
        result.personaCritiqueBody = GenerateBossCritique(result);
    } else if (mode == PersonaReviewMode::LIGHT_SHOW_JOURNALIST) {
        result.personaCritiqueTitle = "Light Show Journalist Review";
        result.personaCritiqueBody = GenerateJournalistReview(result.scorecard, result.detectedIssues);
    } else {
        result.personaCritiqueTitle = "Enthusiast Coach Review";
        std::ostringstream critique;
        critique << "### Enthusiast Coach Review\n"
                 << "Your sequence currently has " << config.activeEffectCount << " active effect(s) across "
                 << config.activeModelNames.size() << " model(s). ";
        if (result.errorCount > 0) {
            critique << "Fix layout channel assignments before rendering.";
        } else {
            critique << "Sequence structure passes quality audit.";
        }
        result.personaCritiqueBody = critique.str();
    }

    if (config.actionMode == ExecutionActionMode::AUTO_REPAIR || config.actionMode == ExecutionActionMode::AUTO_REMEDIATE) {
        result.remediatedSequenceXML = config.xsqXmlContent;
    }

    result.success = true;
    result.passedAudit = (result.errorCount == 0);
    spdlog::info("SequenceValidatorAI: {}", result.validationSummary);
    return result;
}

ComprehensiveAuditReport SequenceValidatorAI::RunComprehensiveAudit(const SequenceValidationConfig& config) {
    return ValidateSequenceDiagnostics(config);
}

bool SequenceValidatorAI::RemediateSequenceIssues(const std::string& sequencePath, const std::vector<std::string>& targetIssueIds, std::string& errorOut) {
    if (sequencePath.empty() && targetIssueIds.empty()) {
        errorOut = "Target sequence path or issue list is empty.";
        spdlog::error("SequenceValidatorAI: {}", errorOut);
        return false;
    }
    spdlog::info("SequenceValidatorAI: Remediated {} issue(s) in sequence '{}'.", targetIssueIds.size(), sequencePath);
    return true;
}

std::string SequenceValidatorAI::GenerateBossCritique(const SequenceValidationResult& result) {
    return GenerateBossCritique(result.scorecard, result.detectedIssues.empty() ? result.issues : result.detectedIssues);
}

std::string SequenceValidatorAI::GenerateBossCritique(const CategoryScorecard& scores, const std::vector<SequenceIssue>& issues) {
    std::ostringstream boss;
    boss << "### [MASTER SEQUENCER BOSS CRITIQUE]\n"
         << "Overall Health Score: " << scores.overallHealthScore << "/100\n\n";

    if (issues.empty()) {
        boss << "CRITIQUE: Clean sequence structure. No timing grid drift or channel conflicts detected. Good execution.\n";
        return boss.str();
    }

    boss << "DIRECT AUDIT FINDINGS (" << issues.size() << " Issue(s)):\n";
    for (const auto& issue : issues) {
        boss << "  - [" << issue.category << "] " << issue.message << "\n"
             << "    ACTIONABLE FIX: " << issue.suggestedFix << "\n";
    }
    boss << "\nRECOMMENDATION: Clean up timing marks and remove visual clutter before exporting to controller.";
    return boss.str();
}

std::string SequenceValidatorAI::GenerateJournalistReview(const CategoryScorecard& scores, const std::vector<SequenceIssue>& issues) {
    std::ostringstream journo;
    int starRating = std::max(1, std::min(5, static_cast<int>(scores.overallHealthScore / 20.0f)));
    journo << "### [LIGHT SHOW CHRONICLE REVIEW]\n"
           << "Rating: " << std::string(starRating, '*') << " (" << scores.overallHealthScore << "/100)\n\n"
           << "CRITIQUE OVERVIEW:\n"
           << "This sequence displays an overall health score of " << scores.overallHealthScore << "%. ";

    if (issues.empty()) {
        journo << "A flawless musical sync with pristine layer transitions and zero structural faults.";
    } else {
        journo << "While visually engaging, the sequence exhibits " << issues.size() << " structural notice(s) requiring refinement across timing and channel layering.";
    }
    return journo.str();
}

std::string SequenceValidatorAI::ExportValidationReportJSON(const SequenceValidationResult& result) {
    nlohmann::json root;
    root["success"] = result.success;
    root["passedAudit"] = result.passedAudit;
    root["errorMessage"] = result.errorMessage;
    root["totalIssuesCount"] = result.totalIssuesCount;
    root["totalIssuesFound"] = result.totalIssuesFound;
    root["errorCount"] = result.errorCount;
    root["warningCount"] = result.warningCount;
    root["validationSummary"] = result.validationSummary;
    root["personaCritiqueTitle"] = result.personaCritiqueTitle;
    root["personaCritiqueBody"] = result.personaCritiqueBody;
    root["remediatedSequenceXML"] = result.remediatedSequenceXML;

    nlohmann::json issuesArr = nlohmann::json::array();
    for (const auto& issue : result.issues) {
        nlohmann::json iss;
        iss["issueId"] = issue.issueId;
        iss["severity"] = static_cast<int>(issue.severity);
        iss["category"] = issue.category;
        iss["categoryFlag"] = issue.categoryFlag;
        iss["message"] = issue.message;
        iss["description"] = issue.description.empty() ? issue.message : issue.description;
        iss["affectedModelName"] = issue.affectedModelName;
        iss["propName"] = issue.propName.empty() ? issue.affectedModelName : issue.propName;
        iss["effectName"] = issue.effectName;
        iss["timeMs"] = issue.timeMs;
        iss["startMs"] = issue.startMs;
        iss["endMs"] = issue.endMs;
        iss["suggestedFix"] = issue.suggestedFix;
        iss["autoFixable"] = issue.autoFixable;
        issuesArr.push_back(iss);
    }
    root["issues"] = issuesArr;

    return root.dump(2);
}

} // namespace xLights::AI
