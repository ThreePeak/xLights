/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowLogDiagnosticsAnalyzer.h"
#include <sstream>
#include <regex>
#include <algorithm>
#include <unordered_map>
#include <pugixml.hpp>

namespace xLights::AI {

nlohmann::json ShowDiagnosticReport::ToJson() const {
    nlohmann::json j;
    j["has_errors"] = hasErrors;
    j["total_lines_parsed"] = totalLinesParsed;
    j["info_count"] = infoCount;
    j["warning_count"] = warningCount;
    j["critical_count"] = criticalCount;
    j["fatal_count"] = fatalCount;
    j["summary"] = summary;

    nlohmann::json issuesList = nlohmann::json::array();
    for (const auto& issue : issues) {
        nlohmann::json item;
        item["severity"] = ShowLogDiagnosticsAnalyzer::SeverityToString(issue.severity);
        item["category"] = ShowLogDiagnosticsAnalyzer::CategoryToString(issue.category);
        item["error_code"] = issue.errorCode;
        item["description"] = issue.description;
        item["source_location"] = issue.sourceLocation;
        item["timestamp"] = issue.timestamp;
        item["remediation_advice"] = issue.remediationAdvice;
        item["occurrences"] = issue.occurrences;
        issuesList.push_back(item);
    }
    j["issues"] = issuesList;
    return j;
}

std::string ShowLogDiagnosticsAnalyzer::SeverityToString(DiagnosticSeverity sev) {
    switch (sev) {
        case DiagnosticSeverity::INFO: return "INFO";
        case DiagnosticSeverity::WARNING: return "WARNING";
        case DiagnosticSeverity::CRITICAL: return "CRITICAL";
        case DiagnosticSeverity::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string ShowLogDiagnosticsAnalyzer::CategoryToString(DiagnosticCategory cat) {
    switch (cat) {
        case DiagnosticCategory::NETWORK_E131_DDP: return "Network E1.31 / DDP";
        case DiagnosticCategory::AUDIO_DECODER: return "Audio Decoder";
        case DiagnosticCategory::RENDER_ENGINE: return "Render Engine";
        case DiagnosticCategory::MEMORY_OVERFLOW: return "Memory Management";
        case DiagnosticCategory::SHOW_XML_CORRUPTION: return "Show XML Schema";
        case DiagnosticCategory::CONTROLLER_TIMEOUT: return "Controller Communication";
        case DiagnosticCategory::GENERAL_RUNTIME: return "General Runtime";
        default: return "General";
    }
}

ShowDiagnosticReport ShowLogDiagnosticsAnalyzer::AnalyzeLogContent(const std::string& logContent) {
    ShowDiagnosticReport report;
    if (logContent.empty()) {
        report.summary = "Empty log file provided.";
        return report;
    }

    std::istringstream stream(logContent);
    std::string line;
    std::unordered_map<std::string, DiagnosticIssue> issueMap;

    // spdlog pattern: [YYYY-MM-DD HH:MM:SS.mmm] [logger] [level] message
    std::regex logRegex(R"(\[(\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}(?:\.\d+)?)\]\s*\[([^\]]+)\]\s*\[([^\]]+)\]\s*(.*))");

    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        report.totalLinesParsed++;

        std::smatch match;
        std::string timestamp = "";
        std::string logger = "";
        std::string level = "";
        std::string msg = line;

        if (std::regex_search(line, match, logRegex)) {
            timestamp = match[1].str();
            logger = match[2].str();
            level = match[3].str();
            msg = match[4].str();
        }

        std::string lowerMsg = msg;
        std::transform(lowerMsg.begin(), lowerMsg.end(), lowerMsg.begin(), ::tolower);
        std::string lowerLevel = level;
        std::transform(lowerLevel.begin(), lowerLevel.end(), lowerLevel.begin(), ::tolower);

        DiagnosticSeverity severity = DiagnosticSeverity::INFO;
        DiagnosticCategory category = DiagnosticCategory::GENERAL_RUNTIME;
        std::string errCode = "XL_INFO";
        std::string advice = "No action necessary.";

        bool isMatch = false;

        // 1. Network / Packet Loss
        if (lowerMsg.find("packet drop") != std::string::npos || lowerMsg.find("socket timeout") != std::string::npos ||
            lowerMsg.find("e1.31 buffer overflow") != std::string::npos || lowerMsg.find("ddp send error") != std::string::npos) {
            severity = DiagnosticSeverity::CRITICAL;
            category = DiagnosticCategory::NETWORK_E131_DDP;
            errCode = "NET_PACKET_DROP";
            advice = "Verify network switch bandwidth and enable IGMP snooping to prevent multicast flood.";
            isMatch = true;
        }
        // 2. Audio Starvation
        else if (lowerMsg.find("audio underrun") != std::string::npos || lowerMsg.find("audio decoder stall") != std::string::npos ||
                 lowerMsg.find("failed to decode frame") != std::string::npos) {
            severity = DiagnosticSeverity::CRITICAL;
            category = DiagnosticCategory::AUDIO_DECODER;
            errCode = "AUDIO_UNDERRUN";
            advice = "Re-encode audio file to standard 44.1kHz 16-bit uncompressed WAV format.";
            isMatch = true;
        }
        // 3. Memory Warning / Overflow
        else if (lowerMsg.find("out of memory") != std::string::npos || lowerMsg.find("bad_alloc") != std::string::npos ||
                 lowerMsg.find("vram allocation failed") != std::string::npos) {
            severity = DiagnosticSeverity::FATAL;
            category = DiagnosticCategory::MEMORY_OVERFLOW;
            errCode = "MEM_OVERFLOW";
            advice = "Reduce sequence layer count or purge cached render buffers in Sequence Settings.";
            isMatch = true;
        }
        // 4. Controller Timeout
        else if (lowerMsg.find("controller unreachable") != std::string::npos || lowerMsg.find("ping timeout") != std::string::npos ||
                 lowerMsg.find("fpp sync lost") != std::string::npos) {
            severity = DiagnosticSeverity::WARNING;
            category = DiagnosticCategory::CONTROLLER_TIMEOUT;
            errCode = "CTRL_TIMEOUT";
            advice = "Check Ethernet cabling and controller static IP lease in Controller Setup.";
            isMatch = true;
        }
        // 5. Render Engine Warnings
        else if (lowerMsg.find("render queue stalled") != std::string::npos || lowerMsg.find("frame render skipped") != std::string::npos) {
            severity = DiagnosticSeverity::WARNING;
            category = DiagnosticCategory::RENDER_ENGINE;
            errCode = "RENDER_STALL";
            advice = "Lower sequence step time (e.g., from 25ms to 50ms) or enable GPU hardware acceleration.";
            isMatch = true;
        }
        // 6. Generic Error/Critical/Fatal levels
        else if (lowerLevel == "critical" || lowerLevel == "error" || lowerLevel == "err") {
            severity = DiagnosticSeverity::CRITICAL;
            category = DiagnosticCategory::GENERAL_RUNTIME;
            errCode = "RUNTIME_ERROR";
            advice = "Check log details for specific exception stacktrace.";
            isMatch = true;
        } else if (lowerLevel == "warning" || lowerLevel == "warn") {
            severity = DiagnosticSeverity::WARNING;
            category = DiagnosticCategory::GENERAL_RUNTIME;
            errCode = "RUNTIME_WARN";
            advice = "Review non-critical runtime warnings.";
            isMatch = true;
        }

        if (isMatch) {
            std::string key = errCode + ":" + msg;
            if (issueMap.find(key) != issueMap.end()) {
                issueMap[key].occurrences++;
            } else {
                DiagnosticIssue issue;
                issue.severity = severity;
                issue.category = category;
                issue.errorCode = errCode;
                issue.description = msg;
                issue.sourceLocation = logger;
                issue.timestamp = timestamp;
                issue.remediationAdvice = advice;
                issue.occurrences = 1;
                issueMap[key] = issue;
            }
        }
    }

    for (const auto& [k, issue] : issueMap) {
        switch (issue.severity) {
            case DiagnosticSeverity::INFO: report.infoCount += issue.occurrences; break;
            case DiagnosticSeverity::WARNING: report.warningCount += issue.occurrences; break;
            case DiagnosticSeverity::CRITICAL: report.criticalCount += issue.occurrences; break;
            case DiagnosticSeverity::FATAL: report.fatalCount += issue.occurrences; break;
        }
        report.issues.push_back(issue);
    }

    report.hasErrors = (report.criticalCount > 0 || report.fatalCount > 0);

    std::ostringstream summaryStream;
    summaryStream << "Parsed " << report.totalLinesParsed << " log lines. Found "
                  << report.fatalCount << " fatal, " << report.criticalCount << " critical, "
                  << report.warningCount << " warning issues.";
    report.summary = summaryStream.str();

    return report;
}

ShowDiagnosticReport ShowLogDiagnosticsAnalyzer::InspectShowXmlContent(
    const std::string& fileName,
    const std::string& xmlContent
) {
    ShowDiagnosticReport report;
    if (xmlContent.empty()) {
        report.hasErrors = true;
        report.fatalCount = 1;
        DiagnosticIssue issue;
        issue.severity = DiagnosticSeverity::FATAL;
        issue.category = DiagnosticCategory::SHOW_XML_CORRUPTION;
        issue.errorCode = "XML_EMPTY_FILE";
        issue.description = "Show configuration XML file '" + fileName + "' is completely empty (0 bytes).";
        issue.sourceLocation = fileName;
        issue.remediationAdvice = "Restore file from backup or regenerate from xLights layout tab.";
        report.issues.push_back(issue);
        report.summary = "Fatal: XML file is empty.";
        return report;
    }

    pugi::xml_document doc;
    pugi::xml_parse_result parseResult = doc.load_string(xmlContent.c_str());

    if (!parseResult) {
        report.hasErrors = true;
        report.fatalCount = 1;
        DiagnosticIssue issue;
        issue.severity = DiagnosticSeverity::FATAL;
        issue.category = DiagnosticCategory::SHOW_XML_CORRUPTION;
        issue.errorCode = "XML_PARSE_FAILURE";
        issue.description = "XML syntax error in '" + fileName + "': " + std::string(parseResult.description()) + " at offset " + std::to_string(parseResult.offset);
        issue.sourceLocation = fileName;
        issue.remediationAdvice = "Fix XML formatting tags or revert to previous valid show backup.";
        report.issues.push_back(issue);
        report.summary = "Fatal: XML failed schema parse.";
        return report;
    }

    // Inspect xlights_networks.xml
    if (fileName.find("networks") != std::string::npos || doc.child("networks")) {
        auto networksNode = doc.child("networks");
        if (networksNode) {
            std::unordered_map<int, std::string> universeMap;
            for (auto net : networksNode.children("network")) {
                int universe = net.attribute("Universe").as_int(0);
                std::string netType = net.attribute("NetworkType").as_string("E131");
                if (universe > 0) {
                    if (universeMap.find(universe) != universeMap.end()) {
                        report.warningCount++;
                        DiagnosticIssue issue;
                        issue.severity = DiagnosticSeverity::WARNING;
                        issue.category = DiagnosticCategory::SHOW_XML_CORRUPTION;
                        issue.errorCode = "NET_DUPLICATE_UNIVERSE";
                        issue.description = "Duplicate universe #" + std::to_string(universe) + " detected in networks configuration.";
                        issue.sourceLocation = fileName;
                        issue.remediationAdvice = "Renumber conflicting universe channels in Network Setup.";
                        report.issues.push_back(issue);
                    } else {
                        universeMap[universe] = netType;
                    }
                }
            }
        }
    }

    // Inspect xlights_rgbeffects.xml (models)
    if (fileName.find("rgbeffects") != std::string::npos || doc.child("xlights_models")) {
        auto modelsNode = doc.child("xlights_models");
        if (modelsNode) {
            for (auto model : modelsNode.children("model")) {
                std::string modelName = model.attribute("name").as_string("");
                int stringCount = model.attribute("StringCount").as_int(0);
                int nodesPerString = model.attribute("NodesPerString").as_int(0);

                if (modelName.empty()) {
                    report.criticalCount++;
                    DiagnosticIssue issue;
                    issue.severity = DiagnosticSeverity::CRITICAL;
                    issue.category = DiagnosticCategory::SHOW_XML_CORRUPTION;
                    issue.errorCode = "MODEL_MISSING_NAME";
                    issue.description = "Anonymous model tag without a name attribute detected.";
                    issue.sourceLocation = fileName;
                    issue.remediationAdvice = "Assign unique name to model or delete orphaned XML node.";
                    report.issues.push_back(issue);
                } else if (stringCount <= 0 && nodesPerString <= 0 && !model.child("custommodel")) {
                    report.warningCount++;
                    DiagnosticIssue issue;
                    issue.severity = DiagnosticSeverity::WARNING;
                    issue.category = DiagnosticCategory::SHOW_XML_CORRUPTION;
                    issue.errorCode = "MODEL_ZERO_NODES";
                    issue.description = "Model '" + modelName + "' has 0 strings and 0 nodes defined.";
                    issue.sourceLocation = fileName;
                    issue.remediationAdvice = "Configure pixel node counts for model in Layout tab.";
                    report.issues.push_back(issue);
                }
            }
        }
    }

    report.hasErrors = (report.criticalCount > 0 || report.fatalCount > 0);
    std::ostringstream summaryStream;
    summaryStream << "Inspected " << fileName << ". Issues: "
                  << report.fatalCount << " fatal, " << report.criticalCount << " critical, "
                  << report.warningCount << " warnings.";
    report.summary = summaryStream.str();

    return report;
}

ShowDiagnosticReport ShowLogDiagnosticsAnalyzer::MergeReports(const std::vector<ShowDiagnosticReport>& reports) {
    ShowDiagnosticReport merged;
    for (const auto& r : reports) {
        merged.totalLinesParsed += r.totalLinesParsed;
        merged.infoCount += r.infoCount;
        merged.warningCount += r.warningCount;
        merged.criticalCount += r.criticalCount;
        merged.fatalCount += r.fatalCount;
        merged.issues.insert(merged.issues.end(), r.issues.begin(), r.issues.end());
    }
    merged.hasErrors = (merged.criticalCount > 0 || merged.fatalCount > 0);
    std::ostringstream summaryStream;
    summaryStream << "Consolidated Report (" << reports.size() << " files): "
                  << merged.fatalCount << " fatal, " << merged.criticalCount << " critical, "
                  << merged.warningCount << " warnings.";
    merged.summary = summaryStream.str();
    return merged;
}

} // namespace xLights::AI
