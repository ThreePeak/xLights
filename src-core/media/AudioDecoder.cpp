/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "media/AudioDecoder.h"
#include "media/AudioManager.h"
#include "media/FFmpegAudioDecoder.h"
#ifdef __APPLE__
#include "media/AudioToolboxDecoder.h"
#endif

#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

AudioDecoder::AudioDecoder() {
#ifdef __APPLE__
    _underlyingDecoder = std::make_unique<AudioToolboxDecoder>();
#else
    _underlyingDecoder = std::make_unique<FFmpegAudioDecoder>();
#endif
}

bool AudioDecoder::DecodeFile(const std::string& path,
                               long targetRate,
                               int extra,
                               DecodedAudioInfo& info,
                               uint8_t*& pcmData, long& pcmDataSize,
                               float*& leftData, float*& rightData,
                               long& trackSize,
                               std::function<void(int pct)> progress) {
    if (!_underlyingDecoder) return false;
    return _underlyingDecoder->DecodeFile(path, targetRate, extra, info, pcmData, pcmDataSize, leftData, rightData, trackSize, progress);
}

bool AudioDecoder::EncodeToFile(const std::vector<float>& left,
                                const std::vector<float>& right,
                                size_t sampleRate,
                                const std::string& filename) {
    if (!_underlyingDecoder) return false;
    return _underlyingDecoder->EncodeToFile(left, right, sampleRate, filename);
}

size_t AudioDecoder::GetAudioFileLength(const std::string& filename) {
    if (!_underlyingDecoder) return 0;
    return _underlyingDecoder->GetAudioFileLength(filename);
}

DemucsStemResult AudioDecoder::SeparateDemucsStemsONNX(AudioManager* audioManager,
                                                        const std::string& onnxModelPath,
                                                        const std::string& outputFolder,
                                                        std::function<void(int pct)> progress,
                                                        const std::atomic<bool>* cancel) {
    DemucsStemResult result;
    if (!audioManager) {
        result.errorMessage = "Null AudioManager provided.";
        return result;
    }

    spdlog::info("AudioDecoder: Starting Demucs ONNX stem separation with model: {}", onnxModelPath);

    StemSeparatorOptions options;
    bool success = SeparateStems(audioManager, onnxModelPath, result.stemBuffers, options, progress, cancel);

    if (!success) {
        result.errorMessage = "Demucs ONNX stem separation failed or was cancelled.";
        spdlog::error("AudioDecoder: {}", result.errorMessage);
        return result;
    }

    // Save stem WAV files
    std::string baseDir = outputFolder.empty() ? "." : outputFolder;
    result.vocalStemPath = baseDir + "/stem_vocals.wav";
    result.drumStemPath  = baseDir + "/stem_drums.wav";
    result.bassStemPath  = baseDir + "/stem_bass.wav";
    result.otherStemPath = baseDir + "/stem_other.wav";

    size_t rate = result.stemBuffers.sampleRate > 0 ? (size_t)result.stemBuffers.sampleRate : 44100;

    EncodeToFile(result.stemBuffers.vocalsL, result.stemBuffers.vocalsR, rate, result.vocalStemPath);
    EncodeToFile(result.stemBuffers.drumsL,  result.stemBuffers.drumsR,  rate, result.drumStemPath);
    EncodeToFile(result.stemBuffers.bassL,   result.stemBuffers.bassR,   rate, result.bassStemPath);
    EncodeToFile(result.stemBuffers.otherL,  result.stemBuffers.otherR,  rate, result.otherStemPath);

    result.success = true;
    spdlog::info("AudioDecoder: Demucs stem separation successfully completed. Stems written to {}", baseDir);
    return result;
}

