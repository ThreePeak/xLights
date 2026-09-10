/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectPresetAIGenerator.h"
#include "spdlog/spdlog.h"
#include <pugixml.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace xLights::AI {

nlohmann::json EffectPresetSpec::ToJson() const {
    nlohmann::json j;
    j["preset_name"] = presetName;
    j["style_description"] = styleDescription;
    j["layers"] = nlohmann::json::array();
    for (const auto& layer : layers) {
        nlohmann::json lj;
        lj["layer_index"] = layer.layerIndex;
        lj["effect_name"] = layer.effectName;
        lj["blend_mode"] = layer.blendMode;
        lj["palette"] = layer.hexPalette;
        lj["parameters"] = layer.parameters;
        j["layers"].push_back(lj);
    }
    return j;
}

EffectPresetAIGenerator::EffectPresetAIGenerator(ServiceManager* sm)
    : AISubsystemBase(sm) {}

std::future<bool> EffectPresetAIGenerator::InitializeAsync(StatusCallback callback) {
    return std::async(std::launch::async, [this, callback]() {
        if (callback) callback("Initializing EffectPresetAIGenerator...", 0.0f);
        m_isInitialized.store(true);
        if (callback) callback("EffectPresetAIGenerator initialized.", 100.0f);
        return true;
    });
}

void EffectPresetAIGenerator::Shutdown() {
    m_isInitialized.store(false);
}

std::string EffectPresetAIGenerator::CompileSpecToXEffectXML(const EffectPresetSpec& spec) {
    pugi::xml_document doc;

    // XML Declaration
    pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
    decl.append_attribute("version").set_value("1.0");
    decl.append_attribute("encoding").set_value("UTF-8");

    // Root Element: <effect_preset name="PRESET_NAME">
    pugi::xml_node rootNode = doc.append_child("effect_preset");
    rootNode.append_attribute("name").set_value(spec.presetName.c_str());

    // Description attribute if present
    if (!spec.styleDescription.empty()) {
        rootNode.append_attribute("description").set_value(spec.styleDescription.c_str());
    }

    // <layers> Container
    pugi::xml_node layersNode = rootNode.append_child("layers");

    for (const auto& layerSpec : spec.layers) {
        // <layer index="0" blend_mode="Normal">
        pugi::xml_node layerNode = layersNode.append_child("layer");
        layerNode.append_attribute("index").set_value(layerSpec.layerIndex);
        layerNode.append_attribute("blend_mode").set_value(layerSpec.blendMode.c_str());

        // <effect name="Bars">
        pugi::xml_node effectNode = layerNode.append_child("effect");
        effectNode.append_attribute("name").set_value(layerSpec.effectName.c_str());

        // <palette>
        if (!layerSpec.hexPalette.empty()) {
            pugi::xml_node paletteNode = effectNode.append_child("palette");
            for (const auto& hexColor : layerSpec.hexPalette) {
                pugi::xml_node colorNode = paletteNode.append_child("color");
                colorNode.append_attribute("hex").set_value(hexColor.c_str());
            }
        }

        // <parameters>
        if (!layerSpec.parameters.empty()) {
            pugi::xml_node paramsNode = effectNode.append_child("parameters");
            for (const auto& [paramId, paramVal] : layerSpec.parameters) {
                pugi::xml_node paramNode = paramsNode.append_child("param");
                paramNode.append_attribute("id").set_value(paramId.c_str());
                paramNode.append_attribute("value").set_value(paramVal.c_str());
            }
        }
    }

    std::ostringstream ss;
    doc.save(ss, "  ");
    return ss.str();
}

std::optional<EffectPresetSpec> EffectPresetAIGenerator::ParseJSONToSpec(const std::string& jsonPayload) {
    try {
        nlohmann::json j = nlohmann::json::parse(jsonPayload);
        EffectPresetSpec spec;

        if (j.contains("preset_name") && j["preset_name"].is_string()) {
            spec.presetName = j["preset_name"].get<std::string>();
        } else {
            spec.presetName = "AI_Generated_Preset";
        }

        if (j.contains("style_description") && j["style_description"].is_string()) {
            spec.styleDescription = j["style_description"].get<std::string>();
        }

        if (j.contains("layers") && j["layers"].is_array()) {
            for (const auto& lj : j["layers"]) {
                EffectLayerSpec layer;
                if (lj.contains("layer_index") && lj["layer_index"].is_number_integer()) {
                    layer.layerIndex = lj["layer_index"].get<int>();
                }
                if (lj.contains("effect_name") && lj["effect_name"].is_string()) {
                    layer.effectName = lj["effect_name"].get<std::string>();
                }
                if (lj.contains("blend_mode") && lj["blend_mode"].is_string()) {
                    layer.blendMode = lj["blend_mode"].get<std::string>();
                }
                if (lj.contains("palette") && lj["palette"].is_array()) {
                    for (const auto& hexItem : lj["palette"]) {
                        if (hexItem.is_string()) {
                            layer.hexPalette.push_back(hexItem.get<std::string>());
                        }
                    }
                }
                if (lj.contains("parameters") && lj["parameters"].is_object()) {
                    for (auto it = lj["parameters"].begin(); it != lj["parameters"].end(); ++it) {
                        if (it.value().is_string()) {
                            layer.parameters[it.key()] = it.value().get<std::string>();
                        } else if (it.value().is_number()) {
                            layer.parameters[it.key()] = std::to_string(it.value().get<double>());
                        }
                    }
                }
                spec.layers.push_back(layer);
            }
        }

        return spec;
    } catch (const std::exception& e) {
        spdlog::error("[EffectPresetAIGenerator] Failed to parse JSON spec: {}", e.what());
        return std::nullopt;
    }
}

bool EffectPresetAIGenerator::ValidateParametersAgainstMetadata(EffectLayerSpec& layerSpec, const std::string& metadataDir) {
    if (metadataDir.empty() || layerSpec.effectName.empty()) return true;

    std::filesystem::path metaPath = std::filesystem::u8path(metadataDir) / (layerSpec.effectName + ".json");
    std::ifstream metaFile(metaPath);
    if (!metaFile.is_open()) {
        spdlog::warn("[EffectPresetAIGenerator] Effect metadata file missing for '{}' at {}", layerSpec.effectName, metaPath.string());
        return true;
    }

    try {
        nlohmann::json metaJson;
        metaFile >> metaJson;

        std::vector<std::string> invalidKeys;
        for (const auto& [paramId, paramVal] : layerSpec.parameters) {
            if (metaJson.contains("parameters") && metaJson["parameters"].is_object()) {
                if (!metaJson["parameters"].contains(paramId)) {
                    invalidKeys.push_back(paramId);
                }
            }
        }

        // Strip unrecognized parameter keys
        for (const auto& key : invalidKeys) {
            spdlog::warn("[EffectPresetAIGenerator] Stripping unrecognized parameter '{}' for effect '{}'", key, layerSpec.effectName);
            layerSpec.parameters.erase(key);
        }

        return true;
    } catch (const std::exception& e) {
        spdlog::error("[EffectPresetAIGenerator] Error validating metadata for '{}': {}", layerSpec.effectName, e.what());
        return false;
    }
}

} // namespace xLights::AI
