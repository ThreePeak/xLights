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
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <nlohmann/json.hpp>

namespace xLights {

enum class RenderSchedulerPriority {
    REALTIME_PLAYBACK = 0,
    VIEWPORT_SCRUB,
    BACKGROUND_CACHE,
    OFFLINE_EXPORT
};

struct RenderChunkTask {
    uint32_t chunkId{0};
    int startFrame{0};
    int endFrame{0};
    RenderSchedulerPriority priority{RenderSchedulerPriority::BACKGROUND_CACHE};
    std::function<void(int frame)> renderCallback;
};

struct RenderSchedulerMetrics {
    double averageFrameRenderMs{0.0};
    double peakFrameRenderMs{0.0};
    double throughputFramesPerSec{0.0};
    uint64_t totalFramesRendered{0};
    uint64_t cacheHits{0};
    uint64_t cacheMisses{0};
    double cacheHitRatio{0.0};
    int activeWorkerThreads{0};

    nlohmann::json ToJson() const;
};

class RenderScheduler {
public:
    explicit RenderScheduler(size_t workerThreadCount = 0);
    ~RenderScheduler();

    /// Starts worker threads
    void Start();

    /// Stops worker threads and clears queues
    void Stop();

    /// Enqueues a render task chunk with priority
    void EnqueueTask(const RenderChunkTask& task);

    /// Pre-fetches frames around the playback head
    void PrefetchRange(int currentFrame, int lookaheadFrames, int totalSequenceFrames);

    /// Invalidates cached frames in the specified range
    void InvalidateRange(int startFrame, int endFrame);

    /// Records completed frame render time for profiling metrics
    void RecordFrameRenderTime(double durationMs, bool isCacheHit);

    /// Returns current scheduler telemetry and performance metrics
    RenderSchedulerMetrics GetMetrics() const;

    /// Resets profiling metrics counters
    void ResetMetrics();

    bool IsRunning() const { return m_running.load(); }
    size_t GetWorkerCount() const { return m_workers.size(); }

private:
    void WorkerLoop(size_t workerIndex);

    std::vector<std::thread> m_workers;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_stopRequested{false};

    mutable std::mutex m_queueMutex;
    std::condition_variable m_cv;
    std::vector<RenderChunkTask> m_taskQueue;

    // Telemetry & metrics
    mutable std::mutex m_metricsMutex;
    uint64_t m_totalFramesRendered{0};
    uint64_t m_cacheHits{0};
    uint64_t m_cacheMisses{0};
    double m_totalRenderTimeMs{0.0};
    double m_peakRenderTimeMs{0.0};
};

} // namespace xLights
