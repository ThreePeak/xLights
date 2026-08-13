/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// STEP 2: LUA INTERPRETER BINDINGS (xLights/AI/LuaScriptGenerator.cpp)

#include "LuaScriptGenerator.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cctype>
#include <sstream>

namespace xLights::AI {

static std::string ToLower(std::string_view str) {
    std::string lower(str);
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower;
}

bool LuaScriptGenerator::ValidateLuaSandbox(const std::string& luaCode, std::string& errorOut) {
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

bool LuaScriptGenerator::ValidateLuaSyntax(const std::string& luaCode) {
    std::string err;
    return ValidateLuaSandbox(luaCode, err);
}

LuaScriptGeneratorResult LuaScriptGenerator::GenerateLuaScript(const LuaScriptGeneratorConfig& config) {
    LuaScriptGeneratorResult result;

    std::string activePrompt = !config.userPrompt.empty() ? config.userPrompt : config.promptDescription;

    if (activePrompt.empty()) {
        result.success = false;
        result.errorMessage = "userPrompt or promptDescription cannot be empty.";
        spdlog::error("LuaScriptGenerator: {}", result.errorMessage);
        return result;
    }

    std::string lowerPrompt = ToLower(activePrompt);
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
            spdlog::error("LuaScriptGenerator: {}", result.errorMessage);
            return result;
        }
    } else {
        result.passesSandboxValidation = true;
    }

    result.success = true;
    spdlog::info("LuaScriptGenerator: {}", result.scriptDescription);
    return result;
}

LuaScriptSpec LuaScriptGenerator::GenerateScriptFromPrompt(const std::string& userPrompt) {
    LuaScriptGeneratorConfig config;
    config.userPrompt = userPrompt;
    return GenerateLuaScript(config);
}

std::string LuaScriptGenerator::ConstructStructuredLLMPrompt(const LuaScriptGeneratorConfig& config) {
    std::ostringstream prompt;
    prompt << "SYSTEM INSTRUCTIONS: You are an expert xLights Lua Automation Script Generator.\n"
           << "Output ONLY valid, safe, executable Lua code targeting xLights exposed automation functions.\n"
           << "Exposed xLights Lua Functions:\n"
           << "  - xlights.get_model(name_string)\n"
           << "  - xlights.get_active_model()\n"
           << "  - xlights.create_effect(type_string)\n"
           << "  - effect:set_duration_ms(ms_integer)\n"
           << "  - effect:set_palette(palette_string)\n"
           << "  - effect:set_parameter(name_string, value)\n"
           << "  - model:apply_effect(effect_object)\n"
           << "  - xlights.render_sequence()\n"
           << "  - xlights.log_info(message_string)\n"
           << "  - xlights.log_warning(message_string)\n"
           << "SAFETY RULES: Do NOT use os.execute, io.open, require, or external system commands.\n\n"
           << "USER REQUEST: \"" << config.userPrompt << "\"\n"
           << "TARGET MODEL: \"" << (config.targetModelName.empty() ? "SelectedModel" : config.targetModelName) << "\"\n"
           << "DURATION: " << config.durationMs << "ms\n"
           << "PALETTE: \"" << (config.currentPalette.empty() ? "Default Rainbow" : config.currentPalette) << "\"\n";
    return prompt.str();
}

std::string LuaScriptGenerator::ExportLuaScriptJSON(const LuaScriptGeneratorResult& result) {
    nlohmann::json root;
    root["success"] = result.success;
    root["errorMessage"] = result.errorMessage;
    root["generatedLuaCode"] = result.generatedLuaCode;
    root["scriptDescription"] = result.scriptDescription;
    root["passesSandboxValidation"] = result.passesSandboxValidation;
    return root.dump(2);
}

} // namespace xLights::AI
