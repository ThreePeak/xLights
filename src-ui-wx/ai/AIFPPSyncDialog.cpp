#include "src-ui-wx/ai/AIFPPSyncDialog.h"
#include "src-ui-wx/ai/AIHelpGuideDialog.h"
#include <wx/app.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/filepicker.h>
#include <spdlog/spdlog.h>
#include <curl/curl.h>
#include <thread>

namespace xLights::AI {

enum {
    ID_FPP_ANALYZE_BTN = 13001,
    ID_FPP_EXPORT_JSON_BTN = 13002,
    ID_FPP_HELP_BTN = 13003,
    ID_FPP_PUSH_REST_BTN = 13004
};

wxBEGIN_EVENT_TABLE(AIFPPSyncDialog, wxDialog)
    EVT_BUTTON(ID_FPP_ANALYZE_BTN, AIFPPSyncDialog::OnAnalyzeClick)
    EVT_BUTTON(ID_FPP_EXPORT_JSON_BTN, AIFPPSyncDialog::OnExportJsonClick)
    EVT_BUTTON(ID_FPP_PUSH_REST_BTN, AIFPPSyncDialog::OnPushFppClick)
    EVT_BUTTON(ID_FPP_HELP_BTN, AIFPPSyncDialog::OnHelpClick)
    EVT_BUTTON(wxID_CANCEL, AIFPPSyncDialog::OnCloseClick)
wxEND_EVENT_TABLE()

AIFPPSyncDialog::AIFPPSyncDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
}

