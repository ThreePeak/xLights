/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowLogDiagnosticsAnalyzer.h"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "[Unit Test] Running ShowLogDiagnosticsAnalyzer verification..." << std::endl;

    // Test 1: spdlog log parser & error classifier
    std::string sampleLog = 
        "[2026-08-18 12:00:01.123] [network] [info] E1.31 streaming started on 192.168.1.50\n"
        "[2026-08-18 12:00:05.456] [network] [error] Packet drop detected on Universe 12: socket timeout\n"
        "[2026-08-18 12:00:10.789] [audio] [critical] Audio underrun: failed to decode frame at 45.2s\n"
        "[2026-08-18 12:00:15.012] [render] [warning] Frame render skipped due to thread backlog\n";

    auto logReport = xLights::AI::ShowLogDiagnosticsAnalyzer::AnalyzeLogContent(sampleLog);
    assert(logReport.hasErrors);
    assert(logReport.totalLinesParsed == 4);
    assert(logReport.criticalCount == 2);
    assert(logReport.warningCount == 1);
    assert(logReport.issues.size() == 3);
    std::cout << " -> Test 1 (spdlog Parser & Error Classifier): PASSED" << std::endl;

    // Test 2: JSON serialization
    auto j = logReport.ToJson();
    assert(j["has_errors"] == true);
    assert(j["issues"].size() == 3);
    std::cout << " -> Test 2 (JSON Report Serialization): PASSED" << std::endl;

    // Test 3: XML Inspection - Malformed XML
    std::string badXml = "<networks><network Universe=\"1\"><unclosed></networks>";
    auto badReport = xLights::AI::ShowLogDiagnosticsAnalyzer::InspectShowXmlContent("xlights_networks.xml", badXml);
    assert(badReport.hasErrors);
    assert(badReport.fatalCount == 1);
    assert(badReport.issues[0].errorCode == "XML_PARSE_FAILURE");
    std::cout << " -> Test 3 (Malformed XML Syntax Detection): PASSED" << std::endl;

    // Test 4: XML Inspection - Duplicate Universe detection
    std::string duplicateNetXml = 
        "<networks>"
        "  <network Universe=\"1\" NetworkType=\"E131\" />"
        "  <network Universe=\"2\" NetworkType=\"E131\" />"
        "  <network Universe=\"1\" NetworkType=\"DDP\" />"
        "</networks>";
    auto dupReport = xLights::AI::ShowLogDiagnosticsAnalyzer::InspectShowXmlContent("xlights_networks.xml", duplicateNetXml);
    assert(dupReport.warningCount == 1);
    assert(dupReport.issues[0].errorCode == "NET_DUPLICATE_UNIVERSE");
    std::cout << " -> Test 4 (Duplicate Universe Detection): PASSED" << std::endl;

    // Test 5: XML Inspection - Model validation
    std::string modelXml =
        "<xlights_models>"
        "  <model name=\"Tree\" StringCount=\"16\" NodesPerString=\"50\" />"
        "  <model name=\"\" StringCount=\"1\" NodesPerString=\"10\" />"
        "  <model name=\"EmptyModel\" StringCount=\"0\" NodesPerString=\"0\" />"
        "</xlights_models>";
    auto modelReport = xLights::AI::ShowLogDiagnosticsAnalyzer::InspectShowXmlContent("xlights_rgbeffects.xml", modelXml);
    assert(modelReport.hasErrors);
    assert(modelReport.criticalCount == 1); // missing name
    assert(modelReport.warningCount == 1);  // zero nodes
    std::cout << " -> Test 5 (Model XML Node & Name Validation): PASSED" << std::endl;

    std::cout << "[Unit Test] ShowLogDiagnosticsAnalyzer ALL TESTS PASSED!" << std::endl;
    return 0;
}
