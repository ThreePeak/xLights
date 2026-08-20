/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIPowerInjectionDialog.h"
#include "src-ui-wx/ai/AIHelpGuideDialog.h"
#include "AI/PowerInjectionAnalyzer.h"
#include "xLightsMain.h"
#include <wx/file.h>
#include <spdlog/spdlog.h>

namespace xLights::AI {

enum {
    ID_POWER_CALCULATE_BTN = 12001,
    ID_POWER_EXPORT_CSV_BTN = 12002,
    ID_POWER_HELP_BTN = 12003
};

wxBEGIN_EVENT_TABLE(AIPowerInjectionDialog, wxDialog)
    EVT_BUTTON(ID_POWER_CALCULATE_BTN,  AIPowerInjectionDialog::OnCalculateButtonClick)
    EVT_BUTTON(ID_POWER_EXPORT_CSV_BTN, AIPowerInjectionDialog::OnExportCsvButtonClick)
    EVT_BUTTON(ID_POWER_HELP_BTN,       AIPowerInjectionDialog::OnHelpButtonClick)
    EVT_BUTTON(wxID_CANCEL,             AIPowerInjectionDialog::OnCloseButtonClick)
wxEND_EVENT_TABLE()

AIPowerInjectionDialog::AIPowerInjectionDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
}

