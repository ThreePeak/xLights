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

    std::cout << "[Unit Test] CustomPropDesignerAI ALL TESTS PASSED!" << std::endl;
    return 0;
}
