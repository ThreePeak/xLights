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
#include <memory>
#include "src-core/render/RenderScheduler.h"

namespace xLights {

struct RenderBufferSlot {
    int frameIndex{-1};
    uint64_t frameTimestampMs{0};
    std::vector<uint8_t> pixelData;
    bool isReady{false};
};

class RenderEngineSchedulerBridge {
public:
    explicit RenderEngineSchedulerBridge(size_t workerThreads = 0, size_t ringBufferSize = 32);
    ~RenderEngineSchedulerBridge();

    /// Initializes double-buffered ring cache and starts workers
    void Initialize(int channelsPerFrame, int totalSequenceFrames);

    /// Requests high-priority frame rendering for timeline scrub
    void RequestScrubFrame(int frameIndex, std::function<void(const RenderBufferSlot& slot)> onReady);

    /// Dispatches background range caching
    void TriggerBackgroundRangeCache(int startFrame, int endFrame);

    /// SIMD / AVX2 accelerated frame alpha-blending helper
    static void AcceleratedBlendFrames(
        const uint8_t* foreground,
        const uint8_t* background,
        uint8_t* output,
        size_t byteCount,
        uint8_t alpha
    );

    /// Returns current scheduler metrics
    RenderSchedulerMetrics GetMetrics() const;

    void Shutdown();

private:
    std::unique_ptr<RenderScheduler> m_scheduler;
    std::vector<RenderBufferSlot> m_ringBuffer;
    size_t m_ringBufferSize{32};
    int m_channelsPerFrame{0};
    int m_totalSequenceFrames{0};
    mutable std::mutex m_bridgeMutex;
};

} // namespace xLights
