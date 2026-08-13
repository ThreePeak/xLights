/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/notebook.h>
#include <wx/filepicker.h>
#include <wx/button.h>
#include <wx/sizer.h>

#include "AI/LocalInferenceEngine.h"

namespace xLights::AI {

class AIInferenceSettingsPanel : public wxPanel {
public:
    AIInferenceSettingsPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    virtual ~AIInferenceSettingsPanel() = default;

private:
    void InitUI();
    void LoadSettingsFromConfig();
    void SaveSettingsToConfig();

    // Notebook & Tabs
    wxNotebook* m_notebook = nullptr;

    // Hardware & Local Execution Controls
    wxChoice* m_backendChoice = nullptr;
    wxSlider* m_memCapSlider = nullptr;
    wxChoice* m_precisionChoice = nullptr;
    wxSpinCtrl* m_threadSpin = nullptr;
    wxDirPickerCtrl* m_onnxDirPicker = nullptr;
    wxStaticText* m_backendStatusLabel = nullptr;

    // Cloud API Credentials & Test Buttons
    wxTextCtrl* m_openaiKeyCtrl = nullptr;
    wxButton* m_testOpenAIBtn = nullptr;

    wxTextCtrl* m_anthropicKeyCtrl = nullptr;
    wxButton* m_testAnthropicBtn = nullptr;

    wxTextCtrl* m_geminiKeyCtrl = nullptr;
    wxButton* m_testGeminiBtn = nullptr;

    wxTextCtrl* m_deepseekKeyCtrl = nullptr;
    wxButton* m_testDeepseekBtn = nullptr;

    wxTextCtrl* m_customEndpointCtrl = nullptr;
    wxTextCtrl* m_ollamaEndpointCtrl = nullptr;
    wxChoice* m_primaryModelChoice = nullptr;

    // LLM Hyperparameters
    wxSlider* m_temperatureSlider = nullptr;
    wxStaticText* m_tempValueLabel = nullptr;
    wxSlider* m_topPSlider = nullptr;
    wxStaticText* m_topPValueLabel = nullptr;
    wxSpinCtrl* m_maxTokensSpin = nullptr;
    wxSlider* m_freqPenaltySlider = nullptr;
    wxSlider* m_presPenaltySlider = nullptr;

    // Custom System Persona Instructions
    wxTextCtrl* m_systemPromptCtrl = nullptr;
};

} // namespace xLights::AI
