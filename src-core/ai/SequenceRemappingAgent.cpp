/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceRemappingAgent.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>
#include <sstream>

ModelEmbeddingVector SequenceRemappingAgent::ComputeModelEmbedding(const std::string& name,
                                                                    const std::string& type,
                                                                    int nodeCount,
                                                                    int strandCount,
                                                                    float x, float y, float z) {
    ModelEmbeddingVector vec;
    vec.modelName = name;
    vec.modelType = type;
    vec.nodeCount = nodeCount;
    vec.strandCount = strandCount;
    vec.posX = x;
    vec.posY = y;
    vec.posZ = z;

    vec.embedding.resize(16, 0.0f);

    // Feature 0: Type Hash / Categorical
    std::string lowerType = type;
    std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);
    if (lowerType.find("matrix") != std::string::npos) vec.embedding[0] = 1.0f;
    else if (lowerType.find("tree") != std::string::npos) vec.embedding[1] = 1.0f;
    else if (lowerType.find("arch") != std::string::npos) vec.embedding[2] = 1.0f;
    else if (lowerType.find("star") != std::string::npos) vec.embedding[3] = 1.0f;
    else if (lowerType.find("window") != std::string::npos) vec.embedding[4] = 1.0f;
    else vec.embedding[5] = 1.0f;

    // Feature 6-8: Normalized 3D Position
    vec.embedding[6] = x / 100.0f;
    vec.embedding[7] = y / 100.0f;
    vec.embedding[8] = z / 100.0f;

    // Feature 9-10: Node Density & Strand Count
    vec.embedding[9] = std::min(1.0f, (float)nodeCount / 1000.0f);
    vec.embedding[10] = std::min(1.0f, (float)strandCount / 16.0f);

    // Normalize entire embedding vector
    float norm = 0.0f;
    for (float val : vec.embedding) norm += val * val;
    norm = std::sqrt(norm);
    if (norm > 0.0001f) {
        for (float& val : vec.embedding) val /= norm;
    }

    return vec;
}

float SequenceRemappingAgent::ComputeCosineSimilarity(const std::vector<float>& vecA, const std::vector<float>& vecB) {
    if (vecA.size() != vecB.size() || vecA.empty()) return 0.0f;
    float dot = 0.0f;
    float normA = 0.0f;
    float normB = 0.0f;

    for (size_t i = 0; i < vecA.size(); ++i) {
        dot += vecA[i] * vecB[i];
        normA += vecA[i] * vecA[i];
        normB += vecB[i] * vecB[i];
    }

    float denom = std::sqrt(normA) * std::sqrt(normB);
    return (denom > 0.0001f) ? std::max(0.0f, std::min(1.0f, dot / denom)) : 0.0f;
}

CrossDisplayRemapResult SequenceRemappingAgent::RemapSequenceModels(const std::vector<ModelEmbeddingVector>& sourceModels,
                                                                      const std::vector<ModelEmbeddingVector>& targetModels,
                                                                      float minSimilarityThreshold) {
    CrossDisplayRemapResult result;
    result.totalSourceModels = (int)sourceModels.size();

    if (sourceModels.empty() || targetModels.empty()) {
        result.summaryReport = "Empty source or target model list.";
        return result;
    }

    spdlog::info("SequenceRemappingAgent: Remapping {} source models across {} target models",
                 sourceModels.size(), targetModels.size());

    std::vector<bool> targetMapped(targetModels.size(), false);

    for (const auto& src : sourceModels) {
        float maxSim = -1.0f;
        int bestTargetIdx = -1;

        for (size_t t = 0; t < targetModels.size(); ++t) {
            if (targetMapped[t]) continue;
            float sim = ComputeCosineSimilarity(src.embedding, targetModels[t].embedding);

            // Exact name match boost
            if (src.modelName == targetModels[t].modelName) sim = 1.0f;

            if (sim > maxSim) {
                maxSim = sim;
                bestTargetIdx = (int)t;
            }
        }

        if (bestTargetIdx != -1 && maxSim >= minSimilarityThreshold) {
            targetMapped[bestTargetIdx] = true;
            ModelMappingPair pair;
            pair.sourceModel = src.modelName;
            pair.targetModel = targetModels[bestTargetIdx].modelName;
            pair.similarityScore = maxSim;

            std::ostringstream ss;
            ss << "Vector Similarity Match: " << (int)(maxSim * 100.0f) << "% (" << src.modelType << " -> " << targetModels[bestTargetIdx].modelType << ")";
            pair.matchReason = ss.str();

            result.mappingPairs.push_back(pair);
            result.mappedModelCount++;
        }
    }

    std::ostringstream ss;
    ss << "Cross-Display Sequence Remapping Report:\n"
       << "  - Total Source Models: " << result.totalSourceModels << "\n"
       << "  - Successfully Mapped Models: " << result.mappedModelCount << "\n"
       << "  - Unmapped Source Models: " << (result.totalSourceModels - result.mappedModelCount) << "\n";
    result.summaryReport = ss.str();
    result.success = true;

    spdlog::info("SequenceRemappingAgent: Successfully mapped {}/{} models.", result.mappedModelCount, result.totalSourceModels);
    return result;
}

std::string SequenceRemappingAgent::ApplyRemappingToSequenceXML(const std::string& xsqXmlContent,
                                                                 const std::vector<ModelMappingPair>& mappings) {
    std::string remappedXml = xsqXmlContent;
    for (const auto& pair : mappings) {
        std::string srcTag = "target=\"" + pair.sourceModel + "\"";
        std::string tgtTag = "target=\"" + pair.targetModel + "\"";

        size_t pos = 0;
        while ((pos = remappedXml.find(srcTag, pos)) != std::string::npos) {
            remappedXml.replace(pos, srcTag.length(), tgtTag);
            pos += tgtTag.length();
        }
    }
    return remappedXml;
}
