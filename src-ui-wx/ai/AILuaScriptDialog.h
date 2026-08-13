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
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/msgdlg.h>

#include "AI/LuaScriptGenerator.h"

namespace xLights::AI {

class AILuaScriptDialog : public wxDialog {
public:
    AILuaScriptDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("AI Natural Language Lua Script Assistant"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(750, 560), long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AILuaScriptDialog() = default;

    [[nodiscard]] std::string GetGeneratedScript() const { return m_lastGeneratedScript; }

private:
    void InitUI();

    // Event Handlers
    void OnGenerateButtonClick(wxCommandEvent& event);
    void OnValidateButtonClick(wxCommandEvent& event);
    void OnRunScriptButtonClick(wxCommandEvent& event);
    void OnCloseButtonClick(wxCommandEvent& event);

    std::string m_lastGeneratedScript;

    // Controls
    wxChoice* m_modelProviderChoice = nullptr;
    wxSlider* m_temperatureSlider = nullptr;
    wxCheckBox* m_sandboxEnforceChk = nullptr;

    wxTextCtrl* m_promptTextCtrl = nullptr;
    wxTextCtrl* m_scriptEditorCtrl = nullptr;
    wxStaticText* m_statusLabel = nullptr;

    wxButton* m_generateBtn = nullptr;
    wxButton* m_validateBtn = nullptr;
    wxButton* m_runBtn = nullptr;
    wxButton* m_closeBtn = nullptr;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
