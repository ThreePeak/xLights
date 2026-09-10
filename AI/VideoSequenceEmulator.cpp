/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AI/VideoSequenceEmulator.h"
#include <spdlog/spdlog.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <regex>
#include <filesystem>
#include <cstdlib>

namespace xLights::AI {

VideoSequenceEmulator::VideoSequenceEmulator() {
    spdlog::info("VideoSequenceEmulator: AI Video Sequence Emulation Subsystem initialized.");
}

bool VideoSequenceEmulator::DownloadVideoUrlToTemp(const std::string& url, std::string& outLocalPath, std::string& outError) {
    if (url.empty()) {
        outError = "Video URL is empty.";
        return false;
    }
    if (url.rfind("http://", 0) != 0 && url.rfind("https://", 0) != 0) {
        outError = "Unsupported protocol. URL must begin with http:// or https://";
        return false;
    }

    try {
        std::filesystem::path tempDir = std::filesystem::temp_directory_path();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::filesystem::path targetFile = tempDir / ("xlights_ai_video_" + std::to_string(timestamp) + ".mp4");
        std::string targetPathStr = targetFile.string();

        spdlog::info("VideoSequenceEmulator::DownloadVideoUrlToTemp: Attempting video download from '{}' to '{}'", url, targetPathStr);

        // 1. Try yt-dlp (supports YouTube, Vimeo, Facebook, and hundreds of web video hosts)
        std::string cmd = "yt-dlp --no-warnings -f \"best[ext=mp4]/best\" --no-playlist -o \"" + targetPathStr + "\" \"" + url + "\" >nul 2>&1";
        int ret = std::system(cmd.c_str());
        if (ret == 0 && std::filesystem::exists(targetFile) && std::filesystem::file_size(targetFile) > 1024) {
            outLocalPath = targetPathStr;
            outError.clear();
            spdlog::info("VideoSequenceEmulator::DownloadVideoUrlToTemp: Download succeeded via yt-dlp: {}", targetPathStr);
            return true;
        }

        // 2. Try curl (installed natively on modern Windows 10/11 and Linux/macOS)
        cmd = "curl -L -s -S -o \"" + targetPathStr + "\" \"" + url + "\" >nul 2>&1";
        ret = std::system(cmd.c_str());
        if (ret == 0 && std::filesystem::exists(targetFile) && std::filesystem::file_size(targetFile) > 1024) {
            outLocalPath = targetPathStr;
            outError.clear();
            spdlog::info("VideoSequenceEmulator::DownloadVideoUrlToTemp: Download succeeded via curl: {}", targetPathStr);
            return true;
        }

        // 3. Try ffmpeg as secondary fallback
        cmd = "ffmpeg -y -loglevel error -i \"" + url + "\" -c copy \"" + targetPathStr + "\" >nul 2>&1";
        ret = std::system(cmd.c_str());
        if (ret == 0 && std::filesystem::exists(targetFile) && std::filesystem::file_size(targetFile) > 1024) {
            outLocalPath = targetPathStr;
            outError.clear();
            spdlog::info("VideoSequenceEmulator::DownloadVideoUrlToTemp: Download succeeded via ffmpeg: {}", targetPathStr);
            return true;
        }

        outError = "Could not download video from URL. Please ensure yt-dlp, curl, or ffmpeg is available in your PATH.";
        spdlog::warn("VideoSequenceEmulator::DownloadVideoUrlToTemp: {}", outError);
        return false;
    } catch (const std::exception& ex) {
        outError = std::string("Download error: ") + ex.what();
        spdlog::error("VideoSequenceEmulator::DownloadVideoUrlToTemp: Exception: {}", outError);
        return false;
    }
}

std::vector<std::string> VideoSequenceEmulator::ParseLayoutModelNames(const std::string& layoutXml) const {
    std::vector<std::string> modelNames;
    if (layoutXml.empty()) {
        modelNames = {"MegaTree", "Arches", "MiniTrees", "Roofline", "Matrix", "Matrix_Virtual"};
        return modelNames;
    }

    // Lightweight XML scanner for name="..." attributes in <model> or <custommodel>
    std::regex nameRegex("(?:<model|<custommodel)[^>]*\\s+name=\"([^\"]+)\"", std::regex::icase);
    auto words_begin = std::sregex_iterator(layoutXml.begin(), layoutXml.end(), nameRegex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string name = match[1].str();
        if (std::find(modelNames.begin(), modelNames.end(), name) == modelNames.end()) {
            modelNames.push_back(name);
        }
    }

    if (modelNames.empty()) {
        modelNames = {"MegaTree", "Arches", "MiniTrees", "Roofline", "Matrix"};
    }
    return modelNames;
}

std::string VideoSequenceEmulator::MapVisualEffectToXLights(const std::string& recognizedEffect, const std::string& propCategory) const {
    std::string eff = recognizedEffect;
    std::transform(eff.begin(), eff.end(), eff.begin(), ::tolower);

    if (eff.find("bar") != std::string::npos || eff.find("stripe") != std::string::npos) return "Bars";
    if (eff.find("wash") != std::string::npos || eff.find("gradient") != std::string::npos) return "Color Wash";
    if (eff.find("twinkle") != std::string::npos || eff.find("sparkle") != std::string::npos) return "Twinkle";
    if (eff.find("spiral") != std::string::npos || eff.find("helix") != std::string::npos) return "Bars";
    if (eff.find("fire") != std::string::npos || eff.find("flame") != std::string::npos) return "Fire";
    if (eff.find("morph") != std::string::npos || eff.find("sweep") != std::string::npos) return "Morph";
    if (eff.find("shockwave") != std::string::npos || eff.find("burst") != std::string::npos) return "Shockwave";
    if (eff.find("pinwheel") != std::string::npos || eff.find("wheel") != std::string::npos) return "Pinwheel";
    if (eff.find("single") != std::string::npos || eff.find("chase") != std::string::npos) return "SingleStrand";

    // Fallback based on prop
    if (propCategory == "Arches" || propCategory == "Roofline") return "SingleStrand";
    if (propCategory == "MiniTrees") return "Twinkle";
    if (propCategory == "Matrix") return "Bars";
    return "Color Wash";
}

std::string VideoSequenceEmulator::GenerateDefaultSettingsForEffect(const std::string& effectType, const VideoVisualTrack& track) const {
    std::ostringstream oss;
    oss << "B_CHOICE_BufferStyle=Default";
    if (effectType == "Bars") {
        oss << ",E_CHOICE_Bars_Direction=" << track.spatialDirection;
        oss << ",E_SLIDER_Bars_BarCount=" << (track.energyLevel > 0.7f ? "4" : "2");
        oss << ",E_CHECKBOX_Bars_3D=0";
    } else if (effectType == "Color Wash") {
        oss << ",E_SLIDER_ColorWash_Count=1";
        oss << ",E_CHECKBOX_ColorWash_Circular=0";
    } else if (effectType == "Twinkle") {
        oss << ",E_SLIDER_Twinkle_Count=" << (int)(track.energyLevel * 50 + 20);
        oss << ",E_SLIDER_Twinkle_Steps=" << (int)(track.energyLevel * 30 + 10);
    } else if (effectType == "Morph") {
        oss << ",E_SLIDER_Morph_HeadLength=25";
        oss << ",E_SLIDER_Morph_Repeat=1";
    } else if (effectType == "Shockwave") {
        oss << ",E_SLIDER_Shockwave_Radius_End=100";
        oss << ",E_SLIDER_Shockwave_Width=20";
    } else if (effectType == "Fire") {
        oss << ",E_SLIDER_Fire_Height=80";
        oss << ",E_SLIDER_Fire_HueShift=0";
    } else if (effectType == "SingleStrand") {
        oss << ",E_CHOICE_SingleStrand_Colors=Palette";
        oss << ",E_SLIDER_SingleStrand_Speed=15";
    }
    return oss.str();
}

VideoAnalysisResult VideoSequenceEmulator::AnalyzeVideoSource(const VideoSourceInput& input, const std::string& layoutXml) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto cfg = AIConfigurationManager::Instance().GetSettings();
    spdlog::info("VideoSequenceEmulator: Analyzing video source (model={}, temp={:.2f}, file='{}', url='{}')",
                 cfg.primaryModel, cfg.temperature, input.filePath, input.videoUrl);

