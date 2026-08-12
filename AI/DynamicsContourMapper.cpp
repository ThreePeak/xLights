#include "DynamicsContourMapper.h"
#include <nlohmann/json.hpp>

namespace xLights::AI {

AudioDynamicsContourResult DynamicsContourMapper::AnalyzeDynamicsContour(
    const std::vector<float>& leftChannel,
    const std::vector<float>& rightChannel,
    size_t sampleRate,
    long framePeriodMS,
    std::function<void(int pct)> progress)
{
    AudioDynamicsMapper mapper;
    return mapper.AnalyzeDynamicsContour(leftChannel, rightChannel, sampleRate, framePeriodMS, progress);
}

AudioDynamicsContourResult DynamicsContourMapper::AnalyzeDynamicsContour(
    AudioManager* audioManager,
    long framePeriodMS,
    std::function<void(int pct)> progress)
{
    AudioDynamicsMapper mapper;
    return mapper.AnalyzeDynamicsContour(audioManager, framePeriodMS, progress);
}

AudioDynamicsContourResult DynamicsContourMapper::AnalyzeAudioDynamics(
    AudioManager* audioManager,
    long framePeriodMS,
    std::function<void(int pct)> progress)
{
    return AnalyzeDynamicsContour(audioManager, framePeriodMS, progress);
}

std::string DynamicsContourMapper::ExportAsValueCurveString(const AudioDynamicsContourResult& contour) {
    return AudioDynamicsMapper::ExportAsValueCurveString(contour);
}

std::string DynamicsContourMapper::ExportAsValueCurveJson(const AudioDynamicsContourResult& contour) {
    return AudioDynamicsMapper::ExportAsValueCurveJson(contour);
}

std::string DynamicsContourMapper::ExportContourToValueCurveJSON(const DynamicsMapResult& dynamics, const std::string& metricName) {
    nlohmann::json j;
    j["Type"] = "Custom";
    j["Points"] = nlohmann::json::array();

    if (dynamics.success && !dynamics.frames.empty()) {
        size_t total = dynamics.frames.size();
        for (size_t i = 0; i < total; ++i) {
            float normX = (total > 1) ? (float)i / (float)(total - 1) : 0.0f;
            float normY = 50.0f;
            const auto& f = dynamics.frames[i];

            if (metricName == "valence") {
                normY = (f.valence + 1.0f) * 50.0f; // Scale -1..1 to 0..100
            } else if (metricName == "arousal") {
                normY = f.arousal * 100.0f;
            } else if (metricName == "tension" || metricName == "harmonicTension") {
                normY = f.harmonicTension * 100.0f;
            } else { // "brightness", "rms", or default
                normY = f.brightnessLevel;
            }

            nlohmann::json pt;
            pt["x"] = normX;
            pt["y"] = normY;
            j["Points"].push_back(pt);
        }
    }
    return j.dump();
}

} // namespace xLights::AI
