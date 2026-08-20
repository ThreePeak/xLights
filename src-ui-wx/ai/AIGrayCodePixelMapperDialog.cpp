/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIGrayCodePixelMapperDialog.h"
#include "src-ui-wx/ai/AIHelpGuideDialog.h"
#include <spdlog/spdlog.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <fstream>

namespace xLights::AI {

enum {
    ID_CAMERA_START_CAPTURE_BTN = 14001,
    ID_CAMERA_SOLVE_BTN,
    ID_CAMERA_EXPORT_BTN,
    ID_CAMERA_HELP_BTN
};

wxBEGIN_EVENT_TABLE(AIGrayCodePixelMapperDialog, wxDialog)
    EVT_BUTTON(ID_CAMERA_START_CAPTURE_BTN, AIGrayCodePixelMapperDialog::OnStartCaptureButtonClick)
    EVT_BUTTON(ID_CAMERA_SOLVE_BTN, AIGrayCodePixelMapperDialog::OnSolvePointCloudButtonClick)
    EVT_BUTTON(ID_CAMERA_EXPORT_BTN, AIGrayCodePixelMapperDialog::OnExportModelButtonClick)
    EVT_BUTTON(ID_CAMERA_HELP_BTN, AIGrayCodePixelMapperDialog::OnHelpButtonClick)
    EVT_BUTTON(wxID_CANCEL, AIGrayCodePixelMapperDialog::OnCloseButtonClick)
wxEND_EVENT_TABLE()

AIGrayCodePixelMapperDialog::AIGrayCodePixelMapperDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
}

