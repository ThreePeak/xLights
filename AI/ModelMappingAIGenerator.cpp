/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #4: Four-Pass LLM Fallback Engine for Model Mapping

#include "ModelMappingAIGenerator.h"
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

static bool ContainsIgnoreCase(std::string_view str, std::string_view sub) {
    return ToLower(str).find(ToLower(sub)) != std::string::npos;
}

ModelMappingResult ModelMappingAIGenerator::GenerateModelMapping(const ModelMappingConfig& config) {
    ModelMappingResult result;
    result.totalSourceChannels = static_cast<int>(config.sourceChannels.size());

    if (config.sourceChannels.empty() || config.targetModels.empty()) {
        result.success = false;
        result.errorMessage = "sourceChannels and targetModels must not be empty.";
        spdlog::error("ModelMappingAIGenerator: {}", result.errorMessage);
        return result;
    }

    std::vector<bool> channelMapped(config.sourceChannels.size(), false);

    // Pass 1: Primary Model Matching (Exact/High fuzzy match on model name & prop type)
    for (size_t i = 0; i < config.sourceChannels.size(); ++i) {
        const auto& src = config.sourceChannels[i];
        for (const auto& tgt : config.targetModels) {
            if (ContainsIgnoreCase(src.channelName, tgt.modelName) || 
               (!src.propTypeHint.empty() && ContainsIgnoreCase(tgt.modelType, src.propTypeHint))) {
                ChannelMappingPair pair;
                pair.sourceChannelName = src.channelName;
                pair.targetModelName = tgt.modelName;
                pair.confidenceScore = 0.95f;
                pair.matchedPass = MappingPassStage::PrimaryModelMatching;
                pair.matchReason = "Primary Model Name/Type match ('" + tgt.modelName + "')";
                result.mappings.push_back(pair);
                channelMapped[i] = true;
                break;
            }
        }
    }

    // Pass 2: Submodel Channel Alignment (Matching submodel names e.g., Eyes, Mouth, Ring)
    for (size_t i = 0; i < config.sourceChannels.size(); ++i) {
        if (channelMapped[i]) continue;
        const auto& src = config.sourceChannels[i];

        for (const auto& tgt : config.targetModels) {
            for (const auto& submodel : tgt.submodels) {
                if (ContainsIgnoreCase(src.channelName, submodel)) {
                    ChannelMappingPair pair;
                    pair.sourceChannelName = src.channelName;
                    pair.targetModelName = tgt.modelName;
                    pair.targetSubmodelName = submodel;
                    pair.confidenceScore = 0.85f;
                    pair.matchedPass = MappingPassStage::SubmodelChannelAlignment;
                    pair.matchReason = "Submodel alignment match ('" + submodel + "' on '" + tgt.modelName + "')";
                    result.mappings.push_back(pair);
                    channelMapped[i] = true;
                    break;
                }
            }
            if (channelMapped[i]) break;
        }
    }

    // Pass 3: Strand Ordering Resolution (Matching strand count / node count patterns)
    for (size_t i = 0; i < config.sourceChannels.size(); ++i) {
        if (channelMapped[i]) continue;
        const auto& src = config.sourceChannels[i];

        for (const auto& tgt : config.targetModels) {
            if (src.strandCount == tgt.strandCount && src.strandCount > 1) {
                ChannelMappingPair pair;
                pair.sourceChannelName = src.channelName;
                pair.targetModelName = tgt.modelName;
                pair.confidenceScore = 0.70f;
                pair.matchedPass = MappingPassStage::StrandOrderingResolution;
                pair.matchReason = "Strand ordering count match (" + std::to_string(src.strandCount) + " strands)";
                result.mappings.push_back(pair);
                channelMapped[i] = true;
                break;
            }
        }
    }

    // Pass 4: Node-Level Mapping Verification (Node density / ratio matching fallback)
    for (size_t i = 0; i < config.sourceChannels.size(); ++i) {
        if (channelMapped[i]) continue;
        const auto& src = config.sourceChannels[i];

        const TargetModelSpec* bestTgt = nullptr;
        float minRatioDiff = 1.0f;

        for (const auto& tgt : config.targetModels) {
            if (tgt.nodeCount > 0 && src.nodeCount > 0) {
                float diff = std::abs(1.0f - (static_cast<float>(src.nodeCount) / static_cast<float>(tgt.nodeCount)));
                if (diff < minRatioDiff && diff < 0.5f) {
                    minRatioDiff = diff;
                    bestTgt = &tgt;
                }
            }
        }

        if (bestTgt) {
            ChannelMappingPair pair;
            pair.sourceChannelName = src.channelName;
            pair.targetModelName = bestTgt->modelName;
            pair.confidenceScore = 0.55f;
            pair.matchedPass = MappingPassStage::NodeLevelMappingVerification;
            pair.matchReason = "Node count ratio verification match (node diff: " + std::to_string(static_cast<int>(minRatioDiff * 100)) + "%)";
            result.mappings.push_back(pair);
            channelMapped[i] = true;
        }
    }

    result.mappedChannelsCount = static_cast<int>(result.mappings.size());
    result.unmappedChannelsCount = result.totalSourceChannels - result.mappedChannelsCount;

    std::ostringstream summary;
    summary << "4-Pass Model Mapping Engine: Mapped " << result.mappedChannelsCount << "/"
            << result.totalSourceChannels << " channels across " << config.targetModels.size() << " target model(s).";
    result.mappingSummary = summary.str();

    result.success = true;
    spdlog::info("ModelMappingAIGenerator: {}", result.mappingSummary);
    return result;
}

std::string ModelMappingAIGenerator::ExportMappingReportJSON(const ModelMappingResult& result) {
    nlohmann::json root;
    root["success"] = result.success;
    root["errorMessage"] = result.errorMessage;
    root["totalSourceChannels"] = result.totalSourceChannels;
    root["mappedChannelsCount"] = result.mappedChannelsCount;
    root["unmappedChannelsCount"] = result.unmappedChannelsCount;
    root["mappingSummary"] = result.mappingSummary;

    nlohmann::json mappings = nlohmann::json::array();
    for (const auto& m : result.mappings) {
        nlohmann::json mapObj;
        mapObj["sourceChannelName"] = m.sourceChannelName;
        mapObj["targetModelName"] = m.targetModelName;
        mapObj["targetSubmodelName"] = m.targetSubmodelName;
        mapObj["confidenceScore"] = m.confidenceScore;
        mapObj["matchedPass"] = static_cast<int>(m.matchedPass);
        mapObj["matchReason"] = m.matchReason;
        mappings.push_back(mapObj);
    }
    root["mappings"] = mappings;

    return root.dump(2);
}

} // namespace xLights::AI
