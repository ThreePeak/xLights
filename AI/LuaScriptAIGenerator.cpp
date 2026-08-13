/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #12: Natural Language Macro / Lua Script Generator

#include "LuaScriptAIGenerator.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cctype>
#include <sstream>

namespace xLights::AI {

static std::string ToLower(std::string_view str) {
    std::string lower(str);
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower;
}

bool LuaScriptAIGenerator::ValidateLuaSandbox(const std::string& luaCode, std::string& errorOut) {
    if (luaCode.empty()) {
        errorOut = "Lua code is empty.";
        return false;
    }

    // Blacklist forbidden calls for safety sandbox
    static const std::vector<std::string> forbidden = {
        "os.execute", "os.remove", "os.rename", "os.exit",
        "io.open", "io.popen", "io.read", "io.write",
        "require", "package", "loadstring", "dofile", "debug."
    };

    std::string lowerCode = ToLower(luaCode);
    for (const auto& f : forbidden) {
        if (lowerCode.find(f) != std::string::npos) {
            errorOut = "Sandbox Violation: Forbidden function '" + f + "' detected in Lua script.";
            return false;
        }
    }

    return true;
}

LuaScriptResult LuaScriptAIGenerator::GenerateLuaScript(const LuaScriptConfig& config) {
    LuaScriptResult result;

    if (config.userPrompt.empty()) {
        result.success = false;
        result.errorMessage = "userPrompt cannot be empty.";
        spdlog::error("LuaScriptAIGenerator: {}", result.errorMessage);
        return result;
    }

    std::string lowerPrompt = ToLower(config.userPrompt);
    std::string model = config.targetModelName.empty() ? "SelectedModel" : config.targetModelName;

    std::string effectType = "Bars";
    if (lowerPrompt.find("spiral") != std::string::npos || lowerPrompt.find("rainbow") != std::string::npos) {
        effectType = "SingleStrand";
    } else if (lowerPrompt.find("shimmer") != std::string::npos || lowerPrompt.find("sparkle") != std::string::npos) {
        effectType = "Shimmer";
    } else if (lowerPrompt.find("fire") != std::string::npos || lowerPrompt.find("flame") != std::string::npos) {
        effectType = "Fire";
    } else if (lowerPrompt.find("chase") != std::string::npos || lowerPrompt.find("marquee") != std::string::npos) {
        effectType = "Marquee";
    } else if (lowerPrompt.find("curtain") != std::string::npos) {
        effectType = "Curtain";
    }

    std::ostringstream luaStream;
    luaStream << "-- Auto-generated xLights Lua Macro Script\n"
              << "-- Prompt: \"" << config.userPrompt << "\"\n"
              << "-- Target Model: " << model << "\n\n"
              << "local model = xlights.get_model(\"" << model << "\")\n"
              << "if not model then\n"
              << "    xlights.log_warning(\"Model '" << model << "' not found. Falling back to active selection.\")\n"
              << "    model = xlights.get_active_model()\n"
              << "end\n\n"
              << "if model then\n"
              << "    local effect = xlights.create_effect(\"" << effectType << "\")\n"
              << "    effect:set_duration_ms(" << (config.durationMs > 0 ? config.durationMs : 5000) << ")\n";

    if (!config.currentPalette.empty()) {
        luaStream << "    effect:set_palette(\"" << config.currentPalette << "\")\n";
    } else {
        luaStream << "    effect:set_palette(\"Rainbow\")\n";
    }

    luaStream << "    effect:set_parameter(\"Speed\", 50)\n"
              << "    effect:set_parameter(\"Spatial3D\", true)\n"
              << "    model:apply_effect(effect)\n"
              << "    xlights.render_sequence()\n"
              << "    xlights.log_info(\"Successfully applied '" << effectType << "' effect to " << model << ".\")\n"
              << "end\n";

    result.generatedLuaCode = luaStream.str();
    result.scriptDescription = "Generates a " + effectType + " effect script for model '" + model + "' (" + std::to_string(config.durationMs) + "ms duration).";

    if (config.sandboxValidation) {
        std::string err;
        result.passesSandboxValidation = ValidateLuaSandbox(result.generatedLuaCode, err);
        if (!result.passesSandboxValidation) {
            result.success = false;
            result.errorMessage = err;
            spdlog::error("LuaScriptAIGenerator: {}", result.errorMessage);
            return result;
        }
    } else {
        result.passesSandboxValidation = true;
    }

    result.success = true;
    spdlog::info("LuaScriptAIGenerator: {}", result.scriptDescription);
    return result;
}

} // namespace xLights::AI
