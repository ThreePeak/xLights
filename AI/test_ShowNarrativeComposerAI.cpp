/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/media/ShowNarrativeComposerAI.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running ShowNarrativeComposerAI verification..." << std::endl;

    // Test 1: Script Composition across Styles
    {
        NarrativePromptParameters params;
        params.showTitle = "Winter Wonderland";
        params.familyOrCityName = "The Johnsons";
        params.style = NarrativeStyle::FESTIVE_CHRISTMAS_STORY;

        auto script = ShowNarrativeComposerAI::ComposeScript(params);
        assert(!script.empty());
        assert(script.find("Winter Wonderland") != std::string::npos);
        assert(script.find("The Johnsons") != std::string::npos);
        std::cout << " -> Test 1 (Script Composition across Styles): PASSED" << std::endl;
    }

    // Test 2: Aligned Timings & Viseme Generation
    {
        std::string script = "Merry Christmas to all and to all a good night!";
        auto timings = ShowNarrativeComposerAI::GenerateAlignedTimings(script, 1.0f);
        assert(timings.size() == 10);
        assert(timings.front().startMs >= 500);
        assert(!timings.front().phonemeVisemeHint.empty());
        std::cout << " -> Test 2 (Aligned Timings & Viseme Generation): PASSED (" << timings.size() << " timing marks)" << std::endl;
    }

    // Test 3: xTiming XML Export & JSON Serialization
    {
        NarrativePromptParameters params;
        params.style = NarrativeStyle::RADIO_DJ_INTRO;
        auto result = ShowNarrativeComposerAI::GenerateNarrative(params);

        std::string xml = result.ExportXTimingXml("AI Voiceover");
        assert(xml.find("<timing name=\"AI Voiceover\"") != std::string::npos);
        assert(xml.find("<mark start=") != std::string::npos);

        auto j = result.ToJson();
        assert(j.contains("timingMarks"));
        assert(j.contains("recommendedLightingCues"));
        std::cout << " -> Test 3 (xTiming XML Export & JSON Serialization): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] ShowNarrativeComposerAI ALL TESTS PASSED!" << std::endl;
    return 0;
}
