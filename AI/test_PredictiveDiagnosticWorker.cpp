/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <catch2/catch_test_macros.hpp>
#include "AI/PredictiveDiagnosticWorker.h"
#include "AI/MusicalSynesthesiaEngine.h"
#include "AI/VisionPropReconstructionEngine.h"
#include "AI/AtmosphericVRPreviewer.h"

TEST_CASE("AI Predictive Telemetry & Synesthesia Tests", "[AI][Predictive]") {
    SECTION("Run Background Diagnostic Telemetry Scan") {
        std::vector<std::string> models = {"MegaTree", "Front_Arch", "Matrix_Main"};
        auto report = xLights::AI::PredictiveDiagnosticWorker::RunBackgroundDiagnosticScan("", 60000, models);

        REQUIRE(report.isDirty == true);
        REQUIRE(report.badges.size() >= 2);
    }

    SECTION("Analyze Musical Synesthesia Color Palette Mappings") {
        std::vector<float> pcm(44100, 0.1f);
        auto pal = xLights::AI::MusicalSynesthesiaEngine::AnalyzeAudioBarSynesthesia(pcm, 44100, 0, 1000);

        REQUIRE(!pal.keySignature.empty());
        REQUIRE(!pal.hexColors.empty());
    }

    SECTION("Reconstruct 3D Layout from Video Clip") {
        auto res = xLights::AI::VisionPropReconstructionEngine::Reconstruct3DLayoutFromVideo("yard_scan.mp4");

        REQUIRE(res.success == true);
        REQUIRE(res.detectedPropsCount == 3);
    }

    SECTION("Initialize OpenXR Atmospheric VR Viewport") {
        xLights::AI::AtmosphericVRConfig config;
        bool ok = xLights::AI::AtmosphericVRPreviewer::InitializeVRViewport(config);

        REQUIRE(ok == true);
    }
}