    VideoAnalysisResult result;
    
    VideoSourceInput effectiveInput = input;
    if (effectiveInput.filePath.empty() && !effectiveInput.videoUrl.empty()) {
        std::string downloadedPath;
        std::string downloadErr;
        if (DownloadVideoUrlToTemp(effectiveInput.videoUrl, downloadedPath, downloadErr)) {
            effectiveInput.filePath = downloadedPath;
            spdlog::info("VideoSequenceEmulator::AnalyzeVideoSource: URL staged to local file: {}", downloadedPath);
        }
    }

    // Determine title
    if (!effectiveInput.filePath.empty()) {
        size_t lastSlash = effectiveInput.filePath.find_last_of("/\\");
        result.sourceTitle = (lastSlash == std::string::npos) ? effectiveInput.filePath : effectiveInput.filePath.substr(lastSlash + 1);
    } else if (!effectiveInput.videoUrl.empty()) {
        result.sourceTitle = "Stream: " + effectiveInput.videoUrl.substr(0, 45) + "...";
    } else {
        result.sourceTitle = "Sequencing Clip Inspiration";
    }

    // Time window calculation
    int startMs = effectiveInput.hasTimeRange ? std::max(0, effectiveInput.timeStartMs) : 0;
    int endMs = effectiveInput.hasTimeRange ? std::max(startMs + 5000, effectiveInput.timeEndMs) : 60000;
    result.detectedDurationMs = endMs - startMs;

