/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIPixelAutoHealingDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace xLights {

enum {
    ID_BTN_RUN_DIAGNOSIS = wxID_HIGHEST + 801,
    ID_CHK_LIVE_REMAP,
    ID_BTN_EXPORT_PROFILE,
    ID_BTN_UNDO,
    ID_BTN_REDO
};

AIPixelAutoHealingDialog::AIPixelAutoHealingDialog(
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

void AIPixelAutoHealingDialog::CreateControls() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(28, 44, 48));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Computer Vision Dead Pixel Auto-Healer & Spatial Remapper"));
    titleTxt->SetForegroundColour(wxColour(255, 255, 255));
    auto font = titleTxt->GetFont();
    font.SetPointSize(12);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);
    bannerSizer->Add(titleTxt, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Top Setup Panel
    auto* topPanel = new wxPanel(this, wxID_ANY);
    BuildTopSetupPanel(topPanel);
    mainSizer->Add(topPanel, 0, wxEXPAND | wxALL, 8);

    // Center Notebook
    auto* centerPanel = new wxPanel(this, wxID_ANY);
    BuildCenterNotebook(centerPanel);
    mainSizer->Add(centerPanel, 1, wxEXPAND | wxALL, 8);

    // Bottom Action Bar
    auto* botBar = new wxPanel(this, wxID_ANY);
    auto* botSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* btnDiag = new wxButton(botBar, ID_BTN_RUN_DIAGNOSIS, wxT("🔍 Run Vision Diagnosis"));
    btnDiag->SetBackgroundColour(wxColour(30, 160, 180));
    btnDiag->SetForegroundColour(wxColour(255, 255, 255));

    m_btnUndo = new wxButton(botBar, ID_BTN_UNDO, wxT("↶ Undo (Ctrl+Z)"));
    m_btnRedo = new wxButton(botBar, ID_BTN_REDO, wxT("↷ Redo (Ctrl+Y)"));
    m_btnUndo->Enable(false);
    m_btnRedo->Enable(false);

    auto* btnExport = new wxButton(botBar, ID_BTN_EXPORT_PROFILE, wxT("📄 Export Healing Profile..."));
    auto* btnClose = new wxButton(botBar, wxID_CANCEL, wxT("Close"));

    botSizer->Add(btnDiag, 0, wxALL, 4);
    botSizer->Add(m_btnUndo, 0, wxALL, 4);
    botSizer->Add(m_btnRedo, 0, wxALL, 4);
    botSizer->Add(btnExport, 0, wxALL, 4);
    botSizer->AddStretchSpacer();
    botSizer->Add(btnClose, 0, wxALL, 4);

    botBar->SetSizer(botSizer);
    mainSizer->Add(botBar, 0, wxEXPAND | wxALL, 6);

    SetSizer(mainSizer);

    // Event Bindings
    Bind(wxEVT_BUTTON, &AIPixelAutoHealingDialog::OnRunVisionDiagnosis, this, ID_BTN_RUN_DIAGNOSIS);
    Bind(wxEVT_CHECKBOX, &AIPixelAutoHealingDialog::OnToggleLiveRemap, this, ID_CHK_LIVE_REMAP);
    Bind(wxEVT_BUTTON, &AIPixelAutoHealingDialog::OnExportHealingProfile, this, ID_BTN_EXPORT_PROFILE);
    Bind(wxEVT_BUTTON, &AIPixelAutoHealingDialog::OnUndo, this, ID_BTN_UNDO);
    Bind(wxEVT_BUTTON, &AIPixelAutoHealingDialog::OnRedo, this, ID_BTN_REDO);
}

void AIPixelAutoHealingDialog::BuildTopSetupPanel(wxPanel* parent) {
    auto* sizer = new wxStaticBoxSizer(wxHORIZONTAL, parent, wxT("Calibration & Real-Time Remapping Control"));

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Target Display Prop:")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    wxArrayString props;
    props.Add(wxT("MegaTree (800 Nodes)"));
    props.Add(wxT("Matrix Panel (1200 Nodes)"));
    props.Add(wxT("Roof Arches (300 Nodes)"));
    m_choiceProps = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxSize(200, -1), props);
    m_choiceProps->SetSelection(0);
    sizer->Add(m_choiceProps, 0, wxALIGN_CENTER_VERTICAL | wxALL, 4);

    m_chkEnableLiveRemap = new wxCheckBox(parent, ID_CHK_LIVE_REMAP, wxT("Enable Real-Time Show Output Laplacian Remapping"));
    m_chkEnableLiveRemap->SetForegroundColour(wxColour(0, 230, 200));
    m_chkEnableLiveRemap->SetValue(true);
    sizer->Add(m_chkEnableLiveRemap, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 16);

    parent->SetSizer(sizer);
}

