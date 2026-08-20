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
#include <wx/textctrl.h>
#include <wx/sizer.h>

#include "src-core/controllers/AIFreeRTOSAffinityOptimizer.h"

namespace xLights::AI {

class AIFreeRTOSAffinityDialog : public wxDialog {
public:
    AIFreeRTOSAffinityDialog(wxWindow* parent,
                             wxWindowID id = wxID_ANY,
                             const wxString& title = wxT("AI ESP32 FreeRTOS Multi-Core Affinity & Task Scheduler"),
                             const wxPoint& pos = wxDefaultPosition,
                             const wxSize& size = wxSize(880, 640),
                             long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);
    virtual ~AIFreeRTOSAffinityDialog() = default;

    const FreeRTOSOptimizationResult& GetLastResult() const { return m_lastResult; }

private:
    void InitUI();

    void OnOptimizeClick(wxCommandEvent& event);
    void OnExportCppClick(wxCommandEvent& event);
    void OnExportPioClick(wxCommandEvent& event);
    void OnExportReportClick(wxCommandEvent& event);
    void OnHelpClick(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);

    wxChoice* m_choiceChipType{nullptr};
    wxSpinCtrl* m_spinPorts{nullptr};
    wxSpinCtrl* m_spinPixelsPerPort{nullptr};
    wxCheckBox* m_chkDdpListener{nullptr};
    wxCheckBox* m_chkSdFppPlayback{nullptr};
    wxCheckBox* m_chkWebOta{nullptr};
    wxCheckBox* m_chkTelemetry{nullptr};

    wxListCtrl* m_listTasks{nullptr};
    wxStaticText* m_lblStatus{nullptr};
    wxTextCtrl* m_txtPreview{nullptr};

    wxButton* m_btnOptimize{nullptr};
    wxButton* m_btnExportCpp{nullptr};
    wxButton* m_btnExportPio{nullptr};
    wxButton* m_btnExportReport{nullptr};
    wxButton* m_btnClose{nullptr};

    FreeRTOSOptimizationResult m_lastResult;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
