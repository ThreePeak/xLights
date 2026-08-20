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
#include <nlohmann/json.hpp>

namespace xLights::AI {

enum class DiagnosticSeverity {
    INFO = 0,
    WARNING,
    CRITICAL,
    FATAL
};

enum class DiagnosticCategory {
    NETWORK_E131_DDP = 0,
    AUDIO_DECODER,
    RENDER_ENGINE,
    MEMORY_OVERFLOW,
    SHOW_XML_CORRUPTION,
    CONTROLLER_TIMEOUT,
    GENERAL_RUNTIME
};

struct DiagnosticIssue {
    DiagnosticSeverity severity{DiagnosticSeverity::INFO};
    DiagnosticCategory category{DiagnosticCategory::GENERAL_RUNTIME};
    std::string errorCode;
    std::string description;
    std::string sourceLocation;
    std::string timestamp;
    std::string remediationAdvice;
    uint32_t occurrences{1};
};

struct ShowDiagnosticReport {
    bool hasErrors{false};
    uint32_t totalLinesParsed{0};
    uint32_t infoCount{0};
    uint32_t warningCount{0};
    uint32_t criticalCount{0};
    uint32_t fatalCount{0};
    std::vector<DiagnosticIssue> issues;
    std::string summary;

    nlohmann::json ToJson() const;
};

class ShowLogDiagnosticsAnalyzer {
public:
    ShowLogDiagnosticsAnalyzer() = default;
    ~ShowLogDiagnosticsAnalyzer() = default;

    /// Analyzes raw spdlog log content and returns structured diagnostics
    static ShowDiagnosticReport AnalyzeLogContent(const std::string& logContent);

    /// Inspects and parses show directory XML structures (xlights_rgbeffects.xml, xlights_networks.xml, sequence .xsq)
    static ShowDiagnosticReport InspectShowXmlContent(
        const std::string& fileName,
        const std::string& xmlContent
    );

    /// Merges multiple diagnostic reports into a single consolidated report
    static ShowDiagnosticReport MergeReports(const std::vector<ShowDiagnosticReport>& reports);

    static std::string SeverityToString(DiagnosticSeverity sev);
    static std::string CategoryToString(DiagnosticCategory cat);
};

} // namespace xLights::AI
