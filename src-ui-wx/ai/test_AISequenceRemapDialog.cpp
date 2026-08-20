/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AI/SequenceRemappingAgent.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running AISequenceRemapDialog backend verification..." << std::endl;

    ModelSpatialDescriptor src;
    src.name = "SourceTree";
    src.category = ModelCategory::TREE_360;
    src.totalNodes = 800;

    ModelSpatialDescriptor tgt;
    tgt.name = "TargetTree";
    tgt.category = ModelCategory::TREE_360;
    tgt.totalNodes = 1600;

    auto plan = SequenceRemappingAgent::GenerateRemapPlan({src}, {tgt}, 0.5f);
    assert(plan.success);
    assert(plan.mappings.size() == 1);
    assert(plan.mappings[0].sourceModel == "SourceTree");
    assert(plan.mappings[0].targetModel == "TargetTree");
    std::cout << " -> AISequenceRemapDialog remap plan generator: PASSED" << std::endl;
    return 0;
}
