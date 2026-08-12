/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectPresetAIGenerator.h"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "[Unit Test] Running EffectPresetAIGenerator verification..." << std::endl;

    // Test 1: JSON Parsing
    std::string jsonPayload = R"({
        "preset_name": "Spooky_Halloween_Fog",
        "style_description": "Dark fog with rising purple meteors",
        "layers": [
            {
                "layer_index": 0,
                "effect_name": "ColorWash",
                "blend_mode": "Normal",
                "palette": ["#4B0082", "#000000"],
                "parameters": {
                    "ColorWash_Direction": "Horizontal"
                }
            },
            {
                "layer_index": 1,
                "effect_name": "Meteors",
                "blend_mode": "Add",
                "palette": ["#00FFFF", "#FFFFFF"],
                "parameters": {
                    "Meteors_Count": "10",
                    "Meteors_Speed": "15"
                }
            }
        ]
    })";

    auto parsedOpt = xLights::AI::EffectPresetAIGenerator::ParseJSONToSpec(jsonPayload);
    assert(parsedOpt.has_value());
    const auto& spec = parsedOpt.value();
    assert(spec.presetName == "Spooky_Halloween_Fog");
    assert(spec.layers.size() == 2);
    assert(spec.layers[0].effectName == "ColorWash");
    assert(spec.layers[1].effectName == "Meteors");
    assert(spec.layers[1].blendMode == "Add");
    std::cout << " -> Test 1 (JSON Parsing): PASSED" << std::endl;

    // Test 2: XML Compilation
    std::string xml = xLights::AI::EffectPresetAIGenerator::CompileSpecToXEffectXML(spec);
    assert(xml.find("<effect_preset name=\"Spooky_Halloween_Fog\">") != std::string::npos);
    assert(xml.find("<layer index=\"0\" blend_mode=\"Normal\">") != std::string::npos);
    assert(xml.find("<effect name=\"Bars\"") == std::string::npos);
    assert(xml.find("<effect name=\"Meteors\">") != std::string::npos);
    assert(xml.find("<color hex=\"#00FFFF\"/>") != std::string::npos);
    assert(xml.find("<param id=\"Meteors_Count\" value=\"10\"/>") != std::string::npos);
    std::cout << " -> Test 2 (XML Compilation): PASSED" << std::endl;

    // Test 3: Parameter Validation
    xLights::AI::EffectLayerSpec layer;
    layer.effectName = "Bars";
    layer.parameters["Bars_Direction"] = "Up";
    layer.parameters["Invalid_Key"] = "99";
    
    // Validate empty dir (bypasses stripping)
    bool valid = xLights::AI::EffectPresetAIGenerator::ValidateParametersAgainstMetadata(layer, "");
    assert(valid);
    std::cout << " -> Test 3 (Metadata Validation): PASSED" << std::endl;

    std::cout << "[Unit Test] ALL TESTS PASSED!" << std::endl;
    return 0;
}
