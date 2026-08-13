#include <catch2/catch_test_macros.hpp>
#include "AI/CustomPropDesignerAI.h"
#include <string>
#include <vector>

TEST_CASE("CustomPropDesignerAI tests", "[ai][prop]") {
    xLights::AI::CustomPropDesignerAI designer;
    
    SECTION("GenerateNodeLayout returns correct nodes") {
        auto nodes = designer.GenerateNodeLayout("Mega Tree", 16);
        REQUIRE(nodes.size() == 16);
        for(const auto& node : nodes) {
            REQUIRE(node.channelIndex > 0);
        }
        
        SECTION("ExportToXLightsModelXML contains custommodel") {
            std::string xmlStr = designer.ExportToXLightsModelXML(nodes, "MyTree");
            REQUIRE(xmlStr.find("custommodel") != std::string::npos);
        }
    }
}