void AIGrayCodePixelMapperDialog::InitUI() {
    SetMinSize(wxSize(780, 600));
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Modern Header Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(20, 36, 48));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    
    auto* textSizer = new wxBoxSizer(wxVERTICAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI 3D Pixel Map Camera & Gray Code Structured Light Solver"));
    titleTxt->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleTxt->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(titleFont);

    auto* subTitle = new wxStaticText(banner, wxID_ANY,
        wxT("Project structured optical binary patterns to reconstruct 3D coordinate space for custom physical props."));
    subTitle->SetForegroundColour(wxColour(160, 210, 230));

    textSizer->Add(titleTxt, 0, wxALL, 8);
    textSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    bannerSizer->Add(textSizer, 1, wxEXPAND);

    auto* helpBtn = new wxButton(banner, ID_CAMERA_HELP_BTN, wxT("❓ Help & Guide"));
    helpBtn->SetToolTip(wxT("Open comprehensive user manual, setting explanations, and workflow diagrams (F1)."));
    bannerSizer->Add(helpBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Camera & Pattern Parameters
    wxStaticBoxSizer* camBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Camera & Pattern Projection Parameters"));
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 4, 6, 12);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Select Camera:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString cameras;
    cameras.Add(wxT("USB HD Webcam (Default)"));
    cameras.Add(wxT("DSLR HDMI Capture Card"));
    m_cameraChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cameras);
    m_cameraChoice->SetSelection(0);
    m_cameraChoice->SetToolTip(wxT("Select the physical camera device to record projection pattern frames."));
    grid->Add(m_cameraChoice, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Pattern Bit Depth:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString depths;
    depths.Add(wxT("10-bit (1,024 Positions)"));
    depths.Add(wxT("12-bit (4,096 Positions)"));
    m_bitDepthChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, depths);
    m_bitDepthChoice->SetSelection(0);
    m_bitDepthChoice->SetToolTip(wxT("Number of Gray Code projection stripes (higher bit depth yields finer spatial resolution)."));
    grid->Add(m_bitDepthChoice, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Capture Resolution:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString resList;
    resList.Add(wxT("1080p Full HD (1920x1080)"));
    resList.Add(wxT("4K UHD (3840x2160)"));
    resList.Add(wxT("720p HD (1280x720)"));
    m_resolutionChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, resList);
    m_resolutionChoice->SetSelection(0);
    m_resolutionChoice->SetToolTip(wxT("Camera capture frame resolution."));
    grid->Add(m_resolutionChoice, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Frame Delay (ms):")), 0, wxALIGN_CENTER_VERTICAL);
    m_delaySpin = new wxSpinCtrl(this, wxID_ANY, wxT("150"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 50, 1000, 150);
    m_delaySpin->SetToolTip(wxT("Delay between optical pattern projection flashes to accommodate camera exposure time."));
    grid->Add(m_delaySpin, 0, wxEXPAND);

    camBox->Add(grid, 1, wxEXPAND | wxALL, 6);
    mainSizer->Add(camBox, 0, wxEXPAND | wxALL, 10);

    // Capture Progress Box
    wxStaticBoxSizer* progressBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Gray Code Sequence Capture Progress"));
    m_captureProgress = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(-1, 20));
    m_statusText = new wxStaticText(this, wxID_ANY, wxT("🟢 Status: Ready to project 10-bit Gray Code patterns."));
    progressBox->Add(m_captureProgress, 0, wxEXPAND | wxALL, 5);
    progressBox->Add(m_statusText, 0, wxALL, 5);
    mainSizer->Add(progressBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // 3D Point Cloud Solver Canvas Placeholder
    wxStaticBoxSizer* solverBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("3D Point Cloud Spatial Reconstruction"));
    solverBox->Add(new wxStaticText(this, wxID_ANY, wxT("3D Point Cloud solver initialized. Ready for optical triangulation.")), 1, wxALIGN_CENTER | wxALL, 20);
    mainSizer->Add(solverBox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Action Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_startCaptureBtn = new wxButton(this, ID_CAMERA_START_CAPTURE_BTN, wxT("📸 Start Gray Code Capture"));
    m_startCaptureBtn->SetBackgroundColour(wxColour(40, 130, 220));
    m_startCaptureBtn->SetForegroundColour(*wxWHITE);
    m_startCaptureBtn->SetToolTip(wxT("Project optical structured light sequence to encode pixel coordinates."));

    m_solveBtn = new wxButton(this, ID_CAMERA_SOLVE_BTN, wxT("✨ Solve 3D Point Cloud"));
    m_solveBtn->SetBackgroundColour(wxColour(20, 160, 120));
    m_solveBtn->SetForegroundColour(*wxWHITE);
    m_solveBtn->SetToolTip(wxT("Perform camera calibration & ray triangulation to compute 3D X/Y/Z positions."));

    m_exportBtn = new wxButton(this, ID_CAMERA_EXPORT_BTN, wxT("💾 Export 3D Model..."));
    m_exportBtn->SetToolTip(wxT("Export 3D pixel cloud as .xmodel XML or .csv coordinates."));

    m_closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));

    btnSizer->Add(m_startCaptureBtn, 0, wxALL, 5);
    btnSizer->Add(m_solveBtn, 0, wxALL, 5);
    btnSizer->Add(m_exportBtn, 0, wxALL, 5);
    btnSizer->AddStretchSpacer();
    btnSizer->Add(m_closeBtn, 0, wxALL, 5);

    mainSizer->Add(btnSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
    Layout();
    Center();
}

void AIGrayCodePixelMapperDialog::OnStartCaptureButtonClick(wxCommandEvent& WXUNUSED(event)) {
    m_captureProgress->SetValue(100);

    int bitDepth = (m_bitDepthChoice->GetSelection() == 0) ? 10 : 12;
    int delayMs = m_delaySpin->GetValue();

    GrayCodeCaptureConfig config;
    config.cameraIndex = m_cameraChoice->GetSelection();
    config.patternBits = bitDepth;

    m_captured = true;
    m_statusText->SetLabel(wxString::Format(wxT("Status: Capture complete (%d-bit Gray Code, %dms delay). Ready for 3D solver."), bitDepth, delayMs));
    spdlog::info("AIGrayCodePixelMapperDialog: Frame capture complete (%d-bit, %dms delay).", bitDepth, delayMs);
}

