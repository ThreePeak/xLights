/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIHelpGuideDialog.h"
#include <wx/filedlg.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>
#include <spdlog/spdlog.h>

namespace xLights::AI {

enum {
    ID_SEARCH_CTRL = 15001,
    ID_TOPIC_LIST,
    ID_EXPORT_HTML_BTN
};

wxBEGIN_EVENT_TABLE(AIHelpGuideDialog, wxDialog)
    EVT_TEXT(ID_SEARCH_CTRL, AIHelpGuideDialog::OnSearchUpdated)
    EVT_LISTBOX(ID_TOPIC_LIST, AIHelpGuideDialog::OnTopicSelected)
    EVT_BUTTON(ID_EXPORT_HTML_BTN, AIHelpGuideDialog::OnExportHtmlClicked)
    EVT_BUTTON(wxID_CANCEL, AIHelpGuideDialog::OnCloseClicked)
wxEND_EVENT_TABLE()

AIHelpGuideDialog::AIHelpGuideDialog(wxWindow* parent, const std::string& initialTopicId,
                                     wxWindowID id, const wxString& title,
                                     const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style),
      m_currentTopicId(initialTopicId) {
    InitUI();
    PopulateTopics();
    if (!initialTopicId.empty()) {
        SelectTopic(initialTopicId);
    } else if (!m_displayedTopics.empty()) {
        SelectTopic(m_displayedTopics[0].topicId);
    }
    Centre();
}

void AIHelpGuideDialog::ShowHelp(wxWindow* parent, const std::string& topicId) {
    AIHelpGuideDialog dlg(parent, topicId);
    dlg.ShowModal();
}

void AIHelpGuideDialog::InitUI() {
    SetMinSize(wxSize(880, 600));
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Modern Header Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(24, 32, 44));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* titleSizer = new wxBoxSizer(wxVERTICAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("xLights AI Copilot — Interactive Manual & Feature Guide"));
    titleTxt->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleTxt->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(titleFont);

    auto* subTitle = new wxStaticText(banner, wxID_ANY,
        wxT("Comprehensive setting-by-setting breakdowns, direct hardware/sequence effect analyses, and visual architecture diagrams."));
    subTitle->SetForegroundColour(wxColour(170, 200, 230));

    titleSizer->Add(titleTxt, 0, wxALL, 6);
    titleSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 6);
    bannerSizer->Add(titleSizer, 1, wxEXPAND | wxALL, 4);

    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Split Content: Left Sidebar (Topic List + Search) and Right HTML Viewer
    auto* contentSizer = new wxBoxSizer(wxHORIZONTAL);

    // Left Panel
    auto* leftPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(280, -1));
    auto* leftSizer = new wxBoxSizer(wxVERTICAL);

    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("Search Features & Settings:")), 0, wxALL, 4);
    m_searchCtrl = new wxTextCtrl(leftPanel, ID_SEARCH_CTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize);
    m_searchCtrl->SetToolTip(wxT("Type to instantly filter topics and settings."));
    leftSizer->Add(m_searchCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);

    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("AI Subsystems & Tools:")), 0, wxLEFT | wxRIGHT | wxTOP, 4);
    m_topicListBox = new wxListBox(leftPanel, ID_TOPIC_LIST, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE | wxBORDER_SUNKEN);
    leftSizer->Add(m_topicListBox, 1, wxEXPAND | wxALL, 4);

    leftPanel->SetSizer(leftSizer);
    contentSizer->Add(leftPanel, 0, wxEXPAND | wxALL, 6);

    // Right Panel (HTML Documentation Viewer)
    auto* rightPanel = new wxPanel(this, wxID_ANY);
    auto* rightSizer = new wxBoxSizer(wxVERTICAL);

    m_htmlViewer = new wxHtmlWindow(rightPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO | wxBORDER_SUNKEN);
    rightSizer->Add(m_htmlViewer, 1, wxEXPAND | wxALL, 4);

    rightPanel->SetSizer(rightSizer);
    contentSizer->Add(rightPanel, 1, wxEXPAND | wxALL, 6);

    mainSizer->Add(contentSizer, 1, wxEXPAND);

    // Bottom Action Bar
    auto* botSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* exportBtn = new wxButton(this, ID_EXPORT_HTML_BTN, wxT("📄 Export Page as HTML..."));
    exportBtn->SetToolTip(wxT("Save this feature guide and diagram page as a standalone HTML document."));

    auto* closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));

    botSizer->Add(exportBtn, 0, wxALL, 6);
    botSizer->AddStretchSpacer();
    botSizer->Add(closeBtn, 0, wxALL, 6);

    mainSizer->Add(botSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);

    SetSizer(mainSizer);
    Layout();
}

