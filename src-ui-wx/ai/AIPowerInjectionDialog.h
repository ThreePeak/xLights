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
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include <wx/gauge.h>
#include <wx/filedlg.h>
#include <wx/sizer.h>

#include "AI/PowerInjectionAnalyzer.h"

namespace xLights::AI {

class AIPowerInjectionDialog : public wxDialog {
public:
    AIPowerInjectionDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("AI Power Injection & Voltage Drop Inspector"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(750, 550), long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AIPowerInjectionDialog() = default;

private:
    void InitUI();
    void RunCalculation();

    // Event Handlers
    void OnCalculateButtonClick(wxCommandEvent& event);
    void OnExportCsvButtonClick(wxCommandEvent& event);
    void OnHelpButtonClick(wxCommandEvent& event);
    void OnCloseButtonClick(wxCommandEvent& event);

    // Controls
    wxChoice* m_voltageChoice = nullptr;
    wxChoice* m_awgChoice = nullptr;
    wxTextCtrl* m_pixelCountCtrl = nullptr;
    wxTextCtrl* m_distanceCtrl = nullptr;

    wxStaticText* m_voltageDropLabel = nullptr;
    wxStaticText* m_currentLabel = nullptr;
    wxListCtrl* m_injectionPointsList = nullptr;
    wxGauge* m_calcProgress = nullptr;

    wxButton* m_calculateBtn = nullptr;
    wxButton* m_exportCsvBtn = nullptr;
    wxButton* m_closeBtn = nullptr;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
