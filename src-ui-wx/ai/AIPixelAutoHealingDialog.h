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
#include <wx/checkbox.h>
#include <wx/choice.h>
#include "src-core/models/PixelAutoHealingAI.h"
#include "src-core/ai/AICommandHistory.h"

namespace xLights {

class AIPixelAutoHealingDialog : public wxDialog {
public:
    AIPixelAutoHealingDialog(wxWindow* parent,
                             wxWindowID id = wxID_ANY,
                             const wxString& title = wxT("AI Computer Vision Dead Pixel Auto-Healer"),
                             const wxPoint& pos = wxDefaultPosition,
                             const wxSize& size = wxSize(1080, 780),
                             long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);

    virtual ~AIPixelAutoHealingDialog() = default;

    const AI::AutoHealingCalibrationResult& GetCalibrationResult() const { return m_calibration; }

    void OnRunVisionDiagnosis(wxCommandEvent& event);
    void OnToggleLiveRemap(wxCommandEvent& event);
    void OnExportHealingProfile(wxCommandEvent& event);

    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);

private:
    void CreateControls();
    void BuildTopSetupPanel(wxPanel* parent);
    void BuildCenterNotebook(wxPanel* parent);
    void UpdateUiFromResults();
    void UpdateUndoRedoButtons();

    std::string m_selectedPropName{"MegaTree"};
    AI::AutoHealingCalibrationResult m_calibration;
    AI::AICommandHistory m_commandHistory;

    // UI Widgets
    wxChoice* m_choiceProps{nullptr};
    wxCheckBox* m_chkEnableLiveRemap{nullptr};
    wxButton* m_btnUndo{nullptr};
    wxButton* m_btnRedo{nullptr};

    wxStaticText* m_lblStatusSummary{nullptr};
    wxStaticText* m_lblQualityScore{nullptr};

    wxListCtrl* m_faultListCtrl{nullptr};
    wxPanel* m_visualComparisonPanel{nullptr};
    wxTextCtrl* m_txtReportSummary{nullptr};

    void OnPaintComparisonCanvas(wxPaintEvent& event);
};

} // namespace xLights
