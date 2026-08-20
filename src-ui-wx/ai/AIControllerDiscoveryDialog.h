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
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>

#include "src-core/controllers/AIControllerDiscoveryMapper.h"
#include "src-core/ai/AICommandHistory.h"

namespace xLights::AI {

class AIControllerDiscoveryDialog : public wxDialog {
public:
    AIControllerDiscoveryDialog(wxWindow* parent,
                                wxWindowID id = wxID_ANY,
                                const wxString& title = wxT("AI Universal Controller Discovery & Model Auto-Mapper"),
                                const wxPoint& pos = wxDefaultPosition,
                                const wxSize& size = wxSize(920, 660),
                                long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);
    virtual ~AIControllerDiscoveryDialog() = default;

    const ControllerDiscoveryReport& GetLastReport() const { return m_lastReport; }

private:
    void InitUI();

    void OnScanSubnetClick(wxCommandEvent& event);
    void OnApplyApprovedMappingsClick(wxCommandEvent& event);
    void OnUndoClick(wxCommandEvent& event);
    void OnRedoClick(wxCommandEvent& event);
    void OnExportReportClick(wxCommandEvent& event);
    void OnHelpClick(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);
    void UpdateUndoRedoState();

    wxTextCtrl* m_txtSubnetCidr{nullptr};
    wxListCtrl* m_listDevices{nullptr};
    wxListCtrl* m_listProposals{nullptr};
    wxStaticText* m_lblStatus{nullptr};

    wxButton* m_btnScan{nullptr};
    wxButton* m_btnApply{nullptr};
    wxButton* m_btnUndo{nullptr};
    wxButton* m_btnRedo{nullptr};
    wxButton* m_btnExportReport{nullptr};
    wxButton* m_btnClose{nullptr};

    AICommandHistory m_commandHistory;
    ControllerDiscoveryReport m_lastReport;
    std::vector<ModelControllerBindingProposal> m_appliedState;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
