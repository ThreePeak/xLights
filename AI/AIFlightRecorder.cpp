#include "AI/AIFlightRecorder.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <ctime>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace xLights::AI {

static std::string GetCurrentTimestampIso() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::stringstream ss;
    std::tm bt{};
#if defined(_WIN32)
    localtime_s(&bt, &in_time_t);
#else
    localtime_r(&in_time_t, &bt);
#endif
    ss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

static std::string GenerateSessionId() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    std::tm bt{};
#if defined(_WIN32)
    localtime_s(&bt, &in_time_t);
#else
    localtime_r(&in_time_t, &bt);
#endif
    ss << "flight_rec_" << std::put_time(&bt, "%Y%m%d_%H%M%S");
    return ss.str();
}

AIFlightRecorder& AIFlightRecorder::Instance() {
    static AIFlightRecorder s_instance;
    return s_instance;
}

std::string AIFlightRecorder::SanitizeString(const std::string& input) {
    if (input.empty()) return input;

    // Redact OpenAI / Anthropic / general API keys
    static const std::regex keyRegex(R"((?:sk-[a-zA-Z0-9_\-]{16,}|sk-ant-[a-zA-Z0-9_\-]{16,}|hf_[a-zA-Z0-9_\-]{16,}|Bearer\s+[a-zA-Z0-9._\-]{16,}))", std::regex::optimize);
    std::string result = std::regex_replace(input, keyRegex, "[REDACTED_API_KEY]");

    return result;
}

nlohmann::json AIFlightRecorder::SanitizeJson(const nlohmann::json& input) {
    if (input.is_string()) {
        return SanitizeString(input.get<std::string>());
    } else if (input.is_object()) {
        nlohmann::json sanitized = nlohmann::json::object();
        for (auto it = input.begin(); it != input.end(); ++it) {
            std::string keyLower = it.key();
            for (char& c : keyLower) c = (char)tolower(c);
            if (keyLower.find("key") != std::string::npos ||
                keyLower.find("secret") != std::string::npos ||
                keyLower.find("token") != std::string::npos ||
                keyLower.find("password") != std::string::npos) {
                sanitized[it.key()] = "[REDACTED_CREDENTIAL]";
            } else {
                sanitized[it.key()] = SanitizeJson(it.value());
            }
        }
        return sanitized;
    } else if (input.is_array()) {
        nlohmann::json sanitized = nlohmann::json::array();
        for (const auto& item : input) {
            sanitized.push_back(SanitizeJson(item));
        }
        return sanitized;
    }
    return input;
}

void AIFlightRecorder::StartSession(const std::string& sessionName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_steps.clear();
    m_metadata = FlightSessionMetadata{};
    m_metadata.sessionId = GenerateSessionId();
    m_metadata.sessionName = sessionName.empty() ? "Diagnostic Session" : sessionName;
    m_metadata.startTimeIso = GetCurrentTimestampIso();
    m_metadata.xLightsVersion = "2026.17";
    m_sessionStartPoint = std::chrono::steady_clock::now();
    m_isRecording.store(true, std::memory_order_release);

    spdlog::info("[AIFlightRecorder] Session started: {} ({})", m_metadata.sessionName, m_metadata.sessionId);

    // Record initial session start step
    FlightStep initStep;
    initStep.stepNumber = 1;
    initStep.timestampIso = m_metadata.startTimeIso;
    initStep.elapsedMs = 0;
    initStep.category = FlightCategory::UI_ACTION;
    initStep.subsystem = "AIFlightRecorder";
    initStep.action = "Session_Started";
    initStep.details = "Recording session initialized: " + m_metadata.sessionName;
    initStep.durationMs = 0;
    initStep.payload = {
        {"sessionId", m_metadata.sessionId},
        {"version", m_metadata.xLightsVersion}
    };
    m_steps.push_back(initStep);
    m_metadata.totalSteps = 1;
}

