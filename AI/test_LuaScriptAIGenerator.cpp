/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #12 Catch2 Unit Tests for LuaScriptAIGenerator

#include <catch2/catch_test_macros.hpp>
#include "LuaScriptAIGenerator.h"

using namespace xLights::AI;

TEST_CASE("LuaScriptAIGenerator: Natural Language Macro / Lua Script Generator", "[LuaScriptAIGenerator]") {

    SECTION("Generates valid Lua code for rainbow spiral prompt") {
        LuaScriptConfig config;
        config.userPrompt = "Create a spiral rainbow effect on MegaTree";
        config.targetModelName = "MegaTree";
        config.durationMs = 5000;
        config.currentPalette = "#FF0000,#00FF00,#0000FF";
        config.sandboxValidation = true;

        LuaScriptResult result = LuaScriptAIGenerator::GenerateLuaScript(config);
        REQUIRE(result.success == true);
        REQUIRE(result.passesSandboxValidation == true);
        REQUIRE(!result.generatedLuaCode.empty());
        REQUIRE(result.generatedLuaCode.find("xlights.create_effect") != std::string::npos);
        REQUIRE(result.generatedLuaCode.find("MegaTree") != std::string::npos);
    }

    SECTION("Rejects sandbox violations e.g. os.execute") {
        std::string dangerousLua = "os.execute('rm -rf /')\nxlights.create_effect('Bars')";
        std::string err;
        bool safe = LuaScriptAIGenerator::ValidateLuaSandbox(dangerousLua, err);
        REQUIRE(safe == false);
        REQUIRE(!err.empty());
    }

    SECTION("Handles empty userPrompt gracefully") {
        LuaScriptConfig config;
        config.userPrompt = "";

        LuaScriptResult result = LuaScriptAIGenerator::GenerateLuaScript(config);
        REQUIRE(result.success == false);
        REQUIRE(!result.errorMessage.empty());
    }
}
