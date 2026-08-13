#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// STEP 1: LUA INTERPRETER BINDINGS HEADER (xLights/AI/LuaScriptGenerator.h)

#include "LuaScriptAIGenerator.h"

namespace xLights::AI {

using LuaScriptGeneratorConfig = LuaScriptConfig;
using LuaScriptGeneratorResult = LuaScriptResult;

class LuaScriptGenerator {
public:
    // Translate a natural language prompt into an executable xLights Lua automation script
    [[nodiscard]] static LuaScriptGeneratorResult GenerateLuaScript(const LuaScriptGeneratorConfig& config) {
        return LuaScriptAIGenerator::GenerateLuaScript(config);
    }

    // Validates Lua code against xLights Lua API schema and safety rules
    [[nodiscard]] static bool ValidateLuaSandbox(const std::string& luaCode, std::string& errorOut) {
        return LuaScriptAIGenerator::ValidateLuaSandbox(luaCode, errorOut);
    }
};

} // namespace xLights::AI