void AIHelpGuideDialog::PopulateTopics() {
    m_displayedTopics = AIHelpContentRegistry::GetAllTopics();
    m_topicListBox->Clear();
    for (const auto& topic : m_displayedTopics) {
        m_topicListBox->Append(wxString::FromUTF8(topic.title.c_str()));
    }
}

void AIHelpGuideDialog::SelectTopic(const std::string& topicId) {
    m_currentTopicId = topicId;
    for (size_t i = 0; i < m_displayedTopics.size(); ++i) {
        if (m_displayedTopics[i].topicId == topicId) {
            m_topicListBox->SetSelection(static_cast<int>(i));
            std::string html = m_displayedTopics[i].GenerateHtml();
            m_htmlViewer->SetPage(wxString::FromUTF8(html.c_str()));
            return;
        }
    }

    const auto* topic = AIHelpContentRegistry::GetTopic(topicId);
    if (topic) {
        std::string html = topic->GenerateHtml();
        m_htmlViewer->SetPage(wxString::FromUTF8(html.c_str()));
    }
}

void AIHelpGuideDialog::OnTopicSelected(wxCommandEvent& event) {
    int sel = m_topicListBox->GetSelection();
    if (sel >= 0 && sel < static_cast<int>(m_displayedTopics.size())) {
        m_currentTopicId = m_displayedTopics[sel].topicId;
        std::string html = m_displayedTopics[sel].GenerateHtml();
        m_htmlViewer->SetPage(wxString::FromUTF8(html.c_str()));
    }
}

void AIHelpGuideDialog::OnSearchUpdated(wxCommandEvent& event) {
    wxString q = m_searchCtrl->GetValue();
    if (q.empty()) {
        PopulateTopics();
    } else {
        m_displayedTopics = AIHelpContentRegistry::Search(std::string(q.ToUTF8()));
        m_topicListBox->Clear();
        for (const auto& topic : m_displayedTopics) {
            m_topicListBox->Append(wxString::FromUTF8(topic.title.c_str()));
        }
    }

    if (!m_displayedTopics.empty()) {
        m_topicListBox->SetSelection(0);
        std::string html = m_displayedTopics[0].GenerateHtml();
        m_htmlViewer->SetPage(wxString::FromUTF8(html.c_str()));
    } else {
        m_htmlViewer->SetPage(wxT("<html><body style='background-color:#1a1e24;color:#d8dee9;font-family:sans-serif;padding:20px;'><h3>No matching AI topics or settings found.</h3></body></html>"));
    }
}

void AIHelpGuideDialog::OnExportHtmlClicked(wxCommandEvent& event) {
    const auto* topic = AIHelpContentRegistry::GetTopic(m_currentTopicId);
    if (!topic) return;

    wxFileDialog saveDlg(this, wxT("Export Documentation HTML"), wxEmptyString,
                         wxString::FromUTF8(topic->topicId.c_str()) + wxT("_Manual.html"),
                         wxT("HTML files (*.html)|*.html|All files (*.*)|*.*"),
                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_CANCEL) return;

    wxFileOutputStream output(saveDlg.GetPath());
    if (!output.IsOk()) {
        wxMessageBox(wxT("Failed to open file for writing."), wxT("Export Error"), wxICON_ERROR | wxOK, this);
        return;
    }
    wxTextOutputStream textOut(output);
    textOut << wxString::FromUTF8(topic->GenerateHtml().c_str());
    wxMessageBox(wxT("User manual and diagrams exported successfully!"), wxT("Export Complete"), wxICON_INFORMATION | wxOK, this);
}

void AIHelpGuideDialog::OnCloseClicked(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
