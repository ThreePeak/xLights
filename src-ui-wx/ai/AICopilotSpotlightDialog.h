// Copyright (c) xLights Project
#pragma once

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <vector>
#include <string>

namespace xLights::AI {

struct SpotlightItem {
    int id;
    std::string icon;
    std::string title;
    std::string category;
    std::string description;
    std::vector<std::string> keywords;
};

class AICopilotSpotlightDialog : public wxDialog {
public:
    AICopilotSpotlightDialog(wxWindow* parent, wxWindowID id = wxID_ANY,
                             const wxString& title = wxT("AI Copilot Spotlight"),
                             const wxPoint& pos = wxDefaultPosition,
                             const wxSize& size = wxSize(700, 460),
                             long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AICopilotSpotlightDialog() = default;

private:
    void InitUI();
    void PopulateCommands();
    void FilterCommands(const wxString& query);
    void ExecuteSelectedCommand();

    void OnSearchTextChanged(wxCommandEvent& event);
    void OnSearchKeyDown(wxKeyEvent& event);
    void OnItemActivated(wxListEvent& event);
    void OnCloseClick(wxCommandEvent& event);

    wxTextCtrl* m_searchCtrl = nullptr;
    wxListCtrl* m_resultsList = nullptr;
    wxStaticText* m_hintLabel = nullptr;

    std::vector<SpotlightItem> m_allCommands;
    std::vector<SpotlightItem> m_filteredCommands;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
