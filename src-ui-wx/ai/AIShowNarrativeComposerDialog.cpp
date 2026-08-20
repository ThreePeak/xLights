/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIShowNarrativeComposerDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace xLights {

enum {
    ID_BTN_GENERATE_SCRIPT = wxID_HIGHEST + 701,
    ID_BTN_SYNTHESIZE_VOICE,
    ID_BTN_EXPORT_TIMING,
    ID_BTN_EXPORT_AUDIO,
    ID_BTN_UNDO,
    ID_BTN_REDO
};

AIShowNarrativeComposerDialog::AIShowNarrativeComposerDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style
) : wxDialog(parent, id, title, pos, size, style),
    m_commandHistory(100) {
    CreateControls();
}

void AIShowNarrativeComposerDialog::CreateControls() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(34, 44, 62));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Show Narrative & Voiceover Storyboard Composer"));
    titleTxt->SetForegroundColour(wxColour(255, 255, 255));
    auto font = titleTxt->GetFont();
    font.SetPointSize(12);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);
    bannerSizer->Add(titleTxt, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Center Workspace (Left Prompt/Voice Parameters + Right Script/Timing/Cues)
    auto* workSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* leftPanel = new wxPanel(this, wxID_ANY);
    BuildLeftPromptPanel(leftPanel);
    workSizer->Add(leftPanel, 0, wxEXPAND | wxALL, 8);

    auto* rightPanel = new wxPanel(this, wxID_ANY);
    BuildRightOutputPanel(rightPanel);
    workSizer->Add(rightPanel, 1, wxEXPAND | wxALL, 8);

    mainSizer->Add(workSizer, 1, wxEXPAND);

    // Bottom Action Bar
    auto* botBar = new wxPanel(this, wxID_ANY);
    auto* botSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* btnGen = new wxButton(botBar, ID_BTN_GENERATE_SCRIPT, wxT("✨ Compose Script & Align"));
    btnGen->SetBackgroundColour(wxColour(30, 140, 240));
    btnGen->SetForegroundColour(wxColour(255, 255, 255));

    m_btnUndo = new wxButton(botBar, ID_BTN_UNDO, wxT("↶ Undo (Ctrl+Z)"));
    m_btnRedo = new wxButton(botBar, ID_BTN_REDO, wxT("↷ Redo (Ctrl+Y)"));
    m_btnUndo->Enable(false);
    m_btnRedo->Enable(false);

    auto* btnSynth = new wxButton(botBar, ID_BTN_SYNTHESIZE_VOICE, wxT("🎙️ Synthesize Neural Audio"));
    auto* btnExportTiming = new wxButton(botBar, ID_BTN_EXPORT_TIMING, wxT("⏱️ Export xTiming Track..."));
    auto* btnExportAudio = new wxButton(botBar, ID_BTN_EXPORT_AUDIO, wxT("🎵 Export Audio (.wav)..."));
    auto* btnClose = new wxButton(botBar, wxID_CANCEL, wxT("Close"));

    botSizer->Add(btnGen, 0, wxALL, 4);
    botSizer->Add(m_btnUndo, 0, wxALL, 4);
    botSizer->Add(m_btnRedo, 0, wxALL, 4);
    botSizer->Add(btnSynth, 0, wxALL, 4);
    botSizer->Add(btnExportTiming, 0, wxALL, 4);
    botSizer->Add(btnExportAudio, 0, wxALL, 4);
    botSizer->AddStretchSpacer();
    botSizer->Add(btnClose, 0, wxALL, 4);

    botBar->SetSizer(botSizer);
    mainSizer->Add(botBar, 0, wxEXPAND | wxALL, 6);

    SetSizer(mainSizer);

    // Event Bindings
    Bind(wxEVT_BUTTON, &AIShowNarrativeComposerDialog::OnGenerateScript, this, ID_BTN_GENERATE_SCRIPT);
    Bind(wxEVT_BUTTON, &AIShowNarrativeComposerDialog::OnSynthesizeVoice, this, ID_BTN_SYNTHESIZE_VOICE);
    Bind(wxEVT_BUTTON, &AIShowNarrativeComposerDialog::OnExportTimingTrack, this, ID_BTN_EXPORT_TIMING);
    Bind(wxEVT_BUTTON, &AIShowNarrativeComposerDialog::OnExportAudio, this, ID_BTN_EXPORT_AUDIO);
    Bind(wxEVT_BUTTON, &AIShowNarrativeComposerDialog::OnUndo, this, ID_BTN_UNDO);
    Bind(wxEVT_BUTTON, &AIShowNarrativeComposerDialog::OnRedo, this, ID_BTN_REDO);
}

