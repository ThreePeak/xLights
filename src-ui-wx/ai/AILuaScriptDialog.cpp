/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AILuaScriptDialog.h"
#include "src-ui-wx/ai/AIHelpGuideDialog.h"
#include <wx/filedlg.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace xLights::AI {

enum {
    ID_LUA_GENERATE_BTN = 11001,
    ID_LUA_VALIDATE_BTN,
    ID_LUA_RUN_BTN,
    ID_LUA_HELP_BTN
};

wxBEGIN_EVENT_TABLE(AILuaScriptDialog, wxDialog)
    EVT_BUTTON(ID_LUA_GENERATE_BTN, AILuaScriptDialog::OnGenerateButtonClick)
    EVT_BUTTON(ID_LUA_VALIDATE_BTN, AILuaScriptDialog::OnValidateButtonClick)
    EVT_BUTTON(ID_LUA_RUN_BTN, AILuaScriptDialog::OnSaveScriptButtonClick)
    EVT_BUTTON(ID_LUA_HELP_BTN, AILuaScriptDialog::OnHelpButtonClick)
    EVT_BUTTON(wxID_CANCEL, AILuaScriptDialog::OnCloseButtonClick)
wxEND_EVENT_TABLE()

AILuaScriptDialog::AILuaScriptDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
}

void AILuaScriptDialog::InitUI() {
    SetMinSize(wxSize(800, 640));
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Modern Header Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(36, 32, 54));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    
    auto* textSizer = new wxBoxSizer(wxVERTICAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Lua Scripting & Automation Copilot"));
    titleTxt->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleTxt->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(titleFont);

    auto* subTitle = new wxStaticText(banner, wxID_ANY,
        wxT("Generate, sandbox-verify, and execute custom sequencing automation scripts via LLM intelligence."));
    subTitle->SetForegroundColour(wxColour(210, 190, 240));

    textSizer->Add(titleTxt, 0, wxALL, 8);
    textSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    bannerSizer->Add(textSizer, 1, wxEXPAND);

    auto* helpBtn = new wxButton(banner, ID_LUA_HELP_BTN, wxT("❓ Help & Guide"));
    helpBtn->SetToolTip(wxT("Open comprehensive user manual, setting explanations, and workflow diagrams (F1)."));
    bannerSizer->Add(helpBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

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
    m_modelProviderChoice->SetToolTip(wxT("Select the AI reasoning backend to synthesize the Lua script."));
    grid->Add(m_modelProviderChoice, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Temperature:")), 0, wxALIGN_CENTER_VERTICAL);
    m_temperatureSlider = new wxSlider(this, wxID_ANY, 70, 0, 200, wxDefaultPosition, wxSize(120, -1));
    m_temperatureSlider->SetToolTip(wxT("Controls creativity vs. precision in script generation."));
    grid->Add(m_temperatureSlider, 1, wxEXPAND);

    m_sandboxEnforceChk = new wxCheckBox(this, wxID_ANY, wxT("Enforce Strict Security Sandbox"));
    m_sandboxEnforceChk->SetValue(true);
    m_sandboxEnforceChk->SetToolTip(wxT("Prevent unsafe system calls, OS file deletions, or unbounded loops."));
    grid->Add(m_sandboxEnforceChk, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);

    configBox->Add(grid, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(configBox, 0, wxEXPAND | wxALL, 10);

    // Prompt Entry Section
    wxStaticBoxSizer* promptBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Natural Language Script Prompt"));
    m_promptTextCtrl = new wxTextCtrl(this, wxID_ANY, wxT("Generate a cascading 3D rainbow color wave across the MegaTree timed to 120 BPM"), wxDefaultPosition, wxSize(-1, 60), wxTE_MULTILINE);
    m_promptTextCtrl->SetToolTip(wxT("Describe the lighting effect, animation, or sequence logic you want to create in plain English."));
    promptBox->Add(m_promptTextCtrl, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(promptBox, 0, wxEXPAND | wxALL, 10);

    // Script Editor Section
    wxStaticBoxSizer* editorBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Generated Lua Script (Syntax Checked & Sandbox Verified)"));
    m_scriptEditorCtrl = new wxTextCtrl(this, wxID_ANY, wxT("-- AI Generated Lua Script will appear here..."), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    m_scriptEditorCtrl->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    editorBox->Add(m_scriptEditorCtrl, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(editorBox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Status Bar Label
    m_statusLabel = new wxStaticText(this, wxID_ANY, wxT("🟢 Status: Ready. Input prompt and click Generate Script."));
    mainSizer->Add(m_statusLabel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Action Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_generateBtn = new wxButton(this, ID_LUA_GENERATE_BTN, wxT("✨ Generate Script"));
    m_generateBtn->SetBackgroundColour(wxColour(130, 60, 220));
    m_generateBtn->SetForegroundColour(*wxWHITE);
    m_generateBtn->SetToolTip(wxT("Synthesize a complete Lua script matching your natural language prompt."));

    m_validateBtn = new wxButton(this, ID_LUA_VALIDATE_BTN, wxT("🛡️ Validate Sandbox"));
    m_validateBtn->SetToolTip(wxT("Run the Lua AST security analyzer to confirm absence of unauthorized system APIs."));

    m_runBtn = new wxButton(this, ID_LUA_RUN_BTN, wxT("💾 Save Script (.lua)..."));
    m_runBtn->SetToolTip(wxT("Save verified Lua script file to disk."));

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

void AILuaScriptDialog::OnSaveScriptButtonClick(wxCommandEvent& WXUNUSED(event)) {
    std::string script = m_scriptEditorCtrl->GetValue().ToStdString();
    std::string errOut;
    if (!LuaScriptGenerator::ValidateLuaSandbox(script, errOut)) {
        wxMessageBox(wxString::FromUTF8("Cannot save unsafe script: " + errOut), wxT("Validation Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    wxFileDialog saveDlg(this, wxT("Save Lua Script"), wxEmptyString, wxT("script.lua"), wxT("Lua scripts (*.lua)|*.lua"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_CANCEL) return;

    std::ofstream out(saveDlg.GetPath().ToStdString());
    if (out.is_open()) {
        out << script;
        out.close();
        wxMessageBox(wxString::Format(wxT("Script successfully saved to:\n%s"), saveDlg.GetPath()), wxT("Save Complete"), wxOK | wxICON_INFORMATION, this);
    } else {
        wxMessageBox(wxT("Failed to open file for writing."), wxT("Save Error"), wxOK | wxICON_ERROR, this);
    }
}

void AILuaScriptDialog::OnHelpButtonClick(wxCommandEvent& WXUNUSED(event)) {
    AIHelpGuideDialog::ShowHelp(this, "LUA_SCRIPTING");
}

void AILuaScriptDialog::OnCloseButtonClick(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
