/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <catch2/catch_test_macros.hpp>
#include "AI/VideoSequenceEmulator.h"

TEST_CASE("VideoSequenceEmulator tests", "[ai][video][choreography]") {
    xLights::AI::VideoSequenceEmulator emulator;

    std::string fakeLayout = 
        "<models>"
        "  <model name=\"MegaTree\" type=\"Tree 360\" parm1=\"16\" parm2=\"50\" />"
        "  <model name=\"Arches\" type=\"Arch\" parm1=\"4\" parm2=\"25\" />"
        "  <model name=\"MiniTrees\" type=\"Tree 180\" parm1=\"4\" parm2=\"20\" />"
        "  <model name=\"Roofline\" type=\"Single Line\" parm1=\"1\" parm2=\"100\" />"
        "</models>";

    SECTION("AnalyzeVideoSource returns valid analysis with beat markers and palette") {
        xLights::AI::VideoSourceInput input;
        input.filePath = "C:/Videos/ChristmasShowPreview.mp4";
        input.userPrompt = "Fast paced EDM holiday light show clip, focus on high contrast sweeps";
        input.hasTimeRange = true;
        input.timeStartMs = 10000;
        input.timeEndMs = 30000;

        auto analysis = emulator.AnalyzeVideoSource(input, fakeLayout);
        REQUIRE(analysis.detectedDurationMs == 20000);
        REQUIRE(analysis.detectedBpm >= 90.0f);
        REQUIRE(!analysis.detectedBeatMarkers.empty());
        REQUIRE(!analysis.dominantPalette.empty());
        REQUIRE(!analysis.visualTracks.empty());
        REQUIRE(!analysis.executiveSummary.empty());
        REQUIRE(!analysis.observations.empty());

        SECTION("SuggestStrategies proposes 3 distinct viable options") {
            auto strategies = emulator.SuggestStrategies(analysis, fakeLayout);
            REQUIRE(strategies.size() >= 3);
            bool foundRecommended = false;
            for (const auto& s : strategies) {
                if (s.isRecommended) foundRecommended = true;
            }
            REQUIRE(foundRecommended);
        }

        SECTION("GenerateConsultationQuestions provides interactive options") {
            auto questions = emulator.GenerateConsultationQuestions(analysis, fakeLayout);
            REQUIRE(questions.size() >= 2);
            for (const auto& q : questions) {
                REQUIRE(!q.questionText.empty());
                REQUIRE(q.options.size() >= 2);
            }
        }

        SECTION("GenerateEmulationPlan creates valid effect cues mapped to layout props") {
            std::map<std::string, std::string> choices;
            choices["palette_policy"] = "Strictly reproduce video footage palette";

            auto plan = emulator.GenerateEmulationPlan(input, analysis, "macro_spatial_flow", choices, fakeLayout);
            REQUIRE(plan.totalCuesCount > 0);
            REQUIRE(!plan.generatedCues.empty());

            const auto& firstCue = plan.generatedCues[0];
            REQUIRE(!firstCue.targetPropName.empty());
            REQUIRE(!firstCue.effectType.empty());
            REQUIRE(firstCue.endMs > firstCue.startMs);
            REQUIRE(!firstCue.primaryColor.empty());
            REQUIRE(!firstCue.rationale.empty());

            SECTION("ExportPlanToXsqXml serializes valid xsequence document") {
                std::string xsq = emulator.ExportPlanToXsqXml(plan);
                REQUIRE(xsq.find("<?xml version=\"1.0\"") != std::string::npos);
                REQUIRE(xsq.find("<xsequence") != std::string::npos);
                REQUIRE(xsq.find("<DisplayElements>") != std::string::npos);
                REQUIRE(xsq.find("<Effect ") != std::string::npos);
            }

            SECTION("RefinePlan dynamically adjusts speed and colors") {
                xLights::AI::SequenceRefinementRequest ref;
                ref.tuningPrompt = "Double speed and shift colors to warm wash";
                ref.speedFactor = 2.0f;
                auto refined = emulator.RefinePlan(plan, ref);
                REQUIRE(refined.totalCuesCount == plan.totalCuesCount);
                // First cue duration should be halved
                int origDuration = plan.generatedCues[0].endMs - plan.generatedCues[0].startMs;
                int newDuration = refined.generatedCues[0].endMs - refined.generatedCues[0].startMs;
                REQUIRE(newDuration <= origDuration);
            }
        }
    }
}
