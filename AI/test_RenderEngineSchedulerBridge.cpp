/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/render/RenderEngineSchedulerBridge.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <chrono>

using namespace xLights;

int main() {
    std::cout << "[Unit Test] Running RenderEngineSchedulerBridge verification..." << std::endl;

    // Test 1: Alpha Blend Math
    {
        std::vector<uint8_t> fg = {255, 0, 100};
        std::vector<uint8_t> bg = {0, 255, 100};
        std::vector<uint8_t> out(3, 0);

        RenderEngineSchedulerBridge::AcceleratedBlendFrames(fg.data(), bg.data(), out.data(), 3, 128);
        assert(out[0] >= 127 && out[0] <= 129);
        assert(out[1] >= 127 && out[1] <= 129);
        assert(out[2] == 100);
        std::cout << " -> Test 1 (AcceleratedBlendFrames Alpha Mixing): PASSED" << std::endl;
    }

    // Test 2: Bridge Initialization & Scrub Request
    {
        RenderEngineSchedulerBridge bridge(2, 16);
        bridge.Initialize(512, 100);

        std::atomic<bool> frameReceived{false};
        bridge.RequestScrubFrame(10, [&frameReceived](const RenderBufferSlot& slot) {
            assert(slot.frameIndex == 10);
            assert(slot.isReady);
            assert(slot.pixelData.size() == 512);
            frameReceived.store(true);
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        assert(frameReceived.load());
        bridge.Shutdown();
        std::cout << " -> Test 2 (Bridge Scrub Frame Request & Ring Buffer): PASSED" << std::endl;
    }

    // Test 3: Background Range Cache & Metrics
    {
        RenderEngineSchedulerBridge bridge(2, 16);
        bridge.Initialize(256, 50);
        bridge.TriggerBackgroundRangeCache(0, 10);

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        auto m = bridge.GetMetrics();
        assert(m.totalFramesRendered >= 1);
        bridge.Shutdown();
        std::cout << " -> Test 3 (Background Range Cache & Telemetry): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] RenderEngineSchedulerBridge ALL TESTS PASSED!" << std::endl;
    return 0;
}
