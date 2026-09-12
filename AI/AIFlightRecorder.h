#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>
#include <functional>
#include <nlohmann/json.hpp>

namespace xLights::AI {

enum class FlightCategory {
    UI_ACTION,
    AI_PIPELINE,
    ALGORITHM,
    RENDER_TIMELINE,
    NETWORK_API,
    SAFETY_RULE,
    ERROR_EVENT
};

inline std::string FlightCategoryToString(FlightCategory cat) {
    switch (cat) {
        case FlightCategory::UI_ACTION:       return "UI_ACTION";
        case FlightCategory::AI_PIPELINE:     return "AI_PIPELINE";
        case FlightCategory::ALGORITHM:       return "ALGORITHM";
        case FlightCategory::RENDER_TIMELINE: return "RENDER_TIMELINE";
        case FlightCategory::NETWORK_API:     return "NETWORK_API";
        case FlightCategory::SAFETY_RULE:     return "SAFETY_RULE";
        case FlightCategory::ERROR_EVENT:     return "ERROR";
        default:                              return "UNKNOWN";
    }
}

inline FlightCategory StringToFlightCategory(const std::string& str) {
    if (str == "UI_ACTION")       return FlightCategory::UI_ACTION;
    if (str == "AI_PIPELINE")     return FlightCategory::AI_PIPELINE;
    if (str == "ALGORITHM")       return FlightCategory::ALGORITHM;
    if (str == "RENDER_TIMELINE") return FlightCategory::RENDER_TIMELINE;
    if (str == "NETWORK_API")     return FlightCategory::NETWORK_API;
    if (str == "SAFETY_RULE")     return FlightCategory::SAFETY_RULE;
    if (str == "ERROR")           return FlightCategory::ERROR_EVENT;
    return FlightCategory::UI_ACTION;
}

struct FlightStep {
    uint32_t stepNumber = 0;
    std::string timestampIso;
    int64_t elapsedMs = 0;
    FlightCategory category = FlightCategory::UI_ACTION;
    std::string subsystem;
    std::string action;
    std::string details;
    int64_t durationMs = 0;
    nlohmann::json payload;
    bool isError = false;
};

struct FlightSessionMetadata {
    std::string sessionId;
    std::string sessionName;
    std::string startTimeIso;
    std::string stopTimeIso;
    int64_t totalDurationMs = 0;
    std::string xLightsVersion;
    std::string osPlatform;
    std::string cpuInfo;
    std::string gpuRenderer;
    std::string activeLlmModel;
    float activeLlmTemp = 0.0f;
    uint32_t totalSteps = 0;
    uint32_t totalErrors = 0;
    std::vector<std::string> activeSubsystems;
};

class AIFlightRecorder {
public:
    static AIFlightRecorder& Instance();

    // Session lifecycle
    void StartSession(const std::string& sessionName = "Diagnostic Session");
    void StopSession();
    bool IsRecording() const noexcept { return m_isRecording.load(std::memory_order_relaxed); }

    // Step recording
    void RecordStep(FlightCategory category,
                    const std::string& subsystem,
                    const std::string& action,
                    const std::string& details,
                    const nlohmann::json& payload = nlohmann::json::object(),
                    int64_t durationMs = 0);

    void RecordError(const std::string& subsystem,
                     const std::string& context,
                     const std::string& errorMsg);

    // Environment info setters
    void SetEnvironmentInfo(const std::string& osPlatform,
                            const std::string& cpuInfo,
                            const std::string& gpuRenderer,
                            const std::string& activeLlmModel,
                            float activeLlmTemp);

    // Inspection
    FlightSessionMetadata GetMetadata() const;
    std::vector<FlightStep> GetSteps() const;
    uint32_t GetStepCount() const;
    int64_t GetCurrentElapsedMs() const;

    // Report and export generation
    std::string GenerateJsonTrace() const;
    std::string GenerateHtmlReport() const;
    bool ExportSessionFolder(const std::string& targetDir) const;

    // Sanitization helper
    static std::string SanitizeString(const std::string& input);
    static nlohmann::json SanitizeJson(const nlohmann::json& input);

    // Listener for UI live updates
    void SetStepCallback(std::function<void(const FlightStep&)> callback);

private:
    AIFlightRecorder() = default;
    ~AIFlightRecorder() = default;
    AIFlightRecorder(const AIFlightRecorder&) = delete;
    AIFlightRecorder& operator=(const AIFlightRecorder&) = delete;

    mutable std::mutex m_mutex;
    std::atomic<bool> m_isRecording{false};
    std::chrono::steady_clock::time_point m_sessionStartPoint;
    FlightSessionMetadata m_metadata;
    std::vector<FlightStep> m_steps;
    std::function<void(const FlightStep&)> m_stepCallback;
};

// RAII performance and step timer
class AIFlightScopeTimer {
public:
    AIFlightScopeTimer(FlightCategory category,
                       std::string subsystem,
                       std::string action,
                       std::string details,
                       nlohmann::json payload = nlohmann::json::object())
        : m_category(category)
        , m_subsystem(std::move(subsystem))
        , m_action(std::move(action))
        , m_details(std::move(details))
        , m_payload(std::move(payload))
        , m_start(std::chrono::steady_clock::now())
        , m_active(AIFlightRecorder::Instance().IsRecording())
    {}

    ~AIFlightScopeTimer() {
        if (m_active && AIFlightRecorder::Instance().IsRecording()) {
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();
            AIFlightRecorder::Instance().RecordStep(m_category, m_subsystem, m_action, m_details, m_payload, duration);
        }
    }

    void UpdateDetails(const std::string& details) { m_details = details; }
    void AddPayload(const std::string& key, const nlohmann::json& value) { m_payload[key] = value; }

private:
    FlightCategory m_category;
    std::string m_subsystem;
    std::string m_action;
    std::string m_details;
    nlohmann::json m_payload;
    std::chrono::steady_clock::time_point m_start;
    bool m_active;
};

} // namespace xLights::AI
