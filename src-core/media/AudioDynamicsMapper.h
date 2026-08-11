#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <functional>
#include <string>
#include <vector>

class AudioManager;

struct AudioEmotionFrame {
    long timeMS = 0;
    float rmsEnergy = 0.0f;       // Normalized 0.0 to 1.0
    float tempoBPM = 120.0f;       // Estimated local BPM
    float brightnessLevel = 50.0f; // Target brightness 0.0 to 100.0%
    float valence = 0.0f;          // Emotional valence -1.0 (melancholy) to +1.0 (joyous)
    float arousal = 0.5f;          // Emotional arousal 0.0 (calm) to 1.0 (intense)
    std::string recommendedVibe;   // Recommended color palette mood/vibe
};

struct AudioDynamicsContourResult {
    bool success = false;
    float averageBPM = 120.0f;
    float peakEnergy = 0.0f;
    std::string dominantEmotion;
    std::vector<AudioEmotionFrame> frames;
};

class AudioDynamicsMapper {
public:
    AudioDynamicsMapper() = default;
    ~AudioDynamicsMapper() = default;

    // Analyze audio PCM channels and compute time-series emotional & dynamics contour
    AudioDynamicsContourResult AnalyzeDynamicsContour(const std::vector<float>& leftChannel,
                                                      const std::vector<float>& rightChannel,
                                                      size_t sampleRate,
                                                      long framePeriodMS = 50,
                                                      std::function<void(int pct)> progress = nullptr);

    // Analyze directly from AudioManager
    AudioDynamicsContourResult AnalyzeDynamicsContour(AudioManager* audioManager,
                                                      long framePeriodMS = 50,
                                                      std::function<void(int pct)> progress = nullptr);

    // Export dynamics brightness contour as serialized xLights ValueCurve string
    static std::string ExportAsValueCurveString(const AudioDynamicsContourResult& contour);
};
