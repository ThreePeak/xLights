/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <vector>
#include <string>
#include <deque>
#include <cstdint>
#include <chrono>

namespace xLights::AI {

enum class PacketInterpolationMode {
    DISABLED,
    LINEAR_INTERPOLATION,
    CUBIC_HERMITE_SPLINE,
    ADAPTIVE_LOOKAHEAD_BEZIER
};

struct InterpolatedFrame {
    uint32_t frameIndex{0};
    uint32_t timestampMs{0};
    std::vector<uint8_t> channels;
    bool isSynthesized{false};
    double confidenceScore{1.0};
};

struct PacketLossTelemetryReport {
    PacketInterpolationMode mode{PacketInterpolationMode::ADAPTIVE_LOOKAHEAD_BEZIER};
    int totalFramesEvaluated{0};
    int droppedFramesDetected{0};
    int synthesizedFramesGenerated{0};
    double packetLossPercent{0.0};
    double meanSquaredError{0.0};
    double throughputFps{0.0};
    double processingLatencyUs{0.0};
    std::vector<std::string> incidentLog;

    std::string GenerateFormattedReport() const;
};

class AIPacketLossInterpolator {
public:
    AIPacketLossInterpolator(PacketInterpolationMode mode = PacketInterpolationMode::ADAPTIVE_LOOKAHEAD_BEZIER, int lookaheadWindow = 4);
    ~AIPacketLossInterpolator() = default;

    void SetMode(PacketInterpolationMode mode) { m_mode = mode; }
    PacketInterpolationMode GetMode() const { return m_mode; }

    void SetLookaheadWindow(int windowFrames) { m_lookaheadWindow = windowFrames; }
    int GetLookaheadWindow() const { return m_lookaheadWindow; }

    void SetChannelCount(size_t channelCount) { m_channelCount = channelCount; }
    size_t GetChannelCount() const { return m_channelCount; }

    void Reset();

    /// Ingest a frame received from WiFi (DDP/E1.31/Art-Net)
    void IngestReceivedFrame(uint32_t frameIndex, uint32_t timestampMs, const std::vector<uint8_t>& channelData);

    /// Synthesizes missing frames between last known and new arrival
    std::vector<InterpolatedFrame> ProcessArrivalAndFillGaps(uint32_t newFrameIndex, uint32_t timestampMs, const std::vector<uint8_t>& channelData);

    /// Offline benchmark & stress simulation tool
    PacketLossTelemetryReport BenchmarkSimulation(int totalFrames, double packetLossRatio, size_t channelCount);

    static std::string GetModeName(PacketInterpolationMode mode);

private:
    std::vector<uint8_t> InterpolateLinear(const std::vector<uint8_t>& before, const std::vector<uint8_t>& after, double t);
    std::vector<uint8_t> InterpolateCubicHermite(const std::vector<uint8_t>& p0, const std::vector<uint8_t>& p1,
                                                 const std::vector<uint8_t>& p2, const std::vector<uint8_t>& p3, double t);
    std::vector<uint8_t> InterpolateAdaptiveBezier(const std::vector<uint8_t>& p0, const std::vector<uint8_t>& p1,
                                                   const std::vector<uint8_t>& p2, double t);

    PacketInterpolationMode m_mode{PacketInterpolationMode::ADAPTIVE_LOOKAHEAD_BEZIER};
    int m_lookaheadWindow{4};
    size_t m_channelCount{1500};
    uint32_t m_lastFrameIndex{0};
    uint32_t m_lastTimestampMs{0};
    std::vector<uint8_t> m_lastFrameData;
    std::deque<InterpolatedFrame> m_historyBuffer;
};

} // namespace xLights::AI
