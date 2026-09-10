#pragma once

#include <string>
#include <algorithm>
#include <cstdint>
#include <tuple>
#include "src-core/models/Model.h"

namespace xLights::AI {

class AIModelGeometryUtils {
public:
    /**
     * Check if a model is 1-dimensional (single-line, strand, arch, or 1px tall buffer).
     */
    static bool Is1DModel(const Model* model) {
        if (!model) return true;
        return model->GetBufferHeight() <= 1;
    }

    /**
     * Check if an effect requires a 2D matrix / surface / tree buffer.
     */
    static bool EffectRequires2D(const std::string& effectName) {
        static const char* const s_2dEffects[] = {
            "Shockwave", "Bars", "Morph", "Spirals", "Fire", "Fireworks",
            "Matrix", "Fan", "Galaxy", "Picture", "Video", "Text", "Plasma",
            "Ripple", "Meteors", "Curtain", "Pinwheel", "Tree"
        };
        for (const auto* eff : s_2dEffects) {
            if (effectName == eff) return true;
        }
        return false;
    }

    /**
     * Fallback from 2D-only effects to 1D-compatible effects if the target model has height <= 1.
     */
    static std::string AdaptEffectForModel(const Model* model, const std::string& effectName) {
        if (!Is1DModel(model)) {
            return effectName;
        }
        if (!EffectRequires2D(effectName)) {
            return effectName;
        }

        // Fallbacks for 1D models
        if (effectName == "Shockwave" || effectName == "Meteors" || effectName == "Spirals") {
            return "SingleStrand";
        } else if (effectName == "Bars" || effectName == "Curtain") {
            return "Color Wash";
        } else if (effectName == "Fire" || effectName == "Fireworks" || effectName == "Plasma") {
            return "Twinkle";
        }
        return "SingleStrand";
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
