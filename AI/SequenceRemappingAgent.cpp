/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceRemappingAgent.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <regex>
#include <set>

namespace xLights::AI {

nlohmann::json RemapPlanResult::ToJson() const {
    nlohmann::json j;
    j["success"] = success;
    j["average_confidence"] = averageConfidence;
    j["summary_message"] = summaryMessage;

    nlohmann::json mappingList = nlohmann::json::array();
    for (const auto& m : mappings) {
        nlohmann::json entry;
        entry["source_model"] = m.sourceModel;
        entry["target_model"] = m.targetModel;
        entry["similarity_score"] = m.similarityScore;
        entry["confidence"] = m.confidence;
        entry["rationale"] = m.rationale;
        entry["submodel_map"] = m.submodelMap;
        mappingList.push_back(entry);
    }
    j["mappings"] = mappingList;
    j["unmapped_source_models"] = unmappedSourceModels;
    j["unmapped_target_models"] = unmappedTargetModels;
    return j;
}

ModelCategory SequenceRemappingAgent::ParseCategory(const std::string& categoryStr) {
    std::string s = categoryStr;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);

    if (s.find("face") != std::string::npos || s.find("singing") != std::string::npos) return ModelCategory::SINGING_FACE;
    if (s.find("matrix") != std::string::npos || s.find("panel") != std::string::npos) return ModelCategory::MATRIX;
    if (s.find("tree360") != std::string::npos || s.find("megatree") != std::string::npos) return ModelCategory::TREE_360;
    if (s.find("tree") != std::string::npos) return ModelCategory::TREE_FLAT;
    if (s.find("arch") != std::string::npos) return ModelCategory::ARCH;
    if (s.find("spinner") != std::string::npos || s.find("rosa") != std::string::npos || s.find("wheel") != std::string::npos) return ModelCategory::SPINNER;
    if (s.find("star") != std::string::npos) return ModelCategory::STAR;
    if (s.find("flood") != std::string::npos || s.find("wash") != std::string::npos) return ModelCategory::FLOOD_LIGHT;
    if (s.find("cane") != std::string::npos) return ModelCategory::CANDY_CANE;
    if (s.find("wreath") != std::string::npos) return ModelCategory::WREATH;
    if (s.find("custom") != std::string::npos) return ModelCategory::CUSTOM_PROP;

    return ModelCategory::UNKNOWN;
}

std::string SequenceRemappingAgent::CategoryToString(ModelCategory category) {
    switch (category) {
        case ModelCategory::MATRIX: return "Matrix";
        case ModelCategory::TREE_360: return "Tree 360 / MegaTree";
        case ModelCategory::TREE_FLAT: return "Flat Tree";
        case ModelCategory::ARCH: return "Arch";
        case ModelCategory::SINGING_FACE: return "Singing Face";
        case ModelCategory::SPINNER: return "Spinner";
        case ModelCategory::STAR: return "Star";
        case ModelCategory::FLOOD_LIGHT: return "Flood Light";
        case ModelCategory::CANDY_CANE: return "Candy Cane";
        case ModelCategory::WREATH: return "Wreath";
        case ModelCategory::CUSTOM_PROP: return "Custom Prop";
        default: return "Unknown";
    }
}

