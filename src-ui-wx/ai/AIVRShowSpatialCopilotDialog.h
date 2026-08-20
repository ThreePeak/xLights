/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/statline.h>
#include "src-core/layout/VRShowSpatialCopilotAI.h"
#include "src-core/ai/AICommandHistory.h"

namespace xLights {

class AIVRShowSpatialCopilotDialog : public wxDialog {
public:
    AIVRShowSpatialCopilotDialog(wxWindow* parent,
                                 wxWindowID id = wxID_ANY,
                                 const wxString& title = wxT("AI 3D Layout VR/AR Spatial Walkthrough Copilot"),
                                 const wxPoint& pos = wxDefaultPosition,
                                 const wxSize& size = wxSize(1080, 780),
                                 long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);

    virtual ~AIVRShowSpatialCopilotDialog() = default;

    const AI::VRSpatialSessionState& GetSessionState() const { return m_session; }

    void OnAuditSpatialClearance(wxCommandEvent& event);
    void OnSendSpatialQuery(wxCommandEvent& event);
    void OnExportSpatialReport(wxCommandEvent& event);

    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);

private:
    void CreateControls();
    void BuildLeftSessionPanel(wxPanel* parent);
    void BuildRightCopilotPanel(wxPanel* parent);
    void UpdateUiFromResults();
    void UpdateUndoRedoButtons();

    AI::VRSpatialSessionState m_session;
    AI::AICommandHistory m_commandHistory;

    // UI Widgets
    wxStaticText* m_lblHeadsetStatus{nullptr};
    wxListCtrl* m_obstacleListCtrl{nullptr};
    wxListCtrl* m_clearanceListCtrl{nullptr};

    wxButton* m_btnUndo{nullptr};
    wxButton* m_btnRedo{nullptr};

    wxPanel* m_vrSpatialCanvas{nullptr};
    wxTextCtrl* m_txtChatHistory{nullptr};
    wxTextCtrl* m_txtChatInput{nullptr};
};

} // namespace xLights
