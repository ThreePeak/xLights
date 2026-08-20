/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIPhoto3DPropReconstructorDialog.h"
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/statline.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace xLights {

enum {
    ID_BTN_RECONSTRUCT = wxID_HIGHEST + 301,
    ID_BTN_UNDO,
    ID_BTN_REDO,
    ID_BTN_EVENLY_SPACE,
    ID_BTN_STRAIGHTEN,
    ID_BTN_SNAP_GRID,
    ID_BTN_SYMMETRIZE,
    ID_BTN_REVERSE_WIRING,
    ID_BTN_AUTO_SUBMODELS,
    ID_BTN_SAVE,
    ID_BTN_SAVE_AS,
    ID_BTN_SAVE_AS_COPY,
    ID_BTN_DUPLICATE,
    ID_BTN_EXPORT_AS,
    ID_BTN_COPY_CLIPBOARD
};

AIPhoto3DPropReconstructorDialog::AIPhoto3DPropReconstructorDialog(
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

void AIPhoto3DPropReconstructorDialog::CreateControls() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Top Header & Mode Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(24, 30, 42));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI 3D Photo Prop Mesh Reconstructor (Single & Multi-Angle Photogrammetry)"));
    titleTxt->SetForegroundColour(wxColour(255, 255, 255));
    auto font = titleTxt->GetFont();
    font.SetPointSize(12);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);
    bannerSizer->Add(titleTxt, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Center Workspace (Left Controls + Right Canvas/Data)
    auto* workSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* leftPanel = new wxPanel(this, wxID_ANY);
    BuildLeftControlsPanel(leftPanel);
    workSizer->Add(leftPanel, 0, wxEXPAND | wxALL, 8);

    auto* rightPanel = new wxPanel(this, wxID_ANY);
    BuildRightCanvasPanel(rightPanel);
    workSizer->Add(rightPanel, 1, wxEXPAND | wxALL, 8);

    mainSizer->Add(workSizer, 1, wxEXPAND);

    // Bottom Action & File Management Bar with Undo/Redo
    auto* botBar = new wxPanel(this, wxID_ANY);
    auto* botSizer = new wxBoxSizer(wxHORIZONTAL);

    m_btnUndo = new wxButton(botBar, ID_BTN_UNDO, wxT("↶ Undo (Ctrl+Z)"));
    m_btnRedo = new wxButton(botBar, ID_BTN_REDO, wxT("↷ Redo (Ctrl+Y)"));
    m_btnUndo->Enable(false);
    m_btnRedo->Enable(false);

    auto* btnSave = new wxButton(botBar, ID_BTN_SAVE, wxT("💾 Save"));
    auto* btnSaveAs = new wxButton(botBar, ID_BTN_SAVE_AS, wxT("Save As..."));
    auto* btnSaveAsCopy = new wxButton(botBar, ID_BTN_SAVE_AS_COPY, wxT("Save as Copy..."));
    auto* btnDuplicate = new wxButton(botBar, ID_BTN_DUPLICATE, wxT("Duplicate"));
    auto* btnExportAs = new wxButton(botBar, ID_BTN_EXPORT_AS, wxT("Export As (.xmodel / .obj)..."));
    auto* btnCopyClip = new wxButton(botBar, ID_BTN_COPY_CLIPBOARD, wxT("Copy to Clipboard"));
    auto* btnClose = new wxButton(botBar, wxID_CANCEL, wxT("Close"));

    botSizer->Add(m_btnUndo, 0, wxALL, 4);
    botSizer->Add(m_btnRedo, 0, wxALL, 4);
    botSizer->Add(btnSave, 0, wxALL, 4);
    botSizer->Add(btnSaveAs, 0, wxALL, 4);
    botSizer->Add(btnSaveAsCopy, 0, wxALL, 4);
    botSizer->Add(btnDuplicate, 0, wxALL, 4);
    botSizer->Add(btnExportAs, 0, wxALL, 4);
    botSizer->Add(btnCopyClip, 0, wxALL, 4);
    botSizer->AddStretchSpacer();
    botSizer->Add(btnClose, 0, wxALL, 4);

    botBar->SetSizer(botSizer);
    mainSizer->Add(botBar, 0, wxEXPAND | wxALL, 6);

    SetSizer(mainSizer);

    // Event Bindings
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnUndo, this, ID_BTN_UNDO);
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnRedo, this, ID_BTN_REDO);
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnEvenlySpace, this, ID_BTN_EVENLY_SPACE);
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnStraighten, this, ID_BTN_STRAIGHTEN);
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnSnapGrid, this, ID_BTN_SNAP_GRID);
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnSymmetrize, this, ID_BTN_SYMMETRIZE);
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnReverseWiring, this, ID_BTN_REVERSE_WIRING);
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnAutoSubmodels, this, ID_BTN_AUTO_SUBMODELS);
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnSave, this, ID_BTN_SAVE);
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnSaveAs, this, ID_BTN_SAVE_AS);
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnSaveAsCopy, this, ID_BTN_SAVE_AS_COPY);
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnDuplicate, this, ID_BTN_DUPLICATE);
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnExportAs, this, ID_BTN_EXPORT_AS);
    Bind(wxEVT_BUTTON, &AIPhoto3DPropReconstructorDialog::OnCopyToClipboard, this, ID_BTN_COPY_CLIPBOARD);
}

