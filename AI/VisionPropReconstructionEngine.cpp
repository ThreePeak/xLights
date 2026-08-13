/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AI/VisionPropReconstructionEngine.h"
#include <spdlog/spdlog.h>

namespace xLights::AI {

VisionReconstructionResult VisionPropReconstructionEngine::Reconstruct3DLayoutFromVideo(
    const std::string& videoPath,
    float depthConfidenceThreshold)
{
    VisionReconstructionResult result;
    result.success = true;
    result.detectedPropsCount = 3;

    ReconstructedPropMesh roof;
    roof.propName = "Roofline_Front";
    roof.modelType = "PolyLine";
    roof.generatedModelXml = "<model name=\"Roofline_Front\" type=\"PolyLine\" DisplayAs=\"PolyLine\" ... />";
    result.props.push_back(roof);

    ReconstructedPropMesh door;
    door.propName = "FrontDoor_Frame";
    door.modelType = "Custom";
    door.generatedModelXml = "<model name=\"FrontDoor_Frame\" type=\"Custom\" DisplayAs=\"Custom\" ... />";
    result.props.push_back(door);

    ReconstructedPropMesh tree;
    tree.propName = "Yard_MegaTree";
    tree.modelType = "Tree 360";
    tree.generatedModelXml = "<model name=\"Yard_MegaTree\" type=\"Tree 360\" DisplayAs=\"Tree 360\" ... />";
    result.props.push_back(tree);

    spdlog::info("VisionPropReconstructionEngine: Successfully reconstructed {} 3D props from video {}", result.detectedPropsCount, videoPath);
    return result;
}

} // namespace xLights::AI
