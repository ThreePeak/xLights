/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AI/ShowLogDiagnosticsAnalyzer.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running AIShowDiagnosticsDialog backend verification..." << std::endl;

    std::string sampleLog = "[2026-08-18 12:00:00] [error] Packet drop detected on Universe 10: socket timeout\n"
                            "[2026-08-18 12:00:01] [warning] Frame render skipped due to thread backlog\n";
    auto rep = ShowLogDiagnosticsAnalyzer::AnalyzeLogContent(sampleLog);
    assert(rep.issues.size() == 2);
    assert(rep.criticalCount >= 1);
    assert(rep.warningCount >= 1);
    std::cout << " -> AIShowDiagnosticsDialog log parsing & categorization: PASSED" << std::endl;
    return 0;
}