void AIPhoto3DPropReconstructorDialog::BuildLeftControlsPanel(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    m_modeNotebook = new wxNotebook(parent, wxID_ANY);

    // Tab 1: Single Image Mode
    auto* singleTab = new wxPanel(m_modeNotebook, wxID_ANY);
    auto* singleSizer = new wxBoxSizer(wxVERTICAL);
    singleSizer->Add(new wxStaticText(singleTab, wxID_ANY, wxT("Single Photo Path:")), 0, wxALL, 4);
    m_singleFilePicker = new wxFilePickerCtrl(singleTab, wxID_ANY, wxEmptyString, wxT("Select Photo"), wxT("Image Files (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg"));
    singleSizer->Add(m_singleFilePicker, 0, wxEXPAND | wxALL, 4);
    singleTab->SetSizer(singleSizer);
    m_modeNotebook->AddPage(singleTab, wxT("Single Photo Mode"));

    // Tab 2: Multi-Angle Photogrammetry Mode
    auto* multiTab = new wxPanel(m_modeNotebook, wxID_ANY);
    auto* multiSizer = new wxBoxSizer(wxVERTICAL);
    multiSizer->Add(new wxStaticText(multiTab, wxID_ANY, wxT("Multi-Angle Photos (Front, 45° Angle, Profile, Top):")), 0, wxALL, 4);
    auto* multiList = new wxListBox(multiTab, wxID_ANY);
    multiList->Append(wxT("Front_View.jpg (0°)"));
    multiList->Append(wxT("Side_45Deg.jpg (45°)"));
    multiList->Append(wxT("Top_Down.jpg (90°)"));
    multiSizer->Add(multiList, 1, wxEXPAND | wxALL, 4);
    multiTab->SetSizer(multiSizer);
    m_modeNotebook->AddPage(multiTab, wxT("Multi-Angle 3D Mode"));

    sizer->Add(m_modeNotebook, 0, wxEXPAND | wxBOTTOM, 6);

    // Extraction Tuning Parameters Box
    auto* paramBox = new wxStaticBoxSizer(wxVERTICAL, parent, wxT("AI Extraction & Geometry Settings"));

    paramBox->Add(new wxStaticText(parent, wxID_ANY, wxT("Edge Detection Sensitivity:")), 0, wxLEFT | wxTOP, 4);
    m_sliderEdgeSens = new wxSlider(parent, wxID_ANY, 50, 0, 100);
    paramBox->Add(m_sliderEdgeSens, 0, wxEXPAND | wxALL, 4);

    paramBox->Add(new wxStaticText(parent, wxID_ANY, wxT("Contour Smoothing:")), 0, wxLEFT | wxTOP, 4);
    m_sliderSmoothing = new wxSlider(parent, wxID_ANY, 3, 0, 10);
    paramBox->Add(m_sliderSmoothing, 0, wxEXPAND | wxALL, 4);

    paramBox->Add(new wxStaticText(parent, wxID_ANY, wxT("Node Pitch / Spacing (Inches):")), 0, wxLEFT | wxTOP, 4);
    m_spinSpacing = new wxSpinCtrlDouble(parent, wxID_ANY, wxT("2.0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.5, 6.0, 2.0, 0.25);
    paramBox->Add(m_spinSpacing, 0, wxEXPAND | wxALL, 4);

    paramBox->Add(new wxStaticText(parent, wxID_ANY, wxT("3D Depth Curvature Scale:")), 0, wxLEFT | wxTOP, 4);
    m_sliderDepthCurvature = new wxSlider(parent, wxID_ANY, 10, 0, 20);
    paramBox->Add(m_sliderDepthCurvature, 0, wxEXPAND | wxALL, 4);

    auto* btnReconstruct = new wxButton(parent, ID_BTN_RECONSTRUCT, wxT("✨ Reconstruct 3D Prop Mesh"));
    btnReconstruct->SetBackgroundColour(wxColour(40, 120, 220));
    btnReconstruct->SetForegroundColour(wxColour(255, 255, 255));
    paramBox->Add(btnReconstruct, 0, wxEXPAND | wxALL, 6);

    sizer->Add(paramBox, 0, wxEXPAND | wxBOTTOM, 6);

    // Advanced Post-Creation Editing Suite Box
    auto* editBox = new wxStaticBoxSizer(wxVERTICAL, parent, wxT("Post-Creation Advanced Editing Suite"));

    auto* btnEven = new wxButton(parent, ID_BTN_EVENLY_SPACE, wxT("📐 Evenly Space Nodes"));
    auto* btnStraight = new wxButton(parent, ID_BTN_STRAIGHTEN, wxT("📏 Straighten Segment"));
    auto* btnSnap = new wxButton(parent, ID_BTN_SNAP_GRID, wxT("🧲 Snap to Grid (0.5\")"));
    auto* btnSymm = new wxButton(parent, ID_BTN_SYMMETRIZE, wxT("❄️ Symmetrize (6-Fold Radial)"));
    auto* btnRev = new wxButton(parent, ID_BTN_REVERSE_WIRING, wxT("🔄 Reverse Wiring Sequence"));
    auto* btnAutoSub = new wxButton(parent, ID_BTN_AUTO_SUBMODELS, wxT("🏷️ Auto-Cluster Submodels"));

    editBox->Add(btnEven, 0, wxEXPAND | wxALL, 2);
    editBox->Add(btnStraight, 0, wxEXPAND | wxALL, 2);
    editBox->Add(btnSnap, 0, wxEXPAND | wxALL, 2);
    editBox->Add(btnSymm, 0, wxEXPAND | wxALL, 2);
    editBox->Add(btnRev, 0, wxEXPAND | wxALL, 2);
    editBox->Add(btnAutoSub, 0, wxEXPAND | wxALL, 2);

    sizer->Add(editBox, 1, wxEXPAND);
    parent->SetSizer(sizer);
}

void AIPhoto3DPropReconstructorDialog::BuildRightCanvasPanel(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    // Stats Bar
    auto* statsPanel = new wxPanel(parent, wxID_ANY);
    statsPanel->SetBackgroundColour(wxColour(32, 38, 48));
    auto* statsSizer = new wxBoxSizer(wxHORIZONTAL);

    m_lblNodeCount = new wxStaticText(statsPanel, wxID_ANY, wxT("Nodes: 0"));
    m_lblNodeCount->SetForegroundColour(wxColour(0, 220, 255));
    m_lblDimensions = new wxStaticText(statsPanel, wxID_ANY, wxT("Size: 0 x 0 x 0 in"));
    m_lblDimensions->SetForegroundColour(wxColour(255, 255, 255));
    m_lblSubmodels = new wxStaticText(statsPanel, wxID_ANY, wxT("Submodels: 0"));
    m_lblSubmodels->SetForegroundColour(wxColour(255, 200, 50));

    statsSizer->Add(m_lblNodeCount, 0, wxALL, 6);
    statsSizer->Add(m_lblDimensions, 0, wxALL, 6);
    statsSizer->Add(m_lblSubmodels, 0, wxALL, 6);
    statsPanel->SetSizer(statsSizer);
    sizer->Add(statsPanel, 0, wxEXPAND | wxBOTTOM, 4);

    // Visual Node Canvas
    m_canvasPanel = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
    m_canvasPanel->SetBackgroundColour(wxColour(18, 22, 28));
    m_canvasPanel->Bind(wxEVT_PAINT, &AIPhoto3DPropReconstructorDialog::OnPaintCanvas, this);
    sizer->Add(m_canvasPanel, 1, wxEXPAND | wxBOTTOM, 4);

    // Node Coordinate Inspection Table
    m_nodeListCtrl = new wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 140), wxLC_REPORT);
    m_nodeListCtrl->InsertColumn(0, wxT("Node #"), wxLIST_FORMAT_LEFT, 70);
    m_nodeListCtrl->InsertColumn(1, wxT("Strand"), wxLIST_FORMAT_LEFT, 70);
    m_nodeListCtrl->InsertColumn(2, wxT("X (in)"), wxLIST_FORMAT_LEFT, 80);
    m_nodeListCtrl->InsertColumn(3, wxT("Y (in)"), wxLIST_FORMAT_LEFT, 80);
    m_nodeListCtrl->InsertColumn(4, wxT("Z (in)"), wxLIST_FORMAT_LEFT, 80);
    m_nodeListCtrl->InsertColumn(5, wxT("Submodel"), wxLIST_FORMAT_LEFT, 140);
    sizer->Add(m_nodeListCtrl, 0, wxEXPAND);

    parent->SetSizer(sizer);
}

