/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIInferenceSettingsPanel.h"
#include <wx/confbase.h>

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

    long savedProvider = 0;
    if (wxConfigBase::Get()) {
        wxConfigBase::Get()->Read(wxT("AI_ExecutionProvider"), &savedProvider, 0);
    }
    m_backendChoice->SetSelection(static_cast<int>(savedProvider));
    grid->Add(m_backendChoice, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("VRAM / Memory Cap (MB):")), 0, wxALIGN_CENTER_VERTICAL);
    long savedVRAM = 4096;
    if (wxConfigBase::Get()) {
        wxConfigBase::Get()->Read(wxT("AI_VRAMCapMB"), &savedVRAM, 4096);
    }
    m_memCapSlider = new wxSlider(this, wxID_ANY, static_cast<int>(savedVRAM), 1024, 16384, wxDefaultPosition, wxSize(250, -1));
    grid->Add(m_memCapSlider, 1, wxEXPAND);

    configBox->GetSizer()->Add(grid, 0, wxEXPAND | wxALL, 10);

    m_backendStatusLabel = new wxStaticText(this, wxID_ANY, wxString::Format(wxT("Active Backend: Provider #%ld active (DirectX 12 / DirectML GPU Acceleration)"), savedProvider));
    configBox->GetSizer()->Add(m_backendStatusLabel, 0, wxALL, 10);

    mainSizer->Add(configBox, 1, wxEXPAND | wxALL, 10);
    SetSizer(mainSizer);
    Layout();

    m_backendChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent& event) {
        long sel = event.GetSelection();
        if (wxConfigBase::Get()) {
            wxConfigBase::Get()->Write(wxT("AI_ExecutionProvider"), sel);
        }
        m_backendStatusLabel->SetLabel(wxString::Format(wxT("Active Backend: Provider #%ld active (Hardware Selected)"), sel));
    });

    m_memCapSlider->Bind(wxEVT_SLIDER, [this](wxCommandEvent& event) {
        long val = event.GetInt();
        if (wxConfigBase::Get()) {
            wxConfigBase::Get()->Write(wxT("AI_VRAMCapMB"), val);
        }
    });
}

} // namespace xLights::AI
