/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIPhoto3DPropReconstructorDialog.h"
#include <iostream>
#include <cassert>

using namespace xLights;

int main() {
    std::cout << "[Unit Test] Running AIPhoto3DPropReconstructorDialog verification..." << std::endl;

    // Headless test of dialog data backend
    ReconstructorParameters params;
    auto model = Photo3DPropReconstructorAI::ReconstructFromSingleImage("sample_star.png", params);
    assert(!model.nodes.empty());

    // Test model mutations
    Photo3DPropReconstructorAI::SnapNodesToGrid(model, 0.5f);
    Photo3DPropReconstructorAI::EvenlySpaceNodes(model, 1, 2.5f);
    Photo3DPropReconstructorAI::AutoClusterSubmodels(model);

    assert(!model.submodels.empty());
    assert(!model.ExportXModelXml().empty());
    assert(!model.ExportObjMesh().empty());

    std::cout << " -> Dialog Model Backend & Editing Pipeline: PASSED" << std::endl;
    std::cout << "[Unit Test] AIPhoto3DPropReconstructorDialog ALL TESTS PASSED!" << std::endl;
    return 0;
}
