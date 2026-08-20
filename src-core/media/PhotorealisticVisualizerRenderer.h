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
#include <functional>
#include <nlohmann/json.hpp>

namespace xLights::AI {

enum class ControlNetConditioningType {
    DEPTH = 0,
    LINEART,
    CANNY,
    SOFTEDGE,
    SEGMENTATION
};

enum class AtmosphericSceneStyle {
    HOLIDAY_TWILIGHT = 0,
    DEEP_WINTER_MIDNIGHT,
    CRISP_SNOW_REFLECTION,
    FOGGY_ATMOSPHERE,
    NEIGHBORHOOD_GLOW
};

struct PhotorealisticRenderConfig {
    std::string houseBackgroundImagePath;
    std::string sequenceFseqPath;
    std::string audioTrackPath;
    std::string outputDirectory;
    ControlNetConditioningType conditioningType{ControlNetConditioningType::DEPTH};
    AtmosphericSceneStyle sceneStyle{AtmosphericSceneStyle::HOLIDAY_TWILIGHT};
    float bloomIntensity{1.20f};        // 0.0 to 2.0
    float surfaceReflection{0.45f};     // 0.0 to 1.0
    float ambientStreetLight{0.15f};    // 0.0 to 1.0
    int outputWidth{1920};
    int outputHeight{1080};
    int fps{30};
    int startFrame{0};
    int endFrame{100};
    bool generateVideoMp4{true};
    std::string promptOverride;
    std::string negativePromptOverride;
};

struct PhotorealisticFrameResult {
    int frameIndex{0};
    long timestampMs{0};
    std::string imagePngPath;
    float peakLuminance{0.0f};
    bool success{false};
};

struct PhotorealisticRenderResult {
    bool success{false};
    std::string videoMp4Path;
    std::vector<PhotorealisticFrameResult> renderedFrames;
    int totalFramesRendered{0};
    double totalRenderTimeSec{0.0};
    std::string summaryMessage;

    nlohmann::json ToJson() const;
};

class PhotorealisticVisualizerRenderer {
public:
    PhotorealisticVisualizerRenderer() = default;
    ~PhotorealisticVisualizerRenderer() = default;

    /// Formulates the tailored ControlNet + Stable Diffusion prompt based on atmospheric style and lighting parameters
    static std::string FormulateSDPrompt(
        AtmosphericSceneStyle style,
        float bloomIntensity,
        const std::string& customAddition = ""
    );

    /// Formulates the standard negative prompt to suppress artifacts
    static std::string FormulateNegativePrompt(const std::string& customAddition = "");

    /// Generates a single high-definition photorealistic snapshot frame
    static PhotorealisticFrameResult RenderSingleSnapshot(
        const PhotorealisticRenderConfig& config,
        int frameIndex,
        std::function<void(int pct, const std::string& status)> progress = nullptr
    );

    /// Executes full batch photorealistic sequence rendering and `.mp4` video compilation
    static PhotorealisticRenderResult RenderSequenceVideo(
        const PhotorealisticRenderConfig& config,
        std::function<void(int pct, const std::string& status)> progress = nullptr
    );

    static std::string ConditioningTypeToString(ControlNetConditioningType type);
    static std::string SceneStyleToString(AtmosphericSceneStyle style);
};

} // namespace xLights::AI
