// Copyright (c) xLights Project
#include "src-ui-wx/ai/AICustomPropDesignerDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/dcclient.h>
#include <wx/dc.h>
#include <wx/textdlg.h>
#include <wx/colour.h>
#include <spdlog/spdlog.h>
#include <map>
#include <string>
#include <cmath>

enum {
    ID_PROP_GENERATE     = 15001,
    ID_PROP_EXPORT       = 15002,
    ID_PROP_UNDO         = 15003,
    ID_PROP_REDO         = 15004,
    ID_PROP_TOGGLE_VIEW  = 15005,
    ID_PROP_ASK_AI       = 15006,
    ID_PROP_CONFIRM_EXEC = 15007,
    ID_PROP_CANCEL_EXEC  = 15008,
    ID_PROP_ADD_NODE     = 15009,
    ID_PROP_REMOVE_NODE  = 15010,
    ID_PROP_NEW_BRANCH   = 15011,
    ID_PROP_SWITCH_BRANCH= 15012,
    ID_NODE_X_SPIN       = 15013,
    ID_NODE_Y_SPIN       = 15014
};

BEGIN_EVENT_TABLE(AICustomPropDesignerDialog, wxDialog)
    EVT_BUTTON(ID_PROP_GENERATE, AICustomPropDesignerDialog::OnGenerateClick)
    EVT_BUTTON(ID_PROP_EXPORT, AICustomPropDesignerDialog::OnExportXmlClick)
    EVT_BUTTON(ID_PROP_UNDO, AICustomPropDesignerDialog::OnUndoClick)
    EVT_BUTTON(ID_PROP_REDO, AICustomPropDesignerDialog::OnRedoClick)
    EVT_BUTTON(ID_PROP_TOGGLE_VIEW, AICustomPropDesignerDialog::On3DToggle)
    EVT_BUTTON(ID_PROP_ASK_AI, AICustomPropDesignerDialog::OnAskAIClick)
    EVT_BUTTON(ID_PROP_CONFIRM_EXEC, AICustomPropDesignerDialog::OnConfirmExecClick)
    EVT_BUTTON(ID_PROP_CANCEL_EXEC, AICustomPropDesignerDialog::OnCancelExecClick)
    EVT_BUTTON(ID_PROP_ADD_NODE, AICustomPropDesignerDialog::OnAddNodeClick)
    EVT_BUTTON(ID_PROP_REMOVE_NODE, AICustomPropDesignerDialog::OnRemoveNodeClick)
    EVT_BUTTON(ID_PROP_NEW_BRANCH, AICustomPropDesignerDialog::OnNewBranchClick)
    EVT_BUTTON(ID_PROP_SWITCH_BRANCH, AICustomPropDesignerDialog::OnSwitchBranchClick)
    EVT_BUTTON(wxID_CANCEL, AICustomPropDesignerDialog::OnCloseClick)
    EVT_SPINCTRLDOUBLE(ID_NODE_X_SPIN, AICustomPropDesignerDialog::OnNodeXChanged)
    EVT_SPINCTRLDOUBLE(ID_NODE_Y_SPIN, AICustomPropDesignerDialog::OnNodeYChanged)
END_EVENT_TABLE()

AICustomPropDesignerDialog::AICustomPropDesignerDialog(wxWindow* parent, wxWindowID id,
                                                       const wxString& title,
                                                       const wxPoint& pos,
                                                       const wxSize& size,
                                                       long style)
    : wxDialog(parent, id, title, pos, size, style)
{
    InitUI();
}

