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
#include <nlohmann/json.hpp>

namespace xLights::AI {

/**
 * @brief Represents a spectator vantage photo with calibrated distance/measurement data.
 */
struct SpectatorVantageImage {
    std::string imagePath;
    std::string vantageTag{"Driver In-Car View"}; ///< e.g. "Driver In-Car", "Sidewalk Center", "Across Street"
    float measuredDistanceFeet{45.0f};           ///< Measured ground distance from camera to front lawn/props
    float cameraHeightFeet{3.8f};               ///< Measured height of camera from ground (e.g. 3.8ft car, 5.5ft standing)
    float horizontalAngleDegrees{0.0f};         ///< Angle offset relative to layout center line (-90 to +90)
    float fieldOfViewDegrees{65.0f};            ///< Camera lens horizontal FOV estimate
};

/**
 * @brief Per-prop visibility and sightline metrics from a specific vantage point.
 */
struct PropVisibilityRating {
    std::string propName;
    float visibilityScore{100.0f};              ///< 0.0 to 100.0% visual clarity
    float occlusionPercent{0.0f};               ///< 0.0 to 100.0% of prop blocked by other objects
    std::vector<std::string> occludingPropNames;///< Props that obstruct this prop
    float perspectiveDistortionFactor{1.0f};    ///< 1.0 = ideal perpendicular aspect ratio
    float contrastProminenceScore{85.0f};       ///< Visual pop against background/skyline
    float recommendedTiltDegrees{0.0f};         ///< e.g. +4.5 degrees forward tilt
    float recommendedElevationInches{0.0f};     ///< e.g. +6.0 inches elevation
    std::string actionRecommendation;           ///< Plaintext actionable advice

    [[nodiscard]] nlohmann::json ToJson() const;
};

/**
 * @brief Overall sightline optimization evaluation result.
 */
struct SightlineOptimizationResult {
    bool overallSightlineOptimal{true};
    float overallShowVisibilityIndex{92.5f};    ///< 0 to 100% composite score across all vantage spots
    size_t totalOcclusionConflictsDetected{0};

    std::vector<SpectatorVantageImage> vantagePoints;
    std::vector<PropVisibilityRating> propRatings;
    std::vector<std::string> executiveSummaryBullets;

    [[nodiscard]] std::string GenerateFormattedReportText() const;
    [[nodiscard]] nlohmann::json ToJson() const;
};

/**
 * @brief Input layout model coordinate descriptor for pure C++ testing/execution.
 */
struct LayoutModelDescriptor {
    std::string name;
    float worldX{0.0f};     ///< Feet relative to layout center
    float worldY{0.0f};     ///< Feet height from ground
    float worldZ{0.0f};     ///< Feet depth back from curb
    float widthFeet{10.0f};
    float heightFeet{15.0f};
    float tiltDegrees{0.0f};
};

/**
 * @brief Automated Audience Sightline & Visibility Optimizer AI Engine.
 */
class AudienceViewingOptimizerAI {
public:
    /**
     * @brief Evaluates sightlines and calculates 3D raycasting occlusion across vantage points.
     */
    static SightlineOptimizationResult EvaluateSightlines(
        const std::vector<SpectatorVantageImage>& vantageImages,
        const std::vector<LayoutModelDescriptor>& layoutModels
    );

    /**
     * @brief Computes 2D projected bounding box for a model from a specific spectator camera.
     */
    static void ProjectModelToScreen(
        const LayoutModelDescriptor& model,
        const SpectatorVantageImage& vantage,
        float& outScreenX,
        float& outScreenY,
        float& outScreenW,
        float& outScreenH
    );

    /**
     * @brief Calculates overlap/occlusion percentage between front and back props.
     */
    static float CalculateOcclusion(
        const LayoutModelDescriptor& frontModel,
        const LayoutModelDescriptor& backModel,
        const SpectatorVantageImage& vantage
    );
};

} // namespace xLights::AI