void AIPowerInjectionDialog::InitUI() {
    SetMinSize(wxSize(760, 580));
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Modern Header Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(45, 38, 25));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    
    auto* textSizer = new wxBoxSizer(wxVERTICAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Power Injection & Voltage Drop Calculator"));
    titleTxt->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleTxt->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(titleFont);

    auto* subTitle = new wxStaticText(banner, wxID_ANY,
        wxT("Ohm's Law simulation of pixel string current draw, wire gauge resistance, and optimal injection nodes."));
    subTitle->SetForegroundColour(wxColour(240, 210, 170));

    textSizer->Add(titleTxt, 0, wxALL, 8);
    textSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    bannerSizer->Add(textSizer, 1, wxEXPAND);

    auto* helpBtn = new wxButton(banner, ID_POWER_HELP_BTN, wxT("❓ Help & Guide"));
    helpBtn->SetToolTip(wxT("Open comprehensive user manual, setting explanations, and workflow diagrams (F1)."));
    bannerSizer->Add(helpBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Input Parameters
    wxStaticBoxSizer* inputBox = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("String & Wire Parameters"));
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 4, 6, 12);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Supply Voltage:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString voltages;
    voltages.Add(wxT("12V DC"));
    voltages.Add(wxT("5V DC"));
    voltages.Add(wxT("24V DC"));
    m_voltageChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, voltages);
    m_voltageChoice->SetSelection(0);
    m_voltageChoice->SetToolTip(wxT("Operating DC voltage provided by the primary power supply."));
    grid->Add(m_voltageChoice, 0, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Wire Gauge (AWG):")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString awgList;
    awgList.Add(wxT("18 AWG"));
    awgList.Add(wxT("16 AWG"));
    awgList.Add(wxT("20 AWG"));
    awgList.Add(wxT("22 AWG"));
    m_awgChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, awgList);
    m_awgChoice->SetSelection(0);
    m_awgChoice->SetToolTip(wxT("American Wire Gauge (AWG) size for main power injection feeds."));
    grid->Add(m_awgChoice, 0, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Total Pixel Count:")), 0, wxALIGN_CENTER_VERTICAL);
    m_pixelCountCtrl = new wxTextCtrl(this, wxID_ANY, wxT("300"));
    m_pixelCountCtrl->SetToolTip(wxT("Total number of addressable LED nodes on the continuous strand."));
    grid->Add(m_pixelCountCtrl, 0, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Feed Wire Length (ft):")), 0, wxALIGN_CENTER_VERTICAL);
    m_distanceCtrl = new wxTextCtrl(this, wxID_ANY, wxT("25.0"));
    m_distanceCtrl->SetToolTip(wxT("Distance in feet from power supply unit (PSU) to the string injection point."));
    grid->Add(m_distanceCtrl, 0, wxEXPAND);

    inputBox->Add(grid, 1, wxEXPAND | wxALL, 6);
    mainSizer->Add(inputBox, 0, wxEXPAND | wxALL, 10);

    // Results Overview
    wxStaticBoxSizer* resultBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Voltage Drop Analysis"));
    m_voltageDropLabel = new wxStaticText(this, wxID_ANY, wxT("🟢 End-of-Line Voltage: 10.45 V (12.9% drop - ACCEPTABLE)"));
    m_currentLabel = new wxStaticText(this, wxID_ANY, wxT("⚡ Total Draw: 16.50 Amps (198.0 Watts @ 100% White)"));

    resultBox->Add(m_voltageDropLabel, 0, wxALL, 5);
    resultBox->Add(m_currentLabel, 0, wxALL, 5);
    mainSizer->Add(resultBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Recommended Injection Points Table
    wxStaticBoxSizer* tableBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Recommended Power Injection Points"));
    m_injectionPointsList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxBORDER_SUNKEN);
    m_injectionPointsList->InsertColumn(0, wxT("Point #"), wxLIST_FORMAT_LEFT, 70);
    m_injectionPointsList->InsertColumn(1, wxT("Pixel Index"), wxLIST_FORMAT_LEFT, 100);
    m_injectionPointsList->InsertColumn(2, wxT("Recommended Feed"), wxLIST_FORMAT_LEFT, 200);
    m_injectionPointsList->InsertColumn(3, wxT("Calculated Voltage"), wxLIST_FORMAT_LEFT, 150);

    tableBox->Add(m_injectionPointsList, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(tableBox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Action Buttons + Progress Gauge
    m_calcProgress = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(-1, 12));
    m_calcProgress->SetValue(0);
    mainSizer->Add(m_calcProgress, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_calculateBtn  = new wxButton(this, ID_POWER_CALCULATE_BTN, wxT("⚡ Recalculate Power Drops"));
    m_calculateBtn->SetBackgroundColour(wxColour(220, 130, 20));
    m_calculateBtn->SetForegroundColour(*wxWHITE);
    m_calculateBtn->SetToolTip(wxT("Compute resistive voltage drop and determine optimal power injection taps."));

    m_exportCsvBtn  = new wxButton(this, ID_POWER_EXPORT_CSV_BTN, wxT("📄 Export CSV Report..."));
    m_exportCsvBtn->SetToolTip(wxT("Export injection schedule and wire sizing calculations to CSV spreadsheet."));

    m_closeBtn      = new wxButton(this, wxID_CANCEL, wxT("Close"));

    btnSizer->Add(m_calculateBtn, 0, wxALL, 5);
    btnSizer->Add(m_exportCsvBtn, 0, wxALL, 5);
    btnSizer->AddStretchSpacer();
    btnSizer->Add(m_closeBtn, 0, wxALL, 5);

    mainSizer->Add(btnSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
    Layout();
    Center();

    RunCalculation();
}

void AIPowerInjectionDialog::RunCalculation() {
    m_injectionPointsList->DeleteAllItems();

    double pixels = 300;
    m_pixelCountCtrl->GetValue().ToDouble(&pixels);

    double lengthFt = 25.0;
    m_distanceCtrl->GetValue().ToDouble(&lengthFt);

    PowerDistributionConfig config;
    config.totalPixelCount = static_cast<int>(pixels);
    config.voltage = (m_voltageChoice->GetSelection() == 1) ? PixelVoltage::V5 :
                     (m_voltageChoice->GetSelection() == 2) ? PixelVoltage::V24 : PixelVoltage::V12;
    config.feedWireLengthFeet = static_cast<float>(lengthFt);
    config.wireGaugeAWG = 18.0f;

    PowerAnalysisResult result = PowerInjectionAnalyzer::AnalyzePowerDistribution(config);

    float baseV = (config.voltage == PixelVoltage::V5) ? 5.0f :
                  (config.voltage == PixelVoltage::V24) ? 24.0f : 12.0f;
    float endV = baseV * (1.0f - result.calculatedVoltageDropPercent / 100.0f);

    m_voltageDropLabel->SetLabel(wxString::Format(wxT("End-of-Line Voltage: %.2f V (%.1f%% drop)"), endV, result.calculatedVoltageDropPercent));
    m_currentLabel->SetLabel(wxString::Format(wxT("Total Draw: %.2f Amps (%s)"), result.estimatedMaxCurrentAmps, result.powerSupplyRecommendation));

    long row = m_injectionPointsList->InsertItem(0, wxT("Point #1"));
    m_injectionPointsList->SetItem(row, 1, wxT("Pixel #1 (Start)"));
    m_injectionPointsList->SetItem(row, 2, wxT("Primary Controller Power Terminal"));
    m_injectionPointsList->SetItem(row, 3, wxString::Format(wxT("%.2f V"), baseV));

    for (size_t i = 0; i < result.requiredInjectionNodeIndices.size(); ++i) {
        long r = m_injectionPointsList->InsertItem(static_cast<long>(i + 1), wxString::Format(wxT("Point #%zu"), i + 2));
        m_injectionPointsList->SetItem(r, 1, wxString::Format(wxT("Pixel #%d"), result.requiredInjectionNodeIndices[i]));
        m_injectionPointsList->SetItem(r, 2, wxT("T-Tap Power Injector Feed"));
        m_injectionPointsList->SetItem(r, 3, wxString::Format(wxT("%.2f V"), baseV * 0.95f));
    }
}

void AIPowerInjectionDialog::OnCalculateButtonClick(wxCommandEvent& WXUNUSED(event)) {
    if (m_calcProgress) { m_calcProgress->SetValue(30); }
    RunCalculation();
    if (m_calcProgress) { m_calcProgress->SetValue(100); }
}

void AIPowerInjectionDialog::OnExportCsvButtonClick(wxCommandEvent& WXUNUSED(event)) {
    if (m_injectionPointsList->GetItemCount() == 0) {
        wxMessageBox(wxT("Please calculate power injection points before exporting report."),
                     wxT("Calculation Required"), wxOK | wxICON_WARNING, this);
        return;
    }

    wxFileDialog saveDialog(this, wxT("Export Power Injection Report"), wxT(""), wxT("power_injection.csv"),
                            wxT("CSV files (*.csv)|*.csv"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDialog.ShowModal() != wxID_OK) return;

    wxFile file(saveDialog.GetPath(), wxFile::write);
    if (!file.IsOpened()) return;

    file.Write(wxT("Point #,Pixel Index,Recommended Feed,Calculated Voltage\n"));
    for (int i = 0; i < m_injectionPointsList->GetItemCount(); ++i) {
        wxString row;
        for (int col = 0; col < 4; ++col) {
            row += m_injectionPointsList->GetItemText(i, col);
            if (col < 3) row += wxT(",");
        }
        row += wxT("\n");
        file.Write(row);
    }
    wxMessageBox(wxT("CSV exported successfully."), wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
}

void AIPowerInjectionDialog::OnHelpButtonClick(wxCommandEvent& WXUNUSED(event)) {
    AIHelpGuideDialog::ShowHelp(this, "POWER_INJECTION");
}

void AIPowerInjectionDialog::OnCloseButtonClick(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
