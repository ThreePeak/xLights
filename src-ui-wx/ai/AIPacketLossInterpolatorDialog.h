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
#include <wx/slider.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>

#include "src-core/controllers/AIPacketLossInterpolator.h"

namespace xLights::AI {

class AIPacketLossInterpolatorDialog : public wxDialog {
public:
    AIPacketLossInterpolatorDialog(wxWindow* parent,
                                   wxWindowID id = wxID_ANY,
                                   const wxString& title = wxT("AI Predictive WiFi Packet Loss Concealment & Auto-Interpolator"),
                                   const wxPoint& pos = wxDefaultPosition,
                                   const wxSize& size = wxSize(840, 600),
                                   long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);
    virtual ~AIPacketLossInterpolatorDialog() = default;

    const PacketLossTelemetryReport& GetLastReport() const { return m_lastReport; }

private:
    void InitUI();

    void OnRunBenchmarkClick(wxCommandEvent& event);
    void OnApplySettingsClick(wxCommandEvent& event);
    void OnExportReportClick(wxCommandEvent& event);
    void OnHelpClick(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);

    wxCheckBox* m_chkEnableInterpolator{nullptr};
    wxChoice* m_choiceMode{nullptr};
    wxSpinCtrl* m_spinLookahead{nullptr};
    wxSpinCtrl* m_spinChannels{nullptr};
    wxSlider* m_sliderSimDropRate{nullptr};
    wxSpinCtrl* m_spinTestFrames{nullptr};

    wxStaticText* m_lblStatus{nullptr};
    wxTextCtrl* m_txtResults{nullptr};

    wxButton* m_btnBenchmark{nullptr};
    wxButton* m_btnApply{nullptr};
    wxButton* m_btnExportReport{nullptr};
    wxButton* m_btnClose{nullptr};

    AIPacketLossInterpolator m_interpolator;
    PacketLossTelemetryReport m_lastReport;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
