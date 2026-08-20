/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>

#include "src-core/controllers/AISparseFSEQOptimizer.h"

namespace xLights::AI {

class AISparseFSEQOptimizerDialog : public wxDialog {
public:
    AISparseFSEQOptimizerDialog(wxWindow* parent,
                                wxWindowID id = wxID_ANY,
                                const wxString& title = wxT("AI Neural Sparse .FSEQ & SD Alignment Optimizer"),
                                const wxPoint& pos = wxDefaultPosition,
                                const wxSize& size = wxSize(840, 620),
                                long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);
    virtual ~AISparseFSEQOptimizerDialog() = default;

    const SparseFseqOptimizationResult& GetLastResult() const { return m_lastResult; }

private:
    void InitUI();

    void OnOptimizeClick(wxCommandEvent& event);
    void OnExportSparseFseqClick(wxCommandEvent& event);
    void OnExportReportClick(wxCommandEvent& event);
    void OnHelpClick(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);

    wxTextCtrl* m_txtSequenceName{nullptr};
    wxTextCtrl* m_txtControllerName{nullptr};
    wxSpinCtrl* m_spinTotalChannels{nullptr};
    wxSpinCtrl* m_spinTargetChannels{nullptr};
    wxSpinCtrl* m_spinDurationSeconds{nullptr};
    wxChoice* m_choiceFps{nullptr};
    wxChoice* m_choiceClusterSize{nullptr};
    wxCheckBox* m_chkDeltaCompression{nullptr};

    wxStaticText* m_lblMetrics{nullptr};
    wxTextCtrl* m_txtReport{nullptr};

    wxButton* m_btnOptimize{nullptr};
    wxButton* m_btnExportSparseFseq{nullptr};
    wxButton* m_btnExportReport{nullptr};
    wxButton* m_btnClose{nullptr};

    SparseFseqOptimizationResult m_lastResult;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
