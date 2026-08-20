/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PhonemeMap.h"
#include <iostream>
#include <vector>
#include <cassert>

int main() {
    std::cout << "[Unit Test] Running PhonemeMap verification..." << std::endl;

    // Test 1: ARPAbet phoneme to VisemeState mapping
    assert(xLights::AI::PhonemeMap::PhonemeToViseme("AA1") == xLights::AI::VisemeState::AI);
    assert(xLights::AI::PhonemeMap::PhonemeToViseme("AY0") == xLights::AI::VisemeState::AI);
    assert(xLights::AI::PhonemeMap::PhonemeToViseme("EH") == xLights::AI::VisemeState::E);
    assert(xLights::AI::PhonemeMap::PhonemeToViseme("OW2") == xLights::AI::VisemeState::O);
    assert(xLights::AI::PhonemeMap::PhonemeToViseme("UW") == xLights::AI::VisemeState::U);
    assert(xLights::AI::PhonemeMap::PhonemeToViseme("W") == xLights::AI::VisemeState::WQ);
    assert(xLights::AI::PhonemeMap::PhonemeToViseme("M") == xLights::AI::VisemeState::MBP);
    assert(xLights::AI::PhonemeMap::PhonemeToViseme("L") == xLights::AI::VisemeState::L);
    assert(xLights::AI::PhonemeMap::PhonemeToViseme("S") == xLights::AI::VisemeState::ETC);
    assert(xLights::AI::PhonemeMap::PhonemeToViseme("SIL") == xLights::AI::VisemeState::REST);
    std::cout << " -> Test 1 (ARPAbet to 8-State Viseme Mapping): PASSED" << std::endl;

    // Test 2: Sequence timing mapping
    std::vector<xLights::AI::PhonemeTimeInterval> rawPhonemes = {
        {"SIL", 0, 200, 1.0f},
        {"HH", 200, 250, 0.95f},
        {"EH1", 250, 450, 0.98f},
        {"L", 450, 600, 0.92f},
        {"OW1", 600, 850, 0.99f}
    };

    auto marks = xLights::AI::PhonemeMap::MapPhonemesToVisemes(rawPhonemes);
    assert(marks.size() == 5);
    assert(marks[0].visemeName == "rest");
    assert(marks[1].visemeName == "etc");
    assert(marks[2].visemeName == "E");
    assert(marks[3].visemeName == "L");
    assert(marks[4].visemeName == "O");
    std::cout << " -> Test 2 (Timed Phoneme-to-Viseme Sequence): PASSED" << std::endl;

    // Test 3: Debouncing & smoothing filter
    std::vector<xLights::AI::VisemeTimingMark> jitteryMarks = {
        {xLights::AI::VisemeState::O, "O", 100, 300, 1.0f},
        {xLights::AI::VisemeState::ETC, "etc", 300, 310, 0.5f}, // 10ms micro-jitter
        {xLights::AI::VisemeState::O, "O", 310, 500, 1.0f}
    };

    auto smoothed = xLights::AI::PhonemeMap::SmoothVisemeTransitions(jitteryMarks, 30);
    assert(smoothed.size() < 3); // jitter filtered/merged
    std::cout << " -> Test 3 (Micro-Jitter Debouncing & Smoothing): PASSED" << std::endl;

    // Test 4: End-to-end singing face track generation & xTiming XML export
    auto track = xLights::AI::PhonemeMap::GenerateSingingFaceTrack("Lyric Alignment - Hello", rawPhonemes, 30);
    assert(track.success);
    assert(track.totalDurationMs == 850);
    std::string xml = track.ToXTimingXml();
    assert(xml.find("<timing name=\"Lyric Alignment - Hello\"") != std::string::npos);
    assert(xml.find("<Effect label=\"O\" starttime=\"600\" endtime=\"850\" />") != std::string::npos);
    std::cout << " -> Test 4 (Track Generation & xTiming XML Serialization): PASSED" << std::endl;

    std::cout << "[Unit Test] PhonemeMap ALL TESTS PASSED!" << std::endl;
    return 0;
}
