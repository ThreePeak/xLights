// Copyright (c) xLights Project
#include "src-ui-wx/ai/AIStatusBar.h"
#include "src-ui-wx/ai/AICustomPropDesignerDialog.h"
#include "src-ui-wx/ai/AISequenceValidatorDialog.h"
#include "src-ui-wx/ai/AIPowerInjectionDialog.h"
#include "src-ui-wx/ai/AIFPPSyncDialog.h"
#include "src-ui-wx/ai/AIDMXAddressDialog.h"
#include "src-ui-wx/ai/AICopilotSpotlightDialog.h"
#include <wx/confbase.h>
#include <wx/msgdlg.h>

enum {
    ID_AI_TOGGLE = 16001,
    ID_AI_PROP_DESIGNER = 16002,
    ID_AI_VALIDATOR = 16003,
    ID_AI_POWER = 16004,
    ID_AI_FPP_SYNC = 16005,
    ID_AI_DMX_ADVISOR = 16006,
    ID_AI_PIN_ALWAYS_VISIBLE = 16007,
    ID_AI_SPOTLIGHT = 16008
};

BEGIN_EVENT_TABLE(AIStatusBar, wxPanel)
    EVT_BUTTON(ID_AI_TOGGLE, AIStatusBar::OnToggleExpand)
    EVT_CHECKBOX(ID_AI_PIN_ALWAYS_VISIBLE, AIStatusBar::OnPinAlwaysVisible)
    EVT_BUTTON(ID_AI_SPOTLIGHT, AIStatusBar::OnOpenSpotlight)
    EVT_BUTTON(ID_AI_PROP_DESIGNER, AIStatusBar::OnOpenPropDesigner)
    EVT_BUTTON(ID_AI_VALIDATOR, AIStatusBar::OnOpenValidator)
    EVT_BUTTON(ID_AI_POWER, AIStatusBar::OnOpenPowerInspector)
    EVT_BUTTON(ID_AI_FPP_SYNC, AIStatusBar::OnOpenFPPSync)
    EVT_BUTTON(ID_AI_DMX_ADVISOR, AIStatusBar::OnOpenDMXAdvisor)
END_EVENT_TABLE()

AIStatusBar::AIStatusBar(wxWindow* parent, wxWindowID id)
    : wxPanel(parent, id)
{
    InitUI();
}

void AIStatusBar::InitUI()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
    
    m_toggleBtn = new wxButton(this, ID_AI_TOGGLE, "⬡ AI ▸", wxDefaultPosition, wxSize(70, -1));
    mainSizer->Add(m_toggleBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
    
    m_modelLabel = new wxStaticText(this, wxID_ANY, "Model: GPT-4o");
    m_modelLabel->SetForegroundColour(wxColour(128, 128, 128));
    mainSizer->Add(m_modelLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    
    m_scanLabel = new wxStaticText(this, wxID_ANY, "");
    mainSizer->Add(m_scanLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    
    m_expandPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* expandSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* propBtn = new wxButton(m_expandPanel, ID_AI_PROP_DESIGNER, "🎨 Prop Designer");
    wxButton* valBtn = new wxButton(m_expandPanel, ID_AI_VALIDATOR, "✓ Validator");
    wxButton* powerBtn = new wxButton(m_expandPanel, ID_AI_POWER, "⚡ Power Inspector");
    wxButton* fppBtn = new wxButton(m_expandPanel, ID_AI_FPP_SYNC, "📡 FPP Sync");
    wxButton* dmxBtn = new wxButton(m_expandPanel, ID_AI_DMX_ADVISOR, "🔀 DMX Advisor");

    m_alwaysVisibleCheck = new wxCheckBox(m_expandPanel, ID_AI_PIN_ALWAYS_VISIBLE, "📌 Always Visible");
    
    wxButton* spotBtn = new wxButton(m_expandPanel, ID_AI_SPOTLIGHT, "🔍 Spotlight");
    spotBtn->SetToolTip("Universal AI Copilot Command Palette & Quick Launcher (Ctrl+Shift+A)");
    spotBtn->SetBackgroundColour(wxColour(45, 60, 90));
    spotBtn->SetForegroundColour(*wxWHITE);

    expandSizer->Add(spotBtn, 0, wxALL, 2);
    expandSizer->Add(propBtn, 0, wxALL, 2);
    expandSizer->Add(valBtn, 0, wxALL, 2);
    expandSizer->Add(powerBtn, 0, wxALL, 2);
    expandSizer->Add(fppBtn, 0, wxALL, 2);
    expandSizer->Add(dmxBtn, 0, wxALL, 2);
    expandSizer->Add(m_alwaysVisibleCheck, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 6);
    
    m_expandPanel->SetSizer(expandSizer);
    
    // Load preference from config
    bool alwaysVisible = false;
    if (wxConfigBase::Get()) {
        wxConfigBase::Get()->Read("AI_ToolbarAlwaysVisible", &alwaysVisible, false);
    }
    m_alwaysVisibleCheck->SetValue(alwaysVisible);
    m_expanded = alwaysVisible;
    m_expandPanel->Show(m_expanded);
    m_toggleBtn->SetLabel(m_expanded ? "⬡ AI ◂" : "⬡ AI ▸");
    
    mainSizer->Add(m_expandPanel, 0, wxALIGN_CENTER_VERTICAL);
    
    SetSizer(mainSizer);
}

void AIStatusBar::OnToggleExpand(wxCommandEvent& WXUNUSED(evt))
{
    m_expanded = !m_expanded;
    m_expandPanel->Show(m_expanded);
    m_toggleBtn->SetLabel(m_expanded ? "⬡ AI ◂" : "⬡ AI ▸");
    if (GetParent()) {
        GetParent()->Layout();
    }
}

void AIStatusBar::OnPinAlwaysVisible(wxCommandEvent& evt)
{
    bool pinned = evt.IsChecked();
    if (wxConfigBase::Get()) {
        wxConfigBase::Get()->Write("AI_ToolbarAlwaysVisible", pinned);
        wxConfigBase::Get()->Flush();
    }
}

void AIStatusBar::OnOpenPropDesigner(wxCommandEvent& WXUNUSED(evt))
{
    AICustomPropDesignerDialog dlg(GetParent());
    dlg.ShowModal();
}

void AIStatusBar::OnOpenValidator(wxCommandEvent& WXUNUSED(evt))
{
    xLights::AI::AISequenceValidatorDialog dlg(GetParent());
    dlg.ShowModal();
}

void AIStatusBar::OnOpenPowerInspector(wxCommandEvent& WXUNUSED(evt))
{
    xLights::AI::AIPowerInjectionDialog dlg(GetParent());
    dlg.ShowModal();
}

void AIStatusBar::OnOpenFPPSync(wxCommandEvent& WXUNUSED(evt))
{
    xLights::AI::AIFPPSyncDialog dlg(GetParent());
    dlg.ShowModal();
}

void AIStatusBar::OnOpenDMXAdvisor(wxCommandEvent& WXUNUSED(evt))
{
    xLights::AI::AIDMXAddressDialog dlg(GetParent());
    dlg.ShowModal();
}

void AIStatusBar::OnOpenSpotlight(wxCommandEvent& WXUNUSED(evt))
{
    xLights::AI::AICopilotSpotlightDialog dlg(GetParent());
    dlg.ShowModal();
}

void AIStatusBar::SetActiveModel(const wxString& modelName)
{
    m_modelLabel->SetLabel("Model: " + modelName);
}

void AIStatusBar::SetScanState(bool scanning)
{
    m_scanLabel->SetLabel(scanning ? "● Scanning..." : "");
}
