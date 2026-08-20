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
#include <wx/choice.h>
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/listctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>

#include "src-core/controllers/AIESP32HardwareOptimizer.h"

namespace xLights::AI {

class AIESP32HardwareOptimizerDialog : public wxDialog {
public:
    AIESP32HardwareOptimizerDialog(wxWindow* parent,
                                   wxWindowID id = wxID_ANY,
                                   const wxString& title = wxT("AI ESP32 Hardware Capability & DMA Pinout Optimizer"),
                                   const wxPoint& pos = wxDefaultPosition,
                                   const wxSize& size = wxSize(880, 640),
                                   long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);
    virtual ~AIESP32HardwareOptimizerDialog() = default;

    const ESP32HardwareOptimizationResult& GetLastResult() const { return m_lastResult; }

private:
    void InitUI();

    void OnOptimizeClick(wxCommandEvent& event);
    void OnExportESPixelStickClick(wxCommandEvent& event);
    void OnExportWledClick(wxCommandEvent& event);
    void OnExportPlatformIoClick(wxCommandEvent& event);
    void OnExportReportClick(wxCommandEvent& event);
    void OnHelpClick(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);

    wxChoice* m_choiceBoard{nullptr};
    wxSpinCtrl* m_spinPorts{nullptr};
    wxSpinCtrl* m_spinPixelsPerPort{nullptr};
    wxChoice* m_choiceProtocol{nullptr};
    wxChoice* m_choiceVoltage{nullptr};
    wxSpinCtrlDouble* m_spinWireLength{nullptr};
    wxChoice* m_choiceWireGauge{nullptr};
    wxCheckBox* m_chkAvoidStrapping{nullptr};

    wxListCtrl* m_listResults{nullptr};
    wxStaticText* m_lblStatus{nullptr};
    wxStaticText* m_lblMetrics{nullptr};

    wxButton* m_btnOptimize{nullptr};
    wxButton* m_btnExportEsps{nullptr};
    wxButton* m_btnExportWled{nullptr};
    wxButton* m_btnExportPio{nullptr};
    wxButton* m_btnExportReport{nullptr};
    wxButton* m_btnClose{nullptr};

    ESP32HardwareOptimizationResult m_lastResult;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