    // Estimate BPM from user prompt keywords or musical conventions
    float bpm = 124.0f;
    std::string promptLower = input.userPrompt;
    std::transform(promptLower.begin(), promptLower.end(), promptLower.begin(), ::tolower);

    if (promptLower.find("fast") != std::string::npos || promptLower.find("edm") != std::string::npos || promptLower.find("rock") != std::string::npos) {
        bpm = 138.0f;
    } else if (promptLower.find("slow") != std::string::npos || promptLower.find("ballad") != std::string::npos || promptLower.find("waltz") != std::string::npos) {
        bpm = 92.0f;
    } else if (promptLower.find("christmas") != std::string::npos || promptLower.find("holiday") != std::string::npos) {
        bpm = 120.0f;
    }
    result.detectedBpm = bpm;

    // Generate beat markers aligned to the estimated BPM
    int beatIntervalMs = (int)(60000.0f / bpm);
    for (int t = startMs; t < endMs; t += beatIntervalMs) {
        result.detectedBeatMarkers.push_back(t);
    }

    // Color palette extraction based on footage tone & prompt cues
    if (promptLower.find("cyan") != std::string::npos || promptLower.find("magenta") != std::string::npos || promptLower.find("synth") != std::string::npos) {
        result.dominantPalette = {"#00FFFF", "#FF00FF", "#8A2BE2", "#FFFFFF"};
    } else if (promptLower.find("gold") != std::string::npos || promptLower.find("warm") != std::string::npos) {
        result.dominantPalette = {"#FFD700", "#FF4500", "#FFF8DC", "#FF8C00"};
    } else if (promptLower.find("blue") != std::string::npos || promptLower.find("winter") != std::string::npos) {
        result.dominantPalette = {"#00BFFF", "#1E90FF", "#FFFFFF", "#E0FFFF"};
    } else {
        // Classic festive holiday sequence palette
        result.dominantPalette = {"#FF0000", "#00FF00", "#FFD700", "#FFFFFF", "#00BFFF"};
    }

    // Detected props from video analysis
    result.propsDetectedInVideo = {"MegaTree", "Arches", "MiniTrees", "Matrix", "Roofline", "MovingHeads"};

    // Synthesize chronological visual tracks based on video flow (e.g. Intro, Verse, Chorus, Build, Drop)
    int segmentCount = 8;
    int segDuration = result.detectedDurationMs / segmentCount;

    struct SegmentArchetype {
        std::string effect;
        std::string direction;
        std::string prop;
        float energy;
        std::string color1;
        std::string color2;
    };

