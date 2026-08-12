#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "../src-core/ai/AISubsystemBase.h"
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <memory>
#include <nlohmann/json.hpp>

namespace xLights::AI {

struct EffectLayerSpec {
    int layerIndex = 0;                        // 0-indexed track layer position
    std::string effectName;                    // e.g., "Bars", "SingleStrand", "Twinkle", "Morph"
    std::string blendMode = "Normal";          // "Normal", "Add", "Subtract", "Mask", "Unmask", "True 3D Mask"
    std::vector<std::string> hexPalette;       // e.g., ["#00FFFF", "#FFFFFF"]
    std::map<std::string, std::string> parameters; // Key-value pairs matching effect parameters
};

struct EffectPresetSpec {
    std::string presetName;
    std::string styleDescription;
    std::vector<EffectLayerSpec> layers;

    [[nodiscard]] nlohmann::json ToJson() const;
};

/**
 * @brief Automated Effect Preset Generator (.xeffect) with XML schema compilation,
 * parameter validation against metadata schemas, and structured JSON parsing.
 */
class EffectPresetAIGenerator : public AISubsystemBase {
public:
    EffectPresetAIGenerator(ServiceManager* sm = nullptr);
    virtual ~EffectPresetAIGenerator() override = default;

    // AISubsystemBase lifecycle implementation
    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override;
    virtual void Shutdown() override;
    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "EffectPresetAIGenerator"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"xeffect_xml_compilation", "effect_metadata_validation", "llm_json_preset_parsing"};
    }

    /**
     * @brief Compiles an EffectPresetSpec C++ struct into a valid, formatted .xeffect XML string payload.
     */
    [[nodiscard]] static std::string CompileSpecToXEffectXML(const EffectPresetSpec& spec);

    /**
     * @brief Parses structured JSON payload returned by LLM into C++ EffectPresetSpec struct.
     */
    [[nodiscard]] static std::optional<EffectPresetSpec> ParseJSONToSpec(const std::string& jsonPayload);

    /**
     * @brief Validates generated effect parameters against resources/effectmetadata/[EffectName].json.
     * Strips any parameter key in layerSpec.parameters that does not exist or falls outside constraints.
     */
    static bool ValidateParametersAgainstMetadata(EffectLayerSpec& layerSpec, const std::string& metadataDir);
};

} // namespace xLights::AI
