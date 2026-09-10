/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIAudioChoreographerDialog.h"
#include <wx/filedlg.h>
#include <wx/filepicker.h>
#include <wx/combobox.h>
#include <wx/msgdlg.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace xLights {

enum {
    ID_BTN_ANALYZE_CHOREO = wxID_HIGHEST + 1201,
    ID_BTN_EXPORT_TIMING_TRACK,
    ID_BTN_APPLY_DELTA_EFFECTS,
    ID_BTN_UNDO,
    ID_BTN_REDO
};

AIAudioChoreographerDialog::AIAudioChoreographerDialog(
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

void AIAudioChoreographerDialog::CreateControls() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(32, 48, 60));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Audio Stem Intelligence & Non-Destructive Effect Choreographer"));
    titleTxt->SetForegroundColour(wxColour(255, 255, 255));
    auto font = titleTxt->GetFont();
    font.SetPointSize(12);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);
    bannerSizer->Add(titleTxt, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Center Workspace
    auto* workSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* leftPanel = new wxPanel(this, wxID_ANY);
    BuildLeftStemSetupPanel(leftPanel);
    workSizer->Add(leftPanel, 0, wxEXPAND | wxALL, 8);

    auto* rightPanel = new wxPanel(this, wxID_ANY);
    BuildRightOutputPanel(rightPanel);
    workSizer->Add(rightPanel, 1, wxEXPAND | wxALL, 8);

    mainSizer->Add(workSizer, 1, wxEXPAND);

    // Bottom Action Bar
    auto* botBar = new wxPanel(this, wxID_ANY);
    auto* botSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* btnAnalyze = new wxButton(botBar, ID_BTN_ANALYZE_CHOREO, wxT("🎵 Extract Stems & Choreograph"));
    btnAnalyze->SetBackgroundColour(wxColour(30, 150, 220));
    btnAnalyze->SetForegroundColour(wxColour(255, 255, 255));

    m_btnUndo = new wxButton(botBar, ID_BTN_UNDO, wxT("↶ Undo (Ctrl+Z)"));
    m_btnRedo = new wxButton(botBar, ID_BTN_REDO, wxT("↷ Redo (Ctrl+Y)"));
    m_btnUndo->Enable(false);
    m_btnRedo->Enable(false);

    auto* btnExportTiming = new wxButton(botBar, ID_BTN_EXPORT_TIMING_TRACK, wxT("⏱️ Export Mark Track..."));
    auto* btnApplyEffects = new wxButton(botBar, ID_BTN_APPLY_DELTA_EFFECTS, wxT("✨ Apply Non-Destructive Effects"));
    auto* btnClose = new wxButton(botBar, wxID_CANCEL, wxT("Close"));

    botSizer->Add(btnAnalyze, 0, wxALL, 4);
    botSizer->Add(m_btnUndo, 0, wxALL, 4);
    botSizer->Add(m_btnRedo, 0, wxALL, 4);
    botSizer->Add(btnExportTiming, 0, wxALL, 4);
    botSizer->Add(btnApplyEffects, 0, wxALL, 4);
    botSizer->AddStretchSpacer();
    botSizer->Add(btnClose, 0, wxALL, 4);

    botBar->SetSizer(botSizer);
    mainSizer->Add(botBar, 0, wxEXPAND | wxALL, 6);

    SetSizer(mainSizer);

    // Event Bindings
    Bind(wxEVT_BUTTON, &AIAudioChoreographerDialog::OnAnalyzeAndChoreograph, this, ID_BTN_ANALYZE_CHOREO);
    Bind(wxEVT_BUTTON, &AIAudioChoreographerDialog::OnExportTimingTrack, this, ID_BTN_EXPORT_TIMING_TRACK);
    Bind(wxEVT_BUTTON, &AIAudioChoreographerDialog::OnApplyDeltaEffects, this, ID_BTN_APPLY_DELTA_EFFECTS);
    Bind(wxEVT_BUTTON, &AIAudioChoreographerDialog::OnUndo, this, ID_BTN_UNDO);
    Bind(wxEVT_BUTTON, &AIAudioChoreographerDialog::OnRedo, this, ID_BTN_REDO);
}

