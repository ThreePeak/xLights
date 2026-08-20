/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PhonemeMap.h"
#include <algorithm>
#include <sstream>
#include <cctype>

namespace xLights::AI {

nlohmann::json SingingFaceTrackResult::ToJson() const {
    nlohmann::json j;
    j["success"] = success;
    j["track_name"] = trackName;
    j["total_duration_ms"] = totalDurationMs;
    j["summary"] = summary;

    nlohmann::json marksList = nlohmann::json::array();
    for (const auto& m : marks) {
        nlohmann::json item;
        item["viseme"] = m.visemeName;
        item["start_ms"] = m.startMs;
        item["end_ms"] = m.endMs;
        item["confidence"] = m.confidence;
        marksList.push_back(item);
    }
    j["marks"] = marksList;
    return j;
}

std::string SingingFaceTrackResult::ToXTimingXml() const {
    std::ostringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    ss << "<timing name=\"" << trackName << "\" SourceVersion=\"2026.04\">\n";
    ss << "  <EffectLayer>\n";
    for (const auto& m : marks) {
        ss << "    <Effect label=\"" << m.visemeName << "\" starttime=\""
           << m.startMs << "\" endtime=\"" << m.endMs << "\" />\n";
    }
    ss << "  </EffectLayer>\n";
    ss << "</timing>\n";
    return ss.str();
}

std::string PhonemeMap::VisemeToString(VisemeState state) {
    switch (state) {
        case VisemeState::AI:  return "AI";
        case VisemeState::E:   return "E";
        case VisemeState::ETC: return "etc";
        case VisemeState::L:   return "L";
        case VisemeState::MBP: return "MBP";
        case VisemeState::O:   return "O";
        case VisemeState::U:   return "U";
        case VisemeState::WQ:  return "WQ";
        case VisemeState::REST:
        default:               return "rest";
    }
}

VisemeState PhonemeMap::StringToViseme(const std::string& visemeStr) {
    std::string s = visemeStr;
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);

    if (s == "AI" || s == "AA" || s == "AY") return VisemeState::AI;
    if (s == "E" || s == "EH" || s == "EY")  return VisemeState::E;
    if (s == "ETC")                          return VisemeState::ETC;
    if (s == "L")                            return VisemeState::L;
    if (s == "MBP" || s == "M" || s == "B" || s == "P") return VisemeState::MBP;
    if (s == "O" || s == "OW" || s == "AO")  return VisemeState::O;
    if (s == "U" || s == "UW" || s == "UH")  return VisemeState::U;
    if (s == "WQ" || s == "W")               return VisemeState::WQ;
    return VisemeState::REST;
}

