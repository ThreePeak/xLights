#include "src-ui-wx/ai/AIFPPSyncDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <spdlog/spdlog.h>

namespace xLights::AI {

enum {
    ID_FPP_ANALYZE_BTN = 13001,
    ID_FPP_EXPORT_JSON_BTN = 13002
};

wxBEGIN_EVENT_TABLE(AIFPPSyncDialog, wxDialog)
    EVT_BUTTON(ID_FPP_ANALYZE_BTN, AIFPPSyncDialog::OnAnalyzeClick)
    EVT_BUTTON(ID_FPP_EXPORT_JSON_BTN, AIFPPSyncDialog::OnExportJsonClick)
    EVT_BUTTON(wxID_CANCEL, AIFPPSyncDialog::OnCloseClick)
wxEND_EVENT_TABLE()

AIFPPSyncDialog::AIFPPSyncDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
}

void AIFPPSyncDialog::InitUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // FPP Host Configuration
    wxStaticBoxSizer* configBox = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("FPP Host Configuration"));
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 3, 5, 10);
    grid->AddGrowableCol(1, 1);
    
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("FPP Host IP:")), 0, wxALIGN_CENTER_VERTICAL);
    m_fppHostCtrl = new wxTextCtrl(this, wxID_ANY, wxT("192.168.1.100"));
    grid->Add(m_fppHostCtrl, 1, wxEXPAND);
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("")), 0, wxEXPAND);
    
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("show.xml Path:")), 0, wxALIGN_CENTER_VERTICAL);
    m_showXmlPathCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""));
    grid->Add(m_showXmlPathCtrl, 1, wxEXPAND);
    m_browseBtn = new wxButton(this, wxID_ANY, wxT("Browse..."));
    grid->Add(m_browseBtn, 0, wxALIGN_CENTER_VERTICAL);

    configBox->GetSizer()->Add(grid, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(configBox, 0, wxEXPAND | wxALL, 10);

    // Analysis Results
    wxStaticBoxSizer* resultsBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Analysis Results"));
    m_resultsListCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxBORDER_SUNKEN);
    m_resultsListCtrl->InsertColumn(0, wxT("Point#"), wxLIST_FORMAT_LEFT, 60);
    m_resultsListCtrl->InsertColumn(1, wxT("Controller IP"), wxLIST_FORMAT_LEFT, 140);
    m_resultsListCtrl->InsertColumn(2, wxT("Universe"), wxLIST_FORMAT_LEFT, 100);
    m_resultsListCtrl->InsertColumn(3, wxT("Start Channel"), wxLIST_FORMAT_LEFT, 130);
    m_resultsListCtrl->InsertColumn(4, wxT("Rationale"), wxLIST_FORMAT_LEFT, 260);
    
    resultsBox->GetSizer()->Add(m_resultsListCtrl, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(resultsBox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    m_progressGauge = new wxGauge(this, wxID_ANY, 100);
    m_progressGauge->SetValue(0);
    m_progressGauge->Hide();
    mainSizer->Add(m_progressGauge, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    m_statusLabel = new wxStaticText(this, wxID_ANY, wxT("Ready"));
    mainSizer->Add(m_statusLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Button row
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_analyzeBtn = new wxButton(this, ID_FPP_ANALYZE_BTN, wxT("Analyze Controllers"));
    m_exportJsonBtn = new wxButton(this, ID_FPP_EXPORT_JSON_BTN, wxT("Export JSON"));
    m_closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));
    
    btnSizer->Add(m_analyzeBtn, 0, wxALL, 5);
    btnSizer->Add(m_exportJsonBtn, 0, wxALL, 5);
    btnSizer->AddStretchSpacer();
    btnSizer->Add(m_closeBtn, 0, wxALL, 5);
    
    mainSizer->Add(btnSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
    Layout();
    Center();
}

void AIFPPSyncDialog::OnAnalyzeClick(wxCommandEvent& event) {
    m_progressGauge->Show();
    m_progressGauge->Pulse();
    Layout();

    FPPControllerSyncAdvisor advisor;
    wxString xmlPath = m_showXmlPathCtrl->GetValue();
    m_lastResults = advisor.AnalyzeControllerLayout(xmlPath.ToStdString());

    m_resultsListCtrl->DeleteAllItems();
    long row = 0;
    for (const auto& suggestion : m_lastResults) {
        long itemIndex = m_resultsListCtrl->InsertItem(row, wxString::Format(wxT("%ld"), row + 1));
        m_resultsListCtrl->SetItem(itemIndex, 1, wxString(suggestion.controllerIP));
        m_resultsListCtrl->SetItem(itemIndex, 2, wxString(suggestion.universe));
        m_resultsListCtrl->SetItem(itemIndex, 3, wxString::Format(wxT("%d"), suggestion.suggestedStartChannel));
        m_resultsListCtrl->SetItem(itemIndex, 4, wxString(suggestion.rationale));
        row++;
    }

    m_progressGauge->SetValue(100);
    m_progressGauge->Hide();
    Layout();
    
    m_statusLabel->SetLabel(wxString::Format(wxT("Analysis complete - %ld controllers mapped"), row));
    spdlog::info("AIFPPSyncDialog: Analysis complete - {} mapped", row);
}

void AIFPPSyncDialog::OnExportJsonClick(wxCommandEvent& event) {
    if (m_lastResults.empty()) {
        wxMessageBox(wxT("No analysis results to export."), wxT("Export Error"), wxICON_WARNING | wxOK);
        return;
    }
    
    wxFileDialog saveFileDialog(this, wxT("Save JSON file"), wxT(""), wxT("manifest.json"), wxT("JSON files (*.json)|*.json"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) return;

    FPPControllerSyncAdvisor advisor;
    std::string jsonStr = advisor.GenerateFPPJsonManifest(m_lastResults);
    
    wxFile file;
    if (file.Open(saveFileDialog.GetPath(), wxFile::write)) {
        file.Write(jsonStr);
        file.Close();
        spdlog::info("AIFPPSyncDialog: Exported JSON to {}", saveFileDialog.GetPath().ToStdString());
    } else {
        spdlog::error("AIFPPSyncDialog: Failed to open file for writing JSON");
    }
}

void AIFPPSyncDialog::OnCloseClick(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