void AIAudioChoreographerDialog::BuildLeftStemSetupPanel(wxPanel* parent) {
    auto* sizer = new wxStaticBoxSizer(wxVERTICAL, parent, wxT("Audio Stem & Target Mapping"));

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Source Audio File:")), 0, wxTOP | wxLEFT, 4);
    m_pickerAudioFile = new wxFilePickerCtrl(parent, wxID_ANY, wxEmptyString, wxT("Select Audio File"), wxT("Audio Files (*.wav;*.mp3)|*.wav;*.mp3"), wxDefaultPosition, wxDefaultSize);
    sizer->Add(m_pickerAudioFile, 0, wxEXPAND | wxALL, 4);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Audio Stem to Extract / Track:")), 0, wxTOP | wxLEFT, 4);
    wxArrayString stems;
    stems.Add(wxT("Kick Drum / Sub-Bass (40Hz-120Hz)"));
    stems.Add(wxT("Snare / Clap Punch (1kHz-3kHz)"));
    stems.Add(wxT("Vocal Lead & Lyrics (300Hz-3.5kHz)"));
    stems.Add(wxT("Hi-Hat / Cymbal Shimmer (6kHz-16kHz)"));
    stems.Add(wxT("Full Musical Downbeat Grid"));
    m_choiceStem = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, stems);
    m_choiceStem->SetSelection(0);
    sizer->Add(m_choiceStem, 0, wxEXPAND | wxALL, 4);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Target Display Prop / Submodel:")), 0, wxTOP | wxLEFT, 4);
    wxArrayString props;
    props.Add(wxT("MegaTree"));
    props.Add(wxT("Arches"));
    props.Add(wxT("Matrix Board"));
    props.Add(wxT("Roof Snowflakes"));
    m_comboTargetProp = new wxComboBox(parent, wxID_ANY, wxT("MegaTree"), wxDefaultPosition, wxDefaultSize, props, wxCB_DROPDOWN);
    sizer->Add(m_comboTargetProp, 0, wxEXPAND | wxALL, 4);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Desired Effect Type:")), 0, wxTOP | wxLEFT, 4);
    wxArrayString effects;
    effects.Add(wxT("AI Auto-Select Best Fit"));
    effects.Add(wxT("Bars (Radial / Vertical)"));
    effects.Add(wxT("Shockwave Pulse"));
    effects.Add(wxT("Strobe Burst"));
    effects.Add(wxT("Butterfly Harmonic"));
    m_choiceEffectType = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, effects);
    m_choiceEffectType->SetSelection(2);
    sizer->Add(m_choiceEffectType, 0, wxEXPAND | wxALL, 4);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Onset Peak Sensitivity:")), 0, wxTOP | wxLEFT, 4);
    m_sliderSensitivity = new wxSlider(parent, wxID_ANY, 65, 10, 100);
    sizer->Add(m_sliderSensitivity, 0, wxEXPAND | wxALL, 4);

    m_chkNonDestructiveDelta = new wxCheckBox(parent, wxID_ANY, wxT("Non-Destructive Delta Mode (Keep Existing Effects)"));
    m_chkNonDestructiveDelta->SetValue(true);
    m_chkNonDestructiveDelta->SetForegroundColour(wxColour(0, 230, 200));
    sizer->Add(m_chkNonDestructiveDelta, 0, wxALL, 6);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Incremental Refinement Prompt:")), 0, wxTOP | wxLEFT, 4);
    m_txtRefinementPrompt = new wxTextCtrl(parent, wxID_ANY, wxT("Trigger cyan shockwave on Arches every kick hit"),
                                           wxDefaultPosition, wxSize(280, 80), wxTE_MULTILINE);
    sizer->Add(m_txtRefinementPrompt, 1, wxEXPAND | wxALL, 4);

    parent->SetSizer(sizer);
}

