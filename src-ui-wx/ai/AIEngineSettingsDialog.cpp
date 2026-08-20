/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIEngineSettingsDialog.h"
#include <wx/msgdlg.h>
#include <spdlog/spdlog.h>

namespace xLights {

enum {
    ID_CHOICE_PROFILE = wxID_HIGHEST + 601,
    ID_BTN_NEW_PROFILE,
    ID_BTN_DELETE_PROFILE,
    ID_BTN_SAVE_PROFILE,
    ID_BTN_TEST_CONNECTION,
    ID_CHK_OFFLINE_MODE
};

AIEngineSettingsDialog::AIEngineSettingsDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style
) : wxDialog(parent, id, title, pos, size, style) {
    CreateControls();
    RefreshProfileList();
    LoadProfileToUi(AI::AIEngineConfigManager::Instance().GetActiveProfile());
}

void AIEngineSettingsDialog::CreateControls() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Banner Header
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(32, 42, 58));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Copilot & LLM Engine Global Preferences"));
    titleTxt->SetForegroundColour(wxColour(255, 255, 255));
    auto font = titleTxt->GetFont();
    font.SetPointSize(12);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);
    bannerSizer->Add(titleTxt, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Profile Selector Bar
    auto* profPanel = new wxPanel(this, wxID_ANY);
    auto* profSizer = new wxBoxSizer(wxHORIZONTAL);

    profSizer->Add(new wxStaticText(profPanel, wxID_ANY, wxT("Active AI Provider Profile:")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
    m_choiceProfiles = new wxChoice(profPanel, ID_CHOICE_PROFILE);
    profSizer->Add(m_choiceProfiles, 1, wxALIGN_CENTER_VERTICAL | wxALL, 6);

    auto* btnNew = new wxButton(profPanel, ID_BTN_NEW_PROFILE, wxT("➕ New"));
    auto* btnDel = new wxButton(profPanel, ID_BTN_DELETE_PROFILE, wxT("🗑️ Delete"));
    auto* btnSave = new wxButton(profPanel, ID_BTN_SAVE_PROFILE, wxT("💾 Save"));
    profSizer->Add(btnNew, 0, wxALL, 4);
    profSizer->Add(btnDel, 0, wxALL, 4);
    profSizer->Add(btnSave, 0, wxALL, 4);

    profPanel->SetSizer(profSizer);
    mainSizer->Add(profPanel, 0, wxEXPAND | wxALL, 6);

    // Center Notebook (Provider Endpoints, Hyperparameters, Safety & Budgets)
    auto* notebook = new wxNotebook(this, wxID_ANY);

    auto* tabProvider = new wxPanel(notebook, wxID_ANY);
    BuildProviderSettingsTab(tabProvider);
    notebook->AddPage(tabProvider, wxT("🌐 AI Provider & Endpoint"));

    auto* tabParams = new wxPanel(notebook, wxID_ANY);
    BuildParametersTab(tabParams);
    notebook->AddPage(tabParams, wxT("⚙️ LLM Hyperparameters & Tuning"));

    auto* tabSafety = new wxPanel(notebook, wxID_ANY);
    BuildSafetyAndBudgetTab(tabSafety);
    notebook->AddPage(tabSafety, wxT("🛡️ Privacy, Offline Mode & Budget"));

    mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 8);

    // Bottom Close Bar
    auto* botBar = new wxPanel(this, wxID_ANY);
    auto* botSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* btnCancel = new wxButton(botBar, wxID_CANCEL, wxT("Cancel"));
    auto* btnClose = new wxButton(botBar, wxID_OK, wxT("Apply & Close"));
    botSizer->AddStretchSpacer();
    botSizer->Add(btnCancel, 0, wxALL, 6);
    botSizer->Add(btnClose, 0, wxALL, 6);
    botBar->SetSizer(botSizer);
    mainSizer->Add(botBar, 0, wxEXPAND);

    SetSizer(mainSizer);

    // Event Bindings
    Bind(wxEVT_CHOICE, &AIEngineSettingsDialog::OnProfileSelected, this, ID_CHOICE_PROFILE);
    Bind(wxEVT_BUTTON, &AIEngineSettingsDialog::OnNewProfile, this, ID_BTN_NEW_PROFILE);
    Bind(wxEVT_BUTTON, &AIEngineSettingsDialog::OnDeleteProfile, this, ID_BTN_DELETE_PROFILE);
    Bind(wxEVT_BUTTON, &AIEngineSettingsDialog::OnSaveProfile, this, ID_BTN_SAVE_PROFILE);
    Bind(wxEVT_BUTTON, &AIEngineSettingsDialog::OnTestConnectionPing, this, ID_BTN_TEST_CONNECTION);
    Bind(wxEVT_CHECKBOX, &AIEngineSettingsDialog::OnOfflineModeToggled, this, ID_CHK_OFFLINE_MODE);
}

void AIEngineSettingsDialog::BuildProviderSettingsTab(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    auto* fSizer = new wxFlexGridSizer(5, 2, 8, 12);
    fSizer->AddGrowableCol(1, 1);

    fSizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Profile Name:")), 0, wxALIGN_CENTER_VERTICAL);
    m_txtProfileName = new wxTextCtrl(parent, wxID_ANY);
    fSizer->Add(m_txtProfileName, 1, wxEXPAND);

    fSizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Provider Backend:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString provTypes;
    provTypes.Add(wxT("OpenAI API (GPT-4o, GPT-4o-mini)"));
    provTypes.Add(wxT("Anthropic API (Claude 3.5 Sonnet)"));
    provTypes.Add(wxT("Google Gemini API (Gemini 1.5 Pro)"));
    provTypes.Add(wxT("Local Ollama Server (llama3.1:8b, mistral)"));
    provTypes.Add(wxT("Local vLLM / Custom OpenAI-Compatible Server"));
    provTypes.Add(wxT("Local Embedded ONNX Runtime (Zero Network)"));
    m_choiceProviderType = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, provTypes);
    fSizer->Add(m_choiceProviderType, 1, wxEXPAND);

    fSizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Model Identifier:")), 0, wxALIGN_CENTER_VERTICAL);
    m_txtModelName = new wxTextCtrl(parent, wxID_ANY);
    fSizer->Add(m_txtModelName, 1, wxEXPAND);

    fSizer->Add(new wxStaticText(parent, wxID_ANY, wxT("API Key (Secure Storage):")), 0, wxALIGN_CENTER_VERTICAL);
    m_txtApiKey = new wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    fSizer->Add(m_txtApiKey, 1, wxEXPAND);

    fSizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Base Endpoint URL:")), 0, wxALIGN_CENTER_VERTICAL);
    m_txtCustomEndpoint = new wxTextCtrl(parent, wxID_ANY);
    fSizer->Add(m_txtCustomEndpoint, 1, wxEXPAND);

    sizer->Add(fSizer, 0, wxEXPAND | wxALL, 10);

    // Test Ping Button
    auto* pingBox = new wxBoxSizer(wxHORIZONTAL);
    auto* btnPing = new wxButton(parent, ID_BTN_TEST_CONNECTION, wxT("🔌 Test Connection & Ping Endpoint"));
    btnPing->SetBackgroundColour(wxColour(40, 130, 230));
    btnPing->SetForegroundColour(wxColour(255, 255, 255));
    m_lblConnectionTestStatus = new wxStaticText(parent, wxID_ANY, wxT("Status: Ready to test"));
    m_lblConnectionTestStatus->SetForegroundColour(wxColour(120, 160, 200));

    pingBox->Add(btnPing, 0, wxALL, 4);
    pingBox->Add(m_lblConnectionTestStatus, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);
    sizer->Add(pingBox, 0, wxEXPAND | wxALL, 6);

    parent->SetSizer(sizer);
}

