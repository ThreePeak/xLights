#pragma once

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
#include "AI/AIFlightRecorder.h"

namespace xLights::AI {

class AIFlightRecorderDialog : public wxDialog {
public:
    AIFlightRecorderDialog(wxWindow* parent,
                           wxWindowID id = wxID_ANY,
                           const wxString& title = wxT("AI Diagnostic Flight Recorder & Problem Steps"),
                           const wxPoint& pos = wxDefaultPosition,
                           const wxSize& size = wxSize(880, 620),
                           long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    virtual ~AIFlightRecorderDialog();

private:
    void InitUI();
    void RefreshStepList();
    void UpdateControls();

    // Event handlers
    void OnStartRecord(wxCommandEvent& event);
    void OnStopRecord(wxCommandEvent& event);
    void OnExportPackage(wxCommandEvent& event);
    void OnViewHtmlReport(wxCommandEvent& event);
    void OnClearSession(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);
    void OnStepSelected(wxListEvent& event);
    void OnTimerTick(wxTimerEvent& event);

    // Controls
    wxStaticText* m_statusBadge = nullptr;
    wxStaticText* m_timerLabel = nullptr;
    wxButton* m_startBtn = nullptr;
    wxButton* m_stopBtn = nullptr;
    wxButton* m_exportBtn = nullptr;
    wxButton* m_viewHtmlBtn = nullptr;
    wxButton* m_clearBtn = nullptr;
    wxButton* m_closeBtn = nullptr;
    wxListCtrl* m_stepList = nullptr;
    wxTextCtrl* m_payloadViewer = nullptr;
    wxTimer m_updateTimer;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
