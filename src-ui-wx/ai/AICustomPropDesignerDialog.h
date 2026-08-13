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
#include "AI/CustomPropDesignerAI.h"
#include "AI/AIConfigurationManager.h"
#include "AI/PropNodeEditHistory.h"
#include "AI/PropModelEditEngine.h"
#include "AI/AINodeEditInterpreter.h"
#include <vector>
#include <map>
#include <string>

class ModelPreview;

class AICustomPropDesignerDialog : public wxDialog {
public:
    AICustomPropDesignerDialog(wxWindow* parent, wxWindowID id = wxID_ANY,
                               const wxString& title = wxT("AI Custom Prop Designer"),
                               const wxPoint& pos = wxDefaultPosition,
                               const wxSize& size = wxSize(1100, 720),
                               long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AICustomPropDesignerDialog() = default;

private:
    void InitUI();
    void InitToolbar();
    void RefreshCanvas();
    void UpdateUndoRedoButtons();
    void UpdateSelectedNodeControls();

    void OnGenerateClick(wxCommandEvent& event);
    void OnExportXmlClick(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);
    void OnUndoClick(wxCommandEvent& event);
    void OnRedoClick(wxCommandEvent& event);
    void On3DToggle(wxCommandEvent& event);
    void OnAskAIClick(wxCommandEvent& event);
    void OnConfirmExecClick(wxCommandEvent& event);
    void OnCancelExecClick(wxCommandEvent& event);
    void OnAddNodeClick(wxCommandEvent& event);
    void OnRemoveNodeClick(wxCommandEvent& event);
    void OnNodeXChanged(wxSpinDoubleEvent& event);
    void OnNodeYChanged(wxSpinDoubleEvent& event);
    void OnNewBranchClick(wxCommandEvent& event);
    void OnSwitchBranchClick(wxCommandEvent& event);
    void OnCanvasPaint(wxPaintEvent& event);
    void OnCanvasLeftDown(wxMouseEvent& event);
    void OnCanvasLeftUp(wxMouseEvent& event);
    void OnCanvasMouseMove(wxMouseEvent& event);
    void OnCanvasMouseWheel(wxMouseEvent& event);

    // Toolbar
    wxButton* m_undoBtn = nullptr;
    wxButton* m_redoBtn = nullptr;
    wxButton* m_toggleViewBtn = nullptr; // "Switch to 2D" / "Switch to 3D"
    bool m_is3DMode = true;

    // Left panel — description
    wxTextCtrl* m_descriptionCtrl = nullptr;
    wxSpinCtrl* m_nodeCountSpin = nullptr;
    wxChoice* m_shapePresetChoice = nullptr;
    wxButton* m_generateBtn = nullptr;

    // Left panel — AI LLM prompt
    wxTextCtrl* m_aiPromptCtrl = nullptr;
    wxButton* m_askAIBtn = nullptr;
    wxTextCtrl* m_aiResponseCtrl = nullptr; // read-only grey response
    wxButton* m_confirmExecBtn = nullptr;
    wxButton* m_cancelExecBtn = nullptr;

    // Left panel — selected node controls
    wxStaticText* m_selectedNodeLabel = nullptr;
    wxSpinCtrlDouble* m_nodeXSpin = nullptr;
    wxSpinCtrlDouble* m_nodeYSpin = nullptr;
    wxSpinCtrl* m_newNodeIdSpin = nullptr;
    wxButton* m_addNodeBtn = nullptr;
    wxButton* m_removeNodeBtn = nullptr;

    // Left panel — branches
    wxChoice* m_branchChoice = nullptr;
    wxButton* m_newBranchBtn = nullptr;
    wxButton* m_switchBranchBtn = nullptr;

    // Canvas
    wxPanel* m_canvasPanel = nullptr;

    // Bottom
    wxStaticText* m_statusLabel = nullptr;
    wxButton* m_exportXmlBtn = nullptr;
    wxButton* m_closeBtn = nullptr;

    // State
    std::vector<PropNodeSuggestion> m_lastNodes;
    int m_selectedNodeIndex = -1;
    PropNodeEditHistory m_history;
    PropModelEditEngine m_engine;
    NodeEditPlan m_pendingPlan;
    std::map<std::string, std::vector<PropNodeSuggestion>> m_branches;

    // Mouse drag state
    bool m_isDragging = false;
    wxPoint m_lastMousePos;

    DECLARE_EVENT_TABLE()
};
