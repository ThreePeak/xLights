/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include "AI/VideoSequenceEmulator.h"

int main() {
    std::cout << "[Unit Test] Running VideoSequenceEmulator verification..." << std::endl;
    xLights::AI::VideoSequenceEmulator emulator;

    std::string fakeLayout = 
        "<models>"
        "  <model name=\"MegaTree\" type=\"Tree 360\" parm1=\"16\" parm2=\"50\" />"
        "  <model name=\"Arches\" type=\"Arch\" parm1=\"4\" parm2=\"25\" />"
        "  <model name=\"MiniTrees\" type=\"Tree 180\" parm1=\"4\" parm2=\"20\" />"
        "  <model name=\"Roofline\" type=\"Single Line\" parm1=\"1\" parm2=\"100\" />"
        "</models>";

    xLights::AI::VideoSourceInput input;
    input.filePath = "C:/Videos/ChristmasShowPreview.mp4";
    input.userPrompt = "Fast paced EDM holiday light show clip, focus on high contrast sweeps";
    input.hasTimeRange = true;
    input.timeStartMs = 10000;
    input.timeEndMs = 30000;

    auto analysis = emulator.AnalyzeVideoSource(input, fakeLayout);
    assert(analysis.detectedDurationMs == 20000);
    assert(analysis.detectedBpm >= 90.0f);
    assert(!analysis.detectedBeatMarkers.empty());
    assert(!analysis.dominantPalette.empty());
    assert(!analysis.visualTracks.empty());
    assert(!analysis.executiveSummary.empty());
    assert(!analysis.observations.empty());
    std::cout << " -> Test 1 (AnalyzeVideoSource): PASSED" << std::endl;

    auto strategies = emulator.SuggestStrategies(analysis, fakeLayout);
    assert(strategies.size() >= 3);
    bool foundRecommended = false;
    for (const auto& s : strategies) {
        if (s.isRecommended) foundRecommended = true;
    }
    assert(foundRecommended);
    std::cout << " -> Test 2 (SuggestStrategies): PASSED" << std::endl;

    auto questions = emulator.GenerateConsultationQuestions(analysis, fakeLayout);
    assert(questions.size() >= 2);
    for (const auto& q : questions) {
        assert(!q.questionText.empty());
        assert(q.options.size() >= 2);
    }
    std::cout << " -> Test 3 (GenerateConsultationQuestions): PASSED" << std::endl;

    std::map<std::string, std::string> choices;
    choices["palette_policy"] = "Strictly reproduce video footage palette";
    auto plan = emulator.GenerateEmulationPlan(input, analysis, "macro_spatial_flow", choices, fakeLayout);
    assert(plan.totalCuesCount > 0);
    assert(!plan.generatedCues.empty());
    const auto& firstCue = plan.generatedCues[0];
    assert(!firstCue.targetPropName.empty());
    assert(!firstCue.effectType.empty());
    assert(firstCue.endMs > firstCue.startMs);
    assert(!firstCue.primaryColor.empty());
    assert(!firstCue.rationale.empty());
    std::cout << " -> Test 4 (GenerateEmulationPlan): PASSED" << std::endl;

    std::string xsq = emulator.ExportPlanToXsqXml(plan);
    assert(xsq.find("<?xml version=\"1.0\"") != std::string::npos);
    assert(xsq.find("<xsequence") != std::string::npos);
    assert(xsq.find("<DisplayElements>") != std::string::npos);
    assert(xsq.find("<Effect ") != std::string::npos);
    std::cout << " -> Test 5 (ExportPlanToXsqXml): PASSED" << std::endl;

    xLights::AI::SequenceRefinementRequest ref;
    ref.tuningPrompt = "Double speed and shift colors to warm wash";
    ref.speedFactor = 2.0f;
    auto refined = emulator.RefinePlan(plan, ref);
    assert(refined.totalCuesCount == plan.totalCuesCount);
    int origDuration = plan.generatedCues[0].endMs - plan.generatedCues[0].startMs;
    int newDuration = refined.generatedCues[0].endMs - refined.generatedCues[0].startMs;
    assert(newDuration <= origDuration);
    std::cout << " -> Test 6 (RefinePlan): PASSED" << std::endl;

    std::string path, err;
    assert(!xLights::AI::VideoSequenceEmulator::DownloadVideoUrlToTemp("", path, err));
    assert(!xLights::AI::VideoSequenceEmulator::DownloadVideoUrlToTemp("ftp://invalid.com/video.mp4", path, err));
    std::cout << " -> Test 7 (DownloadVideoUrlToTemp): PASSED" << std::endl;

    std::cout << "[Unit Test] All VideoSequenceEmulator tests PASSED!" << std::endl;
    return 0;
}