void AIPhoto3DPropReconstructorDialog::OnPaintCanvas(wxPaintEvent& WXUNUSED(event)) {
    if (!m_canvasPanel) return;
    wxPaintDC dc(m_canvasPanel);
    wxSize sz = m_canvasPanel->GetSize();
    dc.SetBackground(wxBrush(wxColour(18, 22, 28)));
    dc.Clear();

    if (m_currentModel.nodes.empty()) {
        dc.SetTextForeground(wxColour(120, 140, 160));
        dc.DrawText(wxT("Select a photo and click '✨ Reconstruct 3D Prop Mesh' to preview 3D nodes."), 20, sz.GetHeight() / 2 - 10);
        return;
    }

    // Determine coordinate bounds
    float minX = 1e9f, maxX = -1e9f, minY = 1e9f, maxY = -1e9f;
    for (const auto& n : m_currentModel.nodes) {
        minX = std::min(minX, n.x);
        maxX = std::max(maxX, n.x);
        minY = std::min(minY, n.y);
        maxY = std::max(maxY, n.y);
    }

    float spanX = std::max(1.0f, maxX - minX);
    float spanY = std::max(1.0f, maxY - minY);
    int pad = 30;
    int drawW = sz.GetWidth() - pad * 2;
    int drawH = sz.GetHeight() - pad * 2;

    auto mapPt = [&](float x, float y) -> wxPoint {
        int px = pad + static_cast<int>(((x - minX) / spanX) * drawW);
        int py = sz.GetHeight() - pad - static_cast<int>(((y - minY) / spanY) * drawH);
        return wxPoint(px, py);
    };

    // Draw connecting lines
    dc.SetPen(wxPen(wxColour(40, 100, 160), 2));
    for (size_t i = 1; i < m_currentModel.nodes.size(); ++i) {
        if (m_currentModel.nodes[i].strandIndex == m_currentModel.nodes[i-1].strandIndex) {
            wxPoint p1 = mapPt(m_currentModel.nodes[i-1].x, m_currentModel.nodes[i-1].y);
            wxPoint p2 = mapPt(m_currentModel.nodes[i].x, m_currentModel.nodes[i].y);
            dc.DrawLine(p1, p2);
        }
    }

    // Draw nodes
    for (const auto& n : m_currentModel.nodes) {
        wxPoint pt = mapPt(n.x, n.y);
        int brightness = std::clamp(static_cast<int>(180 + n.z * 15.0f), 80, 255);
        dc.SetBrush(wxBrush(wxColour(0, brightness, std::min(255, brightness + 30))));
        dc.SetPen(wxPen(wxColour(255, 255, 255), 1));
        dc.DrawCircle(pt, 4);
    }
}