void AIFlightRecorder::StopSession() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_isRecording.load(std::memory_order_relaxed)) return;

    auto now = std::chrono::steady_clock::now();
    m_metadata.totalDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_sessionStartPoint).count();
    m_metadata.stopTimeIso = GetCurrentTimestampIso();
    m_metadata.totalSteps = static_cast<uint32_t>(m_steps.size());

    // Record stop step
    FlightStep stopStep;
    stopStep.stepNumber = static_cast<uint32_t>(m_steps.size() + 1);
    stopStep.timestampIso = m_metadata.stopTimeIso;
    stopStep.elapsedMs = m_metadata.totalDurationMs;
    stopStep.category = FlightCategory::UI_ACTION;
    stopStep.subsystem = "AIFlightRecorder";
    stopStep.action = "Session_Stopped";
    stopStep.details = "Recording session finished. Total steps: " + std::to_string(m_steps.size());
    stopStep.durationMs = 0;
    stopStep.payload = {
        {"totalDurationMs", m_metadata.totalDurationMs},
        {"totalErrors", m_metadata.totalErrors}
    };
    m_steps.push_back(stopStep);
    m_metadata.totalSteps = static_cast<uint32_t>(m_steps.size());

    m_isRecording.store(false, std::memory_order_release);
    spdlog::info("[AIFlightRecorder] Session stopped. Total steps: {}, duration: {} ms",
                 m_metadata.totalSteps, m_metadata.totalDurationMs);
}

void AIFlightRecorder::RecordStep(FlightCategory category,
                                  const std::string& subsystem,
                                  const std::string& action,
                                  const std::string& details,
                                  const nlohmann::json& payload,
                                  int64_t durationMs) {
    if (!m_isRecording.load(std::memory_order_relaxed)) return;

    FlightStep step;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_isRecording.load(std::memory_order_relaxed)) return;

        auto now = std::chrono::steady_clock::now();
        step.stepNumber = static_cast<uint32_t>(m_steps.size() + 1);
        step.timestampIso = GetCurrentTimestampIso();
        step.elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_sessionStartPoint).count();
        step.category = category;
        step.subsystem = subsystem;
        step.action = action;
        step.details = SanitizeString(details);
        step.durationMs = durationMs;
        step.payload = SanitizeJson(payload);
        step.isError = (category == FlightCategory::ERROR_EVENT);

        if (step.isError) {
            m_metadata.totalErrors++;
        }

        // Track active subsystem list
        bool found = false;
        for (const auto& s : m_metadata.activeSubsystems) {
            if (s == subsystem) { found = true; break; }
        }
        if (!found && !subsystem.empty()) {
            m_metadata.activeSubsystems.push_back(subsystem);
        }

        m_steps.push_back(step);
        m_metadata.totalSteps = static_cast<uint32_t>(m_steps.size());
    }

    // Invoke callback without holding mutex
    if (m_stepCallback) {
        m_stepCallback(step);
    }
}

void AIFlightRecorder::RecordError(const std::string& subsystem,
                                  const std::string& context,
                                  const std::string& errorMsg) {
    nlohmann::json errPayload = {
        {"context", context},
        {"errorMessage", errorMsg}
    };
    RecordStep(FlightCategory::ERROR_EVENT, subsystem, "ERROR: " + context, errorMsg, errPayload);
}

void AIFlightRecorder::SetEnvironmentInfo(const std::string& osPlatform,
                                         const std::string& cpuInfo,
                                         const std::string& gpuRenderer,
                                         const std::string& activeLlmModel,
                                         float activeLlmTemp) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_metadata.osPlatform = osPlatform;
    m_metadata.cpuInfo = cpuInfo;
    m_metadata.gpuRenderer = gpuRenderer;
    m_metadata.activeLlmModel = activeLlmModel;
    m_metadata.activeLlmTemp = activeLlmTemp;
}

FlightSessionMetadata AIFlightRecorder::GetMetadata() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_metadata;
}

std::vector<FlightStep> AIFlightRecorder::GetSteps() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_steps;
}

uint32_t AIFlightRecorder::GetStepCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<uint32_t>(m_steps.size());
}

int64_t AIFlightRecorder::GetCurrentElapsedMs() const {
    if (!m_isRecording.load(std::memory_order_relaxed)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_metadata.totalDurationMs;
    }
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_sessionStartPoint).count();
}

void AIFlightRecorder::SetStepCallback(std::function<void(const FlightStep&)> callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stepCallback = std::move(callback);
}

