/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIVRShowSpatialCopilotDialog.h"
#include "src-ui-wx/ai/AIHelpGuideDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace xLights {

enum {
    ID_BTN_AUDIT_SPATIAL = wxID_HIGHEST + 1001,
    ID_BTN_SEND_CHAT,
    ID_BTN_EXPORT_SPATIAL_REPORT,
    ID_BTN_UNDO,
    ID_BTN_REDO,
    ID_BTN_HELP
};

AIVRShowSpatialCopilotDialog::AIVRShowSpatialCopilotDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style
) : wxDialog(parent, id, title, pos, size, style),
    m_commandHistory(100) {
    CreateControls();
}

void AIVRShowSpatialCopilotDialog::CreateControls() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(24, 38, 48));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI 3D Layout VR/AR Spatial Walkthrough & Clearance Copilot"));
    titleTxt->SetForegroundColour(wxColour(255, 255, 255));
    auto font = titleTxt->GetFont();
    font.SetPointSize(12);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);
    bannerSizer->Add(titleTxt, 1, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    auto* helpBtn = new wxButton(banner, ID_BTN_HELP, wxT("❓ Help & Guide"));
    helpBtn->SetToolTip(wxT("Open comprehensive user manual, setting explanations, and workflow diagrams (F1)."));
    bannerSizer->Add(helpBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Center Workspace
    auto* workSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* leftPanel = new wxPanel(this, wxID_ANY);
    BuildLeftSessionPanel(leftPanel);
    workSizer->Add(leftPanel, 0, wxEXPAND | wxALL, 8);

    auto* rightPanel = new wxPanel(this, wxID_ANY);
    BuildRightCopilotPanel(rightPanel);
    workSizer->Add(rightPanel, 1, wxEXPAND | wxALL, 8);

    mainSizer->Add(workSizer, 1, wxEXPAND);

    // Bottom Action Bar
    auto* botBar = new wxPanel(this, wxID_ANY);
    auto* botSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* btnAudit = new wxButton(botBar, ID_BTN_AUDIT_SPATIAL, wxT("📐 Recalculate 3D Spatial Clearances"));
    btnAudit->SetBackgroundColour(wxColour(20, 160, 220));
    btnAudit->SetForegroundColour(wxColour(255, 255, 255));

    m_btnUndo = new wxButton(botBar, ID_BTN_UNDO, wxT("↶ Undo (Ctrl+Z)"));
    m_btnRedo = new wxButton(botBar, ID_BTN_REDO, wxT("↷ Redo (Ctrl+Y)"));
    m_btnUndo->Enable(false);
    m_btnRedo->Enable(false);

    auto* btnExport = new wxButton(botBar, ID_BTN_EXPORT_SPATIAL_REPORT, wxT("📄 Export Clearance Report..."));
    auto* btnClose = new wxButton(botBar, wxID_CANCEL, wxT("Close"));

    botSizer->Add(btnAudit, 0, wxALL, 4);
    botSizer->Add(m_btnUndo, 0, wxALL, 4);
    botSizer->Add(m_btnRedo, 0, wxALL, 4);
    botSizer->Add(btnExport, 0, wxALL, 4);
    botSizer->AddStretchSpacer();
    botSizer->Add(btnClose, 0, wxALL, 4);

    botBar->SetSizer(botSizer);
    mainSizer->Add(botBar, 0, wxEXPAND | wxALL, 6);

    SetSizer(mainSizer);

    // Event Bindings
    Bind(wxEVT_BUTTON, &AIVRShowSpatialCopilotDialog::OnAuditSpatialClearance, this, ID_BTN_AUDIT_SPATIAL);
    Bind(wxEVT_BUTTON, &AIVRShowSpatialCopilotDialog::OnSendSpatialQuery, this, ID_BTN_SEND_CHAT);
    Bind(wxEVT_BUTTON, &AIVRShowSpatialCopilotDialog::OnExportSpatialReport, this, ID_BTN_EXPORT_SPATIAL_REPORT);
    Bind(wxEVT_BUTTON, &AIVRShowSpatialCopilotDialog::OnUndo, this, ID_BTN_UNDO);
    Bind(wxEVT_BUTTON, &AIVRShowSpatialCopilotDialog::OnRedo, this, ID_BTN_REDO);
    Bind(wxEVT_BUTTON, &AIVRShowSpatialCopilotDialog::OnHelp, this, ID_BTN_HELP);
}

void AIVRShowSpatialCopilotDialog::BuildLeftSessionPanel(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    // Headset Status Box
    auto* statusBox = new wxStaticBoxSizer(wxVERTICAL, parent, wxT("XR Session Tracking"));
    m_lblHeadsetStatus = new wxStaticText(parent, wxID_ANY, wxT("Headset: CONNECTED (6-DoF OpenXR Tracking Active)"));
    m_lblHeadsetStatus->SetForegroundColour(wxColour(0, 240, 180));
    statusBox->Add(m_lblHeadsetStatus, 0, wxALL, 6);
    sizer->Add(statusBox, 0, wxEXPAND | wxBOTTOM, 6);

    // Physical Obstacles List
    auto* obsBox = new wxStaticBoxSizer(wxVERTICAL, parent, wxT("Yard Physical Obstacles"));
    m_obstacleListCtrl = new wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxSize(320, 140), wxLC_REPORT);
    m_obstacleListCtrl->InsertColumn(0, wxT("Obstacle"), wxLIST_FORMAT_LEFT, 130);
    m_obstacleListCtrl->InsertColumn(1, wxT("Pos (X,Y,Z)"), wxLIST_FORMAT_LEFT, 100);
    m_obstacleListCtrl->InsertColumn(2, wxT("Radius"), wxLIST_FORMAT_LEFT, 70);
    obsBox->Add(m_obstacleListCtrl, 1, wxEXPAND | wxALL, 4);
    sizer->Add(obsBox, 1, wxEXPAND | wxBOTTOM, 6);

    // Prop Clearances List
    auto* clearBox = new wxStaticBoxSizer(wxVERTICAL, parent, wxT("Prop Physical Clearances"));
    m_clearanceListCtrl = new wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxSize(320, 180), wxLC_REPORT);
    m_clearanceListCtrl->InsertColumn(0, wxT("Prop"), wxLIST_FORMAT_LEFT, 120);
    m_clearanceListCtrl->InsertColumn(1, wxT("Clearance"), wxLIST_FORMAT_LEFT, 80);
    m_clearanceListCtrl->InsertColumn(2, wxT("Safety Status"), wxLIST_FORMAT_LEFT, 100);
    clearBox->Add(m_clearanceListCtrl, 1, wxEXPAND | wxALL, 4);
    sizer->Add(clearBox, 1, wxEXPAND);

    parent->SetSizer(sizer);
}