void AIPhoto3DPropReconstructorDialog::PushModelMutation(const std::string& actionName, const ReconstructedPropModel& previousState) {
    auto currentState = m_currentModel;
    auto cmd = std::make_unique<AI::LambdaAICommand>(
        actionName,
        [this, currentState]() {
            m_currentModel = currentState;
            return true;
        },
        [this, previousState]() {
            m_currentModel = previousState;
            return true;
        }
    );
    m_commandHistory.ExecuteCommand(std::move(cmd));
    UpdateUndoRedoButtons();
}

void AIPhoto3DPropReconstructorDialog::TriggerReconstruction() {
    m_params.mode = (m_modeNotebook && m_modeNotebook->GetSelection() == 1)
                    ? ReconstructionMode::MULTI_IMAGE_PHOTOGRAMMETRY
                    : ReconstructionMode::SINGLE_IMAGE;

    if (m_singleFilePicker && !m_singleFilePicker->GetPath().IsEmpty()) {
        m_singleImagePath = m_singleFilePicker->GetPath().ToStdString();
    }

    if (m_params.mode == ReconstructionMode::SINGLE_IMAGE) {
        m_currentModel = Photo3DPropReconstructorAI::ReconstructFromSingleImage(m_singleImagePath, m_params);
    } else {
        m_multiImages = {
            {"Front.jpg", CameraViewAngle::FRONT, 0.0f, 1.0f},
            {"Side.jpg", CameraViewAngle::SIDE_PROFILE, 90.0f, 0.9f},
            {"Top.jpg", CameraViewAngle::TOP_DOWN, 90.0f, 0.85f}
        };
        m_currentModel = Photo3DPropReconstructorAI::ReconstructFromMultiViewImages(m_multiImages, m_params);
    }

    UpdateStats();
    if (m_canvasPanel) {
        m_canvasPanel->Refresh();
    }
}