void AIPixelAutoHealingDialog::BuildCenterNotebook(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    // Status Banner
    auto* metricsPanel = new wxPanel(parent, wxID_ANY);
    metricsPanel->SetBackgroundColour(wxColour(20, 30, 36));
    auto* mSizer = new wxBoxSizer(wxHORIZONTAL);
    m_lblStatusSummary = new wxStaticText(metricsPanel, wxID_ANY, wxT("Status: Diagnosing..."));
    m_lblStatusSummary->SetForegroundColour(wxColour(0, 230, 200));
    m_lblQualityScore = new wxStaticText(metricsPanel, wxID_ANY, wxT("Visual Quality: 100.0%"));
    m_lblQualityScore->SetForegroundColour(wxColour(255, 255, 255));
    mSizer->Add(m_lblStatusSummary, 0, wxALL, 8);
    mSizer->AddStretchSpacer();
    mSizer->Add(m_lblQualityScore, 0, wxALL, 8);
    metricsPanel->SetSizer(mSizer);
    sizer->Add(metricsPanel, 0, wxEXPAND | wxBOTTOM, 6);

    auto* notebook = new wxNotebook(parent, wxID_ANY);

    // Tab 1: Diagnosed Faults List
    auto* faultPanel = new wxPanel(notebook, wxID_ANY);
    auto* faultSizer = new wxBoxSizer(wxVERTICAL);
    m_faultListCtrl = new wxListCtrl(faultPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    m_faultListCtrl->InsertColumn(0, wxT("Node #"), wxLIST_FORMAT_LEFT, 80);
    m_faultListCtrl->InsertColumn(1, wxT("Strand"), wxLIST_FORMAT_LEFT, 80);
    m_faultListCtrl->InsertColumn(2, wxT("Fault Classification"), wxLIST_FORMAT_LEFT, 180);
    m_faultListCtrl->InsertColumn(3, wxT("Vision Confidence"), wxLIST_FORMAT_LEFT, 130);
    m_faultListCtrl->InsertColumn(4, wxT("Healing Interpolation Neighbors"), wxLIST_FORMAT_LEFT, 320);
    faultSizer->Add(m_faultListCtrl, 1, wxEXPAND | wxALL, 4);
    faultPanel->SetSizer(faultSizer);
    notebook->AddPage(faultPanel, wxT("🩺 Diagnosed Dead Pixels"));

    // Tab 2: Visual Comparison Canvas
    m_visualComparisonPanel = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
    m_visualComparisonPanel->SetBackgroundColour(wxColour(14, 20, 24));
    m_visualComparisonPanel->Bind(wxEVT_PAINT, &AIPixelAutoHealingDialog::OnPaintComparisonCanvas, this);
    notebook->AddPage(m_visualComparisonPanel, wxT("👁️ Live Healed vs Unhealed Canvas"));

    // Tab 3: Full Audit Report Text View
    auto* reportPanel = new wxPanel(notebook, wxID_ANY);
    auto* reportSizer = new wxBoxSizer(wxVERTICAL);
    m_txtReportSummary = new wxTextCtrl(reportPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    m_txtReportSummary->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    reportSizer->Add(m_txtReportSummary, 1, wxEXPAND | wxALL, 4);
    reportPanel->SetSizer(reportSizer);
    notebook->AddPage(reportPanel, wxT("Healing Audit Report"));

    sizer->Add(notebook, 1, wxEXPAND);
    parent->SetSizer(sizer);
}

void AIPixelAutoHealingDialog::OnPaintComparisonCanvas(wxPaintEvent& WXUNUSED(event)) {
    if (!m_visualComparisonPanel) return;
    wxPaintDC dc(m_visualComparisonPanel);
    wxSize sz = m_visualComparisonPanel->GetSize();
    dc.SetBackground(wxBrush(wxColour(14, 20, 24)));
    dc.Clear();

    int halfW = sz.GetWidth() / 2;
    int h = sz.GetHeight();

    // Dividing separator line
    dc.SetPen(wxPen(wxColour(40, 60, 80), 2, wxPENSTYLE_DOT));
    dc.DrawLine(halfW, 10, halfW, h - 10);

    // Headers
    dc.SetTextForeground(wxColour(255, 100, 100));
    dc.DrawText(wxT("🔴 Unhealed Live Matrix (Faulty Nodes Black/Off)"), 20, 15);
    dc.SetTextForeground(wxColour(0, 230, 200));
    dc.DrawText(wxT("🟢 AI Laplacian Blended Output (Healed Photometry)"), halfW + 20, 15);

    // Render 16x16 simulated LED pixel grid on both sides
    int cols = 16;
    int rows = 16;
    int cellSize = std::min((halfW - 60) / cols, (h - 70) / rows);
    int topOffset = 45;

    std::unordered_set<int> deadSet;
    for (const auto& f : m_calibration.diagnosedFaults) {
        deadSet.insert(f.nodeIndex % (cols * rows));
    }

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int nodeIdx = r * cols + c;
            bool isDead = deadSet.find(nodeIdx) != deadSet.end();

            // Left (Unhealed)
            int leftX = 30 + c * cellSize;
            int leftY = topOffset + r * cellSize;
            if (isDead) {
                dc.SetBrush(wxBrush(wxColour(240, 40, 40)));
                dc.SetPen(wxPen(wxColour(255, 100, 100), 2));
            } else {
                dc.SetBrush(wxBrush(wxColour(30, 140, 220)));
                dc.SetPen(wxPen(wxColour(60, 180, 255), 1));
            }
            dc.DrawCircle(leftX + cellSize / 2, leftY + cellSize / 2, cellSize / 3);

            // Right (Healed)
            int rightX = halfW + 30 + c * cellSize;
            int rightY = topOffset + r * cellSize;
            if (isDead) {
                dc.SetBrush(wxBrush(wxColour(0, 240, 200)));
                dc.SetPen(wxPen(wxColour(180, 255, 240), 2));
            } else {
                dc.SetBrush(wxBrush(wxColour(30, 140, 220)));
                dc.SetPen(wxPen(wxColour(60, 180, 255), 1));
            }
            dc.DrawCircle(rightX + cellSize / 2, rightY + cellSize / 2, cellSize / 3);
        }
    }
}

