/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/render/RenderScheduler.h"
#include <iostream>
#include <cassert>
#include <chrono>

using namespace xLights;

int main() {
    std::cout << "[Unit Test] Running RenderScheduler verification..." << std::endl;

    // Test 1: Construction & Thread Allocation
    {
        RenderScheduler sched(4);
        assert(sched.GetWorkerCount() == 4);
        assert(!sched.IsRunning());
        std::cout << " -> Test 1 (Construction & Thread Pool Count): PASSED" << std::endl;
    }

    // Test 2: Start, Enqueue & Execute Tasks
    {
        RenderScheduler sched(2);
        sched.Start();
        assert(sched.IsRunning());

        std::atomic<int> framesRendered{0};
        RenderChunkTask task;
        task.chunkId = 1;
        task.startFrame = 0;
        task.endFrame = 9;
        task.priority = RenderSchedulerPriority::REALTIME_PLAYBACK;
        task.renderCallback = [&framesRendered, &sched](int f) {
            framesRendered++;
            sched.RecordFrameRenderTime(1.5, false);
        };

        sched.EnqueueTask(task);

        // Wait brief moment for worker threads to process
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        assert(framesRendered.load() == 10);
        sched.Stop();
        assert(!sched.IsRunning());
        std::cout << " -> Test 2 (Task Enqueue & Asynchronous Execution): PASSED" << std::endl;
    }

    // Test 3: Metrics & Telemetry
    {
        RenderScheduler sched(2);
        sched.RecordFrameRenderTime(2.0, true);
        sched.RecordFrameRenderTime(4.0, false);

        auto m = sched.GetMetrics();
        assert(m.totalFramesRendered == 2);
        assert(m.cacheHits == 1);
        assert(m.cacheMisses == 1);
        assert(m.averageFrameRenderMs == 3.0);
        assert(m.peakFrameRenderMs == 4.0);
        assert(m.cacheHitRatio == 0.5);
        std::cout << " -> Test 3 (Telemetry & Telemetry Metrics): PASSED" << std::endl;
    }

    // Test 4: Prefetch and Invalidation
    {
        RenderScheduler sched(2);
        sched.PrefetchRange(0, 20, 100);
        sched.InvalidateRange(5, 15);
        sched.ResetMetrics();
        auto m = sched.GetMetrics();
        assert(m.totalFramesRendered == 0);
        std::cout << " -> Test 4 (Prefetch & Cache Invalidation): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] RenderScheduler ALL TESTS PASSED!" << std::endl;
    return 0;
}
