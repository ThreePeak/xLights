/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/render/NeuralShaderSynthesizerAI.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running NeuralShaderSynthesizerAI verification..." << std::endl;

    // Test 1: Shader Prompt Compilation & Audio Uniforms
    {
        ShaderPromptParameters params;
        params.userPrompt = "Swirling rainbow nebula with bass pulse";
        params.audioReactive = true;

        auto result = NeuralShaderSynthesizerAI::SynthesizeShader(params);
        assert(result.compilationSuccess);
        assert(result.glslSourceCode.find("u_bass") != std::string::npos);
        assert(result.glslSourceCode.find("mainImage") != std::string::npos);
        std::cout << " -> Test 1 (Shader Prompt Compilation & Audio Uniforms): PASSED" << std::endl;
    }

    // Test 2: GLSL Syntax Validator
    {
        std::string err;
        bool ok = NeuralShaderSynthesizerAI::ValidateGlslSyntax("void mainImage() { fragColor = vec4(1.0); }", err);
        assert(ok);

        bool fail = NeuralShaderSynthesizerAI::ValidateGlslSyntax("invalid code without entry", err);
        assert(!fail);
        std::cout << " -> Test 2 (GLSL Syntax Validator): PASSED" << std::endl;
    }

    // Test 3: Shader XML Preset Export & JSON
    {
        ShaderPromptParameters params;
        auto result = NeuralShaderSynthesizerAI::SynthesizeShader(params);

        std::string xml = result.ExportShaderXmlPreset();
        assert(xml.find("<xlights_shader_preset") != std::string::npos);
        assert(xml.find("<uniform name=") != std::string::npos);

        auto j = result.ToJson();
        assert(j.contains("glslSourceCode"));
        assert(j.contains("uniforms"));
        std::cout << " -> Test 3 (Shader XML Preset Export & JSON): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] NeuralShaderSynthesizerAI ALL TESTS PASSED!" << std::endl;
    return 0;
}
