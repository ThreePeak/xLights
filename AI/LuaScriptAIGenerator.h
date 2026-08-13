#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #12: Natural Language Macro / Lua Script Generator

#include "AISubsystemBase.h"
#include <string>
#include <vector>
#include <future>

namespace xLights::AI {

struct LuaScriptConfig {
    std::string userPrompt;            // e.g. "Create a spiral rainbow effect on MegaTree lasting 5 seconds"
    std::string targetModelName;       // e.g. "MegaTree"
    int durationMs = 5000;             // Duration in milliseconds
    std::string currentPalette;        // e.g. "#FF0000,#00FF00,#0000FF"
    bool sandboxValidation = true;     // Syntax & safety check before returning
};

struct LuaScriptResult {
    bool success = false;
    std::string errorMessage;
    std::string generatedLuaCode;      // Clean, executable xLights Lua script
    std::string scriptDescription;     // Summary of what the generated Lua code does
    bool passesSandboxValidation = false;
};

class LuaScriptAIGenerator : public AISubsystemBase {
public:
    LuaScriptAIGenerator(ServiceManager* sm = nullptr) : AISubsystemBase(sm) {}
    virtual ~LuaScriptAIGenerator() override = default;

    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override {
        return std::async(std::launch::async, [this, callback]() {
            if (callback) callback("Initializing LuaScriptAIGenerator...", 0.0f);
            m_isInitialized.store(true);
            if (callback) callback("LuaScriptAIGenerator initialized.", 100.0f);
            return true;
        });
    }

    virtual void Shutdown() override {
        m_isInitialized.store(false);
    }

    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "LuaScriptAIGenerator"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"natural_language_lua_generation", "xlights_lua_api_binding", "sandbox_script_validation"};
    }

    // Converts natural language prompts into executable sandboxed xLights Lua scripts
    [[nodiscard]] static LuaScriptResult GenerateLuaScript(const LuaScriptConfig& config);

    // Validates Lua code against xLights Lua API schema and safety rules
    [[nodiscard]] static bool ValidateLuaSandbox(const std::string& luaCode, std::string& errorOut);
};

} // namespace xLights::AI
