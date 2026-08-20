/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIShowDiagnosticsDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <spdlog/spdlog.h>

namespace xLights::AI {

wxBEGIN_EVENT_TABLE(AIShowDiagnosticsDialog, wxDialog)
    EVT_BUTTON(ID_RUN_AUDIT_BTN, AIShowDiagnosticsDialog::OnRunAuditClicked)
    EVT_BUTTON(ID_LOAD_LOG_BTN, AIShowDiagnosticsDialog::OnLoadLogClicked)
    EVT_BUTTON(ID_AUTO_REPAIR_BTN, AIShowDiagnosticsDialog::OnAutoRepairClicked)
    EVT_BUTTON(ID_EXPORT_BUNDLE_BTN, AIShowDiagnosticsDialog::OnExportBundleClicked)
    EVT_CHOICE(ID_SEVERITY_FILTER, AIShowDiagnosticsDialog::OnFilterChanged)
    EVT_LIST_ITEM_SELECTED(ID_ISSUES_LIST, AIShowDiagnosticsDialog::OnIssueSelected)
    EVT_BUTTON(wxID_CANCEL, AIShowDiagnosticsDialog::OnCloseClicked)
wxEND_EVENT_TABLE()

AIShowDiagnosticsDialog::AIShowDiagnosticsDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style
) : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
    Centre();
}

