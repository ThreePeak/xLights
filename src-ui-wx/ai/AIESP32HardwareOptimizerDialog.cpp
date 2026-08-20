/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIESP32HardwareOptimizerDialog.h"
#include "src-ui-wx/ai/AIHelpGuideDialog.h"
#include <spdlog/spdlog.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <fstream>

namespace xLights::AI {

enum {
    ID_BTN_OPTIMIZE = 21001,
    ID_BTN_EXPORT_ESPS,
    ID_BTN_EXPORT_WLED,
    ID_BTN_EXPORT_PIO,
    ID_BTN_EXPORT_REPORT,
    ID_BTN_HELP
};

wxBEGIN_EVENT_TABLE(AIESP32HardwareOptimizerDialog, wxDialog)
    EVT_BUTTON(ID_BTN_OPTIMIZE, AIESP32HardwareOptimizerDialog::OnOptimizeClick)
    EVT_BUTTON(ID_BTN_EXPORT_ESPS, AIESP32HardwareOptimizerDialog::OnExportESPixelStickClick)
    EVT_BUTTON(ID_BTN_EXPORT_WLED, AIESP32HardwareOptimizerDialog::OnExportWledClick)
    EVT_BUTTON(ID_BTN_EXPORT_PIO, AIESP32HardwareOptimizerDialog::OnExportPlatformIoClick)
    EVT_BUTTON(ID_BTN_EXPORT_REPORT, AIESP32HardwareOptimizerDialog::OnExportReportClick)
    EVT_BUTTON(ID_BTN_HELP, AIESP32HardwareOptimizerDialog::OnHelpClick)
    EVT_BUTTON(wxID_CANCEL, AIESP32HardwareOptimizerDialog::OnCloseClick)
wxEND_EVENT_TABLE()

AIESP32HardwareOptimizerDialog::AIESP32HardwareOptimizerDialog(
    wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
    wxCommandEvent dummy;
    OnOptimizeClick(dummy);
}

void AIESP32HardwareOptimizerDialog::InitUI() {
    SetMinSize(wxSize(880, 640));
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Header Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(18, 42, 58));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* textSizer = new wxBoxSizer(wxVERTICAL);

    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI ESP32 Hardware Capability & DMA Pinout Optimizer"));
    titleTxt->SetForegroundColour(*wxWHITE);
    auto font = titleTxt->GetFont();
    font.SetPointSize(font.GetPointSize() + 2);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);

    auto* subTitle = new wxStaticText(banner, wxID_ANY,
        wxT("Auto-calculate safe RMT/I2S DMA pin allocations, avoid strapping traps, and tune damping resistors & voltage drop."));
    subTitle->SetForegroundColour(wxColour(160, 220, 240));

    textSizer->Add(titleTxt, 0, wxALL, 6);
    textSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 6);
    bannerSizer->Add(textSizer, 1, wxEXPAND);

    auto* helpBtn = new wxButton(banner, ID_BTN_HELP, wxT("❓ Help & Guide"));
    helpBtn->SetToolTip(wxT("Open comprehensive user manual, setting explanations, and hardware diagrams (F1)."));
    bannerSizer->Add(helpBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 8);

    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Parameters Box
    auto* paramBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Target Controller & Pixel Line Specifications"));
    auto* grid = new wxFlexGridSizer(3, 4, 8, 12);
    grid->AddGrowableCol(1);
    grid->AddGrowableCol(3);

    // Row 1
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Target Board:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString boards;
    boards.Add(wxT("QuinLED Dig-Quad (4-Port)"));
    boards.Add(wxT("QuinLED Dig-Octa (8-Port)"));
    boards.Add(wxT("QuinLED Dig-Uno (2-Port)"));
    boards.Add(wxT("ESP32-S3 DevKit (Dual-Core + AI Vector)"));
    boards.Add(wxT("ESP32 DevKit V1 (Standard Dual-Core)"));
    boards.Add(wxT("ESP32-C3 / C6 (RISC-V)"));
    boards.Add(wxT("Custom DIY ESP32 Board"));
    m_choiceBoard = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, boards);
    m_choiceBoard->SetSelection(0);
    grid->Add(m_choiceBoard, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Pixel Protocol:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString protocols;
    protocols.Add(wxT("WS2811 (800 kHz High Speed)"));
    protocols.Add(wxT("WS2812B (800 kHz RGB)"));
    protocols.Add(wxT("SK6812 (RGBW 4-Channel)"));
    protocols.Add(wxT("GS8208 (12V Dual Data Backup)"));
    protocols.Add(wxT("APA102 / DotStar (SPI Data + Clock)"));
    m_choiceProtocol = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, protocols);
    m_choiceProtocol->SetSelection(0);
    grid->Add(m_choiceProtocol, 1, wxEXPAND);

    // Row 2
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Number of Ports:")), 0, wxALIGN_CENTER_VERTICAL);
    m_spinPorts = new wxSpinCtrl(this, wxID_ANY, wxT("4"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 16, 4);
    grid->Add(m_spinPorts, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Pixels Per Port:")), 0, wxALIGN_CENTER_VERTICAL);
    m_spinPixelsPerPort = new wxSpinCtrl(this, wxID_ANY, wxT("600"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10, 3000, 600);
    grid->Add(m_spinPixelsPerPort, 1, wxEXPAND);

    // Row 3
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Supply Voltage:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString volts;
    volts.Add(wxT("12V DC (Standard Pixel String)"));
    volts.Add(wxT("5V DC (High Current Strip)"));
    volts.Add(wxT("24V DC (Commercial)"));
    m_choiceVoltage = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, volts);
    m_choiceVoltage->SetSelection(0);
    grid->Add(m_choiceVoltage, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Feed Wire Distance:")), 0, wxALIGN_CENTER_VERTICAL);
    auto* wireSizer = new wxBoxSizer(wxHORIZONTAL);
    m_spinWireLength = new wxSpinCtrlDouble(this, wxID_ANY, wxT("5.0"), wxDefaultPosition, wxSize(70, -1), wxSP_ARROW_KEYS, 0.5, 50.0, 5.0, 0.5);
    wireSizer->Add(m_spinWireLength, 0, wxRIGHT, 4);
    wireSizer->Add(new wxStaticText(this, wxID_ANY, wxT("meters @")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    wxArrayString gauges;
    gauges.Add(wxT("14 AWG"));
    gauges.Add(wxT("16 AWG"));
    gauges.Add(wxT("18 AWG"));
    gauges.Add(wxT("20 AWG"));
    gauges.Add(wxT("22 AWG"));
    m_choiceWireGauge = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(80, -1), gauges);
    m_choiceWireGauge->SetSelection(2); // 18 AWG
    wireSizer->Add(m_choiceWireGauge, 0);
    grid->Add(wireSizer, 1, wxEXPAND);

    paramBox->Add(grid, 0, wxEXPAND | wxALL, 6);

    m_chkAvoidStrapping = new wxCheckBox(this, wxID_ANY, wxT("Strictly avoid ESP32 strapping pins (GPIO 0, 2, 12, 15) and boot-flash bus"));
    m_chkAvoidStrapping->SetValue(true);
    paramBox->Add(m_chkAvoidStrapping, 0, wxLEFT | wxRIGHT | wxBOTTOM, 6);

    mainSizer->Add(paramBox, 0, wxEXPAND | wxALL, 6);

    // Run Button & Metrics Bar
    auto* actionSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnOptimize = new wxButton(this, ID_BTN_OPTIMIZE, wxT("⚡ Run AI Pinout & Signal Optimization"));
    m_btnOptimize->SetBackgroundColour(wxColour(30, 110, 150));
    m_btnOptimize->SetForegroundColour(*wxWHITE);
    actionSizer->Add(m_btnOptimize, 0, wxALL, 4);

    m_lblMetrics = new wxStaticText(this, wxID_ANY, wxT("Ready to compute optimal pin assignments."));
    actionSizer->Add(m_lblMetrics, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
    mainSizer->Add(actionSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 6);

    // Results List Table
    m_listResults = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    m_listResults->InsertColumn(0, wxT("Port"), wxLIST_FORMAT_LEFT, 60);
    m_listResults->InsertColumn(1, wxT("Assigned GPIO"), wxLIST_FORMAT_LEFT, 110);
    m_listResults->InsertColumn(2, wxT("Driver Peripheral"), wxLIST_FORMAT_LEFT, 130);
    m_listResults->InsertColumn(3, wxT("Nodes"), wxLIST_FORMAT_LEFT, 70);
    m_listResults->InsertColumn(4, wxT("Resistor"), wxLIST_FORMAT_LEFT, 90);
    m_listResults->InsertColumn(5, wxT("Voltage Sag"), wxLIST_FORMAT_LEFT, 100);
    m_listResults->InsertColumn(6, wxT("Hardware Recommendation & Status"), wxLIST_FORMAT_LEFT, 280);
    mainSizer->Add(m_listResults, 1, wxEXPAND | wxALL, 6);

    // Bottom Action Bar
    auto* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnExportEsps = new wxButton(this, ID_BTN_EXPORT_ESPS, wxT("💾 Export ESPixelStick JSON"));
    m_btnExportWled = new wxButton(this, ID_BTN_EXPORT_WLED, wxT("💾 Export WLED cfg.json"));
    m_btnExportPio  = new wxButton(this, ID_BTN_EXPORT_PIO,  wxT("💾 Export PlatformIO .ini"));
    m_btnExportReport = new wxButton(this, ID_BTN_EXPORT_REPORT, wxT("📄 Export Report"));
    m_btnClose = new wxButton(this, wxID_CANCEL, wxT("Close"));

    bottomSizer->Add(m_btnExportEsps, 0, wxRIGHT, 4);
    bottomSizer->Add(m_btnExportWled, 0, wxRIGHT, 4);
    bottomSizer->Add(m_btnExportPio,  0, wxRIGHT, 4);
    bottomSizer->Add(m_btnExportReport, 0, wxRIGHT, 4);
    bottomSizer->AddStretchSpacer();
    bottomSizer->Add(m_btnClose, 0);

    mainSizer->Add(bottomSizer, 0, wxEXPAND | wxALL, 6);
    SetSizer(mainSizer);
    Center();
}

void AIESP32HardwareOptimizerDialog::OnOptimizeClick(wxCommandEvent& WXUNUSED(event)) {
    ESP32HardwareOptimizationRequest req;
    
    int bSel = m_choiceBoard->GetSelection();
    if (bSel == 0) req.boardType = ESP32BoardType::QUINLED_DIG_QUAD;
    else if (bSel == 1) req.boardType = ESP32BoardType::QUINLED_DIG_OCTA;
    else if (bSel == 2) req.boardType = ESP32BoardType::QUINLED_DIG_UNO;
    else if (bSel == 3) req.boardType = ESP32BoardType::ESP32_S3_DEVKIT;
    else if (bSel == 4) req.boardType = ESP32BoardType::ESP32_DEVKIT_V1;
    else if (bSel == 5) req.boardType = ESP32BoardType::ESP32_C3_MINI;
    else req.boardType = ESP32BoardType::CUSTOM_DIY_BOARD;

    int pSel = m_choiceProtocol->GetSelection();
    if (pSel == 0) req.protocol = PixelProtocolType::WS2811_800KHZ;
    else if (pSel == 1) req.protocol = PixelProtocolType::WS2812B_800KHZ;
    else if (pSel == 2) req.protocol = PixelProtocolType::SK6812_RGBW;
    else if (pSel == 3) req.protocol = PixelProtocolType::GS8208_12V;
    else req.protocol = PixelProtocolType::APA102_SPI;

    req.numPixelPorts = m_spinPorts->GetValue();
    req.pixelsPerPort = m_spinPixelsPerPort->GetValue();

    int vSel = m_choiceVoltage->GetSelection();
    req.supplyVoltageVolts = (vSel == 1) ? 5.0 : ((vSel == 2) ? 24.0 : 12.0);

    req.wireLengthMeters = m_spinWireLength->GetValue();
    
    int gSel = m_choiceWireGauge->GetSelection();
    int awgValues[] = {14, 16, 18, 20, 22};
    req.wireGaugeAwg = (gSel >= 0 && gSel < 5) ? awgValues[gSel] : 18;

    req.avoidStrappingPinsStrict = m_chkAvoidStrapping->GetValue();

    m_lastResult = AIESP32HardwareOptimizer::OptimizeHardwareLayout(req);

    // Update UI List
    m_listResults->DeleteAllItems();
    for (size_t i = 0; i < m_lastResult.portAssignments.size(); ++i) {
        const auto& p = m_lastResult.portAssignments[i];
        long idx = m_listResults->InsertItem(i, wxString::Format(wxT("Port %d"), p.portIndex));
        m_listResults->SetItem(idx, 1, wxString::Format(wxT("GPIO %d (%s)"), p.assignedGpio, wxString::FromUTF8(p.portLabel)));
        m_listResults->SetItem(idx, 2, p.driverType == PeripheralDriverType::RMT_CHANNEL ? wxT("RMT Channel") : wxT("I2S DMA"));
        m_listResults->SetItem(idx, 3, wxString::Format(wxT("%d px"), p.pixelCount));
        m_listResults->SetItem(idx, 4, wxString::Format(wxT("%.0f Ω"), p.inlineResistorOhms));
        m_listResults->SetItem(idx, 5, wxString::Format(wxT("-%.2fV (%.1fV)"), p.estimatedVoltageDropVolts, p.voltageAtPixelStrip));
        m_listResults->SetItem(idx, 6, wxString::FromUTF8(p.recommendation));
    }

    m_lblMetrics->SetLabel(wxString::Format(
        wxT("Total: %d pixels | Est. Max Rate: %.1f FPS | Peak Current: %.2f A"),
        m_lastResult.totalPixelsSupported, m_lastResult.estimatedMaxFps, m_lastResult.maxTotalCurrentAmps));
}

void AIESP32HardwareOptimizerDialog::OnExportESPixelStickClick(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog dlg(this, wxT("Save ESPixelStick v4 Config JSON"), wxEmptyString,
                     wxT("esps_config.json"), wxT("JSON files (*.json)|*.json"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        std::ofstream out(dlg.GetPath().ToStdString());
        out << m_lastResult.GenerateESPixelStickJsonConfig();
        wxMessageBox(wxT("ESPixelStick v4 hardware configuration JSON exported successfully!"), wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    }
}

void AIESP32HardwareOptimizerDialog::OnExportWledClick(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog dlg(this, wxT("Save WLED cfg.json"), wxEmptyString,
                     wxT("wled_cfg.json"), wxT("JSON files (*.json)|*.json"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        std::ofstream out(dlg.GetPath().ToStdString());
        out << m_lastResult.GenerateWledJsonConfig();
        wxMessageBox(wxT("WLED cfg.json hardware configuration exported successfully!"), wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    }
}

void AIESP32HardwareOptimizerDialog::OnExportPlatformIoClick(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog dlg(this, wxT("Save PlatformIO configuration"), wxEmptyString,
                     wxT("platformio.ini"), wxT("INI files (*.ini)|*.ini"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        std::ofstream out(dlg.GetPath().ToStdString());
        out << m_lastResult.GeneratePlatformIoIni();
        wxMessageBox(wxT("PlatformIO configuration exported successfully!"), wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    }
}

void AIESP32HardwareOptimizerDialog::OnExportReportClick(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog dlg(this, wxT("Save Hardware Audit Report"), wxEmptyString,
                     wxT("ESP32_Hardware_Report.txt"), wxT("Text files (*.txt)|*.txt"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        std::ofstream out(dlg.GetPath().ToStdString());
        out << m_lastResult.GenerateFormattedReport();
        wxMessageBox(wxT("Hardware audit report exported successfully!"), wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    }
}

void AIESP32HardwareOptimizerDialog::OnHelpClick(wxCommandEvent& WXUNUSED(event)) {
    AIHelpGuideDialog::ShowHelp(this, "ESP32_PINOUT_OPTIMIZER");
}

void AIESP32HardwareOptimizerDialog::OnCloseClick(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
