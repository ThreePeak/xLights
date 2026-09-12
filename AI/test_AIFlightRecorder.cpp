#include <catch2/catch_test_macros.hpp>
#include "AI/AIFlightRecorder.h"
#include <thread>
#include <chrono>

TEST_CASE("AIFlightRecorder: Session Lifecycle and Step Recording", "[AIFlightRecorder]") {
    auto& recorder = xLights::AI::AIFlightRecorder::Instance();

    REQUIRE_FALSE(recorder.IsRecording());

    recorder.StartSession("Test Automated Unit Session");
    REQUIRE(recorder.IsRecording());
    REQUIRE(recorder.GetStepCount() >= 1); // Initial session start step

    // Record custom steps
    recorder.RecordStep(xLights::AI::FlightCategory::UI_ACTION,
                        "CustomPropDesigner",
                        "Button_Click_Generate",
                        "User clicked Generate Layout with 32 nodes",
                        {{"nodeCount", 32}, {"preset", "Circle"}});

    {
        xLights::AI::AIFlightScopeTimer timer(xLights::AI::FlightCategory::ALGORITHM,
                                              "CustomPropDesigner",
                                              "TSP_2Opt_Optimization",
                                              "Running 2-opt TSP wire solver",
                                              {{"startNodes", 32}});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timer.AddPayload("endWireReductionPct", 42.5);
    }

    recorder.RecordError("VideoSequenceEmulator",
                         "VideoFrameDecode",
                         "Sample video codec missing packet");

    REQUIRE(recorder.GetStepCount() >= 4);

    recorder.StopSession();
    REQUIRE_FALSE(recorder.IsRecording());

    auto meta = recorder.GetMetadata();
    REQUIRE(meta.sessionName == "Test Automated Unit Session");
    REQUIRE(meta.totalSteps >= 5); // + Stop step
    REQUIRE(meta.totalErrors == 1);
    REQUIRE(meta.xLightsVersion == "2026.17");

    // JSON trace validation
    std::string jsonStr = recorder.GenerateJsonTrace();
    REQUIRE(jsonStr.find("Test Automated Unit Session") != std::string::npos);
    REQUIRE(jsonStr.find("TSP_2Opt_Optimization") != std::string::npos);
    REQUIRE(jsonStr.find("Button_Click_Generate") != std::string::npos);
    REQUIRE(jsonStr.find("Sample video codec missing packet") != std::string::npos);

    // HTML report validation
    std::string htmlStr = recorder.GenerateHtmlReport();
    REQUIRE(htmlStr.find("<!DOCTYPE html>") != std::string::npos);
    REQUIRE(htmlStr.find("xLights AI Problem Steps & Flight Record") != std::string::npos);
    REQUIRE(htmlStr.find("TSP_2Opt_Optimization") != std::string::npos);
    REQUIRE(htmlStr.find("⚠️ 1 Issues Detected") != std::string::npos);
}

TEST_CASE("AIFlightRecorder: Automatic Redaction of API Keys & Secrets", "[AIFlightRecorder]") {
    std::string sensitive = "Calling OpenAI with key sk-1234567890abcdef12345678 and Anthropic sk-ant-abcdef1234567890.";
    std::string sanitized = xLights::AI::AIFlightRecorder::SanitizeString(sensitive);

    REQUIRE(sanitized.find("sk-1234567890abcdef12345678") == std::string::npos);
    REQUIRE(sanitized.find("sk-ant-abcdef1234567890") == std::string::npos);
    REQUIRE(sanitized.find("[REDACTED_API_KEY]") != std::string::npos);

    nlohmann::json sensitiveJson = {
        {"apiKey", "sk-live-999988887777666655554444"},
        {"userPrompt", "Choreograph sequence with sk-test-1111222233334444"},
        {"settings", {
            {"tokenSecret", "my_super_secret_token_123"},
            {"model", "gpt-4o"}
        }}
    };

    nlohmann::json sanitizedJson = xLights::AI::AIFlightRecorder::SanitizeJson(sensitiveJson);
    REQUIRE(sanitizedJson["apiKey"] == "[REDACTED_CREDENTIAL]");
    REQUIRE(sanitizedJson["settings"]["tokenSecret"] == "[REDACTED_CREDENTIAL]");
    REQUIRE(sanitizedJson["settings"]["model"] == "gpt-4o");
    REQUIRE(sanitizedJson["userPrompt"].get<std::string>().find("sk-test") == std::string::npos);
}
