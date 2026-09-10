#pragma once

#include <string>
#include <algorithm>
#include <cstdint>
#include <tuple>
#include "src-core/models/Model.h"

namespace xLights::AI {

struct ModelDimensionGuardResult {
    bool isAllowed{true};
    std::string originalEffect;
    std::string adaptedEffect;
    std::string reason;
};

class AIModelGeometryUtils {
public:
    /**
     * Check if buffer dimensions represent a 1-dimensional model (single line, strand, arch, or 1px strip).
     */
    static bool Is1DDimensions(int width, int height) {
        return (height <= 1 || width <= 1);
    }

    /**
     * Check if a model is 1-dimensional (single-line, strand, arch, or 1px tall/wide buffer).
     */
    static bool Is1DModel(const Model* model) {
        if (!model) return true;
        return Is1DDimensions(model->GetBufferWidth(), model->GetBufferHeight());
    }

    /**
     * Check if an effect requires a 2D matrix / surface / tree / canvas buffer.
     * Prevents 2D planar effects from rendering on 1D linear strands.
     */
    static bool EffectRequires2D(const std::string& effectName) {
        static const char* const s_2dEffects[] = {
            "Shockwave", "Bars", "Morph", "Spirals", "Fire", "Fireworks",
            "Matrix", "Fan", "Galaxy", "Picture", "Video", "Text", "Plasma",
            "Ripple", "Meteors", "Curtain", "Pinwheel", "Tree", "Shader",
            "Canvas", "Canvas Shader", "GLSL Shader", "ISF Shader",
            "Warp", "Marquee", "Shapes", "Life", "Spiro", "Tendril", "Liquid"
        };
        for (const auto* eff : s_2dEffects) {
            if (effectName == eff) return true;
        }
        return false;
    }

    /**
     * Verify whether an effect can safely render on given buffer dimensions.
     * Returns false if buffer is non-positive or if a 2D matrix effect is placed on a 1D model.
     */
    static bool CanRenderOnDimensions(int width, int height, const std::string& effectName) {
        if (width <= 0 || height <= 0) return false;
        if (Is1DDimensions(width, height) && EffectRequires2D(effectName)) return false;
        return true;
    }

    /**
     * Verify whether an effect can safely render on a target model.
     */
    static bool CanRenderOnModel(const Model* model, const std::string& effectName) {
        if (!model) return false;
        return CanRenderOnDimensions(model->GetBufferWidth(), model->GetBufferHeight(), effectName);
    }

    /**
     * Fallback from 2D-only matrix effects to 1D-compatible linear effects for given buffer dimensions.
     */
    static std::string AdaptEffectForDimensions(int width, int height, const std::string& effectName) {
        if (width <= 0 || height <= 0) {
            return "Off";
        }
        if (!Is1DDimensions(width, height) || !EffectRequires2D(effectName)) {
            return effectName;
        }

        // 1D adaptations for 2D matrix/shader effects
        if (effectName == "Bars" || effectName == "Curtain" || effectName == "Picture" || effectName == "Video") {
            return "Color Wash";
        } else if (effectName == "Shader" || effectName == "Canvas Shader" || effectName == "Canvas" ||
                   effectName == "GLSL Shader" || effectName == "ISF Shader") {
            return "Color Wash";
        } else if (effectName == "Fire" || effectName == "Fireworks" || effectName == "Plasma") {
            return "Twinkle";
        } else if (effectName == "Text") {
            return "SingleStrand";
        } else if (effectName == "Shockwave" || effectName == "Meteors" || effectName == "Spirals" ||
                   effectName == "Morph" || effectName == "Matrix" || effectName == "Fan" ||
                   effectName == "Galaxy" || effectName == "Pinwheel" || effectName == "Ripple" ||
                   effectName == "Tree" || effectName == "Warp" || effectName == "Shapes" ||
                   effectName == "Life" || effectName == "Spiro" || effectName == "Tendril" ||
                   effectName == "Liquid") {
            return "SingleStrand";
        }
        return "SingleStrand";
    }

    /**
     * Fallback from 2D-only effects to 1D-compatible effects if the target model has height or width <= 1.
     */
    static std::string AdaptEffectForModel(const Model* model, const std::string& effectName) {
        if (!model) return "Off";
        return AdaptEffectForDimensions(model->GetBufferWidth(), model->GetBufferHeight(), effectName);
    }

    /**
     * Evaluate and validate effect placement against target buffer dimensions with detailed diagnostics.
     */
    static ModelDimensionGuardResult ValidateEffectPlacement(int width, int height, const std::string& effectName) {
        ModelDimensionGuardResult result;
        result.originalEffect = effectName;
        result.adaptedEffect = effectName;

        if (width <= 0 || height <= 0) {
            result.isAllowed = false;
            result.adaptedEffect = "Off";
            result.reason = "Buffer has zero or negative dimensions (" + std::to_string(width) + "x" + std::to_string(height) + "). Effect suppressed to 'Off'.";
            return result;
        }

        if (Is1DDimensions(width, height) && EffectRequires2D(effectName)) {
            result.isAllowed = false;
            result.adaptedEffect = AdaptEffectForDimensions(width, height, effectName);
            result.reason = "2D matrix/shader effect '" + effectName + "' cannot render on 1D linear model (" +
                            std::to_string(width) + "x" + std::to_string(height) + "). Adapted to 1D effect '" +
                            result.adaptedEffect + "'.";
            return result;
        }

        result.isAllowed = true;
        result.reason = "Effect '" + effectName + "' is compatible with buffer dimensions (" +
                        std::to_string(width) + "x" + std::to_string(height) + ").";
        return result;
    }

    /**
     * Explicit RGB clamping between 0 and 255 to guard against color overflow.
     */
    static std::tuple<uint8_t, uint8_t, uint8_t> ClampRGB(int r, int g, int b) {
        uint8_t cr = static_cast<uint8_t>(std::clamp(r, 0, 255));
        uint8_t cg = static_cast<uint8_t>(std::clamp(g, 0, 255));
        uint8_t cb = static_cast<uint8_t>(std::clamp(b, 0, 255));
        return {cr, cg, cb};
    }
};

} // namespace xLights::AI
