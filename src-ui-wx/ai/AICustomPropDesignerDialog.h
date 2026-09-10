// Copyright (c) xLights Project
#pragma once

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <wx/checkbox.h>
#include <wx/scrolwin.h>
#include "AI/CustomPropDesignerAI.h"
#include "AI/AIConfigurationManager.h"
#include "AI/PropNodeEditHistory.h"
#include "AI/PropModelEditEngine.h"
#include "AI/AINodeEditInterpreter.h"
#include <vector>
#include <map>
#include <string>

class AICustomPropDesignerDialog : public wxDialog {
public:
    AICustomPropDesignerDialog(wxWindow* parent, wxWindowID id = wxID_ANY,
                               const wxString& title = wxT("AI Custom Prop Designer & 3D CAD"),
                               const wxPoint& pos = wxDefaultPosition,
                               const wxSize& size = wxSize(1200, 800),
                               long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AICustomPropDesignerDialog() = default;

private:
    void InitUI();
    void RefreshCanvas();
    void UpdateUndoRedoButtons();
    void UpdateSelectedNodeControls();
    void UpdateDynamicParameterCards(const xLights::AI::PropGenerationSpec& spec);
    void SyncSpecFromUI();
    wxPoint Project3D(float x, float y, float z, int W, int H, float* outDepth = nullptr) const;

    // Event Handlers
    void OnAnalyzeClick(wxCommandEvent& event);
    void OnGenerateClick(wxCommandEvent& event);
    void OnExportXmlClick(wxCommandEvent& event);
    void OnInsertLayoutClick(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);
    void OnUndoClick(wxCommandEvent& event);
    void OnRedoClick(wxCommandEvent& event);
    void On3DToggle(wxCommandEvent& event);
    void OnViewFront(wxCommandEvent& event);
    void OnViewTop(wxCommandEvent& event);
    void OnViewIso(wxCommandEvent& event);
    void OnResetView(wxCommandEvent& event);
    void OnPresetChanged(wxCommandEvent& event);
    void OnArcQuickSet(wxCommandEvent& event);
    void OnToggleShowLabels(wxCommandEvent& event);
    void OnToggleShowStrands(wxCommandEvent& event);
    void OnToggleShowDimensions(wxCommandEvent& event);
    void OnAskAIClick(wxCommandEvent& event);
    void OnConfirmExecClick(wxCommandEvent& event);
    void OnCancelExecClick(wxCommandEvent& event);
    void OnAddNodeClick(wxCommandEvent& event);
    void OnRemoveNodeClick(wxCommandEvent& event);
    void OnNodeXChanged(wxSpinDoubleEvent& event);
    void OnNodeYChanged(wxSpinDoubleEvent& event);
    void OnNodeZChanged(wxSpinDoubleEvent& event);
    void OnNewBranchClick(wxCommandEvent& event);
    void OnSwitchBranchClick(wxCommandEvent& event);
    void OnCanvasPaint(wxPaintEvent& event);
    void OnCanvasLeftDown(wxMouseEvent& event);
    void OnCanvasLeftUp(wxMouseEvent& event);
    void OnCanvasRightDown(wxMouseEvent& event);
    void OnCanvasRightUp(wxMouseEvent& event);
    void OnCanvasMouseMove(wxMouseEvent& event);
    void OnCanvasMouseWheel(wxMouseEvent& event);

    // Toolbar
    wxButton* m_undoBtn = nullptr;
    wxButton* m_redoBtn = nullptr;
    wxButton* m_toggleViewBtn = nullptr;
    wxButton* m_viewFrontBtn = nullptr;
    wxButton* m_viewTopBtn = nullptr;
    wxButton* m_viewIsoBtn = nullptr;
    wxButton* m_resetViewBtn = nullptr;
    wxCheckBox* m_showStrandsCheck = nullptr;
    wxCheckBox* m_showLabelsCheck = nullptr;
    wxCheckBox* m_showDimensionsCheck = nullptr;
    bool m_is3DMode = true;

    // Left Scrollable Panel
    wxScrolledWindow* m_scrollPanel = nullptr;
    wxStaticText* m_modelBadge = nullptr;
    wxTextCtrl* m_descriptionCtrl = nullptr;
    wxChoice* m_shapePresetChoice = nullptr;
    wxButton* m_analyzeBtn = nullptr;
    wxButton* m_generateBtn = nullptr;

    // AI Executive Summary Card
    wxStaticBoxSizer* m_aiSummaryBox = nullptr;
    wxTextCtrl* m_aiSummaryText = nullptr;
    wxStaticText* m_aiClarificationsLabel = nullptr;

    // Dynamic Parameter Cards
    wxPanel* m_paramCardPanel = nullptr;
    wxBoxSizer* m_paramCardSizer = nullptr;
    
    // Tree Parameters
    wxStaticBoxSizer* m_treeParamBox = nullptr;
    wxSpinCtrlDouble* m_treeHeightSpin = nullptr;
    wxSpinCtrlDouble* m_treeBaseDiamSpin = nullptr;
    wxSpinCtrlDouble* m_treeTopDiamSpin = nullptr;
    wxSpinCtrl* m_treeColsSpin = nullptr;
    wxSpinCtrl* m_treeNodesPerColSpin = nullptr;
    wxSpinCtrlDouble* m_treeArcSpin = nullptr;
    wxChoice* m_treeWiringChoice = nullptr;
    wxStaticText* m_treePitchLabel = nullptr;
    wxCheckBox* m_hasTopperCheck = nullptr;
    wxSpinCtrl* m_starPointsSpin = nullptr;
    wxSpinCtrl* m_topperNodesSpin = nullptr;
    wxSpinCtrlDouble* m_topperDiamSpin = nullptr;

    // AI LLM Follow-up prompt
    wxTextCtrl* m_aiPromptCtrl = nullptr;
    wxButton* m_askAIBtn = nullptr;
    wxTextCtrl* m_aiResponseCtrl = nullptr;
    wxButton* m_confirmExecBtn = nullptr;
    wxButton* m_cancelExecBtn = nullptr;

    // Selected node controls
    wxStaticText* m_selectedNodeLabel = nullptr;
    wxSpinCtrlDouble* m_nodeXSpin = nullptr;
    wxSpinCtrlDouble* m_nodeYSpin = nullptr;
    wxSpinCtrlDouble* m_nodeZSpin = nullptr;
    wxSpinCtrl* m_newNodeIdSpin = nullptr;
    wxButton* m_addNodeBtn = nullptr;
    wxButton* m_removeNodeBtn = nullptr;

    // Branches
    wxChoice* m_branchChoice = nullptr;
    wxButton* m_newBranchBtn = nullptr;
    wxButton* m_switchBranchBtn = nullptr;

    // 3D Canvas Viewport
    wxPanel* m_canvasPanel = nullptr;

    // Bottom Bar
    wxStaticText* m_statusLabel = nullptr;
    wxButton* m_exportXmlBtn = nullptr;
    wxButton* m_insertLayoutBtn = nullptr;
    wxButton* m_closeBtn = nullptr;

    // State
    xLights::AI::PropGenerationSpec m_currentSpec;
    xLights::AI::AIPropAnalysisResult m_lastAnalysis;
    std::vector<xLights::AI::PropNodeSuggestion> m_lastNodes;
    int m_selectedNodeIndex = -1;
    xLights::AI::PropNodeEditHistory m_history;
    xLights::AI::PropModelEditEngine m_engine;
    xLights::AI::NodeEditPlan m_pendingPlan;
    std::map<std::string, std::vector<xLights::AI::PropNodeSuggestion>> m_branches;

    // 3D Camera & Mouse drag state
    float m_cameraYaw = 25.0f;
    float m_cameraPitch = 20.0f;
    float m_cameraZoom = 1.0f;
    float m_cameraPanX = 0.0f;
    float m_cameraPanY = 0.0f;
    bool m_isDragging = false;
    bool m_isOrbiting = false;
    wxPoint m_lastMousePos;
    wxPoint m_lastRightMousePos;

    DECLARE_EVENT_TABLE()
};
