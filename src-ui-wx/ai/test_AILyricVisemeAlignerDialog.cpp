/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AI/PhonemeMap.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running AILyricVisemeAlignerDialog backend verification..." << std::endl;

    std::vector<PhonemeTimeInterval> raw = {
        {"SIL", 0, 100, 1.0f},
        {"M", 100, 200, 0.95f},
        {"EH1", 200, 350, 0.98f}
    };
    auto track = PhonemeMap::GenerateSingingFaceTrack("TestTrack", raw, 30);
    assert(track.success);
    assert(track.marks.size() >= 2);
    assert(track.marks[0].visemeName == "rest");
    std::cout << " -> AILyricVisemeAlignerDialog singing face track generator: PASSED" << std::endl;
    return 0;
}
