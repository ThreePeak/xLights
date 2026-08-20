/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/gauge.h>
#include <wx/sizer.h>

#include "src-core/controllers/AIESP32TelemetryMonitor.h"

namespace xLights::AI {

class AIESP32TelemetryMonitorDialog : public wxDialog {
public:
    AIESP32TelemetryMonitorDialog(wxWindow* parent,
                                  wxWindowID id = wxID_ANY,
                                  const wxString& title = wxT("AI ESP32 Live Telemetry & Predictive Safety Throttler"),
                                  const wxPoint& pos = wxDefaultPosition,
                                  const wxSize& size = wxSize(880, 640),
                                  long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);
    virtual ~AIESP32TelemetryMonitorDialog() = default;

    const ESP32TelemetryAuditReport& GetLastReport() const { return m_lastReport; }

private:
    void InitUI();

    void OnPollTelemetryClick(wxCommandEvent& event);
    void OnSimulateStressClick(wxCommandEvent& event);
    void OnApplyThrottlingClick(wxCommandEvent& event);
    void OnExportReportClick(wxCommandEvent& event);
    void OnHelpClick(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);

    wxTextCtrl* m_txtControllerIp{nullptr};
    wxSpinCtrlDouble* m_spinNominalVolts{nullptr};
    wxSpinCtrlDouble* m_spinMaxAmps{nullptr};
    wxSpinCtrl* m_spinMaxTemp{nullptr};
    wxCheckBox* m_chkAutoThrottle{nullptr};

    wxGauge* m_gaugeVolts{nullptr};
    wxGauge* m_gaugeAmps{nullptr};
    wxGauge* m_gaugeTemp{nullptr};

    wxStaticText* m_lblVoltsVal{nullptr};
    wxStaticText* m_lblAmpsVal{nullptr};
    wxStaticText* m_lblTempVal{nullptr};
    wxStaticText* m_lblStatusSummary{nullptr};

    wxTextCtrl* m_txtLog{nullptr};

    wxButton* m_btnPoll{nullptr};
    wxButton* m_btnSimStress{nullptr};
    wxButton* m_btnApplyThrottle{nullptr};
    wxButton* m_btnExportReport{nullptr};
    wxButton* m_btnClose{nullptr};

    AIESP32TelemetryMonitor m_monitor;
    ESP32TelemetryAuditReport m_lastReport;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
