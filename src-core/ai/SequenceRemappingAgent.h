#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <functional>
#include <string>
#include <vector>

struct ModelEmbeddingVector {
    std::string modelName;
    std::string modelType;
    int nodeCount = 0;
    int strandCount = 1;
    float posX = 0.0f;
    float posY = 0.0f;
    float posZ = 0.0f;
    std::vector<float> embedding; // 16-dimensional dense embedding
};

struct ModelMappingPair {
    std::string sourceModel;
    std::string targetModel;
    float similarityScore = 0.0f; // Cosine similarity 0.0 to 1.0
    std::string matchReason;
};

struct CrossDisplayRemapResult {
    bool success = false;
    int totalSourceModels = 0;
    int mappedModelCount = 0;
    std::vector<ModelMappingPair> mappingPairs;
    std::string summaryReport;
};

class SequenceRemappingAgent {
public:
    SequenceRemappingAgent() = default;
    ~SequenceRemappingAgent() = default;

    // Compute dense 16-D vector embedding for a model
    static ModelEmbeddingVector ComputeModelEmbedding(const std::string& name,
                                                      const std::string& type,
                                                      int nodeCount,
                                                      int strandCount,
                                                      float x, float y, float z);

    // Compute Cosine Similarity between two vector embeddings
    static float ComputeCosineSimilarity(const std::vector<float>& vecA, const std::vector<float>& vecB);

    // Remap source sequence models to target display models using vector embedding matching
    static CrossDisplayRemapResult RemapSequenceModels(const std::vector<ModelEmbeddingVector>& sourceModels,
                                                        const std::vector<ModelEmbeddingVector>& targetModels,
                                                        float minSimilarityThreshold = 0.60f);

    // Apply model mapping to sequence .xsq XML string payload
    static std::string ApplyRemappingToSequenceXML(const std::string& xsqXmlContent,
                                                   const std::vector<ModelMappingPair>& mappings);
};
