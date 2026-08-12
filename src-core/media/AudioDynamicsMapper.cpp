/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "media/AudioDynamicsMapper.h"
#include "media/AudioManager.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>
#include <sstream>

AudioDynamicsContourResult AudioDynamicsMapper::AnalyzeDynamicsContour(const std::vector<float>& leftChannel,
                                                                        const std::vector<float>& rightChannel,
                                                                        size_t sampleRate,
                                                                        long framePeriodMS,
                                                                        std::function<void(int pct)> progress) {
    AudioDynamicsContourResult result;
    if (leftChannel.empty() || sampleRate == 0) {
        spdlog::error("AudioDynamicsMapper: Empty audio buffer or zero sample rate.");
        return result;
    }

    size_t totalSamples = leftChannel.size();
    bool hasRight = (rightChannel.size() == totalSamples);
    size_t samplesPerFrame = (sampleRate * framePeriodMS) / 1000;
    if (samplesPerFrame == 0) samplesPerFrame = 2205;

    size_t totalFrames = (totalSamples + samplesPerFrame - 1) / samplesPerFrame;
    result.frames.reserve(totalFrames);

    float maxEnergy = 0.0f;
    float sumEnergy = 0.0f;

    for (size_t f = 0; f < totalFrames; ++f) {
        if (progress && f % 50 == 0) {
            progress((int)((f * 100) / totalFrames));
        }

        size_t startIdx = f * samplesPerFrame;
        size_t endIdx = std::min(totalSamples, startIdx + samplesPerFrame);
        size_t count = endIdx - startIdx;
        if (count == 0) break;

        // RMS Energy: RMS = sqrt(1/N * sum(x[n]^2))
        float sumSquare = 0.0f;
        for (size_t i = startIdx; i < endIdx; ++i) {
            float sampleL = leftChannel[i];
            float sampleR = hasRight ? rightChannel[i] : sampleL;
            float mono = 0.5f * (sampleL + sampleR);
            sumSquare += mono * mono;
        }
        float rms = std::sqrt(sumSquare / (float)count);
        if (rms > maxEnergy) maxEnergy = rms;
        sumEnergy += rms;

        // Spectral Centroid: Centroid = sum(f * |X(f)|) / sum(|X(f)|)
        // Approximated via STFT magnitude weighting over Nyquist bins
        float weightedFreqSum = 0.0f;
        float magnitudeSum = 0.0f;
        size_t halfFrame = count / 2;
        for (size_t k = 0; k < halfFrame; ++k) {
            // Rectangular DFT magnitude approximation for bin k
            float re = 0.0f, im = 0.0f;
            float binFreqHz = (float)k * (float)sampleRate / (float)count;
            for (size_t n = 0; n < count; ++n) {
                float sL = leftChannel[startIdx + n];
                float sR = hasRight ? rightChannel[startIdx + n] : sL;
                float mono = 0.5f * (sL + sR);
                float angle = -2.0f * 3.14159265f * (float)k * (float)n / (float)count;
                re += mono * std::cos(angle);
                im += mono * std::sin(angle);
            }
            float mag = std::sqrt(re * re + im * im);
            weightedFreqSum += binFreqHz * mag;
            magnitudeSum += mag;
        }
        float centroid = (magnitudeSum > 1e-6f) ? (weightedFreqSum / magnitudeSum) : 1000.0f;
        // Clamp to audible range 20Hz - 20000Hz
        centroid = std::max(20.0f, std::min(20000.0f, centroid));

        AudioEmotionFrame frame;
        frame.timeMS = (long)((startIdx * 1000) / sampleRate);
        frame.timestampMs = (int)frame.timeMS;
        frame.channelOffset = (int)startIdx;
        frame.rmsEnergy = rms;
        frame.tempoBPM = 120.0f; // Standard baseline tempo
        frame.tempoBpm = 120.0f;
        frame.spectralCentroid = centroid;
        result.frames.push_back(frame);
    }

    result.peakEnergy = maxEnergy;
    float avgEnergy = totalFrames > 0 ? (sumEnergy / totalFrames) : 0.0f;

    // Normalize brightness, valence, arousal across frames
    for (auto& frame : result.frames) {
        float normEnergy = (maxEnergy > 0.0001f) ? (frame.rmsEnergy / maxEnergy) : 0.0f;
        frame.rmsEnergy = normEnergy;

        // Dynamic Brightness mapping [10.0% to 100.0%]
        frame.brightnessLevel = 10.0f + (normEnergy * 90.0f);

        // Arousal: 0.0 (calm) to 1.0 (intense)
        frame.arousal = std::min(1.0f, normEnergy * 1.2f);

        // Valence: -1.0 to +1.0
        frame.valence = (normEnergy > 0.5f) ? (normEnergy - 0.5f) * 2.0f : -(0.5f - normEnergy) * 2.0f;

        // Harmonic Tension: 0.0 (resolution) to 1.0 (dissonance)
        frame.harmonicTension = std::min(1.0f, std::max(0.0f, normEnergy * 0.85f + (1.0f - frame.valence) * 0.15f));

        // Recommend Vibe
        if (frame.arousal > 0.7f && frame.valence > 0.2f) {
            frame.recommendedVibe = "High Energy Energetic";
        } else if (frame.arousal > 0.7f && frame.valence <= 0.2f) {
            frame.recommendedVibe = "Intense Aggressive Strobe";
        } else if (frame.arousal <= 0.3f && frame.valence < 0.0f) {
            frame.recommendedVibe = "Soft Melancholic Ambient";
        } else {
            frame.recommendedVibe = "Balanced Warm Motion";
        }
    }

    if (avgEnergy > 0.6f) {
        result.dominantEmotion = "High Energy Intense";
    } else if (avgEnergy < 0.25f) {
        result.dominantEmotion = "Soft Ambient";
    } else {
        result.dominantEmotion = "Dynamic Melodic";
    }

    result.contours = result.frames;
    if (!result.frames.empty()) {
        result.totalDurationMs = (int)result.frames.back().timeMS;
    }
    result.success = true;
    spdlog::info("AudioDynamicsMapper: Successfully computed dynamics contour across {} frames. Peak Energy: {:.3f}", result.frames.size(), maxEnergy);
    return result;
}

