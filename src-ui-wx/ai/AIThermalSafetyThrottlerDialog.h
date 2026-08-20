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
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include "src-core/controllers/ThermalSafetyThrottlerAI.h"
#include "src-core/ai/AICommandHistory.h"

namespace xLights {

class AIThermalSafetyThrottlerDialog : public wxDialog {
public:
    AIThermalSafetyThrottlerDialog(wxWindow* parent,
                                   wxWindowID id = wxID_ANY,
                                   const wxString& title = wxT("AI Intelligent Thermal & Current Load Throttler"),
                                   const wxPoint& pos = wxDefaultPosition,
                                   const wxSize& size = wxSize(1120, 820),
                                   long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);

    virtual ~AIThermalSafetyThrottlerDialog() = default;

    /// Direct access for unit testing
    const ThermalSimulationResult& GetSimulationResult() const { return m_simResult; }
    void SetTestSimulationParameters(const ThermalSimulationParameters& params) { m_params = params; }
    void RunSimulation();

    void OnModeChanged(wxBookCtrlEvent& event);
    void OnRunSimulationClicked(wxCommandEvent& event);
    void OnApplySelectedFix(wxCommandEvent& event);
    void OnExportRemediationXML(wxCommandEvent& event);
    void OnToggleAutoOverride(wxCommandEvent& event);
    void OnEditAppliedCurve(wxCommandEvent& event);
    void OnRevertAppliedCurve(wxCommandEvent& event);
    void OnExportAuditReport(wxCommandEvent& event);

    // Undo / Redo handlers
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);

private:
    void CreateControls();
    void BuildTopControlsPanel(wxPanel* parent);
    void BuildNotebookViews(wxPanel* parent);
    void UpdateUiFromResults();
    void UpdateUndoRedoButtons();

    ThermalSimulationParameters m_params;
    ThermalSimulationResult m_simResult;
    AI::AICommandHistory m_commandHistory;

    // UI Widgets
    wxNotebook* m_modeNotebook{nullptr};
    wxSpinCtrlDouble* m_spinPsuWattage{nullptr};
    wxSpinCtrlDouble* m_spinMaxAmps{nullptr};
    wxSpinCtrl* m_spinMaxTemp{nullptr};
    wxCheckBox* m_chkIntentionalOverride{nullptr};

    wxButton* m_btnUndo{nullptr};
    wxButton* m_btnRedo{nullptr};

    wxStaticText* m_lblStatusSummary{nullptr};
    wxStaticText* m_lblPeakMetrics{nullptr};

    // Mode 1 & Mode 2 List
    wxListCtrl* m_incidentListCtrl{nullptr};
    wxCheckListBox* m_chkListObservations{nullptr};
    wxStaticText* m_lblObservationDetails{nullptr};

    // Mode 3 Applied Curves Table
    wxListCtrl* m_appliedCurvesListCtrl{nullptr};
    wxTextCtrl* m_txtReportPreview{nullptr};
};

} // namespace xLights
