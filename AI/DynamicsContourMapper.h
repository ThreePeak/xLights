#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "media/AudioDynamicsMapper.h"
#include "ai/AISubsystemBase.h"

namespace xLights::AI {

using AudioFrameContour = ::AudioFrameContour;
using AudioDynamicsContourResult = ::AudioDynamicsContourResult;
using DynamicsMapResult = ::AudioDynamicsContourResult;

class DynamicsContourMapper : public AISubsystemBase {
public:
    DynamicsContourMapper(ServiceManager* sm = nullptr) : AISubsystemBase(sm) {}
    virtual ~DynamicsContourMapper() override = default;

    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override {
        return std::async(std::launch::async, [this, callback]() {
            if (callback) callback("Initializing DynamicsContourMapper...", 0.0f);
            m_isInitialized.store(true);
            if (callback) callback("DynamicsContourMapper initialized.", 100.0f);
            return true;
        });
    }

    virtual void Shutdown() override {
        m_isInitialized.store(false);
    }

    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "DynamicsContourMapper"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"rms_dynamics_contour", "tempo_valence_arousal", "value_curve_export"};
    }

    [[nodiscard]] static AudioDynamicsContourResult AnalyzeDynamicsContour(
        const std::vector<float>& leftChannel,
        const std::vector<float>& rightChannel,
        size_t sampleRate,
        long framePeriodMS = 50,
        std::function<void(int pct)> progress = nullptr);

    [[nodiscard]] static AudioDynamicsContourResult AnalyzeDynamicsContour(
        AudioManager* audioManager,
        long framePeriodMS = 50,
        std::function<void(int pct)> progress = nullptr);

    /**
     * @brief Process full song audio track into a time-series DynamicsMapResult.
     */
    [[nodiscard]] static AudioDynamicsContourResult AnalyzeAudioDynamics(
        AudioManager* audioManager,
        long framePeriodMS = 50,
        std::function<void(int pct)> progress = nullptr);

using DynamicsMapResult = AudioDynamicsContourResult;

    [[nodiscard]] static std::string ExportAsValueCurveString(const AudioDynamicsContourResult& contour);

    [[nodiscard]] static std::string ExportAsValueCurveJson(const AudioDynamicsContourResult& contour);

    [[nodiscard]] static std::string ExportContourToValueCurveJSON(const DynamicsMapResult& dynamics, const std::string& metricName);
};

} // namespace xLights::AI