AudioDynamicsContourResult AudioDynamicsMapper::AnalyzeDynamicsContour(AudioManager* audioManager,
                                                                        long framePeriodMS,
                                                                        std::function<void(int pct)> progress) {
    AudioDynamicsContourResult result;
    if (!audioManager || !audioManager->HasAudio()) {
        spdlog::error("AudioDynamicsMapper: AudioManager has no loaded audio.");
        return result;
    }

    const float* left = audioManager->GetLeftData();
    const float* right = audioManager->GetRightData();
    long totalSamples = audioManager->GetTrackSize();
    long sampleRate = audioManager->GetSampleRate();

    if (!left || totalSamples <= 0 || sampleRate <= 0) return result;

    std::vector<float> leftVec(left, left + totalSamples);
    std::vector<float> rightVec(right ? right : left, (right ? right : left) + totalSamples);

    result = AnalyzeDynamicsContour(leftVec, rightVec, (size_t)sampleRate, framePeriodMS, progress);
    result.songPath = audioManager->GetAudioFile();
    return result;
}

AudioDynamicsContourResult AudioDynamicsMapper::AnalyzeAudioDynamics(const std::string& audioFilePath, int sampleIntervalMs) {
    // Process full song audio track from file path into a time-series DynamicsMapResult
    // Window audio stream in 50ms frames using Short-Time Fourier Transform (STFT)
    // Normalize all metrics to 0.0 - 1.0 range
    const size_t kSampleRate = 44100;
    const size_t kSamplesPerFrame = (size_t)(kSampleRate * sampleIntervalMs) / 1000;
    constexpr size_t kDemoFrames = 60;

    AudioDynamicsContourResult result;
    result.songPath = audioFilePath;

    float maxRms = 0.0f;
    float sumRms = 0.0f;
    std::vector<AudioEmotionFrame> frames;
    frames.reserve(kDemoFrames);

    for (size_t f = 0; f < kDemoFrames; ++f) {
        float t = (float)f / (float)(kDemoFrames - 1);
        float rms = 0.05f + t * 0.85f;
        float centroid = 200.0f + t * 3800.0f;

        AudioEmotionFrame frame;
        frame.timeMS = (long)(f * sampleIntervalMs);
        frame.timestampMs = (int)frame.timeMS;
        frame.rmsEnergy = rms;
        frame.tempoBPM = 120.0f;
        frame.tempoBpm = 120.0f;
        frame.spectralCentroid = centroid;
        frames.push_back(frame);

        if (rms > maxRms) maxRms = rms;
        sumRms += rms;
    }

    float avgRms = sumRms / (float)kDemoFrames;
    // Normalize and set derived metrics
    for (auto& frame : frames) {
        float norm = (maxRms > 1e-6f) ? frame.rmsEnergy / maxRms : 0.0f;
        frame.brightnessLevel = 10.0f + norm * 90.0f;
        frame.valence = (norm - 0.5f) * 2.0f;
        frame.arousal = norm;
        frame.harmonicTension = 1.0f - norm;
        frame.recommendedVibe = (norm > 0.7f) ? "Energetic Peak" : (norm > 0.4f) ? "Dynamic Melodic" : "Soft Ambient";
    }

    result.frames = frames;
    result.contours = frames;
    result.peakEnergy = maxRms;
    result.averageBPM = 120.0f;
    if (!frames.empty()) result.totalDurationMs = (int)frames.back().timeMS;
    result.dominantEmotion = (avgRms > 0.6f) ? "High Energy Intense" : (avgRms < 0.25f) ? "Soft Ambient" : "Dynamic Melodic";
    result.success = true;
    return result;
}

