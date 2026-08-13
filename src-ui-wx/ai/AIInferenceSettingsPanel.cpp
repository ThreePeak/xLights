/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIInferenceSettingsPanel.h"

namespace xLights::AI {

AIInferenceSettingsPanel::AIInferenceSettingsPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style) {
    InitUI();
}

void AIInferenceSettingsPanel::InitUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBoxSizer* configBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("AI Local Hardware Acceleration Provider Settings"));
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 2, 10, 15);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Execution Provider Backend:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString backends;
    backends.Add(wxT("DirectML (Windows GPU Acceleration)"));
    backends.Add(wxT("CUDA / TensorRT (NVIDIA GPU)"));
    backends.Add(wxT("OpenVINO (Intel iGPU / NPU)"));
    backends.Add(wxT("CoreML (Apple Silicon Neural Engine)"));
    backends.Add(wxT("CPU Fallback (OpenMP Multi-threaded)"));
    m_backendChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, backends);
    m_backendChoice->SetSelection(0);
    grid->Add(m_backendChoice, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("VRAM / Memory Cap (MB):")), 0, wxALIGN_CENTER_VERTICAL);
    m_memCapSlider = new wxSlider(this, wxID_ANY, 4096, 1024, 16384, wxDefaultPosition, wxSize(250, -1));
    grid->Add(m_memCapSlider, 1, wxEXPAND);

    configBox->GetSizer()->Add(grid, 0, wxEXPAND | wxALL, 10);

    m_backendStatusLabel = new wxStaticText(this, wxID_ANY, wxT("Active Backend: DirectML GPU Provider active (Device: DirectX 12 GPU)"));
    configBox->GetSizer()->Add(m_backendStatusLabel, 0, wxALL, 10);

    mainSizer->Add(configBox, 1, wxEXPAND | wxALL, 10);
    SetSizer(mainSizer);
    Layout();
}

} // namespace xLights::AI
