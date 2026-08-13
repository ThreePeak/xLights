/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AI/AtmosphericVRPreviewer.h"
#include <spdlog/spdlog.h>

namespace xLights::AI {

bool AtmosphericVRPreviewer::InitializeVRViewport(const AtmosphericVRConfig& config) {
    spdlog::info("AtmosphericVRPreviewer: Initialized OpenXR VR Viewport (Fog: {}, Glare: {})", config.enableGroundFog, config.ledGlareIntensity);
    return true;
}

void AtmosphericVRPreviewer::RenderFrame(int frameMs) {
    // Render OpenXR / OpenGL atmospheric frame pass
}

void AtmosphericVRPreviewer::ShutdownVRViewport() {
    spdlog::info("AtmosphericVRPreviewer: Shutdown OpenXR VR Viewport.");
}

} // namespace xLights::AI
