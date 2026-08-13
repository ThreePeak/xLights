#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// STEP 1: LUA INTERPRETER BINDINGS HEADER (xLights/AI/LuaScriptGenerator.h)

#include "AISubsystemBase.h"
#include <string>
#include <vector>
#include <future>

namespace xLights::AI {

struct LuaScriptGeneratorConfig {
    std::string userPrompt;            // e.g. "Create a spiral rainbow effect on MegaTree"
    std::string targetModelName;       // e.g. "MegaTree"
    int durationMs = 5000;             // Duration in milliseconds
    std::string currentPalette;        // e.g. "#FF0000,#00FF00,#0000FF"
    bool sandboxValidation = true;     // Syntax & safety check before returning
};

struct LuaScriptGeneratorResult {
    bool success = false;
    std::string errorMessage;
    std::string generatedLuaCode;      // Clean, executable xLights Lua script
    std::string scriptDescription;     // Summary of what the generated Lua code does
    bool passesSandboxValidation = false;
};

class LuaScriptGenerator : public AISubsystemBase {
public:
    LuaScriptGenerator(ServiceManager* sm = nullptr) : AISubsystemBase(sm) {}
    virtual ~LuaScriptGenerator() override = default;

    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override {
        return std::async(std::launch::async, [this, callback]() {
            if (callback) callback("Initializing LuaScriptGenerator...", 0.0f);
            m_isInitialized.store(true);
            if (callback) callback("LuaScriptGenerator initialized.", 100.0f);
            return true;
        });
    }

    virtual void Shutdown() override {
        m_isInitialized.store(false);
    }

    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "LuaScriptGenerator"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"natural_language_lua_generation", "xlights_lua_api_binding", "sandbox_script_validation"};
    }

    // Translate a natural language prompt into an executable xLights Lua automation script
    [[nodiscard]] static LuaScriptGeneratorResult GenerateLuaScript(const LuaScriptGeneratorConfig& config);

    // Validates Lua code against xLights Lua API schema and safety rules
    [[nodiscard]] static bool ValidateLuaSandbox(const std::string& luaCode, std::string& errorOut);

    // Exports script generation result to structured JSON for MCP tool consumption
    [[nodiscard]] static std::string ExportLuaScriptJSON(const LuaScriptGeneratorResult& result);
};

} // namespace xLights::AI