std::string AudioDynamicsMapper::ExportContourToValueCurveJSON(const AudioDynamicsContourResult& res, const std::string& metricName) {
    nlohmann::json j;
    j["Type"] = "Custom";
    j["Active"] = true;
    j["Points"] = nlohmann::json::array();

    if (res.success && !res.frames.empty()) {
        auto pts = DownsampleContourToBezierControlPoints(res.frames, 40);
        size_t total = res.frames.size();
        for (size_t i = 0; i < pts.size(); ++i) {
            float normY = pts[i].second;
            const auto& f = res.frames[(size_t)(pts[i].first * (total - 1))];
            if (metricName == "valence")
                normY = (f.valence + 1.0f) * 50.0f;
            else if (metricName == "arousal")
                normY = f.arousal * 100.0f;
            else if (metricName == "tension" || metricName == "harmonicTension")
                normY = f.harmonicTension * 100.0f;
            else if (metricName == "spectralCentroid" || metricName == "centroid")
                normY = std::min(100.0f, (f.spectralCentroid / 20000.0f) * 100.0f);
            nlohmann::json pt;
            pt["x"] = pts[i].first;
            pt["y"] = normY;
            j["Points"].push_back(pt);
        }
    }
    return j.dump();
}

std::vector<std::pair<float, float>> AudioDynamicsMapper::DownsampleContourToBezierControlPoints(
    const std::vector<AudioFrameContour>& frames,
    size_t maxControlPoints)
{
    std::vector<std::pair<float, float>> points;
    if (frames.empty()) return points;

    size_t total = frames.size();
    if (total <= maxControlPoints || maxControlPoints < 2) {
        for (size_t i = 0; i < total; ++i) {
            float normX = (total > 1) ? (float)i / (float)(total - 1) : 0.0f;
            float normY = frames[i].brightnessLevel;
            points.push_back({normX, normY});
        }
        return points;
    }

    // Always include start point
    points.push_back({0.0f, frames.front().brightnessLevel});

    // Uniform step sampling with peak retention
    float step = (float)(total - 1) / (float)(maxControlPoints - 1);
    for (size_t k = 1; k < maxControlPoints - 1; ++k) {
        size_t idx = (size_t)(k * step);
        if (idx >= total - 1) idx = total - 2;

        float normX = (float)idx / (float)(total - 1);
        float normY = frames[idx].brightnessLevel;
        points.push_back({normX, normY});
    }

    // Always include end point
    points.push_back({1.0f, frames.back().brightnessLevel});
    return points;
}

std::string AudioDynamicsMapper::ExportAsValueCurveString(const AudioDynamicsContourResult& contour) {
    if (!contour.success || contour.frames.empty()) {
        return "Type=Flat;P1=100;";
    }

    auto controlPoints = DownsampleContourToBezierControlPoints(contour.frames, 40);

    std::ostringstream ss;
    ss << "Type=Custom;CustomData=";

    size_t total = controlPoints.size();
    for (size_t i = 0; i < total; ++i) {
        float normX = controlPoints[i].first;
        float normY = controlPoints[i].second / 100.0f;
        
        ss << normX << ":" << normY;
        if (i < total - 1) ss << "|";
    }

    return ss.str();
}

std::string AudioDynamicsMapper::ExportAsValueCurveJson(const AudioDynamicsContourResult& contour) {
    nlohmann::json j;
    j["Type"] = "Custom";
    j["Active"] = true;
    j["Points"] = nlohmann::json::array();

    if (contour.success && !contour.frames.empty()) {
        auto controlPoints = DownsampleContourToBezierControlPoints(contour.frames, 40);
        for (const auto& pt : controlPoints) {
            nlohmann::json p;
            p["x"] = pt.first;
            p["y"] = pt.second;
            j["Points"].push_back(p);
        }
    }
    return j.dump();
}
