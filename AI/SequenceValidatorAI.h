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
    Critical,
    CRITICAL_ERROR
};

using IssueSeverity = ValidationIssueSeverity;

struct SequenceValidationIssue {
    std::string issueId;
    ValidationIssueSeverity severity = ValidationIssueSeverity::Warning;
    std::string category;              // "TimingGrid", "ChannelOverlap", "UnassignedModel", "Performance", "ChannelBound"
    std::string message;
    std::string affectedModelName;
    int timeMs = -1;
    std::string suggestedFix;
    bool autoFixable = false;          // Whether AI engine can automatically resolve this issue
};

enum AuditCategoryFlags {
    AUDIT_NONE               = 0,
    AUDIT_TIMING             = 1 << 0,
    AUDIT_OVERLAP            = 1 << 1,
    AUDIT_PERFORMANCE        = 1 << 2,
    AUDIT_BOUNDS             = 1 << 3,
    AUDIT_ALL                = 0xFF
};

enum class ExecutionActionMode {
    REPORT_ONLY,
    AUTO_REPAIR,
    AUTO_REMEDIATE       // Automatically fix all safe technical issues in XML
};

enum class PersonaReviewMode {
    MASTER_SEQUENCER_BOSS,
    LIGHT_SHOW_JOURNALIST,
    ENTHUSIAST_COACH
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
    ExecutionActionMode actionMode = ExecutionActionMode::REPORT_ONLY;
    PersonaReviewMode reviewMode = PersonaReviewMode::MASTER_SEQUENCER_BOSS;
};

using SequenceIssue = SequenceValidationIssue;

struct CategoryScorecard {
    float timingGridScore = 100.0f;
    float channelOverlapScore = 100.0f;
    float modelAssignmentScore = 100.0f;
    float performanceScore = 100.0f;
    float rhythmicPrecisionScore = 100.0f; // 0.0 to 100.0
    float overallSequenceHealthScore = 100.0f;
    float overallHealthScore = 100.0f;     // Weighted average
};

struct SequenceValidationResult {
    bool success = false;
    std::string errorMessage;
    int totalIssuesCount = 0;
    int errorCount = 0;
    int warningCount = 0;
    std::vector<SequenceValidationIssue> issues;
    std::vector<SequenceIssue> detectedIssues; // Alias vector for detected issues
    CategoryScorecard scorecard;
    std::string validationSummary;
    std::string personaCritiqueTitle;  // Title of persona critique e.g. "Master Sequencer Boss Review"
    std::string personaCritiqueBody;   // Contains Master Sequencer or Journalist critique
};

using ComprehensiveAuditReport = SequenceValidationResult;

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

    // Calculates health category scorecard from detected sequence issues
    [[nodiscard]] static CategoryScorecard CalculateScorecard(const std::vector<SequenceIssue>& issues);

    // Executes an automated sequence quality and diagnostic audit
    [[nodiscard]] static SequenceValidationResult ValidateSequenceDiagnostics(const SequenceValidationConfig& config);
    [[nodiscard]] static ComprehensiveAuditReport RunComprehensiveAudit(const SequenceValidationConfig& config);

    // Formats a no-fluff, direct Master Sequencer Boss critique detailing timing errors, sloppy transitions, and visual clutter
    [[nodiscard]] static std::string GenerateBossCritique(const SequenceValidationResult& result);

    // Export validation report to structured JSON format
    [[nodiscard]] static std::string ExportValidationReportJSON(const SequenceValidationResult& result);
};

} // namespace xLights::AI
