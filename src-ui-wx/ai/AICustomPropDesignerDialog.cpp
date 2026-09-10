// Copyright (c) xLights Project
#include "src-ui-wx/ai/AICustomPropDesignerDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/dcclient.h>
#include <wx/dc.h>
#include <wx/textdlg.h>
#include <wx/colour.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/time.h>
#include "src-ui-wx/xLightsMain.h"
#include "src-ui-wx/layout/LayoutPanel.h"
#include <spdlog/spdlog.h>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace xLights::AI;

enum {
    ID_PROP_ANALYZE      = 15000,
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
    ID_PROP_PRESET_CHOICE= 15013,
    ID_PROP_VIEW_FRONT   = 15014,
    ID_PROP_VIEW_TOP     = 15015,
    ID_PROP_VIEW_ISO     = 15016,
    ID_PROP_RESET_VIEW   = 15017,
    ID_PROP_SHOW_STRANDS = 15018,
    ID_PROP_SHOW_LABELS  = 15019,
    ID_PROP_SHOW_DIMS    = 15020,
    ID_PROP_ARC_180      = 15021,
    ID_PROP_ARC_240      = 15022,
    ID_PROP_ARC_270      = 15023,
    ID_PROP_ARC_360      = 15024,
    ID_PROP_INSERT_LAYOUT= 15025,
    ID_PROP_OPTIMIZE_TSP = 15026,
    ID_PROP_BATCH_INSERT = 15027,
};

