// Copyright (c) xLights Project
#include "src-ui-wx/ai/AIStatusBar.h"
#include "src-ui-wx/ai/AICustomPropDesignerDialog.h"
#include "src-ui-wx/ai/AISequenceValidatorDialog.h"
#include "src-ui-wx/ai/AIPowerInjectionDialog.h"
#include <wx/msgdlg.h>

enum {
    ID_AI_TOGGLE = 16001,
    ID_AI_PROP_DESIGNER = 16002,
    ID_AI_VALIDATOR = 16003,
    ID_AI_POWER = 16004
};

BEGIN_EVENT_TABLE(AIStatusBar, wxPanel)
    EVT_BUTTON(ID_AI_TOGGLE, AIStatusBar::OnToggleExpand)
    EVT_BUTTON(ID_AI_PROP_DESIGNER, AIStatusBar::OnOpenPropDesigner)
    EVT_BUTTON(ID_AI_VALIDATOR, AIStatusBar::OnOpenValidator)
    EVT_BUTTON(ID_AI_POWER, AIStatusBar::OnOpenPowerInspector)
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
    expandSizer->Add(propBtn, 0, wxALL, 2);
    expandSizer->Add(valBtn, 0, wxALL, 2);
    expandSizer->Add(powerBtn, 0, wxALL, 2);
    m_expandPanel->SetSizer(expandSizer);
    m_expandPanel->Show(false);
    
    mainSizer->Add(m_expandPanel, 0, wxALIGN_CENTER_VERTICAL);
    
    SetSizer(mainSizer);
}

void AIStatusBar::OnToggleExpand(wxCommandEvent& evt)
{
    m_expanded = !m_expanded;
    m_expandPanel->Show(m_expanded);
    m_toggleBtn->SetLabel(m_expanded ? "⬡ AI ◂" : "⬡ AI ▸");
    GetParent()->Layout();
}

void AIStatusBar::OnOpenPropDesigner(wxCommandEvent& evt)
{
    (new AICustomPropDesignerDialog(GetParent()))->ShowModal();
}

void AIStatusBar::OnOpenValidator(wxCommandEvent& evt)
{
    (new AISequenceValidatorDialog(GetParent()))->ShowModal();
}

void AIStatusBar::OnOpenPowerInspector(wxCommandEvent& evt)
{
    (new AIPowerInjectionDialog(GetParent()))->ShowModal();
}

void AIStatusBar::SetActiveModel(const wxString& modelName)
{
    m_modelLabel->SetLabel("Model: " + modelName);
}

void AIStatusBar::SetScanState(bool scanning)
{
    m_scanLabel->SetLabel(scanning ? "● Scanning..." : "");
}
