#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <functional>
#include <map>
#include <string>
#include <vector>

struct AlignedPhoneme {
    std::string phoneme;   // e.g. "AA", "B", "SH", "IY"
    std::string viseme;    // e.g. "Rest", "AI", "E", "L", "M", "O", "U", "W", "etc"
    long startMS = 0;
    long endMS = 0;
    float confidence = 1.0f;
};

struct ForcedAlignmentResult {
    bool success = false;
    std::string transcriptText;
    std::vector<AlignedPhoneme> alignedPhonemes;
    std::string errorMessage;
};

class PhonemeMap {
public:
    PhonemeMap();
    ~PhonemeMap() = default;

    // Convert CMU / ARPAbet phoneme to xLights Singing Face state (Viseme)
    static std::string PhonemeToViseme(const std::string& phoneme);

    // Multimodal Whisper / Wav2Vec2 forced alignment of vocal audio & lyrics
    ForcedAlignmentResult AlignVocalPhonemes(const std::vector<float>& pcmFloatSamples,
                                             size_t sampleRate,
                                             const std::string& lyricText,
                                             std::function<void(int pct)> progress = nullptr);

    // Smooth rapid phoneme/viseme transitions to prevent LED display chatter
    static std::vector<AlignedPhoneme> SmoothVisemeTransitions(const std::vector<AlignedPhoneme>& rawPhonemes,
                                                                 long minVisemeDurationMS = 40);

private:
    std::map<std::string, std::string> _phonemeToVisemeMap;
    void InitVisemeMappings();
};