BEGIN_EVENT_TABLE(AICustomPropDesignerDialog, wxDialog)
    EVT_BUTTON(ID_PROP_ANALYZE, AICustomPropDesignerDialog::OnAnalyzeClick)
    EVT_BUTTON(ID_PROP_GENERATE, AICustomPropDesignerDialog::OnGenerateClick)
    EVT_BUTTON(ID_PROP_EXPORT, AICustomPropDesignerDialog::OnExportXmlClick)
    EVT_BUTTON(ID_PROP_INSERT_LAYOUT, AICustomPropDesignerDialog::OnInsertLayoutClick)
    EVT_BUTTON(ID_PROP_OPTIMIZE_TSP, AICustomPropDesignerDialog::OnOptimizeTSPClick)
    EVT_BUTTON(ID_PROP_BATCH_INSERT, AICustomPropDesignerDialog::OnBatchInsertClick)
    EVT_BUTTON(ID_PROP_UNDO, AICustomPropDesignerDialog::OnUndoClick)
    EVT_BUTTON(ID_PROP_REDO, AICustomPropDesignerDialog::OnRedoClick)
    EVT_BUTTON(ID_PROP_TOGGLE_VIEW, AICustomPropDesignerDialog::On3DToggle)
    EVT_BUTTON(ID_PROP_VIEW_FRONT, AICustomPropDesignerDialog::OnViewFront)
    EVT_BUTTON(ID_PROP_VIEW_TOP, AICustomPropDesignerDialog::OnViewTop)
    EVT_BUTTON(ID_PROP_VIEW_ISO, AICustomPropDesignerDialog::OnViewIso)
    EVT_BUTTON(ID_PROP_RESET_VIEW, AICustomPropDesignerDialog::OnResetView)
    EVT_CHECKBOX(ID_PROP_SHOW_STRANDS, AICustomPropDesignerDialog::OnToggleShowStrands)
    EVT_CHECKBOX(ID_PROP_SHOW_LABELS, AICustomPropDesignerDialog::OnToggleShowLabels)
    EVT_CHECKBOX(ID_PROP_SHOW_DIMS, AICustomPropDesignerDialog::OnToggleShowDimensions)
    EVT_CHOICE(ID_PROP_PRESET_CHOICE, AICustomPropDesignerDialog::OnPresetChanged)
    EVT_BUTTON(ID_PROP_ARC_180, AICustomPropDesignerDialog::OnArcQuickSet)
    EVT_BUTTON(ID_PROP_ARC_240, AICustomPropDesignerDialog::OnArcQuickSet)
    EVT_BUTTON(ID_PROP_ARC_270, AICustomPropDesignerDialog::OnArcQuickSet)
    EVT_BUTTON(ID_PROP_ARC_360, AICustomPropDesignerDialog::OnArcQuickSet)
    EVT_BUTTON(ID_PROP_ASK_AI, AICustomPropDesignerDialog::OnAskAIClick)
    EVT_BUTTON(ID_PROP_CONFIRM_EXEC, AICustomPropDesignerDialog::OnConfirmExecClick)
    EVT_BUTTON(ID_PROP_CANCEL_EXEC, AICustomPropDesignerDialog::OnCancelExecClick)
    EVT_BUTTON(ID_PROP_ADD_NODE, AICustomPropDesignerDialog::OnAddNodeClick)
    EVT_BUTTON(ID_PROP_REMOVE_NODE, AICustomPropDesignerDialog::OnRemoveNodeClick)
    EVT_BUTTON(ID_PROP_NEW_BRANCH, AICustomPropDesignerDialog::OnNewBranchClick)
    EVT_BUTTON(ID_PROP_SWITCH_BRANCH, AICustomPropDesignerDialog::OnSwitchBranchClick)
    EVT_BUTTON(wxID_CANCEL, AICustomPropDesignerDialog::OnCloseClick)
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
    
    // Top Toolbar Sizer
    wxBoxSizer* toolbarSizer = new wxBoxSizer(wxHORIZONTAL);
    m_undoBtn = new wxButton(this, ID_PROP_UNDO, "↩ Undo");
    m_undoBtn->Enable(false);
    toolbarSizer->Add(m_undoBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    
    m_redoBtn = new wxButton(this, ID_PROP_REDO, "↪ Redo");
    m_redoBtn->Enable(false);
    toolbarSizer->Add(m_redoBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    
    toolbarSizer->AddSpacer(8);
    m_toggleViewBtn = new wxButton(this, ID_PROP_TOGGLE_VIEW, "Switch to 2D");
    toolbarSizer->Add(m_toggleViewBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    
    m_viewFrontBtn = new wxButton(this, ID_PROP_VIEW_FRONT, "Front");
    toolbarSizer->Add(m_viewFrontBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    m_viewTopBtn = new wxButton(this, ID_PROP_VIEW_TOP, "Top");
    toolbarSizer->Add(m_viewTopBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    m_viewIsoBtn = new wxButton(this, ID_PROP_VIEW_ISO, "Iso 3D");
    toolbarSizer->Add(m_viewIsoBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    m_resetViewBtn = new wxButton(this, ID_PROP_RESET_VIEW, "Reset");
    toolbarSizer->Add(m_resetViewBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    
    toolbarSizer->AddSpacer(8);
    m_showStrandsCheck = new wxCheckBox(this, ID_PROP_SHOW_STRANDS, "Wiring Lines");
    m_showStrandsCheck->SetValue(true);
    toolbarSizer->Add(m_showStrandsCheck, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    
    m_showDimensionsCheck = new wxCheckBox(this, ID_PROP_SHOW_DIMS, "Dimensions");
    m_showDimensionsCheck->SetValue(true);
    toolbarSizer->Add(m_showDimensionsCheck, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    
    m_showLabelsCheck = new wxCheckBox(this, ID_PROP_SHOW_LABELS, "Labels");
    m_showLabelsCheck->SetValue(false);
    toolbarSizer->Add(m_showLabelsCheck, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    
    toolbarSizer->AddSpacer(8);
    m_optimizeTspBtn = new wxButton(this, ID_PROP_OPTIMIZE_TSP, "⚡ Optimize Wiring (TSP)");
    m_optimizeTspBtn->SetToolTip("2-opt Euclidean Traveling Salesperson optimizer to minimize wire length & voltage drop.");
    toolbarSizer->Add(m_optimizeTspBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

    toolbarSizer->AddStretchSpacer(1);
    mainSizer->Add(toolbarSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    
    // Main Content
    wxBoxSizer* contentSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Left Control Panel (Scrollable, 380px width)
    m_scrollPanel = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxSize(390, -1), wxVSCROLL);
    m_scrollPanel->SetScrollRate(0, 15);
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    
    // Header & AI Model Badge
    wxBoxSizer* modelHeaderSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* aiIcon = new wxStaticText(m_scrollPanel, wxID_ANY, "⬡ AI Intent Engine:");
    aiIcon->SetFont(aiIcon->GetFont().Bold());
    modelHeaderSizer->Add(aiIcon, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    
    m_modelBadge = new wxStaticText(m_scrollPanel, wxID_ANY, "Gemini 3.7 Flash (Active)");
    m_modelBadge->SetForegroundColour(wxColour(0, 180, 140));
    modelHeaderSizer->Add(m_modelBadge, 0, wxALIGN_CENTER_VERTICAL);
    leftSizer->Add(modelHeaderSizer, 0, wxEXPAND | wxALL, 4);
    
    // Group 1: Natural Language Prop Description
    wxStaticBoxSizer* descBox = new wxStaticBoxSizer(wxVERTICAL, m_scrollPanel, "1. Natural Language Prop Description");
    m_descriptionCtrl = new wxTextCtrl(m_scrollPanel, wxID_ANY,
        "Mini tree with 80 nodes on the body and 20 for a top star. 8 vertical columns of 10 nodes each evenly spaced and mini tree body covers a 240 degree area with the back left open. Mini tree body will be 30\" high in real life",
        wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE);
    descBox->Add(m_descriptionCtrl, 0, wxEXPAND | wxBOTTOM, 4);
    
    wxBoxSizer* presetSizer = new wxBoxSizer(wxHORIZONTAL);
    presetSizer->Add(new wxStaticText(m_scrollPanel, wxID_ANY, "Preset:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    wxArrayString presets;
    presets.Add("Mini Tree");
    presets.Add("Mega Tree");
    presets.Add("Star");
    presets.Add("Matrix / Grid");
    presets.Add("Arch");
    presets.Add("Candy Cane");
    presets.Add("Snowflake");
    presets.Add("Wreath / Circle");
    presets.Add("Custom");
    m_shapePresetChoice = new wxChoice(m_scrollPanel, ID_PROP_PRESET_CHOICE, wxDefaultPosition, wxDefaultSize, presets);
    m_shapePresetChoice->SetSelection(0);
    presetSizer->Add(m_shapePresetChoice, 1, wxEXPAND);
    descBox->Add(presetSizer, 0, wxEXPAND | wxBOTTOM, 6);
    
    m_analyzeBtn = new wxButton(m_scrollPanel, ID_PROP_ANALYZE, "🔍 Analyze Prompt & Reveal Parameters");
    m_analyzeBtn->SetBackgroundColour(wxColour(30, 80, 130));
    m_analyzeBtn->SetForegroundColour(*wxWHITE);
    m_analyzeBtn->SetFont(m_analyzeBtn->GetFont().Bold());
    descBox->Add(m_analyzeBtn, 0, wxEXPAND);
    leftSizer->Add(descBox, 0, wxEXPAND | wxBOTTOM, 6);
    
    // Group 2: AI Executive Summary & Clarifications Dialogue Card
    m_aiSummaryBox = new wxStaticBoxSizer(wxVERTICAL, m_scrollPanel, "2. AI Interpretation & Clarifications");
    m_aiSummaryText = new wxTextCtrl(m_scrollPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 70), wxTE_MULTILINE | wxTE_READONLY);
    m_aiSummaryText->SetBackgroundColour(wxColour(35, 42, 54));
    m_aiSummaryText->SetForegroundColour(wxColour(220, 235, 245));
    m_aiSummaryBox->Add(m_aiSummaryText, 0, wxEXPAND | wxBOTTOM, 4);
    
    m_aiClarificationsLabel = new wxStaticText(m_scrollPanel, wxID_ANY, "");
    m_aiClarificationsLabel->SetForegroundColour(wxColour(170, 200, 220));
    m_aiSummaryBox->Add(m_aiClarificationsLabel, 0, wxEXPAND);
    leftSizer->Add(m_aiSummaryBox, 0, wxEXPAND | wxBOTTOM, 6);
    
    // Group 3: Dynamic Context-Aware Parameter Cards Panel
    m_paramCardPanel = new wxPanel(m_scrollPanel, wxID_ANY);
    m_paramCardSizer = new wxBoxSizer(wxVERTICAL);
    
    // ── Tree Parameter Card ──
    m_treeParamBox = new wxStaticBoxSizer(wxVERTICAL, m_paramCardPanel, "3. Tree Physical & Wiring Parameters");
    
    wxFlexGridSizer* treeGrid = new wxFlexGridSizer(7, 2, 4, 6);
    treeGrid->AddGrowableCol(1, 1);
    
    treeGrid->Add(new wxStaticText(m_paramCardPanel, wxID_ANY, "Height (in):"), 0, wxALIGN_CENTER_VERTICAL);
    m_treeHeightSpin = new wxSpinCtrlDouble(m_paramCardPanel, wxID_ANY, "30.0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1.0, 600.0, 30.0, 1.0);
    treeGrid->Add(m_treeHeightSpin, 1, wxEXPAND);
    
    treeGrid->Add(new wxStaticText(m_paramCardPanel, wxID_ANY, "Base Diam (in):"), 0, wxALIGN_CENTER_VERTICAL);
    m_treeBaseDiamSpin = new wxSpinCtrlDouble(m_paramCardPanel, wxID_ANY, "18.0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1.0, 600.0, 18.0, 1.0);
    treeGrid->Add(m_treeBaseDiamSpin, 1, wxEXPAND);
    
    treeGrid->Add(new wxStaticText(m_paramCardPanel, wxID_ANY, "Top Diam (in):"), 0, wxALIGN_CENTER_VERTICAL);
    m_treeTopDiamSpin = new wxSpinCtrlDouble(m_paramCardPanel, wxID_ANY, "3.5", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 100.0, 3.5, 0.5);
    treeGrid->Add(m_treeTopDiamSpin, 1, wxEXPAND);
    
    treeGrid->Add(new wxStaticText(m_paramCardPanel, wxID_ANY, "Strands / Cols:"), 0, wxALIGN_CENTER_VERTICAL);
    m_treeColsSpin = new wxSpinCtrl(m_paramCardPanel, wxID_ANY, "8", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 128, 8);
    treeGrid->Add(m_treeColsSpin, 1, wxEXPAND);
    
    treeGrid->Add(new wxStaticText(m_paramCardPanel, wxID_ANY, "Nodes / Strand:"), 0, wxALIGN_CENTER_VERTICAL);
    m_treeNodesPerColSpin = new wxSpinCtrl(m_paramCardPanel, wxID_ANY, "10", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 512, 10);
    treeGrid->Add(m_treeNodesPerColSpin, 1, wxEXPAND);
    
    treeGrid->Add(new wxStaticText(m_paramCardPanel, wxID_ANY, "Arc Coverage:"), 0, wxALIGN_CENTER_VERTICAL);
    wxBoxSizer* arcSizer = new wxBoxSizer(wxHORIZONTAL);
    m_treeArcSpin = new wxSpinCtrlDouble(m_paramCardPanel, wxID_ANY, "240.0", wxDefaultPosition, wxSize(65, -1), wxSP_ARROW_KEYS, 10.0, 360.0, 240.0, 10.0);
    arcSizer->Add(m_treeArcSpin, 0, wxRIGHT, 2);
    arcSizer->Add(new wxButton(m_paramCardPanel, ID_PROP_ARC_180, "180°", wxDefaultPosition, wxSize(36, -1)), 0, wxRIGHT, 1);
    arcSizer->Add(new wxButton(m_paramCardPanel, ID_PROP_ARC_240, "240°", wxDefaultPosition, wxSize(36, -1)), 0, wxRIGHT, 1);
    arcSizer->Add(new wxButton(m_paramCardPanel, ID_PROP_ARC_360, "360°", wxDefaultPosition, wxSize(36, -1)), 0);
    treeGrid->Add(arcSizer, 1, wxEXPAND);
    
    treeGrid->Add(new wxStaticText(m_paramCardPanel, wxID_ANY, "Wiring Pattern:"), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString wiringOptions;
    wiringOptions.Add("Bottom to Top");
    wiringOptions.Add("Top to Bottom");
    wiringOptions.Add("Zig-Zag (Up & Down)");
    m_treeWiringChoice = new wxChoice(m_paramCardPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wiringOptions);
    m_treeWiringChoice->SetSelection(0);
    treeGrid->Add(m_treeWiringChoice, 1, wxEXPAND);
    
    m_treeParamBox->Add(treeGrid, 0, wxEXPAND | wxBOTTOM, 4);
    
    m_treePitchLabel = new wxStaticText(m_paramCardPanel, wxID_ANY, "Calculated Pixel Pitch: ~3.1\" (79mm)");
    m_treePitchLabel->SetForegroundColour(wxColour(0, 200, 160));
    m_treeParamBox->Add(m_treePitchLabel, 0, wxEXPAND | wxBOTTOM, 6);
    
    // Star Topper Sub-section
    m_hasTopperCheck = new wxCheckBox(m_paramCardPanel, wxID_ANY, "Enable Top Star Topper");
    m_hasTopperCheck->SetValue(true);
    m_treeParamBox->Add(m_hasTopperCheck, 0, wxBOTTOM, 4);
    
    wxFlexGridSizer* starGrid = new wxFlexGridSizer(3, 2, 3, 6);
    starGrid->AddGrowableCol(1, 1);
    
    starGrid->Add(new wxStaticText(m_paramCardPanel, wxID_ANY, "Star Points:"), 0, wxALIGN_CENTER_VERTICAL);
    m_starPointsSpin = new wxSpinCtrl(m_paramCardPanel, wxID_ANY, "5", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 3, 16, 5);
    starGrid->Add(m_starPointsSpin, 1, wxEXPAND);
    
    starGrid->Add(new wxStaticText(m_paramCardPanel, wxID_ANY, "Topper Nodes:"), 0, wxALIGN_CENTER_VERTICAL);
    m_topperNodesSpin = new wxSpinCtrl(m_paramCardPanel, wxID_ANY, "20", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 5, 256, 20);
    starGrid->Add(m_topperNodesSpin, 1, wxEXPAND);
    
    starGrid->Add(new wxStaticText(m_paramCardPanel, wxID_ANY, "Topper Diam (in):"), 0, wxALIGN_CENTER_VERTICAL);
    m_topperDiamSpin = new wxSpinCtrlDouble(m_paramCardPanel, wxID_ANY, "10.0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 2.0, 50.0, 10.0, 1.0);
    starGrid->Add(m_topperDiamSpin, 1, wxEXPAND);
    
    m_treeParamBox->Add(starGrid, 0, wxEXPAND);
    m_paramCardSizer->Add(m_treeParamBox, 0, wxEXPAND);
    
    m_paramCardPanel->SetSizer(m_paramCardSizer);
    leftSizer->Add(m_paramCardPanel, 0, wxEXPAND | wxBOTTOM, 6);
    
    // Group 4: Primary Action Button (Generate 3D Prop)
    m_generateBtn = new wxButton(m_scrollPanel, ID_PROP_GENERATE, "🚀 Generate 3D Prop");
    m_generateBtn->SetBackgroundColour(wxColour(0, 140, 90));
    m_generateBtn->SetForegroundColour(*wxWHITE);
    m_generateBtn->SetFont(m_generateBtn->GetFont().Bold().Larger());
    leftSizer->Add(m_generateBtn, 0, wxEXPAND | wxBOTTOM, 6);
    
    // Group 5: AI Natural Language Node Editor
    wxStaticBoxSizer* aiBox = new wxStaticBoxSizer(wxVERTICAL, m_scrollPanel, "AI LLM Node Tweaks & Edits");
    m_aiPromptCtrl = new wxTextCtrl(m_scrollPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 45), wxTE_MULTILINE);
    m_aiPromptCtrl->SetHint("e.g. 'make star 50% larger', 'spread base to 24 inches'");
    aiBox->Add(m_aiPromptCtrl, 0, wxEXPAND | wxBOTTOM, 4);
    
    m_askAIBtn = new wxButton(m_scrollPanel, ID_PROP_ASK_AI, "Ask AI");
    aiBox->Add(m_askAIBtn, 0, wxEXPAND | wxBOTTOM, 4);
    
    m_aiResponseCtrl = new wxTextCtrl(m_scrollPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 45), wxTE_MULTILINE | wxTE_READONLY);
    m_aiResponseCtrl->SetBackgroundColour(wxColour(45, 45, 45));
    m_aiResponseCtrl->SetForegroundColour(*wxWHITE);
    m_aiResponseCtrl->Hide();
    aiBox->Add(m_aiResponseCtrl, 0, wxEXPAND | wxBOTTOM, 4);
    
    wxBoxSizer* confirmSizer = new wxBoxSizer(wxHORIZONTAL);
    m_confirmExecBtn = new wxButton(m_scrollPanel, ID_PROP_CONFIRM_EXEC, "Confirm");
    m_confirmExecBtn->Hide();
    confirmSizer->Add(m_confirmExecBtn, 1, wxRIGHT, 2);
    
    m_cancelExecBtn = new wxButton(m_scrollPanel, ID_PROP_CANCEL_EXEC, "Cancel");
    m_cancelExecBtn->Hide();
    confirmSizer->Add(m_cancelExecBtn, 1, wxLEFT, 2);
    aiBox->Add(confirmSizer, 0, wxEXPAND);
    leftSizer->Add(aiBox, 0, wxEXPAND | wxBOTTOM, 6);
    
    // Group 6: Selected Node Controls
    wxStaticBoxSizer* selBox = new wxStaticBoxSizer(wxVERTICAL, m_scrollPanel, "Selected Node Inspector");
    m_selectedNodeLabel = new wxStaticText(m_scrollPanel, wxID_ANY, "No node selected (Click node on canvas)");
    selBox->Add(m_selectedNodeLabel, 0, wxEXPAND | wxBOTTOM, 4);
    
    wxFlexGridSizer* selGrid = new wxFlexGridSizer(3, 2, 3, 4);
    selGrid->AddGrowableCol(1, 1);
    
    selGrid->Add(new wxStaticText(m_scrollPanel, wxID_ANY, "X:"), 0, wxALIGN_CENTER_VERTICAL);
    m_nodeXSpin = new wxSpinCtrlDouble(m_scrollPanel, wxID_ANY, "0.0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -9999.0, 9999.0, 0.0, 0.5);
    selGrid->Add(m_nodeXSpin, 1, wxEXPAND);
    
    selGrid->Add(new wxStaticText(m_scrollPanel, wxID_ANY, "Y:"), 0, wxALIGN_CENTER_VERTICAL);
    m_nodeYSpin = new wxSpinCtrlDouble(m_scrollPanel, wxID_ANY, "0.0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -9999.0, 9999.0, 0.0, 0.5);
    selGrid->Add(m_nodeYSpin, 1, wxEXPAND);
    
    selGrid->Add(new wxStaticText(m_scrollPanel, wxID_ANY, "Z:"), 0, wxALIGN_CENTER_VERTICAL);
    m_nodeZSpin = new wxSpinCtrlDouble(m_scrollPanel, wxID_ANY, "0.0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -9999.0, 9999.0, 0.0, 0.5);
    selGrid->Add(m_nodeZSpin, 1, wxEXPAND);
    selBox->Add(selGrid, 0, wxEXPAND | wxBOTTOM, 4);
    
    wxBoxSizer* nodeBtnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_newNodeIdSpin = new wxSpinCtrl(m_scrollPanel, wxID_ANY, "1", wxDefaultPosition, wxSize(55, -1), wxSP_ARROW_KEYS, 1, 9999, 1);
    nodeBtnSizer->Add(m_newNodeIdSpin, 0, wxRIGHT, 3);
    m_addNodeBtn = new wxButton(m_scrollPanel, ID_PROP_ADD_NODE, "+ Add");
    nodeBtnSizer->Add(m_addNodeBtn, 1, wxRIGHT, 2);
    m_removeNodeBtn = new wxButton(m_scrollPanel, ID_PROP_REMOVE_NODE, "- Remove");
    nodeBtnSizer->Add(m_removeNodeBtn, 1, wxLEFT, 2);
    selBox->Add(nodeBtnSizer, 0, wxEXPAND);
    leftSizer->Add(selBox, 0, wxEXPAND | wxBOTTOM, 6);
    
    // Group 7: Model Branches
    wxStaticBoxSizer* branchBox = new wxStaticBoxSizer(wxVERTICAL, m_scrollPanel, "Model Branches");
    wxBoxSizer* brSizer = new wxBoxSizer(wxHORIZONTAL);
    m_branchChoice = new wxChoice(m_scrollPanel, wxID_ANY);
    m_branchChoice->Append("[Main]");
    m_branchChoice->SetSelection(0);
    brSizer->Add(m_branchChoice, 1, wxEXPAND | wxRIGHT, 3);
    
    m_newBranchBtn = new wxButton(m_scrollPanel, ID_PROP_NEW_BRANCH, "New");
    brSizer->Add(m_newBranchBtn, 0, wxRIGHT, 2);
    m_switchBranchBtn = new wxButton(m_scrollPanel, ID_PROP_SWITCH_BRANCH, "Switch");
    brSizer->Add(m_switchBranchBtn, 0);
    branchBox->Add(brSizer, 0, wxEXPAND);
    leftSizer->Add(branchBox, 0, wxEXPAND | wxBOTTOM, 6);

    // Group 8: Multi-Model Batch Insertion
    wxStaticBoxSizer* batchBox = new wxStaticBoxSizer(wxVERTICAL, m_scrollPanel, "📦 Multi-Model Batch Placement");
    wxFlexGridSizer* batchGrid = new wxFlexGridSizer(3, 2, 3, 6);
    batchGrid->AddGrowableCol(1, 1);

    batchGrid->Add(new wxStaticText(m_scrollPanel, wxID_ANY, "Model Count:"), 0, wxALIGN_CENTER_VERTICAL);
    m_batchCountSpin = new wxSpinCtrl(m_scrollPanel, wxID_ANY, "4", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 2, 64, 4);
    batchGrid->Add(m_batchCountSpin, 1, wxEXPAND);

    batchGrid->Add(new wxStaticText(m_scrollPanel, wxID_ANY, "Pattern:"), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString patternOptions;
    patternOptions.Add("Arc / Fan (e.g. 4 Mini-Trees)");
    patternOptions.Add("Linear Array");
    patternOptions.Add("Grid Matrix");
    m_batchPatternChoice = new wxChoice(m_scrollPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, patternOptions);
    m_batchPatternChoice->SetSelection(0);
    batchGrid->Add(m_batchPatternChoice, 1, wxEXPAND);

    batchGrid->Add(new wxStaticText(m_scrollPanel, wxID_ANY, "Spacing / Radius:"), 0, wxALIGN_CENTER_VERTICAL);
    m_batchSpacingSpin = new wxSpinCtrlDouble(m_scrollPanel, wxID_ANY, "50.0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 5.0, 500.0, 50.0, 5.0);
    batchGrid->Add(m_batchSpacingSpin, 1, wxEXPAND);

    batchBox->Add(batchGrid, 0, wxEXPAND | wxBOTTOM, 4);

    m_batchInsertBtn = new wxButton(m_scrollPanel, ID_PROP_BATCH_INSERT, "📦 Batch Generate & Export XML");
    m_batchInsertBtn->SetToolTip("Generates chained multi-model array / arc layout XML ready for import into xLights.");
    batchBox->Add(m_batchInsertBtn, 0, wxEXPAND);

    leftSizer->Add(batchBox, 0, wxEXPAND);
    
    m_scrollPanel->SetSizer(leftSizer);
    contentSizer->Add(m_scrollPanel, 0, wxEXPAND | wxALL, 4);
    
    // Right Panel: 3D Viewport
    wxPanel* rightPanel = new wxPanel(this, wxID_ANY);
    wxStaticBoxSizer* previewBox = new wxStaticBoxSizer(wxVERTICAL, rightPanel, "3D Viewport (Right-Click Drag to Orbit, Left-Click to Select/Drag)");
    m_canvasPanel = new wxPanel(rightPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBG_STYLE_PAINT);
    
    m_canvasPanel->Bind(wxEVT_PAINT, &AICustomPropDesignerDialog::OnCanvasPaint, this);
    m_canvasPanel->Bind(wxEVT_LEFT_DOWN, &AICustomPropDesignerDialog::OnCanvasLeftDown, this);
    m_canvasPanel->Bind(wxEVT_LEFT_UP, &AICustomPropDesignerDialog::OnCanvasLeftUp, this);
    m_canvasPanel->Bind(wxEVT_RIGHT_DOWN, &AICustomPropDesignerDialog::OnCanvasRightDown, this);
    m_canvasPanel->Bind(wxEVT_RIGHT_UP, &AICustomPropDesignerDialog::OnCanvasRightUp, this);
    m_canvasPanel->Bind(wxEVT_MOTION, &AICustomPropDesignerDialog::OnCanvasMouseMove, this);
    m_canvasPanel->Bind(wxEVT_MOUSEWHEEL, &AICustomPropDesignerDialog::OnCanvasMouseWheel, this);
    
    previewBox->Add(m_canvasPanel, 1, wxEXPAND);
    rightPanel->SetSizer(previewBox);
    contentSizer->Add(rightPanel, 1, wxEXPAND | wxALL, 4);
    
    mainSizer->Add(contentSizer, 1, wxEXPAND);
    
    // Bottom Status Bar & Export
    m_statusLabel = new wxStaticText(this, wxID_ANY, "Ready — Click Analyze or Generate");
    mainSizer->Add(m_statusLabel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);
    
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_exportXmlBtn = new wxButton(this, ID_PROP_EXPORT, "Export Model XML (.xmodel)");
    m_insertLayoutBtn = new wxButton(this, ID_PROP_INSERT_LAYOUT, "📥 Insert into Current Layout");
    m_insertLayoutBtn->SetToolTip("Generate and prime this model directly on the Layout Panel canvas for 1-click placement.");
    btnSizer->Add(m_exportXmlBtn, 0, wxALL, 4);
    btnSizer->Add(m_insertLayoutBtn, 0, wxALL, 4);
    btnSizer->AddStretchSpacer(1);
    m_closeBtn = new wxButton(this, wxID_CANCEL, "Close");
    btnSizer->Add(m_closeBtn, 0, wxALL, 4);
    mainSizer->Add(btnSizer, 0, wxEXPAND);
    
    SetSizer(mainSizer);
    UpdateSelectedNodeControls();
    
    // Initial analysis & generation from default prompt
    wxCommandEvent evt;
    OnAnalyzeClick(evt);
    OnGenerateClick(evt);
}

void AICustomPropDesignerDialog::OnAnalyzeClick(wxCommandEvent& event)
{
    CustomPropDesignerAI ai;
    std::string desc = m_descriptionCtrl->GetValue().ToStdString();
    m_lastAnalysis = ai.AnalyzePrompt(desc, 100);
    m_currentSpec = m_lastAnalysis.spec;
    
    // Update Executive Summary box
    m_aiSummaryText->SetValue(wxString(m_lastAnalysis.executiveSummary));
    
    // Update Clarifications list
    std::ostringstream clarOss;
    for (const auto& c : m_lastAnalysis.clarifications) {
        clarOss << "• " << c << "\n";
    }
    m_aiClarificationsLabel->SetLabel(wxString(clarOss.str()));
    
    // Update Dynamic Parameter Controls with analyzed values
    UpdateDynamicParameterCards(m_currentSpec);
    m_scrollPanel->Layout();
    m_statusLabel->SetLabel("Analysis complete. Review parameters and click Generate 3D Prop.");
}

void AICustomPropDesignerDialog::UpdateDynamicParameterCards(const PropGenerationSpec& spec)
{
    m_treeHeightSpin->SetValue(spec.heightInches);
    m_treeBaseDiamSpin->SetValue(spec.baseDiameterInches);
    m_treeTopDiamSpin->SetValue(spec.topDiameterInches);
    m_treeColsSpin->SetValue(spec.columns);
    m_treeNodesPerColSpin->SetValue(spec.nodesPerColumn);
    m_treeArcSpin->SetValue(spec.arcDegrees);
    
    if (spec.wiringDirection == "TopToBottom") {
        m_treeWiringChoice->SetSelection(1);
    } else if (spec.wiringDirection == "ZigZag") {
        m_treeWiringChoice->SetSelection(2);
    } else {
        m_treeWiringChoice->SetSelection(0);
    }
    
    m_treePitchLabel->SetLabel(wxString::Format("Calculated Pixel Pitch: ~%.1f\" (%.0fmm)",
                                               spec.calculatedPixelPitchInches,
                                               spec.calculatedPixelPitchInches * 25.4f));
                                               
    m_hasTopperCheck->SetValue(spec.hasTopper);
    m_starPointsSpin->SetValue(spec.starPoints);
    m_topperNodesSpin->SetValue(spec.topperNodes);
    m_topperDiamSpin->SetValue(spec.topperDiameterInches);
}

void AICustomPropDesignerDialog::SyncSpecFromUI()
{
    m_currentSpec.heightInches = (float)m_treeHeightSpin->GetValue();
    m_currentSpec.baseDiameterInches = (float)m_treeBaseDiamSpin->GetValue();
    m_currentSpec.topDiameterInches = (float)m_treeTopDiamSpin->GetValue();
    m_currentSpec.columns = m_treeColsSpin->GetValue();
    m_currentSpec.nodesPerColumn = m_treeNodesPerColSpin->GetValue();
    m_currentSpec.arcDegrees = (float)m_treeArcSpin->GetValue();
    m_currentSpec.bodyNodes = m_currentSpec.columns * m_currentSpec.nodesPerColumn;
    
    int wiringSel = m_treeWiringChoice->GetSelection();
    if (wiringSel == 1) m_currentSpec.wiringDirection = "TopToBottom";
    else if (wiringSel == 2) m_currentSpec.wiringDirection = "ZigZag";
    else m_currentSpec.wiringDirection = "BottomToTop";
    
    m_currentSpec.hasTopper = m_hasTopperCheck->GetValue();
    m_currentSpec.starPoints = m_starPointsSpin->GetValue();
    m_currentSpec.topperNodes = m_topperNodesSpin->GetValue();
    m_currentSpec.topperDiameterInches = (float)m_topperDiamSpin->GetValue();
    
    m_currentSpec.totalNodes = m_currentSpec.bodyNodes + (m_currentSpec.hasTopper ? m_currentSpec.topperNodes : 0);
    if (m_currentSpec.nodesPerColumn > 1) {
        m_currentSpec.calculatedPixelPitchInches = m_currentSpec.heightInches / (m_currentSpec.nodesPerColumn - 1);
    }
}

void AICustomPropDesignerDialog::OnGenerateClick(wxCommandEvent& event)
{
    SyncSpecFromUI();
    CustomPropDesignerAI ai;
    auto before = m_lastNodes;
    auto after = ai.GenerateFromSpec(m_currentSpec);
    m_history.PushEdit("Generate 3D Prop", before, after);
    m_lastNodes = after;
    m_selectedNodeIndex = -1;
    RefreshCanvas();
    UpdateUndoRedoButtons();
    m_statusLabel->SetLabel(wxString::Format("Generated %d 3D nodes (%d body, %d star). Right-Click drag to orbit.",
                                           (int)m_lastNodes.size(), m_currentSpec.bodyNodes,
                                           m_currentSpec.hasTopper ? m_currentSpec.topperNodes : 0));
}

void AICustomPropDesignerDialog::OnArcQuickSet(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == ID_PROP_ARC_180) m_treeArcSpin->SetValue(180.0);
    else if (id == ID_PROP_ARC_240) m_treeArcSpin->SetValue(240.0);
    else if (id == ID_PROP_ARC_270) m_treeArcSpin->SetValue(270.0);
    else if (id == ID_PROP_ARC_360) m_treeArcSpin->SetValue(360.0);
    
    wxCommandEvent genEvt;
    OnGenerateClick(genEvt);
}

void AICustomPropDesignerDialog::OnPresetChanged(wxCommandEvent& event)
{
    int sel = m_shapePresetChoice->GetSelection();
    if (sel == 0) { // Mini Tree
        m_descriptionCtrl->SetValue("Mini tree with 80 nodes on the body and 20 for a top star. 8 vertical columns of 10 nodes each evenly spaced and mini tree body covers a 240 degree area with the back left open. Mini tree body will be 30\" high in real life");
    } else if (sel == 1) { // Mega Tree
        m_descriptionCtrl->SetValue("Mega Tree with 16 strands of 50 nodes covering 270 degrees with 50 node 5-point star topper, 12ft height, 6ft base");
    } else if (sel == 2) { // Star
        m_descriptionCtrl->SetValue("5-point star with 50 nodes along perimeter, 24 inch diameter");
    } else if (sel == 3) { // Matrix
        m_descriptionCtrl->SetValue("16x25 pixel matrix panel with 400 nodes total, horizontal serpentine wiring");
    } else if (sel == 4) { // Arch
        m_descriptionCtrl->SetValue("3 concentric jumping arches with 30 nodes per arch, 180 degree span");
    } else if (sel == 5) { // Candy Cane
        m_descriptionCtrl->SetValue("Candy cane with 65 nodes on the stem and 35 nodes on the 180 degree curved hook");
    } else if (sel == 6) { // Snowflake
        m_descriptionCtrl->SetValue("6-arm snowflake with 16 nodes per arm and 2-node branching prongs");
    } else if (sel == 7) { // Wreath
        m_descriptionCtrl->SetValue("Circular wreath ring with 70 nodes evenly spaced, 30 inch diameter");
    }
    
    wxCommandEvent analyzeEvt;
    OnAnalyzeClick(analyzeEvt);
    OnGenerateClick(analyzeEvt);
}

wxPoint AICustomPropDesignerDialog::Project3D(float x, float y, float z, int W, int H, float* outDepth) const
{
    if (!m_is3DMode) {
        float sx = (x / 100.0f) * (W * 0.8f) + (W * 0.1f) + m_cameraPanX;
        float sy = H - ((y / 100.0f) * (H * 0.8f) + (H * 0.1f)) + m_cameraPanY;
        if (outDepth) *outDepth = 0.0f;
        return wxPoint((int)sx, (int)sy);
    }
    
    float cx = x - 50.0f;
    float cy = y - 50.0f;
    float cz = z;
    
    float radYaw = m_cameraYaw * (float)M_PI / 180.0f;
    float cosY = std::cos(radYaw);
    float sinY = std::sin(radYaw);
    float rx = cx * cosY + cz * sinY;
    float rz = -cx * sinY + cz * cosY;
    
    float radPitch = m_cameraPitch * (float)M_PI / 180.0f;
    float cosP = std::cos(radPitch);
    float sinP = std::sin(radPitch);
    float ry = cy * cosP - rz * sinP;
    float rz2 = cy * sinP + rz * cosP;
    
    float dist = 140.0f;
    float denom = dist + rz2;
    float fov = (denom > 10.0f) ? (dist / denom) : 1.0f;
    
    float scale = std::min(W, H) * 0.0105f * m_cameraZoom;
    float screenX = (W / 2.0f) + rx * fov * scale + m_cameraPanX;
    float screenY = (H / 2.0f) - ry * fov * scale + m_cameraPanY;
    
    if (outDepth) *outDepth = rz2;
    return wxPoint((int)screenX, (int)screenY);
}

void AICustomPropDesignerDialog::OnCanvasPaint(wxPaintEvent& event)
{
    wxPaintDC dc(m_canvasPanel);
    wxSize sz = m_canvasPanel->GetClientSize();
    int W = sz.GetWidth();
    int H = sz.GetHeight();
    
    // Background Dark Slate
    dc.SetBrush(wxBrush(wxColour(18, 22, 30)));
    dc.DrawRectangle(0, 0, W, H);
    
    // Grid Floor
    if (!m_is3DMode) {
        dc.SetPen(wxPen(wxColour(40, 48, 64)));
        for (int i = 0; i < W; i += 40) dc.DrawLine(i, 0, i, H);
        for (int i = 0; i < H; i += 40) dc.DrawLine(0, i, W, i);
    } else {
        dc.SetPen(wxPen(wxColour(32, 42, 58)));
        for (int gx = -40; gx <= 40; gx += 10) {
            wxPoint p1 = Project3D(50.0f + gx, 5.0f, -40.0f, W, H);
            wxPoint p2 = Project3D(50.0f + gx, 5.0f, 40.0f, W, H);
            dc.DrawLine(p1, p2);
        }
        for (int gz = -40; gz <= 40; gz += 10) {
            wxPoint p1 = Project3D(10.0f, 5.0f, (float)gz, W, H);
            wxPoint p2 = Project3D(90.0f, 5.0f, (float)gz, W, H);
            dc.DrawLine(p1, p2);
        }
    }
    
    // Draw Physical Dimension Overlay Lines
    if (m_showDimensionsCheck->GetValue() && !m_lastNodes.empty()) {
        dc.SetPen(wxPen(wxColour(100, 160, 240, 180), 1, wxPENSTYLE_DOT));
        dc.SetTextForeground(wxColour(120, 180, 255));
        
        // Height Annotation
        wxPoint baseLeft = Project3D(20.0f, 12.0f, 0.0f, W, H);
        wxPoint topCollar = Project3D(20.0f, 68.0f, 0.0f, W, H);
        dc.DrawLine(baseLeft, topCollar);
        dc.DrawLine(baseLeft.x - 5, baseLeft.y, baseLeft.x + 5, baseLeft.y);
        dc.DrawLine(topCollar.x - 5, topCollar.y, topCollar.x + 5, topCollar.y);
        dc.DrawText(wxString::Format("Height: %.1f\"", m_currentSpec.heightInches), baseLeft.x - 75, (baseLeft.y + topCollar.y) / 2);
        
        // Base Width Annotation
        wxPoint baseL = Project3D(28.0f, 10.0f, 0.0f, W, H);
        wxPoint baseR = Project3D(72.0f, 10.0f, 0.0f, W, H);
        dc.DrawLine(baseL, baseR);
        dc.DrawText(wxString::Format("Base: %.1f\"", m_currentSpec.baseDiameterInches), (baseL.x + baseR.x) / 2 - 25, baseL.y + 8);
        
        // Arc Annotation
        wxPoint arcPos = Project3D(50.0f, 8.0f, 25.0f, W, H);
        dc.DrawText(wxString::Format("Arc: %.0f°", m_currentSpec.arcDegrees), arcPos.x - 20, arcPos.y + 5);
    }
    
    // Draw Strand / Wiring Connection Lines
    if (m_showStrandsCheck->GetValue() && !m_lastNodes.empty()) {
        std::map<std::string, std::vector<size_t>> groupMap;
        for (size_t i = 0; i < m_lastNodes.size(); ++i) {
            groupMap[m_lastNodes[i].group].push_back(i);
        }
        
        for (const auto& [grp, indices] : groupMap) {
            if (grp.find("Star") != std::string::npos) {
                dc.SetPen(wxPen(wxColour(255, 205, 40, 200), 2));
            } else {
                dc.SetPen(wxPen(wxColour(0, 210, 165, 150), 1, wxPENSTYLE_SOLID));
            }
            
            for (size_t k = 0; k + 1 < indices.size(); ++k) {
                auto& n1 = m_lastNodes[indices[k]];
                auto& n2 = m_lastNodes[indices[k + 1]];
                wxPoint p1 = Project3D(n1.x, n1.y, n1.z, W, H);
                wxPoint p2 = Project3D(n2.x, n2.y, n2.z, W, H);
                dc.DrawLine(p1, p2);
            }
            if (grp.find("Star") != std::string::npos && indices.size() > 2) {
                auto& n1 = m_lastNodes[indices.back()];
                auto& n2 = m_lastNodes[indices.front()];
                wxPoint p1 = Project3D(n1.x, n1.y, n1.z, W, H);
                wxPoint p2 = Project3D(n2.x, n2.y, n2.z, W, H);
                dc.DrawLine(p1, p2);
            }
        }
    }
    
    // Sort nodes by projected depth (back to front) in 3D mode
    struct RenderNode {
        size_t index;
        wxPoint screenPos;
        float depth;
    };
    std::vector<RenderNode> renderNodes;
    renderNodes.reserve(m_lastNodes.size());
    
    for (size_t i = 0; i < m_lastNodes.size(); ++i) {
        float depth = 0.0f;
        wxPoint pt = Project3D(m_lastNodes[i].x, m_lastNodes[i].y, m_lastNodes[i].z, W, H, &depth);
        renderNodes.push_back({ i, pt, depth });
    }
    
    if (m_is3DMode) {
        std::sort(renderNodes.begin(), renderNodes.end(), [](const RenderNode& a, const RenderNode& b) {
            return a.depth > b.depth;
        });
    }
    
    bool drawAllLabels = m_showLabelsCheck->GetValue();
    
    // Draw Nodes
    for (const auto& rn : renderNodes) {
        size_t i = rn.index;
        auto& node = m_lastNodes[i];
        bool selected = ((int)i == m_selectedNodeIndex);
        
        int radius = selected ? 7 : (m_is3DMode ? 4 : 5);
        
        if (selected) {
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.SetPen(wxPen(wxColour(255, 80, 40, 220), 2));
            dc.DrawCircle(rn.screenPos.x, rn.screenPos.y, radius + 3);
            dc.SetBrush(wxBrush(wxColour(255, 70, 30)));
        } else if (node.group.find("Star") != std::string::npos) {
            dc.SetBrush(wxBrush(wxColour(255, 215, 0))); // Gold
        } else {
            dc.SetBrush(wxBrush(wxColour(0, 230, 180))); // Emerald
        }
        
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawCircle(rn.screenPos.x, rn.screenPos.y, radius);
        
        if (drawAllLabels) {
            dc.SetTextForeground(wxColour(200, 215, 230));
            dc.DrawText(std::to_string(node.channelIndex), rn.screenPos.x + 5, rn.screenPos.y - 6);
        }
    }
    
    // Draw 3D Gizmo and Detailed Badge for Selected Node
    if (m_selectedNodeIndex >= 0 && m_selectedNodeIndex < (int)m_lastNodes.size()) {
        auto& sn = m_lastNodes[m_selectedNodeIndex];
        wxPoint pt = Project3D(sn.x, sn.y, sn.z, W, H);
        
        wxPoint ptX = Project3D(sn.x + 8.0f, sn.y, sn.z, W, H);
        wxPoint ptY = Project3D(sn.x, sn.y + 8.0f, sn.z, W, H);
        wxPoint ptZ = Project3D(sn.x, sn.y, sn.z + 8.0f, W, H);
        
        dc.SetPen(wxPen(wxColour(255, 60, 60), 2));
        dc.DrawLine(pt, ptX);
        dc.SetTextForeground(wxColour(255, 100, 100));
        dc.DrawText("X", ptX.x + 2, ptX.y - 5);
        
        dc.SetPen(wxPen(wxColour(60, 255, 60), 2));
        dc.DrawLine(pt, ptY);
        dc.SetTextForeground(wxColour(100, 255, 100));
        dc.DrawText("Y", ptY.x + 2, ptY.y - 12);
        
        dc.SetPen(wxPen(wxColour(80, 140, 255), 2));
        dc.DrawLine(pt, ptZ);
        dc.SetTextForeground(wxColour(120, 170, 255));
        dc.DrawText("Z", ptZ.x + 2, ptZ.y - 5);
        
        wxString badge = wxString::Format("Node %d (#%d) [%s] (%.1f, %.1f, %.1f)",
                                         m_selectedNodeIndex + 1, sn.channelIndex,
                                         sn.group.c_str(), sn.x, sn.y, sn.z);
        wxSize txtSz = dc.GetTextExtent(badge);
        dc.SetBrush(wxBrush(wxColour(30, 40, 55, 230)));
        dc.SetPen(wxPen(wxColour(255, 100, 50)));
        dc.DrawRoundedRectangle(pt.x + 12, pt.y - 20, txtSz.GetWidth() + 10, txtSz.GetHeight() + 6, 3);
        dc.SetTextForeground(*wxWHITE);
        dc.DrawText(badge, pt.x + 17, pt.y - 17);
    }
}

void AICustomPropDesignerDialog::OnCanvasLeftDown(wxMouseEvent& event)
{
    m_isDragging = false;
    m_lastMousePos = event.GetPosition();
    
    wxSize sz = m_canvasPanel->GetClientSize();
    int W = sz.GetWidth(), H = sz.GetHeight();
    
    int bestIdx = -1;
    float bestDistSq = 15.0f * 15.0f;
    
    for (size_t i = 0; i < m_lastNodes.size(); ++i) {
        wxPoint pt = Project3D(m_lastNodes[i].x, m_lastNodes[i].y, m_lastNodes[i].z, W, H);
        float dx = (float)(pt.x - m_lastMousePos.x);
        float dy = (float)(pt.y - m_lastMousePos.y);
        float distSq = dx * dx + dy * dy;
        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestIdx = (int)i;
        }
    }
    
    m_selectedNodeIndex = bestIdx;
    UpdateSelectedNodeControls();
    RefreshCanvas();
}

void AICustomPropDesignerDialog::OnCanvasLeftUp(wxMouseEvent& event)
{
    m_isDragging = false;
}

void AICustomPropDesignerDialog::OnCanvasRightDown(wxMouseEvent& event)
{
    m_isOrbiting = true;
    m_lastRightMousePos = event.GetPosition();
}

void AICustomPropDesignerDialog::OnCanvasRightUp(wxMouseEvent& event)
{
    m_isOrbiting = false;
}

void AICustomPropDesignerDialog::OnCanvasMouseMove(wxMouseEvent& event)
{
    if (m_isOrbiting && event.RightIsDown()) {
        wxPoint delta = event.GetPosition() - m_lastRightMousePos;
        m_lastRightMousePos = event.GetPosition();
        
        m_cameraYaw += delta.x * 0.6f;
        m_cameraPitch += delta.y * 0.6f;
        m_cameraPitch = std::clamp(m_cameraPitch, -85.0f, 85.0f);
        
        RefreshCanvas();
        return;
    }
    
    if (event.LeftIsDown() && m_selectedNodeIndex >= 0 && m_selectedNodeIndex < (int)m_lastNodes.size()) {
        wxPoint delta = event.GetPosition() - m_lastMousePos;
        m_lastMousePos = event.GetPosition();
        
        wxSize sz = m_canvasPanel->GetClientSize();
        float dx = (float)delta.x / (float)sz.GetWidth() * 80.0f;
        float dy = -(float)delta.y / (float)sz.GetHeight() * 80.0f;
        
        m_lastNodes[m_selectedNodeIndex].x += dx;
        m_lastNodes[m_selectedNodeIndex].y += dy;
        m_isDragging = true;
        
        UpdateSelectedNodeControls();
        RefreshCanvas();
    }
}

void AICustomPropDesignerDialog::OnCanvasMouseWheel(wxMouseEvent& event)
{
    int rot = event.GetWheelRotation();
    if (rot > 0) m_cameraZoom *= 1.08f;
    else m_cameraZoom *= 0.92f;
    m_cameraZoom = std::clamp(m_cameraZoom, 0.2f, 8.0f);
    RefreshCanvas();
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
    RefreshCanvas();
}

void AICustomPropDesignerDialog::OnViewFront(wxCommandEvent& event)
{
    m_cameraYaw = 0.0f;
    m_cameraPitch = 0.0f;
    m_is3DMode = true;
    m_toggleViewBtn->SetLabel("Switch to 2D");
    RefreshCanvas();
}

void AICustomPropDesignerDialog::OnViewTop(wxCommandEvent& event)
{
    m_cameraYaw = 0.0f;
    m_cameraPitch = 85.0f;
    m_is3DMode = true;
    m_toggleViewBtn->SetLabel("Switch to 2D");
    RefreshCanvas();
}

void AICustomPropDesignerDialog::OnViewIso(wxCommandEvent& event)
{
    m_cameraYaw = 35.0f;
    m_cameraPitch = 25.0f;
    m_is3DMode = true;
    m_toggleViewBtn->SetLabel("Switch to 2D");
    RefreshCanvas();
}

void AICustomPropDesignerDialog::OnResetView(wxCommandEvent& event)
{
    m_cameraYaw = 25.0f;
    m_cameraPitch = 20.0f;
    m_cameraZoom = 1.0f;
    m_cameraPanX = 0.0f;
    m_cameraPanY = 0.0f;
    RefreshCanvas();
}

void AICustomPropDesignerDialog::OnToggleShowLabels(wxCommandEvent& event)
{
    RefreshCanvas();
}

void AICustomPropDesignerDialog::OnToggleShowStrands(wxCommandEvent& event)
{
    RefreshCanvas();
}

void AICustomPropDesignerDialog::OnToggleShowDimensions(wxCommandEvent& event)
{
    RefreshCanvas();
}

void AICustomPropDesignerDialog::OnAskAIClick(wxCommandEvent& event)
{
    std::string prompt = m_aiPromptCtrl->GetValue().ToStdString();
    if (prompt.empty()) {
        wxMessageBox("Please enter a prompt describing what you want to edit.", "Notice");
        return;
    }
    auto cfg = AIConfigurationManager::Instance().GetSettings();
    AINodeEditInterpreter interp;
    m_pendingPlan = interp.InterpretPrompt(prompt, m_lastNodes, cfg);
    m_aiResponseCtrl->SetValue(wxString(m_pendingPlan.confirmationMessage));
    m_aiResponseCtrl->Show(true);
    m_confirmExecBtn->Show(true);
    m_cancelExecBtn->Show(true);
    m_scrollPanel->Layout();
    m_statusLabel->SetLabel("AI response ready. Click Confirm to apply or Cancel to discard.");
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
    m_scrollPanel->Layout();
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
    m_scrollPanel->Layout();
    m_statusLabel->SetLabel("AI edit cancelled.");
}

void AICustomPropDesignerDialog::OnAddNodeClick(wxCommandEvent& event)
{
    float cx = 50.0f, cy = 50.0f, cz = 0.0f;
    int reqId = m_newNodeIdSpin->GetValue();
    auto before = m_lastNodes;
    InsertNodeResult result = m_engine.InsertNode(m_lastNodes, cx, cy, cz, reqId);
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
    m_engine.SetNodePosition(m_lastNodes, m_selectedNodeIndex, (float)m_nodeXSpin->GetValue(), (float)m_nodeYSpin->GetValue(), (float)m_nodeZSpin->GetValue());
    m_history.PushEdit("Move Node " + std::to_string(m_selectedNodeIndex), before, m_lastNodes);
    RefreshCanvas();
    UpdateUndoRedoButtons();
}

void AICustomPropDesignerDialog::OnNodeYChanged(wxSpinDoubleEvent& event)
{
    if (m_selectedNodeIndex < 0 || m_isDragging) return;
    auto before = m_lastNodes;
    m_engine.SetNodePosition(m_lastNodes, m_selectedNodeIndex, (float)m_nodeXSpin->GetValue(), (float)m_nodeYSpin->GetValue(), (float)m_nodeZSpin->GetValue());
    m_history.PushEdit("Move Node " + std::to_string(m_selectedNodeIndex), before, m_lastNodes);
    RefreshCanvas();
    UpdateUndoRedoButtons();
}

void AICustomPropDesignerDialog::OnNodeZChanged(wxSpinDoubleEvent& event)
{
    if (m_selectedNodeIndex < 0 || m_isDragging) return;
    auto before = m_lastNodes;
    m_engine.SetNodePosition(m_lastNodes, m_selectedNodeIndex, (float)m_nodeXSpin->GetValue(), (float)m_nodeYSpin->GetValue(), (float)m_nodeZSpin->GetValue());
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
    m_statusLabel->SetLabel(wxString::Format("%d nodes in 3D | Undo: %d steps available", (int)m_lastNodes.size(), m_history.UndoDepth()));
}

void AICustomPropDesignerDialog::UpdateSelectedNodeControls()
{
    if (m_selectedNodeIndex < 0 || m_selectedNodeIndex >= (int)m_lastNodes.size()) {
        m_selectedNodeLabel->SetLabel("No node selected (Click node on canvas)");
        m_nodeXSpin->Enable(false);
        m_nodeYSpin->Enable(false);
        m_nodeZSpin->Enable(false);
        m_removeNodeBtn->Enable(false);
    } else {
        auto& n = m_lastNodes[m_selectedNodeIndex];
        m_selectedNodeLabel->SetLabel(wxString::Format("Node %d (#%d): %s | Group: %s",
                                                      m_selectedNodeIndex + 1, n.channelIndex,
                                                      n.label.c_str(), n.group.c_str()));
        m_nodeXSpin->SetValue(n.x);
        m_nodeYSpin->SetValue(n.y);
        m_nodeZSpin->SetValue(n.z);
        m_nodeXSpin->Enable(true);
        m_nodeYSpin->Enable(true);
        m_nodeZSpin->Enable(true);
        m_removeNodeBtn->Enable(true);
    }
}

void AICustomPropDesignerDialog::OnExportXmlClick(wxCommandEvent& event)
{
    if (m_lastNodes.empty()) {
        wxMessageBox("No model nodes to export.", "Notice");
        return;
    }
    
    wxFileDialog saveFileDialog(this, "Save xLights Custom Model", "", "MiniTree.xmodel",
                                "xLights Model Files (*.xmodel)|*.xmodel",
                                wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }
    
    CustomPropDesignerAI ai;
    std::string xml = ai.ExportToXLightsModelXML(m_lastNodes, saveFileDialog.GetFilename().BeforeLast('.').ToStdString(), m_currentSpec);
    
    wxFile file(saveFileDialog.GetPath(), wxFile::write);
    if (file.IsOpened()) {
        file.Write(wxString(xml));
        file.Close();
        m_statusLabel->SetLabel("Model successfully exported to " + saveFileDialog.GetFilename());
        wxMessageBox("Model exported successfully with submodels!", "Success");
    } else {
        wxMessageBox("Failed to open file for writing.", "Error", wxICON_ERROR);
    }
}

void AICustomPropDesignerDialog::OnInsertLayoutClick(wxCommandEvent& event)
{
    if (m_lastNodes.empty()) {
        wxMessageBox("No model nodes to insert into layout. Please generate or load a prop first.", "Notice", wxOK | wxICON_INFORMATION, this);
        return;
    }
    
    std::string modelName = "AI_" + m_currentSpec.propType;
    if (modelName.empty() || modelName == "AI_") {
        modelName = "AI_CustomProp";
    }
    std::replace_if(modelName.begin(), modelName.end(), [](char c) { return !isalnum(c) && c != '_'; }, '_');

    CustomPropDesignerAI ai;
    std::string xml = ai.ExportToXLightsModelXML(m_lastNodes, modelName, m_currentSpec);

    wxString tempDir = wxStandardPaths::Get().GetTempDir();
    wxFileName tempFile(tempDir, wxString::Format("AI_Prop_%lld.xmodel", (long long)wxGetLocalTimeMillis().GetValue()));
    wxString tempPath = tempFile.GetFullPath();

    wxFile file(tempPath, wxFile::write);
    if (!file.IsOpened()) {
        wxMessageBox("Could not create temporary model file for insertion.", "Error", wxOK | wxICON_ERROR, this);
        return;
    }
    file.Write(wxString(xml));
    file.Close();

    xLightsFrame* frame = xLightsFrame::GetFrame();
    if (frame && frame->GetLayoutPanel()) {
        frame->Notebook1->SetSelection(LAYOUTTAB);
        frame->GetLayoutPanel()->BeginImportModelFromFile(tempPath.ToStdString());
        m_statusLabel->SetLabel("Primed prop for layout insertion. Click layout to place.");
        EndModal(wxID_OK);
    } else {
        wxMessageBox("Layout panel is not available.", "Error", wxOK | wxICON_ERROR, this);
    }
}

void AICustomPropDesignerDialog::OnOptimizeTSPClick(wxCommandEvent& WXUNUSED(event))
{
    if (m_lastNodes.empty()) {
        wxMessageBox("No model nodes to optimize. Please generate or load a prop first.", "AI TSP Wire Optimizer", wxOK | wxICON_INFORMATION, this);
        return;
    }

    auto before = m_lastNodes;
    xLights::AI::CustomPropDesignerAI ai;
    auto tspRes = ai.OptimizeWirePathTSP(m_lastNodes);

    if (tspRes.wireSavingsPercent > 0.01f) {
        m_history.PushEdit(wxString::Format("Optimize Wire (TSP %.1f%% saved)", tspRes.wireSavingsPercent).ToStdString(), before, tspRes.optimizedNodes);
        m_lastNodes = tspRes.optimizedNodes;
        RefreshCanvas();
        UpdateUndoRedoButtons();
        m_statusLabel->SetLabel(wxString(tspRes.summary));
        wxMessageBox(wxString(tspRes.summary), "AI Wire Path Optimization", wxICON_INFORMATION | wxOK, this);
    } else {
        m_statusLabel->SetLabel("Wire path is already optimal (0% reduction possible).");
        wxMessageBox("The current wire order is already optimal or near-optimal.", "AI Wire Path Optimization", wxICON_INFORMATION | wxOK, this);
    }
}

void AICustomPropDesignerDialog::OnBatchInsertClick(wxCommandEvent& WXUNUSED(event))
{
    if (m_lastNodes.empty()) {
        wxMessageBox("Please generate or design a template prop model first before batch placing.", "Batch Model Placement", wxICON_WARNING | wxOK, this);
        return;
    }

    xLights::AI::BatchModelSpec bSpec;
    bSpec.baseModelName = m_currentSpec.propType;
    if (bSpec.baseModelName.empty()) bSpec.baseModelName = "CustomProp";
    bSpec.count = m_batchCountSpin ? m_batchCountSpin->GetValue() : 4;
    int patSel = m_batchPatternChoice ? m_batchPatternChoice->GetSelection() : 0;
    if (patSel == 1) {
        bSpec.pattern = xLights::AI::BatchPlacementPattern::LinearArray;
    } else if (patSel == 2) {
        bSpec.pattern = xLights::AI::BatchPlacementPattern::GridMatrix;
    } else {
        bSpec.pattern = xLights::AI::BatchPlacementPattern::ArcFan;
    }
    bSpec.spacingOrRadius = m_batchSpacingSpin ? (float)m_batchSpacingSpin->GetValue() : 50.0f;

    xLights::AI::CustomPropDesignerAI ai;
    std::string batchXml = ai.GenerateBatchPropModelsXML(m_lastNodes, bSpec, m_currentSpec);

    wxFileDialog saveDlg(this, "Export Multi-Model Batch XML", "", 
                         wxString::Format("%s_Batch_%d.xml", bSpec.baseModelName, bSpec.count),
                         "XML Files (*.xml)|*.xml", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        wxFile file(saveDlg.GetPath(), wxFile::write);
        if (file.IsOpened()) {
            file.Write(wxString(batchXml));
            file.Close();
            m_statusLabel->SetLabel(wxString::Format("Exported batch of %d models to %s", bSpec.count, saveDlg.GetPath()));
            wxMessageBox(wxString::Format("Successfully exported %d '%s' models in a %s configuration to:\n%s",
                                         bSpec.count, bSpec.baseModelName,
                                         (patSel == 0 ? "Arc/Fan" : (patSel == 1 ? "Linear Array" : "Grid")),
                                         saveDlg.GetPath()),
                         "Batch Multi-Model Export Complete", wxICON_INFORMATION | wxOK, this);
        }
    }
}

void AICustomPropDesignerDialog::OnCloseClick(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}
