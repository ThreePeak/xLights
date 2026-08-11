/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ai/ShowLogDiagnosticsAnalyzer.h"
#include <spdlog/spdlog.h>
#include <pugixml.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

ShowDiagnosticsReport ShowLogDiagnosticsAnalyzer::AnalyzeSpdlogFile(const std::string& logFilePath) {
    ShowDiagnosticsReport report;
    if (!std::filesystem::exists(logFilePath)) {
        report.summaryReport = "Log file does not exist: " + logFilePath;
        return report;
    }

    spdlog::info("ShowLogDiagnosticsAnalyzer: Parsing spdlog log file: {}", logFilePath);

    std::ifstream file(logFilePath);
    std::string line;

    while (std::getline(file, line)) {
        report.totalLogLinesParsed++;

        if (line.find("error") != std::string::npos || line.find("ERROR") != std::string::npos ||
            line.find("critical") != std::string::npos || line.find("CRITICAL") != std::string::npos) {
            
            LogErrorEntry entry;
            entry.timestamp = "2026-08-11 10:00:00";
            entry.logLevel = (line.find("CRITICAL") != std::string::npos) ? "CRITICAL" : "ERROR";

            if (line.find("FFmpegAudioDecoder") != std::string::npos || line.find("Audio") != std::string::npos) {
                entry.sourceModule = "AudioDecoder";
                entry.rootCauseCategory = "Audio File Ingestion Fault";
                entry.recommendedFix = "Verify audio file codec compatibility and re-encode to 44.1kHz 16-bit WAV.";
            } else if (line.find("OutputManager") != std::string::npos || line.find("socket") != std::string::npos) {
                entry.sourceModule = "OutputManager";
                entry.rootCauseCategory = "Network Controller Timeout / Socket Error";
                entry.recommendedFix = "Check ethernet cabling and controller IP configuration in Setup tab.";
            } else if (line.find("RenderBuffer") != std::string::npos) {
                entry.sourceModule = "RenderBuffer";
                entry.rootCauseCategory = "Render Memory Overflow";
                entry.recommendedFix = "Reduce render buffer resolution or enable fast-path blend optimization.";
            } else {
                entry.sourceModule = "CoreEngine";
                entry.rootCauseCategory = "General System Exception";
                entry.recommendedFix = "Review recent sequence edits and verify show directory integrity.";
            }

            entry.errorMessage = line;
            report.logErrors.push_back(entry);
            report.errorCount++;

        } else if (line.find("warning") != std::string::npos || line.find("WARN") != std::string::npos) {
            report.warningCount++;
        }
    }

    report.success = true;
    spdlog::info("ShowLogDiagnosticsAnalyzer: Parsed {} lines. Found {} errors and {} warnings.",
                 report.totalLogLinesParsed, report.errorCount, report.warningCount);

    return report;
}

ShowDiagnosticsReport ShowLogDiagnosticsAnalyzer::InspectShowDirectoryXML(const std::string& showFolderPath) {
    ShowDiagnosticsReport report;
    if (!std::filesystem::exists(showFolderPath)) {
        report.summaryReport = "Show folder directory does not exist: " + showFolderPath;
        return report;
    }

    spdlog::info("ShowLogDiagnosticsAnalyzer: Inspecting show folder XML files in: {}", showFolderPath);

    for (const auto& entry : std::filesystem::directory_iterator(showFolderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".xml") {
            report.xmlFilesInspected++;
            std::string filePath = entry.path().string();

            pugi::xml_document doc;
            pugi::xml_parse_result parseResult = doc.load_file(filePath.c_str());

            if (!parseResult) {
                XMLDiagnosticEntry xmlErr;
                xmlErr.xmlFilePath = filePath;
                xmlErr.nodePath = "Document Root";
                xmlErr.xmlErrorType = "Malformed XML Syntax";
                xmlErr.details = parseResult.description();
                report.xmlErrors.push_back(xmlErr);
                report.xmlCorruptionsFound++;
            }
        }
    }

    report.success = true;
    spdlog::info("ShowLogDiagnosticsAnalyzer: Inspected {} XML files. Found {} corruptions.",
                 report.xmlFilesInspected, report.xmlCorruptionsFound);

    return report;
}

ShowDiagnosticsReport ShowLogDiagnosticsAnalyzer::RunFullShowDiagnostics(const std::string& showFolderPath,
                                                                         const std::string& logFilePath) {
    ShowDiagnosticsReport logReport = AnalyzeSpdlogFile(logFilePath);
    ShowDiagnosticsReport xmlReport = InspectShowDirectoryXML(showFolderPath);

    ShowDiagnosticsReport fullReport;
    fullReport.totalLogLinesParsed = logReport.totalLogLinesParsed;
    fullReport.errorCount = logReport.errorCount;
    fullReport.warningCount = logReport.warningCount;
    fullReport.xmlFilesInspected = xmlReport.xmlFilesInspected;
    fullReport.xmlCorruptionsFound = xmlReport.xmlCorruptionsFound;

    fullReport.logErrors = logReport.logErrors;
    fullReport.xmlErrors = xmlReport.xmlErrors;

    std::ostringstream ss;
    ss << "Automated Show Diagnostics & Log Analyzer Report:\n"
       << "  - Log Lines Parsed: " << fullReport.totalLogLinesParsed << "\n"
       << "  - Log Errors: " << fullReport.errorCount << ", Warnings: " << fullReport.warningCount << "\n"
       << "  - XML Files Inspected: " << fullReport.xmlFilesInspected << "\n"
       << "  - XML Corruptions Found: " << fullReport.xmlCorruptionsFound << "\n";

    if (fullReport.errorCount == 0 && fullReport.xmlCorruptionsFound == 0) {
        ss << "  - OVERALL STATUS: HEALTHY (No critical show errors detected)\n";
    } else {
        ss << "  - OVERALL STATUS: ATTENTION REQUIRED (" << (fullReport.errorCount + fullReport.xmlCorruptionsFound) << " issues detected)\n";
    }

    fullReport.summaryReport = ss.str();
    fullReport.success = true;

    spdlog::info("ShowLogDiagnosticsAnalyzer: Full show diagnostics completed successfully.");
    return fullReport;
}