std::string AIFlightRecorder::GenerateJsonTrace() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    nlohmann::json root;

    root["session"] = {
        {"id", m_metadata.sessionId},
        {"name", m_metadata.sessionName},
        {"startTime", m_metadata.startTimeIso},
        {"stopTime", m_metadata.stopTimeIso},
        {"durationMs", m_metadata.totalDurationMs},
        {"totalSteps", m_steps.size()},
        {"totalErrors", m_metadata.totalErrors},
        {"xLightsVersion", m_metadata.xLightsVersion},
        {"os", m_metadata.osPlatform},
        {"cpu", m_metadata.cpuInfo},
        {"gpu", m_metadata.gpuRenderer},
        {"activeModel", m_metadata.activeLlmModel},
        {"temperature", m_metadata.activeLlmTemp},
        {"subsystems", m_metadata.activeSubsystems}
    };

    nlohmann::json stepsArr = nlohmann::json::array();
    for (const auto& step : m_steps) {
        stepsArr.push_back({
            {"step", step.stepNumber},
            {"timestamp", step.timestampIso},
            {"elapsedMs", step.elapsedMs},
            {"category", FlightCategoryToString(step.category)},
            {"subsystem", step.subsystem},
            {"action", step.action},
            {"details", step.details},
            {"durationMs", step.durationMs},
            {"payload", step.payload},
            {"isError", step.isError}
        });
    }
    root["steps"] = stepsArr;

    return root.dump(2);
}

std::string AIFlightRecorder::GenerateHtmlReport() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::ostringstream ss;
    ss << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
       << "<meta charset=\"UTF-8\">\n"
       << "<title>xLights AI Flight Record - " << m_metadata.sessionName << "</title>\n"
       << "<style>\n"
       << "  :root { --bg: #0d1117; --panel: #161b22; --border: #30363d; --text: #c9d1d9; --accent: #58a6ff; --green: #3fb950; --amber: #d29922; --red: #f85149; --purple: #bc8cff; }\n"
       << "  * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Helvetica, Arial, sans-serif; }\n"
       << "  body { background: var(--bg); color: var(--text); padding: 24px; line-height: 1.5; }\n"
       << "  .header { display: flex; justify-content: space-between; align-items: center; border-bottom: 1px solid var(--border); padding-bottom: 16px; margin-bottom: 24px; }\n"
       << "  .title { font-size: 24px; font-weight: 700; color: #fff; }\n"
       << "  .badge { display: inline-block; padding: 4px 10px; border-radius: 12px; font-size: 12px; font-weight: 600; text-transform: uppercase; }\n"
       << "  .badge-success { background: rgba(63,185,80,0.2); color: var(--green); border: 1px solid var(--green); }\n"
       << "  .badge-error { background: rgba(248,81,73,0.2); color: var(--red); border: 1px solid var(--red); }\n"
       << "  .cards { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 16px; margin-bottom: 24px; }\n"
       << "  .card { background: var(--panel); border: 1px solid var(--border); border-radius: 8px; padding: 16px; }\n"
       << "  .card-label { font-size: 12px; color: #8b949e; text-transform: uppercase; margin-bottom: 4px; }\n"
       << "  .card-val { font-size: 20px; font-weight: 700; color: #fff; }\n"
       << "  .timeline { position: relative; margin-left: 20px; border-left: 2px solid var(--border); padding-left: 24px; }\n"
       << "  .step { position: relative; margin-bottom: 24px; background: var(--panel); border: 1px solid var(--border); border-radius: 8px; padding: 16px; }\n"
       << "  .step.error-step { border-color: var(--red); background: rgba(248,81,73,0.05); }\n"
       << "  .step-dot { position: absolute; left: -31px; top: 18px; width: 12px; height: 12px; border-radius: 50%; background: var(--accent); border: 2px solid var(--bg); }\n"
       << "  .step-dot.err { background: var(--red); }\n"
       << "  .step-header { display: flex; justify-content: space-between; margin-bottom: 8px; font-size: 13px; color: #8b949e; }\n"
       << "  .step-title { font-size: 16px; font-weight: 600; color: #fff; margin-bottom: 4px; }\n"
       << "  .step-cat { font-size: 11px; padding: 2px 6px; border-radius: 4px; background: rgba(88,166,255,0.15); color: var(--accent); font-weight: 600; }\n"
       << "  .step-details { font-size: 14px; margin-bottom: 8px; }\n"
       << "  pre { background: #010409; border: 1px solid var(--border); border-radius: 6px; padding: 12px; font-size: 12px; overflow-x: auto; color: #79c0ff; }\n"
       << "  details summary { cursor: pointer; color: var(--accent); font-size: 12px; font-weight: 500; margin-top: 6px; }\n"
       << "</style>\n</head>\n<body>\n"
       << "<div class=\"header\">\n"
       << "  <div>\n"
       << "    <div class=\"title\">📋 xLights AI Problem Steps & Flight Record</div>\n"
       << "    <div style=\"color: #8b949e; font-size: 13px;\">Session: " << m_metadata.sessionName << " (" << m_metadata.sessionId << ")</div>\n"
       << "  </div>\n"
       << "  <div>\n"
       << "    <span class=\"badge " << (m_metadata.totalErrors == 0 ? "badge-success" : "badge-error") << "\">"
       << (m_metadata.totalErrors == 0 ? "✓ 0 Anomalies" : "⚠️ " + std::to_string(m_metadata.totalErrors) + " Issues Detected")
       << "    </span>\n"
       << "  </div>\n"
       << "</div>\n"
       << "<div class=\"cards\">\n"
       << "  <div class=\"card\"><div class=\"card-label\">Total Steps</div><div class=\"card-val\">" << m_steps.size() << "</div></div>\n"
       << "  <div class=\"card\"><div class=\"card-label\">Duration</div><div class=\"card-val\">" << (m_metadata.totalDurationMs / 1000.0f) << "s</div></div>\n"
       << "  <div class=\"card\"><div class=\"card-label\">xLights Engine</div><div class=\"card-val\">" << m_metadata.xLightsVersion << "</div></div>\n"
       << "  <div class=\"card\"><div class=\"card-label\">Active Subsystems</div><div class=\"card-val\">" << m_metadata.activeSubsystems.size() << "</div></div>\n"
       << "</div>\n"
       << "<div class=\"timeline\">\n";

    for (const auto& step : m_steps) {
        ss << "  <div class=\"step " << (step.isError ? "error-step" : "") << "\">\n"
           << "    <div class=\"step-dot " << (step.isError ? "err" : "") << "\"></div>\n"
           << "    <div class=\"step-header\">\n"
           << "      <span>Step #" << step.stepNumber << " • " << step.subsystem << "</span>\n"
           << "      <span>+" << step.elapsedMs << " ms" << (step.durationMs > 0 ? " (Took " + std::to_string(step.durationMs) + " ms)" : "") << "</span>\n"
           << "    </div>\n"
           << "    <div class=\"step-title\">" << step.action << " <span class=\"step-cat\">" << FlightCategoryToString(step.category) << "</span></div>\n"
           << "    <div class=\"step-details\">" << step.details << "</div>\n";
        
        if (!step.payload.empty() && step.payload != nlohmann::json::object()) {
            ss << "    <details><summary>View Payload / State Dump</summary>\n"
               << "      <pre>" << step.payload.dump(2) << "</pre>\n"
               << "    </details>\n";
        }
        ss << "  </div>\n";
    }

    ss << "</div>\n"
       << "</body>\n</html>\n";

    return ss.str();
}

