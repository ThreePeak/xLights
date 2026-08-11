#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <functional>
#include <map>
#include <string>
#include <vector>

struct AIScriptSynthesisResult {
    bool success = false;
    std::string generatedLuaScript;
    std::string explanation;
    std::vector<std::string> targetModels;
    std::string errorMessage;
};

struct LuaExecutionResult {
    bool success = false;
    std::string outputLog;
    int commandsExecuted = 0;
    std::string errorMessage;
};

class LuaManager {
public:
    LuaManager() = default;
    ~LuaManager() = default;

    // Convert natural language prompt to executable xLights Lua script via LLM
    AIScriptSynthesisResult SynthesizeLuaFromPrompt(const std::string& prompt,
                                                     const std::vector<std::string>& availableModels,
                                                     const std::vector<std::string>& availablePresets = {});

    // Execute xLights Lua script against current sequence context
    LuaExecutionResult ExecuteLuaScript(const std::string& luaScript);

    // Get built-in xLights Lua API documentation schema for LLM prompting
    static std::string GetxLightsLuaAPISchema();
};