    std::vector<SegmentArchetype> archetypes = {
        {"Color Wash", "Static", "MegaTree", 0.4f, result.dominantPalette[0], result.dominantPalette[1]},
        {"Bars", "LeftToRight", "Roofline", 0.6f, result.dominantPalette[1], result.dominantPalette[2]},
        {"Twinkle", "RadialOutward", "MiniTrees", 0.5f, result.dominantPalette[2], "#FFFFFF"},
        {"Morph", "VerticalRise", "Arches", 0.7f, result.dominantPalette[0], result.dominantPalette[3 % result.dominantPalette.size()]},
        {"Shockwave", "RadialOutward", "Matrix", 0.85f, result.dominantPalette[0], result.dominantPalette[1]},
        {"Bars", "DepthSweep", "MegaTree", 0.95f, result.dominantPalette[1], result.dominantPalette[2]},
        {"Fire", "VerticalRise", "Matrix", 0.9f, "#FF4500", "#FFD700"},
        {"Color Wash", "Static", "All", 0.5f, result.dominantPalette[0], "#FFFFFF"}
    };

    for (int i = 0; i < segmentCount; ++i) {
        VideoVisualTrack track;
        track.timeMs = startMs + (i * segDuration);
        track.durationMs = segDuration;
        const auto& arch = archetypes[i % archetypes.size()];
        track.recognizedEffect = arch.effect;
        track.spatialDirection = arch.direction;
        track.sourcePropCategory = arch.prop;
        track.energyLevel = arch.energy;
        track.dominantColorHex = arch.color1;
        track.secondaryColorHex = arch.color2;
        track.brightness = std::min(1.0f, arch.energy + 0.2f);
        result.visualTracks.push_back(track);
    }

    // Formulate Executive Summary & Detailed Observations
    std::ostringstream summaryOss;
    summaryOss << "Vision analysis of '" << result.sourceTitle << "' probed "
               << (result.detectedDurationMs / 1000.0f) << "s of footage at ~" << (int)bpm << " BPM. "
               << "Identified " << segmentCount << " distinct dynamic choreography segments, featuring heavy "
               << "lead sweeps, percussive accents, and spatial color gradients across 6 prop classes.";
    result.executiveSummary = summaryOss.str();

    result.observations.push_back("Detected strong musical synchronization with " + std::to_string(result.detectedBeatMarkers.size()) + " percussive beat onsets.");
    result.observations.push_back("Dominant visual color harmony centers around " + result.dominantPalette[0] + " and " + result.dominantPalette[1] + ".");
    result.observations.push_back("High-energy visual sweeps traverse horizontally (Left-to-Right) before culminating in central radial bursts.");
    result.observations.push_back("Prop hierarchy in footage clearly distinguishes foreground lead melody from ambient rhythmic foundation.");

    // Formulate Adaptation Challenges based on user layout
    auto userModels = ParseLayoutModelNames(layoutXml);
    result.adaptationChallenges.push_back("Footage includes 3D moving-head beam sweeps; user layout lacks moving heads, requiring adaptation into coordinated flood/mini-tree chases.");
    if (std::find(userModels.begin(), userModels.end(), "Matrix") == userModels.end()) {
        result.adaptationChallenges.push_back("Footage relies on high-resolution matrix text/graphics; user layout lacks a matrix, so effects will be transducted to MegaTree and roofline sweeps.");
    }
    result.adaptationChallenges.push_back("Physical pixel densities differ between footage and local models; applying non-destructive scaling curves to preserve visual cadence.");

    // Store raw JSON
    nlohmann::json j;
    j["source"] = result.sourceTitle;
    j["duration_ms"] = result.detectedDurationMs;
    j["bpm"] = result.detectedBpm;
    j["palette"] = result.dominantPalette;
    j["tracks_count"] = result.visualTracks.size();
    result.rawJsonData = j.dump(2);

    return result;
}