void AIPhoto3DPropReconstructorDialog::UpdateStats() {
    if (m_lblNodeCount) {
        m_lblNodeCount->SetLabel(wxString::Format(wxT("Nodes: %zu"), m_currentModel.nodes.size()));
    }
    if (m_lblDimensions) {
        m_lblDimensions->SetLabel(wxString::Format(wxT("Size: %.1f\"W x %.1f\"H x %.1f\"D"),
            m_currentModel.boundingWidthInches,
            m_currentModel.boundingHeightInches,
            m_currentModel.boundingDepthInches));
    }
    if (m_lblSubmodels) {
        m_lblSubmodels->SetLabel(wxString::Format(wxT("Submodels: %zu groups"), m_currentModel.submodels.size()));
    }

    if (m_nodeListCtrl) {
        m_nodeListCtrl->DeleteAllItems();
        for (size_t i = 0; i < std::min(m_currentModel.nodes.size(), size_t(200)); ++i) {
            const auto& n = m_currentModel.nodes[i];
            long idx = m_nodeListCtrl->InsertItem(static_cast<long>(i), wxString::Format(wxT("%d"), n.nodeIndex));
            m_nodeListCtrl->SetItem(idx, 1, wxString::Format(wxT("%d"), n.strandIndex));
            m_nodeListCtrl->SetItem(idx, 2, wxString::Format(wxT("%.2f"), n.x));
            m_nodeListCtrl->SetItem(idx, 3, wxString::Format(wxT("%.2f"), n.y));
            m_nodeListCtrl->SetItem(idx, 4, wxString::Format(wxT("%.2f"), n.z));
            m_nodeListCtrl->SetItem(idx, 5, wxString::FromUTF8(n.submodelGroup));
        }
    }

    UpdateUndoRedoButtons();
}

void AIPhoto3DPropReconstructorDialog::UpdateUndoRedoButtons() {
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

void AIPhoto3DPropReconstructorDialog::OnUndo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Undo()) {
        UpdateStats();
    }
}

void AIPhoto3DPropReconstructorDialog::OnRedo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Redo()) {
        UpdateStats();
    }
}

void AIPhoto3DPropReconstructorDialog::OnEvenlySpace(wxCommandEvent& WXUNUSED(event)) {
    auto prevState = m_currentModel;
    Photo3DPropReconstructorAI::EvenlySpaceNodes(m_currentModel, -1, 2.0f);
    PushModelMutation("Evenly Space Nodes", prevState);
    UpdateStats();
}

void AIPhoto3DPropReconstructorDialog::OnStraighten(wxCommandEvent& WXUNUSED(event)) {
    auto prevState = m_currentModel;
    if (m_currentModel.nodes.size() >= 10) {
        Photo3DPropReconstructorAI::StraightenSegment(m_currentModel, 0, 10);
    }
    PushModelMutation("Straighten Segment", prevState);
    UpdateStats();
}

void AIPhoto3DPropReconstructorDialog::OnSnapGrid(wxCommandEvent& WXUNUSED(event)) {
    auto prevState = m_currentModel;
    Photo3DPropReconstructorAI::SnapNodesToGrid(m_currentModel, 0.5f);
    PushModelMutation("Snap Nodes to Grid", prevState);
    UpdateStats();
}

