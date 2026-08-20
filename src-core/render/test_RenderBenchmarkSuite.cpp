/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/render/RenderBenchmarkSuite.h"
#include <iostream>
#include <cassert>

using namespace xLights;

int main() {
    std::cout << "[Unit Test] Running RenderBenchmarkSuite verification..." << std::endl;

    // Test 1: CompareFrameBuffers exact match
    {
        std::vector<uint8_t> buf1 = {10, 20, 30, 40, 50};
        std::vector<uint8_t> buf2 = {10, 20, 30, 40, 50};
        std::string diff;
        bool match = RenderBenchmarkSuite::CompareFrameBuffers(buf1.data(), buf2.data(), 5, diff);
        assert(match);
        assert(diff.find("Exact byte-identical") != std::string::npos);
        std::cout << " -> Test 1 (CompareFrameBuffers Exact Match): PASSED" << std::endl;
    }

    // Test 2: CompareFrameBuffers mismatch detection
    {
        std::vector<uint8_t> buf1 = {10, 20, 30, 40, 50};
        std::vector<uint8_t> buf2 = {10, 20, 99, 40, 50};
        std::string diff;
        bool match = RenderBenchmarkSuite::CompareFrameBuffers(buf1.data(), buf2.data(), 5, diff);
        assert(!match);
        assert(diff.find("Diff found") != std::string::npos);
        std::cout << " -> Test 2 (CompareFrameBuffers Mismatch Diagnostics): PASSED" << std::endl;
    }

    // Test 3: BenchmarkSuite Execution
    {
        std::vector<std::string> seqs = {
            "C:/Show/WizardsInWinter.xsq",
            "C:/Show/CarolOfTheBells.xsq"
        };
        auto res = RenderBenchmarkSuite::RunBenchmark(seqs, "", 50, 1024);
        assert(res.sequenceReports.size() == 2);
        assert(res.totalSequencesPassed == 2);
        assert(res.allByteIdentical);
        assert(res.averageFps > 0.0);
        std::cout << " -> Test 3 (RunBenchmark Automated Suite Execution): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] RenderBenchmarkSuite ALL TESTS PASSED!" << std::endl;
    return 0;
}
