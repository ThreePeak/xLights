/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <string>
#include <vector>
#include "AI/ShowLogDiagnosticsAnalyzer.h"

namespace xLights::AI {

class AIShowDiagnosticsDialog : public wxDialog {
public:
    AIShowDiagnosticsDialog(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& title = wxT("AI Show Diagnostics & Log Analyzer"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(880, 640),
        long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    );

    virtual ~AIShowDiagnosticsDialog() = default;

private:
    void InitUI();
    void PopulateIssuesList();
    void RunFullShowAudit();
    void LoadAndParseLogFile(const wxString& logFilePath);
    void AutoRepairSelectedIssue();
    void ExportSupportBundle();

    // Event Handlers
    void OnRunAuditClicked(wxCommandEvent& event);
    void OnLoadLogClicked(wxCommandEvent& event);
    void OnAutoRepairClicked(wxCommandEvent& event);
    void OnExportBundleClicked(wxCommandEvent& event);
    void OnFilterChanged(wxCommandEvent& event);
    void OnIssueSelected(wxListEvent& event);
    void OnCloseClicked(wxCommandEvent& event);

    // Controls
    wxNotebook* m_notebook{nullptr};
    wxListCtrl* m_issuesListCtrl{nullptr};
    wxTextCtrl* m_detailsTextCtrl{nullptr};
    wxTextCtrl* m_remediationTextCtrl{nullptr};
    wxChoice* m_severityFilterChoice{nullptr};
    wxStaticText* m_statusLabel{nullptr};
    wxStaticText* m_statsLabel{nullptr};
    wxButton* m_autoRepairBtn{nullptr};
    wxButton* m_exportBundleBtn{nullptr};

    ShowDiagnosticReport m_currentReport;
    int m_selectedIssueIndex{-1};

    enum {
        ID_RUN_AUDIT_BTN = 10001,
        ID_LOAD_LOG_BTN,
        ID_AUTO_REPAIR_BTN,
        ID_EXPORT_BUNDLE_BTN,
        ID_SEVERITY_FILTER,
        ID_ISSUES_LIST
    };

    wxDECLARE_EVENT_TABLE();
};

} // namespace xLights::AI
