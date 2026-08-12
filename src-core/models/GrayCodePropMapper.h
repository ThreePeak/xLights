/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

// Feature #7: Camera-Based Auto Prop Mapper (Gray Code Detector)
// Generates 2K normal and inverted binary Gray Code light patterns
// (K = ceil(log2(N))) to drive physical prop calibration output.
// Processes camera capture frames, evaluating threshold deltas to
// decode physical pixel locations (X_cam, Y_cam) into 2D custom
// model node matrices.

#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include <cstdint>

namespace xLights {
namespace AI {

// -------------------------------------------------------------------------
// Core Data Structures
// -------------------------------------------------------------------------

// A decoded pixel location from Gray Code camera analysis
struct GrayCodePixelLocation {
    int nodeIndex = -1;          // xLights node index (0-based)
    int camX = 0;                // Camera frame X coordinate (pixels)
    int camY = 0;                // Camera frame Y coordinate (pixels)
    int modelCol = 0;            // Decoded custom model column
    int modelRow = 0;            // Decoded custom model row
    float confidence = 0.0f;     // Decode confidence 0.0 to 1.0
    bool decoded = false;        // True if Gray Code successfully decoded
};

// A single Gray Code illumination pattern for one K-bit pass
struct GrayCodePattern {
    int passIndex = 0;           // Which bit pass (0 = MSB)
    bool inverted = false;       // True = inverted complement pattern
    std::vector<bool> litNodes;  // Which nodes are ON in this pattern
    uint32_t grayCodeMask = 0;   // Gray Code bitmask for this pass
};

// Full result of a Gray Code prop mapping session
struct GrayCodeMapResult {
    bool success = false;
    std::string errorMessage;    // Error description if success == false
    int totalNodes = 0;          // Total prop nodes analyzed
    int decodedNodes = 0;        // Nodes successfully decoded
    int modelWidth = 0;          // Custom model grid width
    int modelHeight = 0;         // Custom model grid height
    std::vector<GrayCodePixelLocation> pixels; // Per-node decoded locations
    std::string customModelXML;  // Exported xLights Custom Model XML
};

// High-level prop mapping result — aggregates GrayCodeMapResult with
// OpenCV camera metadata and calibration quality diagnostics
struct PropMappingResult {
    bool success = false;
    std::string errorMessage;    // Error description if success == false
    std::string modelName;       // xLights Custom Model name
    int totalNodes = 0;          // Total prop nodes requested
    int mappedNodes = 0;         // Successfully mapped nodes
    int unmappedNodes = 0;       // Nodes that failed to decode
    float coveragePct = 0.0f;    // Percentage of nodes mapped (0.0 - 100.0)
    float avgConfidence = 0.0f;  // Average per-node decode confidence
    int cameraWidth = 0;         // Camera frame width (pixels)
    int cameraHeight = 0;        // Camera frame height (pixels)
    int patternCount = 0;        // Total Gray Code patterns projected (2*K)
    std::string customModelXML;  // Exported xLights Custom Model XML
    std::string csvData;         // Pixel coordinate table (CSV format)
    GrayCodeMapResult rawResult; // Underlying decode result
};

// Camera frame capture for a single Gray Code pattern exposure
struct CameraFrameCapture {
    int patternIndex = 0;        // Index into pattern sequence
    bool isInverted = false;     // Whether capture is for inverted pattern
    std::vector<uint8_t> pixels; // Grayscale pixel buffer (width * height bytes)
    int width = 0;
    int height = 0;
};

// -------------------------------------------------------------------------
// GrayCodePropMapper
// -------------------------------------------------------------------------

class GrayCodePropMapper {
public:
    GrayCodePropMapper() = default;
    ~GrayCodePropMapper() = default;

    // Step 1: Generate the full 2K normal + inverted Gray Code pattern sequence
    // K = ceil(log2(N)) patterns per axis
    // Returns the ordered sequence of patterns to project onto the prop
    static std::vector<GrayCodePattern> GenerateGrayCodePatterns(int totalNodes);

    // Step 2: Decode camera frames for each pattern pair into pixel locations
    // Evaluates threshold delta: Bit_k = Frame_pattern > Frame_inverse_pattern
    // Returns one GrayCodePixelLocation per node
    static std::vector<GrayCodePixelLocation> DecodeCameraFrames(
        const std::vector<CameraFrameCapture>& captures,
        int totalNodes,
        int modelWidth,
        int modelHeight,
        float thresholdDelta = 10.0f);

    // Step 3: Run full end-to-end Gray Code mapping session
    // progress callback receives pct 0..100
    static GrayCodeMapResult RunMappingSession(
        int totalNodes,
        int modelWidth,
        int modelHeight,
        const std::vector<CameraFrameCapture>& captures,
        std::function<void(int pct, const std::string& status)> progress = nullptr);

    // Step 4: Export mapped pixel grid to xLights Custom Model XML format
    // Produces a <custommodel> element compatible with xLights layout import
    static std::string ExportToCustomModelXML(
        const GrayCodeMapResult& result,
        const std::string& modelName = "AI_MappedProp");

    // Step 5: Export pixel coordinate table as CSV for external calibration
    static std::string ExportToCSV(const GrayCodeMapResult& result);

    // Utility: Convert binary index to Gray Code: G(n) = n XOR (n >> 1)
    static constexpr uint32_t BinaryToGray(uint32_t n) noexcept { return n ^ (n >> 1); }

    // Utility: Convert Gray Code back to binary index
    static uint32_t GrayToBinary(uint32_t gray) noexcept;

    // Utility: Compute required bit depth K = ceil(log2(N))
    static int ComputeBitDepth(int n) noexcept {
        if (n <= 1) return 1;
        return (int)std::ceil(std::log2((double)n));
    }

    // Pattern bit comparator: Bit_k = Frame_pattern > Frame_inverse_pattern
    static bool ComputePatternBit(float framePattern, float frameInversePattern) noexcept {
        return framePattern > frameInversePattern;
    }
};

} // namespace AI
} // namespace xLights