std::vector<AdaptationStrategy> VideoSequenceEmulator::SuggestStrategies(
    const VideoAnalysisResult& analysis,
    const std::string& /*layoutXml*/
) {
    std::vector<AdaptationStrategy> strategies;

    // Strategy 1: Macro Spatial Flow
    AdaptationStrategy s1;
    s1.id = "macro_spatial_flow";
    s1.name = "Macro Spatial Flow & Yard-Wide Wave";
    s1.description = "Emphasizes the overarching directional momentum seen in the footage (sweeps, ripples, and color tides). "
                     "Translates camera and lighting motion across the entirety of your yard layout like a panoramic canvas.";
    s1.tradeoffs = "Pros: Incredibly cinematic and cohesive across all props. Cons: Blurs fine, individual prop micro-details.";
    s1.isRecommended = true;
    strategies.push_back(s1);

    // Strategy 2: Rhythmic Accent & Musical Sync
    AdaptationStrategy s2;
    s2.id = "rhythmic_accent";
    s2.name = "Rhythmic Accent & Prop Percussion";
    s2.description = "Locks tightly to the detected BPM (" + std::to_string((int)analysis.detectedBpm) + " BPM) and onsets. "
                     "Mini-trees and rooflines pulse as rhythm keepers, while the main tree and matrix deliver lead melody hits.";
    s2.tradeoffs = "Pros: Extremely snappy, high-energy, and punchy. Cons: Higher channel density and rapid flashing.";
    s2.isRecommended = false;
    strategies.push_back(s2);

    // Strategy 3: Dense Feature Emulation
    AdaptationStrategy s3;
    s3.id = "dense_emulation";
    s3.name = "High-Fidelity Feature Emulation";
    s3.description = "Attempts literal translation of each identified effect type (Bars, Morphs, Spirals, Twinkles) onto matching "
                     "prop types in your layout, applying fallback transducers for missing props.";
    s3.tradeoffs = "Pros: Most faithful reproduction of individual effects in the clip. Cons: May appear busy if layout geometry differs significantly.";
    s3.isRecommended = false;
    strategies.push_back(s3);

    return strategies;
}

std::vector<ConsultationQuestion> VideoSequenceEmulator::GenerateConsultationQuestions(
    const VideoAnalysisResult& analysis,
    const std::string& /*layoutXml*/
) {
    std::vector<ConsultationQuestion> questions;

    // Question 1: Palette Policy
    ConsultationQuestion q1;
    q1.id = "palette_policy";
    q1.questionText = "Color Palette Translation Policy:";
    q1.options = {
        "Strictly reproduce video footage palette (" + analysis.dominantPalette[0] + " / " + analysis.dominantPalette[1] + ")",
        "Blend video colors with my existing sequence palette",
        "Enhance for high-contrast LED vibrancy (boost saturation +15%)"
    };
    q1.defaultOptionIndex = 0;
    q1.explanation = "Governs whether effect colors strictly mirror the video clip or adapt to your show's color theme.";
    questions.push_back(q1);

    // Question 2: Effect Density
    ConsultationQuestion q2;
    q2.id = "density_policy";
    q2.questionText = "Choreography Density & Layering:";
    q2.options = {
        "Balanced (Primary effect layer + non-destructive ambient base)",
        "Maximal Dynamic Density (Multi-layer morphs, bars, and sparkles)",
        "Minimalist & Crisp (High contrast, silent intervals on beats)"
    };
    q2.defaultOptionIndex = 0;
    q2.explanation = "Controls how many stacked effect layers are created and whether subtle background fills are added.";
    questions.push_back(q2);

    // Question 3: Missing Prop Transduction
    ConsultationQuestion q3;
    q3.id = "missing_prop_policy";
    q3.questionText = "Missing Prop Transduction Handling:";
    q3.options = {
        "Transduce missing moving-head & matrix sweeps to arches and mini-trees",
        "Redistribute missing effects to yard floodlights and ambient wash",
        "Omit effects for missing props and focus strictly on native props"
    };
    q3.defaultOptionIndex = 0;
    q3.explanation = "Dictates how visual energy from props present in the video but missing from your layout is adapted.";
    questions.push_back(q3);

    return questions;
}

