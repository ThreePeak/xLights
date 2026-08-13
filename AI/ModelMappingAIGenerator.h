#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #4: Structured Model Mapping & 4-Pass LLM Import

#include "AISubsystemBase.h"
#include <string>
#include <vector>
#include <map>
#include <future>

namespace xLights::AI {

enum class MappingPassStage {
    PrimaryModelMatching = 1,
    SubmodelChannelAlignment = 2,
    StrandOrderingResolution = 3,
    NodeLevelMappingVerification = 4
};

struct SourceChannelSpec {
    std::string channelName;           // e.g. "Vendor_MegaTree_String1"
    std::string propTypeHint;          // e.g. "Tree", "Star", "Arch"
    int nodeCount = 0;
    int strandCount = 1;
};

struct TargetModelSpec {
    std::string modelName;             // e.g. "My_MegaTree"
    std::string modelType;             // e.g. "Tree", "Star", "Custom"
    int nodeCount = 0;
    int strandCount = 1;
    std::vector<std::string> submodels;// e.g. ["Outer_Ring", "Inner_Core"]
};

struct ChannelMappingPair {
    std::string sourceChannelName;
    std::string targetModelName;
    std::string targetSubmodelName;
    float confidenceScore = 0.0f;      // 0.0 to 1.0
    MappingPassStage matchedPass = MappingPassStage::PrimaryModelMatching;
    std::string matchReason;
};

struct ModelMappingConfig {
    std::vector<SourceChannelSpec> sourceChannels;
    std::vector<TargetModelSpec> targetModels;
    float minimumConfidenceThreshold = 0.5f;
    bool enableFourPassEngine = true;
};

struct ModelMappingResult {
    bool success = false;
    std::string errorMessage;
    int totalSourceChannels = 0;
    int mappedChannelsCount = 0;
    int unmappedChannelsCount = 0;
    std::vector<ChannelMappingPair> mappings;
    std::string mappingSummary;
};

class ModelMappingAIGenerator : public AISubsystemBase {
public:
    ModelMappingAIGenerator(ServiceManager* sm = nullptr) : AISubsystemBase(sm) {}
    virtual ~ModelMappingAIGenerator() override = default;

    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override {
        return std::async(std::launch::async, [this, callback]() {
            if (callback) callback("Initializing ModelMappingAIGenerator...", 0.0f);
            m_isInitialized.store(true);
            if (callback) callback("ModelMappingAIGenerator initialized.", 100.0f);
            return true;
        });
    }

    virtual void Shutdown() override {
        m_isInitialized.store(false);
    }

    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "ModelMappingAIGenerator"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"four_pass_model_mapping", "vendor_sequence_import", "submodel_channel_alignment", "strand_ordering_resolution"};
    }

    // Four-Pass LLM Fallback Engine for Model Mapping
    [[nodiscard]] static ModelMappingResult GenerateModelMapping(const ModelMappingConfig& config);

    // Export structured JSON report of channel mappings
    [[nodiscard]] static std::string ExportMappingReportJSON(const ModelMappingResult& result);
};

} // namespace xLights::AI
