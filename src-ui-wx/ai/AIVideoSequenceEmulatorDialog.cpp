/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIVideoSequenceEmulatorDialog.h"
#include "src-ui-wx/xLightsMain.h"
#include "src-core/render/SequenceElements.h"
#include "src-core/render/Element.h"
#include "src-core/render/EffectLayer.h"
#include "src-core/render/Effect.h"
#include "src-core/render/UndoManager.h"
#include <wx/app.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/file.h>
#include <spdlog/spdlog.h>
#include <thread>

namespace xLights::AI {

enum {
    ID_VSE_ANALYZE_BTN = 17001,
    ID_VSE_GENERATE_BTN = 17002,
    ID_VSE_INSERT_BTN = 17003,
    ID_VSE_EXPORT_BTN = 17004,
    ID_VSE_POLISH_BTN = 17005,
    ID_VSE_SPEED_SLIDER = 17006
};

wxBEGIN_EVENT_TABLE(AIVideoSequenceEmulatorDialog, wxDialog)
    EVT_BUTTON(ID_VSE_ANALYZE_BTN, AIVideoSequenceEmulatorDialog::OnAnalyzeVideoClick)
    EVT_BUTTON(ID_VSE_GENERATE_BTN, AIVideoSequenceEmulatorDialog::OnGenerateSequenceClick)
    EVT_BUTTON(ID_VSE_INSERT_BTN, AIVideoSequenceEmulatorDialog::OnInsertIntoActiveSeqClick)
    EVT_BUTTON(ID_VSE_EXPORT_BTN, AIVideoSequenceEmulatorDialog::OnExportXsqClick)
    EVT_BUTTON(ID_VSE_POLISH_BTN, AIVideoSequenceEmulatorDialog::OnApplyPolishClick)
    EVT_BUTTON(wxID_CANCEL, AIVideoSequenceEmulatorDialog::OnCloseClick)
wxEND_EVENT_TABLE()

AIVideoSequenceEmulatorDialog::AIVideoSequenceEmulatorDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style
) : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
    PopulateModelList();
}

