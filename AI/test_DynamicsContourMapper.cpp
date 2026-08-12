/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "media/AudioDynamicsMapper.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

int main() {
    std::cout << "[Unit Test] Running AudioDynamicsMapper verification..." << std::endl;

    // Generate synthetic float PCM audio (2 seconds @ 44100 Hz)
    size_t sampleRate = 44100;
    std::vector<float> leftChannel(sampleRate * 2, 0.05f);
    std::vector<float> rightChannel(sampleRate * 2, 0.05f);

    // Inject high energy section at t=1.0s to 1.5s
    for (size_t i = sampleRate; i < static_cast<size_t>(sampleRate * 1.5); ++i) {
        float val = 0.8f * std::sin(2.0f * 3.14159f * 440.0f * (i / 44100.0f));
        leftChannel[i] = val;
        rightChannel[i] = val;
    }

    AudioDynamicsMapper mapper;
    auto result = mapper.AnalyzeDynamicsContour(leftChannel, rightChannel, sampleRate, 50);

    // Test 1: Verification of frame analysis output
    assert(result.success);
    assert(!result.frames.empty());
    std::cout << " -> Test 1 (AnalyzeDynamicsContour Frame Output): PASSED" << std::endl;

    // Test 2: Energy & Emotion Metrics (Arousal, Valence, Harmonic Tension)
    bool foundPeak = false;
    for (const auto& frame : result.frames) {
        if (frame.timeMS >= 1000 && frame.timeMS <= 1400) {
            assert(frame.rmsEnergy > 0.5f);
            assert(frame.brightnessLevel > 50.0f);
            assert(frame.arousal > 0.5f);
            assert(frame.harmonicTension >= 0.0f && frame.harmonicTension <= 1.0f);
            foundPeak = true;
        }
    }
    assert(foundPeak);
    std::cout << " -> Test 2 (Emotion & HarmonicTension Metrics): PASSED" << std::endl;

    // Test 3: Export to Serialized xLights ValueCurve Custom Data String
    std::string vcStr = AudioDynamicsMapper::ExportAsValueCurveString(result);
    assert(!vcStr.empty());
    assert(vcStr.find("Active=TRUE") != std::string::npos);
    assert(vcStr.find("Type=Custom") != std::string::npos);
    assert(vcStr.find("CustomData=") != std::string::npos);
    std::cout << " -> Test 3 (ExportAsValueCurveString): PASSED" << std::endl;

    // Test 4: Export to xLights ValueCurve JSON Format
    std::string vcJson = AudioDynamicsMapper::ExportAsValueCurveJson(result);
    assert(!vcJson.empty());
    assert(vcJson.find("\"Type\":\"Custom\"") != std::string::npos || vcJson.find("\"Type\": \"Custom\"") != std::string::npos);
    assert(vcJson.find("\"Points\"") != std::string::npos);
    std::cout << " -> Test 4 (ExportAsValueCurveJson Output): PASSED" << std::endl;

    std::cout << "[Unit Test] ALL DYNAMICS CONTOUR MAPPER TESTS PASSED!" << std::endl;
    return 0;
}
