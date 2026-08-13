/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AISequenceValidatorDialog.h"
#include "xLightsMain.h"
#include "xLightsApp.h"
#include <spdlog/spdlog.h>
#include <fstream>
#include <wx/msgdlg.h>

namespace xLights::AI {

enum {
    ID_RUN_AUDIT_BTN = 10001,
    ID_AUTO_REMEDIATE_BTN,
    ID_EXPORT_JSON_BTN,
    ID_PERSONA_CHOICE,
    ID_CATEGORY_CHOICE
};

wxBEGIN_EVENT_TABLE(AISequenceValidatorDialog, wxDialog)
    EVT_BUTTON(ID_RUN_AUDIT_BTN, AISequenceValidatorDialog::OnRunAuditButtonClick)
    EVT_BUTTON(ID_AUTO_REMEDIATE_BTN, AISequenceValidatorDialog::OnAutoRemediateButtonClick)
    EVT_BUTTON(ID_EXPORT_JSON_BTN, AISequenceValidatorDialog::OnExportJSONButtonClick)
    EVT_CHOICE(ID_PERSONA_CHOICE, AISequenceValidatorDialog::OnPersonaChoiceSelected)
    EVT_BUTTON(wxID_CANCEL, AISequenceValidatorDialog::OnCloseButtonClick)
wxEND_EVENT_TABLE()

AISequenceValidatorDialog::AISequenceValidatorDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
}

void AISequenceValidatorDialog::SetValidationConfig(const SequenceValidationConfig& config) {
    m_config = config;
}

