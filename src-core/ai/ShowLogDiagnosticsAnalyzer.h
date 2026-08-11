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

struct LogErrorEntry {
    std::string timestamp;
    std::string logLevel;        // "ERROR", "CRITICAL", "WARN"
    std::string sourceModule;    // e.g. "OutputManager", "FFmpegAudioDecoder"
    std::string errorMessage;
    std::string rootCauseCategory;
    std::string recommendedFix;
};

struct XMLDiagnosticEntry {
    std::string xmlFilePath;
    std::string nodePath;
    std::string xmlErrorType;   // "Malformed Tag", "Missing Attribute", "Schema Error"
    std::string details;
};

struct ShowDiagnosticsReport {
    bool success = false;
    int totalLogLinesParsed = 0;
    int errorCount = 0;
    int warningCount = 0;
    int xmlFilesInspected = 0;
    int xmlCorruptionsFound = 0;
    std::vector<LogErrorEntry> logErrors;
    std::vector<XMLDiagnosticEntry> xmlErrors;
    std::string summaryReport;
};

class ShowLogDiagnosticsAnalyzer {
public:
    ShowLogDiagnosticsAnalyzer() = default;
    ~ShowLogDiagnosticsAnalyzer() = default;

    // Parse spdlog log files and classify errors & root causes
    static ShowDiagnosticsReport AnalyzeSpdlogFile(const std::string& logFilePath);

    // Inspect show folder XML files (xlights_networks.xml, xlights_rgbeffects.xml, .xsq) using pugixml
    static ShowDiagnosticsReport InspectShowDirectoryXML(const std::string& showFolderPath);

    // Complete diagnostic audit combining spdlog logs and show directory XML files
    static ShowDiagnosticsReport RunFullShowDiagnostics(const std::string& showFolderPath,
                                                         const std::string& logFilePath);
};
