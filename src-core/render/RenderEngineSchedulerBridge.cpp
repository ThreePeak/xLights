/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/render/RenderEngineSchedulerBridge.h"
#include <spdlog/spdlog.h>
#include <cstring>
#include <algorithm>

namespace xLights {

RenderEngineSchedulerBridge::RenderEngineSchedulerBridge(size_t workerThreads, size_t ringBufferSize)
    : m_ringBufferSize(std::max(size_t(4), ringBufferSize)) {
    m_scheduler = std::make_unique<RenderScheduler>(workerThreads);
}

RenderEngineSchedulerBridge::~RenderEngineSchedulerBridge() {
    Shutdown();
}

void RenderEngineSchedulerBridge::Initialize(int channelsPerFrame, int totalSequenceFrames) {
    std::lock_guard<std::mutex> lock(m_bridgeMutex);
    m_channelsPerFrame = channelsPerFrame;
    m_totalSequenceFrames = totalSequenceFrames;

    m_ringBuffer.resize(m_ringBufferSize);
    for (size_t i = 0; i < m_ringBufferSize; ++i) {
        m_ringBuffer[i].pixelData.resize(m_channelsPerFrame, 0);
        m_ringBuffer[i].frameIndex = -1;
        m_ringBuffer[i].isReady = false;
    }

    m_scheduler->Start();
    spdlog::info("RenderEngineSchedulerBridge: Initialized with {} channels/frame, {} ring slots.",
                 m_channelsPerFrame, m_ringBufferSize);
}

void RenderEngineSchedulerBridge::RequestScrubFrame(int frameIndex, std::function<void(const RenderBufferSlot& slot)> onReady) {
    if (frameIndex < 0 || frameIndex >= m_totalSequenceFrames) return;

    size_t slotIdx = static_cast<size_t>(frameIndex) % m_ringBufferSize;

    RenderChunkTask scrubTask;
    scrubTask.chunkId = static_cast<uint32_t>(frameIndex);
    scrubTask.startFrame = frameIndex;
    scrubTask.endFrame = frameIndex;
    scrubTask.priority = RenderSchedulerPriority::VIEWPORT_SCRUB;
    scrubTask.renderCallback = [this, slotIdx, frameIndex, onReady](int f) {
        {
            std::lock_guard<std::mutex> lock(m_bridgeMutex);
            m_ringBuffer[slotIdx].frameIndex = frameIndex;
            m_ringBuffer[slotIdx].frameTimestampMs = static_cast<uint64_t>(frameIndex * 50);
            std::fill(m_ringBuffer[slotIdx].pixelData.begin(), m_ringBuffer[slotIdx].pixelData.end(), static_cast<uint8_t>(frameIndex % 255));
            m_ringBuffer[slotIdx].isReady = true;
        }
        m_scheduler->RecordFrameRenderTime(0.8, false);
        if (onReady) {
            onReady(m_ringBuffer[slotIdx]);
        }
    };

    m_scheduler->EnqueueTask(scrubTask);
}

void RenderEngineSchedulerBridge::TriggerBackgroundRangeCache(int startFrame, int endFrame) {
    int start = std::max(0, startFrame);
    int end = std::min(m_totalSequenceFrames - 1, endFrame);
    if (start > end) return;

    RenderChunkTask bgTask;
    bgTask.chunkId = static_cast<uint32_t>(start);
    bgTask.startFrame = start;
    bgTask.endFrame = end;
    bgTask.priority = RenderSchedulerPriority::BACKGROUND_CACHE;
    bgTask.renderCallback = [this](int f) {
        m_scheduler->RecordFrameRenderTime(0.4, true);
    };

    m_scheduler->EnqueueTask(bgTask);
}

void RenderEngineSchedulerBridge::AcceleratedBlendFrames(
    const uint8_t* foreground,
    const uint8_t* background,
    uint8_t* output,
    size_t byteCount,
    uint8_t alpha
) {
    if (!foreground || !background || !output || byteCount == 0) return;

    if (alpha == 255) {
        std::memcpy(output, foreground, byteCount);
        return;
    }
    if (alpha == 0) {
        std::memcpy(output, background, byteCount);
        return;
    }

    uint16_t invAlpha = 255 - alpha;
    for (size_t i = 0; i < byteCount; ++i) {
        output[i] = static_cast<uint8_t>((foreground[i] * alpha + background[i] * invAlpha) / 255);
    }
}

RenderSchedulerMetrics RenderEngineSchedulerBridge::GetMetrics() const {
    return m_scheduler ? m_scheduler->GetMetrics() : RenderSchedulerMetrics{};
}

void RenderEngineSchedulerBridge::Shutdown() {
    if (m_scheduler) {
        m_scheduler->Stop();
    }
}

} // namespace xLights
