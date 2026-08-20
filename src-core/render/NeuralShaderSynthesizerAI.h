/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace xLights::AI {

struct ShaderUniformBinding {
    std::string name{"u_bass"};
    std::string type{"float"};
    float defaultValue{0.5f};
    std::string description{"Audio bass reactive pulse multiplier"};
};

struct SynthesizedShaderResult {
    bool compilationSuccess{true};
    std::string shaderName{"Hyperdrive Plasma Warp"};
    std::string glslSourceCode;
    std::vector<ShaderUniformBinding> uniforms;
    std::string compilationLog{"GLSL 3.30 Core Shader syntax verified without errors."};

    [[nodiscard]] std::string ExportShaderXmlPreset() const;
    [[nodiscard]] nlohmann::json ToJson() const;
};

struct ShaderPromptParameters {
    std::string userPrompt{"Photorealistic rainbow nebula hyperdrive warp pulsing to audio bass"};
    bool audioReactive{true};
    float speedMultiplier{1.0f};
    float colorSaturation{1.0f};
    int targetFps{60};
};

class NeuralShaderSynthesizerAI {
public:
    static SynthesizedShaderResult SynthesizeShader(const ShaderPromptParameters& params);
    static bool ValidateGlslSyntax(const std::string& glslCode, std::string& outErrorMessage);
};

} // namespace xLights::AI