void AIPhoto3DPropReconstructorDialog::OnSymmetrize(wxCommandEvent& WXUNUSED(event)) {
    auto prevState = m_currentModel;
    Photo3DPropReconstructorAI::SymmetrizeRadial(m_currentModel, 6);
    PushModelMutation("Symmetrize 6-Fold", prevState);
    UpdateStats();
}

void AIPhoto3DPropReconstructorDialog::OnReverseWiring(wxCommandEvent& WXUNUSED(event)) {
    auto prevState = m_currentModel;
    Photo3DPropReconstructorAI::ReverseWiringOrder(m_currentModel, -1);
    PushModelMutation("Reverse Wiring Order", prevState);
    UpdateStats();
}

void AIPhoto3DPropReconstructorDialog::OnAutoSubmodels(wxCommandEvent& WXUNUSED(event)) {
    auto prevState = m_currentModel;
    Photo3DPropReconstructorAI::AutoClusterSubmodels(m_currentModel);
    PushModelMutation("Auto-Cluster Submodels", prevState);
    UpdateStats();
}

void AIPhoto3DPropReconstructorDialog::OnSave(wxCommandEvent& WXUNUSED(event)) {
    if (m_currentSavedPath.empty()) {
        m_currentSavedPath = m_currentModel.propName + ".xmodel";
    }
    std::ofstream out(m_currentSavedPath);
    out << m_currentModel.ExportXModelXml();
    spdlog::info("AIPhoto3DPropReconstructorDialog: Saved model to '{}'", m_currentSavedPath);
}

void AIPhoto3DPropReconstructorDialog::OnSaveAs(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Save Custom Model As"), wxEmptyString,
                         wxString::FromUTF8(m_currentModel.propName) + wxT(".xmodel"),
                         wxT("xLights Custom Model (*.xmodel)|*.xmodel"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        m_currentSavedPath = saveDlg.GetPath().ToStdString();
        std::ofstream out(m_currentSavedPath);
        out << m_currentModel.ExportXModelXml();
    }
}

void AIPhoto3DPropReconstructorDialog::OnSaveAsCopy(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Save Design Copy As"), wxEmptyString,
                         wxString::FromUTF8(m_currentModel.propName) + wxT("_Checkpoint.xmodel"),
                         wxT("xLights Custom Model (*.xmodel)|*.xmodel"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        std::string copyPath = saveDlg.GetPath().ToStdString();
        std::ofstream out(copyPath);
        out << m_currentModel.ExportXModelXml();
        spdlog::info("AIPhoto3DPropReconstructorDialog: Saved design copy to '{}'", copyPath);
    }
}

void AIPhoto3DPropReconstructorDialog::OnDuplicate(wxCommandEvent& WXUNUSED(event)) {
    m_currentModel = Photo3DPropReconstructorAI::DuplicateModel(m_currentModel);
    UpdateStats();
    spdlog::info("AIPhoto3DPropReconstructorDialog: Duplicated model as '{}'", m_currentModel.propName);
}

void AIPhoto3DPropReconstructorDialog::OnExportAs(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog exportDlg(this, wxT("Export 3D Prop Mesh"), wxEmptyString,
                          wxString::FromUTF8(m_currentModel.propName),
                          wxT("xLights Model (*.xmodel)|*.xmodel|Wavefront OBJ (*.obj)|*.obj|SVG Vector (*.svg)|*.svg"),
                          wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (exportDlg.ShowModal() == wxID_OK) {
        std::string exportPath = exportDlg.GetPath().ToStdString();
        std::ofstream out(exportPath);
        if (exportPath.find(".obj") != std::string::npos) {
            out << m_currentModel.ExportObjMesh();
        } else if (exportPath.find(".svg") != std::string::npos) {
            out << m_currentModel.ExportSvgPath();
        } else {
            out << m_currentModel.ExportXModelXml();
        }
        spdlog::info("AIPhoto3DPropReconstructorDialog: Exported to '{}'", exportPath);
    }
}

void AIPhoto3DPropReconstructorDialog::OnCopyToClipboard(wxCommandEvent& WXUNUSED(event)) {
    if (wxTheClipboard->Open()) {
        std::string csv = m_currentModel.ExportCsvCoordinates();
        wxTheClipboard->SetData(new wxTextDataObject(wxString::FromUTF8(csv)));
        wxTheClipboard->Close();
        spdlog::info("AIPhoto3DPropReconstructorDialog: Copied node coordinates to clipboard.");
    }
}

} // namespace xLights
