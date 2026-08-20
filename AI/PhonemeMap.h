/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace xLights::AI {

/// xLights Standard 8-State Singing Face Visemes + REST
enum class VisemeState {
    REST = 0,
    AI,
    E,
    ETC,
    L,
    MBP,
    O,
    U,
    WQ
};

struct PhonemeTimeInterval {
    std::string phoneme;
    int64_t startMs{0};
    int64_t endMs{0};
    float confidence{1.0f};
};

struct VisemeTimingMark {
    VisemeState state{VisemeState::REST};
    std::string visemeName;
    int64_t startMs{0};
    int64_t endMs{0};
    float confidence{1.0f};
};

struct SingingFaceTrackResult {
    bool success{false};
    std::string trackName;
    std::vector<VisemeTimingMark> marks;
    int64_t totalDurationMs{0};
    std::string summary;

    nlohmann::json ToJson() const;
    std::string ToXTimingXml() const;
};

class PhonemeMap {
public:
    PhonemeMap() = default;
    ~PhonemeMap() = default;

    /// Maps a single phonetic symbol (ARPAbet, CMUDict, IPA) to xLights 8-state VisemeState
    static VisemeState PhonemeToViseme(const std::string& rawPhoneme);

    /// Converts a VisemeState enum to its canonical string representation ("AI", "E", "etc", "L", "MBP", "O", "U", "WQ", "rest")
    static std::string VisemeToString(VisemeState state);

    /// Converts a canonical string representation to a VisemeState enum
    static VisemeState StringToViseme(const std::string& visemeStr);

    /// Maps a sequence of timed phonemes into timed visemes
    static std::vector<VisemeTimingMark> MapPhonemesToVisemes(
        const std::vector<PhonemeTimeInterval>& phonemes
    );

    /// Applies temporal smoothing and debouncing to eliminate LED chatter on rapid phonemes
    static std::vector<VisemeTimingMark> SmoothVisemeTransitions(
        const std::vector<VisemeTimingMark>& rawMarks,
        int64_t minDurationMs = 30
    );

    /// Decomposes lyrics text into timed phonetic sequence using lexicon and letter-to-sound rules
    static std::vector<PhonemeTimeInterval> WordsToPhonemes(
        const std::string& lyricsText,
        int64_t totalDurationMs = 0
    );

    /// Complete pipeline: converts timed phonemes to smoothed singing face timing track
    static SingingFaceTrackResult GenerateSingingFaceTrack(
        const std::string& trackName,
        const std::vector<PhonemeTimeInterval>& phonemes,
        int64_t minDebounceDurationMs = 30
    );
};

} // namespace xLights::AI