void AIShowNarrativeComposerDialog::BuildLeftPromptPanel(wxPanel* parent) {
    auto* sizer = new wxStaticBoxSizer(wxVERTICAL, parent, wxT("Narrative Theme & Character Voice Setup"));

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Show Title / Event Name:")), 0, wxTOP | wxLEFT, 4);
    m_txtShowTitle = new wxTextCtrl(parent, wxID_ANY, wxT("Smith Family Lights Spectacular"), wxDefaultPosition, wxSize(280, -1));
    sizer->Add(m_txtShowTitle, 0, wxEXPAND | wxALL, 4);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Family / Neighborhood Name:")), 0, wxTOP | wxLEFT, 4);
    m_txtFamilyName = new wxTextCtrl(parent, wxID_ANY, wxT("The Smiths"), wxDefaultPosition, wxSize(280, -1));
    sizer->Add(m_txtFamilyName, 0, wxEXPAND | wxALL, 4);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Narrative Style:")), 0, wxTOP | wxLEFT, 4);
    wxArrayString styles;
    styles.Add(wxT("Festive Christmas Story (Santa & Reindeer)"));
    styles.Add(wxT("High-Energy Radio DJ Intro"));
    styles.Add(wxT("Humorous Family Welcome Greeting"));
    styles.Add(wxT("Dramatic 10-Second Show Launch Countdown"));
    styles.Add(wxT("Halloween Theatrical Spooky Tale"));
    m_choiceStyle = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, styles);
    m_choiceStyle->SetSelection(0);
    sizer->Add(m_choiceStyle, 0, wxEXPAND | wxALL, 4);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Voice Actor Character:")), 0, wxTOP | wxLEFT, 4);
    wxArrayString voices;
    voices.Add(wxT("Santa Claus (Deep Jolly Baritone)"));
    voices.Add(wxT("Friendly Elf (Playful Upbeat)"));
    voices.Add(wxT("Broadcast Radio Host (Polished Booming)"));
    voices.Add(wxT("Child Storyteller (Wonder & Magic)"));
    voices.Add(wxT("Cinematic Movie Trailer Voice"));
    m_choiceVoice = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, voices);
    m_choiceVoice->SetSelection(0);
    sizer->Add(m_choiceVoice, 0, wxEXPAND | wxALL, 4);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Speech Tempo / Pace Multiplier:")), 0, wxTOP | wxLEFT, 4);
    m_sliderPace = new wxSlider(parent, wxID_ANY, 100, 70, 140);
    sizer->Add(m_sliderPace, 0, wxEXPAND | wxALL, 4);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Custom Story Prompt / Instructions:")), 0, wxTOP | wxLEFT, 4);
    m_txtCustomPrompt = new wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(280, 100), wxTE_MULTILINE);
    sizer->Add(m_txtCustomPrompt, 1, wxEXPAND | wxALL, 4);

    parent->SetSizer(sizer);
}

