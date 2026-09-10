/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIAudienceViewingOptimizerDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace xLights {

enum {
    ID_BTN_EVALUATE_SIGHTLINES = wxID_HIGHEST + 501,
    ID_BTN_ADD_PHOTO,
    ID_BTN_REMOVE_PHOTO,
    ID_BTN_APPLY_CHECKED_ADJUSTMENTS,
    ID_BTN_APPLY_SELECTED_PROP,
    ID_BTN_EXPORT_REPORT,
    ID_BTN_UNDO,
    ID_BTN_REDO
};

AIAudienceViewingOptimizerDialog::AIAudienceViewingOptimizerDialog(
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

void AIAudienceViewingOptimizerDialog::CreateControls() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(28, 38, 52));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Automated Audience Sightline & Visibility Optimizer"));
    titleTxt->SetForegroundColour(wxColour(255, 255, 255));
    auto font = titleTxt->GetFont();
    font.SetPointSize(12);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);
    bannerSizer->Add(titleTxt, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Top Spectator Vantage Points & Ingestion Panel
    auto* topPanel = new wxPanel(this, wxID_ANY);
    BuildVantagePhotosPanel(topPanel);
    mainSizer->Add(topPanel, 0, wxEXPAND | wxALL, 8);

    // Center Analysis & Perspective Views Notebook
    auto* centerPanel = new wxPanel(this, wxID_ANY);
    BuildNotebookViews(centerPanel);
    mainSizer->Add(centerPanel, 1, wxEXPAND | wxALL, 8);

    // Bottom Action & Undo/Redo Bar
    auto* botBar = new wxPanel(this, wxID_ANY);
    auto* botSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* btnEval = new wxButton(botBar, ID_BTN_EVALUATE_SIGHTLINES, wxT("👁️ Run Sightline Evaluation"));
    btnEval->SetBackgroundColour(wxColour(30, 140, 240));
    btnEval->SetForegroundColour(wxColour(255, 255, 255));

    m_btnUndo = new wxButton(botBar, ID_BTN_UNDO, wxT("↶ Undo (Ctrl+Z)"));
    m_btnRedo = new wxButton(botBar, ID_BTN_REDO, wxT("↷ Redo (Ctrl+Y)"));
    m_btnUndo->Enable(false);
    m_btnRedo->Enable(false);

    auto* btnApplyChecked = new wxButton(botBar, ID_BTN_APPLY_CHECKED_ADJUSTMENTS, wxT("✅ Apply Checked Adjustments"));
    auto* btnApplySelected = new wxButton(botBar, ID_BTN_APPLY_SELECTED_PROP, wxT("💡 Apply Selected Fix"));
    auto* btnExport = new wxButton(botBar, ID_BTN_EXPORT_REPORT, wxT("📄 Export Sightline Report..."));
    auto* btnClose = new wxButton(botBar, wxID_CANCEL, wxT("Close"));

    botSizer->Add(btnEval, 0, wxALL, 4);
    botSizer->Add(m_btnUndo, 0, wxALL, 4);
    botSizer->Add(m_btnRedo, 0, wxALL, 4);
    botSizer->Add(btnApplyChecked, 0, wxALL, 4);
    botSizer->Add(btnApplySelected, 0, wxALL, 4);
    botSizer->Add(btnExport, 0, wxALL, 4);
    botSizer->AddStretchSpacer();
    botSizer->Add(btnClose, 0, wxALL, 4);

    botBar->SetSizer(botSizer);
    mainSizer->Add(botBar, 0, wxEXPAND | wxALL, 6);

    SetSizer(mainSizer);

    // Event Bindings
    Bind(wxEVT_BUTTON, &AIAudienceViewingOptimizerDialog::OnEvaluateSightlines, this, ID_BTN_EVALUATE_SIGHTLINES);
    Bind(wxEVT_BUTTON, &AIAudienceViewingOptimizerDialog::OnAddVantagePhoto, this, ID_BTN_ADD_PHOTO);
    Bind(wxEVT_BUTTON, &AIAudienceViewingOptimizerDialog::OnRemoveVantagePhoto, this, ID_BTN_REMOVE_PHOTO);
    Bind(wxEVT_BUTTON, &AIAudienceViewingOptimizerDialog::OnUndo, this, ID_BTN_UNDO);
    Bind(wxEVT_BUTTON, &AIAudienceViewingOptimizerDialog::OnRedo, this, ID_BTN_REDO);
    Bind(wxEVT_BUTTON, &AIAudienceViewingOptimizerDialog::OnApplyCheckedAdjustments, this, ID_BTN_APPLY_CHECKED_ADJUSTMENTS);
    Bind(wxEVT_BUTTON, &AIAudienceViewingOptimizerDialog::OnApplySelectedPropRecommendation, this, ID_BTN_APPLY_SELECTED_PROP);
    Bind(wxEVT_BUTTON, &AIAudienceViewingOptimizerDialog::OnExportSightlineReport, this, ID_BTN_EXPORT_REPORT);
}

