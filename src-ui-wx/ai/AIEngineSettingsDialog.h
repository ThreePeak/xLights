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
#include <wx/spinctrl.h>
#include <wx/slider.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include "src-core/ai/AIEngineConfigManager.h"

namespace xLights {

class AIEngineSettingsDialog : public wxDialog {
public:
    AIEngineSettingsDialog(wxWindow* parent,
                           wxWindowID id = wxID_ANY,
                           const wxString& title = wxT("AI Copilot & LLM Engine Settings"),
                           const wxPoint& pos = wxDefaultPosition,
                           const wxSize& size = wxSize(900, 680),
                           long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);

    virtual ~AIEngineSettingsDialog() = default;

    void OnProfileSelected(wxCommandEvent& event);
    void OnNewProfile(wxCommandEvent& event);
    void OnDeleteProfile(wxCommandEvent& event);
    void OnSaveProfile(wxCommandEvent& event);
    void OnTestConnectionPing(wxCommandEvent& event);
    void OnOfflineModeToggled(wxCommandEvent& event);

private:
    void CreateControls();
    void BuildProviderSettingsTab(wxPanel* parent);
    void BuildParametersTab(wxPanel* parent);
    void BuildSafetyAndBudgetTab(wxPanel* parent);
    void LoadProfileToUi(const AI::AIProviderProfile& profile);
    AI::AIProviderProfile ReadProfileFromUi();
    void RefreshProfileList();

    // UI Widgets
    wxChoice* m_choiceProfiles{nullptr};
    wxChoice* m_choiceProviderType{nullptr};
    wxTextCtrl* m_txtProfileName{nullptr};
    wxTextCtrl* m_txtModelName{nullptr};
    wxTextCtrl* m_txtApiKey{nullptr};
    wxTextCtrl* m_txtCustomEndpoint{nullptr};

    wxSlider* m_sliderTemperature{nullptr};
    wxSlider* m_sliderTopP{nullptr};
    wxSpinCtrl* m_spinMaxTokens{nullptr};
    wxSpinCtrl* m_spinTimeout{nullptr};
    wxCheckBox* m_chkStreaming{nullptr};
    wxCheckBox* m_chkOfflineOnly{nullptr};

    wxSpinCtrl* m_spinTokenBudget{nullptr};
    wxStaticText* m_lblTokenUsageStatus{nullptr};
    wxStaticText* m_lblConnectionTestStatus{nullptr};
};

} // namespace xLights
