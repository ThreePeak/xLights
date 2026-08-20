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
#include <wx/slider.h>
#include <wx/checkbox.h>
#include "src-core/render/NeuralShaderSynthesizerAI.h"
#include "src-core/ai/AICommandHistory.h"

namespace xLights {

class AINeuralShaderSynthesizerDialog : public wxDialog {
public:
    AINeuralShaderSynthesizerDialog(wxWindow* parent,
                                    wxWindowID id = wxID_ANY,
                                    const wxString& title = wxT("AI Neural Shader Synthesizer (GLSL / ISF GPU Engine)"),
                                    const wxPoint& pos = wxDefaultPosition,
                                    const wxSize& size = wxSize(1080, 780),
                                    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);

    virtual ~AINeuralShaderSynthesizerDialog() = default;

    const AI::SynthesizedShaderResult& GetShaderResult() const { return m_shaderResult; }

    void OnSynthesizeShader(wxCommandEvent& event);
    void OnSaveShaderPreset(wxCommandEvent& event);
    void OnExportGlslFile(wxCommandEvent& event);

    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);

private:
    void CreateControls();
    void BuildLeftControlsPanel(wxPanel* parent);
    void BuildRightShaderCanvas(wxPanel* parent);
    void UpdateUiFromResults();
    void UpdateUndoRedoButtons();

    AI::ShaderPromptParameters m_params;
    AI::SynthesizedShaderResult m_shaderResult;
    AI::AICommandHistory m_commandHistory;

    // UI Widgets
    wxTextCtrl* m_txtPrompt{nullptr};
    wxCheckBox* m_chkAudioReactive{nullptr};
    wxSlider* m_sliderSpeed{nullptr};
    wxSlider* m_sliderSaturation{nullptr};

    wxButton* m_btnUndo{nullptr};
    wxButton* m_btnRedo{nullptr};

    wxPanel* m_shaderPreviewCanvas{nullptr};
    wxTextCtrl* m_txtGlslSourceCode{nullptr};
    wxListCtrl* m_uniformListCtrl{nullptr};
    wxStaticText* m_lblCompileStatus{nullptr};
};

} // namespace xLights