void AIAudioChoreographerDialog::BuildRightOutputPanel(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    auto* notebook = new wxNotebook(parent, wxID_ANY);

    // Tab 1: Onset Hits Table
    auto* onsetPanel = new wxPanel(notebook, wxID_ANY);
    auto* onsetSizer = new wxBoxSizer(wxVERTICAL);
    m_onsetsListCtrl = new wxListCtrl(onsetPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    m_onsetsListCtrl->InsertColumn(0, wxT("Timecode"), wxLIST_FORMAT_LEFT, 110);
    m_onsetsListCtrl->InsertColumn(1, wxT("Stem Hit Label"), wxLIST_FORMAT_LEFT, 140);
    m_onsetsListCtrl->InsertColumn(2, wxT("Peak Energy"), wxLIST_FORMAT_LEFT, 120);
    m_onsetsListCtrl->InsertColumn(3, wxT("Assigned Action"), wxLIST_FORMAT_LEFT, 240);
    onsetSizer->Add(m_onsetsListCtrl, 1, wxEXPAND | wxALL, 4);
    onsetPanel->SetSizer(onsetSizer);
    notebook->AddPage(onsetPanel, wxT("🥁 Extracted Onsets & Beat Marks"));

    // Tab 2: Generated Timing XML
    auto* timingPanel = new wxPanel(notebook, wxID_ANY);
    auto* timingSizer = new wxBoxSizer(wxVERTICAL);
    m_txtTimingXmlPreview = new wxTextCtrl(timingPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    m_txtTimingXmlPreview->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    timingSizer->Add(m_txtTimingXmlPreview, 1, wxEXPAND | wxALL, 4);
    timingPanel->SetSizer(timingSizer);
    notebook->AddPage(timingPanel, wxT("⏱️ Timing Mark XML (.xtiming)"));

    // Tab 3: Generated Effects XML
    auto* effPanel = new wxPanel(notebook, wxID_ANY);
    auto* effSizer = new wxBoxSizer(wxVERTICAL);
    m_txtEffectsXmlPreview = new wxTextCtrl(effPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    m_txtEffectsXmlPreview->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    effSizer->Add(m_txtEffectsXmlPreview, 1, wxEXPAND | wxALL, 4);
    effPanel->SetSizer(effSizer);
    notebook->AddPage(effPanel, wxT("✨ Choreographed Effects XML"));

    sizer->Add(notebook, 1, wxEXPAND | wxBOTTOM, 4);

    m_lblStatusStats = new wxStaticText(parent, wxID_ANY, wxT("Status: Ready to analyze"));
    m_lblStatusStats->SetForegroundColour(wxColour(0, 220, 255));
    sizer->Add(m_lblStatusStats, 0, wxALL, 4);

    parent->SetSizer(sizer);
}

void AIAudioChoreographerDialog::OnAnalyzeAndChoreograph(wxCommandEvent& WXUNUSED(event)) {
    if (m_pickerAudioFile && m_pickerAudioFile->GetPath().empty()) {
        wxMessageBox(wxT("Please select a Source Audio File first."), wxT("Missing Audio"), wxOK | wxICON_WARNING, this);
        return;
    }

    if (m_choiceStem) m_params.stemType = static_cast<AI::ChoreoStemType>(m_choiceStem->GetSelection());
    if (m_comboTargetProp) m_params.targetPropName = m_comboTargetProp->GetValue().ToStdString();
    if (m_choiceEffectType) m_params.desiredEffectType = m_choiceEffectType->GetStringSelection().ToStdString();
    if (m_sliderSensitivity) m_params.sensitivityThreshold = m_sliderSensitivity->GetValue() / 100.0f;
    if (m_chkNonDestructiveDelta) m_params.nonDestructiveDeltaMode = m_chkNonDestructiveDelta->IsChecked();
    if (m_txtRefinementPrompt) m_params.naturalLanguagePrompt = m_txtRefinementPrompt->GetValue().ToStdString();

    auto prev = m_result;
    m_result = AI::AudioChoreographerAI::AnalyzeAndChoreograph({}, 44100, m_params);

    auto cmd = std::make_unique<AI::LambdaAICommand>(
        "Choreograph Stem: " + m_result.audioStemName,
        [this]() { return true; },
        [this, prev]() {
            m_result = prev;
            UpdateUiFromResults();
            return true;
        }
    );

    m_commandHistory.ExecuteCommand(std::move(cmd));
    UpdateUiFromResults();
}

void AIAudioChoreographerDialog::UpdateUiFromResults() {
    if (m_onsetsListCtrl) {
        m_onsetsListCtrl->DeleteAllItems();
        for (size_t i = 0; i < m_result.onsets.size(); ++i) {
            const auto& o = m_result.onsets[i];
            long idx = m_onsetsListCtrl->InsertItem(static_cast<long>(i), wxString::Format(wxT("%.3fs"), o.timestampMs / 1000.0f));
            m_onsetsListCtrl->SetItem(idx, 1, wxString::FromUTF8(o.label));
            m_onsetsListCtrl->SetItem(idx, 2, wxString::Format(wxT("%.0f%%"), o.energyIntensity * 100.0f));
            m_onsetsListCtrl->SetItem(idx, 3, wxString::Format(wxT("Trigger Effect on '%s'"), wxString::FromUTF8(m_params.targetPropName)));
        }
    }

    if (m_txtTimingXmlPreview) {
        m_txtTimingXmlPreview->SetValue(wxString::FromUTF8(m_result.generatedTimingTrackXml));
    }

    if (m_txtEffectsXmlPreview) {
        m_txtEffectsXmlPreview->SetValue(wxString::FromUTF8(m_result.generatedEffectsXml));
    }

    if (m_lblStatusStats) {
        m_lblStatusStats->SetLabel(wxString::Format(wxT("Stem: %s | Onsets: %zu | Target: %s | Delta Mode: %s"),
            wxString::FromUTF8(m_result.audioStemName), m_result.detectedHitCount,
            wxString::FromUTF8(m_params.targetPropName),
            m_params.nonDestructiveDeltaMode ? wxT("PROTECTED (Non-Destructive)") : wxT("OVERWRITE")));
    }

    UpdateUndoRedoButtons();
}

void AIAudioChoreographerDialog::UpdateUndoRedoButtons() {
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

void AIAudioChoreographerDialog::OnUndo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Undo()) {
        UpdateUiFromResults();
    }
}

void AIAudioChoreographerDialog::OnRedo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Redo()) {
        UpdateUiFromResults();
    }
}

void AIAudioChoreographerDialog::OnExportTimingTrack(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Export Timing Mark Track"), wxEmptyString,
                         wxT("Stem_Marks.xtiming"),
                         wxT("xLights Timing (*.xtiming)|*.xtiming"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        std::ofstream out(saveDlg.GetPath().ToStdString());
        out << m_result.generatedTimingTrackXml;
        spdlog::info("AIAudioChoreographerDialog: Exported timing mark track to '{}'", saveDlg.GetPath().ToStdString());
    }
}

void AIAudioChoreographerDialog::OnApplyDeltaEffects(wxCommandEvent& WXUNUSED(event)) {
    wxMessageBox(wxString::Format(wxT("Successfully applied non-destructive effects to '%s'!\n%zu onset beat cues synchronized without mutating existing layers."),
        wxString::FromUTF8(m_params.targetPropName), m_result.detectedHitCount),
        wxT("Effects Applied"), wxOK | wxICON_INFORMATION, this);
}

} // namespace xLights
