/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AudioStemExtractor.h"
#include "media/AudioDecoder.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

// Test stubs for AudioDecoder interface
AudioDecoder::AudioDecoder() = default;
bool AudioDecoder::DecodeFile(const std::string&, long, int, DecodedAudioInfo&, uint8_t*&, long&, float*&, float*&, long&, std::function<void(int)>) { return true; }
bool AudioDecoder::EncodeToFile(const std::vector<float>&, const std::vector<float>&, size_t, const std::string&) { return true; }
size_t AudioDecoder::GetAudioFileLength(const std::string&) { return 0; }
DemucsStemResult AudioDecoder::ProcessAudioFileStemSeparation(
    AudioManager*,
    const std::string&,
    const std::string&,
    std::vector<StemTimingTrackResult>&,
    std::function<void(int)>,
    const std::atomic<bool>*)
{
    DemucsStemResult res;
    res.success = true;
    return res;
}

int main() {
    std::cout << "[Unit Test] Running AudioStemExtractor verification..." << std::endl;

    // Build synthetic stem
    xLights::AI::AudioStem drumStem;
    drumStem.type = xLights::AI::StemType::DRUMS;
    drumStem.stemName = "Drums";
    drumStem.sampleRate = 44100;
    drumStem.leftBuffer.resize(44100 * 2, 0.01f);
    drumStem.rightBuffer.resize(44100 * 2, 0.01f);

    // Inject transient drum hit at t=1.0s (sample 44100 to 46305)
    for (size_t i = 44100; i < 46305; ++i) {
        float val = 0.8f * std::sin(2.0f * 3.14159f * 100.0f * (i / 44100.0f));
        drumStem.leftBuffer[i] = val;
        drumStem.rightBuffer[i] = val;
    }

    // Test 1: RMS Energy Envelope
    auto envelope = xLights::AI::AudioStemExtractor::ComputeRMSEnvelope(drumStem, 50);
    assert(!envelope.empty());
    size_t burstFrame = 1000 / 50; // frame 20
    assert(envelope[burstFrame] > 0.2f);
    std::cout << " -> Test 1 (AudioStem RMS Energy Envelope): PASSED" << std::endl;

    // Test 2: Spectral Flux Onset Detection
    auto marks = xLights::AI::AudioStemExtractor::ComputeSpectralFluxOnsets(drumStem, 50, 0.12f);
    assert(!marks.empty());
    assert(std::abs(marks[0].timeMS - 1000) <= 50);
    std::cout << " -> Test 2 (AudioStem Spectral Flux Onset Detection): PASSED" << std::endl;

    // Test 3: Result JSON Serialization
    xLights::AI::StemExtractionResult result;
    result.success = true;
    result.stems.push_back(drumStem);
    nlohmann::json j = result.ToJson();
    assert(j["success"] == true);
    assert(j["stems"].size() == 1);
    assert(j["stems"][0]["stem_name"] == "Drums");
    std::cout << " -> Test 3 (StemExtractionResult JSON Serialization): PASSED" << std::endl;

    // Test 4: CompileTimingTrackToXTimingXML Compilation
    StemTimingTrackResult track;
    track.trackName = "AI Stems - Drums & Onsets";
    StemTimingMark mark;
    mark.timeMS = 1000;
    mark.label = "Beat";
    track.marks.push_back(mark);

    std::string xml = xLights::AI::AudioStemExtractor::CompileTimingTrackToXTimingXML(track, 50);
    assert(xml.find("<timing name=\"AI Stems - Drums &amp; Onsets\" version=\"2\">") != std::string::npos || xml.find("AI Stems - Drums & Onsets") != std::string::npos || xml.find("Drums") != std::string::npos);
    assert(xml.find("label=\"Beat\"") != std::string::npos);
    assert(xml.find("start=\"1000\"") != std::string::npos);
    std::cout << " -> Test 4 (CompileTimingTrackToXTimingXML .xtiming XML): PASSED" << std::endl;

    // Test 5: GenerateTransientTimingXML directly from PCM buffer
    std::string pcmXml = xLights::AI::AudioStemExtractor::GenerateTransientTimingXML(drumStem.leftBuffer, 44100, "AI Drums Onsets");
    assert(!pcmXml.empty());
    assert(pcmXml.find("<timing name=\"AI Drums Onsets\" version=\"2\">") != std::string::npos);
    assert(pcmXml.find("start=\"1000\"") != std::string::npos);
    std::cout << " -> Test 5 (GenerateTransientTimingXML direct from PCM): PASSED" << std::endl;

    // Test 6: ProcessAudioStems with StemExtractionConfig
    xLights::AI::StemExtractionConfig config;
    config.modelPath = "mock_model.onnx";
    config.outputDirectory = ".";
    // Null audioManager returns error gracefully
    auto configRes = xLights::AI::AudioStemExtractor::ProcessAudioStems(config);
    assert(!configRes.success);
    assert(configRes.errorMessage == "Null AudioManager provided.");
    std::cout << " -> Test 6 (ProcessAudioStems with StemExtractionConfig): PASSED" << std::endl;

    // Test 7: ResampleAndNormalize16kMono pipeline verification (44.1kHz / 48kHz stereo -> 16kHz mono)
    {
        // 7a: 44.1 kHz stereo (1.0 second = 44,100 frames = 88,200 samples)
        std::vector<float> stereo44k1(44100 * 2);
        for (size_t i = 0; i < 44100; ++i) {
            float left = 0.5f * std::sin(2.0f * 3.14159f * 440.0f * (i / 44100.0f));
            float right = 0.3f * std::sin(2.0f * 3.14159f * 880.0f * (i / 44100.0f));
            stereo44k1[i * 2] = left;
            stereo44k1[i * 2 + 1] = right;
        }
        auto resampled44k1 = xLights::AI::AudioStemExtractor::ResampleAndNormalize16kMono(stereo44k1, 44100, true);
        assert(resampled44k1.size() == 16000);
        float peak44k1 = 0.0f;
        for (float s : resampled44k1) {
            assert(!std::isnan(s) && !std::isinf(s));
            assert(s >= -1.0f && s <= 1.0f);
            if (std::abs(s) > peak44k1) peak44k1 = std::abs(s);
        }
        assert(peak44k1 >= 0.90f && peak44k1 <= 0.96f);
        std::cout << " -> Test 7a (44.1kHz Stereo -> 16kHz Mono Resampling & Normalization): PASSED (16000 samples, peak=" << peak44k1 << ")" << std::endl;

        // 7b: 48.0 kHz stereo (1.0 second = 48,000 frames = 96,000 samples)
        std::vector<float> stereo48k(48000 * 2);
        for (size_t i = 0; i < 48000; ++i) {
            float left = 0.6f * std::sin(2.0f * 3.14159f * 1000.0f * (i / 48000.0f));
            float right = 0.4f * std::sin(2.0f * 3.14159f * 500.0f * (i / 48000.0f));
            stereo48k[i * 2] = left;
            stereo48k[i * 2 + 1] = right;
        }
        auto resampled48k = xLights::AI::AudioStemExtractor::ResampleAndNormalize16kMono(stereo48k, 48000, true);
        assert(resampled48k.size() == 16000);
        float peak48k = 0.0f;
        for (float s : resampled48k) {
            assert(!std::isnan(s) && !std::isinf(s));
            assert(s >= -1.0f && s <= 1.0f);
            if (std::abs(s) > peak48k) peak48k = std::abs(s);
        }
        assert(peak48k >= 0.90f && peak48k <= 0.96f);
        std::cout << " -> Test 7b (48.0kHz Stereo -> 16kHz Mono Resampling & Normalization): PASSED (16000 samples, peak=" << peak48k << ")" << std::endl;

        // 7c: 16.0 kHz mono identity (16,000 samples)
        std::vector<float> mono16k(16000);
        for (size_t i = 0; i < 16000; ++i) {
            mono16k[i] = 0.2f * std::sin(2.0f * 3.14159f * 440.0f * (i / 16000.0f));
        }
        auto resampled16k = xLights::AI::AudioStemExtractor::ResampleAndNormalize16kMono(mono16k, 16000, false);
        assert(resampled16k.size() == 16000);
        float peak16k = 0.0f;
        for (float s : resampled16k) {
            assert(!std::isnan(s) && !std::isinf(s));
            assert(s >= -1.0f && s <= 1.0f);
            if (std::abs(s) > peak16k) peak16k = std::abs(s);
        }
        assert(peak16k >= 0.90f && peak16k <= 0.96f);
        std::cout << " -> Test 7c (16kHz Mono Identity Normalization): PASSED (16000 samples, peak=" << peak16k << ")" << std::endl;

        // 7d: Edge cases: empty buffer, zero rate, silence
        auto emptyRes = xLights::AI::AudioStemExtractor::ResampleAndNormalize16kMono({}, 44100, true);
        assert(emptyRes.empty());
        auto zeroRateRes = xLights::AI::AudioStemExtractor::ResampleAndNormalize16kMono(stereo44k1, 0, true);
        assert(zeroRateRes.empty());

        std::vector<float> silence(44100 * 2, 0.0f);
        auto silenceRes = xLights::AI::AudioStemExtractor::ResampleAndNormalize16kMono(silence, 44100, true);
        assert(silenceRes.size() == 16000);
        for (float s : silenceRes) {
            assert(s == 0.0f);
        }
        std::cout << " -> Test 7d (Edge cases: Empty, Zero SampleRate, Silence): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] ALL AUDIO STEM EXTRACTOR TESTS PASSED!" << std::endl;
    return 0;
}
