/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

// Feature #7 (High-Level Facade): AutoPropMapper
// Wraps GrayCodePropMapper with a configuration-driven pipeline API.
// Accepts a PropMappingConfig, orchestrates pattern generation, camera
// capture, Gray Code decoding, and exports a PropMappingResult.

#include <string>
#include <vector>
#include <functional>
#include "models/GrayCodePropMapper.h"

namespace xLights {
namespace AI {

// -------------------------------------------------------------------------
// PropMappingConfig — Configuration for a full auto-mapping session
// -------------------------------------------------------------------------

struct PropMappingConfig {
    // Prop geometry
    int totalNodes = 0;               // Total LED/pixel nodes on the prop
    int modelWidth = 0;               // Target Custom Model grid width (cols)
    int modelHeight = 0;              // Target Custom Model grid height (rows)
    std::string modelName = "AI_MappedProp"; // xLights Custom Model name

    // Camera settings
    int cameraIndex = 0;              // OpenCV camera device index
    int cameraWidth = 1280;           // Camera capture resolution width
    int cameraHeight = 720;           // Camera capture resolution height
    int captureDelayMs = 100;         // Delay (ms) between pattern projection and capture

    // Gray Code decode settings
    float thresholdDelta = 10.0f;     // Min brightness delta to confirm a pattern bit
    int sampleIntervalMs = 50;        // Frame sampling interval

    // Output options
    bool exportXML = true;            // Export xLights Custom Model XML
    bool exportCSV = false;           // Export pixel coordinate CSV
    std::string outputDirectory;      // Directory to write output files (empty = in-memory only)

    // Calibration
    bool autoExposure = true;         // Use auto-exposure for camera during capture
    float exposureCompensation = 0.0f; // Manual exposure compensation EV
};

// -------------------------------------------------------------------------
// AutoPropMapper — High-level facade over GrayCodePropMapper
// -------------------------------------------------------------------------

class AutoPropMapper {
public:
    AutoPropMapper() = default;
    ~AutoPropMapper() = default;

    // Run a complete auto-mapping session from config
    // Orchestrates: pattern generation → capture scheduling → Gray Code
    // decode → PropMappingResult assembly → optional file export.
    // progress(pct 0..100, status message)
    static PropMappingResult RunAutoMap(
        const PropMappingConfig& config,
        const std::vector<CameraFrameCapture>& captures,
        std::function<void(int pct, const std::string& status)> progress = nullptr);

    // Build synthetic test captures for offline validation (no camera required)
    // Useful for unit tests and CI pipelines
    static std::vector<CameraFrameCapture> BuildSyntheticCaptures(
        const PropMappingConfig& config);

    // Validate a PropMappingConfig before running a session
    // Returns empty string if valid, or an error description
    static std::string ValidateConfig(const PropMappingConfig& config);

    // Serialize PropMappingResult to JSON string for REST API / MCP response
    static std::string SerializeResultToJSON(const PropMappingResult& result);

    // Deserialize a PropMappingConfig from a JSON string (MCP tool arguments)
    static PropMappingConfig DeserializeConfig(const std::string& json);
};

} // namespace AI
} // namespace xLights
