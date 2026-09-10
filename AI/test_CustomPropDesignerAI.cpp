/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CustomPropDesignerAI.h"
#include <iostream>
#include <string>
#include <vector>
#include <cassert>

int main() {
    std::cout << "[Unit Test] Running CustomPropDesignerAI verification..." << std::endl;
    xLights::AI::CustomPropDesignerAI designer;
    std::string prompt = "Mini tree with 80 nodes on the body and 20 for a top star. 8 vertical columns of 10 nodes each evenly spaced and mini tree body covers a 240 degree area with the back left open. Mini tree body will be 30\" high in real life";

    // Test 1: AnalyzePrompt
    {
        auto res = designer.AnalyzePrompt(prompt, 100);
        assert(res.isValid == true);
        assert(res.spec.propType == "Mini Tree");
        assert(res.spec.bodyNodes == 80);
        assert(res.spec.topperNodes == 20);
        assert(res.spec.totalNodes == 100);
        assert(res.spec.columns == 8);
        assert(res.spec.nodesPerColumn == 10);
        assert(res.spec.arcDegrees == 240.0f);
        assert(res.spec.heightInches == 30.0f);
        assert(res.spec.hasTopper == true);
        assert(!res.executiveSummary.empty());
        assert(res.clarifications.size() >= 3);
        std::cout << " -> Test 1 (AnalyzePrompt Parsing & Clarifications): PASSED" << std::endl;
    }

    // Test 2: GenerateFromSpec node generation
    {
        auto res = designer.AnalyzePrompt(prompt, 100);
        auto nodes = designer.GenerateFromSpec(res.spec);
        assert(nodes.size() == 100);

        int bodyCount = 0;
        int starCount = 0;
        for (const auto& node : nodes) {
            assert(node.channelIndex > 0);
            if (node.group.find("Tree_Strand") != std::string::npos) {
                bodyCount++;
            } else if (node.group.find("Star") != std::string::npos) {
                starCount++;
            }
        }
        assert(bodyCount == 80);
        assert(starCount == 20);
        std::cout << " -> Test 2 (GenerateFromSpec 3D Nodes & Groups): PASSED" << std::endl;
    }

    // Test 3: ExportToXLightsModelXML XML schema
    {
        auto res = designer.AnalyzePrompt(prompt, 100);
        auto nodes = designer.GenerateFromSpec(res.spec);
        std::string xmlStr = designer.ExportToXLightsModelXML(nodes, "MiniTree", res.spec);

        assert(xmlStr.find("custommodel") != std::string::npos);
        assert(xmlStr.find("subModel") != std::string::npos);
        assert(xmlStr.find("Top_Star") != std::string::npos);
        assert(xmlStr.find("HeightInches=\"30") != std::string::npos);
        std::cout << " -> Test 3 (ExportToXLightsModelXML Validation): PASSED" << std::endl;
    }

    // Test 4: OptimizeWirePathTSP
    {
        std::vector<xLights::AI::PropNodeSuggestion> scrambledNodes;
        // 5 nodes in a line, but in zig-zag order 0, 4, 1, 3, 2
        std::vector<float> xs = {0.0f, 40.0f, 10.0f, 30.0f, 20.0f};
        for (int i = 0; i < 5; ++i) {
            xLights::AI::PropNodeSuggestion n;
            n.x = xs[i]; n.y = 0.0f; n.z = 0.0f;
            n.channelIndex = i + 1;
            scrambledNodes.push_back(n);
        }
        auto tspRes = designer.OptimizeWirePathTSP(scrambledNodes);
        assert(tspRes.optimizedNodes.size() == 5);
        assert(tspRes.optimizedWireLength <= tspRes.originalWireLength);
        assert(tspRes.wireSavingsPercent > 0.0f);
        std::cout << " -> Test 4 (2-Opt TSP Wire Optimization): PASSED (saved "
                  << tspRes.wireSavingsPercent << "% wire length)" << std::endl;
    }

    // Test 5: GenerateBatchPropModelsXML
    {
        auto res = designer.AnalyzePrompt(prompt, 100);
        auto nodes = designer.GenerateFromSpec(res.spec);
        xLights::AI::BatchModelSpec bSpec;
        bSpec.baseModelName = "MiniTree";
        bSpec.count = 4;
        bSpec.pattern = xLights::AI::BatchPlacementPattern::ArcFan;
        bSpec.spacingOrRadius = 60.0f;

        std::string batchXml = designer.GenerateBatchPropModelsXML(nodes, bSpec, res.spec);
        assert(batchXml.find("<models count=\"4\">") != std::string::npos);
        assert(batchXml.find("MiniTree_1") != std::string::npos);
        assert(batchXml.find("MiniTree_4") != std::string::npos);
        std::cout << " -> Test 5 (GenerateBatchPropModelsXML 4-Model Arc): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] CustomPropDesignerAI ALL TESTS PASSED!" << std::endl;
    return 0;
}