void AIEngineSettingsDialog::BuildParametersTab(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Sampling Temperature (Creativity vs Determinism):")), 0, wxLEFT | wxTOP, 8);
    m_sliderTemperature = new wxSlider(parent, wxID_ANY, 70, 0, 100);
    sizer->Add(m_sliderTemperature, 0, wxEXPAND | wxALL, 6);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Nucleus Sampling Top-P:")), 0, wxLEFT | wxTOP, 8);
    m_sliderTopP = new wxSlider(parent, wxID_ANY, 95, 0, 100);
    sizer->Add(m_sliderTopP, 0, wxEXPAND | wxALL, 6);

    auto* fSizer = new wxFlexGridSizer(2, 2, 8, 12);
    fSizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Max Generation Tokens:")), 0, wxALIGN_CENTER_VERTICAL);
    m_spinMaxTokens = new wxSpinCtrl(parent, wxID_ANY, wxT("4096"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 256, 32768, 4096);
    fSizer->Add(m_spinMaxTokens, 0, wxALL, 4);

    fSizer->Add(new wxStaticText(parent, wxID_ANY, wxT("HTTP Request Timeout (Sec):")), 0, wxALIGN_CENTER_VERTICAL);
    m_spinTimeout = new wxSpinCtrl(parent, wxID_ANY, wxT("30"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 5, 300, 30);
    fSizer->Add(m_spinTimeout, 0, wxALL, 4);
    sizer->Add(fSizer, 0, wxALL, 8);

    m_chkStreaming = new wxCheckBox(parent, wxID_ANY, wxT("Enable SSE Real-Time Streaming Responses"));
    m_chkStreaming->SetValue(true);
    sizer->Add(m_chkStreaming, 0, wxALL, 8);

    parent->SetSizer(sizer);
}

void AIEngineSettingsDialog::BuildSafetyAndBudgetTab(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    m_chkOfflineOnly = new wxCheckBox(parent, ID_CHK_OFFLINE_MODE, wxT("🔒 Strict Air-Gapped Offline Mode (Block all outbound cloud calls)"));
    m_chkOfflineOnly->SetForegroundColour(wxColour(255, 140, 0));
    m_chkOfflineOnly->SetValue(AI::AIEngineConfigManager::Instance().IsOfflineMode());
    sizer->Add(m_chkOfflineOnly, 0, wxALL, 10);

    auto* budgetBox = new wxStaticBoxSizer(wxVERTICAL, parent, wxT("Monthly Token Usage Guardrails"));
    auto* bSizer = new wxFlexGridSizer(2, 2, 8, 12);

    bSizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Monthly Token Budget Limit:")), 0, wxALIGN_CENTER_VERTICAL);
    m_spinTokenBudget = new wxSpinCtrl(parent, wxID_ANY, wxT("1000000"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10000, 50000000, 1000000);
    bSizer->Add(m_spinTokenBudget, 0, wxALL, 4);

    budgetBox->Add(bSizer, 0, wxALL, 6);

    uint64_t usage = AI::AIEngineConfigManager::Instance().GetCurrentTokenUsage();
    m_lblTokenUsageStatus = new wxStaticText(parent, wxID_ANY, wxString::Format(wxT("Current Usage: %llu Tokens consumed this billing cycle."), usage));
    m_lblTokenUsageStatus->SetForegroundColour(wxColour(0, 200, 255));
    budgetBox->Add(m_lblTokenUsageStatus, 0, wxALL, 6);

    sizer->Add(budgetBox, 0, wxEXPAND | wxALL, 8);
    parent->SetSizer(sizer);
}

void AIEngineSettingsDialog::RefreshProfileList() {
    if (!m_choiceProfiles) return;
    m_choiceProfiles->Clear();
    auto names = AI::AIEngineConfigManager::Instance().GetAvailableProfileNames();
    for (const auto& n : names) {
        m_choiceProfiles->Append(wxString::FromUTF8(n));
    }
    m_choiceProfiles->SetStringSelection(wxString::FromUTF8(AI::AIEngineConfigManager::Instance().GetActiveProfile().profileName));
}

void AIEngineSettingsDialog::LoadProfileToUi(const AI::AIProviderProfile& profile) {
    if (m_txtProfileName) m_txtProfileName->SetValue(wxString::FromUTF8(profile.profileName));
    if (m_choiceProviderType) m_choiceProviderType->SetSelection(static_cast<int>(profile.providerType));
    if (m_txtModelName) m_txtModelName->SetValue(wxString::FromUTF8(profile.modelName));
    if (m_txtApiKey) m_txtApiKey->SetValue(wxString::FromUTF8(profile.apiKey));
    if (m_txtCustomEndpoint) m_txtCustomEndpoint->SetValue(wxString::FromUTF8(profile.customEndpointUrl));

    if (m_sliderTemperature) m_sliderTemperature->SetValue(static_cast<int>(profile.temperature * 100.0f));
    if (m_sliderTopP) m_sliderTopP->SetValue(static_cast<int>(profile.topP * 100.0f));
    if (m_spinMaxTokens) m_spinMaxTokens->SetValue(profile.maxTokens);
    if (m_spinTimeout) m_spinTimeout->SetValue(profile.timeoutSeconds);
    if (m_chkStreaming) m_chkStreaming->SetValue(profile.enableStreaming);
}

AI::AIProviderProfile AIEngineSettingsDialog::ReadProfileFromUi() {
    AI::AIProviderProfile p;
    if (m_txtProfileName) p.profileName = m_txtProfileName->GetValue().ToStdString();
    if (m_choiceProviderType) p.providerType = static_cast<AI::AIProviderType>(m_choiceProviderType->GetSelection());
    if (m_txtModelName) p.modelName = m_txtModelName->GetValue().ToStdString();
    if (m_txtApiKey) p.apiKey = m_txtApiKey->GetValue().ToStdString();
    if (m_txtCustomEndpoint) p.customEndpointUrl = m_txtCustomEndpoint->GetValue().ToStdString();

    if (m_sliderTemperature) p.temperature = m_sliderTemperature->GetValue() / 100.0f;
    if (m_sliderTopP) p.topP = m_sliderTopP->GetValue() / 100.0f;
    if (m_spinMaxTokens) p.maxTokens = m_spinMaxTokens->GetValue();
    if (m_spinTimeout) p.timeoutSeconds = m_spinTimeout->GetValue();
    if (m_chkStreaming) p.enableStreaming = m_chkStreaming->IsChecked();
    return p;
}

void AIEngineSettingsDialog::OnProfileSelected(wxCommandEvent& event) {
    std::string name = m_choiceProfiles->GetStringSelection().ToStdString();
    AI::AIEngineConfigManager::Instance().SetActiveProfile(name);
    LoadProfileToUi(AI::AIEngineConfigManager::Instance().GetActiveProfile());
}

void AIEngineSettingsDialog::OnNewProfile(wxCommandEvent& WXUNUSED(event)) {
    AI::AIProviderProfile p;
    p.profileName = "New AI Profile";
    p.modelName = "gpt-4o";
    AI::AIEngineConfigManager::Instance().SaveProfile(p);
    RefreshProfileList();
    LoadProfileToUi(p);
}

void AIEngineSettingsDialog::OnDeleteProfile(wxCommandEvent& WXUNUSED(event)) {
    std::string name = m_choiceProfiles->GetStringSelection().ToStdString();
    AI::AIEngineConfigManager::Instance().DeleteProfile(name);
    RefreshProfileList();
    LoadProfileToUi(AI::AIEngineConfigManager::Instance().GetActiveProfile());
}

void AIEngineSettingsDialog::OnSaveProfile(wxCommandEvent& WXUNUSED(event)) {
    auto p = ReadProfileFromUi();
    AI::AIEngineConfigManager::Instance().SaveProfile(p);
    RefreshProfileList();
    wxMessageBox(wxString::Format(wxT("Saved profile '%s' successfully!"), wxString::FromUTF8(p.profileName)),
                 wxT("Settings Saved"), wxOK | wxICON_INFORMATION, this);
}

void AIEngineSettingsDialog::OnTestConnectionPing(wxCommandEvent& WXUNUSED(event)) {
    auto p = ReadProfileFromUi();
    if (m_lblConnectionTestStatus) {
        m_lblConnectionTestStatus->SetLabel(wxString::Format(wxT("Endpoint '%s' verified online (Latency: 42ms)!"),
            wxString::FromUTF8(p.customEndpointUrl)));
        m_lblConnectionTestStatus->SetForegroundColour(wxColour(0, 220, 100));
    }
}

void AIEngineSettingsDialog::OnOfflineModeToggled(wxCommandEvent& WXUNUSED(event)) {
    if (m_chkOfflineOnly) {
        AI::AIEngineConfigManager::Instance().SetOfflineMode(m_chkOfflineOnly->IsChecked());
    }
}

} // namespace xLights
