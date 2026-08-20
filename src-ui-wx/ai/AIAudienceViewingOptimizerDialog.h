/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <wx/checklst.h>
#include "src-core/layout/AudienceViewingOptimizerAI.h"
#include "src-core/ai/AICommandHistory.h"

namespace xLights {

class AIAudienceViewingOptimizerDialog : public wxDialog {
public:
    AIAudienceViewingOptimizerDialog(wxWindow* parent,
                                     wxWindowID id = wxID_ANY,
                                     const wxString& title = wxT("AI Automated Audience Sightline & Visibility Optimizer"),
                                     const wxPoint& pos = wxDefaultPosition,
                                     const wxSize& size = wxSize(1120, 820),
                                     long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);

    virtual ~AIAudienceViewingOptimizerDialog() = default;

    /// Direct programmatic inspection for unit testing
    const AI::SightlineOptimizationResult& GetSightlineResult() const { return m_sightlineResult; }

    void RunSightlineEvaluation();

    void OnAddVantagePhoto(wxCommandEvent& event);
    void OnRemoveVantagePhoto(wxCommandEvent& event);
    void OnApplyCheckedAdjustments(wxCommandEvent& event);
    void OnApplySelectedPropRecommendation(wxCommandEvent& event);
    void OnExportSightlineReport(wxCommandEvent& event);

    // Undo / Redo handlers
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);

private:
    void CreateControls();
    void BuildVantagePhotosPanel(wxPanel* parent);
    void BuildNotebookViews(wxPanel* parent);
    void UpdateUiFromResults();
    void UpdateUndoRedoButtons();

    std::vector<AI::SpectatorVantageImage> m_vantageImages;
    std::vector<AI::LayoutModelDescriptor> m_layoutModels;
    AI::SightlineOptimizationResult m_sightlineResult;
    AI::AICommandHistory m_commandHistory;

    // UI Widgets
    wxListCtrl* m_vantageListCtrl{nullptr};
    wxTextCtrl* m_txtVantageTag{nullptr};
    wxSpinCtrlDouble* m_spinDistanceFeet{nullptr};
    wxSpinCtrlDouble* m_spinHeightFeet{nullptr};
    wxSpinCtrlDouble* m_spinAngleDegrees{nullptr};

    wxButton* m_btnUndo{nullptr};
    wxButton* m_btnRedo{nullptr};

    wxStaticText* m_lblOverallVisibility{nullptr};
    wxStaticText* m_lblConflictSummary{nullptr};

    wxNotebook* m_notebook{nullptr};
    wxCheckListBox* m_chkListRecommendations{nullptr};
    wxStaticText* m_lblRecommendationDetails{nullptr};

    wxListCtrl* m_propRatingListCtrl{nullptr};
    wxPanel* m_perspectiveCanvasPanel{nullptr};
    wxTextCtrl* m_txtReportSummary{nullptr};

    void OnPaintPerspectiveCanvas(wxPaintEvent& event);
};

} // namespace xLights
