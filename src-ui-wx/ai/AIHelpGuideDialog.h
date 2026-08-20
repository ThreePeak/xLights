#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
#include <wx/html/htmlwin.h>
#include <wx/panel.h>
#include <string>
#include <vector>
#include "AI/AIHelpContentRegistry.h"

namespace xLights::AI {

class AIHelpGuideDialog : public wxDialog {
public:
    AIHelpGuideDialog(wxWindow* parent, const std::string& initialTopicId = "",
                      wxWindowID id = wxID_ANY,
                      const wxString& title = wxT("xLights AI Copilot - Comprehensive User Manual & Feature Guide"),
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxSize(960, 680),
                      long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);

    static void ShowHelp(wxWindow* parent, const std::string& topicId = "");

    void SelectTopic(const std::string& topicId);

private:
    void InitUI();
    void PopulateTopics();
    void OnTopicSelected(wxCommandEvent& event);
    void OnSearchUpdated(wxCommandEvent& event);
    void OnExportHtmlClicked(wxCommandEvent& event);
    void OnCloseClicked(wxCommandEvent& event);

    wxTextCtrl* m_searchCtrl{nullptr};
    wxListBox* m_topicListBox{nullptr};
    wxHtmlWindow* m_htmlViewer{nullptr};
    wxStaticText* m_topicTitleLabel{nullptr};

    std::vector<AIHelpTopic> m_displayedTopics;
    std::string m_currentTopicId;

    wxDECLARE_EVENT_TABLE();
};

} // namespace xLights::AI