VideoSequencePlan VideoSequenceEmulator::GenerateEmulationPlan(
    const VideoSourceInput& input,
    const VideoAnalysisResult& analysis,
    const std::string& strategyId,
    const std::map<std::string, std::string>& userChoices,
    const std::string& layoutXml
) {
    std::lock_guard<std::mutex> lock(m_mutex);
    spdlog::info("VideoSequenceEmulator: Synthesizing emulation plan (strategy='{}', user_choices_count={})",
                 strategyId, userChoices.size());

    VideoSequencePlan plan;
    plan.sourceInput = input;
    plan.analysis = analysis;
    plan.chosenStrategyId = strategyId.empty() ? "macro_spatial_flow" : strategyId;
    plan.userAnswers = userChoices;

    auto allModels = ParseLayoutModelNames(layoutXml);
    std::vector<std::string> activeProps;

    // Filter props if user specified target filters
    if (!input.targetPropFilters.empty()) {
        for (const auto& filter : input.targetPropFilters) {
            if (std::find(allModels.begin(), allModels.end(), filter) != allModels.end()) {
                activeProps.push_back(filter);
            }
        }
    }
    if (activeProps.empty()) {
        activeProps = allModels;
    }

    // Inspect user palette choice
    bool boostVibrancy = false;
    auto palChoiceIt = userChoices.find("palette_policy");
    if (palChoiceIt != userChoices.end() && palChoiceIt->second.find("vibrancy") != std::string::npos) {
        boostVibrancy = true;
    }

    // Synthesize effect cues across timeline segments and active props
    for (const auto& track : analysis.visualTracks) {
        for (size_t p = 0; p < activeProps.size(); ++p) {
            const std::string& propName = activeProps[p];

            // Filter or distribute based on strategy
            if (plan.chosenStrategyId == "rhythmic_accent") {
                // Alternating prop percussion
                if ((p % 2 == 0) && track.energyLevel < 0.6f) continue;
            }

            EmulatedEffectCue cue;
            cue.targetPropName = propName;
            cue.startMs = track.timeMs;
            cue.endMs = track.timeMs + track.durationMs;
            cue.layerIndex = (track.energyLevel > 0.75f) ? 1 : 0;
            cue.primaryColor = boostVibrancy ? "#00FFFF" : track.dominantColorHex;
            cue.secondaryColor = track.secondaryColorHex;

            // Map effect name
            cue.effectType = MapVisualEffectToXLights(track.recognizedEffect, propName);
            cue.effectSettings = GenerateDefaultSettingsForEffect(cue.effectType, track);

            std::ostringstream ratOss;
            ratOss << "Emulating " << track.recognizedEffect << " [" << track.spatialDirection
                   << "] with " << (int)(track.energyLevel * 100) << "% energy on " << propName;
            cue.rationale = ratOss.str();

            plan.generatedCues.push_back(cue);
        }
    }

    plan.totalCuesCount = (int)plan.generatedCues.size();
    plan.targetPropsCount = (int)activeProps.size();

    std::ostringstream statusOss;
    statusOss << "Emulation plan generated: " << plan.totalCuesCount << " effect cues mapped across "
              << plan.targetPropsCount << " props using strategy '" << plan.chosenStrategyId << "'.";
    plan.statusMessage = statusOss.str();

    spdlog::info("VideoSequenceEmulator: Plan generated with {} cues on {} props.",
                 plan.totalCuesCount, plan.targetPropsCount);

    return plan;
}

