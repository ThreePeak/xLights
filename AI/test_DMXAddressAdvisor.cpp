#include <catch2/catch_test_macros.hpp>
#include "AI/DMXAddressAdvisor.h"
#include <string>
#include <vector>

TEST_CASE("DMXAddressAdvisor tests", "[ai][dmx]") {
    xLights::AI::DMXAddressAdvisor advisor;
    
    SECTION("DetectConflicts returns non-empty vector") {
        auto conflicts = advisor.DetectConflicts("<fixture>");
        REQUIRE(!conflicts.empty());
        
        SECTION("SuggestRemapping contains dmx_remapping") {
            std::string jsonStr = advisor.SuggestRemapping(conflicts);
            REQUIRE(jsonStr.find("dmx_remapping") != std::string::npos);
        }
    }
}
