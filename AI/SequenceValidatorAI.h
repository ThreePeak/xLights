#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #14: Sequence Diagnostic & Audit AI Validator (xLights/AI/SequenceValidatorAI.h)

#include "AISubsystemBase.h"
#include <string>
#include <vector>
#include <future>

namespace xLights::AI {

enum class ValidationIssueSeverity {
    Info,
    Warning,
    Error,
    Critical
};

struct SequenceValidationIssue {
    std::string issueId;
    ValidationIssueSeverity severity = ValidationIssueSeverity::Warning;
    std::string category;              // "TimingGrid", "ChannelOverlap", "UnassignedModel", "Performance", "ChannelBound"
    std::string message;
    std::string affectedModelName;
    int timeMs = -1;
    std::string suggestedFix;
};

struct SequenceValidationConfig {
    std::string sequenceFilePath;
    int totalDurationMs = 0;
    int activeEffectCount = 0;
    std::vector<std::string> activeModelNames;
    bool checkTimingGaps = true;
    bool checkChannelOverlaps = true;
    bool checkPerformanceBottlenecks = true;
    bool checkChannelBounds = true;
};

struct SequenceValidationResult {
    bool success = false;
    std::string errorMessage;
    int totalIssuesCount = 0;
    int errorCount = 0;
    int warningCount = 0;
    std::vector<SequenceValidationIssue> issues;
    std::string validationSummary;
};

class SequenceValidatorAI : public AISubsystemBase {
public:
    SequenceValidatorAI(ServiceManager* sm = nullptr) : AISubsystemBase(sm) {}
    virtual ~SequenceValidatorAI() override = default;

    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override {
        return std::async(std::launch::async, [this, callback]() {
            if (callback) callback("Initializing SequenceValidatorAI...", 0.0f);
            m_isInitialized.store(true);
            if (callback) callback("SequenceValidatorAI initialized.", 100.0f);
            return true;
        });
    }

    virtual void Shutdown() override {
        m_isInitialized.store(false);
    }

    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "SequenceValidatorAI"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"sequence_validation", "error_diagnostics", "timing_grid_gap_check", "channel_overlap_audit"};
    }

    // Executes an automated sequence quality and diagnostic audit
    [[nodiscard]] static SequenceValidationResult ValidateSequenceDiagnostics(const SequenceValidationConfig& config);

    // Export validation report to structured JSON format
    [[nodiscard]] static std::string ExportValidationReportJSON(const SequenceValidationResult& result);
};

} // namespace xLights::AI
