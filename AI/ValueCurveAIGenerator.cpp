/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ValueCurveAIGenerator.h"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace xLights::AI {

ValueCurveAIGenerator::ValueCurveAIGenerator(ServiceManager* sm)
    : AISubsystemBase(sm) {}

std::future<bool> ValueCurveAIGenerator::InitializeAsync(StatusCallback callback) {
    return std::async(std::launch::async, [this, callback]() {
        if (callback) callback("Initializing ValueCurveAIGenerator...", 0.0f);
        m_isInitialized.store(true);
        if (callback) callback("ValueCurveAIGenerator initialized.", 100.0f);
        return true;
    });
}

void ValueCurveAIGenerator::Shutdown() {
    m_isInitialized.store(false);
}

std::string ValueCurveAIGenerator::GenerateCurveJsonFromPrompt(const std::string& prompt, float minVal, float maxVal) {
    std::lock_guard<std::mutex> lock(m_subsystemMutex);
    spdlog::info("[ValueCurveAIGenerator] Synthesizing JSON curve for prompt: '{}'", prompt);

    std::vector<Point2D> points = SynthesizePoints(prompt, minVal, maxVal);

    std::ostringstream json;
    json << std::fixed << std::setprecision(2);
    json << "{\n";
    json << "  \"Type\": \"Custom\",\n";
    json << "  \"Min\": " << minVal << ",\n";
    json << "  \"Max\": " << maxVal << ",\n";
    json << "  \"Points\": [\n";
    for (size_t i = 0; i < points.size(); ++i) {
        json << "    {\"x\": " << points[i].x << ", \"y\": " << points[i].y << "}";
        if (i + 1 < points.size()) json << ",";
        json << "\n";
    }
    json << "  ]\n";
    json << "}";

    return json.str();
}

std::vector<Point2D> ValueCurveAIGenerator::SynthesizePoints(const std::string& prompt, float minVal, float maxVal) {
    std::string lowerPrompt = prompt;
    std::transform(lowerPrompt.begin(), lowerPrompt.end(), lowerPrompt.begin(), ::tolower);

    std::vector<Point2D> points;
    const int numSteps = 20;

    float peak = maxVal;
    if (lowerPrompt.find("peaking at 90") != std::string::npos || lowerPrompt.find("peak at 90") != std::string::npos) {
        peak = 90.0f;
    } else if (lowerPrompt.find("peaking at 50") != std::string::npos) {
        peak = 50.0f;
    }

    if (lowerPrompt.find("exponential") != std::string::npos || lowerPrompt.find("exp") != std::string::npos) {
        for (int i = 0; i <= numSteps; ++i) {
            float x = i / static_cast<float>(numSteps);
            float y = minVal + (std::pow(x, 2.5f)) * (peak - minVal);
            points.push_back({x, y});
        }
    } else if (lowerPrompt.find("logarithmic") != std::string::npos || lowerPrompt.find("log") != std::string::npos) {
        for (int i = 0; i <= numSteps; ++i) {
            float x = i / static_cast<float>(numSteps);
            float y = minVal + std::log1p(x * 1.718f) * (peak - minVal);
            points.push_back({x, y});
        }
    } else if (lowerPrompt.find("sine") != std::string::npos || lowerPrompt.find("wave") != std::string::npos) {
        float cycles = 1.0f;
        if (lowerPrompt.find("3 cycle") != std::string::npos) cycles = 3.0f;
        else if (lowerPrompt.find("2 cycle") != std::string::npos) cycles = 2.0f;

        for (int i = 0; i <= numSteps; ++i) {
            float x = i / static_cast<float>(numSteps);
            float sineVal = (std::sin(x * cycles * 6.2831853f - 1.5707963f) + 1.0f) * 0.5f;
            float y = minVal + sineVal * (peak - minVal);
            points.push_back({x, y});
        }
    } else {
        // Linear Ramp Default
        for (int i = 0; i <= numSteps; ++i) {
            float x = i / static_cast<float>(numSteps);
            float y = minVal + x * (peak - minVal);
            points.push_back({x, y});
        }
    }

    return points;
}

std::string ValueCurveAIGenerator::JsonToSerializedValueCurve(const std::string& jsonPayload) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "Active=TRUE|Type=Custom|Min=0.00|Max=100.00|Values=";

    // Parse "x": <val>, "y": <val> pairs from JSON string dynamically
    bool first = true;
    size_t pos = 0;
    while ((pos = jsonPayload.find("\"x\":", pos)) != std::string::npos) {
        pos += 4;
        while (pos < jsonPayload.length() && (jsonPayload[pos] == ' ' || jsonPayload[pos] == '\t')) pos++;
        size_t endX = jsonPayload.find_first_of(",}\n\r\t ", pos);
        if (endX == std::string::npos) break;
        float xVal = std::strtof(jsonPayload.substr(pos, endX - pos).c_str(), nullptr);

        size_t yPos = jsonPayload.find("\"y\":", endX);
        if (yPos == std::string::npos) break;
        yPos += 4;
        while (yPos < jsonPayload.length() && (jsonPayload[yPos] == ' ' || jsonPayload[yPos] == '\t')) yPos++;
        size_t endY = jsonPayload.find_first_of(",}\n\r\t ", yPos);
        if (endY == std::string::npos) break;
        float yVal = std::strtof(jsonPayload.substr(yPos, endY - yPos).c_str(), nullptr);

        if (!first) ss << ";";
        ss << std::setprecision(2) << xVal << ":" << yVal;
        first = false;
        pos = endY;
    }

    ss << "|";
    return ss.str();
}

} // namespace xLights::AI
