#include <catch2/catch_test_macros.hpp>
#include "AI/FPPControllerSyncAdvisor.h"
#include <string>
#include <vector>

TEST_CASE("FPPControllerSyncAdvisor tests", "[ai][fpp]") {
    xLights::AI::FPPControllerSyncAdvisor advisor;
    
    SECTION("AnalyzeControllerLayout returns non-empty vector") {
        auto suggestions = advisor.AnalyzeControllerLayout("<controller>");
        REQUIRE(!suggestions.empty());
        
        SECTION("GenerateFPPJsonManifest contains fpp_version") {
            std::string jsonStr = advisor.GenerateFPPJsonManifest(suggestions);
            REQUIRE(jsonStr.find("fpp_version") != std::string::npos);
        }
    }
}
