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

struct AudioFrameContour {
    long timeMS = 0;
    int timestampMs = 0;           // Timestamp in milliseconds
    float rmsEnergy = 0.0f;       // Normalized 0.0 to 1.0
    float tempoBPM = 120.0f;       // Estimated local BPM
    float tempoBpm = 120.0f;       // Local BPM tracking
    float brightnessLevel = 50.0f; // Target brightness 0.0 to 100.0%
    float valence = 0.0f;          // Emotional valence -1.0 (melancholy) to +1.0 (joyous)
    float arousal = 0.5f;          // Emotional arousal 0.0 (calm) to 1.0 (intense)
    float harmonicTension = 0.0f;  // Dissonance vs Resolution (0.0 to 1.0)
    float spectralCentroid = 1000.0f; // Frequency brightness (Bass vs Treble)
    std::string recommendedVibe;   // Recommended color palette mood/vibe
};

using AudioEmotionFrame = AudioFrameContour;

struct AudioDynamicsContourResult {
    bool success = false;
    std::string songPath;                     // Path to analyzed audio file
    int totalDurationMs = 0;                  // Total track duration in milliseconds
    float averageBPM = 120.0f;
    float peakEnergy = 0.0f;
    std::string dominantEmotion;
    std::vector<AudioEmotionFrame> frames;
    std::vector<AudioFrameContour> contours;  // Frame contours list alias
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

    // Downsample 50ms frame arrays into Bezier control points compatible with ValueCurveDialog.cpp
    static std::vector<std::pair<float, float>> DownsampleContourToBezierControlPoints(
        const std::vector<AudioFrameContour>& frames,
        size_t maxControlPoints = 40);

    // Export dynamics brightness contour as serialized xLights ValueCurve string
    static std::string ExportAsValueCurveString(const AudioDynamicsContourResult& contour);

    // Export dynamics brightness contour as xLights ValueCurve JSON format
    static std::string ExportAsValueCurveJson(const AudioDynamicsContourResult& contour);
};
