/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/controllers/AIPacketLossInterpolator.h"
#include <spdlog/spdlog.h>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <random>

namespace xLights::AI {

AIPacketLossInterpolator::AIPacketLossInterpolator(PacketInterpolationMode mode, int lookaheadWindow)
    : m_mode(mode), m_lookaheadWindow(lookaheadWindow) {
}

void AIPacketLossInterpolator::Reset() {
    m_lastFrameIndex = 0;
    m_lastTimestampMs = 0;
    m_lastFrameData.clear();
    m_historyBuffer.clear();
}

std::string AIPacketLossInterpolator::GetModeName(PacketInterpolationMode mode) {
    switch (mode) {
        case PacketInterpolationMode::DISABLED: return "Disabled (Hold Last Frame / Blackout)";
        case PacketInterpolationMode::LINEAR_INTERPOLATION: return "Linear Smooth Interpolation";
        case PacketInterpolationMode::CUBIC_HERMITE_SPLINE: return "Cubic Hermite Spline Smoothing";
        case PacketInterpolationMode::ADAPTIVE_LOOKAHEAD_BEZIER: return "Adaptive Lookahead Bezier (AI Predictive)";
        default: return "Unknown Mode";
    }
}

std::vector<uint8_t> AIPacketLossInterpolator::InterpolateLinear(
    const std::vector<uint8_t>& before, const std::vector<uint8_t>& after, double t) {
    size_t sz = std::min(before.size(), after.size());
    std::vector<uint8_t> out(sz);
    for (size_t i = 0; i < sz; ++i) {
        double val = (1.0 - t) * static_cast<double>(before[i]) + t * static_cast<double>(after[i]);
        out[i] = static_cast<uint8_t>(std::clamp(val + 0.5, 0.0, 255.0));
    }
    return out;
}

std::vector<uint8_t> AIPacketLossInterpolator::InterpolateCubicHermite(
    const std::vector<uint8_t>& p0, const std::vector<uint8_t>& p1,
    const std::vector<uint8_t>& p2, const std::vector<uint8_t>& p3, double t) {
    size_t sz = std::min({p0.size(), p1.size(), p2.size(), p3.size()});
    std::vector<uint8_t> out(sz);
    double t2 = t * t;
    double t3 = t2 * t;

    for (size_t i = 0; i < sz; ++i) {
        double v0 = p0[i];
        double v1 = p1[i];
        double v2 = p2[i];
        double v3 = p3[i];

        double a = -0.5 * v0 + 1.5 * v1 - 1.5 * v2 + 0.5 * v3;
        double b = v0 - 2.5 * v1 + 2.0 * v2 - 0.5 * v3;
        double c = -0.5 * v0 + 0.5 * v2;
        double d = v1;

        double val = a * t3 + b * t2 + c * t + d;
        out[i] = static_cast<uint8_t>(std::clamp(val + 0.5, 0.0, 255.0));
    }
    return out;
}

std::vector<uint8_t> AIPacketLossInterpolator::InterpolateAdaptiveBezier(
    const std::vector<uint8_t>& p0, const std::vector<uint8_t>& p1,
    const std::vector<uint8_t>& p2, double t) {
    size_t sz = std::min({p0.size(), p1.size(), p2.size()});
    std::vector<uint8_t> out(sz);
    double u = 1.0 - t;
    double tt = t * t;
    double uu = u * u;

    for (size_t i = 0; i < sz; ++i) {
        double v0 = p0[i];
        double v1 = p1[i];
        double v2 = p2[i];

        // Quadratic Bezier
        double val = uu * v0 + 2.0 * u * t * v1 + tt * v2;
        out[i] = static_cast<uint8_t>(std::clamp(val + 0.5, 0.0, 255.0));
    }
    return out;
}

void AIPacketLossInterpolator::IngestReceivedFrame(uint32_t frameIndex, uint32_t timestampMs, const std::vector<uint8_t>& channelData) {
    InterpolatedFrame frame;
    frame.frameIndex = frameIndex;
    frame.timestampMs = timestampMs;
    frame.channels = channelData;
    frame.isSynthesized = false;
    frame.confidenceScore = 1.0;

    m_historyBuffer.push_back(frame);
    if ((int)m_historyBuffer.size() > m_lookaheadWindow * 2 + 2) {
        m_historyBuffer.pop_front();
    }

    m_lastFrameIndex = frameIndex;
    m_lastTimestampMs = timestampMs;
    m_lastFrameData = channelData;
}

std::vector<InterpolatedFrame> AIPacketLossInterpolator::ProcessArrivalAndFillGaps(
    uint32_t newFrameIndex, uint32_t timestampMs, const std::vector<uint8_t>& channelData) {
    std::vector<InterpolatedFrame> synthesizedList;

    if (m_lastFrameData.empty() || m_mode == PacketInterpolationMode::DISABLED) {
        IngestReceivedFrame(newFrameIndex, timestampMs, channelData);
        return synthesizedList;
    }

    uint32_t gap = (newFrameIndex > m_lastFrameIndex) ? (newFrameIndex - m_lastFrameIndex - 1) : 0;

    if (gap > 0 && gap <= 10) { // Support filling up to 10 lost frames (250ms at 40FPS)
        uint32_t timeStep = (timestampMs > m_lastTimestampMs) ? (timestampMs - m_lastTimestampMs) / (gap + 1) : 25;

        for (uint32_t step = 1; step <= gap; ++step) {
            double t = static_cast<double>(step) / static_cast<double>(gap + 1);
            InterpolatedFrame synth;
            synth.frameIndex = m_lastFrameIndex + step;
            synth.timestampMs = m_lastTimestampMs + (step * timeStep);
            synth.isSynthesized = true;
            synth.confidenceScore = 1.0 - (0.05 * step);

            if (m_mode == PacketInterpolationMode::LINEAR_INTERPOLATION) {
                synth.channels = InterpolateLinear(m_lastFrameData, channelData, t);
            } else if (m_mode == PacketInterpolationMode::CUBIC_HERMITE_SPLINE && m_historyBuffer.size() >= 2) {
                const auto& p0 = m_historyBuffer[m_historyBuffer.size() - 2].channels;
                const auto& p1 = m_lastFrameData;
                const auto& p2 = channelData;
                const auto& p3 = channelData; // Projected derivative
                synth.channels = InterpolateCubicHermite(p0, p1, p2, p3, t);
            } else {
                // Adaptive Bezier
                const auto& p0 = (m_historyBuffer.size() >= 2) ? m_historyBuffer[m_historyBuffer.size() - 2].channels : m_lastFrameData;
                synth.channels = InterpolateAdaptiveBezier(p0, m_lastFrameData, channelData, t);
            }

            synthesizedList.push_back(synth);
            m_historyBuffer.push_back(synth);
        }
    }

    IngestReceivedFrame(newFrameIndex, timestampMs, channelData);
    return synthesizedList;
}

PacketLossTelemetryReport AIPacketLossInterpolator::BenchmarkSimulation(int totalFrames, double packetLossRatio, size_t channelCount) {
    PacketLossTelemetryReport report;
    report.mode = m_mode;
    report.totalFramesEvaluated = totalFrames;
    report.packetLossPercent = packetLossRatio * 100.0;

    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    // Generate ground-truth smooth sinusoidal wave pattern
    std::vector<std::vector<uint8_t>> groundTruth(totalFrames);
    for (int f = 0; f < totalFrames; ++f) {
        groundTruth[f].resize(channelCount);
        for (size_t c = 0; c < channelCount; ++c) {
            double angle = (f * 0.08) + (c * 0.02);
            groundTruth[f][c] = static_cast<uint8_t>((std::sin(angle) * 0.5 + 0.5) * 255.0);
        }
    }

    Reset();
    auto startTime = std::chrono::high_resolution_clock::now();

    double totalSquaredError = 0.0;
    int synthCount = 0;
    int dropCount = 0;

    uint32_t lastKnownSent = 0;
    for (int f = 0; f < totalFrames; ++f) {
        bool dropped = (f > 0 && f < totalFrames - 1) && (dist(rng) < packetLossRatio);
        if (dropped) {
            dropCount++;
            continue;
        }

        uint32_t ts = f * 25; // 40 FPS = 25ms per frame
        auto synths = ProcessArrivalAndFillGaps(f, ts, groundTruth[f]);
        for (const auto& s : synths) {
            synthCount++;
            int originalIdx = s.frameIndex;
            if (originalIdx < totalFrames) {
                for (size_t c = 0; c < channelCount; ++c) {
                    double diff = static_cast<double>(s.channels[c]) - static_cast<double>(groundTruth[originalIdx][c]);
                    totalSquaredError += (diff * diff);
                }
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsedUs = std::chrono::duration<double, std::micro>(endTime - startTime).count();

    report.droppedFramesDetected = dropCount;
    report.synthesizedFramesGenerated = synthCount;
    report.processingLatencyUs = (totalFrames > 0) ? (elapsedUs / totalFrames) : 0.0;
    report.throughputFps = (elapsedUs > 0.0) ? (totalFrames / (elapsedUs / 1000000.0)) : 0.0;
    report.meanSquaredError = (synthCount * channelCount > 0) ? (totalSquaredError / (synthCount * channelCount)) : 0.0;

    std::ostringstream ss;
    ss << "Recovered " << synthCount << " / " << dropCount << " lost frames using " << GetModeName(m_mode);
    report.incidentLog.push_back(ss.str());

    return report;
}

std::string PacketLossTelemetryReport::GenerateFormattedReport() const {
    std::ostringstream ss;
    ss << "=================================================================\n";
    ss << "   xLights AI Predictive WiFi Loss Concealment & Benchmarks     \n";
    ss << "=================================================================\n\n";
    ss << "Active Mode:          " << AIPacketLossInterpolator::GetModeName(mode) << "\n";
    ss << "Total Frames Tested:  " << totalFramesEvaluated << "\n";
    ss << "Simulated Loss Rate:  " << std::fixed << std::setprecision(1) << packetLossPercent << " %\n";
    ss << "Dropped Frames:       " << droppedFramesDetected << "\n";
    ss << "Reconstructed Frames: " << synthesizedFramesGenerated << "\n";
    ss << "Mean Squared Error:   " << std::fixed << std::setprecision(3) << meanSquaredError << " (lower is better)\n";
    ss << "Reconstruction Speed: " << std::fixed << std::setprecision(1) << throughputFps << " FPS\n";
    ss << "Latency Per Frame:    " << std::fixed << std::setprecision(2) << processingLatencyUs << " µs\n\n";
    ss << "Log Observations:\n";
    for (const auto& log : incidentLog) {
        ss << "  - " << log << "\n";
    }
    return ss.str();
}

} // namespace xLights::AI