void AIFPPSyncDialog::InitUI() {
    SetMinSize(wxSize(760, 580));
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Modern Header Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(24, 45, 56));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    
    auto* textSizer = new wxBoxSizer(wxVERTICAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Falcon Player (FPP) Multi-Sync & Controller Auto-Provisioner"));
    titleTxt->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleTxt->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(titleFont);

    auto* subTitle = new wxStaticText(banner, wxID_ANY,
        wxT("Auto-discover FPP instances, synchronize universe maps, and export controller configuration manifests."));
    subTitle->SetForegroundColour(wxColour(170, 210, 230));

    textSizer->Add(titleTxt, 0, wxALL, 8);
    textSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    bannerSizer->Add(textSizer, 1, wxEXPAND);

    auto* helpBtn = new wxButton(banner, ID_FPP_HELP_BTN, wxT("❓ Help & Guide"));
    helpBtn->SetToolTip(wxT("Open comprehensive user manual, setting explanations, and workflow diagrams (F1)."));
    bannerSizer->Add(helpBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // FPP Host Configuration
    wxStaticBoxSizer* configBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("FPP Host & Show Configuration"));
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 2, 6, 12);
    grid->AddGrowableCol(1, 1);
    
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("FPP Host IP / Hostname:")), 0, wxALIGN_CENTER_VERTICAL);
    m_fppHostCtrl = new wxTextCtrl(this, wxID_ANY, wxT("192.168.1.100"));
    m_fppHostCtrl->SetToolTip(wxT("Enter the IP address or hostname of the primary Falcon Player master or remote node."));
    grid->Add(m_fppHostCtrl, 1, wxEXPAND);
    
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("show.xml File Path:")), 0, wxALIGN_CENTER_VERTICAL);
    m_showXmlPicker = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, wxT("Select show.xml"),
                                           wxT("XML files (*.xml)|*.xml|All files (*.*)|*.*"),
                                           wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE | wxFLP_USE_TEXTCTRL);
    m_showXmlPicker->SetToolTip(wxT("Path to the local xLights show.xml file containing network & controller definitions."));
    grid->Add(m_showXmlPicker, 1, wxEXPAND);

    configBox->Add(grid, 1, wxEXPAND | wxALL, 6);
    mainSizer->Add(configBox, 0, wxEXPAND | wxALL, 10);

    // Analysis Results
    wxStaticBoxSizer* resultsBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Controller Auto-Provisioning Table"));
    m_resultsListCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxBORDER_SUNKEN);
    m_resultsListCtrl->InsertColumn(0, wxT("Point#"), wxLIST_FORMAT_LEFT, 60);
    m_resultsListCtrl->InsertColumn(1, wxT("Controller IP"), wxLIST_FORMAT_LEFT, 140);
    m_resultsListCtrl->InsertColumn(2, wxT("Universe"), wxLIST_FORMAT_LEFT, 100);
    m_resultsListCtrl->InsertColumn(3, wxT("Start Channel"), wxLIST_FORMAT_LEFT, 130);
    m_resultsListCtrl->InsertColumn(4, wxT("Rationale"), wxLIST_FORMAT_LEFT, 260);
    
    resultsBox->Add(m_resultsListCtrl, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(resultsBox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    m_progressGauge = new wxGauge(this, wxID_ANY, 100);
    m_progressGauge->SetValue(0);
    m_progressGauge->Hide();
    mainSizer->Add(m_progressGauge, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    m_statusLabel = new wxStaticText(this, wxID_ANY, wxT("🟢 Ready. Specify FPP host and click Analyze Controllers."));
    mainSizer->Add(m_statusLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Button row
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_analyzeBtn = new wxButton(this, ID_FPP_ANALYZE_BTN, wxT("⚡ Analyze Controllers"));
    m_analyzeBtn->SetBackgroundColour(wxColour(20, 150, 220));
    m_analyzeBtn->SetForegroundColour(*wxWHITE);
    m_analyzeBtn->SetToolTip(wxT("Analyze local network controllers and match against FPP remote channel boundaries."));

    m_exportJsonBtn = new wxButton(this, ID_FPP_EXPORT_JSON_BTN, wxT("📄 Export FPP JSON Manifest..."));
    m_exportJsonBtn->SetToolTip(wxT("Export universe definitions in standard Falcon Player JSON format."));

    m_pushFppBtn = new wxButton(this, ID_FPP_PUSH_REST_BTN, wxT("🌐 Push to FPP (REST)"));
    m_pushFppBtn->SetToolTip(wxT("Upload generated channel manifest directly to Falcon Player over HTTP REST API."));
    m_pushFppBtn->SetBackgroundColour(wxColour(40, 160, 90));
    m_pushFppBtn->SetForegroundColour(*wxWHITE);

    m_closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));
    
    btnSizer->Add(m_analyzeBtn, 0, wxALL, 5);
    btnSizer->Add(m_exportJsonBtn, 0, wxALL, 5);
    btnSizer->Add(m_pushFppBtn, 0, wxALL, 5);
    btnSizer->AddStretchSpacer();
    btnSizer->Add(m_closeBtn, 0, wxALL, 5);
    
    mainSizer->Add(btnSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
    Layout();
    Center();
}

void AIFPPSyncDialog::OnAnalyzeClick(wxCommandEvent& event) {
    wxString xmlPath = m_showXmlPicker ? m_showXmlPicker->GetPath() : wxString();

    m_progressGauge->Show();
    m_progressGauge->Pulse();
    Layout();

    FPPControllerSyncAdvisor advisor;
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

void AIFPPSyncDialog::OnPushFppClick(wxCommandEvent& event) {
    if (m_lastResults.empty()) {
        wxMessageBox(wxT("Please analyze controllers first before pushing to FPP."), wxT("Notice"), wxICON_INFORMATION | wxOK, this);
        return;
    }

    wxString host = m_fppHostCtrl ? m_fppHostCtrl->GetValue().Trim() : wxString();
    if (host.empty()) {
        wxMessageBox(wxT("Please enter a valid FPP Host or IP address."), wxT("Missing Host"), wxICON_WARNING | wxOK, this);
        return;
    }

    FPPControllerSyncAdvisor advisor;
    std::string jsonManifest = advisor.GenerateFPPJsonManifest(m_lastResults);

    m_progressGauge->Show();
    m_progressGauge->Pulse();
    m_statusLabel->SetLabel(wxString::Format(wxT("Pushing configuration to FPP at %s..."), host));
    Layout();

    std::string hostStr = host.ToStdString();
    std::thread([this, hostStr, jsonManifest]() {
        std::string url = "http://" + hostStr + "/api/channel/output/co-other";
        CURL* curl = curl_easy_init();
        bool success = false;
        long httpCode = 0;
        std::string respStr;

        if (curl) {
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonManifest.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)jsonManifest.length());
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 4000L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000L);

            CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
                success = (httpCode >= 200 && httpCode < 300);
            } else {
                respStr = curl_easy_strerror(res);
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        wxTheApp->CallAfter([this, success, httpCode, hostStr, respStr]() {
            m_progressGauge->Hide();
            Layout();
            if (success) {
                m_statusLabel->SetLabel(wxString::Format(wxT("✓ Successfully pushed manifest to FPP at %s (HTTP %ld)"), wxString::FromUTF8(hostStr), httpCode));
                wxMessageBox(wxString::Format(wxT("FPP at %s updated successfully!\nChannel output configuration applied."), wxString::FromUTF8(hostStr)),
                             wxT("FPP Sync Complete"), wxOK | wxICON_INFORMATION, this);
            } else {
                m_statusLabel->SetLabel(wxString::Format(wxT("✗ FPP sync failed: %s"), wxString::FromUTF8(respStr.empty() ? ("HTTP " + std::to_string(httpCode)) : respStr)));
                wxMessageBox(wxString::Format(wxT("Failed to sync with FPP at %s.\n%s\nPlease verify FPP is reachable and FPPD is running."),
                             wxString::FromUTF8(hostStr), wxString::FromUTF8(respStr)),
                             wxT("FPP Sync Error"), wxOK | wxICON_ERROR, this);
            }
        });
    }).detach();
}

void AIFPPSyncDialog::OnHelpClick(wxCommandEvent& event) {
    AIHelpGuideDialog::ShowHelp(this, "FPP_MULTI_SYNC");
}

void AIFPPSyncDialog::OnCloseClick(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
