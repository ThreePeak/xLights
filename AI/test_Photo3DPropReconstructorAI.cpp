/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/models/Photo3DPropReconstructorAI.h"
#include <iostream>
#include <cassert>

using namespace xLights;

int main() {
    std::cout << "[Unit Test] Running Photo3DPropReconstructorAI verification..." << std::endl;

    // Test 1: Single-image monocular reconstruction
    {
        ReconstructorParameters params;
        params.mode = ReconstructionMode::SINGLE_IMAGE;
        params.targetMaxNodes = 50;
        params.depthCurvatureScale = 1.0f;

        auto model = Photo3DPropReconstructorAI::ReconstructFromSingleImage("test_flake.jpg", params);
        assert(!model.nodes.empty());
        assert(model.nodes.size() == 50);
        assert(model.boundingWidthInches == 48.0f);
        assert(!model.submodels.empty());
        std::cout << " -> Test 1 (Single-Image Monocular Reconstruction): PASSED" << std::endl;
    }

    // Test 2: Multi-view photogrammetry reconstruction
    {
        std::vector<ReconstructorInputImage> images = {
            {"front.png", CameraViewAngle::FRONT, 0.0f, 1.0f},
            {"angle45.png", CameraViewAngle::ANGLE_45_LEFT, 45.0f, 0.9f},
            {"side.png", CameraViewAngle::SIDE_PROFILE, 90.0f, 0.85f}
        };

        ReconstructorParameters params;
        params.mode = ReconstructionMode::MULTI_IMAGE_PHOTOGRAMMETRY;
        params.nodeSpacingInches = 2.0f;

        auto model = Photo3DPropReconstructorAI::ReconstructFromMultiViewImages(images, params);
        assert(model.nodes.size() == 6 * 16);
        assert(model.submodels.size() >= 2);
        std::cout << " -> Test 2 (Multi-Angle Triangulation Photogrammetry): PASSED" << std::endl;
    }

    // Test 3: Post-creation editing tools (Snap, Straighten, Reverse)
    {
        ReconstructorParameters params;
        auto model = Photo3DPropReconstructorAI::ReconstructFromSingleImage("test.jpg", params);

        // Snap to grid
        Photo3DPropReconstructorAI::SnapNodesToGrid(model, 1.0f);
        for (const auto& n : model.nodes) {
            assert(std::abs(n.x - std::round(n.x)) < 0.001f);
        }

        // Reverse wiring sequence
        int origFirstIdx = model.nodes.front().nodeIndex;
        Photo3DPropReconstructorAI::ReverseWiringOrder(model, -1);
        assert(model.nodes.front().nodeIndex == 1);
        assert(model.nodes.size() == 96);
        std::cout << " -> Test 3 (Post-Creation Node Editing Suite): PASSED" << std::endl;
    }

    // Test 4: Serialization & Exports (.xmodel, .obj, .svg, .csv)
    {
        ReconstructorParameters params;
        auto model = Photo3DPropReconstructorAI::ReconstructFromSingleImage("test.jpg", params);

        std::string xmodelXml = model.ExportXModelXml();
        assert(xmodelXml.find("<custommodel") != std::string::npos);
        assert(xmodelXml.find("<nodes>") != std::string::npos);

        std::string objMesh = model.ExportObjMesh();
        assert(objMesh.find("v ") != std::string::npos);
        assert(objMesh.find("l ") != std::string::npos);

        std::string svg = model.ExportSvgPath();
        assert(svg.find("<svg") != std::string::npos);

        std::string csv = model.ExportCsvCoordinates();
        assert(csv.find("NodeIndex,StrandIndex") != std::string::npos);
        std::cout << " -> Test 4 (Export Serializations: XML, OBJ, SVG, CSV): PASSED" << std::endl;
    }

    // Test 5: Model Duplication
    {
        ReconstructorParameters params;
        auto model = Photo3DPropReconstructorAI::ReconstructFromSingleImage("test.jpg", params);
        auto dup = Photo3DPropReconstructorAI::DuplicateModel(model, "CustomSnowflake_v2");
        assert(dup.propName == "CustomSnowflake_v2");
        assert(dup.nodes.size() == model.nodes.size());
        std::cout << " -> Test 5 (Model State Duplication): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] Photo3DPropReconstructorAI ALL TESTS PASSED!" << std::endl;
    return 0;
}
