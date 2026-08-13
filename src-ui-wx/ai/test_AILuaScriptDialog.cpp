/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <catch2/catch_test_macros.hpp>
#include "AI/LuaScriptGenerator.h"

TEST_CASE("AI Lua Script Assistant Backend Validation Tests", "[AI][Lua]") {
    SECTION("Generate Lua Script for natural language prompt") {
        std::string prompt = "Generate a rainbow wave across MegaTree at 120 BPM";
        std::string script = xLights::AI::LuaScriptGenerator::GenerateLuaScript(prompt);

        REQUIRE(!script.empty());
        REQUIRE(script.find("RenderEffect") != std::string::npos);
    }

    SECTION("Validate Lua Sandbox Safety") {
        std::string safeScript = "function RenderEffect(buffer, time) buffer:ApplyColorWave() end";
        std::string errOut;
        bool isSafe = xLights::AI::LuaScriptGenerator::ValidateLuaSandbox(safeScript, errOut);

        REQUIRE(isSafe == true);
        REQUIRE(errOut.empty());
    }

    SECTION("Reject Unsafe Lua Script in Sandbox") {
        std::string unsafeScript = "os.execute('rm -rf /') io.open('secret.txt', 'w')";
        std::string errOut;
        bool isSafe = xLights::AI::LuaScriptGenerator::ValidateLuaSandbox(unsafeScript, errOut);

        REQUIRE(isSafe == false);
        REQUIRE(!errOut.empty());
    }
}