SequenceRemappingAgent::ModelEmbedding SequenceRemappingAgent::ComputeModelEmbedding(const ModelSpatialDescriptor& desc) {
    ModelEmbedding emb{};
    emb.fill(0.0f);

    // Feature 0: Category One-Hot / Normalized Index [0.0, 1.0]
    emb[0] = static_cast<float>(desc.category) / 11.0f;

    // Features 1-3: Normalized Bounding Box Aspect Ratios
    float maxDim = std::max({desc.width, desc.height, desc.depth, 1.0f});
    emb[1] = desc.width / maxDim;
    emb[2] = desc.height / maxDim;
    emb[3] = desc.depth / maxDim;

    // Feature 4: Aspect Ratio (W / H) clamped
    float whRatio = desc.height > 0.001f ? (desc.width / desc.height) : 1.0f;
    emb[4] = std::clamp(whRatio / 5.0f, 0.0f, 1.0f);

    // Feature 5: Log-scale Total Nodes (normalized up to ~10,000 nodes)
    float logNodes = desc.totalNodes > 0 ? std::log10(static_cast<float>(desc.totalNodes) + 1.0f) : 0.0f;
    emb[5] = std::clamp(logNodes / 4.0f, 0.0f, 1.0f); // 10^4 = 10000 -> 1.0

    // Feature 6: Strand Count Log-Scale
    float logStrands = desc.strandCount > 0 ? std::log2(static_cast<float>(desc.strandCount) + 1.0f) : 0.0f;
    emb[6] = std::clamp(logStrands / 6.0f, 0.0f, 1.0f); // 2^6 = 64 strands -> 1.0

    // Feature 7: Nodes per Strand
    float nodesPerStrand = desc.strandCount > 0 ? (static_cast<float>(desc.totalNodes) / desc.strandCount) : 0.0f;
    emb[7] = std::clamp(nodesPerStrand / 200.0f, 0.0f, 1.0f);

    // Feature 8: Dimensionality (2D vs 3D)
    emb[8] = desc.is3D ? 1.0f : 0.0f;

    // Feature 9: Submodel Count
    float submodelCount = static_cast<float>(desc.submodelNames.size());
    emb[9] = std::clamp(submodelCount / 10.0f, 0.0f, 1.0f);

    // Features 10-12: Relative Spatial Anchor Coordinates (normalized position assuming ~100m show layout)
    emb[10] = std::clamp((desc.posX + 50.0f) / 100.0f, 0.0f, 1.0f);
    emb[11] = std::clamp((desc.posY + 50.0f) / 100.0f, 0.0f, 1.0f);
    emb[12] = std::clamp((desc.posZ + 50.0f) / 100.0f, 0.0f, 1.0f);

    // Feature 13: Channels per node (1=Single color, 3=RGB, 4=RGBW)
    emb[13] = std::clamp(static_cast<float>(desc.channelsPerNode) / 4.0f, 0.0f, 1.0f);

    // Feature 14: Category-specific signature weighting (Tree/Matrix/Face weight)
    if (desc.category == ModelCategory::MATRIX || desc.category == ModelCategory::TREE_360) {
        emb[14] = 1.0f;
    } else if (desc.category == ModelCategory::SINGING_FACE) {
        emb[14] = 0.75f;
    } else if (desc.category == ModelCategory::ARCH || desc.category == ModelCategory::SPINNER) {
        emb[14] = 0.5f;
    } else {
        emb[14] = 0.25f;
    }

    // Feature 15: Name / Shape tag hash projection [0.0, 1.0]
    size_t nameHash = std::hash<std::string>{}(desc.customShapeTag.empty() ? desc.name : desc.customShapeTag);
    emb[15] = static_cast<float>(nameHash % 1000) / 1000.0f;

    // L2 Normalize the embedding vector
    float normSq = 0.0f;
    for (float val : emb) {
        normSq += val * val;
    }
    float norm = std::sqrt(normSq);
    if (norm > 1e-6f) {
        for (float& val : emb) {
            val /= norm;
        }
    }

    return emb;
}

float SequenceRemappingAgent::ComputeCosineSimilarity(const ModelEmbedding& a, const ModelEmbedding& b) {
    float dot = 0.0f;
    for (size_t i = 0; i < EMBEDDING_DIM; ++i) {
        dot += a[i] * b[i];
    }
    // Since embeddings are unit normalized, dot product is cosine similarity
    return std::clamp(dot, 0.0f, 1.0f);
}

