/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #13 & #14: Sequence Diagnostics & Conversational Copilot

#include "SequenceDiagnosticsCopilot.h"
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

SequenceAuditResult SequenceDiagnosticsCopilot::AuditSequenceDiagnostics(const SequenceAuditConfig& config) {
    SequenceAuditResult result;

    if (config.totalDurationMs <= 0 && config.activeEffectCount <= 0 && config.activeModelNames.empty()) {
        result.success = false;
        result.errorMessage = "Sequence configuration parameters are empty.";
        spdlog::error("SequenceDiagnosticsCopilot: {}", result.errorMessage);
        return result;
    }

    int issueCounter = 1;

    // Audit Rule 1: Check for empty/unassigned models
    if (config.activeModelNames.empty()) {
        DiagnosticIssue issue;
        issue.issueId = "AUD-" + std::to_string(issueCounter++);
        issue.severity = DiagnosticSeverity::Error;
        issue.category = "UnassignedModel";
        issue.message = "No active target models declared in sequence layout.";
        issue.suggestedFix = "Add at least one model to the sequence layout before rendering.";
        result.issues.push_back(issue);
        result.errorCount++;
    }

    // Audit Rule 2: Check high effect density / performance bottlenecks
    if (config.checkPerformanceBottlenecks && config.activeEffectCount > 2000) {
        DiagnosticIssue issue;
        issue.issueId = "AUD-" + std::to_string(issueCounter++);
        issue.severity = DiagnosticSeverity::Warning;
        issue.category = "Performance";
        issue.message = "High effect density detected (" + std::to_string(config.activeEffectCount) + " effects). Render latency may increase.";
        issue.suggestedFix = "Consolidate overlapping static effects into layer groups or submodel ranges.";
        result.issues.push_back(issue);
        result.warningCount++;
    }

    // Audit Rule 3: Check timing grid gaps
    if (config.checkTimingGaps && config.totalDurationMs > 30000 && config.activeEffectCount < 5) {
        DiagnosticIssue issue;
        issue.issueId = "AUD-" + std::to_string(issueCounter++);
        issue.severity = DiagnosticSeverity::Warning;
        issue.category = "TimingGrid";
        issue.message = "Large un-sequenced timing gaps detected across 30+ second audio duration.";
        issue.suggestedFix = "Use AI Value Curve / Preset Synthesizer to fill empty timing tracks.";
        result.issues.push_back(issue);
        result.warningCount++;
    }

    // Audit Rule 4: Model specific check
    for (const auto& model : config.activeModelNames) {
        if (ToLower(model).find("tree") != std::string::npos && config.activeEffectCount < 2) {
            DiagnosticIssue issue;
            issue.issueId = "AUD-" + std::to_string(issueCounter++);
            issue.severity = DiagnosticSeverity::Info;
            issue.category = "DesignSuggestion";
            issue.message = "Model '" + model + "' has low effect coverage.";
            issue.affectedModelName = model;
            issue.suggestedFix = "Apply a 3D Spiral or Bars effect preset to " + model + ".";
            result.issues.push_back(issue);
        }
    }

    result.totalIssuesCount = static_cast<int>(result.issues.size());

    std::ostringstream summary;
    summary << "Sequence Diagnostic Audit Complete: Found " << result.totalIssuesCount << " issue(s) ("
            << result.errorCount << " Error, " << result.warningCount << " Warning).";
    result.auditSummary = summary.str();

    result.success = true;
    spdlog::info("SequenceDiagnosticsCopilot: {}", result.auditSummary);
    return result;
}

AssistantCopilotResponse SequenceDiagnosticsCopilot::QueryAssistantCopilot(const AssistantCopilotQuery& query) {
    AssistantCopilotResponse response;

    if (query.userQuery.empty()) {
        response.success = false;
        response.errorMessage = "userQuery cannot be empty.";
        spdlog::error("SequenceDiagnosticsCopilot: {}", response.errorMessage);
        return response;
    }

    std::string lowerQuery = ToLower(query.userQuery);
    std::string modelStr = query.selectedModelName.empty() ? "active model" : query.selectedModelName;

    std::ostringstream reply;
    reply << "### xLights AI Assistant Copilot Recommendation\n\n"
          << "Based on your sequence context and target model **" << modelStr << "**:\n\n";

    if (lowerQuery.find("palette") != std::string::npos || lowerQuery.find("color") != std::string::npos) {
        reply << "- **Color Harmony**: Recommended warm festive palette (`#FF0000`, `#00FF00`, `#FFFFFF`, `#FFD700`).\n"
              << "- **Layer Blend**: Use `Over` or `Average` mode for vibrant transitions.\n";
        response.recommendedShortcuts.push_back("Apply Festive Palette");
        response.recommendedShortcuts.push_back("Set Layer Mode to Over");
        response.suggestedEffectPreset = "WarmFestive_Palette_Preset";
    } else if (lowerQuery.find("transition") != std::string::npos || lowerQuery.find("smooth") != std::string::npos) {
        reply << "- **Transition Curve**: Apply an Exponential Up Value Curve to `Speed` and `SubBuffer` parameters.\n"
              << "- **Debounce**: Enable 50ms transition smoothing on visemes and morph channels.\n";
        response.recommendedShortcuts.push_back("Apply Exponential Up Value Curve");
        response.recommendedShortcuts.push_back("Enable 50ms Viseme Smoothing");
        response.suggestedEffectPreset = "Smooth_Morph_Transition";
    } else {
        reply << "- **Effect Recommendation**: Try applying a **Bars** or **Spiral** effect on " << modelStr << ".\n"
              << "- **Timing Alignment**: Snap effect markers to the nearest 50ms beat grid track.\n";
        response.recommendedShortcuts.push_back("Create Spiral Effect");
        response.recommendedShortcuts.push_back("Snap to Beat Grid");
        response.suggestedEffectPreset = "Spiral_Rainbow_3D";
    }

    response.replyText = reply.str();
    response.success = true;
    spdlog::info("SequenceDiagnosticsCopilot: Answered assistant query for '{}'.", query.userQuery);
    return response;
}

std::string SequenceDiagnosticsCopilot::ExportAuditReportJSON(const SequenceAuditResult& result) {
    nlohmann::json root;
    root["success"] = result.success;
    root["errorMessage"] = result.errorMessage;
    root["totalIssuesCount"] = result.totalIssuesCount;
    root["errorCount"] = result.errorCount;
    root["warningCount"] = result.warningCount;
    root["auditSummary"] = result.auditSummary;

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
        issuesArr.push_back(iss);
    }
    root["issues"] = issuesArr;

    return root.dump(2);
}

} // namespace xLights::AI
