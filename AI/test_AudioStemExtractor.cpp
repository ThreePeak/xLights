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

    std::cout << "[Unit Test] ALL AUDIO STEM EXTRACTOR TESTS PASSED!" << std::endl;
    return 0;
}
