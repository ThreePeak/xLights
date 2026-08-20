/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AINeuralShaderSynthesizerDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace xLights {

enum {
    ID_BTN_SYNTHESIZE_SHADER = wxID_HIGHEST + 901,
    ID_BTN_SAVE_PRESET,
    ID_BTN_EXPORT_GLSL,
    ID_BTN_UNDO,
    ID_BTN_REDO
};

AINeuralShaderSynthesizerDialog::AINeuralShaderSynthesizerDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style
) : wxDialog(parent, id, title, pos, size, style),
    m_commandHistory(100) {
    CreateControls();
}

void AINeuralShaderSynthesizerDialog::CreateControls() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(38, 28, 48));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Neural Shader Synthesizer (GLSL / ISF GPU Pixel Engine)"));
    titleTxt->SetForegroundColour(wxColour(255, 255, 255));
    auto font = titleTxt->GetFont();
    font.SetPointSize(12);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);
    bannerSizer->Add(titleTxt, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Center Workspace
    auto* workSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* leftPanel = new wxPanel(this, wxID_ANY);
    BuildLeftControlsPanel(leftPanel);
    workSizer->Add(leftPanel, 0, wxEXPAND | wxALL, 8);

    auto* rightPanel = new wxPanel(this, wxID_ANY);
    BuildRightShaderCanvas(rightPanel);
    workSizer->Add(rightPanel, 1, wxEXPAND | wxALL, 8);

    mainSizer->Add(workSizer, 1, wxEXPAND);

    // Bottom Action Bar
    auto* botBar = new wxPanel(this, wxID_ANY);
    auto* botSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* btnSynth = new wxButton(botBar, ID_BTN_SYNTHESIZE_SHADER, wxT("✨ Synthesize Shader Code"));
    btnSynth->SetBackgroundColour(wxColour(160, 40, 220));
    btnSynth->SetForegroundColour(wxColour(255, 255, 255));

    m_btnUndo = new wxButton(botBar, ID_BTN_UNDO, wxT("↶ Undo (Ctrl+Z)"));
    m_btnRedo = new wxButton(botBar, ID_BTN_REDO, wxT("↷ Redo (Ctrl+Y)"));
    m_btnUndo->Enable(false);
    m_btnRedo->Enable(false);

    auto* btnSavePreset = new wxButton(botBar, ID_BTN_SAVE_PRESET, wxT("💾 Save to Shader Preset..."));
    auto* btnExportGlsl = new wxButton(botBar, ID_BTN_EXPORT_GLSL, wxT("📄 Export .frag / .fs..."));
    auto* btnClose = new wxButton(botBar, wxID_CANCEL, wxT("Close"));

    botSizer->Add(btnSynth, 0, wxALL, 4);
    botSizer->Add(m_btnUndo, 0, wxALL, 4);
    botSizer->Add(m_btnRedo, 0, wxALL, 4);
    botSizer->Add(btnSavePreset, 0, wxALL, 4);
    botSizer->Add(btnExportGlsl, 0, wxALL, 4);
    botSizer->AddStretchSpacer();
    botSizer->Add(btnClose, 0, wxALL, 4);

    botBar->SetSizer(botSizer);
    mainSizer->Add(botBar, 0, wxEXPAND | wxALL, 6);

    SetSizer(mainSizer);

    // Event Bindings
    Bind(wxEVT_BUTTON, &AINeuralShaderSynthesizerDialog::OnSynthesizeShader, this, ID_BTN_SYNTHESIZE_SHADER);
    Bind(wxEVT_BUTTON, &AINeuralShaderSynthesizerDialog::OnSaveShaderPreset, this, ID_BTN_SAVE_PRESET);
    Bind(wxEVT_BUTTON, &AINeuralShaderSynthesizerDialog::OnExportGlslFile, this, ID_BTN_EXPORT_GLSL);
    Bind(wxEVT_BUTTON, &AINeuralShaderSynthesizerDialog::OnUndo, this, ID_BTN_UNDO);
    Bind(wxEVT_BUTTON, &AINeuralShaderSynthesizerDialog::OnRedo, this, ID_BTN_REDO);
}