void AISequenceValidatorDialog::InitUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Top Controls Sizer (Category & Persona Selection)
    wxStaticBoxSizer* topBoxSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Audit Controls & Persona Review"));

    topBoxSizer->GetSizer()->Add(new wxStaticText(this, wxID_ANY, wxT("Audit Category:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    wxArrayString categories;
    categories.Add(wxT("ALL (Full Diagnostic Audit)"));
    categories.Add(wxT("Hardware Safety"));
    categories.Add(wxT("Timing Grid & Rhythm Sync"));
    categories.Add(wxT("Channel Overlaps"));
    m_categoryChoice = new wxChoice(this, ID_CATEGORY_CHOICE, wxDefaultPosition, wxDefaultSize, categories);
    m_categoryChoice->SetSelection(0);
    topBoxSizer->GetSizer()->Add(m_categoryChoice, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    topBoxSizer->GetSizer()->Add(new wxStaticText(this, wxID_ANY, wxT("Review Persona:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    wxArrayString personas;
    personas.Add(wxT("Master Sequencer Boss"));
    personas.Add(wxT("Light Show Journalist (5-Star Review)"));
    personas.Add(wxT("Enthusiast Coach"));
    m_personaChoice = new wxChoice(this, ID_PERSONA_CHOICE, wxDefaultPosition, wxDefaultSize, personas);
    m_personaChoice->SetSelection(0);
    topBoxSizer->GetSizer()->Add(m_personaChoice, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    mainSizer->Add(topBoxSizer, 0, wxEXPAND | wxALL, 10);

    // Scorecard Section
    wxStaticBoxSizer* scorecardBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Sequence Health Scorecard"));
    wxFlexGridSizer* scoreGrid = new wxFlexGridSizer(2, 4, 5, 10);

    scoreGrid->Add(new wxStaticText(this, wxID_ANY, wxT("Overall Health:")), 0, wxALIGN_CENTER_VERTICAL);
    m_overallHealthGauge = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(200, 20));
    scoreGrid->Add(m_overallHealthGauge, 1, wxEXPAND);

    m_healthScoreText = new wxStaticText(this, wxID_ANY, wxT("100% / 100"));
    scoreGrid->Add(m_healthScoreText, 0, wxALIGN_CENTER_VERTICAL);
    scoreGrid->AddSpacer(1);

    scoreGrid->Add(new wxStaticText(this, wxID_ANY, wxT("Timing Grid:")), 0, wxALIGN_CENTER_VERTICAL);
    m_timingGridGauge = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(150, 15));
    scoreGrid->Add(m_timingGridGauge, 1, wxEXPAND);

    scoreGrid->Add(new wxStaticText(this, wxID_ANY, wxT("Channel Overlaps:")), 0, wxALIGN_CENTER_VERTICAL);
    m_channelOverlapGauge = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(150, 15));
    scoreGrid->Add(m_channelOverlapGauge, 1, wxEXPAND);

    scoreGrid->Add(new wxStaticText(this, wxID_ANY, wxT("Hardware Safety:")), 0, wxALIGN_CENTER_VERTICAL);
    m_hardwareSafetyGauge = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(150, 15));
    scoreGrid->Add(m_hardwareSafetyGauge, 1, wxEXPAND);

    scoreGrid->Add(new wxStaticText(this, wxID_ANY, wxT("Visual Harmony:")), 0, wxALIGN_CENTER_VERTICAL);
    m_visualHarmonyGauge = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(150, 15));
    scoreGrid->Add(m_visualHarmonyGauge, 1, wxEXPAND);

    scorecardBox->GetSizer()->Add(scoreGrid, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(scorecardBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Notebook (Issues List & Persona Critique)
    wxNotebook* notebook = new wxNotebook(this, wxID_ANY);

    // Tab 1: Diagnostic Findings
    wxPanel* issuesPanel = new wxPanel(notebook);
    wxBoxSizer* issuesSizer = new wxBoxSizer(wxVERTICAL);
    m_issuesListCtrl = new wxListCtrl(issuesPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxBORDER_SUNKEN);
    m_issuesListCtrl->InsertColumn(0, wxT("Issue ID"), wxLIST_FORMAT_LEFT, 80);
    m_issuesListCtrl->InsertColumn(1, wxT("Severity"), wxLIST_FORMAT_LEFT, 80);
    m_issuesListCtrl->InsertColumn(2, wxT("Category"), wxLIST_FORMAT_LEFT, 110);
    m_issuesListCtrl->InsertColumn(3, wxT("Message"), wxLIST_FORMAT_LEFT, 280);
    m_issuesListCtrl->InsertColumn(4, wxT("Actionable Fix"), wxLIST_FORMAT_LEFT, 220);
    issuesSizer->Add(m_issuesListCtrl, 1, wxEXPAND | wxALL, 5);
    issuesPanel->SetSizer(issuesSizer);
    notebook->AddPage(issuesPanel, wxT("Diagnostic Findings"), true);

    // Tab 2: Persona Critique Review
    wxPanel* critiquePanel = new wxPanel(notebook);
    wxBoxSizer* critiqueSizer = new wxBoxSizer(wxVERTICAL);
    m_critiqueTextCtrl = new wxTextCtrl(critiquePanel, wxID_ANY, wxT("Run audit scan to generate AI persona critique report."), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    critiqueSizer->Add(m_critiqueTextCtrl, 1, wxEXPAND | wxALL, 5);
    critiquePanel->SetSizer(critiqueSizer);
    notebook->AddPage(critiquePanel, wxT("Persona Critique Report"));

    mainSizer->Add(notebook, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Bottom Action Buttons Sizer
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_runAuditBtn = new wxButton(this, ID_RUN_AUDIT_BTN, wxT("Run Audit Scan"));
    m_autoRemediateBtn = new wxButton(this, ID_AUTO_REMEDIATE_BTN, wxT("Auto-Remediate XML"));
    m_exportJsonBtn = new wxButton(this, ID_EXPORT_JSON_BTN, wxT("Export JSON Report..."));
    m_closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));

    btnSizer->Add(m_runAuditBtn, 0, wxALL, 5);
    btnSizer->Add(m_autoRemediateBtn, 0, wxALL, 5);
    btnSizer->Add(m_exportJsonBtn, 0, wxALL, 5);
    btnSizer->AddStretchSpacer();
    btnSizer->Add(m_closeBtn, 0, wxALL, 5);

    mainSizer->Add(btnSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
    Layout();
    Center();

    // Default Scorecard Gauge Values
    UpdateScorecardUI();
}

void AISequenceValidatorDialog::UpdateScorecardUI() {
    int overall = static_cast<int>(m_lastResult.scorecard.overallHealthScore);
    m_overallHealthGauge->SetValue(overall);
    m_healthScoreText->SetLabel(wxString::Format(wxT("%d%% / 100"), overall));

    m_timingGridGauge->SetValue(static_cast<int>(m_lastResult.scorecard.timingGridScore));
    m_channelOverlapGauge->SetValue(static_cast<int>(m_lastResult.scorecard.channelOverlapScore));
    m_hardwareSafetyGauge->SetValue(static_cast<int>(m_lastResult.scorecard.hardwareSafetyScore));
    m_visualHarmonyGauge->SetValue(static_cast<int>(m_lastResult.scorecard.visualHarmonyScore));
}

void AISequenceValidatorDialog::PopulateIssuesList() {
    m_issuesListCtrl->DeleteAllItems();
    long index = 0;
    for (const auto& issue : m_lastResult.issues) {
        long row = m_issuesListCtrl->InsertItem(index, wxString::FromUTF8(issue.issueId));

        wxString sevStr = wxT("Warning");
        if (issue.severity == ValidationIssueSeverity::Error || issue.severity == ValidationIssueSeverity::ERROR) sevStr = wxT("Error");
        else if (issue.severity == ValidationIssueSeverity::Critical || issue.severity == ValidationIssueSeverity::CRITICAL_ERROR) sevStr = wxT("Critical");
        else if (issue.severity == ValidationIssueSeverity::Info || issue.severity == ValidationIssueSeverity::INFO) sevStr = wxT("Info");

        m_issuesListCtrl->SetItem(row, 1, sevStr);
        m_issuesListCtrl->SetItem(row, 2, wxString::FromUTF8(issue.category));
        m_issuesListCtrl->SetItem(row, 3, wxString::FromUTF8(issue.message));
        m_issuesListCtrl->SetItem(row, 4, wxString::FromUTF8(issue.suggestedFix));
        index++;
    }
}

void AISequenceValidatorDialog::UpdateCritiquePanel() {
    if (!m_lastResult.personaCritiqueBody.empty()) {
        m_critiqueTextCtrl->SetValue(wxString::FromUTF8(m_lastResult.personaCritiqueBody));
    } else {
        m_critiqueTextCtrl->SetValue(wxT("No persona critique available."));
    }
}

void AISequenceValidatorDialog::OnRunAuditButtonClick(wxCommandEvent& WXUNUSED(event)) {
    // Determine selected persona mode
    int personaSel = m_personaChoice->GetSelection();
    if (personaSel == 0) {
        m_config.personaMode = PersonaReviewMode::MASTER_SEQUENCER_BOSS;
        m_config.reviewMode = PersonaReviewMode::MASTER_SEQUENCER_BOSS;
    } else if (personaSel == 1) {
        m_config.personaMode = PersonaReviewMode::LIGHT_SHOW_JOURNALIST;
        m_config.reviewMode = PersonaReviewMode::LIGHT_SHOW_JOURNALIST;
    } else {
        m_config.personaMode = PersonaReviewMode::ENTHUSIAST_COACH;
        m_config.reviewMode = PersonaReviewMode::ENTHUSIAST_COACH;
    }

    // Live Sequence Data Binding: Populate configuration from active xLights sequence
    if (xLightsFrame::CurrentSeqXmlFile && xLightsFrame::CurrentSeqXmlFile->GetSequenceLoaded()) {
        m_config.sequenceFilePath = xLightsFrame::CurrentSeqXmlFile->GetFullPath().ToStdString();
        m_config.totalDurationMs = xLightsFrame::CurrentSeqXmlFile->GetSequenceLengthMS();
        m_config.activeEffectCount = xLightsFrame::CurrentSeqXmlFile->GetTotalEffectCount();
        m_config.xsqXmlContent = xLightsFrame::CurrentSeqXmlFile->GetRawXMLContent();
    } else {
        if (m_config.totalDurationMs <= 0) m_config.totalDurationMs = 60000;
        if (m_config.activeEffectCount <= 0) m_config.activeEffectCount = 50;
    }

    m_lastResult = SequenceValidatorAI::ValidateSequenceDiagnostics(m_config);

    UpdateScorecardUI();
    PopulateIssuesList();
    UpdateCritiquePanel();

    spdlog::info("AISequenceValidatorDialog: Audit executed. Health score: {}", m_lastResult.scorecard.overallHealthScore);
}

void AISequenceValidatorDialog::OnAutoRemediateButtonClick(wxCommandEvent& WXUNUSED(event)) {
    if (m_lastResult.issues.empty()) {
        wxMessageBox(wxT("No issues detected to remediate."), wxT("Auto-Remediate"), wxOK | wxICON_INFORMATION, this);
        return;
    }

    std::string remediated = SequenceValidatorAI::AutoRemediateSequence(m_config.xsqXmlContent, m_lastResult.issues);
    m_config.xsqXmlContent = remediated;
    m_lastResult.remediatedSequenceXML = remediated;

    wxMessageBox(wxT("Auto-Remediation complete! XML issue fixes applied."), wxT("Auto-Remediate XML"), wxOK | wxICON_INFORMATION, this);
}

void AISequenceValidatorDialog::OnExportJSONButtonClick(wxCommandEvent& WXUNUSED(event)) {
    std::string jsonStr = SequenceValidatorAI::ExportValidationReportJSON(m_lastResult);

    wxFileDialog saveFileDialog(this, wxT("Save Audit Report JSON"), wxT(""), wxT("sequence_audit_report.json"), wxT("JSON files (*.json)|*.json"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) return;

    std::ofstream file(saveFileDialog.GetPath().ToStdString());
    if (file.is_open()) {
        file << jsonStr;
        file.close();
        wxMessageBox(wxT("Validation report successfully saved to JSON."), wxT("Export Report"), wxOK | wxICON_INFORMATION, this);
    }
}

void AISequenceValidatorDialog::OnPersonaChoiceSelected(wxCommandEvent& event) {
    if (m_lastResult.success) {
        OnRunAuditButtonClick(event);
    }
}

void AISequenceValidatorDialog::OnCloseButtonClick(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
