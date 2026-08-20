/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>

#include "src-core/controllers/AIFPPLogSelfHealingAgent.h"
#include "src-core/ai/AICommandHistory.h"

namespace xLights::AI {

class AIFPPLogSelfHealingDialog : public wxDialog {
public:
    AIFPPLogSelfHealingDialog(wxWindow* parent,
                              wxWindowID id = wxID_ANY,
                              const wxString& title = wxT("AI Distributed FPP & ESPixelStick Log Self-Healing Agent"),
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxSize(920, 660),
                              long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);
    virtual ~AIFPPLogSelfHealingDialog() = default;

    const FPPFleetDiagnosticReport& GetLastReport() const { return m_lastReport; }

private:
    void InitUI();

    void OnFetchLogsClick(wxCommandEvent& event);
    void OnSimulateErrorsClick(wxCommandEvent& event);
    void OnApplyRemediationClick(wxCommandEvent& event);
    void OnUndoRemediationClick(wxCommandEvent& event);
    void OnExportReportClick(wxCommandEvent& event);
    void OnHelpClick(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);
    void UpdateUndoState();

    wxListCtrl* m_listProposals{nullptr};
    wxTextCtrl* m_txtLogViewer{nullptr};
    wxStaticText* m_lblStatus{nullptr};

    wxButton* m_btnFetchLogs{nullptr};
    wxButton* m_btnSimulateErrors{nullptr};
    wxButton* m_btnApplyRemediation{nullptr};
    wxButton* m_btnUndoRemediation{nullptr};
    wxButton* m_btnExportReport{nullptr};
    wxButton* m_btnClose{nullptr};

    AIFPPLogSelfHealingAgent m_agent;
    AICommandHistory m_commandHistory;
    FPPFleetDiagnosticReport m_lastReport;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