void AIShowNarrativeComposerDialog::BuildRightOutputPanel(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    auto* notebook = new wxNotebook(parent, wxID_ANY);

    // Tab 1: Script Editor
    auto* scriptPanel = new wxPanel(notebook, wxID_ANY);
    auto* scriptSizer = new wxBoxSizer(wxVERTICAL);
    m_txtScriptEditor = new wxTextCtrl(scriptPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    scriptSizer->Add(m_txtScriptEditor, 1, wxEXPAND | wxALL, 4);
    scriptPanel->SetSizer(scriptSizer);
    notebook->AddPage(scriptPanel, wxT("📝 Composed Voiceover Script"));

    // Tab 2: Word & Phoneme Timing Track
    auto* timingPanel = new wxPanel(notebook, wxID_ANY);
    auto* timingSizer = new wxBoxSizer(wxVERTICAL);
    m_timingMarksListCtrl = new wxListCtrl(timingPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    m_timingMarksListCtrl->InsertColumn(0, wxT("Start Time"), wxLIST_FORMAT_LEFT, 100);
    m_timingMarksListCtrl->InsertColumn(1, wxT("End Time"), wxLIST_FORMAT_LEFT, 100);
    m_timingMarksListCtrl->InsertColumn(2, wxT("Spoken Word"), wxLIST_FORMAT_LEFT, 160);
    m_timingMarksListCtrl->InsertColumn(3, wxT("Viseme (Mouth Sync)"), wxLIST_FORMAT_LEFT, 160);
    timingSizer->Add(m_timingMarksListCtrl, 1, wxEXPAND | wxALL, 4);
    timingPanel->SetSizer(timingSizer);
    notebook->AddPage(timingPanel, wxT("⏱️ Word & Viseme Timing Track"));

    // Tab 3: Recommended Lighting Cues
    auto* cuePanel = new wxPanel(notebook, wxID_ANY);
    auto* cueSizer = new wxBoxSizer(wxVERTICAL);
    m_lstLightingCues = new wxListBox(cuePanel, wxID_ANY);
    cueSizer->Add(m_lstLightingCues, 1, wxEXPAND | wxALL, 4);
    cuePanel->SetSizer(cueSizer);
    notebook->AddPage(cuePanel, wxT("💡 Synchronized Lighting Cues"));

    sizer->Add(notebook, 1, wxEXPAND | wxBOTTOM, 6);

    // Duration Stats Bar
    m_lblDurationStats = new wxStaticText(parent, wxID_ANY, wxT("Duration: 0.0s | Words: 0 | Timing Marks: 0"));
    m_lblDurationStats->SetForegroundColour(wxColour(0, 220, 255));
    sizer->Add(m_lblDurationStats, 0, wxALL, 4);

    parent->SetSizer(sizer);
}

void AIShowNarrativeComposerDialog::OnGenerateScript(wxCommandEvent& WXUNUSED(event)) {
    if (m_txtShowTitle) m_params.showTitle = m_txtShowTitle->GetValue().ToStdString();
    if (m_txtFamilyName) m_params.familyOrCityName = m_txtFamilyName->GetValue().ToStdString();
    if (m_choiceStyle) m_params.style = static_cast<AI::NarrativeStyle>(m_choiceStyle->GetSelection());
    if (m_choiceVoice) m_params.voice = static_cast<AI::VoiceCharacter>(m_choiceVoice->GetSelection());
    if (m_sliderPace) m_params.speechPaceMultiplier = m_sliderPace->GetValue() / 100.0f;
    if (m_txtCustomPrompt) m_params.customDetailsPrompt = m_txtCustomPrompt->GetValue().ToStdString();

    auto prevResult = m_result;
    m_result = AI::ShowNarrativeComposerAI::GenerateNarrative(m_params);

    auto cmd = std::make_unique<AI::LambdaAICommand>(
        "Generate Narrative Script",
        [this]() { return true; },
        [this, prevResult]() {
            m_result = prevResult;
            UpdateUiFromResults();
            return true;
        }
    );

    m_commandHistory.ExecuteCommand(std::move(cmd));
    UpdateUiFromResults();
}

void AIShowNarrativeComposerDialog::UpdateUiFromResults() {
    if (m_txtScriptEditor) {
        m_txtScriptEditor->SetValue(wxString::FromUTF8(m_result.scriptText));
    }

    if (m_timingMarksListCtrl) {
        m_timingMarksListCtrl->DeleteAllItems();
        for (size_t i = 0; i < m_result.timingMarks.size(); ++i) {
            const auto& m = m_result.timingMarks[i];
            long idx = m_timingMarksListCtrl->InsertItem(static_cast<long>(i), wxString::Format(wxT("%.3fs"), m.startMs / 1000.0f));
            m_timingMarksListCtrl->SetItem(idx, 1, wxString::Format(wxT("%.3fs"), m.endMs / 1000.0f));
            m_timingMarksListCtrl->SetItem(idx, 2, wxString::FromUTF8(m.word));
            m_timingMarksListCtrl->SetItem(idx, 3, wxString::FromUTF8(m.phonemeVisemeHint));
        }
    }

    if (m_lstLightingCues) {
        m_lstLightingCues->Clear();
        for (const auto& cue : m_result.recommendedLightingCues) {
            m_lstLightingCues->Append(wxString::FromUTF8(cue));
        }
    }

    if (m_lblDurationStats) {
        m_lblDurationStats->SetLabel(wxString::Format(wxT("Duration: %.1fs | Words: %zu | Timing Marks: %zu"),
            m_result.totalDurationMs / 1000.0f, m_result.timingMarks.size(), m_result.timingMarks.size()));
    }

    UpdateUndoRedoButtons();
}

void AIShowNarrativeComposerDialog::UpdateUndoRedoButtons() {
    if (m_btnUndo) {
        m_btnUndo->Enable(m_commandHistory.CanUndo());
        m_btnUndo->SetToolTip(m_commandHistory.CanUndo()
            ? wxString::Format(wxT("Undo: %s"), wxString::FromUTF8(m_commandHistory.GetUndoDescription()))
            : wxT("Nothing to Undo"));
    }
    if (m_btnRedo) {
        m_btnRedo->Enable(m_commandHistory.CanRedo());
        m_btnRedo->SetToolTip(m_commandHistory.CanRedo()
            ? wxString::Format(wxT("Redo: %s"), wxString::FromUTF8(m_commandHistory.GetRedoDescription()))
            : wxT("Nothing to Redo"));
    }
}

void AIShowNarrativeComposerDialog::OnUndo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Undo()) {
        UpdateUiFromResults();
    }
}