void AIPixelAutoHealingDialog::OnRunVisionDiagnosis(wxCommandEvent& WXUNUSED(event)) {
    m_selectedPropName = (m_choiceProps ? m_choiceProps->GetStringSelection().ToStdString() : "MegaTree");
    auto prev = m_calibration;
    m_calibration = AI::PixelAutoHealingAI::DiagnoseAndComputeHealing(m_selectedPropName, 800);

    auto cmd = std::make_unique<AI::LambdaAICommand>(
        "Diagnose Pixel Faults on " + m_selectedPropName,
        [this]() { return true; },
        [this, prev]() {
            m_calibration = prev;
            UpdateUiFromResults();
            return true;
        }
    );

    m_commandHistory.ExecuteCommand(std::move(cmd));
    UpdateUiFromResults();
}

void AIPixelAutoHealingDialog::UpdateUiFromResults() {
    if (m_lblStatusSummary) {
        m_lblStatusSummary->SetLabel(wxString::Format(wxT("Status: %zu Faulty Nodes Detected on '%s'"),
            m_calibration.deadPixelsDetected, wxString::FromUTF8(m_calibration.propName)));
    }

    if (m_lblQualityScore) {
        m_lblQualityScore->SetLabel(wxString::Format(wxT("Quality Score: %.1f%% Recovery"),
            m_calibration.showQualityRecoveryScore));
    }

    if (m_faultListCtrl) {
        m_faultListCtrl->DeleteAllItems();
        for (size_t i = 0; i < m_calibration.diagnosedFaults.size(); ++i) {
            const auto& f = m_calibration.diagnosedFaults[i];
            long idx = m_faultListCtrl->InsertItem(static_cast<long>(i), wxString::Format(wxT("#%d"), f.nodeIndex));
            m_faultListCtrl->SetItem(idx, 1, wxString::Format(wxT("%d"), f.strandIndex));
            m_faultListCtrl->SetItem(idx, 2, f.faultType == AI::PixelFaultType::DEAD_DARK ? wxT("DEAD (NO LIGHT)") : wxT("SIGNAL FAULT"));
            m_faultListCtrl->SetItem(idx, 3, wxString::Format(wxT("%.0f%%"), f.confidenceScore * 100.0f));

            wxString neighbors = wxT("Nodes: [ ");
            for (size_t k = 0; k < f.neighborNodeIndices.size(); ++k) {
                neighbors += wxString::Format(wxT("#%d (%.0f%%) "), f.neighborNodeIndices[k], f.neighborWeightFactors[k] * 100.0f);
            }
            neighbors += wxT("]");
            m_faultListCtrl->SetItem(idx, 4, neighbors);
        }
    }

    if (m_txtReportSummary) {
        m_txtReportSummary->SetValue(wxString::FromUTF8(m_calibration.GenerateFormattedReport()));
    }

    if (m_visualComparisonPanel) {
        m_visualComparisonPanel->Refresh();
    }

    UpdateUndoRedoButtons();
}

void AIPixelAutoHealingDialog::UpdateUndoRedoButtons() {
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

void AIPixelAutoHealingDialog::OnUndo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Undo()) {
        UpdateUiFromResults();
    }
}

void AIPixelAutoHealingDialog::OnRedo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Redo()) {
        UpdateUiFromResults();
    }
}

void AIPixelAutoHealingDialog::OnToggleLiveRemap(wxCommandEvent& WXUNUSED(event)) {
    spdlog::info("AIPixelAutoHealingDialog: Live remapping set to {}", m_chkEnableLiveRemap->IsChecked());
}

void AIPixelAutoHealingDialog::OnExportHealingProfile(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Export Dead Pixel Healing Profile"), wxEmptyString,
                         wxT("Pixel_Healing_Profile.json"),
                         wxT("JSON Files (*.json)|*.json|Text Files (*.txt)|*.txt"),
                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        std::ofstream out(saveDlg.GetPath().ToStdString());
        out << m_calibration.ToJson().dump(2);
        spdlog::info("AIPixelAutoHealingDialog: Exported healing profile to '{}'", saveDlg.GetPath().ToStdString());
    }
}

} // namespace xLights
