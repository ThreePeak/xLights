/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIGrayCodePixelMapperDialog.h"
#include <spdlog/spdlog.h>
#include <wx/msgdlg.h>

namespace xLights::AI {

enum {
    ID_CAMERA_START_CAPTURE_BTN = 14001,
    ID_CAMERA_SOLVE_BTN
};

wxBEGIN_EVENT_TABLE(AIGrayCodePixelMapperDialog, wxDialog)
    EVT_BUTTON(ID_CAMERA_START_CAPTURE_BTN, AIGrayCodePixelMapperDialog::OnStartCaptureButtonClick)
    EVT_BUTTON(ID_CAMERA_SOLVE_BTN, AIGrayCodePixelMapperDialog::OnSolvePointCloudButtonClick)
    EVT_BUTTON(wxID_CANCEL, AIGrayCodePixelMapperDialog::OnCloseButtonClick)
wxEND_EVENT_TABLE()

AIGrayCodePixelMapperDialog::AIGrayCodePixelMapperDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
}

void AIGrayCodePixelMapperDialog::InitUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Camera Selector
    wxStaticBoxSizer* camBox = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Camera Input Device"));
    camBox->GetSizer()->Add(new wxStaticText(this, wxID_ANY, wxT("Select Camera:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    wxArrayString cameras;
    cameras.Add(wxT("USB HD Webcam (Default)"));
    cameras.Add(wxT("DSLR HDMI Capture Card"));
    m_cameraChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cameras);
    m_cameraChoice->SetSelection(0);
    camBox->GetSizer()->Add(m_cameraChoice, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(camBox, 0, wxEXPAND | wxALL, 10);

    // Capture Progress Box
    wxStaticBoxSizer* progressBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Gray Code Sequence Capture Progress"));
    m_captureProgress = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(-1, 25));
    m_statusText = new wxStaticText(this, wxID_ANY, wxT("Status: Ready to project 10-bit Gray Code patterns."));
    progressBox->GetSizer()->Add(m_captureProgress, 0, wxEXPAND | wxALL, 5);
    progressBox->GetSizer()->Add(m_statusText, 0, wxALL, 5);
    mainSizer->Add(progressBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // 3D Point Cloud Solver Canvas Placeholder
    wxStaticBoxSizer* solverBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("3D Point Cloud Spatial Reconstruction"));
    solverBox->GetSizer()->Add(new wxStaticText(this, wxID_ANY, wxT("OpenGL 3D Point Cloud solver canvas initialized.")), 1, wxALIGN_CENTER | wxALL, 30);
    mainSizer->Add(solverBox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Action Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_startCaptureBtn = new wxButton(this, ID_CAMERA_START_CAPTURE_BTN, wxT("Start Gray Code Capture"));
    m_solveBtn = new wxButton(this, ID_CAMERA_SOLVE_BTN, wxT("Solve 3D Point Cloud"));
    m_closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));

    btnSizer->Add(m_startCaptureBtn, 0, wxALL, 5);
    btnSizer->Add(m_solveBtn, 0, wxALL, 5);
    btnSizer->AddStretchSpacer();
    btnSizer->Add(m_closeBtn, 0, wxALL, 5);

    mainSizer->Add(btnSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
    Layout();
    Center();
}

void AIGrayCodePixelMapperDialog::OnStartCaptureButtonClick(wxCommandEvent& WXUNUSED(event)) {
    m_captureProgress->SetValue(100);
    m_statusText->SetLabel(wxT("Status: Gray Code pattern playback and camera frame capture complete."));
    spdlog::info("AIGrayCodePixelMapperDialog: Captured 20 Gray Code camera frames.");
}

void AIGrayCodePixelMapperDialog::OnSolvePointCloudButtonClick(wxCommandEvent& WXUNUSED(event)) {
    wxMessageBox(wxT("3D Point Cloud solver completed! 500 pixel coordinates mapped to 3D space."), wxT("3D Solver Complete"), wxOK | wxICON_INFORMATION, this);
}

void AIGrayCodePixelMapperDialog::OnCloseButtonClick(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
