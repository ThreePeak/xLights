#pragma once

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include <wx/gauge.h>
#include <wx/stattext.h>
#include <wx/sizer.h>

#include "AI/FPPControllerSyncAdvisor.h"
#include "AI/AIConfigurationManager.h"

namespace xLights::AI {

class AIFPPSyncDialog : public wxDialog {
public:
    AIFPPSyncDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("AI FPP Controller Sync Advisor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(780, 560), long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AIFPPSyncDialog() = default;

private:
    void InitUI();

    void OnAnalyzeClick(wxCommandEvent& event);
    void OnExportJsonClick(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);

    wxTextCtrl* m_fppHostCtrl = nullptr;
    wxTextCtrl* m_showXmlPathCtrl = nullptr;
    wxButton* m_browseBtn = nullptr;
    wxButton* m_analyzeBtn = nullptr;
    wxButton* m_exportJsonBtn = nullptr;
    wxButton* m_closeBtn = nullptr;
    wxListCtrl* m_resultsListCtrl = nullptr;
    wxGauge* m_progressGauge = nullptr;
    wxStaticText* m_statusLabel = nullptr;
    
    std::vector<FPPSyncSuggestion> m_lastResults;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
