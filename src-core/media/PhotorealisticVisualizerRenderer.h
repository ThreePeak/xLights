#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <functional>
#include <string>
#include <vector>

struct VisualizerRenderOptions {
    std::string fseqFilePath;
    std::string housePhotoPath;
    std::string audioFilePath;
    std::string outputVideoPath = "photorealistic_show_preview.mp4";
    std::string prompt = "Photorealistic nighttime Christmas light show on a suburban house, glowing vibrant LED pixels, 8k resolution, cinematic lighting";
    int frameWidth = 1920;
    int frameHeight = 1080;
    int fps = 20; // 50ms frames
    float controlNetStrength = 0.85f;
    bool enableLocalSD = true; // Use local OpenVINO / CoreML SD ControlNet
};

struct VisualizerRenderResult {
    bool success = false;
    int totalFramesRendered = 0;
    std::string outputVideoPath;
    double durationSeconds = 0.0;
    std::string summaryReport;
    std::string errorMessage;
};

class PhotorealisticVisualizerRenderer {
public:
    PhotorealisticVisualizerRenderer() = default;
    ~PhotorealisticVisualizerRenderer() = default;

    // Render ControlNet conditioning frame combining house photo + fseq light state
    static std::vector<uint8_t> GenerateControlNetConditioningFrame(const std::string& housePhotoPath,
                                                                      const std::vector<uint8_t>& channelFrameData,
                                                                      int width, int height);

    // Complete photorealistic video rendering pipeline (.fseq + ControlNet SD -> .mp4)
    static VisualizerRenderResult RenderPhotorealisticVideo(const VisualizerRenderOptions& options,
                                                            std::function<void(int pct)> progress = nullptr);
};