RemapPlanResult SequenceRemappingAgent::GenerateRemapPlan(
    const std::vector<ModelSpatialDescriptor>& sourceModels,
    const std::vector<ModelSpatialDescriptor>& targetModels,
    float minimumSimilarityThreshold
) {
    RemapPlanResult result;
    if (sourceModels.empty() || targetModels.empty()) {
        result.success = false;
        result.summaryMessage = "Source or target model list is empty.";
        return result;
    }

    // Compute embeddings for all source and target models
    std::vector<ModelEmbedding> srcEmbeddings;
    srcEmbeddings.reserve(sourceModels.size());
    for (const auto& src : sourceModels) {
        srcEmbeddings.push_back(ComputeModelEmbedding(src));
    }

    std::vector<ModelEmbedding> tgtEmbeddings;
    tgtEmbeddings.reserve(targetModels.size());
    for (const auto& tgt : targetModels) {
        tgtEmbeddings.push_back(ComputeModelEmbedding(tgt));
    }

    std::set<size_t> usedTargetIndices;
    float confidenceSum = 0.0f;

    for (size_t s = 0; s < sourceModels.size(); ++s) {
        const auto& src = sourceModels[s];
        const auto& srcEmb = srcEmbeddings[s];

        float bestScore = -1.0f;
        int bestTargetIdx = -1;

        for (size_t t = 0; t < targetModels.size(); ++t) {
            if (usedTargetIndices.find(t) != usedTargetIndices.end()) {
                continue;
            }

            const auto& tgt = targetModels[t];
            const auto& tgtEmb = tgtEmbeddings[t];

            float sim = ComputeCosineSimilarity(srcEmb, tgtEmb);

            // Category match bonus
            if (src.category != ModelCategory::UNKNOWN && src.category == tgt.category) {
                sim = std::min(1.0f, sim + 0.15f);
            }

            // Exact or partial name match bonus
            std::string sName = src.name;
            std::string tName = tgt.name;
            std::transform(sName.begin(), sName.end(), sName.begin(), ::tolower);
            std::transform(tName.begin(), tName.end(), tName.begin(), ::tolower);
            if (sName == tName) {
                sim = std::min(1.0f, sim + 0.20f);
            } else if (sName.find(tName) != std::string::npos || tName.find(sName) != std::string::npos) {
                sim = std::min(1.0f, sim + 0.10f);
            }

            if (sim > bestScore) {
                bestScore = sim;
                bestTargetIdx = static_cast<int>(t);
            }
        }

        if (bestTargetIdx >= 0 && bestScore >= minimumSimilarityThreshold) {
            usedTargetIndices.insert(static_cast<size_t>(bestTargetIdx));
            const auto& tgt = targetModels[static_cast<size_t>(bestTargetIdx)];

            ModelMappingMatch match;
            match.sourceModel = src.name;
            match.targetModel = tgt.name;
            match.similarityScore = bestScore;
            match.confidence = std::clamp(bestScore, 0.0f, 1.0f);

            std::ostringstream ss;
            ss << "Matched " << CategoryToString(src.category) << " to " << CategoryToString(tgt.category)
               << " with similarity score " << std::fixed << std::setprecision(2) << bestScore;
            match.rationale = ss.str();

            // Submodel alignment
            for (const auto& srcSub : src.submodelNames) {
                for (const auto& tgtSub : tgt.submodelNames) {
                    if (srcSub == tgtSub) {
                        match.submodelMap[srcSub] = tgtSub;
                        break;
                    }
                }
            }

            confidenceSum += match.confidence;
            result.mappings.push_back(match);
        } else {
            result.unmappedSourceModels.push_back(src.name);
        }
    }

    for (size_t t = 0; t < targetModels.size(); ++t) {
        if (usedTargetIndices.find(t) == usedTargetIndices.end()) {
            result.unmappedTargetModels.push_back(targetModels[t].name);
        }
    }

    result.success = !result.mappings.empty();
    result.averageConfidence = result.mappings.empty() ? 0.0f : (confidenceSum / static_cast<float>(result.mappings.size()));

    std::ostringstream sumMsg;
    sumMsg << "Remapping Plan: " << result.mappings.size() << " models mapped, "
           << result.unmappedSourceModels.size() << " unmapped source props, "
           << result.unmappedTargetModels.size() << " unmapped target props (Avg Confidence: "
           << std::fixed << std::setprecision(1) << (result.averageConfidence * 100.0f) << "%).";
    result.summaryMessage = sumMsg.str();

    return result;
}

std::string SequenceRemappingAgent::ApplyRemappingToSequenceXML(
    const std::string& sequenceXmlContent,
    const RemapPlanResult& plan
) {
    if (!plan.success || plan.mappings.empty() || sequenceXmlContent.empty()) {
        return sequenceXmlContent;
    }

    std::string modifiedXml = sequenceXmlContent;

    for (const auto& m : plan.mappings) {
        if (m.sourceModel == m.targetModel) continue;

        // Replace Target="..." attributes in sequence XML
        std::string patternTarget = "Target=\"" + m.sourceModel + "\"";
        std::string replaceTarget = "Target=\"" + m.targetModel + "\"";
        
        size_t pos = 0;
        while ((pos = modifiedXml.find(patternTarget, pos)) != std::string::npos) {
            modifiedXml.replace(pos, patternTarget.length(), replaceTarget);
            pos += replaceTarget.length();
        }

        // Replace model names in Element type="model" name="..." tags
        std::string patternModelTag = "name=\"" + m.sourceModel + "\"";
        std::string replaceModelTag = "name=\"" + m.targetModel + "\"";
        pos = 0;
        while ((pos = modifiedXml.find(patternModelTag, pos)) != std::string::npos) {
            modifiedXml.replace(pos, patternModelTag.length(), replaceModelTag);
            pos += replaceModelTag.length();
        }
    }

    return modifiedXml;
}

} // namespace xLights::AI
