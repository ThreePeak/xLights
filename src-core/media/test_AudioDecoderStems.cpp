/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "media/AudioDecoder.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

int main() {
    std::cout << "[Unit Test] Running AudioDecoder RMS & Spectral Flux verification..." << std::endl;

    // Generate synthetic 44.1kHz audio with a beat burst at t=1.0s (44100 samples)
    long sampleRate = 44100;
    size_t totalSamples = 44100 * 2; // 2 seconds
    std::vector<float> left(totalSamples, 0.01f);
    std::vector<float> right(totalSamples, 0.01f);

    // Inject transient beat burst at t=1.0s (sample 44100 to 46305)
    for (size_t i = 44100; i < 46305; ++i) {
        float val = 0.8f * std::sin(2.0f * 3.14159f * 100.0f * (i / 44100.0f));
        left[i] = val;
        right[i] = val;
    }

    // Test 1: RMS Energy Envelope Calculation
    std::vector<float> envelope = AudioDecoder::ComputeRMSEnvelope(left, right, sampleRate, 50);
    assert(!envelope.empty());
    size_t burstFrame = (1000) / 50; // frame 20 at 1.0s
    assert(envelope[burstFrame] > 0.2f);
    std::cout << " -> Test 1 (RMS Energy Envelope Calculation): PASSED" << std::endl;

    // Test 2: Spectral Flux Onset Detection
    std::vector<StemTimingMark> marks = AudioDecoder::ComputeSpectralFluxOnsets(left, right, sampleRate, 50, 1.5f);
    assert(!marks.empty());
    assert(std::abs(marks[0].timeMS - 1000) <= 50);
    std::cout << " -> Test 2 (Spectral Flux Onset Detection): PASSED" << std::endl;

    // Test 3: GenerateStemTimingTracks
    StemOutput stems;
    stems.sampleRate = sampleRate;
    stems.drumsL = left;
    stems.drumsR = right;
    stems.vocalsL = left;
    stems.vocalsR = right;

    AudioDecoder decoder;
    auto timingTracks = decoder.GenerateStemTimingTracks(stems, 50);
    assert(timingTracks.size() == 2);
    assert(timingTracks[0].trackName == "AI Stems - Drums & Onsets");
    assert(timingTracks[1].trackName == "AI Stems - Vocals");
    std::cout << " -> Test 3 (GenerateStemTimingTracks Integration): PASSED" << std::endl;

    std::cout << "[Unit Test] ALL AUDIO STEM TESTS PASSED!" << std::endl;
    return 0;
}