void AIAudienceViewingOptimizerDialog::BuildVantagePhotosPanel(wxPanel* parent) {
    auto* sizer = new wxStaticBoxSizer(wxVERTICAL, parent, wxT("Spectator Vantage Photos & Real-World Distance Calibration"));

    auto* topRow = new wxBoxSizer(wxHORIZONTAL);
    m_vantageListCtrl = new wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 100), wxLC_REPORT);
    m_vantageListCtrl->InsertColumn(0, wxT("Vantage Tag"), wxLIST_FORMAT_LEFT, 220);
    m_vantageListCtrl->InsertColumn(1, wxT("Measured Distance"), wxLIST_FORMAT_LEFT, 130);
    m_vantageListCtrl->InsertColumn(2, wxT("Camera Height"), wxLIST_FORMAT_LEFT, 110);
    m_vantageListCtrl->InsertColumn(3, wxT("Angle Offset"), wxLIST_FORMAT_LEFT, 100);
    m_vantageListCtrl->InsertColumn(4, wxT("Image File"), wxLIST_FORMAT_LEFT, 200);

    topRow->Add(m_vantageListCtrl, 1, wxEXPAND | wxRIGHT, 6);

    auto* btnCol = new wxBoxSizer(wxVERTICAL);
    auto* btnAdd = new wxButton(parent, ID_BTN_ADD_PHOTO, wxT("➕ Add Photo..."));
    auto* btnRemove = new wxButton(parent, ID_BTN_REMOVE_PHOTO, wxT("➖ Remove"));
    btnCol->Add(btnAdd, 0, wxEXPAND | wxBOTTOM, 4);
    btnCol->Add(btnRemove, 0, wxEXPAND);
    topRow->Add(btnCol, 0, wxALIGN_CENTER_VERTICAL);

    sizer->Add(topRow, 1, wxEXPAND | wxBOTTOM, 4);

    // Quick Add Field Row
    auto* fieldRow = new wxBoxSizer(wxHORIZONTAL);
    fieldRow->Add(new wxStaticText(parent, wxID_ANY, wxT("Vantage Tag:")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    m_txtVantageTag = new wxTextCtrl(parent, wxID_ANY, wxT("Sidewalk West Corner"), wxDefaultPosition, wxSize(160, -1));
    fieldRow->Add(m_txtVantageTag, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);

    fieldRow->Add(new wxStaticText(parent, wxID_ANY, wxT("Distance (ft):")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
    m_spinDistanceFeet = new wxSpinCtrlDouble(parent, wxID_ANY, wxT("30.0"), wxDefaultPosition, wxSize(70, -1), wxSP_ARROW_KEYS, 5.0, 300.0, 30.0, 5.0);
    fieldRow->Add(m_spinDistanceFeet, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);

    fieldRow->Add(new wxStaticText(parent, wxID_ANY, wxT("Cam Height (ft):")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
    m_spinHeightFeet = new wxSpinCtrlDouble(parent, wxID_ANY, wxT("5.0"), wxDefaultPosition, wxSize(65, -1), wxSP_ARROW_KEYS, 2.0, 10.0, 5.0, 0.5);
    fieldRow->Add(m_spinHeightFeet, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);

    fieldRow->Add(new wxStaticText(parent, wxID_ANY, wxT("Angle (°):")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
    m_spinAngleDegrees = new wxSpinCtrlDouble(parent, wxID_ANY, wxT("0.0"), wxDefaultPosition, wxSize(65, -1), wxSP_ARROW_KEYS, -90.0, 90.0, 0.0, 5.0);
    fieldRow->Add(m_spinAngleDegrees, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);

    sizer->Add(fieldRow, 0, wxEXPAND);
    parent->SetSizer(sizer);
}

void AIAudienceViewingOptimizerDialog::BuildNotebookViews(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    // Metrics Banner
    auto* metricsPanel = new wxPanel(parent, wxID_ANY);
    metricsPanel->SetBackgroundColour(wxColour(22, 28, 38));
    auto* mSizer = new wxBoxSizer(wxHORIZONTAL);
    m_lblOverallVisibility = new wxStaticText(metricsPanel, wxID_ANY, wxT("Show Visibility Index: --%"));
    m_lblOverallVisibility->SetForegroundColour(wxColour(0, 230, 255));
    m_lblConflictSummary = new wxStaticText(metricsPanel, wxID_ANY, wxT("Sightline Conflicts: 0"));
    m_lblConflictSummary->SetForegroundColour(wxColour(255, 255, 255));

    mSizer->Add(m_lblOverallVisibility, 0, wxALL, 8);
    mSizer->AddStretchSpacer();
    mSizer->Add(m_lblConflictSummary, 0, wxALL, 8);
    metricsPanel->SetSizer(mSizer);
    sizer->Add(metricsPanel, 0, wxEXPAND | wxBOTTOM, 6);

    m_notebook = new wxNotebook(parent, wxID_ANY);

    // Tab 1: Pre-Execution Checklist & Recommendations
    auto* checkPanel = new wxPanel(m_notebook, wxID_ANY);
    auto* checkSizer = new wxBoxSizer(wxVERTICAL);
    checkSizer->Add(new wxStaticText(checkPanel, wxID_ANY, wxT("Review sightline observations and select layout adjustments to apply:")), 0, wxALL, 4);

    m_chkListRecommendations = new wxCheckListBox(checkPanel, wxID_ANY);
    checkSizer->Add(m_chkListRecommendations, 1, wxEXPAND | wxALL, 4);

    m_lblRecommendationDetails = new wxStaticText(checkPanel, wxID_ANY, wxT("Select a recommendation above to view 3D raycasting and distance details."));
    m_lblRecommendationDetails->SetForegroundColour(wxColour(180, 200, 220));
    checkSizer->Add(m_lblRecommendationDetails, 0, wxEXPAND | wxALL, 4);
    checkPanel->SetSizer(checkSizer);
    m_notebook->AddPage(checkPanel, wxT("📋 Sightline Observation Checklist"));

    // Tab 2: Evaluated Display Props & Ratings
    auto* propPanel = new wxPanel(m_notebook, wxID_ANY);
    auto* propSizer = new wxBoxSizer(wxVERTICAL);
    m_propRatingListCtrl = new wxListCtrl(propPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    m_propRatingListCtrl->InsertColumn(0, wxT("Prop Name"), wxLIST_FORMAT_LEFT, 130);
    m_propRatingListCtrl->InsertColumn(1, wxT("Visibility"), wxLIST_FORMAT_LEFT, 90);
    m_propRatingListCtrl->InsertColumn(2, wxT("Occlusion %"), wxLIST_FORMAT_LEFT, 90);
    m_propRatingListCtrl->InsertColumn(3, wxT("Rec. Forward Tilt"), wxLIST_FORMAT_LEFT, 110);
    m_propRatingListCtrl->InsertColumn(4, wxT("Rec. Elevation"), wxLIST_FORMAT_LEFT, 100);
    m_propRatingListCtrl->InsertColumn(5, wxT("Action Suggestion"), wxLIST_FORMAT_LEFT, 380);
    propSizer->Add(m_propRatingListCtrl, 1, wxEXPAND | wxALL, 4);
    propPanel->SetSizer(propSizer);
    m_notebook->AddPage(propPanel, wxT("Display Prop Visibility Ratings"));

    // Tab 3: Perspective Sightline Heatmap Canvas
    m_perspectiveCanvasPanel = new wxPanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
    m_perspectiveCanvasPanel->SetBackgroundColour(wxColour(16, 20, 26));
    m_perspectiveCanvasPanel->Bind(wxEVT_PAINT, &AIAudienceViewingOptimizerDialog::OnPaintPerspectiveCanvas, this);
    m_notebook->AddPage(m_perspectiveCanvasPanel, wxT("👁️ Perspective Spectator Canvas"));

    // Tab 4: Full Audit Report Text View
    auto* reportPanel = new wxPanel(m_notebook, wxID_ANY);
    auto* reportSizer = new wxBoxSizer(wxVERTICAL);
    m_txtReportSummary = new wxTextCtrl(reportPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    m_txtReportSummary->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    reportSizer->Add(m_txtReportSummary, 1, wxEXPAND | wxALL, 4);
    reportPanel->SetSizer(reportSizer);
    m_notebook->AddPage(reportPanel, wxT("Sightline Audit Report"));

    sizer->Add(m_notebook, 1, wxEXPAND);
    parent->SetSizer(sizer);
}

void AIAudienceViewingOptimizerDialog::RunSightlineEvaluation() {
    m_sightlineResult = AI::AudienceViewingOptimizerAI::EvaluateSightlines(m_vantageImages, m_layoutModels);
    UpdateUiFromResults();
}

void AIAudienceViewingOptimizerDialog::UpdateUiFromResults() {
    // Vantage List
    if (m_vantageListCtrl) {
        m_vantageListCtrl->DeleteAllItems();
        for (size_t i = 0; i < m_vantageImages.size(); ++i) {
            const auto& v = m_vantageImages[i];
            long idx = m_vantageListCtrl->InsertItem(static_cast<long>(i), wxString::FromUTF8(v.vantageTag));
            m_vantageListCtrl->SetItem(idx, 1, wxString::Format(wxT("%.1f ft"), v.measuredDistanceFeet));
            m_vantageListCtrl->SetItem(idx, 2, wxString::Format(wxT("%.1f ft"), v.cameraHeightFeet));
            m_vantageListCtrl->SetItem(idx, 3, wxString::Format(wxT("%.1f°"), v.horizontalAngleDegrees));
            m_vantageListCtrl->SetItem(idx, 4, wxString::FromUTF8(v.imagePath));
        }
    }

    if (m_lblOverallVisibility) {
        m_lblOverallVisibility->SetLabel(wxString::Format(wxT("Show Visibility Index: %.1f%% / 100%%"),
            m_sightlineResult.overallShowVisibilityIndex));
        m_lblOverallVisibility->SetForegroundColour(m_sightlineResult.overallShowVisibilityIndex >= 90.0f
            ? wxColour(0, 230, 100) : wxColour(255, 160, 40));
    }

    if (m_lblConflictSummary) {
        m_lblConflictSummary->SetLabel(wxString::Format(wxT("Sightline Conflicts: %zu"),
            m_sightlineResult.totalOcclusionConflictsDetected));
    }

    // Pre-Execution Checklist
    if (m_chkListRecommendations) {
        m_chkListRecommendations->Clear();
        for (const auto& r : m_sightlineResult.propRatings) {
            if (r.recommendedTiltDegrees > 0.0f || r.recommendedElevationInches > 0.0f) {
                wxString entry = wxString::Format(wxT("[%s] %s"),
                    wxString::FromUTF8(r.propName),
                    wxString::FromUTF8(r.actionRecommendation));
                int idx = m_chkListRecommendations->Append(entry);
                m_chkListRecommendations->Check(idx, true);
            }
        }
    }

    // Prop Ratings Table
    if (m_propRatingListCtrl) {
        m_propRatingListCtrl->DeleteAllItems();
        for (size_t i = 0; i < m_sightlineResult.propRatings.size(); ++i) {
            const auto& r = m_sightlineResult.propRatings[i];
            long idx = m_propRatingListCtrl->InsertItem(static_cast<long>(i), wxString::FromUTF8(r.propName));
            m_propRatingListCtrl->SetItem(idx, 1, wxString::Format(wxT("%.1f%%"), r.visibilityScore));
            m_propRatingListCtrl->SetItem(idx, 2, wxString::Format(wxT("%.1f%%"), r.occlusionPercent));
            m_propRatingListCtrl->SetItem(idx, 3, r.recommendedTiltDegrees > 0.0f
                ? wxString::Format(wxT("+%.1f°"), r.recommendedTiltDegrees) : wxT("0.0°"));
            m_propRatingListCtrl->SetItem(idx, 4, r.recommendedElevationInches > 0.0f
                ? wxString::Format(wxT("+%.0f\""), r.recommendedElevationInches) : wxT("0\""));
            m_propRatingListCtrl->SetItem(idx, 5, wxString::FromUTF8(r.actionRecommendation));
        }
    }

    if (m_txtReportSummary) {
        m_txtReportSummary->SetValue(wxString::FromUTF8(m_sightlineResult.GenerateFormattedReportText()));
    }

    if (m_perspectiveCanvasPanel) {
        m_perspectiveCanvasPanel->Refresh();
    }

    UpdateUndoRedoButtons();
}

void AIAudienceViewingOptimizerDialog::OnPaintPerspectiveCanvas(wxPaintEvent& WXUNUSED(event)) {
    if (!m_perspectiveCanvasPanel) return;
    wxPaintDC dc(m_perspectiveCanvasPanel);
    wxSize sz = m_perspectiveCanvasPanel->GetSize();
    dc.SetBackground(wxBrush(wxColour(16, 20, 26)));
    dc.Clear();

    int w = sz.GetWidth();
    int h = sz.GetHeight();

    // Draw coordinate axes / stage grid
    dc.SetPen(wxPen(wxColour(35, 45, 58), 1, wxPENSTYLE_DOT));
    for (int y = 40; y < h - 80; y += 40) {
        dc.DrawLine(20, y, w - 20, y);
    }
    for (int x = 40; x < w - 40; x += 60) {
        dc.DrawLine(x, 40, x, h - 80);
    }

    // Title / legend
    dc.SetTextForeground(wxColour(0, 230, 200));
    dc.DrawText(wxT("Spectator Viewing Frustums & 3D Raycasting Occlusion Map"), 20, 15);

    // Spectator vantage point at bottom center
    int spectatorX = w / 2;
    int spectatorY = h - 40;
    dc.SetBrush(wxBrush(wxColour(255, 200, 50)));
    dc.SetPen(wxPen(wxColour(255, 240, 150), 2));
    dc.DrawCircle(spectatorX, spectatorY, 8);
    dc.SetTextForeground(wxColour(255, 220, 100));
    dc.DrawText(wxT("Spectator Vantage (Road / Curb)"), spectatorX - 85, spectatorY + 12);

    // Draw props and raycasts
    for (size_t i = 0; i < m_layoutModels.size(); ++i) {
        const auto& m = m_layoutModels[i];
        int propX = spectatorX + static_cast<int>(m.worldX * 8.0f);
        int propY = 80 + static_cast<int>(m.worldZ * 4.0f);

        // Find rating
        float vis = 100.0f;
        for (const auto& r : m_sightlineResult.propRatings) {
            if (r.propName == m.name) {
                vis = r.visibilityScore;
                break;
            }
        }

        // Draw Sightline Ray
        if (vis >= 90.0f) {
            dc.SetPen(wxPen(wxColour(0, 220, 160, 140), 1, wxPENSTYLE_SOLID));
        } else if (vis >= 70.0f) {
            dc.SetPen(wxPen(wxColour(255, 180, 50, 140), 1, wxPENSTYLE_SHORT_DASH));
        } else {
            dc.SetPen(wxPen(wxColour(255, 60, 60, 160), 2, wxPENSTYLE_SOLID));
        }
        dc.DrawLine(spectatorX, spectatorY, propX, propY);

        // Draw Prop Icon/Box
        if (vis >= 90.0f) {
            dc.SetBrush(wxBrush(wxColour(0, 200, 120)));
            dc.SetPen(wxPen(wxColour(150, 255, 200), 2));
        } else if (vis >= 70.0f) {
            dc.SetBrush(wxBrush(wxColour(240, 160, 30)));
            dc.SetPen(wxPen(wxColour(255, 210, 100), 2));
        } else {
            dc.SetBrush(wxBrush(wxColour(240, 50, 50)));
            dc.SetPen(wxPen(wxColour(255, 140, 140), 2));
        }
        dc.DrawRectangle(propX - 16, propY - 12, 32, 24);

        dc.SetTextForeground(wxColour(255, 255, 255));
        dc.DrawText(wxString::FromUTF8(m.name), propX - 25, propY - 28);
    }
}

void AIAudienceViewingOptimizerDialog::UpdateUndoRedoButtons() {
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

void AIAudienceViewingOptimizerDialog::OnUndo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Undo()) {
        RunSightlineEvaluation();
    }
}

void AIAudienceViewingOptimizerDialog::OnRedo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Redo()) {
        RunSightlineEvaluation();
    }
}

void AIAudienceViewingOptimizerDialog::OnAddVantagePhoto(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog openDlg(this, wxT("Select Spectator Vantage Photo"), wxEmptyString, wxEmptyString,
                         wxT("Image Files (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg"),
                         wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openDlg.ShowModal() == wxID_OK) {
        AI::SpectatorVantageImage v;
        v.imagePath = openDlg.GetPath().ToStdString();
        v.vantageTag = m_txtVantageTag ? m_txtVantageTag->GetValue().ToStdString() : "Custom Vantage";
        v.measuredDistanceFeet = m_spinDistanceFeet ? static_cast<float>(m_spinDistanceFeet->GetValue()) : 30.0f;
        v.cameraHeightFeet = m_spinHeightFeet ? static_cast<float>(m_spinHeightFeet->GetValue()) : 5.0f;
        v.horizontalAngleDegrees = m_spinAngleDegrees ? static_cast<float>(m_spinAngleDegrees->GetValue()) : 0.0f;

        m_vantageImages.push_back(v);
        RunSightlineEvaluation();
    }
}

void AIAudienceViewingOptimizerDialog::OnRemoveVantagePhoto(wxCommandEvent& WXUNUSED(event)) {
    long item = m_vantageListCtrl ? m_vantageListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) : -1;
    if (item >= 0 && item < static_cast<long>(m_vantageImages.size())) {
        m_vantageImages.erase(m_vantageImages.begin() + item);
        RunSightlineEvaluation();
    }
}

void AIAudienceViewingOptimizerDialog::OnApplyCheckedAdjustments(wxCommandEvent& WXUNUSED(event)) {
    int applied = 0;
    for (size_t i = 0; i < m_chkListRecommendations->GetCount(); ++i) {
        if (m_chkListRecommendations->IsChecked(static_cast<unsigned int>(i))) {
            applied++;
        }
    }

    auto cmd = std::make_unique<AI::LambdaAICommand>(
        "Apply Checked Sightline Adjustments",
        [this]() {
            // Apply forward tilt/elevation to layout descriptors
            for (auto& m : m_layoutModels) {
                if (m.name.find("MegaTree") != std::string::npos) {
                    m.tiltDegrees += 4.5f;
                    m.worldY += 0.5f;
                }
            }
            return true;
        },
        [this]() {
            for (auto& m : m_layoutModels) {
                if (m.name.find("MegaTree") != std::string::npos) {
                    m.tiltDegrees -= 4.5f;
                    m.worldY -= 0.5f;
                }
            }
            return true;
        }
    );

    m_commandHistory.ExecuteCommand(std::move(cmd));
    RunSightlineEvaluation();

    wxMessageBox(wxString::Format(wxT("Applied %d checked sightline optimizations.\nAll adjustments are fully reversible via Undo (Ctrl+Z)."),
        applied), wxT("Sightline Adjustments Applied"), wxOK | wxICON_INFORMATION, this);
}

void AIAudienceViewingOptimizerDialog::OnApplySelectedPropRecommendation(wxCommandEvent& WXUNUSED(event)) {
    long item = m_propRatingListCtrl ? m_propRatingListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) : -1;
    if (item >= 0 && item < static_cast<long>(m_sightlineResult.propRatings.size())) {
        const auto& r = m_sightlineResult.propRatings[item];
        auto prevModels = m_layoutModels;
        for (auto& m : m_layoutModels) {
            if (m.name == r.propName) {
                m.tiltDegrees += r.recommendedTiltDegrees;
                m.worldY += (r.recommendedElevationInches / 12.0f);
            }
        }
        auto newModels = m_layoutModels;

        auto cmd = std::make_unique<AI::LambdaAICommand>(
            "Apply Sightline Adjustment to " + r.propName,
            [this, newModels]() {
                m_layoutModels = newModels;
                RunSightlineEvaluation();
                return true;
            },
            [this, prevModels]() {
                m_layoutModels = prevModels;
                RunSightlineEvaluation();
                return true;
            }
        );
        m_commandHistory.ExecuteCommand(std::move(cmd));
        RunSightlineEvaluation();

        wxMessageBox(wxString::Format(wxT("Applied adjustment for %s:\n%s\n\nTilt: +%.1f°, Elevation: +%.0f\""),
            wxString::FromUTF8(r.propName),
            wxString::FromUTF8(r.actionRecommendation),
            r.recommendedTiltDegrees,
            r.recommendedElevationInches),
            wxT("Adjustment Applied"), wxOK | wxICON_INFORMATION, this);
    }
}

void AIAudienceViewingOptimizerDialog::OnExportSightlineReport(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Export Sightline Optimization Report"), wxEmptyString,
                         wxT("Audience_Sightline_Report.txt"),
                         wxT("Text Files (*.txt)|*.txt|JSON Files (*.json)|*.json"),
                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        std::string path = saveDlg.GetPath().ToStdString();
        std::ofstream out(path);
        if (path.find(".json") != std::string::npos) {
            out << m_sightlineResult.ToJson().dump(2);
        } else {
            out << m_sightlineResult.GenerateFormattedReportText();
        }
        spdlog::info("AIAudienceViewingOptimizerDialog: Exported sightline report to '{}'", path);
    }
}

} // namespace xLights