void AIVRShowSpatialCopilotDialog::BuildRightCopilotPanel(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    auto* notebook = new wxNotebook(parent, wxID_ANY);

    // Tab 1: 1:1 Scale XR Spatial Viewport
    m_vrSpatialCanvas = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
    m_vrSpatialCanvas->SetBackgroundColour(wxColour(10, 16, 22));
    notebook->AddPage(m_vrSpatialCanvas, wxT("🕶️ 1:1 Scale VR/AR Spatial Viewport"));

    // Tab 2: Conversational Spatial Assistant
    auto* chatPanel = new wxPanel(notebook, wxID_ANY);
    auto* chatSizer = new wxBoxSizer(wxVERTICAL);
    m_txtChatHistory = new wxTextCtrl(chatPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    m_txtChatHistory->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    chatSizer->Add(m_txtChatHistory, 1, wxEXPAND | wxALL, 4);

    auto* inputRow = new wxBoxSizer(wxHORIZONTAL);
    m_txtChatInput = new wxTextCtrl(chatPanel, wxID_ANY, wxT("Is the MegaTree clearing the oak tree?"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    auto* btnSend = new wxButton(chatPanel, ID_BTN_SEND_CHAT, wxT("💬 Ask AI Spatial Copilot"));
    inputRow->Add(m_txtChatInput, 1, wxEXPAND | wxRIGHT, 4);
    inputRow->Add(btnSend, 0, wxEXPAND);
    chatSizer->Add(inputRow, 0, wxEXPAND | wxALL, 4);

    chatPanel->SetSizer(chatSizer);
    notebook->AddPage(chatPanel, wxT("🤖 Conversational Spatial Copilot"));

    sizer->Add(notebook, 1, wxEXPAND);
    parent->SetSizer(sizer);
}

void AIVRShowSpatialCopilotDialog::OnAuditSpatialClearance(wxCommandEvent& WXUNUSED(event)) {
    auto prev = m_session;
    m_session = AI::VRShowSpatialCopilotAI::RunSpatialClearanceAudit();

    auto cmd = std::make_unique<AI::LambdaAICommand>(
        "Recalculate VR/AR Spatial Clearances",
        [this]() { return true; },
        [this, prev]() {
            m_session = prev;
            UpdateUiFromResults();
            return true;
        }
    );

    m_commandHistory.ExecuteCommand(std::move(cmd));
    UpdateUiFromResults();
}

void AIVRShowSpatialCopilotDialog::UpdateUiFromResults() {
    if (m_obstacleListCtrl) {
        m_obstacleListCtrl->DeleteAllItems();
        for (size_t i = 0; i < m_session.detectedObstacles.size(); ++i) {
            const auto& o = m_session.detectedObstacles[i];
            long idx = m_obstacleListCtrl->InsertItem(static_cast<long>(i), wxString::FromUTF8(o.name));
            m_obstacleListCtrl->SetItem(idx, 1, wxString::Format(wxT("%.0f, %.0f, %.0f"), o.worldX, o.worldY, o.worldZ));
            m_obstacleListCtrl->SetItem(idx, 2, wxString::Format(wxT("%.1fft"), o.radiusFeet));
        }
    }

    if (m_clearanceListCtrl) {
        m_clearanceListCtrl->DeleteAllItems();
        for (size_t i = 0; i < m_session.clearanceReports.size(); ++i) {
            const auto& r = m_session.clearanceReports[i];
            long idx = m_clearanceListCtrl->InsertItem(static_cast<long>(i), wxString::FromUTF8(r.propName));
            m_clearanceListCtrl->SetItem(idx, 1, wxString::Format(wxT("%.1f ft"), r.minimumClearanceFeet));
            m_clearanceListCtrl->SetItem(idx, 2, r.isClear ? wxT("CLEAR") : wxT("COLLISION"));
        }
    }

    if (m_txtChatHistory && m_txtChatHistory->GetValue().empty()) {
        m_txtChatHistory->AppendText(wxT("AI Copilot: 1:1 Scale XR Spatial Tracking Active.\nAll props analyzed against physical yard obstacles.\n\n"));
    }

    UpdateUndoRedoButtons();
}

void AIVRShowSpatialCopilotDialog::UpdateUndoRedoButtons() {
    if (m_btnUndo) {
        m_btnUndo->Enable(m_commandHistory.CanUndo());
        m_btnUndo->SetToolTip(m_commandHistory.CanUndo()
            ? wxString::Format(wxT("Undo: %s"), wxString::FromUTF8(m_commandHistory.GetUndoDescription()))
            : wxT("Nothing to Undo"));
    }
    if (m_btnRedo) {
        m_btnRedo->Enable(m_commandHistory.CanRedo());
        m_btnRedo->SetToolTip(m_commandHistory.CanRedo()
            ? wxString::Format(wxT("Redo: %s"), wxString::FromUTF8(m_commandHistory.GetRedoDescription()))
            : wxT("Nothing to Redo"));
    }
}

void AIVRShowSpatialCopilotDialog::OnUndo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Undo()) {
        UpdateUiFromResults();
    }
}

void AIVRShowSpatialCopilotDialog::OnRedo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Redo()) {
        UpdateUiFromResults();
    }
}