std::vector<StemTimingTrackResult> AudioDecoder::GenerateStemTimingTracks(const StemOutput& stems,
                                                                           long framePeriodMS) {
    std::vector<StemTimingTrackResult> results;
    if (stems.sampleRate <= 0 || stems.drumsL.empty()) return results;

    long sampleRate = stems.sampleRate;
    long samplesPerFrame = (sampleRate * framePeriodMS) / 1000;
    if (samplesPerFrame <= 0) samplesPerFrame = 2205; // ~50ms @ 44.1k

    // 1. Drum Beat Timing Track
    {
        StemTimingTrackResult drumTrack;
        drumTrack.trackName = "AI Stems - Drums & Onsets";
        size_t totalSamples = stems.drumsL.size();
        float prevEnergy = 0.0f;

        for (size_t i = 0; i < totalSamples; i += samplesPerFrame) {
            float energy = 0.0f;
            size_t endIdx = std::min(totalSamples, i + samplesPerFrame);
            for (size_t j = i; j < endIdx; ++j) {
                float val = 0.5f * (std::abs(stems.drumsL[j]) + std::abs(stems.drumsR[j]));
                energy += val * val;
            }
            energy = std::sqrt(energy / (endIdx - i));

            if (energy > 0.15f && energy > prevEnergy * 1.8f) {
                StemTimingMark mark;
                mark.timeMS = (long)((i * 1000) / sampleRate);
                mark.label = "Beat";
                mark.confidence = std::min(1.0f, energy);
                drumTrack.marks.push_back(mark);
            }
            prevEnergy = energy;
        }
        results.push_back(drumTrack);
    }

    // 2. Vocal Timing Track
    {
        StemTimingTrackResult vocalTrack;
        vocalTrack.trackName = "AI Stems - Vocals";
        size_t totalSamples = stems.vocalsL.size();
        bool inVocalPhrase = false;

        for (size_t i = 0; i < totalSamples; i += samplesPerFrame) {
            float energy = 0.0f;
            size_t endIdx = std::min(totalSamples, i + samplesPerFrame);
            for (size_t j = i; j < endIdx; ++j) {
                float val = 0.5f * (std::abs(stems.vocalsL[j]) + std::abs(stems.vocalsR[j]));
                energy += val * val;
            }
            energy = std::sqrt(energy / (endIdx - i));

            if (energy > 0.08f && !inVocalPhrase) {
                inVocalPhrase = true;
                StemTimingMark mark;
                mark.timeMS = (long)((i * 1000) / sampleRate);
                mark.label = "Vocal Start";
                mark.confidence = std::min(1.0f, energy);
                vocalTrack.marks.push_back(mark);
            } else if (energy <= 0.04f && inVocalPhrase) {
                inVocalPhrase = false;
                StemTimingMark mark;
                mark.timeMS = (long)((i * 1000) / sampleRate);
                mark.label = "Vocal End";
                mark.confidence = 0.8f;
                vocalTrack.marks.push_back(mark);
            }
        }
        results.push_back(vocalTrack);
    }

    return results;
}

std::vector<float> AudioDecoder::ComputeRMSEnvelope(const std::vector<float>& channelL,
                                                   const std::vector<float>& channelR,
                                                   long sampleRate,
                                                   int framePeriodMS) {
    std::vector<float> envelope;
    if (sampleRate <= 0 || channelL.empty()) return envelope;

    long samplesPerFrame = (sampleRate * framePeriodMS) / 1000;
    if (samplesPerFrame <= 0) samplesPerFrame = 2205;
    size_t totalSamples = channelL.size();

    envelope.reserve(totalSamples / samplesPerFrame + 1);

    for (size_t i = 0; i < totalSamples; i += samplesPerFrame) {
        size_t endIdx = std::min(totalSamples, i + samplesPerFrame);
        float sumSq = 0.0f;
        for (size_t j = i; j < endIdx; ++j) {
            float rVal = (j < channelR.size()) ? channelR[j] : channelL[j];
            float mono = 0.5f * (std::abs(channelL[j]) + std::abs(rVal));
            sumSq += mono * mono;
        }
        float rms = std::sqrt(sumSq / (endIdx - i));
        envelope.push_back(rms);
    }

    return envelope;
}

std::vector<StemTimingMark> AudioDecoder::ComputeSpectralFluxOnsets(const std::vector<float>& channelL,
                                                                    const std::vector<float>& channelR,
                                                                    long sampleRate,
                                                                    int framePeriodMS,
                                                                    float thresholdMultiplier) {
    std::vector<StemTimingMark> marks;
    std::vector<float> envelope = ComputeRMSEnvelope(channelL, channelR, sampleRate, framePeriodMS);
    if (envelope.empty()) return marks;

    float prevEnergy = 0.0f;
    for (size_t frame = 0; frame < envelope.size(); ++frame) {
        float energy = envelope[frame];
        float flux = energy - prevEnergy;
        if (energy > 0.12f && flux > 0.0f && energy > prevEnergy * thresholdMultiplier) {
            StemTimingMark mark;
            mark.timeMS = (long)(frame * framePeriodMS);
            mark.label = "Onset";
            mark.confidence = std::min(1.0f, energy);
            marks.push_back(mark);
        }
        prevEnergy = energy;
    }

    return marks;
}

DemucsStemResult AudioDecoder::ProcessAudioFileStemSeparation(AudioManager* audioManager,
                                                              const std::string& onnxModelPath,
                                                              const std::string& outputFolder,
                                                              std::vector<StemTimingTrackResult>& outTimingTracks,
                                                              std::function<void(int pct)> progress,
                                                              const std::atomic<bool>* cancel) {
    spdlog::info("AudioDecoder: Executing integrated audio import stem separation workflow...");
    DemucsStemResult result = SeparateDemucsStemsONNX(audioManager, onnxModelPath, outputFolder, progress, cancel);
    if (result.success) {
        outTimingTracks = GenerateStemTimingTracks(result.stemBuffers);
        spdlog::info("AudioDecoder: Generated {} timing tracks from separated audio stems.", outTimingTracks.size());
    }
    return result;
}
