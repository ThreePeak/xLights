/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIPowerInjectionDialog.h"
#include <spdlog/spdlog.h>

namespace xLights::AI {

enum {
    ID_POWER_CALCULATE_BTN = 12001
};

wxBEGIN_EVENT_TABLE(AIPowerInjectionDialog, wxDialog)
    EVT_BUTTON(ID_POWER_CALCULATE_BTN, AIPowerInjectionDialog::OnCalculateButtonClick)
    EVT_BUTTON(wxID_CANCEL, AIPowerInjectionDialog::OnCloseButtonClick)
wxEND_EVENT_TABLE()

AIPowerInjectionDialog::AIPowerInjectionDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
}

void AIPowerInjectionDialog::InitUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Input Parameters
    wxStaticBoxSizer* inputBox = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("String & Wire Parameters"));
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 4, 5, 10);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Supply Voltage:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString voltages;
    voltages.Add(wxT("12V DC"));
    voltages.Add(wxT("5V DC"));
    voltages.Add(wxT("24V DC"));
    m_voltageChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, voltages);
    m_voltageChoice->SetSelection(0);
    grid->Add(m_voltageChoice, 0, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Wire Gauge (AWG):")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString awgList;
    awgList.Add(wxT("18 AWG"));
    awgList.Add(wxT("16 AWG"));
    awgList.Add(wxT("20 AWG"));
    awgList.Add(wxT("22 AWG"));
    m_awgChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, awgList);
    m_awgChoice->SetSelection(0);
    grid->Add(m_awgChoice, 0, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Total Pixel Count:")), 0, wxALIGN_CENTER_VERTICAL);
    m_pixelCountCtrl = new wxTextCtrl(this, wxID_ANY, wxT("300"));
    grid->Add(m_pixelCountCtrl, 0, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Feed Wire Length (ft):")), 0, wxALIGN_CENTER_VERTICAL);
    m_distanceCtrl = new wxTextCtrl(this, wxID_ANY, wxT("25.0"));
    grid->Add(m_distanceCtrl, 0, wxEXPAND);

    inputBox->GetSizer()->Add(grid, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(inputBox, 0, wxEXPAND | wxALL, 10);

    // Results Overview
    wxStaticBoxSizer* resultBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Voltage Drop Analysis"));
    m_voltageDropLabel = new wxStaticText(this, wxID_ANY, wxT("End-of-Line Voltage: 10.45 V (12.9% drop - ACCEPTABLE)"));
    m_currentLabel = new wxStaticText(this, wxID_ANY, wxT("Total Draw: 16.50 Amps (198.0 Watts @ 100% White)"));

    resultBox->GetSizer()->Add(m_voltageDropLabel, 0, wxALL, 5);
    resultBox->GetSizer()->Add(m_currentLabel, 0, wxALL, 5);
    mainSizer->Add(resultBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Recommended Injection Points Table
    wxStaticBoxSizer* tableBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Recommended Power Injection Points"));
    m_injectionPointsList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxBORDER_SUNKEN);
    m_injectionPointsList->InsertColumn(0, wxT("Point #"), wxLIST_FORMAT_LEFT, 70);
    m_injectionPointsList->InsertColumn(1, wxT("Pixel Index"), wxLIST_FORMAT_LEFT, 100);
    m_injectionPointsList->InsertColumn(2, wxT("Recommended Feed"), wxLIST_FORMAT_LEFT, 200);
    m_injectionPointsList->InsertColumn(3, wxT("Calculated Voltage"), wxLIST_FORMAT_LEFT, 150);

    tableBox->GetSizer()->Add(m_injectionPointsList, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(tableBox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Action Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_calculateBtn = new wxButton(this, ID_POWER_CALCULATE_BTN, wxT("Recalculate Power Drops"));
    m_closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));

    btnSizer->Add(m_calculateBtn, 0, wxALL, 5);
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

    PowerInjectionConfig config;
    config.pixelCount = static_cast<int>(pixels);
    config.supplyVoltage = (m_voltageChoice->GetSelection() == 1) ? 5.0f : 12.0f;
    config.wireGaugeAWG = 18;

    PowerInjectionResult result = PowerInjectionAnalyzer::CalculatePowerInjection(config);

    m_voltageDropLabel->SetLabel(wxString::Format(wxT("End-of-Line Voltage: %.2f V (%.1f%% drop)"), result.endOfLineVoltage, result.percentageVoltageDrop));
    m_currentLabel->SetLabel(wxString::Format(wxT("Total Draw: %.2f Amps (%.1f Watts @ 100%% White)"), result.totalCurrentAmps, result.totalPowerWatts));

    long row = m_injectionPointsList->InsertItem(0, wxT("Point #1"));
    m_injectionPointsList->SetItem(row, 1, wxT("Pixel #1 (Start)"));
    m_injectionPointsList->SetItem(row, 2, wxT("Primary Controller Power Terminal"));
    m_injectionPointsList->SetItem(row, 3, wxString::Format(wxT("%.2f V"), config.supplyVoltage));

    if (result.requiresInjection) {
        long row2 = m_injectionPointsList->InsertItem(1, wxT("Point #2"));
        m_injectionPointsList->SetItem(row2, 1, wxString::Format(wxT("Pixel #%d (Middle)"), config.pixelCount / 2));
        m_injectionPointsList->SetItem(row2, 2, wxT("T-Tap Power Injector Feed"));
        m_injectionPointsList->SetItem(row2, 3, wxString::Format(wxT("%.2f V"), config.supplyVoltage * 0.95f));
    }
}

void AIPowerInjectionDialog::OnCalculateButtonClick(wxCommandEvent& WXUNUSED(event)) {
    RunCalculation();
}

void AIPowerInjectionDialog::OnCloseButtonClick(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
