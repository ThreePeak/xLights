/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/panel.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>

class xLightsFrame;

namespace xLights::AI {

class AIAssistantPanel : public wxPanel {
public:
    AIAssistantPanel(wxWindow* parent, xLightsFrame* frame = nullptr, wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxSize(320, 500),
                     long style = wxTAB_TRAVERSAL | wxBORDER_NONE);
    virtual ~AIAssistantPanel() = default;

    void SetDiagnosticWarningCount(int count, const wxString& summary = wxEmptyString);
    void SetActiveModel(const wxString& modelName);

private:
    void InitUI();

    // Studio Hub Launchers
    void OnOpenPropStudio(wxCommandEvent& evt);
    void OnOpenAudioStudio(wxCommandEvent& evt);
    void OnOpenVideoStudio(wxCommandEvent& evt);
    void OnOpenHardwareHub(wxCommandEvent& evt);
    void OnOpenDiagnosticsHub(wxCommandEvent& evt);

    // Prompt Execution
    void OnExecutePrompt(wxCommandEvent& evt);
    void OnFixAllClick(wxCommandEvent& evt);

    xLightsFrame* m_frame = nullptr;

    wxStaticText* m_healthBadge = nullptr;
    wxButton* m_fixBtn = nullptr;

    wxTextCtrl* m_promptCtrl = nullptr;
    wxButton* m_executeBtn = nullptr;
    wxTextCtrl* m_outputCtrl = nullptr;

    wxStaticText* m_modelLabel = nullptr;
    wxStaticText* m_tokensLabel = nullptr;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
