#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #13 & #14: Sequence Diagnostics & Conversational Copilot

#include "AISubsystemBase.h"
#include <string>
#include <vector>
#include <future>

namespace xLights::AI {

enum class DiagnosticSeverity {
    Info,
    Warning,
    Error,
    Critical
};

struct DiagnosticIssue {
    std::string issueId;
    DiagnosticSeverity severity = DiagnosticSeverity::Warning;
    std::string category;              // e.g. "TimingGrid", "ChannelOverlap", "UnassignedModel", "Performance"
    std::string message;
    std::string affectedModelName;
    int timeMs = -1;
    std::string suggestedFix;
};

struct SequenceAuditConfig {
    std::string sequenceFilePath;
    int totalDurationMs = 0;
    int activeEffectCount = 0;
    std::vector<std::string> activeModelNames;
    bool checkTimingGaps = true;
    bool checkChannelOverlaps = true;
    bool checkPerformanceBottlenecks = true;
};

struct SequenceAuditResult {
    bool success = false;
    std::string errorMessage;
    int totalIssuesCount = 0;
    int errorCount = 0;
    int warningCount = 0;
    std::vector<DiagnosticIssue> issues;
    std::string auditSummary;
};

struct AssistantCopilotQuery {
    std::string userQuery;             // e.g. "How can I improve transitions on my MegaTree?"
    std::string sequenceContext;       // Sequence name, audio title, active models
    std::string selectedModelName;
    std::vector<std::string> activePalettes;
};

struct AssistantCopilotResponse {
    bool success = false;
    std::string errorMessage;
    std::string replyText;             // Formatted markdown conversational response
    std::vector<std::string> recommendedShortcuts; // e.g. ["Apply Rainbow Palette", "Align to Beat Grid"]
    std::string suggestedEffectPreset;
};

class SequenceDiagnosticsCopilot : public AISubsystemBase {
public:
    SequenceDiagnosticsCopilot(ServiceManager* sm = nullptr) : AISubsystemBase(sm) {}
    virtual ~SequenceDiagnosticsCopilot() override = default;

    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override {
        return std::async(std::launch::async, [this, callback]() {
            if (callback) callback("Initializing SequenceDiagnosticsCopilot...", 0.0f);
            m_isInitialized.store(true);
            if (callback) callback("SequenceDiagnosticsCopilot initialized.", 100.0f);
            return true;
        });
    }

    virtual void Shutdown() override {
        m_isInitialized.store(false);
    }

    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "SequenceDiagnosticsCopilot"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"sequence_quality_audit", "error_diagnostics", "conversational_copilot", "design_recommendation"};
    }

    // Feature #14: Automated Sequence Quality Audit & Error Diagnostic Copilot
    [[nodiscard]] static SequenceAuditResult AuditSequenceDiagnostics(const SequenceAuditConfig& config);

    // Feature #13: In-App Light Show Assistant & Conversational Copilot
    [[nodiscard]] static AssistantCopilotResponse QueryAssistantCopilot(const AssistantCopilotQuery& query);

    // Export audit report to structured JSON
    [[nodiscard]] static std::string ExportAuditReportJSON(const SequenceAuditResult& result);
};

} // namespace xLights::AI
