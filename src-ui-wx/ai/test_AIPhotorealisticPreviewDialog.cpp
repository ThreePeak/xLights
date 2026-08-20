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
    std::cout << "[Unit Test] Running AIPhotorealisticPreviewDialog backend verification..." << std::endl;

    std::string prompt = PhotorealisticVisualizerRenderer::FormulateSDPrompt(
        AtmosphericSceneStyle::CRISP_SNOW_REFLECTION, 1.0f);
    assert(prompt.find("sparkling white snow") != std::string::npos);
    assert(prompt.find("photorealistic") != std::string::npos);

    PhotorealisticRenderConfig cfg;
    cfg.fps = 30;
    auto frame = PhotorealisticVisualizerRenderer::RenderSingleSnapshot(cfg, 0, nullptr);
    assert(frame.success);
    assert(frame.timestampMs == 0);

    std::cout << " -> AIPhotorealisticPreviewDialog prompt and snapshot pipeline: PASSED" << std::endl;
    return 0;
}