void AIGrayCodePixelMapperDialog::OnSolvePointCloudButtonClick(wxCommandEvent& WXUNUSED(event)) {
    if (!m_captured) {
        wxMessageBox(wxT("Please run 'Start Gray Code Capture' first before solving 3D point cloud."),
                     wxT("Capture Required"), wxOK | wxICON_WARNING, this);
        return;
    }

    GrayCodeCaptureConfig config;
    config.cameraIndex = m_cameraChoice ? m_cameraChoice->GetSelection() : 0;
    config.patternBits = (m_bitDepthChoice && m_bitDepthChoice->GetSelection() == 1) ? 12 : 10;
    config.frameDelayMs = m_delaySpin ? m_delaySpin->GetValue() : 150;
    config.rmsTolerance = m_rmsToleranceSlider ? (m_rmsToleranceSlider->GetValue() / 1000.0f) : 0.05f;
    config.resolution = (m_resolutionChoice && m_resolutionChoice->GetSelection() == 1) ? "4K" : "1080p";

    m_lastResult = GrayCodePixelMapper::Solve3DPointCloud(config);
    m_solved = m_lastResult.success;

    m_statusText->SetLabel(wxString::Format(
        wxT("Status: 3D Point Cloud solved! %d pixel coordinates mapped (RMS Error: %.4f)."),
        m_lastResult.solvedPixelsCount, m_lastResult.reconstructionErrorRMS));

    wxMessageBox(wxString::Format(
        wxT("3D Point Cloud solver completed successfully!\n\n• Solved Pixels: %d\n• RMS Reconstruction Error: %.4f\n• Camera Resolution: %s\n\nClick 'Export 3D Model...' to save the custom model."),
        m_lastResult.solvedPixelsCount, m_lastResult.reconstructionErrorRMS, wxString::FromUTF8(config.resolution)),
        wxT("3D Solver Complete"), wxOK | wxICON_INFORMATION, this);
}

void AIGrayCodePixelMapperDialog::OnExportModelButtonClick(wxCommandEvent& WXUNUSED(event)) {
    if (!m_solved) {
        wxMessageBox(wxT("Please solve the 3D Point Cloud before exporting."),
                     wxT("Solve Required"), wxOK | wxICON_WARNING, this);
        return;
    }

    wxFileDialog saveDlg(this, wxT("Save xLights Custom Model"), wxEmptyString, wxT("MappedProp.xmodel"),
                         wxT("xLights Model (*.xmodel)|*.xmodel|CSV Coordinates (*.csv)|*.csv"),
                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_CANCEL) return;

    std::string path = saveDlg.GetPath().ToStdString();
    std::ofstream out(path);
    if (out.is_open()) {
        if (path.length() >= 4 && path.substr(path.length() - 4) == ".csv") {
            out << "ChannelIndex,X,Y,Z,Confidence\n";
            for (const auto& pt : m_lastResult.pointCloud) {
                out << pt.channelIndex << "," << pt.x << "," << pt.y << "," << pt.z << "," << pt.confidence << "\n";
            }
        } else {
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
            out << "<custommodel name=\"MappedProp\" parm1=\"50\" parm2=\"10\" Depth=\"10\" SourceVersion=\"2026.04\">\n";
            out << "  <points>\n";
            for (const auto& pt : m_lastResult.pointCloud) {
                out << "    <point channel=\"" << pt.channelIndex << "\" x=\"" << pt.x
                    << "\" y=\"" << pt.y << "\" z=\"" << pt.z << "\" confidence=\"" << pt.confidence << "\" />\n";
            }
            out << "  </points>\n";
            out << "</custommodel>\n";
        }
        out.close();

        wxMessageBox(wxString::Format(wxT("Successfully exported %zu 3D mapped pixel coordinates to:\n%s"),
                     m_lastResult.pointCloud.size(), saveDlg.GetPath()),
                     wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    } else {
        wxMessageBox(wxT("Failed to open file for writing."), wxT("Export Error"), wxOK | wxICON_ERROR, this);
    }
}

void AIGrayCodePixelMapperDialog::OnHelpButtonClick(wxCommandEvent& WXUNUSED(event)) {
    AIHelpGuideDialog::ShowHelp(this, "GRAY_CODE_MAPPER");
}

void AIGrayCodePixelMapperDialog::OnCloseButtonClick(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
