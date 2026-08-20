#pragma once

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>

#include "AI/DMXAddressAdvisor.h"
#include "AI/AIConfigurationManager.h"

namespace xLights::AI {

class AIDMXAddressDialog : public wxDialog {
public:
    AIDMXAddressDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("AI DMX E1.31 Address Conflict Advisor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(760, 540), long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AIDMXAddressDialog() = default;

private:
    void InitUI();

    void OnDetectClick(wxCommandEvent& event);
    void OnAutoRemapClick(wxCommandEvent& event);
    void OnExportClick(wxCommandEvent& event);
    void OnLoadFileClick(wxCommandEvent& event);
    void OnHelpClick(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);

    wxTextCtrl* m_universeXmlCtrl = nullptr;
    wxButton* m_loadFileBtn = nullptr;
    wxButton* m_detectBtn = nullptr;
    wxButton* m_autoRemapBtn = nullptr;
    wxButton* m_exportBtn = nullptr;
    wxButton* m_closeBtn = nullptr;
    wxListCtrl* m_conflictsListCtrl = nullptr;
    wxStaticText* m_statusLabel = nullptr;
    
    std::vector<DMXAddressConflict> m_lastConflicts;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