VisemeState PhonemeMap::PhonemeToViseme(const std::string& rawPhoneme) {
    if (rawPhoneme.empty()) return VisemeState::REST;

    // Normalize: strip trailing stress digits (e.g., "AA1" -> "AA", "EH0" -> "EH") and uppercase
    std::string clean;
    for (char c : rawPhoneme) {
        if (!std::isdigit(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
            clean += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
    }

    if (clean.empty() || clean == "SIL" || clean == "SP" || clean == "PAUSE" || clean == "REST") {
        return VisemeState::REST;
    }

    // ARPAbet mapping table:
    // AI: AA, AE, AH, AW, AY
    if (clean == "AA" || clean == "AE" || clean == "AH" || clean == "AW" || clean == "AY") {
        return VisemeState::AI;
    }
    // E: EH, EY, IH, IY, Y
    if (clean == "EH" || clean == "EY" || clean == "IH" || clean == "IY" || clean == "Y") {
        return VisemeState::E;
    }
    // O: AO, OW, OY
    if (clean == "AO" || clean == "OW" || clean == "OY") {
        return VisemeState::O;
    }
    // U: UH, UW
    if (clean == "UH" || clean == "UW") {
        return VisemeState::U;
    }
    // WQ: W, Q
    if (clean == "W" || clean == "Q") {
        return VisemeState::WQ;
    }
    // L: L, EL, R, ER, AX, AXR
    if (clean == "L" || clean == "EL" || clean == "R" || clean == "ER" || clean == "AX" || clean == "AXR") {
        return VisemeState::L;
    }
    // MBP: M, B, P, EM
    if (clean == "M" || clean == "B" || clean == "P" || clean == "EM") {
        return VisemeState::MBP;
    }
    // ETC: D, T, N, S, Z, SH, ZH, CH, JH, TH, DH, F, V, G, K, HH, NG, EN
    if (clean == "D" || clean == "T" || clean == "N" || clean == "S" || clean == "Z" ||
        clean == "SH" || clean == "ZH" || clean == "CH" || clean == "JH" ||
        clean == "TH" || clean == "DH" || clean == "F" || clean == "V" ||
        clean == "G" || clean == "K" || clean == "HH" || clean == "NG" || clean == "EN") {
        return VisemeState::ETC;
    }

    return VisemeState::ETC;
}

std::vector<VisemeTimingMark> PhonemeMap::MapPhonemesToVisemes(
    const std::vector<PhonemeTimeInterval>& phonemes
) {
    std::vector<VisemeTimingMark> marks;
    marks.reserve(phonemes.size());

    for (const auto& p : phonemes) {
        if (p.endMs <= p.startMs) continue;

        VisemeState state = PhonemeToViseme(p.phoneme);
        VisemeTimingMark mark;
        mark.state = state;
        mark.visemeName = VisemeToString(state);
        mark.startMs = p.startMs;
        mark.endMs = p.endMs;
        mark.confidence = p.confidence;
        marks.push_back(mark);
    }
    return marks;
}

std::vector<VisemeTimingMark> PhonemeMap::SmoothVisemeTransitions(
    const std::vector<VisemeTimingMark>& rawMarks,
    int64_t minDurationMs
) {
    if (rawMarks.empty()) return {};

    // First pass: Merge consecutive identical visemes
    std::vector<VisemeTimingMark> merged;
    merged.reserve(rawMarks.size());

    for (const auto& mark : rawMarks) {
        if (!merged.empty() && merged.back().state == mark.state && merged.back().endMs >= mark.startMs) {
            merged.back().endMs = std::max(merged.back().endMs, mark.endMs);
            merged.back().confidence = (merged.back().confidence + mark.confidence) * 0.5f;
        } else {
            merged.push_back(mark);
        }
    }

    // Second pass: Filter or merge sub-threshold brief chatter marks
    std::vector<VisemeTimingMark> smoothed;
    smoothed.reserve(merged.size());

    for (size_t i = 0; i < merged.size(); ++i) {
        int64_t duration = merged[i].endMs - merged[i].startMs;
        if (duration >= minDurationMs) {
            smoothed.push_back(merged[i]);
        } else {
            // Ultra-short chatter mark: absorb into previous mark if possible, or lengthen
            if (!smoothed.empty()) {
                smoothed.back().endMs = merged[i].endMs;
            } else if (i + 1 < merged.size()) {
                merged[i + 1].startMs = merged[i].startMs;
            } else {
                // If it's the only mark, keep it padded to minDurationMs
                auto padded = merged[i];
                padded.endMs = padded.startMs + minDurationMs;
                smoothed.push_back(padded);
            }
        }
    }

    return smoothed;
}

std::vector<PhonemeTimeInterval> PhonemeMap::WordsToPhonemes(
    const std::string& lyricsText,
    int64_t totalDurationMs
) {
    static const std::unordered_map<std::string, std::vector<std::string>> cmuDict = {
        {"MERRY", {"M", "EH", "R", "IY"}},
        {"CHRISTMAS", {"K", "R", "IH", "S", "M", "AH", "S"}},
        {"TO", {"T", "UW"}},
        {"ALL", {"AO", "L"}},
        {"AND", {"AE", "N", "D"}},
        {"A", {"AH"}},
        {"GOOD", {"G", "UH", "D"}},
        {"NIGHT", {"N", "AY", "T"}},
        {"HAPPY", {"HH", "AE", "P", "IY"}},
        {"HOLIDAY", {"HH", "AA", "L", "AH", "D", "EY"}},
        {"HOLIDAYS", {"HH", "AA", "L", "AH", "D", "EY", "Z"}},
        {"SILENT", {"S", "AY", "L", "AH", "N", "T"}},
        {"HOLY", {"HH", "OW", "L", "IY"}},
        {"JINGLE", {"JH", "IH", "NG", "G", "AH", "L"}},
        {"BELLS", {"B", "EH", "L", "Z"}},
        {"SANTA", {"S", "AE", "N", "T", "AH"}},
        {"CLAUS", {"K", "L", "AO", "Z"}},
        {"SNOW", {"S", "N", "OW"}},
        {"WINTER", {"W", "IH", "N", "T", "ER"}},
        {"WONDERLAND", {"W", "AH", "N", "D", "ER", "L", "AE", "N", "D"}},
        {"LIGHTS", {"L", "AY", "T", "S"}},
        {"SHOW", {"SH", "OW"}},
        {"MAGIC", {"M", "AE", "JH", "IH", "K"}},
        {"JOY", {"JH", "OY"}},
        {"PEACE", {"P", "IY", "S"}},
        {"LOVE", {"L", "AH", "V"}},
        {"STAR", {"S", "T", "AA", "R"}},
        {"TREE", {"T", "R", "IY"}},
        {"THE", {"DH", "AH"}},
        {"IS", {"IH", "Z"}},
        {"IT", {"IH", "T"}},
        {"YOU", {"Y", "UW"}},
        {"WE", {"W", "IY"}},
        {"WISH", {"W", "IH", "SH"}},
        {"DECK", {"D", "EH", "K"}},
        {"HALLS", {"HH", "AO", "L", "Z"}},
        {"LET", {"L", "EH", "T"}},
        {"SHINE", {"SH", "AY", "N"}},
        {"BRIGHT", {"B", "R", "AY", "T"}}
    };

    std::vector<std::string> rawTokens;
    std::string current;
    for (char c : lyricsText) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            current += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        } else if (!current.empty()) {
            rawTokens.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) {
        rawTokens.push_back(current);
    }

    if (rawTokens.empty()) {
        return { {"SIL", 0, 500, 1.0f} };
    }

    std::vector<std::string> sequencePhonemes;
    sequencePhonemes.push_back("SIL");

    for (const auto& token : rawTokens) {
        auto it = cmuDict.find(token);
        if (it != cmuDict.end()) {
            for (const auto& ph : it->second) {
                sequencePhonemes.push_back(ph);
            }
        } else {
            // Rule-based grapheme-to-phoneme fallback
            for (size_t i = 0; i < token.length(); ++i) {
                char ch = token[i];
                if (ch == 'A') sequencePhonemes.push_back("AE");
                else if (ch == 'E') sequencePhonemes.push_back("EH");
                else if (ch == 'I') sequencePhonemes.push_back("IH");
                else if (ch == 'O') sequencePhonemes.push_back("OW");
                else if (ch == 'U') sequencePhonemes.push_back("UW");
                else if (ch == 'B') sequencePhonemes.push_back("B");
                else if (ch == 'C' && i + 1 < token.length() && token[i+1] == 'H') { sequencePhonemes.push_back("CH"); i++; }
                else if (ch == 'C') sequencePhonemes.push_back("K");
                else if (ch == 'D') sequencePhonemes.push_back("D");
                else if (ch == 'F') sequencePhonemes.push_back("F");
                else if (ch == 'G') sequencePhonemes.push_back("G");
                else if (ch == 'H') sequencePhonemes.push_back("HH");
                else if (ch == 'J') sequencePhonemes.push_back("JH");
                else if (ch == 'K') sequencePhonemes.push_back("K");
                else if (ch == 'L') sequencePhonemes.push_back("L");
                else if (ch == 'M') sequencePhonemes.push_back("MBP");
                else if (ch == 'N') sequencePhonemes.push_back("N");
                else if (ch == 'P') sequencePhonemes.push_back("P");
                else if (ch == 'R') sequencePhonemes.push_back("R");
                else if (ch == 'S' && i + 1 < token.length() && token[i+1] == 'H') { sequencePhonemes.push_back("SH"); i++; }
                else if (ch == 'S') sequencePhonemes.push_back("S");
                else if (ch == 'T' && i + 1 < token.length() && token[i+1] == 'H') { sequencePhonemes.push_back("TH"); i++; }
                else if (ch == 'T') sequencePhonemes.push_back("T");
                else if (ch == 'V') sequencePhonemes.push_back("V");
                else if (ch == 'W') sequencePhonemes.push_back("W");
                else if (ch == 'Y') sequencePhonemes.push_back("Y");
                else if (ch == 'Z') sequencePhonemes.push_back("Z");
            }
        }
    }
    sequencePhonemes.push_back("SIL");

    int64_t duration = (totalDurationMs > 0) ? totalDurationMs : static_cast<int64_t>(sequencePhonemes.size() * 120);
    int64_t stepMs = duration / std::max<int64_t>(1, static_cast<int64_t>(sequencePhonemes.size()));

    std::vector<PhonemeTimeInterval> intervals;
    intervals.reserve(sequencePhonemes.size());

    int64_t curMs = 0;
    for (const auto& ph : sequencePhonemes) {
        PhonemeTimeInterval interval;
        interval.phoneme = ph;
        interval.startMs = curMs;
        interval.endMs = curMs + stepMs;
        interval.confidence = (ph == "SIL") ? 1.0f : 0.95f;
        intervals.push_back(interval);
        curMs += stepMs;
    }

    return intervals;
}

SingingFaceTrackResult PhonemeMap::GenerateSingingFaceTrack(
    const std::string& trackName,
    const std::vector<PhonemeTimeInterval>& phonemes,
    int64_t minDebounceDurationMs
) {
    SingingFaceTrackResult result;
    result.trackName = trackName.empty() ? "AI Singing Face Visemes" : trackName;

    if (phonemes.empty()) {
        result.success = false;
        result.summary = "No phonemes provided.";
        return result;
    }

    auto rawMarks = MapPhonemesToVisemes(phonemes);
    result.marks = SmoothVisemeTransitions(rawMarks, minDebounceDurationMs);

    if (!result.marks.empty()) {
        result.totalDurationMs = result.marks.back().endMs;
        result.success = true;
    }

    std::ostringstream ss;
    ss << "Generated " << result.marks.size() << " viseme marks across "
       << result.totalDurationMs << "ms with " << minDebounceDurationMs << "ms debouncing.";
    result.summary = ss.str();

    return result;
}

} // namespace xLights::AI
