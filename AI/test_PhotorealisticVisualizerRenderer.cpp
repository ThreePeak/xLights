/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/media/PhotorealisticVisualizerRenderer.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running PhotorealisticVisualizerRenderer verification..." << std::endl;

    // Test 1: FormulateSDPrompt contains keywords
    {
        std::string prompt = PhotorealisticVisualizerRenderer::FormulateSDPrompt(
            AtmosphericSceneStyle::HOLIDAY_TWILIGHT, 1.2f, "snowy pine trees");
        assert(prompt.find("photorealistic") != std::string::npos);
        assert(prompt.find("twilight") != std::string::npos);
        assert(prompt.find("snowy pine trees") != std::string::npos);
        std::cout << " -> Test 1 (FormulateSDPrompt Keyword Extraction): PASSED" << std::endl;
    }

    // Test 2: Negative prompt contains artifact suppressors
    {
        std::string neg = PhotorealisticVisualizerRenderer::FormulateNegativePrompt("oversaturated");
        assert(neg.find("daytime") != std::string::npos);
        assert(neg.find("blurry") != std::string::npos);
        assert(neg.find("oversaturated") != std::string::npos);
        std::cout << " -> Test 2 (Negative Prompt Formatting): PASSED" << std::endl;
    }

    // Test 3: RenderSingleSnapshot frame generation
    {
        PhotorealisticRenderConfig cfg;
        cfg.bloomIntensity = 1.5f;
        cfg.outputDirectory = "./test_out";
        cfg.fps = 30;

        auto frameRes = PhotorealisticVisualizerRenderer::RenderSingleSnapshot(cfg, 15, nullptr);
        assert(frameRes.success);
        assert(frameRes.frameIndex == 15);
        assert(frameRes.timestampMs == 500);
        assert(frameRes.peakLuminance > 0.5f);
        std::cout << " -> Test 3 (RenderSingleSnapshot Snapshot Generation): PASSED" << std::endl;
    }

    // Test 4: Batch Sequence Video Render
    {
        PhotorealisticRenderConfig cfg;
        cfg.startFrame = 0;
        cfg.endFrame = 10;
        cfg.fps = 30;
        cfg.outputDirectory = "./test_out";

        auto vidRes = PhotorealisticVisualizerRenderer::RenderSequenceVideo(cfg, nullptr);
        assert(vidRes.success);
        assert(vidRes.totalFramesRendered == 11);
        assert(!vidRes.videoMp4Path.empty());
        assert(!vidRes.summaryMessage.empty());
        std::cout << " -> Test 4 (RenderSequenceVideo Batch Compilation): PASSED" << std::endl;
    }

    // Test 5: String conversions
    {
        assert(PhotorealisticVisualizerRenderer::ConditioningTypeToString(ControlNetConditioningType::DEPTH) == "Depth Map");
        assert(PhotorealisticVisualizerRenderer::SceneStyleToString(AtmosphericSceneStyle::CRISP_SNOW_REFLECTION) == "Crisp Snow Reflection");
        std::cout << " -> Test 5 (Enum to String Serializers): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] PhotorealisticVisualizerRenderer ALL TESTS PASSED!" << std::endl;
    return 0;
}
