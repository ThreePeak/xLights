#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "IAudioDecoder.h"
#include "StemSeparator.h"
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class AudioManager;

struct DemucsStemResult {
    bool success = false;
    std::string vocalStemPath;
    std::string drumStemPath;
    std::string bassStemPath;
    std::string otherStemPath;
    StemOutput stemBuffers;
    std::string errorMessage;
};

struct StemTimingMark {
    long timeMS = 0;
    std::string label;
    float confidence = 1.0f;
};

struct StemTimingTrackResult {
    std::string trackName;
    std::vector<StemTimingMark> marks;
};

class AudioDecoder : public IAudioDecoder {
public:
    AudioDecoder();
    ~AudioDecoder() override = default;

    // Decode an audio file into 16-bit stereo PCM and float channel buffers
    bool DecodeFile(const std::string& path,
                    long targetRate,
                    int extra,
                    DecodedAudioInfo& info,
                    uint8_t*& pcmData, long& pcmDataSize,
                    float*& leftData, float*& rightData,
                    long& trackSize,
                    std::function<void(int pct)> progress = nullptr) override;

    // Encode float PCM to an audio file (.wav, .m4a, etc.)
    bool EncodeToFile(const std::vector<float>& left,
                      const std::vector<float>& right,
                      size_t sampleRate,
                      const std::string& filename) override;

    // Get byte length of audio file without full decode
    size_t GetAudioFileLength(const std::string& filename) override;

    // -------------------------------------------------------------------------
    // Demucs ONNX Model Stem Separation & Timing Tracks Integration
    // -------------------------------------------------------------------------
    
    // Separate audio into 4 stems (Vocals, Drums, Bass, Other) using ONNX model
    DemucsStemResult SeparateDemucsStemsONNX(AudioManager* audioManager,
                                              const std::string& onnxModelPath,
                                              const std::string& outputFolder,
                                              std::function<void(int pct)> progress = nullptr,
                                              const std::atomic<bool>* cancel = nullptr);

    // Generate timing tracks directly from separated stems
    std::vector<StemTimingTrackResult> GenerateStemTimingTracks(const StemOutput& stems,
                                                                 long framePeriodMS = 50);

private:
    std::unique_ptr<IAudioDecoder> _underlyingDecoder;
};