bool AIFlightRecorder::ExportSessionFolder(const std::string& targetDir) const {
    try {
        std::filesystem::path dir(targetDir);
        std::error_code ec;
        if (!std::filesystem::exists(dir, ec)) {
            std::filesystem::create_directories(dir, ec);
        }

        // 1. JSON trace
        std::ofstream jsonFile((dir / "session_trace.json").string());
        if (jsonFile.is_open()) {
            jsonFile << GenerateJsonTrace();
            jsonFile.close();
        }

        // 2. HTML report
        std::ofstream htmlFile((dir / "session_report.html").string());
        if (htmlFile.is_open()) {
            htmlFile << GenerateHtmlReport();
            htmlFile.close();
        }

        // 3. Summary text
        std::ofstream sumFile((dir / "session_summary.txt").string());
        if (sumFile.is_open()) {
            FlightSessionMetadata meta = GetMetadata();
            sumFile << "xLights AI Flight Record Summary\n"
                    << "================================\n"
                    << "Session ID: " << meta.sessionId << "\n"
                    << "Session Name: " << meta.sessionName << "\n"
                    << "xLights Build: " << meta.xLightsVersion << "\n"
                    << "Start Time: " << meta.startTimeIso << "\n"
                    << "Duration: " << (meta.totalDurationMs / 1000.0f) << " s\n"
                    << "Total Steps: " << meta.totalSteps << "\n"
                    << "Total Errors: " << meta.totalErrors << "\n"
                    << "Active Subsystems: " << meta.activeSubsystems.size() << "\n";
            sumFile.close();
        }

        spdlog::info("[AIFlightRecorder] Successfully exported session to: {}", targetDir);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("[AIFlightRecorder] Failed to export session folder: {}", e.what());
        return false;
    }
}

} // namespace xLights::AI