void AINeuralShaderSynthesizerDialog::BuildLeftControlsPanel(wxPanel* parent) {
    auto* sizer = new wxStaticBoxSizer(wxVERTICAL, parent, wxT("Natural Language Shader Prompt"));

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Visual Phenomenon / Shader Prompt:")), 0, wxTOP | wxLEFT, 4);
    m_txtPrompt = new wxTextCtrl(parent, wxID_ANY, wxT("Photorealistic rainbow nebula hyperdrive warp pulsing to audio bass"),
                                 wxDefaultPosition, wxSize(280, 100), wxTE_MULTILINE);
    sizer->Add(m_txtPrompt, 0, wxEXPAND | wxALL, 4);

    m_chkAudioReactive = new wxCheckBox(parent, wxID_ANY, wxT("Bind Audio Spectrum Uniforms (u_bass, u_treble)"));
    m_chkAudioReactive->SetValue(true);
    m_chkAudioReactive->SetForegroundColour(wxColour(255, 120, 220));
    sizer->Add(m_chkAudioReactive, 0, wxALL, 6);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Animation Speed Multiplier:")), 0, wxTOP | wxLEFT, 4);
    m_sliderSpeed = new wxSlider(parent, wxID_ANY, 100, 20, 300);
    sizer->Add(m_sliderSpeed, 0, wxEXPAND | wxALL, 4);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Color Saturation & Contrast:")), 0, wxTOP | wxLEFT, 4);
    m_sliderSaturation = new wxSlider(parent, wxID_ANY, 100, 0, 200);
    sizer->Add(m_sliderSaturation, 0, wxEXPAND | wxALL, 4);

    parent->SetSizer(sizer);
}

