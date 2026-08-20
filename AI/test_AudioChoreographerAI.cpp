/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/media/AudioChoreographerAI.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running AudioChoreographerAI verification..." << std::endl;

    // Test 1: Kick Drum Stem Onset Detection & Timing Track Generation
    {
        ChoreographyParameters params;
        params.stemType = AudioStemType::KICK_DRUM;
        params.outputMode = ChoreographyOutputMode::GENERATE_TIMING_MARK_TRACK;
        params.targetPropName = "MegaTree";

        auto result = AudioChoreographerAI::AnalyzeAndChoreograph({}, 44100, params);
        assert(result.detectedHitCount > 0);
        assert(!result.generatedTimingTrackXml.empty());
        assert(result.generatedTimingTrackXml.find("<timing name=\"Kick Drum") != std::string::npos);
        std::cout << " -> Test 1 (Kick Drum Stem Onset Detection & Timing Track Generation): PASSED ("
                  << result.detectedHitCount << " onsets detected)" << std::endl;
    }

    // Test 2: Non-Destructive Delta Refinement Patching
    {
        ChoreographyParameters params;
        params.targetPropName = "Arches";
        params.desiredEffectType = "Shockwave";
        params.naturalLanguagePrompt = "Trigger cyan shockwave on Arches every kick hit";

        auto result = AudioChoreographerAI::AnalyzeAndChoreograph({}, 44100, params);
        std::string patched = AudioChoreographerAI::ApplyDeltaRefinement("<base_sequence/>", params.naturalLanguagePrompt, result);

        assert(!patched.empty());
        assert(patched.find("Trigger cyan shockwave") != std::string::npos);
        assert(patched.find("<base_sequence/>") != std::string::npos);
        assert(patched.find("<effects model=\"Arches\">") != std::string::npos);
        std::cout << " -> Test 2 (Non-Destructive Delta Refinement Patching): PASSED" << std::endl;
    }

    // Test 3: Formatted Report & JSON Serialization
    {
        ChoreographyParameters params;
        auto result = AudioChoreographerAI::AnalyzeAndChoreograph({}, 44100, params);
        std::string report = result.GenerateFormattedReport();
        assert(report.find("AUDIO STEM INTELLIGENCE") != std::string::npos);

        auto j = result.ToJson();
        assert(j.contains("onsets"));
        assert(j.contains("audioStemName"));
        std::cout << " -> Test 3 (Formatted Report & JSON Serialization): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] AudioChoreographerAI ALL TESTS PASSED!" << std::endl;
    return 0;
}
