#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "../src-core/ai/AISubsystemBase.h"
#include "../src-core/media/AudioDecoder.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <atomic>
#include <functional>
#include <nlohmann/json.hpp>

class AudioManager;

namespace xLights::AI {

enum class StemType {
    VOCALS,
    DRUMS,
    BASS,
    OTHER
};

struct AudioStem {
    StemType type = StemType::VOCALS;
    std::string stemName;                 // "Vocals", "Drums", "Bass", "Other"
    std::string wavFilePath;              // Path to exported WAV file
    std::vector<float> leftBuffer;
    std::vector<float> rightBuffer;
    long sampleRate = 44100;
};

struct StemSeparationOptions {
    std::string modelPath;                // Path to Demucs ONNX or CoreML model
    std::string outputDirectory;
    int framePeriodMS = 50;               // Window size for timing marks
    float transientSensitivity = 0.12f;
    bool exportWavFiles = true;
};

struct StemExtractionConfig {
    AudioManager* audioManager = nullptr;
    std::string modelPath;
    std::string outputDirectory;
    int framePeriodMS = 50;
    float transientSensitivity = 0.12f;
    bool exportWavFiles = true;
    std::function<void(int pct)> progress = nullptr;
    const std::atomic<bool>* cancel = nullptr;
};

struct StemExtractionResult {
    bool success = false;
    std::vector<AudioStem> stems;
    std::vector<StemTimingTrackResult> timingTracks;
    std::vector<std::string> generatedTimingTrackFiles;       // List of .xtiming XML files
    std::string errorMessage;

    [[nodiscard]] nlohmann::json ToJson() const;
};

/**
 * @brief AudioStemExtractor subsystem.
 * Handles deep learning HTDemucs source separation (Vocals, Drums, Bass, Other),
 * computes RMS energy envelopes, spectral flux onset detection, and generates timing tracks.
 */
class AudioStemExtractor : public AISubsystemBase {
public:
    AudioStemExtractor(ServiceManager* sm = nullptr);
    virtual ~AudioStemExtractor() override = default;

    // AISubsystemBase lifecycle implementation
    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override;
    virtual void Shutdown() override;
    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "AudioStemExtractor"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"htdemucs_stem_separation", "rms_energy_envelopes", "spectral_flux_onsets", "stem_timing_tracks"};
    }

    /**
     * @brief Extracts 4 stems and compiles .xtiming XML files using structured StemExtractionConfig.
     */
    [[nodiscard]] static StemExtractionResult ProcessAudioStems(const StemExtractionConfig& config);

    /**
     * @brief Extracts 4 stems (Vocals, Drums, Bass, Other) from audio manager and generates timing tracks.
     */
    [[nodiscard]] static StemExtractionResult ExtractStems(
        AudioManager* audioManager,
        const StemSeparationOptions& options,
        std::function<void(int pct)> progress = nullptr,
        const std::atomic<bool>* cancel = nullptr
    );

    /**
     * @brief Computes RMS energy envelope for a target AudioStem over framePeriodMS windows.
     */
    [[nodiscard]] static std::vector<float> ComputeRMSEnvelope(const AudioStem& stem, int framePeriodMS = 50);

    /**
     * @brief Computes spectral flux onsets for a target AudioStem.
     */
    [[nodiscard]] static std::vector<StemTimingMark> ComputeSpectralFluxOnsets(
        const AudioStem& stem,
        int framePeriodMS = 50,
        float transientSensitivity = 0.12f
    );

    /**
     * @brief Constructs xLights .xtiming XML files anchoring marks to detected transient timestamps (ms).
     */
    [[nodiscard]] static std::string CompileTimingTrackToXTimingXML(
        const StemTimingTrackResult& timingTrack,
        int markDurationMS = 50
    );

    /**
     * @brief Computes spectral flux onsets on raw PCM buffer and generates formatted .xtiming XML payload.
     */
    [[nodiscard]] static std::string GenerateTransientTimingXML(
        const std::vector<float>& pcmBuffer,
        int sampleRate,
        const std::string& trackName
    );
};

} // namespace xLights::AI