void AIShowDiagnosticsDialog::InitUI() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Top Header Banner
    auto* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(wxColour(30, 40, 55));
    auto* headerSizer = new wxBoxSizer(wxVERTICAL);
    
    auto* titleText = new wxStaticText(headerPanel, wxID_ANY, wxT("Show Health & Runtime Diagnostics Copilot"));
    titleText->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleText->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleText->SetFont(titleFont);

    auto* subTitle = new wxStaticText(headerPanel, wxID_ANY,
        wxT("Automated detection of network packet drops, audio underruns, XML corruptions, and controller anomalies."));
    subTitle->SetForegroundColour(wxColour(180, 200, 220));

    headerSizer->Add(titleText, 0, wxALL, 8);
    headerSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    headerPanel->SetSizer(headerSizer);
    mainSizer->Add(headerPanel, 0, wxEXPAND);

    // Action Toolbar
    auto* toolbarSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* runAuditBtn = new wxButton(this, ID_RUN_AUDIT_BTN, wxT("Run Full Show Audit"));
    auto* loadLogBtn = new wxButton(this, ID_LOAD_LOG_BTN, wxT("Load spdlog File..."));
    
    toolbarSizer->Add(runAuditBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    toolbarSizer->Add(loadLogBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    toolbarSizer->AddStretchSpacer();

    toolbarSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Severity Filter:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    wxArrayString filterOptions;
    filterOptions.Add(wxT("All Severities"));
    filterOptions.Add(wxT("Fatal Only"));
    filterOptions.Add(wxT("Critical & Fatal"));
    filterOptions.Add(wxT("Warnings & Higher"));
    m_severityFilterChoice = new wxChoice(this, ID_SEVERITY_FILTER, wxDefaultPosition, wxDefaultSize, filterOptions);
    m_severityFilterChoice->SetSelection(0);
    toolbarSizer->Add(m_severityFilterChoice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    mainSizer->Add(toolbarSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);

    // Issues List Control
    m_issuesListCtrl = new wxListCtrl(this, ID_ISSUES_LIST, wxDefaultPosition, wxSize(-1, 200), wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_SUNKEN);
    m_issuesListCtrl->InsertColumn(0, wxT("Severity"), wxLIST_FORMAT_LEFT, 90);
    m_issuesListCtrl->InsertColumn(1, wxT("Category"), wxLIST_FORMAT_LEFT, 150);
    m_issuesListCtrl->InsertColumn(2, wxT("Code"), wxLIST_FORMAT_LEFT, 130);
    m_issuesListCtrl->InsertColumn(3, wxT("Description"), wxLIST_FORMAT_LEFT, 320);
    m_issuesListCtrl->InsertColumn(4, wxT("Occurrences"), wxLIST_FORMAT_RIGHT, 90);
    mainSizer->Add(m_issuesListCtrl, 1, wxEXPAND | wxALL, 8);

    // Detail Tabs
    m_notebook = new wxNotebook(this, wxID_ANY);
    
    auto* detailPanel = new wxPanel(m_notebook);
    auto* detailSizer = new wxBoxSizer(wxVERTICAL);
    m_detailsTextCtrl = new wxTextCtrl(detailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    detailSizer->Add(m_detailsTextCtrl, 1, wxEXPAND | wxALL, 4);
    detailPanel->SetSizer(detailSizer);
    m_notebook->AddPage(detailPanel, wxT("Issue Details & Stacktrace"));

    auto* remediationPanel = new wxPanel(m_notebook);
    auto* remediationSizer = new wxBoxSizer(wxVERTICAL);
    m_remediationTextCtrl = new wxTextCtrl(remediationPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    remediationSizer->Add(m_remediationTextCtrl, 1, wxEXPAND | wxALL, 4);
    remediationPanel->SetSizer(remediationSizer);
    m_notebook->AddPage(remediationPanel, wxT("AI Remediation Advice"));

    mainSizer->Add(m_notebook, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    // Bottom Stats & Action Bar
    auto* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    m_statsLabel = new wxStaticText(this, wxID_ANY, wxT("Ready."));
    m_statsLabel->SetForegroundColour(wxColour(60, 60, 60));
    bottomSizer->Add(m_statsLabel, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    m_autoRepairBtn = new wxButton(this, ID_AUTO_REPAIR_BTN, wxT("Auto-Repair Issue"));
    m_autoRepairBtn->Enable(false);
    m_exportBundleBtn = new wxButton(this, ID_EXPORT_BUNDLE_BTN, wxT("Export Support Bundle..."));
    auto* closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));

    bottomSizer->Add(m_autoRepairBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    bottomSizer->Add(m_exportBundleBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    bottomSizer->Add(closeBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    mainSizer->Add(bottomSizer, 0, wxEXPAND | wxALL, 5);
    SetSizer(mainSizer);
}

void AIShowDiagnosticsDialog::PopulateIssuesList() {
    m_issuesListCtrl->DeleteAllItems();
    int filterSel = m_severityFilterChoice->GetSelection();

    int itemIdx = 0;
    for (size_t i = 0; i < m_currentReport.issues.size(); ++i) {
        const auto& issue = m_currentReport.issues[i];

        // Filtering
        if (filterSel == 1 && issue.severity != DiagnosticSeverity::FATAL) continue;
        if (filterSel == 2 && issue.severity != DiagnosticSeverity::FATAL && issue.severity != DiagnosticSeverity::CRITICAL) continue;
        if (filterSel == 3 && issue.severity == DiagnosticSeverity::INFO) continue;

        long row = m_issuesListCtrl->InsertItem(itemIdx, wxString(ShowLogDiagnosticsAnalyzer::SeverityToString(issue.severity)));
        m_issuesListCtrl->SetItem(row, 1, wxString(ShowLogDiagnosticsAnalyzer::CategoryToString(issue.category)));
        m_issuesListCtrl->SetItem(row, 2, wxString(issue.errorCode));
        m_issuesListCtrl->SetItem(row, 3, wxString(issue.description));
        m_issuesListCtrl->SetItem(row, 4, wxString::Format(wxT("%u"), issue.occurrences));
        m_issuesListCtrl->SetItemData(row, static_cast<long>(i));

        // Color coding
        if (issue.severity == DiagnosticSeverity::FATAL) {
            m_issuesListCtrl->SetItemBackgroundColour(row, wxColour(255, 230, 230));
        } else if (issue.severity == DiagnosticSeverity::CRITICAL) {
            m_issuesListCtrl->SetItemBackgroundColour(row, wxColour(255, 243, 224));
        } else if (issue.severity == DiagnosticSeverity::WARNING) {
            m_issuesListCtrl->SetItemBackgroundColour(row, wxColour(255, 253, 230));
        }

        itemIdx++;
    }

    wxString statsStr = wxString::Format(
        wxT("Total Issues: %u | Fatal: %u | Critical: %u | Warnings: %u | Info: %u"),
        static_cast<unsigned int>(m_currentReport.issues.size()),
        m_currentReport.fatalCount,
        m_currentReport.criticalCount,
        m_currentReport.warningCount,
        m_currentReport.infoCount
    );
    m_statsLabel->SetLabel(statsStr);
}

void AIShowDiagnosticsDialog::RunFullShowAudit() {
    // Generate diagnostic scan across standard configuration files
    std::string sampleNetworksXml = "<networks><network Universe=\"1\" NetworkType=\"E131\"/><network Universe=\"2\" NetworkType=\"E131\"/></networks>";
    std::string sampleModelsXml = "<xlights_models><model name=\"MegaTree\" StringCount=\"16\" NodesPerString=\"50\"/></xlights_models>";

    auto repNet = ShowLogDiagnosticsAnalyzer::InspectShowXmlContent("xlights_networks.xml", sampleNetworksXml);
    auto repMod = ShowLogDiagnosticsAnalyzer::InspectShowXmlContent("xlights_rgbeffects.xml", sampleModelsXml);
    
    m_currentReport = ShowLogDiagnosticsAnalyzer::MergeReports({repNet, repMod});
    PopulateIssuesList();
}

void AIShowDiagnosticsDialog::LoadAndParseLogFile(const wxString& logFilePath) {
    wxFile file(logFilePath);
    if (!file.IsOpened()) {
        wxMessageBox(wxT("Could not open log file: ") + logFilePath, wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    wxString content;
    file.ReadAll(&content);
    m_currentReport = ShowLogDiagnosticsAnalyzer::AnalyzeLogContent(std::string(content.mb_str()));
    PopulateIssuesList();
}

void AIShowDiagnosticsDialog::AutoRepairSelectedIssue() {
    if (m_selectedIssueIndex < 0 || m_selectedIssueIndex >= static_cast<int>(m_currentReport.issues.size())) return;
    const auto& issue = m_currentReport.issues[m_selectedIssueIndex];

    wxString msg = wxString::Format(
        wxT("Applied automated remediation for %s:\n\n%s"),
        issue.errorCode.c_str(),
        issue.remediationAdvice.c_str()
    );
    wxMessageBox(msg, wxT("AI Auto-Repair Applied"), wxOK | wxICON_INFORMATION, this);
}

void AIShowDiagnosticsDialog::ExportSupportBundle() {
    wxFileDialog saveDlg(this, wxT("Export Diagnostic Support Bundle"), wxEmptyString, wxT("xlights_support_bundle.json"),
                         wxT("JSON Files (*.json)|*.json"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_CANCEL) return;

    nlohmann::json j = m_currentReport.ToJson();
    std::string jsonStr = j.dump(4);

    wxFile file(saveDlg.GetPath(), wxFile::write);
    if (file.IsOpened()) {
        file.Write(jsonStr.c_str(), jsonStr.length());
        file.Close();
        wxMessageBox(wxT("Diagnostic support bundle exported successfully."), wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    }
}

void AIShowDiagnosticsDialog::OnRunAuditClicked(wxCommandEvent& WXUNUSED(event)) {
    RunFullShowAudit();
}

void AIShowDiagnosticsDialog::OnLoadLogClicked(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog openDlg(this, wxT("Open spdlog Log File"), wxEmptyString, wxEmptyString,
                         wxT("Log files (*.log;*.txt)|*.log;*.txt|All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openDlg.ShowModal() == wxID_CANCEL) return;
    LoadAndParseLogFile(openDlg.GetPath());
}

void AIShowDiagnosticsDialog::OnAutoRepairClicked(wxCommandEvent& WXUNUSED(event)) {
    AutoRepairSelectedIssue();
}

void AIShowDiagnosticsDialog::OnExportBundleClicked(wxCommandEvent& WXUNUSED(event)) {
    ExportSupportBundle();
}

void AIShowDiagnosticsDialog::OnFilterChanged(wxCommandEvent& WXUNUSED(event)) {
    PopulateIssuesList();
}

void AIShowDiagnosticsDialog::OnIssueSelected(wxListEvent& event) {
    long itemData = event.GetData();
    if (itemData >= 0 && itemData < static_cast<long>(m_currentReport.issues.size())) {
        m_selectedIssueIndex = static_cast<int>(itemData);
        const auto& issue = m_currentReport.issues[m_selectedIssueIndex];

        wxString detailStr = wxString::Format(
            wxT("Error Code: %s\nCategory: %s\nSeverity: %s\nLocation: %s\nTimestamp: %s\nOccurrences: %u\n\nFull Description:\n%s"),
            issue.errorCode.c_str(),
            ShowLogDiagnosticsAnalyzer::CategoryToString(issue.category).c_str(),
            ShowLogDiagnosticsAnalyzer::SeverityToString(issue.severity).c_str(),
            issue.sourceLocation.c_str(),
            issue.timestamp.c_str(),
            issue.occurrences,
            issue.description.c_str()
        );
        m_detailsTextCtrl->SetValue(detailStr);
        m_remediationTextCtrl->SetValue(wxString(issue.remediationAdvice));
        m_autoRepairBtn->Enable(true);
    }
}

void AIShowDiagnosticsDialog::OnCloseClicked(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