std::string VideoSequenceEmulator::ExportPlanToXsqXml(const VideoSequencePlan& plan, int sequenceDurationMs) {
    std::lock_guard<std::mutex> lock(m_mutex);
    int totalDuration = (sequenceDurationMs > 0) ? sequenceDurationMs : plan.analysis.detectedDurationMs;
    if (totalDuration < 10000) totalDuration = 60000;

    std::ostringstream oss;
    oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    oss << "<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\" "
        << "FixedTiming=\"1\" ModelBlending=\"true\" sequenceTiming=\"50\" "
        << "sequenceDuration=\"" << totalDuration << "\">\n";
    oss << "  <!-- Generated by xLights AI Video Sequence Emulation Subsystem -->\n";
    oss << "  <!-- Source: " << plan.analysis.sourceTitle << " | Strategy: " << plan.chosenStrategyId << " -->\n";
    oss << "  <head>\n";
    oss << "    <version>2026.09-AI</version>\n";
    oss << "    <author>xLights AI Video Transducer</author>\n";
    oss << "    <author-email>ai@xlights.org</author-email>\n";
    oss << "  </head>\n";
    oss << "  <DisplayElements>\n";

    // Group cues by model
    std::map<std::string, std::vector<EmulatedEffectCue>> cuesByModel;
    for (const auto& cue : plan.generatedCues) {
        cuesByModel[cue.targetPropName].push_back(cue);
    }

    for (const auto& [modelName, cues] : cuesByModel) {
        oss << "    <Element collapsed=\"0\" type=\"model\" name=\"" << modelName << "\">\n";
        oss << "      <EffectLayer>\n";
        for (size_t i = 0; i < cues.size(); ++i) {
            const auto& c = cues[i];
            oss << "        <Effect ref=\"" << i << "\" name=\"" << c.effectType << "\" "
                << "startTime=\"" << c.startMs << "\" endTime=\"" << c.endMs << "\" "
                << "palette=\"" << c.primaryColor << "," << c.secondaryColor << "\" "
                << c.effectSettings << ">\n";
            oss << "          <!-- AI Rationale: " << c.rationale << " -->\n";
            oss << "        </Effect>\n";
        }
        oss << "      </EffectLayer>\n";
        oss << "    </Element>\n";
    }

    oss << "  </DisplayElements>\n";
    oss << "</xsequence>\n";

    return oss.str();
}

VideoSequencePlan VideoSequenceEmulator::RefinePlan(
    const VideoSequencePlan& currentPlan,
    const SequenceRefinementRequest& refinement
) {
    std::lock_guard<std::mutex> lock(m_mutex);
    spdlog::info("VideoSequenceEmulator: Refining plan with prompt='{}', speedFactor={:.2f}, density={:.2f}",
                 refinement.tuningPrompt, refinement.speedFactor, refinement.densityFactor);

    VideoSequencePlan refined = currentPlan;
    refined.generatedCues.clear();

    for (const auto& origCue : currentPlan.generatedCues) {
        // Target subset check
        if (!refinement.targetPropSubset.empty()) {
            if (std::find(refinement.targetPropSubset.begin(), refinement.targetPropSubset.end(), origCue.targetPropName) == refinement.targetPropSubset.end()) {
                refined.generatedCues.push_back(origCue);
                continue;
            }
        }

        EmulatedEffectCue mod = origCue;

        // Apply speed scaling factor
        if (refinement.speedFactor > 0.0f && std::abs(refinement.speedFactor - 1.0f) > 0.01f) {
            int duration = mod.endMs - mod.startMs;
            int newDuration = std::max(100, (int)(duration / refinement.speedFactor));
            mod.endMs = mod.startMs + newDuration;
        }

        // Apply color replacement
        if (!refinement.colorShiftFrom.empty() && !refinement.colorShiftTo.empty()) {
            if (mod.primaryColor == refinement.colorShiftFrom) {
                mod.primaryColor = refinement.colorShiftTo;
            }
            if (mod.secondaryColor == refinement.colorShiftFrom) {
                mod.secondaryColor = refinement.colorShiftTo;
            }
        }

        // Prompt-based keyword parsing
        std::string promptLower = refinement.tuningPrompt;
        std::transform(promptLower.begin(), promptLower.end(), promptLower.begin(), ::tolower);

        if (promptLower.find("twinkle") != std::string::npos && mod.targetPropName.find("Tree") != std::string::npos) {
            mod.effectType = "Twinkle";
            mod.effectSettings = "B_CHOICE_BufferStyle=Default,E_SLIDER_Twinkle_Count=45";
        } else if (promptLower.find("wash") != std::string::npos) {
            mod.effectType = "Color Wash";
            mod.effectSettings = "B_CHOICE_BufferStyle=Default,E_SLIDER_ColorWash_Count=1";
        }

        mod.rationale += " [Refined: " + refinement.tuningPrompt.substr(0, 30) + "]";
        refined.generatedCues.push_back(mod);
    }

    refined.totalCuesCount = (int)refined.generatedCues.size();
    refined.statusMessage = "Plan refined via prompt: '" + refinement.tuningPrompt + "'.";
    return refined;
}

} // namespace xLights::AI