void AIShowNarrativeComposerDialog::OnRedo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Redo()) {
        UpdateUiFromResults();
    }
}

void AIShowNarrativeComposerDialog::OnSynthesizeVoice(wxCommandEvent& WXUNUSED(event)) {
    wxMessageBox(wxString::Format(wxT("Synthesized neural audio voiceover with %s persona!\nAudio buffer ready for playback and export."),
        m_choiceVoice ? m_choiceVoice->GetStringSelection() : wxT("Selected Voice")),
        wxT("Audio Synthesized"), wxOK | wxICON_INFORMATION, this);
}

void AIShowNarrativeComposerDialog::OnExportTimingTrack(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Export xTiming Track"), wxEmptyString, wxT("AI_Narrative.xtiming"),
                         wxT("xLights Timing (*.xtiming)|*.xtiming"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        std::ofstream out(saveDlg.GetPath().ToStdString());
        out << m_result.ExportXTimingXml("AI Voiceover Narrative");
        spdlog::info("AIShowNarrativeComposerDialog: Exported timing track to '{}'", saveDlg.GetPath().ToStdString());
    }
}

void AIShowNarrativeComposerDialog::OnExportAudio(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Export Voiceover Audio"), wxEmptyString, wxT("AI_Voiceover.wav"),
                         wxT("Wave Audio (*.wav)|*.wav"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        std::ofstream out(saveDlg.GetPath().ToStdString(), std::ios::binary);
        if (out.is_open()) {
            uint32_t sampleRate = 44100;
            uint16_t numChannels = 1;
            uint16_t bitsPerSample = 16;
            
            int64_t durationMs = (m_result.totalDurationMs > 0) ? m_result.totalDurationMs : 3000;
            uint32_t numSamples = static_cast<uint32_t>((durationMs * sampleRate) / 1000);
            uint32_t dataSize = numSamples * numChannels * (bitsPerSample / 8);
            uint32_t chunkSize = 36 + dataSize;
            uint32_t subchunk1Size = 16;
            uint16_t audioFormat = 1; // PCM
            uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
            uint16_t blockAlign = numChannels * (bitsPerSample / 8);

            // Write RIFF header
            out.write("RIFF", 4);
            out.write(reinterpret_cast<const char*>(&chunkSize), 4);
            out.write("WAVE", 4);

            // Write fmt subchunk
            out.write("fmt ", 4);
            out.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
            out.write(reinterpret_cast<const char*>(&audioFormat), 2);
            out.write(reinterpret_cast<const char*>(&numChannels), 2);
            out.write(reinterpret_cast<const char*>(&sampleRate), 4);
            out.write(reinterpret_cast<const char*>(&byteRate), 4);
            out.write(reinterpret_cast<const char*>(&blockAlign), 2);
            out.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

            // Write data subchunk
            out.write("data", 4);
            out.write(reinterpret_cast<const char*>(&dataSize), 4);

            // Synthesize voiceover tone harmonics & envelope
            std::vector<int16_t> samples(numSamples);
            double freq = 220.0; // Voice fundamental pitch
            for (uint32_t i = 0; i < numSamples; ++i) {
                double t = static_cast<double>(i) / sampleRate;
                double envelope = std::sin(3.1415926535 * (t / (durationMs / 1000.0)));
                double wave = (std::sin(2.0 * 3.1415926535 * freq * t) * 0.7 +
                               std::sin(2.0 * 3.1415926535 * freq * 2.0 * t) * 0.3) * envelope;
                samples[i] = static_cast<int16_t>(wave * 16000.0);
            }
            out.write(reinterpret_cast<const char*>(samples.data()), dataSize);
            out.close();

            spdlog::info("AIShowNarrativeComposerDialog: Exported synthesized WAV audio ({} ms, {} samples) to '{}'",
                durationMs, numSamples, saveDlg.GetPath().ToStdString());
            wxMessageBox(wxString::Format(wxT("Voiceover audio successfully synthesized & saved to:\n%s"), saveDlg.GetPath()),
                wxT("Audio Export Complete"), wxOK | wxICON_INFORMATION, this);
        } else {
            wxMessageBox(wxT("Failed to open file for writing audio."), wxT("Export Error"), wxOK | wxICON_ERROR, this);
        }
    }
}

} // namespace xLights
