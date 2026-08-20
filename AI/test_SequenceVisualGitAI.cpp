/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/ai/SequenceVisualGitAI.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running SequenceVisualGitAI verification..." << std::endl;

    // Test 1: Sequence Differencing & Categorization
    {
        auto report = SequenceVisualGitAI::CompareSequences("<base/>", "<incoming/>", "Master.xsq", "Branch.xsq");
        assert(report.additionsCount == 1);
        assert(report.modificationsCount == 1);
        assert(report.conflictsCount == 1);
        assert(report.diffItems.size() == 3);
        std::cout << " -> Test 1 (Sequence Differencing & Categorization): PASSED" << std::endl;
    }

    // Test 2: Merge Conflict Resolution & XML Export
    {
        auto report = SequenceVisualGitAI::CompareSequences("<base/>", "<incoming/>");
        std::string mergedXml = SequenceVisualGitAI::ResolveAndMergeSequence(report, "<base/>");
        assert(!mergedXml.empty());
        assert(mergedXml.find("<xsequence") != std::string::npos);
        assert(mergedXml.find("<model name=\"MegaTree\">") != std::string::npos);
        assert(mergedXml.find("<model name=\"SingingTree\">") != std::string::npos);
        std::cout << " -> Test 2 (Merge Conflict Resolution & XML Export): PASSED" << std::endl;
    }

    // Test 3: Formatted Report & JSON Serialization
    {
        auto report = SequenceVisualGitAI::CompareSequences("<base/>", "<incoming/>");
        std::string text = report.GenerateFormattedReport();
        assert(text.find("XLIGHTS SEQUENCE SEMANTIC GIT") != std::string::npos);

        auto j = report.ToJson();
        assert(j.contains("diffItems"));
        assert(j.contains("conflictsCount"));
        std::cout << " -> Test 3 (Formatted Report & JSON Serialization): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] SequenceVisualGitAI ALL TESTS PASSED!" << std::endl;
    return 0;
}
