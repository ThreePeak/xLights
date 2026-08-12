/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "models/AutoPropMapper.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <algorithm>

namespace xLights {
namespace AI {

// -------------------------------------------------------------------------
// ValidateConfig
// -------------------------------------------------------------------------

std::string AutoPropMapper::ValidateConfig(const PropMappingConfig& config) {
    if (config.totalNodes <= 0)
        return "totalNodes must be > 0";
    if (config.modelWidth <= 0)
        return "modelWidth must be > 0";
    if (config.modelHeight <= 0)
        return "modelHeight must be > 0";
    if (config.modelName.empty())
        return "modelName must not be empty";
    if (config.cameraWidth <= 0 || config.cameraHeight <= 0)
        return "cameraWidth and cameraHeight must be > 0";
    if (config.thresholdDelta < 0.0f)
        return "thresholdDelta must be >= 0.0";
    return {};
}

// -------------------------------------------------------------------------
// BuildSyntheticCaptures
// Generates synthetic normal+inverted Gray Code camera frames for testing
// without requiring a physical camera or projected patterns
// -------------------------------------------------------------------------

std::vector<CameraFrameCapture> AutoPropMapper::BuildSyntheticCaptures(
    const PropMappingConfig& config)
{
    int N = config.totalNodes;
    int K = GrayCodePropMapper::ComputeBitDepth(N);
    int camW = config.cameraWidth;
    int camH = config.cameraHeight;

    std::vector<CameraFrameCapture> captures;
    captures.reserve(2 * K);

    for (int k = K - 1; k >= 0; --k) {
        uint32_t mask = (1u << k);

        CameraFrameCapture normal;
        normal.patternIndex = K - 1 - k;
        normal.isInverted   = false;
        normal.width        = camW;
        normal.height       = camH;
        normal.pixels.resize(camW * camH, 0);

        CameraFrameCapture inv;
        inv.patternIndex    = normal.patternIndex;
        inv.isInverted      = true;
        inv.width           = camW;
        inv.height          = camH;
        inv.pixels.resize(camW * camH, 0);

        for (int n = 0; n < N; ++n) {
            int xStart = n * camW / N;
            int xEnd   = (n + 1) * camW / N;
            uint32_t gray = GrayCodePropMapper::BinaryToGray((uint32_t)n);
            bool bit = (gray & mask) != 0;
            for (int row = 0; row < camH; ++row) {
                for (int col = xStart; col < xEnd && col < camW; ++col) {
                    normal.pixels[row * camW + col] = bit ? 220 : 30;
                    inv.pixels[row * camW + col]    = bit ? 30 : 220;
                }
            }
        }

        captures.push_back(std::move(normal));
        captures.push_back(std::move(inv));
    }

    spdlog::info("AutoPropMapper: Built {} synthetic captures for {} nodes (K={}).",
                 captures.size(), N, K);
    return captures;
}

// -------------------------------------------------------------------------
// RunAutoMap
// Orchestrates: validate → generate patterns → decode → assemble result
// -------------------------------------------------------------------------

PropMappingResult AutoPropMapper::RunAutoMap(
    const PropMappingConfig& config,
    const std::vector<CameraFrameCapture>& captures,
    std::function<void(int pct, const std::string& status)> progress)
{
    PropMappingResult pr;
    pr.modelName  = config.modelName;
    pr.totalNodes = config.totalNodes;

    // Validate config
    std::string validationError = ValidateConfig(config);
    if (!validationError.empty()) {
        pr.errorMessage = "Config validation failed: " + validationError;
        spdlog::error("AutoPropMapper: {}", pr.errorMessage);
        return pr;
    }

    if (progress) progress(5, "Validating configuration...");

    // Determine camera dimensions from captures or config
    int camW = (!captures.empty() && captures[0].width  > 0)
               ? captures[0].width  : config.cameraWidth;
    int camH = (!captures.empty() && captures[0].height > 0)
               ? captures[0].height : config.cameraHeight;

    if (progress) progress(15, "Running Gray Code mapping session...");

    // Run core Gray Code mapping
    GrayCodeMapResult raw = GrayCodePropMapper::RunMappingSession(
        config.totalNodes, config.modelWidth, config.modelHeight, captures,
        [&progress](int pct, const std::string& status) {
            if (progress) progress(15 + pct * 70 / 100, status);
        });

    if (progress) progress(88, "Assembling PropMappingResult...");

    // Populate PropMappingResult
    pr.success      = raw.success;
    pr.errorMessage = raw.errorMessage;
    pr.mappedNodes  = raw.decodedNodes;
    pr.unmappedNodes = raw.totalNodes - raw.decodedNodes;
    pr.coveragePct  = raw.totalNodes > 0
                      ? (float)raw.decodedNodes / (float)raw.totalNodes * 100.0f
                      : 0.0f;
    pr.cameraWidth  = camW;
    pr.cameraHeight = camH;
    pr.patternCount = 2 * GrayCodePropMapper::ComputeBitDepth(config.totalNodes);
    pr.customModelXML = raw.customModelXML;
    pr.rawResult    = raw;

    // Compute average decode confidence
    float sumConf = 0.0f;
    for (const auto& px : raw.pixels) {
        if (px.decoded) sumConf += px.confidence;
    }
    pr.avgConfidence = (raw.decodedNodes > 0) ? sumConf / raw.decodedNodes : 0.0f;

    if (config.exportCSV) {
        pr.csvData = GrayCodePropMapper::ExportToCSV(raw);
    }

    if (progress) progress(100, "Auto-mapping complete.");
    spdlog::info("AutoPropMapper: Complete. {}/{} nodes mapped ({:.1f}% coverage). Avg confidence: {:.2f}",
                 pr.mappedNodes, pr.totalNodes, pr.coveragePct, pr.avgConfidence);
    return pr;
}

// -------------------------------------------------------------------------
// SerializeResultToJSON
// -------------------------------------------------------------------------

std::string AutoPropMapper::SerializeResultToJSON(const PropMappingResult& result) {
    nlohmann::json j;
    j["success"]         = result.success;
    j["errorMessage"]    = result.errorMessage;
    j["modelName"]       = result.modelName;
    j["totalNodes"]      = result.totalNodes;
    j["mappedNodes"]     = result.mappedNodes;
    j["unmappedNodes"]   = result.unmappedNodes;
    j["coveragePct"]     = result.coveragePct;
    j["avgConfidence"]   = result.avgConfidence;
    j["cameraWidth"]     = result.cameraWidth;
    j["cameraHeight"]    = result.cameraHeight;
    j["patternCount"]    = result.patternCount;
    j["customModelXML"]  = result.customModelXML;
    j["csvData"]         = result.csvData;
    return j.dump(2);
}

// -------------------------------------------------------------------------
// DeserializeConfig
// -------------------------------------------------------------------------

PropMappingConfig AutoPropMapper::DeserializeConfig(const std::string& json) {
    PropMappingConfig config;
    try {
        auto j = nlohmann::json::parse(json);
        if (j.contains("total_nodes"))    config.totalNodes    = j["total_nodes"];
        if (j.contains("model_width"))    config.modelWidth    = j["model_width"];
        if (j.contains("model_height"))   config.modelHeight   = j["model_height"];
        if (j.contains("model_name"))     config.modelName     = j["model_name"];
        if (j.contains("camera_index"))   config.cameraIndex   = j["camera_index"];
        if (j.contains("camera_width"))   config.cameraWidth   = j["camera_width"];
        if (j.contains("camera_height"))  config.cameraHeight  = j["camera_height"];
        if (j.contains("threshold_delta"))config.thresholdDelta= j["threshold_delta"];
        if (j.contains("export_csv"))     config.exportCSV     = j["export_csv"];
        if (j.contains("output_directory")) config.outputDirectory = j["output_directory"];
    } catch (const std::exception& e) {
        spdlog::error("AutoPropMapper::DeserializeConfig: JSON parse error: {}", e.what());
    }
    return config;
}

} // namespace AI
} // namespace xLights
