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
#include <map>
#include <future>
#include <memory>
#include <cstdint>

namespace xLights::AI {

enum class ValidationIssueSeverity {
    Info,
    INFO = Info,
    Warning,
    WARNING = Warning,
    Error,
    ERROR = Error,
    Critical,
    CRITICAL_ERROR = Critical
};

using IssueSeverity = ValidationIssueSeverity;

struct SequenceValidationIssue {
    std::string issueId;
    ValidationIssueSeverity severity = ValidationIssueSeverity::Warning;
    std::string category;              // "TimingGrid", "ChannelOverlap", "UnassignedModel", "Performance", "ChannelBound"
    uint32_t categoryFlag = 0;         // AuditCategoryFlags enum flag e.g. AUDIT_TIMING, AUDIT_OVERLAP
    std::string message;
    std::string description;          // Alias for issue message
    std::string affectedModelName;
    std::string propName;              // Alias for affected model or prop name
    std::string effectName;            // Target effect name e.g. "Bars", "Shimmer", "SingleStrand"
    int timeMs = -1;
    int startMs = 0;
    int endMs = -1;
    std::string suggestedFix;
    bool autoFixable = false;          // Whether AI engine can automatically resolve this issue
};

// Bitmask flags for pre-run category selection
enum AuditCategoryFlags : uint32_t {
    AUDIT_NONE               = 0,
    AUDIT_XML_INTEGRITY      = 1 << 0, // Corrupted XML, invalid tags, missing schemas
    AUDIT_CHANNEL_BOUNDS     = 1 << 1, // Channel overlaps, unmapped universe bounds
    AUDIT_RHYTHM_SYNC        = 1 << 2, // Off-beat timing marks, transient drift
    AUDIT_TIMING             = 1 << 2,
    AUDIT_RENDER_PERFORMANCE = 1 << 3, // Layer collisions, excessive buffer redraws
    AUDIT_OVERLAP            = 1 << 4,
    AUDIT_PERFORMANCE        = 1 << 4,
    AUDIT_CREATIVE_HARMONY   = 1 << 5, // Color muddying, visual clutter, negative space
    AUDIT_HARDWARE_SAFETY    = 1 << 6, // Over-current risks, high-density white clipping
    AUDIT_BOUNDS             = 1 << 7,
    AUDIT_ALL                = 0xFFFFFFFF
};

enum class ExecutionActionMode {
    REPORT_ONLY,         // Audit and generate report; make no edits
    PROMPT_BEFORE_FIX,   // Flag issues and wait for explicit confirmation per item
    AUTO_REPAIR,
    AUTO_REMEDIATE       // Automatically fix all safe technical issues in XML
};

enum class PersonaReviewMode {
    NONE,                       // Skip persona review critique generation
    MASTER_SEQUENCER_BOSS,      // Direct, unfiltered, expert technical feedback
    LIGHT_SHOW_JOURNALIST,
    LIGHTSHOW_CRITIC_JOURNALIST = LIGHT_SHOW_JOURNALIST, // Artistic, journalistic review article (pros/cons/impact)
    ENTHUSIAST_COACH
};

struct TargetScopeFilter {
    int startMs = 0;                           // 0 = Start of sequence
    int endMs = -1;                            // -1 = Full duration
    std::vector<std::string> targetPropNames;  // Empty = All props
    std::vector<std::string> ignorePropNames;  // Ignored props/directories
    std::vector<std::string> targetTrackNames; // e.g., ["Kick Drum", "Vocals"]
};

struct SequenceValidationConfig {
    std::string sequenceFilePath;
    std::string layoutXmlContent;      // Raw layout XML string for direct structural auditing
    std::string xsqXmlContent;         // Raw .xsq sequence XML string for direct XML sequence auditing
    int totalDurationMs = 0;
    int startMs = 0;                   // 0 = Start of sequence
    int endMs = -1;                    // -1 = Full duration
    int activeEffectCount = 0;
    std::vector<std::string> activeModelNames;
    std::vector<std::string> targetPropNames;   // Empty = All props
    std::vector<std::string> ignorePropNames;   // Ignored props/directories
    std::vector<std::string> targetTrackNames;  // e.g., ["Kick Drum", "Vocals"]
    TargetScopeFilter scopeFilter;             // Target scope filtering configuration
    uint32_t activeCategories = AUDIT_ALL;      // Bitmask flags for active category selection
    bool checkTimingGaps = true;
    bool checkChannelOverlaps = true;
    bool checkPerformanceBottlenecks = true;
    bool checkChannelBounds = true;
    float currentPowerCapPercent = 0.30f;       // Hardware safety power threshold
    ExecutionActionMode actionMode = ExecutionActionMode::REPORT_ONLY;
    PersonaReviewMode reviewMode = PersonaReviewMode::MASTER_SEQUENCER_BOSS;
    PersonaReviewMode personaMode = PersonaReviewMode::MASTER_SEQUENCER_BOSS; // Alias for reviewMode
};

struct DiagnosticConfigSpec : public SequenceValidationConfig {};

struct SequenceIssue : public SequenceValidationIssue {};

struct CategoryScorecard {
    float timingGridScore = 100.0f;
    float channelOverlapScore = 100.0f;
    float modelAssignmentScore = 100.0f;
    float performanceScore = 100.0f;
    float rhythmicPrecisionScore = 100.0f; // 0.0 to 100.0
    float hardwareSafetyScore = 100.0f;    // 0.0 to 100.0
    float visualHarmonyScore = 100.0f;     // 0.0 to 100.0
    float xmlIntegrityScore = 100.0f;      // 0.0 to 100.0
    float overallSequenceHealthScore = 100.0f;
    float overallHealthScore = 100.0f;     // Weighted average
};

struct SequenceValidationResult {
    bool success = false;
    bool passedAudit = false;           // Alias for audit pass/fail status
    std::string errorMessage;
    int totalIssuesCount = 0;
    int totalIssuesFound = 0;          // Alias for totalIssuesCount
    int errorCount = 0;
    int warningCount = 0;
    std::vector<SequenceValidationIssue> issues;
    std::vector<SequenceIssue> detectedIssues; // Alias vector for detected issues
    CategoryScorecard scorecard;
    std::string validationSummary;
    std::string personaCritiqueTitle;  // Title of persona critique e.g. "Master Sequencer Boss Review"
    std::string personaCritiqueBody;   // Contains Master Sequencer or Journalist critique
    std::string remediatedSequenceXML; // Remediated XML content after auto-fix
};

struct ComprehensiveAuditReport : public SequenceValidationResult {};

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

    // Apply specific remediation fixes to sequence XML
    [[nodiscard]] static bool RemediateSequenceIssues(const std::string& sequencePath, const std::vector<std::string>& targetIssueIds, std::string& errorOut);

    // Formats a no-fluff, direct Master Sequencer Boss critique detailing timing errors, sloppy transitions, and visual clutter
    [[nodiscard]] static std::string GenerateBossCritique(const SequenceValidationResult& result);
    [[nodiscard]] static std::string GenerateBossCritique(const CategoryScorecard& scores, const std::vector<SequenceIssue>& issues);

    // Formats a published light show journalist style review with star rating and structural analysis
    [[nodiscard]] static std::string GenerateJournalistReview(const CategoryScorecard& scores, const std::vector<SequenceIssue>& issues);

    // Export validation report to structured JSON format
    [[nodiscard]] static std::string ExportValidationReportJSON(const SequenceValidationResult& result);

private:
    // Internal helper methods for rule processing
};

} // namespace xLights::AI
