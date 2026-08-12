#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AISubsystemBase.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <future>

namespace xLights::AI {

struct MappedPixelNode {
    int pixelIndex = 0;
    float normalizedX = 0.0f; // 0.0 to 1.0 visualizer coordinate
    float normalizedY = 0.0f; // 0.0 to 1.0 visualizer coordinate
    float confidence = 0.0f;  // 0.0 to 1.0 detection confidence
};

struct PropMappingConfig {
    std::string modelTargetName = "AI_MappedProp";
    int totalPixelCount = 0;
    int cameraDeviceId = 0;       // Local camera index or HTTP stream URL
    int patternHoldTimeMs = 100;  // Frame delay per pattern display
    bool enable3DDepthMapping = false; // Multi-camera stereo triangulation flag
};

struct PropMappingResult {
    bool success = false;
    std::string errorMessage;
    std::string modelName;
    std::vector<MappedPixelNode> mappedPixels;
    std::string generatedCustomModelXML; // Output for CustomModel data block
};

/**
 * @brief Feature #7: Camera-Based Auto Prop Mapper
 * Generates Gray Code patterns (2*N normal + inverse), decodes camera frames,
 * and exports xLights CustomModel XML node grids.
 */
class AutoPropMapper : public AISubsystemBase {
public:
    AutoPropMapper(ServiceManager* sm = nullptr) : AISubsystemBase(sm) {}
    virtual ~AutoPropMapper() override = default;

    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override {
        return std::async(std::launch::async, [this, callback]() {
            if (callback) callback("Initializing AutoPropMapper...", 0.0f);
            m_isInitialized.store(true);
            if (callback) callback("AutoPropMapper initialized.", 100.0f);
            return true;
        });
    }

    virtual void Shutdown() override {
        m_isInitialized.store(false);
    }

    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "AutoPropMapper"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"gray_code_pattern_generation", "camera_frame_decoding", "custom_model_xml_export"};
    }

    // Generate sequence of Gray Code binary frame matrices for display
    [[nodiscard]] static std::vector<std::vector<bool>> GenerateGrayCodePatterns(int pixelCount);

    // Process camera capture frames to decode physical pixel X/Y coordinates
    [[nodiscard]] static PropMappingResult ProcessCameraMapping(const PropMappingConfig& config);

    // Format decoded pixel array into xLights CustomModel XML structure
    [[nodiscard]] static std::string ExportToCustomModelXML(
        const std::string& modelName,
        const std::vector<MappedPixelNode>& nodes,
        int gridWidth,
        int gridHeight);
};

} // namespace xLights::AI
