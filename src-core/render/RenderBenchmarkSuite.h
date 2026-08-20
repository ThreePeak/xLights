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
#include <chrono>
#include <nlohmann/json.hpp>

namespace xLights {

struct SequenceBenchmarkReport {
    std::string sequenceName;
    std::string sequenceXsqPath;
    std::string outputFseqPath;
    double renderTimeSeconds{0.0};
    double framesPerSecond{0.0};
    int totalFrames{0};
    int totalChannels{0};
    bool renderSuccess{false};
    bool byteIdenticalToBaseline{false};
    std::string diffSummary;

    nlohmann::json ToJson() const;
};

struct BenchmarkSuiteResult {
    std::vector<SequenceBenchmarkReport> sequenceReports;
    double totalSuiteDurationSec{0.0};
    double averageFps{0.0};
    int totalSequencesPassed{0};
    int totalSequencesFailed{0};
    bool allByteIdentical{false};

    nlohmann::json ToJson() const;
};

class RenderBenchmarkSuite {
public:
    RenderBenchmarkSuite() = default;
    ~RenderBenchmarkSuite() = default;

    /// Compares two raw frame buffers channel-by-channel for exact byte-identity
    static bool CompareFrameBuffers(
        const uint8_t* actual,
        const uint8_t* expected,
        size_t byteCount,
        std::string& outDiffSummary
    );

    /// Runs automated performance and regression benchmark across a list of sequences
    static BenchmarkSuiteResult RunBenchmark(
        const std::vector<std::string>& sequencePaths,
        const std::string& baselineDir = "",
        int sampleFrameCount = 100,
        int channelCount = 2048
    );
};

} // namespace xLights
