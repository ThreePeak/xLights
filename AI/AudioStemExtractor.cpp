/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AudioStemExtractor.h"
#include "spdlog/spdlog.h"
#include <pugixml.hpp>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace xLights::AI {

nlohmann::json StemExtractionResult::ToJson() const {
    nlohmann::json j;
    j["success"] = success;
    j["error_message"] = errorMessage;
    j["stems"] = nlohmann::json::array();
    for (const auto& stem : stems) {
        nlohmann::json sj;
        sj["stem_name"] = stem.stemName;
        sj["wav_file_path"] = stem.wavFilePath;
        sj["sample_rate"] = stem.sampleRate;
        sj["sample_count"] = stem.leftBuffer.size();
        j["stems"].push_back(sj);
    }
    j["timing_tracks"] = nlohmann::json::array();
    for (const auto& tt : timingTracks) {
        nlohmann::json tj;
        tj["track_name"] = tt.trackName;
        tj["mark_count"] = tt.marks.size();
        j["timing_tracks"].push_back(tj);
    }
    j["generated_timing_track_files"] = generatedTimingTrackFiles;
    return j;
}

AudioStemExtractor::AudioStemExtractor(ServiceManager* sm)
    : AISubsystemBase(sm) {}

std::future<bool> AudioStemExtractor::InitializeAsync(StatusCallback callback) {
    return std::async(std::launch::async, [this, callback]() {
        if (callback) callback("Initializing AudioStemExtractor...", 0.0f);
        m_isInitialized.store(true);
        if (callback) callback("AudioStemExtractor initialized.", 100.0f);
        return true;
    });
}

void AudioStemExtractor::Shutdown() {
    m_isInitialized.store(false);
}

StemExtractionResult AudioStemExtractor::ExtractStems(
    AudioManager* audioManager,
    const StemSeparationOptions& options,
    std::function<void(int pct)> progress,
    const std::atomic<bool>* cancel)
{
    StemExtractionResult result;

    if (!audioManager) {
        result.errorMessage = "Null AudioManager provided.";
        return result;
    }

    spdlog::info("AudioStemExtractor: Starting HTDemucs ONNX stem separation (model: {})", options.modelPath);

    AudioDecoder decoder;
    std::vector<StemTimingTrackResult> timingTracks;
    DemucsStemResult demucsRes = decoder.ProcessAudioFileStemSeparation(
        audioManager, options.modelPath, options.outputDirectory, timingTracks, progress, cancel
    );

    if (!demucsRes.success) {
        result.errorMessage = demucsRes.errorMessage.empty() ? "Demucs stem separation failed." : demucsRes.errorMessage;
        return result;
    }

    result.success = true;
    result.timingTracks = timingTracks;

    // Vocals Stem
    AudioStem vocals;
    vocals.type = StemType::VOCALS;
    vocals.stemName = "Vocals";
    vocals.wavFilePath = demucsRes.vocalStemPath;
    vocals.leftBuffer = demucsRes.stemBuffers.vocalsL;
    vocals.rightBuffer = demucsRes.stemBuffers.vocalsR;
    vocals.sampleRate = demucsRes.stemBuffers.sampleRate;
    result.stems.push_back(vocals);

    // Drums Stem
    AudioStem drums;
    drums.type = StemType::DRUMS;
    drums.stemName = "Drums";
    drums.wavFilePath = demucsRes.drumStemPath;
    drums.leftBuffer = demucsRes.stemBuffers.drumsL;
    drums.rightBuffer = demucsRes.stemBuffers.drumsR;
    drums.sampleRate = demucsRes.stemBuffers.sampleRate;
    result.stems.push_back(drums);

    // Bass Stem
    AudioStem bass;
    bass.type = StemType::BASS;
    bass.stemName = "Bass";
    bass.wavFilePath = demucsRes.bassStemPath;
    bass.leftBuffer = demucsRes.stemBuffers.bassL;
    bass.rightBuffer = demucsRes.stemBuffers.bassR;
    bass.sampleRate = demucsRes.stemBuffers.sampleRate;
    result.stems.push_back(bass);

    // Other Stem
    AudioStem other;
    other.type = StemType::OTHER;
    other.stemName = "Other";
    other.wavFilePath = demucsRes.otherStemPath;
    other.leftBuffer = demucsRes.stemBuffers.otherL;
    other.rightBuffer = demucsRes.stemBuffers.otherR;
    other.sampleRate = demucsRes.stemBuffers.sampleRate;
    result.stems.push_back(other);

    spdlog::info("AudioStemExtractor: Extracted {} audio stems and {} timing tracks successfully.", result.stems.size(), result.timingTracks.size());
    return result;
}

std::vector<float> AudioStemExtractor::ComputeRMSEnvelope(const AudioStem& stem, int framePeriodMS) {
    return AudioDecoder::ComputeRMSEnvelope(stem.leftBuffer, stem.rightBuffer, stem.sampleRate, framePeriodMS);
}

std::vector<StemTimingMark> AudioStemExtractor::ComputeSpectralFluxOnsets(
    const AudioStem& stem,
    int framePeriodMS,
    float transientSensitivity)
{
    return AudioDecoder::ComputeSpectralFluxOnsets(stem.leftBuffer, stem.rightBuffer, stem.sampleRate, framePeriodMS, transientSensitivity);
}

std::string AudioStemExtractor::CompileTimingTrackToXTimingXML(
    const StemTimingTrackResult& timingTrack,
    int markDurationMS)
{
    pugi::xml_document doc;

    // XML Declaration
    pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
    decl.append_attribute("version").set_value("1.0");
    decl.append_attribute("encoding").set_value("UTF-8");

    // Root Element: <timing name="TRACK_NAME" version="2">
    pugi::xml_node rootNode = doc.append_child("timing");
    rootNode.append_attribute("name").set_value(timingTrack.trackName.c_str());
    rootNode.append_attribute("version").set_value("2");

    // <EffectDB version="1">
    pugi::xml_node dbNode = rootNode.append_child("EffectDB");
    dbNode.append_attribute("version").set_value("1");

    for (const auto& mark : timingTrack.marks) {
        pugi::xml_node effNode = dbNode.append_child("Effect");
        effNode.append_attribute("label").set_value(mark.label.c_str());
        effNode.append_attribute("start").set_value(mark.timeMS);
        effNode.append_attribute("end").set_value(mark.timeMS + markDurationMS);
    }

    std::ostringstream ss;
    doc.save(ss, "  ");
    return ss.str();
}

} // namespace xLights::AI
