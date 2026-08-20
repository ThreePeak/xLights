/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIFreeRTOSAffinityDialog.h"
#include "src-ui-wx/ai/AIHelpGuideDialog.h"
#include <spdlog/spdlog.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <fstream>

namespace xLights::AI {

enum {
    ID_BTN_OPTIMIZE = 26001,
    ID_BTN_EXPORT_CPP,
    ID_BTN_EXPORT_PIO,
    ID_BTN_EXPORT_REPORT,
    ID_BTN_HELP
};

wxBEGIN_EVENT_TABLE(AIFreeRTOSAffinityDialog, wxDialog)
    EVT_BUTTON(ID_BTN_OPTIMIZE, AIFreeRTOSAffinityDialog::OnOptimizeClick)
    EVT_BUTTON(ID_BTN_EXPORT_CPP, AIFreeRTOSAffinityDialog::OnExportCppClick)
    EVT_BUTTON(ID_BTN_EXPORT_PIO, AIFreeRTOSAffinityDialog::OnExportPioClick)
    EVT_BUTTON(ID_BTN_EXPORT_REPORT, AIFreeRTOSAffinityDialog::OnExportReportClick)
    EVT_BUTTON(ID_BTN_HELP, AIFreeRTOSAffinityDialog::OnHelpClick)
    EVT_BUTTON(wxID_CANCEL, AIFreeRTOSAffinityDialog::OnCloseClick)
wxEND_EVENT_TABLE()

AIFreeRTOSAffinityDialog::AIFreeRTOSAffinityDialog(
    wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
    wxCommandEvent dummy;
    OnOptimizeClick(dummy);
}

void AIFreeRTOSAffinityDialog::InitUI() {
    SetMinSize(wxSize(880, 640));
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Modern Banner Header
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(40, 36, 20));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* textSizer = new wxBoxSizer(wxVERTICAL);

    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI ESP32 FreeRTOS Multi-Core Affinity & Task Scheduler"));
    titleTxt->SetForegroundColour(*wxWHITE);
    auto font = titleTxt->GetFont();
    font.SetPointSize(font.GetPointSize() + 2);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);

    auto* subTitle = new wxStaticText(banner, wxID_ANY,
        wxT("Isolate high-speed RMT/I2S DMA pixel output on Core 1 while pinning WiFi/TCP/LwIP stacks to Core 0, preventing watchdog resets and frame jitter."));
    subTitle->SetForegroundColour(wxColour(240, 230, 180));

    textSizer->Add(titleTxt, 0, wxALL, 6);
    textSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 6);
    bannerSizer->Add(textSizer, 1, wxEXPAND);

    auto* helpBtn = new wxButton(banner, ID_BTN_HELP, wxT("❓ Help & Guide"));
    helpBtn->SetToolTip(wxT("Open comprehensive user manual, setting explanations, and FreeRTOS task scheduling diagrams (F1)."));
    bannerSizer->Add(helpBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 8);

    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Hardware & Task Features Box
    auto* configBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Target Firmware Tasks & Chip Core Architecture"));
    auto* grid = new wxFlexGridSizer(2, 4, 8, 12);
    grid->AddGrowableCol(1);
    grid->AddGrowableCol(3);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Target Architecture:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString chips;
    chips.Add(wxT("Dual-Core Xtensa LX6/LX7 (ESP32 Classic / ESP32-S3)"));
    chips.Add(wxT("Single-Core RISC-V (ESP32-C3 / ESP32-C6)"));
    m_choiceChipType = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, chips);
    m_choiceChipType->SetSelection(0);
    grid->Add(m_choiceChipType, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Number of Pixel Ports:")), 0, wxALIGN_CENTER_VERTICAL);
    m_spinPorts = new wxSpinCtrl(this, wxID_ANY, wxT("4"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 16, 4);
    grid->Add(m_spinPorts, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Pixels Per Port:")), 0, wxALIGN_CENTER_VERTICAL);
    m_spinPixelsPerPort = new wxSpinCtrl(this, wxID_ANY, wxT("600"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10, 3000, 600);
    grid->Add(m_spinPixelsPerPort, 1, wxEXPAND);

    configBox->Add(grid, 0, wxEXPAND | wxALL, 6);

    auto* chkGrid = new wxFlexGridSizer(2, 2, 6, 12);
    m_chkDdpListener = new wxCheckBox(this, wxID_ANY, wxT("Enable UDP DDP / E1.31 Socket Task (Core 0)"));
    m_chkDdpListener->SetValue(true);
    chkGrid->Add(m_chkDdpListener, 0);

    m_chkSdFppPlayback = new wxCheckBox(this, wxID_ANY, wxT("Enable SD Card FAT32 FPP Remote Task (Core 1)"));
    m_chkSdFppPlayback->SetValue(true);
    chkGrid->Add(m_chkSdFppPlayback, 0);

    m_chkWebOta = new wxCheckBox(this, wxID_ANY, wxT("Enable AsyncWebServer & WebSockets Task (Core 0)"));
    m_chkWebOta->SetValue(true);
    chkGrid->Add(m_chkWebOta, 0);

    m_chkTelemetry = new wxCheckBox(this, wxID_ANY, wxT("Enable Syslog & Sensor Telemetry Task (Core 0)"));
    m_chkTelemetry->SetValue(true);
    chkGrid->Add(m_chkTelemetry, 0);

    configBox->Add(chkGrid, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    mainSizer->Add(configBox, 0, wxEXPAND | wxALL, 6);

    // Action Bar
    auto* actionSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnOptimize = new wxButton(this, ID_BTN_OPTIMIZE, wxT("⚡ Compute FreeRTOS Task Allocations"));
    m_btnOptimize->SetBackgroundColour(wxColour(160, 120, 20));
    m_btnOptimize->SetForegroundColour(*wxWHITE);
    actionSizer->Add(m_btnOptimize, 0, wxALL, 4);

    m_lblStatus = new wxStaticText(this, wxID_ANY, wxT("Ready."));
    actionSizer->Add(m_lblStatus, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
    mainSizer->Add(actionSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 6);

    // Results Task List
    m_listTasks = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 140), wxLC_REPORT | wxLC_SINGLE_SEL);
    m_listTasks->InsertColumn(0, wxT("Task Name"), wxLIST_FORMAT_LEFT, 170);
    m_listTasks->InsertColumn(1, wxT("CPU Core Pinning"), wxLIST_FORMAT_LEFT, 200);
    m_listTasks->InsertColumn(2, wxT("Priority"), wxLIST_FORMAT_LEFT, 70);
    m_listTasks->InsertColumn(3, wxT("Stack RAM"), wxLIST_FORMAT_LEFT, 90);
    m_listTasks->InsertColumn(4, wxT("Role & Risk Rationale"), wxLIST_FORMAT_LEFT, 290);
    mainSizer->Add(m_listTasks, 0, wxEXPAND | wxALL, 6);

    // C++ Skeleton Preview Box
    m_txtPreview = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    m_txtPreview->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    mainSizer->Add(m_txtPreview, 1, wxEXPAND | wxALL, 6);

    // Bottom Action Bar
    auto* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnExportCpp = new wxButton(this, ID_BTN_EXPORT_CPP, wxT("💾 Export FreeRTOS C++ Code"));
    m_btnExportPio = new wxButton(this, ID_BTN_EXPORT_PIO, wxT("💾 Export platformio.ini Flags"));
    m_btnExportReport = new wxButton(this, ID_BTN_EXPORT_REPORT, wxT("📄 Export Audit Report"));
    m_btnClose = new wxButton(this, wxID_CANCEL, wxT("Close"));

    bottomSizer->Add(m_btnExportCpp, 0, wxRIGHT, 4);
    bottomSizer->Add(m_btnExportPio, 0, wxRIGHT, 4);
    bottomSizer->Add(m_btnExportReport, 0, wxRIGHT, 4);
    bottomSizer->AddStretchSpacer();
    bottomSizer->Add(m_btnClose, 0);

    mainSizer->Add(bottomSizer, 0, wxEXPAND | wxALL, 6);
    SetSizer(mainSizer);
    Center();
}

void AIFreeRTOSAffinityDialog::OnOptimizeClick(wxCommandEvent& WXUNUSED(event)) {
    FreeRTOSOptimizationRequest req;
    req.isDualCore = (m_choiceChipType->GetSelection() == 0);
    req.numPixelPorts = m_spinPorts->GetValue();
    req.pixelsPerPort = m_spinPixelsPerPort->GetValue();
    req.enableDdpUdpListener = m_chkDdpListener->GetValue();
    req.enableSdCardFppPlayback = m_chkSdFppPlayback->GetValue();
    req.enableWebOtaServer = m_chkWebOta->GetValue();
    req.enableSyslogDiagnostics = m_chkTelemetry->GetValue();

    m_lastResult = AIFreeRTOSAffinityOptimizer::OptimizeAffinity(req);

    m_listTasks->DeleteAllItems();
    for (size_t i = 0; i < m_lastResult.tasks.size(); ++i) {
        const auto& t = m_lastResult.tasks[i];
        long idx = m_listTasks->InsertItem(i, wxString::FromUTF8(t.taskName));
        m_listTasks->SetItem(idx, 1, wxString::FromUTF8(AIFreeRTOSAffinityOptimizer::GetCorePinningName(t.coreAffinity)));
        m_listTasks->SetItem(idx, 2, wxString::Format(wxT("Pri %d"), t.priority));
        m_listTasks->SetItem(idx, 3, wxString::Format(wxT("%zu B"), t.stackSizeBytes));
        m_listTasks->SetItem(idx, 4, wxString::FromUTF8(t.role));
    }

    std::string preview = m_lastResult.freertosCppSkeleton + "\n" + m_lastResult.platformioIniFlags;
    m_txtPreview->SetValue(wxString::FromUTF8(preview));

    m_lblStatus->SetLabel(wxString::Format(wxT("Configured %zu FreeRTOS tasks (%zu KB stack RAM) | WDT Safety Margin: %.1f %%"),
        m_lastResult.tasks.size(), m_lastResult.totalTaskStackRamBytes / 1024, m_lastResult.estimatedWdtMarginPercent));
}

void AIFreeRTOSAffinityDialog::OnExportCppClick(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog dlg(this, wxT("Save FreeRTOS Tasks C++ Source"), wxEmptyString,
                     wxT("FreeRTOSTasks.cpp"), wxT("C++ files (*.cpp)|*.cpp"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        std::ofstream out(dlg.GetPath().ToStdString());
        out << m_lastResult.freertosCppSkeleton;
        wxMessageBox(wxT("FreeRTOS task skeleton C++ code exported successfully!"), wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    }
}

void AIFreeRTOSAffinityDialog::OnExportPioClick(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog dlg(this, wxT("Save PlatformIO Configuration Flags"), wxEmptyString,
                     wxT("platformio_freertos.ini"), wxT("INI files (*.ini)|*.ini"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        std::ofstream out(dlg.GetPath().ToStdString());
        out << m_lastResult.platformioIniFlags;
        wxMessageBox(wxT("PlatformIO configuration flags exported successfully!"), wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    }
}

void AIFreeRTOSAffinityDialog::OnExportReportClick(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog dlg(this, wxT("Save FreeRTOS Audit Report"), wxEmptyString,
                     wxT("FreeRTOS_Affinity_Report.txt"), wxT("Text files (*.txt)|*.txt"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        std::ofstream out(dlg.GetPath().ToStdString());
        out << m_lastResult.GenerateFormattedReport();
        wxMessageBox(wxT("Report exported successfully!"), wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    }
}

void AIFreeRTOSAffinityDialog::OnHelpClick(wxCommandEvent& WXUNUSED(event)) {
    AIHelpGuideDialog::ShowHelp(this, "FREERTOS_AFFINITY_OPTIMIZER");
}

void AIFreeRTOSAffinityDialog::OnCloseClick(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