void AINeuralShaderSynthesizerDialog::BuildRightShaderCanvas(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    auto* notebook = new wxNotebook(parent, wxID_ANY);

    // Tab 1: Live 60 FPS Canvas Preview
    m_shaderPreviewCanvas = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
    m_shaderPreviewCanvas->SetBackgroundColour(wxColour(12, 10, 18));
    notebook->AddPage(m_shaderPreviewCanvas, wxT("👁️ 60 FPS Real-Time Shader Preview"));

    // Tab 2: GLSL Code Editor
    auto* codePanel = new wxPanel(notebook, wxID_ANY);
    auto* codeSizer = new wxBoxSizer(wxVERTICAL);
    m_txtGlslSourceCode = new wxTextCtrl(codePanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    m_txtGlslSourceCode->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    codeSizer->Add(m_txtGlslSourceCode, 1, wxEXPAND | wxALL, 4);
    codePanel->SetSizer(codeSizer);
    notebook->AddPage(codePanel, wxT("💻 GLSL 3.30 Source Code"));

    // Tab 3: Exposed Uniform Parameters
    auto* uniformPanel = new wxPanel(notebook, wxID_ANY);
    auto* uniformSizer = new wxBoxSizer(wxVERTICAL);
    m_uniformListCtrl = new wxListCtrl(uniformPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    m_uniformListCtrl->InsertColumn(0, wxT("Uniform Name"), wxLIST_FORMAT_LEFT, 130);
    m_uniformListCtrl->InsertColumn(1, wxT("Type"), wxLIST_FORMAT_LEFT, 80);
    m_uniformListCtrl->InsertColumn(2, wxT("Default Value"), wxLIST_FORMAT_LEFT, 110);
    m_uniformListCtrl->InsertColumn(3, wxT("Binding Description"), wxLIST_FORMAT_LEFT, 280);
    uniformSizer->Add(m_uniformListCtrl, 1, wxEXPAND | wxALL, 4);
    uniformPanel->SetSizer(uniformSizer);
    notebook->AddPage(uniformPanel, wxT("🎛️ Uniform Bindings"));

    sizer->Add(notebook, 1, wxEXPAND | wxBOTTOM, 4);

    m_lblCompileStatus = new wxStaticText(parent, wxID_ANY, wxT("Status: Ready to synthesize"));
    m_lblCompileStatus->SetForegroundColour(wxColour(0, 230, 100));
    sizer->Add(m_lblCompileStatus, 0, wxALL, 4);

    parent->SetSizer(sizer);
}

void AINeuralShaderSynthesizerDialog::OnSynthesizeShader(wxCommandEvent& WXUNUSED(event)) {
    if (m_txtPrompt) m_params.userPrompt = m_txtPrompt->GetValue().ToStdString();
    if (m_chkAudioReactive) m_params.audioReactive = m_chkAudioReactive->IsChecked();
    if (m_sliderSpeed) m_params.speedMultiplier = m_sliderSpeed->GetValue() / 100.0f;
    if (m_sliderSaturation) m_params.colorSaturation = m_sliderSaturation->GetValue() / 100.0f;

    auto prev = m_shaderResult;
    m_shaderResult = AI::NeuralShaderSynthesizerAI::SynthesizeShader(m_params);

    auto cmd = std::make_unique<AI::LambdaAICommand>(
        "Synthesize Shader: " + m_shaderResult.shaderName,
        [this]() { return true; },
        [this, prev]() {
            m_shaderResult = prev;
            UpdateUiFromResults();
            return true;
        }
    );

    m_commandHistory.ExecuteCommand(std::move(cmd));
    UpdateUiFromResults();
}

void AINeuralShaderSynthesizerDialog::UpdateUiFromResults() {
    if (m_txtGlslSourceCode) {
        m_txtGlslSourceCode->SetValue(wxString::FromUTF8(m_shaderResult.glslSourceCode));
    }

    if (m_uniformListCtrl) {
        m_uniformListCtrl->DeleteAllItems();
        for (size_t i = 0; i < m_shaderResult.uniforms.size(); ++i) {
            const auto& u = m_shaderResult.uniforms[i];
            long idx = m_uniformListCtrl->InsertItem(static_cast<long>(i), wxString::FromUTF8(u.name));
            m_uniformListCtrl->SetItem(idx, 1, wxString::FromUTF8(u.type));
            m_uniformListCtrl->SetItem(idx, 2, wxString::Format(wxT("%.2f"), u.defaultValue));
            m_uniformListCtrl->SetItem(idx, 3, wxString::FromUTF8(u.description));
        }
    }

    if (m_lblCompileStatus) {
        m_lblCompileStatus->SetLabel(wxString::FromUTF8(m_shaderResult.compilationLog));
        m_lblCompileStatus->SetForegroundColour(m_shaderResult.compilationSuccess ? wxColour(0, 230, 100) : wxColour(255, 60, 60));
    }

    UpdateUndoRedoButtons();
}

void AINeuralShaderSynthesizerDialog::UpdateUndoRedoButtons() {
    if (m_btnUndo) {
        m_btnUndo->Enable(m_commandHistory.CanUndo());
        m_btnUndo->SetToolTip(m_commandHistory.CanUndo()
            ? wxString::Format(wxT("Undo: %s"), wxString::FromUTF8(m_commandHistory.GetUndoDescription()))
            : wxT("Nothing to Undo"));
    }
    if (m_btnRedo) {
        m_btnRedo->Enable(m_commandHistory.CanRedo());
        m_btnRedo->SetToolTip(m_commandHistory.CanRedo()
            ? wxString::Format(wxT("Redo: %s"), wxString::FromUTF8(m_commandHistory.GetRedoDescription()))
            : wxT("Nothing to Redo"));
    }
}

void AINeuralShaderSynthesizerDialog::OnUndo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Undo()) {
        UpdateUiFromResults();
    }
}

void AINeuralShaderSynthesizerDialog::OnRedo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Redo()) {
        UpdateUiFromResults();
    }
}

void AINeuralShaderSynthesizerDialog::OnSaveShaderPreset(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Save xLights Shader Preset"), wxEmptyString,
                         wxT("Neural_Shader.xshader"),
                         wxT("xLights Shader (*.xshader)|*.xshader"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        std::ofstream out(saveDlg.GetPath().ToStdString());
        out << m_shaderResult.ExportShaderXmlPreset();
        spdlog::info("AINeuralShaderSynthesizerDialog: Exported shader preset to '{}'", saveDlg.GetPath().ToStdString());
    }
}

void AINeuralShaderSynthesizerDialog::OnExportGlslFile(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Export GLSL Fragment Shader"), wxEmptyString,
                         wxT("Neural_Shader.frag"),
                         wxT("Fragment Shader (*.frag;*.fs)|*.frag;*.fs"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        std::ofstream out(saveDlg.GetPath().ToStdString());
        out << m_shaderResult.glslSourceCode;
        spdlog::info("AINeuralShaderSynthesizerDialog: Exported GLSL file to '{}'", saveDlg.GetPath().ToStdString());
    }
}

} // namespace xLights
