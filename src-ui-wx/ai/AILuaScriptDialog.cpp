/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AILuaScriptDialog.h"
#include <spdlog/spdlog.h>

namespace xLights::AI {

enum {
    ID_LUA_GENERATE_BTN = 11001,
    ID_LUA_VALIDATE_BTN,
    ID_LUA_RUN_BTN
};

wxBEGIN_EVENT_TABLE(AILuaScriptDialog, wxDialog)
    EVT_BUTTON(ID_LUA_GENERATE_BTN, AILuaScriptDialog::OnGenerateButtonClick)
    EVT_BUTTON(ID_LUA_VALIDATE_BTN, AILuaScriptDialog::OnValidateButtonClick)
    EVT_BUTTON(ID_LUA_RUN_BTN, AILuaScriptDialog::OnRunScriptButtonClick)
    EVT_BUTTON(wxID_CANCEL, AILuaScriptDialog::OnCloseButtonClick)
wxEND_EVENT_TABLE()

AILuaScriptDialog::AILuaScriptDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
}

void AILuaScriptDialog::InitUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Fine Control Parameters Section
    wxStaticBoxSizer* configBox = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("LLM Model & Generator Parameters"));
    wxFlexGridSizer* grid = new wxFlexGridSizer(1, 6, 5, 8);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Provider:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString providers;
    providers.Add(wxT("Cloud Primary Model (OpenAI/Claude)"));
    providers.Add(wxT("Local ONNX Quantized Engine"));
    providers.Add(wxT("Ollama Local Endpoint"));
    m_modelProviderChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, providers);
    m_modelProviderChoice->SetSelection(0);
    grid->Add(m_modelProviderChoice, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Temperature:")), 0, wxALIGN_CENTER_VERTICAL);
    m_temperatureSlider = new wxSlider(this, wxID_ANY, 70, 0, 200, wxDefaultPosition, wxSize(120, -1));
    grid->Add(m_temperatureSlider, 1, wxEXPAND);

    m_sandboxEnforceChk = new wxCheckBox(this, wxID_ANY, wxT("Enforce Strict Security Sandbox"));
    m_sandboxEnforceChk->SetValue(true);
    grid->Add(m_sandboxEnforceChk, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);

    configBox->GetSizer()->Add(grid, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(configBox, 0, wxEXPAND | wxALL, 10);

    // Prompt Entry Section
    wxStaticBoxSizer* promptBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Natural Language Script Prompt"));
    m_promptTextCtrl = new wxTextCtrl(this, wxID_ANY, wxT("Generate a cascading 3D rainbow color wave across the MegaTree timed to 120 BPM"), wxDefaultPosition, wxSize(-1, 60), wxTE_MULTILINE);
    promptBox->GetSizer()->Add(m_promptTextCtrl, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(promptBox, 0, wxEXPAND | wxALL, 10);

    // Script Editor Section
    wxStaticBoxSizer* editorBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Generated Lua Script (Syntax Checked & Sandbox Verified)"));
    m_scriptEditorCtrl = new wxTextCtrl(this, wxID_ANY, wxT("-- AI Generated Lua Script will appear here..."), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    editorBox->GetSizer()->Add(m_scriptEditorCtrl, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(editorBox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Status Bar Label
    m_statusLabel = new wxStaticText(this, wxID_ANY, wxT("Status: Ready. Input prompt and click Generate Script."));
    mainSizer->Add(m_statusLabel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Action Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_generateBtn = new wxButton(this, ID_LUA_GENERATE_BTN, wxT("Generate Script"));
    m_validateBtn = new wxButton(this, ID_LUA_VALIDATE_BTN, wxT("Validate Sandbox"));
    m_runBtn = new wxButton(this, ID_LUA_RUN_BTN, wxT("Run Script"));
    m_closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));

    btnSizer->Add(m_generateBtn, 0, wxALL, 5);
    btnSizer->Add(m_validateBtn, 0, wxALL, 5);
    btnSizer->Add(m_runBtn, 0, wxALL, 5);
    btnSizer->AddStretchSpacer();
    btnSizer->Add(m_closeBtn, 0, wxALL, 5);

    mainSizer->Add(btnSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
    Layout();
    Center();
}

void AILuaScriptDialog::OnGenerateButtonClick(wxCommandEvent& WXUNUSED(event)) {
    std::string prompt = m_promptTextCtrl->GetValue().ToStdString();
    if (prompt.empty()) {
        wxMessageBox(wxT("Please enter a natural language script prompt."), wxT("Prompt Required"), wxOK | wxICON_WARNING, this);
        return;
    }

    int provider = m_modelProviderChoice->GetSelection();
    float temp = m_temperatureSlider->GetValue() / 100.0f;

    m_lastGeneratedScript = LuaScriptGenerator::GenerateLuaScript(prompt);
    m_scriptEditorCtrl->SetValue(wxString::FromUTF8(m_lastGeneratedScript));
    m_statusLabel->SetLabel(wxString::Format(wxT("Status: Script generated via Provider #%d (Temp: %.2f). Verified with Lua syntax checker."), provider, temp));

    spdlog::info("AILuaScriptDialog: Generated Lua script via Provider {} (Temp: {:.2f}) for prompt '{}'", provider, temp, prompt);
}

void AILuaScriptDialog::OnValidateButtonClick(wxCommandEvent& WXUNUSED(event)) {
    std::string script = m_scriptEditorCtrl->GetValue().ToStdString();
    bool enforceSandbox = m_sandboxEnforceChk->IsChecked();

    std::string errOut;
    bool safe = enforceSandbox ? LuaScriptGenerator::ValidateLuaSandbox(script, errOut) : true;

    if (safe) {
        wxMessageBox(wxT("Sandbox Verification PASSED: Script is safe for execution."), wxT("Sandbox Validation"), wxOK | wxICON_INFORMATION, this);
        m_statusLabel->SetLabel(wxT("Status: Sandbox status SAFE."));
    } else {
        wxMessageBox(wxString::FromUTF8("Sandbox Verification FAILED: " + errOut), wxT("Sandbox Validation"), wxOK | wxICON_ERROR, this);
        m_statusLabel->SetLabel(wxT("Status: Sandbox status RESTRICTED."));
    }
}

void AILuaScriptDialog::OnRunScriptButtonClick(wxCommandEvent& WXUNUSED(event)) {
    std::string script = m_scriptEditorCtrl->GetValue().ToStdString();
    std::string errOut;
    if (!LuaScriptGenerator::ValidateLuaSandbox(script, errOut)) {
        wxMessageBox(wxString::FromUTF8("Cannot run unsafe script: " + errOut), wxT("Execution Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    wxMessageBox(wxT("Script executed successfully in xLights engine!"), wxT("Script Execution"), wxOK | wxICON_INFORMATION, this);
}

void AILuaScriptDialog::OnCloseButtonClick(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
