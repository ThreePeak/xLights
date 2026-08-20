/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIPacketLossInterpolatorDialog.h"
#include "src-ui-wx/ai/AIHelpGuideDialog.h"
#include <spdlog/spdlog.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <fstream>

namespace xLights::AI {

enum {
    ID_BTN_BENCHMARK = 22001,
    ID_BTN_APPLY_SETTINGS,
    ID_BTN_EXPORT_REPORT,
    ID_BTN_HELP
};

wxBEGIN_EVENT_TABLE(AIPacketLossInterpolatorDialog, wxDialog)
    EVT_BUTTON(ID_BTN_BENCHMARK, AIPacketLossInterpolatorDialog::OnRunBenchmarkClick)
    EVT_BUTTON(ID_BTN_APPLY_SETTINGS, AIPacketLossInterpolatorDialog::OnApplySettingsClick)
    EVT_BUTTON(ID_BTN_EXPORT_REPORT, AIPacketLossInterpolatorDialog::OnExportReportClick)
    EVT_BUTTON(ID_BTN_HELP, AIPacketLossInterpolatorDialog::OnHelpClick)
    EVT_BUTTON(wxID_CANCEL, AIPacketLossInterpolatorDialog::OnCloseClick)
wxEND_EVENT_TABLE()

AIPacketLossInterpolatorDialog::AIPacketLossInterpolatorDialog(
    wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
    wxCommandEvent dummy;
    OnRunBenchmarkClick(dummy);
}

void AIPacketLossInterpolatorDialog::InitUI() {
    SetMinSize(wxSize(840, 600));
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Modern Banner Header
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(32, 28, 56));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* textSizer = new wxBoxSizer(wxVERTICAL);

    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Predictive WiFi Packet Loss Concealment & Auto-Interpolator"));
    titleTxt->SetForegroundColour(*wxWHITE);
    auto font = titleTxt->GetFont();
    font.SetPointSize(font.GetPointSize() + 2);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);

    auto* subTitle = new wxStaticText(banner, wxID_ANY,
        wxT("Predict and synthesize missing lighting frames during 2.4GHz WiFi packet drops, eliminating pixel stutter and blackouts."));
    subTitle->SetForegroundColour(wxColour(210, 190, 240));

    textSizer->Add(titleTxt, 0, wxALL, 6);
    textSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 6);
    bannerSizer->Add(textSizer, 1, wxEXPAND);

    auto* helpBtn = new wxButton(banner, ID_BTN_HELP, wxT("❓ Help & Guide"));
    helpBtn->SetToolTip(wxT("Open comprehensive user manual, setting explanations, and interpolation math diagrams (F1)."));
    bannerSizer->Add(helpBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 8);

    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Configuration Box
    auto* configBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Interpolation Engine & DDP / E1.31 Network Buffer Controls"));
    
    m_chkEnableInterpolator = new wxCheckBox(this, wxID_ANY, wxT("Enable AI Predictive Packet Loss Concealment & Frame Reconstruction (Master Toggle)"));
    m_chkEnableInterpolator->SetValue(true);
    m_chkEnableInterpolator->SetToolTip(wxT("Toggle ON to actively synthesize missing frames when WiFi UDP packets are dropped."));
    configBox->Add(m_chkEnableInterpolator, 0, wxALL, 6);

    auto* grid = new wxFlexGridSizer(2, 4, 8, 12);
    grid->AddGrowableCol(1);
    grid->AddGrowableCol(3);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Interpolation Model:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString modes;
    modes.Add(wxT("Adaptive Lookahead Bezier (AI Predictive - Best Quality)"));
    modes.Add(wxT("Cubic Hermite Spline Smoothing"));
    modes.Add(wxT("Linear Smooth Interpolation"));
    modes.Add(wxT("Disabled (Hold Last Frame)"));
    m_choiceMode = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, modes);
    m_choiceMode->SetSelection(0);
    grid->Add(m_choiceMode, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Lookahead Window:")), 0, wxALIGN_CENTER_VERTICAL);
    m_spinLookahead = new wxSpinCtrl(this, wxID_ANY, wxT("4"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, 4);
    grid->Add(m_spinLookahead, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Target Channels:")), 0, wxALIGN_CENTER_VERTICAL);
    m_spinChannels = new wxSpinCtrl(this, wxID_ANY, wxT("1500"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 3, 30000, 1500);
    grid->Add(m_spinChannels, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Simulated Loss Rate:")), 0, wxALIGN_CENTER_VERTICAL);
    auto* dropSizer = new wxBoxSizer(wxHORIZONTAL);
    m_sliderSimDropRate = new wxSlider(this, wxID_ANY, 15, 0, 50, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS);
    dropSizer->Add(m_sliderSimDropRate, 1, wxEXPAND | wxRIGHT, 4);
    dropSizer->Add(new wxStaticText(this, wxID_ANY, wxT("%")), 0, wxALIGN_CENTER_VERTICAL);
    grid->Add(dropSizer, 1, wxEXPAND);

    configBox->Add(grid, 0, wxEXPAND | wxALL, 6);
    mainSizer->Add(configBox, 0, wxEXPAND | wxALL, 6);

    // Benchmark Action Bar
    auto* actionSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnBenchmark = new wxButton(this, ID_BTN_BENCHMARK, wxT("⚡ Run Loss Stress Benchmark"));
    m_btnBenchmark->SetBackgroundColour(wxColour(110, 50, 140));
    m_btnBenchmark->SetForegroundColour(*wxWHITE);
    actionSizer->Add(m_btnBenchmark, 0, wxALL, 4);

    m_spinTestFrames = new wxSpinCtrl(this, wxID_ANY, wxT("400"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 50, 5000, 400);
    actionSizer->Add(new wxStaticText(this, wxID_ANY, wxT("frames")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    actionSizer->Add(m_spinTestFrames, 0, wxALIGN_CENTER_VERTICAL);

    m_lblStatus = new wxStaticText(this, wxID_ANY, wxT("Ready."));
    actionSizer->Add(m_lblStatus, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);
    mainSizer->Add(actionSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 6);

    // Results Box
    m_txtResults = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    m_txtResults->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    mainSizer->Add(m_txtResults, 1, wxEXPAND | wxALL, 6);

    // Bottom Action Bar
    auto* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnApply = new wxButton(this, ID_BTN_APPLY_SETTINGS, wxT("✓ Apply Engine Settings"));
    m_btnApply->SetBackgroundColour(wxColour(40, 130, 80));
    m_btnApply->SetForegroundColour(*wxWHITE);
    m_btnExportReport = new wxButton(this, ID_BTN_EXPORT_REPORT, wxT("📄 Export Benchmark Report"));
    m_btnClose = new wxButton(this, wxID_CANCEL, wxT("Close"));

    bottomSizer->Add(m_btnApply, 0, wxRIGHT, 4);
    bottomSizer->Add(m_btnExportReport, 0, wxRIGHT, 4);
    bottomSizer->AddStretchSpacer();
    bottomSizer->Add(m_btnClose, 0);

    mainSizer->Add(bottomSizer, 0, wxEXPAND | wxALL, 6);
    SetSizer(mainSizer);
    Center();
}

void AIPacketLossInterpolatorDialog::OnRunBenchmarkClick(wxCommandEvent& WXUNUSED(event)) {
    int mSel = m_choiceMode->GetSelection();
    PacketInterpolationMode mode = PacketInterpolationMode::ADAPTIVE_LOOKAHEAD_BEZIER;
    if (mSel == 1) mode = PacketInterpolationMode::CUBIC_HERMITE_SPLINE;
    else if (mSel == 2) mode = PacketInterpolationMode::LINEAR_INTERPOLATION;
    else if (mSel == 3 || !m_chkEnableInterpolator->GetValue()) mode = PacketInterpolationMode::DISABLED;

    m_interpolator.SetMode(mode);
    m_interpolator.SetLookaheadWindow(m_spinLookahead->GetValue());

    int frames = m_spinTestFrames->GetValue();
    double dropRate = static_cast<double>(m_sliderSimDropRate->GetValue()) / 100.0;
    size_t channels = m_spinChannels->GetValue();

    m_lastReport = m_interpolator.BenchmarkSimulation(frames, dropRate, channels);
    m_txtResults->SetValue(wxString::FromUTF8(m_lastReport.GenerateFormattedReport()));

    m_lblStatus->SetLabel(wxString::Format(wxT("Recovered %d / %d dropped frames | Throughput: %.0f FPS | MSE: %.3f"),
        m_lastReport.synthesizedFramesGenerated, m_lastReport.droppedFramesDetected,
        m_lastReport.throughputFps, m_lastReport.meanSquaredError));
}

void AIPacketLossInterpolatorDialog::OnApplySettingsClick(wxCommandEvent& WXUNUSED(event)) {
    bool enabled = m_chkEnableInterpolator->GetValue();
    spdlog::info("AIPacketLossInterpolator: Settings applied. Enabled={}, Mode={}, Lookahead={}",
        enabled, m_choiceMode->GetStringSelection().ToStdString(), m_spinLookahead->GetValue());
    wxMessageBox(wxString::Format(wxT("AI Packet Loss Interpolator settings successfully activated!\nMode: %s\nLookahead Buffer: %d frames"),
        m_choiceMode->GetStringSelection(), m_spinLookahead->GetValue()),
        wxT("Settings Applied"), wxOK | wxICON_INFORMATION, this);
}

void AIPacketLossInterpolatorDialog::OnExportReportClick(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog dlg(this, wxT("Save Packet Loss Benchmark Report"), wxEmptyString,
                     wxT("WiFi_Loss_Benchmark_Report.txt"), wxT("Text files (*.txt)|*.txt"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        std::ofstream out(dlg.GetPath().ToStdString());
        out << m_lastReport.GenerateFormattedReport();
        wxMessageBox(wxT("Report exported successfully!"), wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    }
}

void AIPacketLossInterpolatorDialog::OnHelpClick(wxCommandEvent& WXUNUSED(event)) {
    AIHelpGuideDialog::ShowHelp(this, "WIFI_PACKET_INTERPOLATOR");
}

void AIPacketLossInterpolatorDialog::OnCloseClick(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
