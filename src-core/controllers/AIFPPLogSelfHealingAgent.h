/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace xLights::AI {

enum class FPPLogSeverity {
    INFO_LOG,
    WARNING_LOG,
    ERROR_LOG,
    CRITICAL_FAULT
};

enum class FPPHealingActionType {
    REEXPORT_SPARSE_FSEQ,
    TUNE_DDP_PACKET_SIZE,
    DISABLE_WIFI_POWER_SAVE,
    RESTART_FPP_MULTISYNC_DAEMON,
    SWITCH_TO_UNICAST_E131
};

struct FPPLogEntry {
    std::string timestamp;
    std::string controllerHost;
    FPPLogSeverity severity{FPPLogSeverity::INFO_LOG};
    std::string subsystem;
    std::string rawMessage;
};

struct FPPHealingActionProposal {
    std::string actionId;
    std::string controllerHost;
    std::string issueSummary;
    std::string rootCauseAnalysis;
    FPPHealingActionType actionType{FPPHealingActionType::REEXPORT_SPARSE_FSEQ};
    std::string remediationCommand;
    bool isApplied{false};
    bool isDryRunSafe{true};
};

struct FPPFleetDiagnosticReport {
    int totalLogsParsed{0};
    int errorCount{0};
    int warningCount{0};
    int activeControllersCount{0};
    std::vector<FPPLogEntry> parsedLogs;
    std::vector<FPPHealingActionProposal> healingProposals;

    std::string GenerateFormattedReport() const;
};

class AIFPPLogSelfHealingAgent {
public:
    AIFPPLogSelfHealingAgent() = default;
    ~AIFPPLogSelfHealingAgent() = default;

    void Clear();
    void IngestLogLine(const std::string& host, const std::string& line);
    void IngestRawLogBuffer(const std::string& host, const std::string& logBuffer);

    FPPFleetDiagnosticReport AnalyzeFleetLogs();
    bool ApplyHealingAction(const std::string& actionId);
    bool RevertHealingAction(const std::string& actionId);

    static std::string GetActionTypeName(FPPHealingActionType type);
    static std::string GetSeverityName(FPPLogSeverity sev);

private:
    std::vector<FPPLogEntry> m_logs;
    std::map<std::string, bool> m_actionStatusMap;
};

} // namespace xLights::AI
