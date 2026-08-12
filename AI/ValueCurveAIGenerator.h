#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "../src-core/ai/AISubsystemBase.h"
#include "DynamicsContourMapper.h"
#include <string>
#include <vector>
#include <memory>

namespace xLights::AI {

struct Point2D {
    float x;
    float y;
};

/**
 * @brief Translates natural language prompts into normalized JSON point arrays
 * and xLights serialized ValueCurve pipe payloads.
 */
class ValueCurveAIGenerator : public AISubsystemBase {
public:
    ValueCurveAIGenerator(ServiceManager* sm = nullptr);
    virtual ~ValueCurveAIGenerator() override = default;

    // AISubsystemBase lifecycle implementation
    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override;
    virtual void Shutdown() override;
    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "ValueCurveAIGenerator"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"natural_language_bezier", "curve_json_synthesis", "xvc_serialization"};
    }

    /**
     * @brief Translates a prompt into a normalized JSON string payload: {"Type": "Custom", "Points": [...]}.
     */
    [[nodiscard]] std::string GenerateCurveJsonFromPrompt(const std::string& prompt, float minVal = 0.0f, float maxVal = 100.0f);

    /**
     * @brief Converts a curve JSON payload into an xLights serialized pipe string (`Active=TRUE|Type=Custom|...`).
     */
    [[nodiscard]] static std::string JsonToSerializedValueCurve(const std::string& jsonPayload);

    /**
     * @brief Auto-binds song dynamics contour into an xLights ValueCurve JSON payload.
     */
    [[nodiscard]] static std::string GenerateCurveJsonFromAudioDynamics(const AudioDynamicsContourResult& dynamics, const std::string& metricName = "brightness");

    /**
     * @brief Auto-binds effect brightness or speed sliders directly to song dynamics into an xLights serialized pipe string.
     */
    [[nodiscard]] static std::string BindAudioDynamicsToValueCurve(const AudioDynamicsContourResult& dynamics, const std::string& metricName = "brightness");

private:
    std::vector<Point2D> SynthesizePoints(const std::string& prompt, float minVal, float maxVal);
};

} // namespace xLights::AI
