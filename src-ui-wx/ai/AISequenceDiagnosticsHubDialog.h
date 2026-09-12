/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/dialog.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/listctrl.h>
#include <thread>
#include <atomic>
#include <vector>
#include <string>

class xLightsFrame;

namespace xLights::AI {

class AISequenceDiagnosticsHubDialog : public wxDialog {
public:
    AISequenceDiagnosticsHubDialog(wxWindow* parent, xLightsFrame* frame = nullptr, wxWindowID id = wxID_ANY,
                                   const wxString& title = wxT("AI Diagnostics, Version Control & Safety Hub"),
                                   const wxPoint& pos = wxDefaultPosition,
                                   const wxSize& size = wxSize(900, 640),
                                   long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AISequenceDiagnosticsHubDialog();

private:
    void InitUI();

    // Tab 1: Validator & Linter
    wxPanel* CreateValidatorTab(wxWindow* parent);
    void OnRunValidationClick(wxCommandEvent& evt);
    void OnFixAllValidationClick(wxCommandEvent& evt);

    // Tab 2: Snapshots & Time-Travel
    wxPanel* CreateSnapshotsTab(wxWindow* parent);
    void OnTakeSnapshotClick(wxCommandEvent& evt);
    void OnRestoreSnapshotClick(wxCommandEvent& evt);

    // Tab 3: Visual Git & 4-Way Merge
    wxPanel* CreateVisualGitTab(wxWindow* parent);
    void OnRunDiffClick(wxCommandEvent& evt);
    void OnAutoMergeClick(wxCommandEvent& evt);

    // Tab 4: Show Health & spdlog Diagnostics
    wxPanel* CreateLogDiagnosticsTab(wxWindow* parent);
    void OnScanLogsClick(wxCommandEvent& evt);

    void OnCloseClick(wxCommandEvent& evt);

    xLightsFrame* m_frame = nullptr;
    wxNotebook* m_notebook = nullptr;
    wxStaticText* m_statusLabel = nullptr;

    // Validator Controls
    wxListCtrl* m_validatorList = nullptr;
    wxButton* m_runValidationBtn = nullptr;
    wxButton* m_fixAllBtn = nullptr;

    // Snapshots Controls
    wxTextCtrl* m_snapshotNameCtrl = nullptr;
    wxListCtrl* m_snapshotList = nullptr;
    wxButton* m_takeSnapshotBtn = nullptr;
    wxButton* m_restoreSnapshotBtn = nullptr;

    // Git Controls
    wxListCtrl* m_diffList = nullptr;
    wxButton* m_runDiffBtn = nullptr;
    wxButton* m_autoMergeBtn = nullptr;

    // Log Controls
    wxListCtrl* m_logList = nullptr;
    wxButton* m_scanLogsBtn = nullptr;

    // Concurrency
    std::atomic<bool> m_workerCancel{false};
    std::thread m_workerThread;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
