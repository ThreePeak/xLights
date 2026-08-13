/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <string>

namespace xLights::AI {

struct AtmosphericVRConfig {
    bool enableGroundFog = true;
    bool enableSnowParticles = true;
    bool enableVolumetricLightCones = true;
    float fogDensity = 0.35f;
    float ledGlareIntensity = 1.2f;
};

class AtmosphericVRPreviewer {
public:
    AtmosphericVRPreviewer() = default;
    ~AtmosphericVRPreviewer() = default;

    static bool InitializeVRViewport(const AtmosphericVRConfig& config);
    static void RenderFrame(int frameMs);
    static void ShutdownVRViewport();
};

} // namespace xLights::AI
