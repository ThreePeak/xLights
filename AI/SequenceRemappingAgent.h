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
#include <array>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace xLights::AI {

enum class ModelCategory {
    UNKNOWN = 0,
    MATRIX,
    TREE_360,
    TREE_FLAT,
    ARCH,
    SINGING_FACE,
    SPINNER,
    STAR,
    FLOOD_LIGHT,
    CANDY_CANE,
    WREATH,
    CUSTOM_PROP
};

struct ModelSpatialDescriptor {
    std::string name;
    ModelCategory category{ModelCategory::UNKNOWN};
    float width{0.0f};
    float height{0.0f};
    float depth{0.0f};
    float posX{0.0f};
    float posY{0.0f};
    float posZ{0.0f};
    uint32_t totalNodes{0};
    uint32_t strandCount{1};
    uint32_t channelsPerNode{3};
    bool is3D{false};
    std::vector<std::string> submodelNames;
    std::string customShapeTag;
};

struct ModelMappingMatch {
    std::string sourceModel;
    std::string targetModel;
    float similarityScore{0.0f};
    float confidence{0.0f};
    std::string rationale;
    std::unordered_map<std::string, std::string> submodelMap;
};

struct RemapPlanResult {
    bool success{false};
    float averageConfidence{0.0f};
    std::vector<ModelMappingMatch> mappings;
    std::vector<std::string> unmappedSourceModels;
    std::vector<std::string> unmappedTargetModels;
    std::string summaryMessage;

    nlohmann::json ToJson() const;
};

class SequenceRemappingAgent {
public:
    static constexpr size_t EMBEDDING_DIM = 16;
    using ModelEmbedding = std::array<float, EMBEDDING_DIM>;

    SequenceRemappingAgent() = default;
    ~SequenceRemappingAgent() = default;

    /// Computes a normalized 16-dimensional dense embedding vector for a given model descriptor
    static ModelEmbedding ComputeModelEmbedding(const ModelSpatialDescriptor& desc);

    /// Computes the cosine similarity between two 16-D embedding vectors in [0.0, 1.0]
    static float ComputeCosineSimilarity(const ModelEmbedding& a, const ModelEmbedding& b);

    /// Generates an optimal model remapping plan from source display models to target display models
    static RemapPlanResult GenerateRemapPlan(
        const std::vector<ModelSpatialDescriptor>& sourceModels,
        const std::vector<ModelSpatialDescriptor>& targetModels,
        float minimumSimilarityThreshold = 0.50f
    );

    /// Applies model remapping substitutions to sequence XML content
    static std::string ApplyRemappingToSequenceXML(
        const std::string& sequenceXmlContent,
        const RemapPlanResult& plan
    );

    /// Utility: Parse ModelCategory from string name
    static ModelCategory ParseCategory(const std::string& categoryStr);
    static std::string CategoryToString(ModelCategory category);
};

} // namespace xLights::AI
