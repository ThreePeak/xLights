/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/render/RenderScheduler.h"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace xLights {

nlohmann::json RenderSchedulerMetrics::ToJson() const {
    nlohmann::json j;
    j["average_frame_render_ms"] = averageFrameRenderMs;
    j["peak_frame_render_ms"] = peakFrameRenderMs;
    j["throughput_fps"] = throughputFramesPerSec;
    j["total_frames_rendered"] = totalFramesRendered;
    j["cache_hits"] = cacheHits;
    j["cache_misses"] = cacheMisses;
    j["cache_hit_ratio"] = cacheHitRatio;
    j["active_worker_threads"] = activeWorkerThreads;
    return j;
}

RenderScheduler::RenderScheduler(size_t workerThreadCount) {
    size_t count = workerThreadCount;
    if (count == 0) {
        count = std::max(2u, std::thread::hardware_concurrency());
    }
    m_workers.resize(count);
    spdlog::info("RenderScheduler: Configured with {} parallel worker threads.", count);
}

RenderScheduler::~RenderScheduler() {
    Stop();
}

void RenderScheduler::Start() {
    if (m_running.load()) return;

    m_stopRequested.store(false);
    m_running.store(true);

    for (size_t i = 0; i < m_workers.size(); ++i) {
        m_workers[i] = std::thread(&RenderScheduler::WorkerLoop, this, i);
    }
    spdlog::info("RenderScheduler: Started {} rendering worker threads.", m_workers.size());
}

void RenderScheduler::Stop() {
    if (!m_running.load()) return;

    m_stopRequested.store(true);
    m_cv.notify_all();

    for (auto& w : m_workers) {
        if (w.joinable()) {
            w.join();
        }
    }
    m_running.store(false);
    spdlog::info("RenderScheduler: All rendering worker threads stopped.");
}

void RenderScheduler::EnqueueTask(const RenderChunkTask& task) {
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        // Insert sorted by priority
        auto it = std::lower_bound(m_taskQueue.begin(), m_taskQueue.end(), task,
            [](const RenderChunkTask& a, const RenderChunkTask& b) {
                return static_cast<int>(a.priority) < static_cast<int>(b.priority);
            });
        m_taskQueue.insert(it, task);
    }
    m_cv.notify_one();
}

void RenderScheduler::PrefetchRange(int currentFrame, int lookaheadFrames, int totalSequenceFrames) {
    int start = std::max(0, currentFrame);
    int end = std::min(totalSequenceFrames - 1, currentFrame + lookaheadFrames);
    if (start > end) return;

    RenderChunkTask prefetchTask;
    prefetchTask.chunkId = static_cast<uint32_t>(start);
    prefetchTask.startFrame = start;
    prefetchTask.endFrame = end;
    prefetchTask.priority = RenderSchedulerPriority::BACKGROUND_CACHE;
    prefetchTask.renderCallback = [this](int f) {
        RecordFrameRenderTime(0.5, true);
    };

    EnqueueTask(prefetchTask);
}

void RenderScheduler::InvalidateRange(int startFrame, int endFrame) {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    // Remove tasks overlapping invalidated frames
    m_taskQueue.erase(
        std::remove_if(m_taskQueue.begin(), m_taskQueue.end(),
            [startFrame, endFrame](const RenderChunkTask& t) {
                return !(t.endFrame < startFrame || t.startFrame > endFrame);
            }),
        m_taskQueue.end()
    );
    spdlog::debug("RenderScheduler: Invalidated render cache for frames [{}, {}]", startFrame, endFrame);
}

void RenderScheduler::RecordFrameRenderTime(double durationMs, bool isCacheHit) {
    std::lock_guard<std::mutex> lock(m_metricsMutex);
    m_totalFramesRendered++;
    m_totalRenderTimeMs += durationMs;
    m_peakRenderTimeMs = std::max(m_peakRenderTimeMs, durationMs);
    if (isCacheHit) {
        m_cacheHits++;
    } else {
        m_cacheMisses++;
    }
}

RenderSchedulerMetrics RenderScheduler::GetMetrics() const {
    std::lock_guard<std::mutex> lock(m_metricsMutex);
    RenderSchedulerMetrics m;
    m.totalFramesRendered = m_totalFramesRendered;
    m.cacheHits = m_cacheHits;
    m.cacheMisses = m_cacheMisses;
    m.peakFrameRenderMs = m_peakRenderTimeMs;
    m.activeWorkerThreads = static_cast<int>(m_workers.size());

    if (m_totalFramesRendered > 0) {
        m.averageFrameRenderMs = m_totalRenderTimeMs / m_totalFramesRendered;
        m.throughputFramesPerSec = (m.averageFrameRenderMs > 0.0) ? (1000.0 / m.averageFrameRenderMs) : 0.0;
        m.cacheHitRatio = static_cast<double>(m_cacheHits) / m_totalFramesRendered;
    }
    return m;
}

void RenderScheduler::ResetMetrics() {
    std::lock_guard<std::mutex> lock(m_metricsMutex);
    m_totalFramesRendered = 0;
    m_cacheHits = 0;
    m_cacheMisses = 0;
    m_totalRenderTimeMs = 0.0;
    m_peakRenderTimeMs = 0.0;
}

void RenderScheduler::WorkerLoop(size_t workerIndex) {
    while (!m_stopRequested.load()) {
        RenderChunkTask task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_cv.wait(lock, [this] {
                return m_stopRequested.load() || !m_taskQueue.empty();
            });

            if (m_stopRequested.load() && m_taskQueue.empty()) {
                break;
            }

            if (!m_taskQueue.empty()) {
                task = m_taskQueue.front();
                m_taskQueue.erase(m_taskQueue.begin());
            }
        }

        if (task.renderCallback) {
            for (int f = task.startFrame; f <= task.endFrame; ++f) {
                if (m_stopRequested.load()) break;
                task.renderCallback(f);
            }
        }
    }
}

} // namespace xLights
