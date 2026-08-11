#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <functional>
#include <string>
#include <vector>

enum class IssueSeverity {
    Info,
    Warning,
    Error,
    Critical
};

struct SequenceAuditIssue {
    IssueSeverity severity = IssueSeverity::Warning;
    std::string category;     // e.g. "Overlapping Effects", "Unrendered Buffers", "Unused Media"
    std::string modelName;
    long startTimeMS = 0;
    long endTimeMS = 0;
    std::string message;
    std::string autoFixRecommendation;
};

struct SequenceAuditReport {
    bool success = false;
    int totalEffectsAudited = 0;
    int totalIssuesFound = 0;
    int warningCount = 0;
    int errorCount = 0;
    std::vector<SequenceAuditIssue> issues;
    std::string summary;
};

struct AssistantCopilotResponse {
    bool success = false;
    std::string responseText;
    std::vector<std::string> suggestedActionLabels;
    std::vector<std::string> luaMacroCommands;
};

class SequenceDiagnosticsCopilot {
public:
    SequenceDiagnosticsCopilot() = default;
    ~SequenceDiagnosticsCopilot() = default;

    // Execute automated static quality audit on sequence structure and media assets
    static SequenceAuditReport AuditSequence(const std::string& sequencePath,
                                              const std::vector<std::string>& modelNames,
                                              const std::vector<std::string>& mediaFiles);

    // Conversational assistant query for light show design guidance
    static AssistantCopilotResponse QueryAssistantCopilot(const std::string& userPrompt,
                                                           const std::string& currentModel,
                                                           const std::string& currentEffectType,
                                                           const SequenceAuditReport* lastAudit = nullptr);

    // Clean up unused media assets from show folder
    static int RemoveUnusedMedia(const std::string& showFolder, const std::vector<std::string>& unusedMediaFiles);
};
