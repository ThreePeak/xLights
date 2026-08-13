/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Catch2 Unit Tests for LuaScriptGenerator

#include <catch2/catch_test_macros.hpp>
#include "LuaScriptGenerator.h"
#include <nlohmann/json.hpp>

using namespace xLights::AI;

TEST_CASE("LuaScriptGenerator: Interpreter Bindings & Script Generation", "[LuaScriptGenerator]") {

    SECTION("Generates valid Lua code and exports JSON") {
        LuaScriptGeneratorConfig config;
        config.userPrompt = "Create a fire flame effect on MegaTree";
        config.targetModelName = "MegaTree";
        config.durationMs = 8000;
        config.sandboxValidation = true;

        LuaScriptGeneratorResult result = LuaScriptGenerator::GenerateLuaScript(config);
        REQUIRE(result.success == true);
        REQUIRE(result.passesSandboxValidation == true);
        REQUIRE(!result.generatedLuaCode.empty());
        REQUIRE(result.generatedLuaCode.find("Fire") != std::string::npos);

        std::string jsonStr = LuaScriptGenerator::ExportLuaScriptJSON(result);
        REQUIRE(!jsonStr.empty());

        nlohmann::json parsed = nlohmann::json::parse(jsonStr);
        REQUIRE(parsed["success"] == true);
        REQUIRE(parsed["passesSandboxValidation"] == true);
    }

    SECTION("Constructs structured LLM prompt targeting exposed xLights automation functions") {
        LuaScriptGeneratorConfig config;
        config.userPrompt = "Create a marquee chase effect";
        config.targetModelName = "Arches";
        config.durationMs = 4000;

        std::string prompt = LuaScriptGenerator::ConstructStructuredLLMPrompt(config);
        REQUIRE(!prompt.empty());
        REQUIRE(prompt.find("SYSTEM INSTRUCTIONS") != std::string::npos);
        REQUIRE(prompt.find("xlights.get_model") != std::string::npos);
        REQUIRE(prompt.find("Create a marquee chase effect") != std::string::npos);
    }
}