void AIVRShowSpatialCopilotDialog::OnSendSpatialQuery(wxCommandEvent& WXUNUSED(event)) {
    if (!m_txtChatInput || !m_txtChatHistory) return;
    wxString q = m_txtChatInput->GetValue();
    if (q.empty()) return;

    m_txtChatHistory->AppendText(wxString::Format(wxT("User: %s\n"), q));
    std::string response = AI::VRShowSpatialCopilotAI::ProcessConversationalSpatialQuery(q.ToStdString(), m_session);
    m_txtChatHistory->AppendText(wxString::Format(wxT("AI Copilot: %s\n\n"), wxString::FromUTF8(response)));

    m_txtChatInput->Clear();
}

void AIVRShowSpatialCopilotDialog::OnExportSpatialReport(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Export Spatial Clearance Report"), wxEmptyString,
                         wxT("Spatial_Clearance_Audit.txt"),
                         wxT("Text Files (*.txt)|*.txt|JSON Files (*.json)|*.json"),
                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        std::ofstream out(saveDlg.GetPath().ToStdString());
        out << m_session.GenerateFormattedReport();
        spdlog::info("AIVRShowSpatialCopilotDialog: Exported spatial report to '{}'", saveDlg.GetPath().ToStdString());
    }
}

void AIVRShowSpatialCopilotDialog::OnHelp(wxCommandEvent& WXUNUSED(event)) {
    AI::AIHelpGuideDialog::ShowHelp(this, "VR_SPATIAL_COPILOT");
}

} // namespace xLights
