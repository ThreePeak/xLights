#pragma once

#include <cmath>
#include <algorithm>
#include <cstdint>
#include <utility>

namespace xLights::AI {

struct QuantizedInterval {
    double startSec = 0.0;
    double endSec = 0.05;
    int32_t startFrame = 0;
    int32_t endFrame = 1;
    double durationSec = 0.05;
    int32_t durationFrames = 1;
};

class AIQuantizationUtils {
public:
    /**
     * Quantize raw start and end timestamps (seconds) to exact sequence frame boundaries.
     * Prevents negative timestamps, zero-length intervals, and exceeding max sequence duration.
     * 
     * @param startSec Raw start time in seconds.
     * @param endSec Raw end time in seconds.
     * @param stepTimeMS Frame duration in milliseconds (typically 50ms for 20fps or 25ms for 40fps).
     * @param maxDurationSec Total sequence duration in seconds (or <= 0 if unconstrained).
     */
    static QuantizedInterval QuantizeTimeInterval(double startSec, double endSec, int stepTimeMS, double maxDurationSec = -1.0) {
        if (stepTimeMS <= 0) {
            stepTimeMS = 50; // default 50ms (20fps)
        }
        const double stepSec = static_cast<double>(stepTimeMS) / 1000.0;

        // Clamp start to non-negative
        startSec = std::max(0.0, startSec);
        if (maxDurationSec > 0.0) {
            startSec = std::min(startSec, std::max(0.0, maxDurationSec - stepSec));
        }

        // Calculate start frame
        int32_t sFrame = static_cast<int32_t>(std::round(startSec / stepSec));
        if (sFrame < 0) sFrame = 0;

        // Ensure endSec is at least startSec + stepSec
        if (endSec <= startSec) {
            endSec = startSec + stepSec;
        }

        // Calculate end frame
        int32_t eFrame = static_cast<int32_t>(std::round(endSec / stepSec));
        if (eFrame <= sFrame) {
            eFrame = sFrame + 1;
        }

        // Clamp to maxDuration if provided
        if (maxDurationSec > 0.0) {
            int32_t maxFrames = static_cast<int32_t>(std::ceil(maxDurationSec / stepSec));
            if (maxFrames > 0) {
                if (sFrame >= maxFrames) {
                    sFrame = std::max(0, maxFrames - 1);
                    eFrame = maxFrames;
                } else if (eFrame > maxFrames) {
                    eFrame = maxFrames;
                }
            }
        }

        QuantizedInterval result;
        result.startFrame = sFrame;
        result.endFrame = eFrame;
        result.durationFrames = eFrame - sFrame;
        result.startSec = sFrame * stepSec;
        result.endSec = eFrame * stepSec;
        result.durationSec = result.endSec - result.startSec;

        return result;
    }

    /**
     * Quantize frame indices ensuring endFrame > startFrame and within bounds.
     */
    static std::pair<int32_t, int32_t> ClampFrames(int32_t startFrame, int32_t endFrame, int32_t maxFrames = -1) {
        if (startFrame < 0) startFrame = 0;
        if (maxFrames > 0 && startFrame >= maxFrames) {
            startFrame = std::max(0, maxFrames - 1);
        }
        if (endFrame <= startFrame) {
            endFrame = startFrame + 1;
        }
        if (maxFrames > 0 && endFrame > maxFrames) {
            endFrame = maxFrames;
        }
        return {startFrame, endFrame};
    }
};

} // namespace xLights::AI
