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
#include <pugixml.hpp>
#include <nlohmann/json.hpp>

namespace xLights {

enum class ReconstructionMode {
    SINGLE_IMAGE,
    MULTI_IMAGE_PHOTOGRAMMETRY
};

enum class CameraViewAngle {
    FRONT = 0,
    ANGLE_45_LEFT,
    ANGLE_45_RIGHT,
    SIDE_PROFILE,
    TOP_DOWN,
    CUSTOM
};

struct ReconstructorInputImage {
    std::string filePath;
    CameraViewAngle viewAngle{CameraViewAngle::FRONT};
    float customAngleDegrees{0.0f};
    float confidenceWeight{1.0f};
};

struct ReconstructedPropNode {
    int nodeIndex{0};
    int strandIndex{0};
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    std::string submodelGroup;
    bool isSelected{false};
};

struct ReconstructorParameters {
    ReconstructionMode mode{ReconstructionMode::SINGLE_IMAGE};
    float edgeSensitivity{0.5f};     // 0.0 to 1.0
    int contourSmoothing{3};          // 0 to 10
    float nodeSpacingInches{2.0f};    // 0.5 to 6.0 inches
    float depthCurvatureScale{1.0f};  // 0.0 (flat 2D) to 2.0 (deep 3D)
    int targetMaxNodes{500};
    bool autoDetectSubmodels{true};
    float gridSnapIncrement{0.5f};    // inches
};

struct ReconstructedPropModel {
    std::string propName{"NewPhotoReconstructedProp"};
    std::vector<ReconstructedPropNode> nodes;
    std::vector<std::string> submodels;
    float boundingWidthInches{0.0f};
    float boundingHeightInches{0.0f};
    float boundingDepthInches{0.0f};
    std::string sourceInfo;

    std::string ExportXModelXml() const;
    std::string ExportObjMesh() const;
    std::string ExportSvgPath() const;
    std::string ExportCsvCoordinates() const;
    nlohmann::json ToJson() const;
};

class Photo3DPropReconstructorAI {
public:
    Photo3DPropReconstructorAI() = default;
    ~Photo3DPropReconstructorAI() = default;

    /// Reconstructs 3D prop geometry from a single photo
    static ReconstructedPropModel ReconstructFromSingleImage(
        const std::string& imagePath,
        const ReconstructorParameters& params
    );

    /// Reconstructs 3D prop geometry from multiple photos taken at different angles
    static ReconstructedPropModel ReconstructFromMultiViewImages(
        const std::vector<ReconstructorInputImage>& inputImages,
        const ReconstructorParameters& params
    );

    // ==========================================
    // Post-Creation Advanced Node Editing Tools
    // ==========================================

    /// Evenly distributes nodes along the selected strand sequence
    static void EvenlySpaceNodes(ReconstructedPropModel& model, int strandIndex, float desiredSpacingInches);

    /// Straightens a sequence of nodes between start and end indices
    static void StraightenSegment(ReconstructedPropModel& model, int startNodeIdx, int endNodeIdx);

    /// Snaps all nodes to the nearest physical grid increment
    static void SnapNodesToGrid(ReconstructedPropModel& model, float gridIncrementInches);

    /// Symmetrizes / mirrors nodes across the X or Y axis or radial N-fold symmetry
    static void SymmetrizeRadial(ReconstructedPropModel& model, int symmetryFoldCount = 6);

    /// Reverses the wiring sequence order of nodes within a strand
    static void ReverseWiringOrder(ReconstructedPropModel& model, int strandIndex = -1);

    /// Auto-clusters submodels into functional groups (e.g. Outer Ring, Star Arms, Center)
    static void AutoClusterSubmodels(ReconstructedPropModel& model);

    /// Duplicates a model instance with a unique name
    static ReconstructedPropModel DuplicateModel(const ReconstructedPropModel& source, const std::string& newName = "");
};

} // namespace xLights
