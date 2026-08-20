/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceRemappingAgent.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

int main() {
    std::cout << "[Unit Test] Running SequenceRemappingAgent verification..." << std::endl;

    // Test 1: Category parsing
    auto cat1 = xLights::AI::SequenceRemappingAgent::ParseCategory("MegaTree 16x50");
    assert(cat1 == xLights::AI::ModelCategory::TREE_360);
    auto cat2 = xLights::AI::SequenceRemappingAgent::ParseCategory("P5 Matrix Display");
    assert(cat2 == xLights::AI::ModelCategory::MATRIX);
    auto cat3 = xLights::AI::SequenceRemappingAgent::ParseCategory("Singing Tree Face");
    assert(cat3 == xLights::AI::ModelCategory::SINGING_FACE);
    std::cout << " -> Test 1 (Category Parsing & Normalization): PASSED" << std::endl;

    // Test 2: Embedding vector generation & L2 unit normalization
    xLights::AI::ModelSpatialDescriptor descA;
    descA.name = "MegaTree_16x50";
    descA.category = xLights::AI::ModelCategory::TREE_360;
    descA.width = 3.0f;
    descA.height = 6.0f;
    descA.depth = 3.0f;
    descA.totalNodes = 800;
    descA.strandCount = 16;
    descA.is3D = true;
    descA.submodelNames = {"Star", "Tree Outline"};

    auto embA = xLights::AI::SequenceRemappingAgent::ComputeModelEmbedding(descA);
    float normSq = 0.0f;
    for (float v : embA) normSq += v * v;
    assert(std::abs(std::sqrt(normSq) - 1.0f) < 1e-4f);
    std::cout << " -> Test 2 (16-D Unit Normalized Embedding): PASSED" << std::endl;

    // Test 3: Cosine Similarity matching
    xLights::AI::ModelSpatialDescriptor descB;
    descB.name = "MegaTree_32x100";
    descB.category = xLights::AI::ModelCategory::TREE_360;
    descB.width = 4.0f;
    descB.height = 8.0f;
    descB.depth = 4.0f;
    descB.totalNodes = 3200;
    descB.strandCount = 32;
    descB.is3D = true;
    descB.submodelNames = {"Star"};

    auto embB = xLights::AI::SequenceRemappingAgent::ComputeModelEmbedding(descB);
    float simTree = xLights::AI::SequenceRemappingAgent::ComputeCosineSimilarity(embA, embB);
    assert(simTree > 0.80f);

    xLights::AI::ModelSpatialDescriptor descArch;
    descArch.name = "Arch_1";
    descArch.category = xLights::AI::ModelCategory::ARCH;
    descArch.width = 2.5f;
    descArch.height = 1.2f;
    descArch.depth = 0.1f;
    descArch.totalNodes = 50;
    descArch.strandCount = 1;
    descArch.is3D = false;

    auto embArch = xLights::AI::SequenceRemappingAgent::ComputeModelEmbedding(descArch);
    float simArch = xLights::AI::SequenceRemappingAgent::ComputeCosineSimilarity(embA, embArch);
    assert(simTree > simArch);
    std::cout << " -> Test 3 (Cosine Similarity Distinction): PASSED (Tree-Tree: " << simTree << " vs Tree-Arch: " << simArch << ")" << std::endl;

    // Test 4: Display layout remapping plan generation
    std::vector<xLights::AI::ModelSpatialDescriptor> sourceList = {descA, descArch};
    std::vector<xLights::AI::ModelSpatialDescriptor> targetList = {descB, descArch};

    auto plan = xLights::AI::SequenceRemappingAgent::GenerateRemapPlan(sourceList, targetList);
    assert(plan.success);
    assert(plan.mappings.size() == 2);
    assert(plan.mappings[0].sourceModel == "MegaTree_16x50");
    assert(plan.mappings[0].targetModel == "MegaTree_32x100");
    std::cout << " -> Test 4 (Layout Remap Plan Generation): PASSED" << std::endl;

    // Test 5: Sequence XML substitution
    std::string sampleXml = "<xsequence><Element type=\"model\" name=\"MegaTree_16x50\"><Effect Target=\"MegaTree_16x50\" name=\"Bars\" /></Element></xsequence>";
    std::string remappedXml = xLights::AI::SequenceRemappingAgent::ApplyRemappingToSequenceXML(sampleXml, plan);
    assert(remappedXml.find("MegaTree_32x100") != std::string::npos);
    assert(remappedXml.find("name=\"MegaTree_16x50\"") == std::string::npos);
    std::cout << " -> Test 5 (XML Sequence Channel Substitution): PASSED" << std::endl;

    std::cout << "[Unit Test] SequenceRemappingAgent ALL TESTS PASSED!" << std::endl;
    return 0;
}