void AICustomPropDesignerDialog::InitUI()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* toolbarSizer = new wxBoxSizer(wxHORIZONTAL);
    m_undoBtn = new wxButton(this, ID_PROP_UNDO, "↩ Undo");
    m_undoBtn->Enable(false);
    m_redoBtn = new wxButton(this, ID_PROP_REDO, "↪ Redo");
    m_redoBtn->Enable(false);
    m_toggleViewBtn = new wxButton(this, ID_PROP_TOGGLE_VIEW, "Switch to 2D");
    toolbarSizer->Add(m_undoBtn, 0, wxALL, 5);
    toolbarSizer->Add(m_redoBtn, 0, wxALL, 5);
    toolbarSizer->Add(m_toggleViewBtn, 0, wxALL, 5);
    toolbarSizer->AddStretchSpacer(1);
    mainSizer->Add(toolbarSizer, 0, wxEXPAND);

    wxBoxSizer* contentSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // LEFT wxPanel
    wxPanel* leftPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(280, -1));
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    
    wxStaticBoxSizer* descSizer = new wxStaticBoxSizer(wxVERTICAL, leftPanel, "Prop Description");
    m_descriptionCtrl = new wxTextCtrl(leftPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE);
    m_descriptionCtrl->SetHint("Describe your prop...");
    descSizer->Add(m_descriptionCtrl, 0, wxEXPAND | wxALL, 5);
    
    wxBoxSizer* nodeCountSizer = new wxBoxSizer(wxHORIZONTAL);
    nodeCountSizer->Add(new wxStaticText(leftPanel, wxID_ANY, "Node Count:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_nodeCountSpin = new wxSpinCtrl(leftPanel, wxID_ANY);
    m_nodeCountSpin->SetRange(1, 512);
    m_nodeCountSpin->SetValue(50);
    nodeCountSizer->Add(m_nodeCountSpin, 1, wxEXPAND | wxALL, 5);
    descSizer->Add(nodeCountSizer, 0, wxEXPAND);
    
    wxBoxSizer* shapeSizer = new wxBoxSizer(wxHORIZONTAL);
    shapeSizer->Add(new wxStaticText(leftPanel, wxID_ANY, "Shape Preset:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    wxArrayString shapes;
    shapes.Add("Circle"); shapes.Add("Grid"); shapes.Add("Star");
    shapes.Add("Mega Tree"); shapes.Add("Candy Cane"); shapes.Add("Custom");
    m_shapePresetChoice = new wxChoice(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, shapes);
    m_shapePresetChoice->SetSelection(0);
    shapeSizer->Add(m_shapePresetChoice, 1, wxEXPAND | wxALL, 5);
    descSizer->Add(shapeSizer, 0, wxEXPAND);
    
    m_generateBtn = new wxButton(leftPanel, ID_PROP_GENERATE, "Generate Layout");
    descSizer->Add(m_generateBtn, 0, wxEXPAND | wxALL, 5);
    leftSizer->Add(descSizer, 0, wxEXPAND | wxALL, 5);
    
    wxStaticBoxSizer* aiSizer = new wxStaticBoxSizer(wxVERTICAL, leftPanel, "AI LLM Node Editor");
    m_aiPromptCtrl = new wxTextCtrl(leftPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 60), wxTE_MULTILINE);
    m_aiPromptCtrl->SetHint("Describe edits in plain English...");
    aiSizer->Add(m_aiPromptCtrl, 0, wxEXPAND | wxALL, 5);
    m_askAIBtn = new wxButton(leftPanel, ID_PROP_ASK_AI, "Ask AI");
    aiSizer->Add(m_askAIBtn, 0, wxEXPAND | wxALL, 5);
    m_aiResponseCtrl = new wxTextCtrl(leftPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE | wxTE_READONLY);
    m_aiResponseCtrl->SetBackgroundColour(wxColour(50,50,50));
    m_aiResponseCtrl->SetForegroundColour(*wxWHITE);
    m_aiResponseCtrl->Show(false);
    aiSizer->Add(m_aiResponseCtrl, 0, wxEXPAND | wxALL, 5);
    
    wxBoxSizer* aiBtnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_confirmExecBtn = new wxButton(leftPanel, ID_PROP_CONFIRM_EXEC, "Confirm Execute");
    m_cancelExecBtn = new wxButton(leftPanel, ID_PROP_CANCEL_EXEC, "Cancel");
    m_confirmExecBtn->Show(false);
    m_cancelExecBtn->Show(false);
    aiBtnSizer->Add(m_confirmExecBtn, 1, wxEXPAND | wxALL, 2);
    aiBtnSizer->Add(m_cancelExecBtn, 1, wxEXPAND | wxALL, 2);
    aiSizer->Add(aiBtnSizer, 0, wxEXPAND | wxALL, 5);
    leftSizer->Add(aiSizer, 0, wxEXPAND | wxALL, 5);
    
    wxStaticBoxSizer* selSizer = new wxStaticBoxSizer(wxVERTICAL, leftPanel, "Selected Node");
    m_selectedNodeLabel = new wxStaticText(leftPanel, wxID_ANY, "No node selected");
    selSizer->Add(m_selectedNodeLabel, 0, wxALL, 5);
    
    wxBoxSizer* xSizer = new wxBoxSizer(wxHORIZONTAL);
    xSizer->Add(new wxStaticText(leftPanel, wxID_ANY, "X:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_nodeXSpin = new wxSpinCtrlDouble(leftPanel, ID_NODE_X_SPIN);
    m_nodeXSpin->SetRange(-9999.0, 9999.0);
    m_nodeXSpin->SetIncrement(0.5);
    xSizer->Add(m_nodeXSpin, 1, wxEXPAND | wxALL, 5);
    selSizer->Add(xSizer, 0, wxEXPAND);
    
    wxBoxSizer* ySizer = new wxBoxSizer(wxHORIZONTAL);
    ySizer->Add(new wxStaticText(leftPanel, wxID_ANY, "Y:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_nodeYSpin = new wxSpinCtrlDouble(leftPanel, ID_NODE_Y_SPIN);
    m_nodeYSpin->SetRange(-9999.0, 9999.0);
    m_nodeYSpin->SetIncrement(0.5);
    ySizer->Add(m_nodeYSpin, 1, wxEXPAND | wxALL, 5);
    selSizer->Add(ySizer, 0, wxEXPAND);
    
    wxBoxSizer* newIdSizer = new wxBoxSizer(wxHORIZONTAL);
    newIdSizer->Add(new wxStaticText(leftPanel, wxID_ANY, "New Node ID:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_newNodeIdSpin = new wxSpinCtrl(leftPanel, wxID_ANY);
    m_newNodeIdSpin->SetRange(1, 9999);
    m_newNodeIdSpin->SetValue(1);
    newIdSizer->Add(m_newNodeIdSpin, 1, wxEXPAND | wxALL, 5);
    selSizer->Add(newIdSizer, 0, wxEXPAND);
    
    wxBoxSizer* nodeBtnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_addNodeBtn = new wxButton(leftPanel, ID_PROP_ADD_NODE, "Add Node");
    m_removeNodeBtn = new wxButton(leftPanel, ID_PROP_REMOVE_NODE, "Remove Node");
    nodeBtnSizer->Add(m_addNodeBtn, 1, wxEXPAND | wxALL, 2);
    nodeBtnSizer->Add(m_removeNodeBtn, 1, wxEXPAND | wxALL, 2);
    selSizer->Add(nodeBtnSizer, 0, wxEXPAND | wxALL, 5);
    leftSizer->Add(selSizer, 0, wxEXPAND | wxALL, 5);
    
    wxStaticBoxSizer* branchSizer = new wxStaticBoxSizer(wxVERTICAL, leftPanel, "Model Branches");
    m_branchChoice = new wxChoice(leftPanel, wxID_ANY);
    m_branchChoice->Append("[Main]");
    m_branchChoice->SetSelection(0);
    branchSizer->Add(m_branchChoice, 0, wxEXPAND | wxALL, 5);
    
    wxBoxSizer* branchBtnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_newBranchBtn = new wxButton(leftPanel, ID_PROP_NEW_BRANCH, "New Branch");
    m_switchBranchBtn = new wxButton(leftPanel, ID_PROP_SWITCH_BRANCH, "Switch");
    branchBtnSizer->Add(m_newBranchBtn, 1, wxEXPAND | wxALL, 2);
    branchBtnSizer->Add(m_switchBranchBtn, 1, wxEXPAND | wxALL, 2);
    branchSizer->Add(branchBtnSizer, 0, wxEXPAND | wxALL, 5);
    leftSizer->Add(branchSizer, 0, wxEXPAND | wxALL, 5);
    
    leftPanel->SetSizer(leftSizer);
    contentSizer->Add(leftPanel, 0, wxEXPAND | wxALL, 5);
    
    // RIGHT wxPanel
    wxPanel* rightPanel = new wxPanel(this, wxID_ANY);
    wxStaticBoxSizer* previewSizer = new wxStaticBoxSizer(wxVERTICAL, rightPanel, "Preview Canvas");
    m_canvasPanel = new wxPanel(rightPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
    m_canvasPanel->SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    m_canvasPanel->Bind(wxEVT_PAINT, &AICustomPropDesignerDialog::OnCanvasPaint, this);
    m_canvasPanel->Bind(wxEVT_LEFT_DOWN, &AICustomPropDesignerDialog::OnCanvasLeftDown, this);
    m_canvasPanel->Bind(wxEVT_LEFT_UP, &AICustomPropDesignerDialog::OnCanvasLeftUp, this);
    m_canvasPanel->Bind(wxEVT_MOTION, &AICustomPropDesignerDialog::OnCanvasMouseMove, this);
    m_canvasPanel->Bind(wxEVT_MOUSEWHEEL, &AICustomPropDesignerDialog::OnCanvasMouseWheel, this);
    
    previewSizer->Add(m_canvasPanel, 1, wxEXPAND | wxALL, 5);
    rightPanel->SetSizer(previewSizer);
    contentSizer->Add(rightPanel, 1, wxEXPAND | wxALL, 5);
    
    mainSizer->Add(contentSizer, 1, wxEXPAND);
    
    m_statusLabel = new wxStaticText(this, wxID_ANY, "Ready — 0 nodes");
    mainSizer->Add(m_statusLabel, 0, wxEXPAND | wxALL, 5);
    
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_exportXmlBtn = new wxButton(this, ID_PROP_EXPORT, "Export Model XML");
    btnSizer->Add(m_exportXmlBtn, 0, wxALL, 5);
    btnSizer->AddStretchSpacer(1);
    m_closeBtn = new wxButton(this, wxID_CANCEL, "Close");
    btnSizer->Add(m_closeBtn, 0, wxALL, 5);
    mainSizer->Add(btnSizer, 0, wxEXPAND);
    
    SetSizer(mainSizer);
    UpdateSelectedNodeControls();
}

void AICustomPropDesignerDialog::InitToolbar()
{
}

void AICustomPropDesignerDialog::OnGenerateClick(wxCommandEvent& event)
{
    CustomPropDesignerAI ai;
    std::string desc = m_descriptionCtrl->GetValue().ToStdString();
    int nodeCount = m_nodeCountSpin->GetValue();
    auto before = m_lastNodes;
    auto after = ai.GenerateNodeLayout(desc, nodeCount);
    m_history.PushEdit("Generate Layout", before, after);
    m_lastNodes = after;
    m_selectedNodeIndex = -1;
    RefreshCanvas();
    UpdateUndoRedoButtons();
    m_statusLabel->SetLabel(wxString::Format("Generated %d nodes. Undo available.", (int)m_lastNodes.size()));
}

void AICustomPropDesignerDialog::OnUndoClick(wxCommandEvent& event)
{
    if (!m_history.CanUndo()) return;
    std::string desc;
    auto state = m_history.Undo(desc);
    m_lastNodes = state;
    m_selectedNodeIndex = -1;
    RefreshCanvas();
    UpdateUndoRedoButtons();
    m_statusLabel->SetLabel(wxString::Format("Undone: %s. Undo depth: %d", desc.c_str(), m_history.UndoDepth()));
}

void AICustomPropDesignerDialog::OnRedoClick(wxCommandEvent& event)
{
    if (!m_history.CanRedo()) return;
    std::string desc;
    auto state = m_history.Redo(desc);
    m_lastNodes = state;
    m_selectedNodeIndex = -1;
    RefreshCanvas();
    UpdateUndoRedoButtons();
    m_statusLabel->SetLabel(wxString::Format("Redone: %s. Undo depth: %d", desc.c_str(), m_history.UndoDepth()));
}

void AICustomPropDesignerDialog::On3DToggle(wxCommandEvent& event)
{
    m_is3DMode = !m_is3DMode;
    m_toggleViewBtn->SetLabel(m_is3DMode ? "Switch to 2D" : "Switch to 3D");
    m_canvasPanel->Refresh();
}

void AICustomPropDesignerDialog::OnAskAIClick(wxCommandEvent& event)
{
    std::string prompt = m_aiPromptCtrl->GetValue().ToStdString();
    if (prompt.empty()) {
        wxMessageBox("Please enter a prompt", "Notice");
        return;
    }
    auto cfg = AIConfigurationManager::Instance().GetSettings();
    AINodeEditInterpreter interp;
    m_pendingPlan = interp.InterpretPrompt(prompt, m_lastNodes, cfg);
    m_aiResponseCtrl->SetValue(wxString(m_pendingPlan.confirmationMessage));
    m_aiResponseCtrl->Show(true);
    m_confirmExecBtn->Show(true);
    m_cancelExecBtn->Show(true);
    Layout();
    m_statusLabel->SetLabel("AI response ready. Confirm or Cancel.");
}

void AICustomPropDesignerDialog::OnConfirmExecClick(wxCommandEvent& event)
{
    auto before = m_lastNodes;
    AINodeEditInterpreter interp;
    m_lastNodes = interp.ExecutePlan(m_pendingPlan, m_lastNodes);
    m_history.PushEdit("AI: " + m_pendingPlan.confirmationMessage.substr(0, 40), before, m_lastNodes);
    m_aiResponseCtrl->Hide();
    m_confirmExecBtn->Hide();
    m_cancelExecBtn->Hide();
    Layout();
    RefreshCanvas();
    UpdateUndoRedoButtons();
    m_statusLabel->SetLabel("AI edits applied. Undo available.");
}

void AICustomPropDesignerDialog::OnCancelExecClick(wxCommandEvent& event)
{
    m_pendingPlan = NodeEditPlan{};
    m_aiResponseCtrl->Hide();
    m_confirmExecBtn->Hide();
    m_cancelExecBtn->Hide();
    Layout();
    m_statusLabel->SetLabel("AI edit cancelled.");
}

void AICustomPropDesignerDialog::OnAddNodeClick(wxCommandEvent& event)
{
    float cx = 50.0f, cy = 50.0f;
    int reqId = m_newNodeIdSpin->GetValue();
    auto before = m_lastNodes;
    InsertNodeResult result = m_engine.InsertNode(m_lastNodes, cx, cy, 0.0f, reqId);
    if (result.renumberingRequired) {
        if (wxMessageBox(wxString(result.aiSuggestion) + "\n\nApply renumbering?", "Confirm", wxYES_NO) == wxYES) {
            result.updatedNodes = m_engine.RenumberNodesFrom(result.updatedNodes, reqId, 1);
        }
    }
    m_history.PushEdit("Add Node ID " + std::to_string(reqId), before, result.updatedNodes);
    m_lastNodes = result.updatedNodes;
    RefreshCanvas();
    UpdateUndoRedoButtons();
    m_statusLabel->SetLabel("Node added. Undo available.");
}

void AICustomPropDesignerDialog::OnRemoveNodeClick(wxCommandEvent& event)
{
    if (m_selectedNodeIndex < 0) return;
    auto before = m_lastNodes;
    m_lastNodes = m_engine.RemoveNode(m_lastNodes, m_selectedNodeIndex);
    m_history.PushEdit("Remove Node " + std::to_string(m_selectedNodeIndex), before, m_lastNodes);
    m_selectedNodeIndex = -1;
    RefreshCanvas();
    UpdateUndoRedoButtons();
}

void AICustomPropDesignerDialog::OnNodeXChanged(wxSpinDoubleEvent& event)
{
    if (m_selectedNodeIndex < 0 || m_isDragging) return;
    auto before = m_lastNodes;
    m_engine.SetNodePosition(m_lastNodes, m_selectedNodeIndex, (float)m_nodeXSpin->GetValue(), (float)m_nodeYSpin->GetValue(), 0.0f);
    m_history.PushEdit("Move Node " + std::to_string(m_selectedNodeIndex), before, m_lastNodes);
    RefreshCanvas();
    UpdateUndoRedoButtons();
}

void AICustomPropDesignerDialog::OnNodeYChanged(wxSpinDoubleEvent& event)
{
    if (m_selectedNodeIndex < 0 || m_isDragging) return;
    auto before = m_lastNodes;
    m_engine.SetNodePosition(m_lastNodes, m_selectedNodeIndex, (float)m_nodeXSpin->GetValue(), (float)m_nodeYSpin->GetValue(), 0.0f);
    m_history.PushEdit("Move Node " + std::to_string(m_selectedNodeIndex), before, m_lastNodes);
    RefreshCanvas();
    UpdateUndoRedoButtons();
}

void AICustomPropDesignerDialog::OnNewBranchClick(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "Branch name:", "New Model Branch", "Branch_1");
    if (dlg.ShowModal() == wxID_OK) {
        std::string name = dlg.GetValue().ToStdString();
        m_branches[name] = m_lastNodes;
        m_branchChoice->Append(dlg.GetValue());
        m_statusLabel->SetLabel("Branch '" + dlg.GetValue() + "' saved.");
    }
}

void AICustomPropDesignerDialog::OnSwitchBranchClick(wxCommandEvent& event)
{
    std::string sel = m_branchChoice->GetStringSelection().ToStdString();
    if (sel == "[Main]" || m_branches.count(sel) == 0) return;
    auto before = m_lastNodes;
    m_history.PushEdit("Switch to branch: " + sel, before, m_branches[sel]);
    m_lastNodes = m_branches[sel];
    m_selectedNodeIndex = -1;
    RefreshCanvas();
    UpdateUndoRedoButtons();
}

void AICustomPropDesignerDialog::RefreshCanvas()
{
    m_canvasPanel->Refresh();
    m_canvasPanel->Update();
}

void AICustomPropDesignerDialog::UpdateUndoRedoButtons()
{
    m_undoBtn->Enable(m_history.CanUndo());
    if (m_history.CanUndo()) {
        m_undoBtn->SetLabel("↩ Undo: " + wxString(m_history.GetUndoDescription()));
    } else {
        m_undoBtn->SetLabel("↩ Undo");
    }
    m_redoBtn->Enable(m_history.CanRedo());
    if (m_history.CanRedo()) {
        m_redoBtn->SetLabel("↪ Redo: " + wxString(m_history.GetRedoDescription()));
    } else {
        m_redoBtn->SetLabel("↪ Redo");
    }
    m_statusLabel->SetLabel(wxString::Format("%d nodes | Undo: %d steps available", (int)m_lastNodes.size(), m_history.UndoDepth()));
}

void AICustomPropDesignerDialog::UpdateSelectedNodeControls()
{
    if (m_selectedNodeIndex < 0) {
        m_selectedNodeLabel->SetLabel("No node selected");
        m_nodeXSpin->Enable(false);
        m_nodeYSpin->Enable(false);
        m_removeNodeBtn->Enable(false);
    } else {
        auto& n = m_lastNodes[m_selectedNodeIndex];
        m_selectedNodeLabel->SetLabel(wxString::Format("Node %d: %s", m_selectedNodeIndex, n.label.c_str()));
        m_nodeXSpin->SetValue(n.x);
        m_nodeYSpin->SetValue(n.y);
        m_nodeXSpin->Enable(true);
        m_nodeYSpin->Enable(true);
        m_removeNodeBtn->Enable(true);
    }
}

void AICustomPropDesignerDialog::OnCanvasPaint(wxPaintEvent& event)
{
    wxPaintDC dc(m_canvasPanel);
    wxSize sz = m_canvasPanel->GetClientSize();
    int W = sz.GetWidth();
    int H = sz.GetHeight();
    
    if (m_is3DMode) {
        dc.SetBrush(wxBrush(wxColour(20, 25, 35)));
    } else {
        dc.SetBrush(wxBrush(wxColour(30, 30, 30)));
    }
    dc.DrawRectangle(0, 0, W, H);
    
    if (!m_is3DMode) {
        dc.SetPen(wxPen(wxColour(60, 60, 60)));
        for (int i = 0; i < W; i += 50) dc.DrawLine(i, 0, i, H);
        for (int i = 0; i < H; i += 50) dc.DrawLine(0, i, W, i);
    } else {
        dc.SetPen(wxPen(wxColour(60, 60, 60)));
        int vpX = W / 2;
        int vpY = H * 0.4;
        for (int i = 0; i <= 5; ++i) {
            dc.DrawLine(i * W / 5, H, vpX, vpY);
        }
        for (int i = 0; i <= 5; ++i) {
            int y = H - (H - vpY) * (i / 5.0f);
            dc.DrawLine(0, y, W, y);
        }
    }
    
    for (size_t i = 0; i < m_lastNodes.size(); ++i) {
        auto& node = m_lastNodes[i];
        float nx = node.x / 100.0f * W;
        float ny = node.y / 100.0f * H;
        if (m_is3DMode) {
            ny = ny * 0.6f + H * 0.2f;
        }
        bool selected = ((int)i == m_selectedNodeIndex);
        if (selected) {
            dc.SetBrush(wxBrush(wxColour(255, 100, 50)));
        } else {
            dc.SetBrush(wxBrush(wxColour(255, 200, 0)));
        }
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawCircle((int)nx, (int)ny, selected ? 7 : 4);
        dc.SetTextForeground(*wxWHITE);
        dc.DrawText(wxString(node.label), (int)nx + 8, (int)ny - 8);
    }
    
    if (m_selectedNodeIndex >= 0 && m_selectedNodeIndex < (int)m_lastNodes.size()) {
        auto& sn = m_lastNodes[m_selectedNodeIndex];
        int sx = sn.x / 100.0f * W;
        int sy = sn.y / 100.0f * H;
        if (m_is3DMode) {
            sy = sy * 0.6f + H * 0.2f;
        }
        dc.SetPen(wxPen(wxColour(255, 50, 50), 2));
        dc.DrawLine(sx, sy, sx + 40, sy);
        dc.DrawText("X", sx + 42, sy - 6);
        
        dc.SetPen(wxPen(wxColour(50, 255, 50), 2));
        dc.DrawLine(sx, sy, sx, sy - 40);
        dc.DrawText("Y", sx + 4, sy - 48);
        
        dc.SetPen(wxPen(wxColour(80, 120, 255), 2));
        dc.DrawLine(sx, sy, sx - 28, sy - 28);
        dc.DrawText("Z", sx - 42, sy - 40);
    }
}

void AICustomPropDesignerDialog::OnCanvasLeftDown(wxMouseEvent& event)
{
    m_isDragging = false;
    m_lastMousePos = event.GetPosition();
    wxSize sz = m_canvasPanel->GetClientSize();
    int W = sz.GetWidth();
    int H = sz.GetHeight();
    
    int bestIdx = -1;
    float bestDist = 12.0f;
    for (size_t i = 0; i < m_lastNodes.size(); ++i) {
        auto& node = m_lastNodes[i];
        float nx = node.x / 100.0f * W;
        float ny = node.y / 100.0f * H;
        if (m_is3DMode) {
            ny = ny * 0.6f + H * 0.2f;
        }
        float dx = nx - m_lastMousePos.x;
        float dy = ny - m_lastMousePos.y;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist < bestDist) {
            bestDist = dist;
            bestIdx = (int)i;
        }
    }
    
    m_selectedNodeIndex = bestIdx;
    UpdateSelectedNodeControls();
    m_canvasPanel->Refresh();
}

void AICustomPropDesignerDialog::OnCanvasMouseMove(wxMouseEvent& event)
{
    if (event.LeftIsDown() && m_selectedNodeIndex >= 0) {
        wxPoint delta = event.GetPosition() - m_lastMousePos;
        m_lastMousePos = event.GetPosition();
        wxSize sz = m_canvasPanel->GetClientSize();
        float dx = (float)delta.x / sz.GetWidth() * 100.0f;
        float dy = (float)delta.y / sz.GetHeight() * 100.0f;
        
        if (m_is3DMode) {
            dy = dy / 0.6f;
        }
        
        m_lastNodes[m_selectedNodeIndex].x += dx;
        m_lastNodes[m_selectedNodeIndex].y += dy;
        m_isDragging = true;
        
        m_nodeXSpin->SetValue(m_lastNodes[m_selectedNodeIndex].x);
        m_nodeYSpin->SetValue(m_lastNodes[m_selectedNodeIndex].y);
        
        m_canvasPanel->Refresh();
    }
}

void AICustomPropDesignerDialog::OnCanvasLeftUp(wxMouseEvent& event)
{
    if (m_isDragging && m_selectedNodeIndex >= 0) {
        m_isDragging = false;
        UpdateUndoRedoButtons();
    }
}

void AICustomPropDesignerDialog::OnCanvasMouseWheel(wxMouseEvent& event)
{
    int rot = event.GetWheelRotation();
    float scale = rot > 0 ? 0.95f : 1.05f;
    for (auto& node : m_lastNodes) {
        node.x = 50.0f + (node.x - 50.0f) * scale;
        node.y = 50.0f + (node.y - 50.0f) * scale;
    }
    m_canvasPanel->Refresh();
}

void AICustomPropDesignerDialog::OnExportXmlClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Export Model XML", "", "CustomProp.xmodel", "xLights Model Files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        CustomPropDesignerAI ai;
        ai.ExportToXLightsModelXML(m_lastNodes, dlg.GetPath().ToStdString());
        wxMessageBox("Export successful!", "Export", wxOK | wxICON_INFORMATION);
    }
}

void AICustomPropDesignerDialog::OnCloseClick(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}
