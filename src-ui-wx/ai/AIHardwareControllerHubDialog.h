/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/dialog.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/slider.h>
#include <wx/gauge.h>
#include <wx/choice.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <thread>
#include <atomic>
#include <vector>
#include <string>

class xLightsFrame;

namespace xLights::AI {

class AIHardwareControllerHubDialog : public wxDialog {
public:
    AIHardwareControllerHubDialog(wxWindow* parent, xLightsFrame* frame = nullptr, wxWindowID id = wxID_ANY,
                                  const wxString& title = wxT("AI Controller & Hardware Engineering Hub"),
                                  const wxPoint& pos = wxDefaultPosition,
                                  const wxSize& size = wxSize(920, 680),
                                  long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AIHardwareControllerHubDialog();

private:
    void InitUI();

    // Tab 1: Discovery & Auto-Mapper
    wxPanel* CreateDiscoveryTab(wxWindow* parent);
    void OnScanNetworkClick(wxCommandEvent& evt);
    void OnAutoBindModelsClick(wxCommandEvent& evt);

    // Tab 2: ESP32 Pinout & Safety Engine
    wxPanel* CreateESP32Tab(wxWindow* parent);
    void OnOptimizePinoutClick(wxCommandEvent& evt);
    void OnExportWledJsonClick(wxCommandEvent& evt);

    // Tab 3: Power Injection & Voltage Drop
    wxPanel* CreatePowerTab(wxWindow* parent);
    void OnCalcPowerClick(wxCommandEvent& evt);

    // Tab 4: FPP Multi-Sync & DMX Conflict Advisor
    wxPanel* CreateFppDmxTab(wxWindow* parent);
    void OnCheckDmxConflictsClick(wxCommandEvent& evt);
    void OnExportFppManifestClick(wxCommandEvent& evt);

    // Tab 5: WiFi Packet Recovery & Sparse FSEQ
    wxPanel* CreateWifiSparseTab(wxWindow* parent);
    void OnOptimizeSparseFseqClick(wxCommandEvent& evt);

    // Tab 6: Live Telemetry & Thermal Throttler
    wxPanel* CreateTelemetryTab(wxWindow* parent);
    void OnPollTelemetryClick(wxCommandEvent& evt);

    void OnCloseClick(wxCommandEvent& evt);

    xLightsFrame* m_frame = nullptr;
    wxNotebook* m_notebook = nullptr;
    wxStaticText* m_statusLabel = nullptr;

    // Discovery Controls
    wxTextCtrl* m_subnetCtrl = nullptr;
    wxListCtrl* m_controllerList = nullptr;
    wxButton* m_scanNetworkBtn = nullptr;
    wxButton* m_autoBindBtn = nullptr;

    // ESP32 Controls
    wxSpinCtrl* m_espPortCountSpin = nullptr;
    wxChoice* m_espChipsetChoice = nullptr;
    wxSlider* m_wireLengthSlider = nullptr;
    wxListCtrl* m_pinoutList = nullptr;
    wxStaticText* m_dampingLabel = nullptr;

    // Power Controls
    wxChoice* m_voltageChoice = nullptr;
    wxSpinCtrl* m_pixelCountSpin = nullptr;
    wxChoice* m_awgChoice = nullptr;
    wxSlider* m_leadDistSlider = nullptr;
    wxStaticText* m_powerResultLabel = nullptr;

    // FPP / DMX Controls
    wxTextCtrl* m_fppHostCtrl = nullptr;
    wxListCtrl* m_dmxConflictList = nullptr;

    // WiFi / Sparse Controls
    wxStaticText* m_sparseSavingsLabel = nullptr;
    wxGauge* m_sparseGauge = nullptr;

    // Telemetry Controls
    wxStaticText* m_voltageSagLabel = nullptr;
    wxStaticText* m_thermalTempLabel = nullptr;

    // Concurrency
    std::atomic<bool> m_workerCancel{false};
    std::thread m_workerThread;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