AIVideoSequenceEmulatorDialog::~AIVideoSequenceEmulatorDialog() {
    m_workerCancel = true;
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void AIVideoSequenceEmulatorDialog::InitUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Header Banner
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(wxColour(32, 40, 56));
    wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* titleLabel = new wxStaticText(headerPanel, wxID_ANY, wxT("🎬 AI Video Sequence Emulation & Choreographer"));
    titleLabel->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleLabel->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 3);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleLabel->SetFont(titleFont);

    wxStaticText* subLabel = new wxStaticText(headerPanel, wxID_ANY, wxT("Multimodal vision analysis • Layout adaptation • Automated sequence generation"));
    subLabel->SetForegroundColour(wxColour(180, 200, 220));

    wxBoxSizer* textHeaderSizer = new wxBoxSizer(wxVERTICAL);
    textHeaderSizer->Add(titleLabel, 0, wxALL, 2);
    textHeaderSizer->Add(subLabel, 0, wxALL, 2);

    headerSizer->Add(textHeaderSizer, 1, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    headerPanel->SetSizer(headerSizer);
    mainSizer->Add(headerPanel, 0, wxEXPAND);

    // Main Tabbed Notebook
    m_notebook = new wxNotebook(this, wxID_ANY);

    // ==========================================
    // TAB 1: Source & Scope
    // ==========================================
    wxPanel* tab1 = new wxPanel(m_notebook, wxID_ANY);
    wxBoxSizer* tab1Sizer = new wxBoxSizer(wxVERTICAL);

    // Source inputs
    wxStaticBoxSizer* sourceBox = new wxStaticBoxSizer(wxVERTICAL, tab1, wxT("Video Source & Timecode Window"));
    wxFlexGridSizer* sourceGrid = new wxFlexGridSizer(3, 2, 8, 12);
    sourceGrid->AddGrowableCol(1, 1);

    sourceGrid->Add(new wxStaticText(tab1, wxID_ANY, wxT("Local Video File:")), 0, wxALIGN_CENTER_VERTICAL);
    m_videoFilePicker = new wxFilePickerCtrl(tab1, wxID_ANY, wxEmptyString, wxT("Select sequence clip"),
        wxT("Video files (*.mp4;*.mov;*.mkv;*.avi)|*.mp4;*.mov;*.mkv;*.avi|All files (*.*)|*.*"),
        wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_FILE_MUST_EXIST | wxFLP_USE_TEXTCTRL);
    sourceGrid->Add(m_videoFilePicker, 1, wxEXPAND);

    sourceGrid->Add(new wxStaticText(tab1, wxID_ANY, wxT("Or Video URL:")), 0, wxALIGN_CENTER_VERTICAL);
    m_videoUrlCtrl = new wxTextCtrl(tab1, wxID_ANY, wxEmptyString);
    m_videoUrlCtrl->SetHint(wxT("https://www.youtube.com/watch?v=... or direct web video stream"));
    sourceGrid->Add(m_videoUrlCtrl, 1, wxEXPAND);

    sourceGrid->Add(new wxStaticText(tab1, wxID_ANY, wxT("Timecode Constraints:")), 0, wxALIGN_CENTER_VERTICAL);
    wxBoxSizer* timeSizer = new wxBoxSizer(wxHORIZONTAL);
    m_timeRangeCheck = new wxCheckBox(tab1, wxID_ANY, wxT("Constrain to range:"));
    m_startTimeSpin = new wxSpinCtrl(tab1, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 0, 3600000, 0);
    m_endTimeSpin = new wxSpinCtrl(tab1, wxID_ANY, wxT("30000"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 1000, 3600000, 30000);
    timeSizer->Add(m_timeRangeCheck, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    timeSizer->Add(new wxStaticText(tab1, wxID_ANY, wxT("Start (ms):")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    timeSizer->Add(m_startTimeSpin, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    timeSizer->Add(new wxStaticText(tab1, wxID_ANY, wxT("End (ms):")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    timeSizer->Add(m_endTimeSpin, 0, wxALIGN_CENTER_VERTICAL);
    sourceGrid->Add(timeSizer, 1, wxEXPAND);

    sourceBox->Add(sourceGrid, 1, wxEXPAND | wxALL, 8);
    tab1Sizer->Add(sourceBox, 0, wxEXPAND | wxALL, 8);

    // Target Props Scope & Prompt
    wxBoxSizer* splitSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticBoxSizer* propsBox = new wxStaticBoxSizer(wxVERTICAL, tab1, wxT("Target Props to Adapt Onto"));
    wxArrayString initialProps;
    m_propsCheckList = new wxCheckListBox(tab1, wxID_ANY, wxDefaultPosition, wxSize(260, 180), initialProps);
    propsBox->Add(m_propsCheckList, 1, wxEXPAND | wxALL, 4);
    splitSizer->Add(propsBox, 0, wxEXPAND | wxRIGHT, 8);

    wxStaticBoxSizer* promptBox = new wxStaticBoxSizer(wxVERTICAL, tab1, wxT("Master Directives & Artistic Intent (Plain English)"));
    m_userPromptCtrl = new wxTextCtrl(tab1, wxID_ANY,
        wxT("Emulate the sequence flow faithfully. Map the dominant lead effects onto my MegaTree and Matrix, and adapt percussive background hits to my mini trees and roofline."),
        wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    promptBox->Add(m_userPromptCtrl, 1, wxEXPAND | wxALL, 4);
    splitSizer->Add(promptBox, 1, wxEXPAND);

    tab1Sizer->Add(splitSizer, 1, wxEXPAND | wxALL, 8);

    m_analyzeBtn = new wxButton(tab1, ID_VSE_ANALYZE_BTN, wxT("🔍 Analyze Video & Layout Models"), wxDefaultPosition, wxSize(240, 36));
    m_analyzeBtn->SetFont(m_analyzeBtn->GetFont().Bold());
    tab1Sizer->Add(m_analyzeBtn, 0, wxALIGN_RIGHT | wxALL, 8);

    tab1->SetSizer(tab1Sizer);
    m_notebook->AddPage(tab1, wxT("1. Video Source & Scope"), true);

    // ==========================================
    // TAB 2: Pre-Flight Consultation
    // ==========================================
    wxPanel* tab2 = new wxPanel(m_notebook, wxID_ANY);
    wxBoxSizer* tab2Sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBoxSizer* obsBox = new wxStaticBoxSizer(wxVERTICAL, tab2, wxT("Vision Observations & Identified Challenges"));
    m_analysisSummaryCtrl = new wxTextCtrl(tab2, wxID_ANY,
        wxT("Click 'Analyze Video & Layout Models' on Step 1 to probe footage, BPM, and prop roles."),
        wxDefaultPosition, wxSize(-1, 140), wxTE_MULTILINE | wxTE_READONLY);
    m_analysisSummaryCtrl->SetBackgroundColour(wxColour(245, 247, 250));
    obsBox->Add(m_analysisSummaryCtrl, 1, wxEXPAND | wxALL, 4);
    tab2Sizer->Add(obsBox, 0, wxEXPAND | wxALL, 8);

    wxStaticBoxSizer* stratBox = new wxStaticBoxSizer(wxVERTICAL, tab2, wxT("Adaptation Strategy & Clarification Options"));
    wxArrayString stratChoices;
    stratChoices.Add(wxT("Macro Spatial Flow (Unified yard-wide sweeps & energy waves) - [RECOMMENDED]"));
    stratChoices.Add(wxT("Rhythmic Accent (Snappy percussive sync to BPM & onsets)"));
    stratChoices.Add(wxT("Dense Feature Emulation (Literal effect-by-effect translation)"));
    m_strategyRadio = new wxRadioBox(tab2, wxID_ANY, wxT("Adaptation Philosophy"), wxDefaultPosition, wxDefaultSize, stratChoices, 1, wxRA_SPECIFY_COLS);
    stratBox->Add(m_strategyRadio, 0, wxEXPAND | wxALL, 4);

    wxFlexGridSizer* choicesGrid = new wxFlexGridSizer(1, 2, 8, 8);
    choicesGrid->AddGrowableCol(0, 1);
    choicesGrid->AddGrowableCol(1, 1);

    wxArrayString palChoices;
    palChoices.Add(wxT("Strictly match video palette"));
    palChoices.Add(wxT("Blend with active show palette"));
    palChoices.Add(wxT("Boost outdoor LED saturation (+15%)"));
    m_paletteChoiceRadio = new wxRadioBox(tab2, wxID_ANY, wxT("Color Policy"), wxDefaultPosition, wxDefaultSize, palChoices, 1, wxRA_SPECIFY_COLS);
    choicesGrid->Add(m_paletteChoiceRadio, 1, wxEXPAND);

    wxArrayString propFallbackChoices;
    propFallbackChoices.Add(wxT("Transduce moving heads to arches/mini-trees"));
    propFallbackChoices.Add(wxT("Map missing props to yard floods"));
    propFallbackChoices.Add(wxT("Omit effects for missing prop categories"));
    m_missingPropChoiceRadio = new wxRadioBox(tab2, wxID_ANY, wxT("Missing Prop Handling"), wxDefaultPosition, wxDefaultSize, propFallbackChoices, 1, wxRA_SPECIFY_COLS);
    choicesGrid->Add(m_missingPropChoiceRadio, 1, wxEXPAND);

    stratBox->Add(choicesGrid, 1, wxEXPAND | wxALL, 4);
    tab2Sizer->Add(stratBox, 1, wxEXPAND | wxALL, 8);

    m_generatePlanBtn = new wxButton(tab2, ID_VSE_GENERATE_BTN, wxT("⚡ Synthesize & Generate Emulated Sequence"), wxDefaultPosition, wxSize(280, 36));
    m_generatePlanBtn->SetFont(m_generatePlanBtn->GetFont().Bold());
    tab2Sizer->Add(m_generatePlanBtn, 0, wxALIGN_RIGHT | wxALL, 8);

    tab2->SetSizer(tab2Sizer);
    m_notebook->AddPage(tab2, wxT("2. Pre-Flight Consultation & Strategy"));

    // ==========================================
    // TAB 3: Generated Sequence & Iterative Polish
    // ==========================================
    wxPanel* tab3 = new wxPanel(m_notebook, wxID_ANY);
    wxBoxSizer* tab3Sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBoxSizer* listStaticBox = new wxStaticBoxSizer(wxVERTICAL, tab3, wxT("Synthesized Effect Cue Map"));
    m_cuesListCtrl = new wxListCtrl(tab3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
    m_cuesListCtrl->InsertColumn(0, wxT("Timecode"), wxLIST_FORMAT_LEFT, 110);
    m_cuesListCtrl->InsertColumn(1, wxT("Target Prop"), wxLIST_FORMAT_LEFT, 130);
    m_cuesListCtrl->InsertColumn(2, wxT("Effect Type"), wxLIST_FORMAT_LEFT, 110);
    m_cuesListCtrl->InsertColumn(3, wxT("Layer"), wxLIST_FORMAT_LEFT, 60);
    m_cuesListCtrl->InsertColumn(4, wxT("Palette"), wxLIST_FORMAT_LEFT, 130);
    m_cuesListCtrl->InsertColumn(5, wxT("AI Translation Rationale"), wxLIST_FORMAT_LEFT, 380);
    listStaticBox->Add(m_cuesListCtrl, 1, wxEXPAND | wxALL, 4);
    tab3Sizer->Add(listStaticBox, 1, wxEXPAND | wxALL, 8);

    // Iterative Tuning Panel
    wxStaticBoxSizer* polishBox = new wxStaticBoxSizer(wxHORIZONTAL, tab3, wxT("Iterative Sequence Polish & Prompt Tuning"));
    m_refinePromptCtrl = new wxTextCtrl(tab3, wxID_ANY, wxEmptyString);
    m_refinePromptCtrl->SetHint(wxT("e.g. Make mini-tree chases 20% faster, switch accent color to gold, boost contrast"));
    polishBox->Add(m_refinePromptCtrl, 1, wxALIGN_CENTER_VERTICAL | wxALL, 6);

    polishBox->Add(new wxStaticText(tab3, wxID_ANY, wxT("Speed:")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    m_speedSlider = new wxSlider(tab3, ID_VSE_SPEED_SLIDER, 100, 50, 200, wxDefaultPosition, wxSize(120, -1));
    m_speedValLabel = new wxStaticText(tab3, wxID_ANY, wxT("1.0x"));
    polishBox->Add(m_speedSlider, 0, wxALIGN_CENTER_VERTICAL);
    polishBox->Add(m_speedValLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 6);

    m_speedSlider->Bind(wxEVT_SLIDER, [this](wxCommandEvent&) {
        float val = m_speedSlider->GetValue() / 100.0f;
        m_speedValLabel->SetLabel(wxString::Format(wxT("%.1fx"), val));
    });

    m_applyPolishBtn = new wxButton(tab3, ID_VSE_POLISH_BTN, wxT("🪄 Apply Polish"));
    polishBox->Add(m_applyPolishBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
    tab3Sizer->Add(polishBox, 0, wxEXPAND | wxALL, 8);

    // Action buttons row
    wxBoxSizer* actionSizer = new wxBoxSizer(wxHORIZONTAL);
    m_exportXsqBtn = new wxButton(tab3, ID_VSE_EXPORT_BTN, wxT("💾 Export Standalone .xsq Sequence..."));
    m_insertSequenceBtn = new wxButton(tab3, ID_VSE_INSERT_BTN, wxT("📥 Insert into Active Sequence (with Undo)"));
    m_insertSequenceBtn->SetFont(m_insertSequenceBtn->GetFont().Bold());
    m_insertSequenceBtn->SetBackgroundColour(wxColour(40, 120, 60));
    m_insertSequenceBtn->SetForegroundColour(*wxWHITE);

    actionSizer->Add(m_exportXsqBtn, 0, wxRIGHT, 8);
    actionSizer->AddStretchSpacer(1);
    actionSizer->Add(m_insertSequenceBtn, 0, wxLEFT, 8);
    tab3Sizer->Add(actionSizer, 0, wxEXPAND | wxALL, 8);

    tab3->SetSizer(tab3Sizer);
    m_notebook->AddPage(tab3, wxT("3. Generated Cues & Live Polish"));

    mainSizer->Add(m_notebook, 1, wxEXPAND | wxALL, 6);

    // Bottom Bar (Progress, Status, Close)
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    m_progressGauge = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(140, 16));
    m_progressGauge->Hide();
    m_statusLabel = new wxStaticText(this, wxID_ANY, wxT("Ready • Provide video clip or URL and click Analyze."));

    m_closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));

    bottomSizer->Add(m_progressGauge, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
    bottomSizer->Add(m_statusLabel, 1, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 10);
    bottomSizer->Add(m_closeBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    mainSizer->Add(bottomSizer, 0, wxEXPAND | wxBOTTOM | wxTOP, 6);

    SetSizer(mainSizer);
    Centre();
}

void AIVideoSequenceEmulatorDialog::PopulateModelList() {
    m_propsCheckList->Clear();

    std::vector<std::string> modelNames;
    xLightsFrame* frame = xLightsFrame::GetFrame();
    if (frame) {
        for (const auto& [name, modelPtr] : frame->AllModels) {
            modelNames.push_back(name);
        }
    }

    if (modelNames.empty()) {
        modelNames = {"MegaTree", "Arches", "MiniTrees", "Roofline", "Matrix", "Floods"};
    }

    std::sort(modelNames.begin(), modelNames.end());
    for (const auto& name : modelNames) {
        int idx = m_propsCheckList->Append(wxString::FromUTF8(name));
        m_propsCheckList->Check(idx, true);
    }
}

void AIVideoSequenceEmulatorDialog::RefreshCuesList() {
    m_cuesListCtrl->DeleteAllItems();
    for (size_t i = 0; i < m_currentPlan.generatedCues.size(); ++i) {
        const auto& cue = m_currentPlan.generatedCues[i];
        wxString timeStr = wxString::Format(wxT("%02d:%02d.%03d - %02d:%02d.%03d"),
            cue.startMs / 60000, (cue.startMs % 60000) / 1000, cue.startMs % 1000,
            cue.endMs / 60000, (cue.endMs % 60000) / 1000, cue.endMs % 1000);

        long item = m_cuesListCtrl->InsertItem(i, timeStr);
        m_cuesListCtrl->SetItem(item, 1, wxString::FromUTF8(cue.targetPropName));
        m_cuesListCtrl->SetItem(item, 2, wxString::FromUTF8(cue.effectType));
        m_cuesListCtrl->SetItem(item, 3, wxString::Format(wxT("Layer %d"), cue.layerIndex));
        m_cuesListCtrl->SetItem(item, 4, wxString::Format(wxT("%s, %s"), wxString::FromUTF8(cue.primaryColor), wxString::FromUTF8(cue.secondaryColor)));
        m_cuesListCtrl->SetItem(item, 5, wxString::FromUTF8(cue.rationale));
    }
}

void AIVideoSequenceEmulatorDialog::OnAnalyzeVideoClick(wxCommandEvent& WXUNUSED(evt)) {
    m_currentInput.filePath = m_videoFilePicker->GetPath().ToStdString();
    m_currentInput.videoUrl = m_videoUrlCtrl->GetValue().ToStdString();
    m_currentInput.hasTimeRange = m_timeRangeCheck->IsChecked();
    m_currentInput.timeStartMs = m_startTimeSpin->GetValue();
    m_currentInput.timeEndMs = m_endTimeSpin->GetValue();
    m_currentInput.userPrompt = m_userPromptCtrl->GetValue().ToStdString();

    m_currentInput.targetPropFilters.clear();
    for (unsigned int i = 0; i < m_propsCheckList->GetCount(); ++i) {
        if (m_propsCheckList->IsChecked(i)) {
            m_currentInput.targetPropFilters.push_back(m_propsCheckList->GetString(i).ToStdString());
        }
    }

    if (m_currentInput.filePath.empty() && m_currentInput.videoUrl.empty()) {
        wxMessageBox(wxT("Please select a local video file or enter a valid video URL."),
                     wxT("Missing Source"), wxOK | wxICON_WARNING, this);
        return;
    }

    m_progressGauge->Show();
    m_progressGauge->Pulse();
    m_analyzeBtn->Enable(false);
    m_statusLabel->SetLabel(wxT("Analyzing video frames, tempo, and prop motion vectors in background..."));
    Layout();

    std::string fakeLayoutXml = "<models>";
    for (const auto& p : m_currentInput.targetPropFilters) {
        fakeLayoutXml += "<model name=\"" + p + "\" />";
    }
    fakeLayoutXml += "</models>";

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
    m_workerCancel = false;

    m_workerThread = std::thread([this, fakeLayoutXml]() {
        auto analysis = m_emulator.AnalyzeVideoSource(m_currentInput, fakeLayoutXml);
        if (m_workerCancel.load()) return;
        auto strategies = m_emulator.SuggestStrategies(analysis, fakeLayoutXml);
        if (m_workerCancel.load()) return;
        auto questions = m_emulator.GenerateConsultationQuestions(analysis, fakeLayoutXml);
        if (m_workerCancel.load()) return;

        wxTheApp->CallAfter([this, analysis, strategies, questions]() {
            if (m_workerCancel.load()) return;
            m_currentAnalysis = analysis;
            m_currentStrategies = strategies;
            m_currentQuestions = questions;

            m_progressGauge->Hide();
            m_analyzeBtn->Enable(true);

            // Populate Tab 2 text
            std::ostringstream oss;
            oss << "=== EXECUTIVE SUMMARY ===\n" << m_currentAnalysis.executiveSummary << "\n\n";
            oss << "=== KEY OBSERVATIONS ===\n";
            for (const auto& obs : m_currentAnalysis.observations) {
                oss << " • " << obs << "\n";
            }
            oss << "\n=== ADAPTATION CHALLENGES & COMPROMISES ===\n";
            for (const auto& chal : m_currentAnalysis.adaptationChallenges) {
                oss << " ⚠️ " << chal << "\n";
            }
            oss << "\n=== DETECTED HARMONY & TEMPO ===\n";
            oss << "Estimated Tempo: " << (int)m_currentAnalysis.detectedBpm << " BPM | Beat Markers: "
                << m_currentAnalysis.detectedBeatMarkers.size() << " onsets\nDominant Colors: ";
            for (const auto& col : m_currentAnalysis.dominantPalette) {
                oss << col << " ";
            }
            oss << "\n";

            m_analysisSummaryCtrl->SetValue(wxString::FromUTF8(oss.str()));

            m_statusLabel->SetLabel(wxString::Format(wxT("Analysis complete for '%s' (%d BPM, %zu tracks). Review strategy on Step 2."),
                wxString::FromUTF8(m_currentAnalysis.sourceTitle), (int)m_currentAnalysis.detectedBpm, m_currentAnalysis.visualTracks.size()));

            m_notebook->SetSelection(1);
        });
    });
}

void AIVideoSequenceEmulatorDialog::OnGenerateSequenceClick(wxCommandEvent& WXUNUSED(evt)) {
    int sel = m_strategyRadio->GetSelection();
    std::string strategyId = "macro_spatial_flow";
    if (sel == 1) strategyId = "rhythmic_accent";
    else if (sel == 2) strategyId = "dense_emulation";

    std::map<std::string, std::string> choices;
    choices["palette_policy"] = m_paletteChoiceRadio->GetStringSelection().ToStdString();
    choices["missing_prop_policy"] = m_missingPropChoiceRadio->GetStringSelection().ToStdString();

    std::string fakeLayoutXml = "<models>";
    for (const auto& p : m_currentInput.targetPropFilters) {
        fakeLayoutXml += "<model name=\"" + p + "\" />";
    }
    fakeLayoutXml += "</models>";

    m_currentPlan = m_emulator.GenerateEmulationPlan(m_currentInput, m_currentAnalysis, strategyId, choices, fakeLayoutXml);
    RefreshCuesList();

    m_statusLabel->SetLabel(wxString::Format(wxT("Generated %d cues across %d props. Ready to insert or export."),
        m_currentPlan.totalCuesCount, m_currentPlan.targetPropsCount));

    m_notebook->SetSelection(2);
}

void AIVideoSequenceEmulatorDialog::OnInsertIntoActiveSeqClick(wxCommandEvent& WXUNUSED(evt)) {
    if (m_currentPlan.generatedCues.empty()) {
        wxMessageBox(wxT("No cues to insert. Please run analysis and generation first."),
                     wxT("Notice"), wxOK | wxICON_INFORMATION, this);
        return;
    }

    xLightsFrame* frame = xLightsFrame::GetFrame();
    if (!frame) {
        wxMessageBox(wxT("xLights frame is not accessible."), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    SequenceFile* seq = xLightsFrame::CurrentSeqXmlFile;
    if (!seq || !seq->GetSequenceLoaded()) {
        wxMessageBox(wxT("No active sequence loaded. Please open or create a sequence first to insert cues."),
                     wxT("No Sequence Loaded"), wxOK | wxICON_INFORMATION, this);
        return;
    }

    SequenceElements& seqElements = frame->GetSequenceElements();
    UndoManager& undoMgr = seqElements.get_undo_mgr();
    AIUndoTransaction tx(&undoMgr, "AI Video Sequence Emulation");

    size_t applied = 0;
    for (const auto& cue : m_currentPlan.generatedCues) {
        Element* elem = seqElements.GetElement(cue.targetPropName);
        if (!elem) continue;

        EffectLayer* layer = elem->GetEffectLayer(cue.layerIndex);
        if (!layer) {
            layer = elem->AddEffectLayer();
        }
        if (!layer) continue;

        std::string palette = "1=" + cue.primaryColor + ",2=" + cue.secondaryColor;
        Effect* eff = layer->AddEffect(0, cue.effectType, cue.effectSettings, palette, cue.startMs, cue.endMs, EFFECT_NOT_SELECTED, false);
        if (eff) {
            undoMgr.CaptureAddedEffect(elem->GetModelName(), layer->GetIndex(), eff->GetID());
            applied++;
        }
    }

    tx.Commit();
    frame->RenderLayout();
    frame->Refresh();

    wxMessageBox(wxString::Format(wxT("Successfully inserted %zu emulated effect cues into active sequence!\nTransaction is fully reversible via Ctrl+Z (AI Undo)."), applied),
                 wxT("Sequence Insertion Complete"), wxOK | wxICON_INFORMATION, this);

    m_statusLabel->SetLabel(wxString::Format(wxT("✓ Inserted %zu cues into active sequence. Undo available."), applied));
}

void AIVideoSequenceEmulatorDialog::OnExportXsqClick(wxCommandEvent& WXUNUSED(evt)) {
    if (m_currentPlan.generatedCues.empty()) {
        wxMessageBox(wxT("No cues generated to export. Please generate a sequence first."),
                     wxT("Notice"), wxOK | wxICON_INFORMATION, this);
        return;
    }

    wxFileDialog saveDlg(this, wxT("Save Emulated Sequence"), wxEmptyString, wxT("Emulated_Video_Sequence.xsq"),
                         wxT("xLights Sequence Files (*.xsq)|*.xsq|XML files (*.xml)|*.xml"),
                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDlg.ShowModal() == wxID_OK) {
        std::string xml = m_emulator.ExportPlanToXsqXml(m_currentPlan);
        wxFile file(saveDlg.GetPath(), wxFile::write);
        if (file.IsOpened()) {
            file.Write(wxString::FromUTF8(xml));
            file.Close();
            spdlog::info("Exported emulated sequence to '{}'", saveDlg.GetPath().ToStdString());
            wxMessageBox(wxT("Sequence successfully exported to:\n") + saveDlg.GetPath(),
                         wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
        }
    }
}

void AIVideoSequenceEmulatorDialog::OnApplyPolishClick(wxCommandEvent& WXUNUSED(evt)) {
    std::string prompt = m_refinePromptCtrl->GetValue().ToStdString();
    float speed = m_speedSlider->GetValue() / 100.0f;

    SequenceRefinementRequest ref;
    ref.tuningPrompt = prompt;
    ref.speedFactor = speed;

    m_currentPlan = m_emulator.RefinePlan(m_currentPlan, ref);
    RefreshCuesList();

    m_statusLabel->SetLabel(wxString::Format(wxT("✓ Polish applied: %s (%d cues updated)"),
        wxString::FromUTF8(prompt.empty() ? "Speed adjusted" : prompt), m_currentPlan.totalCuesCount));
}

void AIVideoSequenceEmulatorDialog::OnCloseClick(wxCommandEvent& WXUNUSED(evt)) {
    m_workerCancel = true;
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
