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
#include <wx/choice.h>
#include "src-core/ai/SequenceVisualGitAI.h"
#include "src-core/ai/AICommandHistory.h"

namespace xLights {

class AISequenceVisualGitDialog : public wxDialog {
public:
    AISequenceVisualGitDialog(wxWindow* parent,
                              wxWindowID id = wxID_ANY,
                              const wxString& title = wxT("AI Sequence Visual Git & Timeline Diff Merge Assistant"),
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxSize(1120, 800),
                              long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);

    virtual ~AISequenceVisualGitDialog() = default;

    const AI::SequenceDiffReport& GetDiffReport() const { return m_report; }

    void OnCompareSequences(wxCommandEvent& event);
    void OnMergeAndExport(wxCommandEvent& event);
    void OnConflictStrategyChanged(wxCommandEvent& event);
    void OnSwapSequences(wxCommandEvent& event);
    void OnListRightClick(wxListEvent& event);
    void OnContextMenuAction(wxCommandEvent& event);

    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);

private:
    void CreateControls();
    void BuildTopSequenceSelector(wxPanel* parent);
    void BuildCenterDiffView(wxPanel* parent);
    void UpdateUiFromResults();
    void UpdateUndoRedoButtons();

    AI::SequenceDiffReport m_report;
    AI::AICommandHistory m_commandHistory;
    long m_contextItemIndex{-1};

    // UI Widgets
    class wxFilePickerCtrl;
    wxFilePickerCtrl* m_pickerBaseSeq{nullptr};
    wxFilePickerCtrl* m_pickerIncomingSeq{nullptr};

    wxButton* m_btnUndo{nullptr};
    wxButton* m_btnRedo{nullptr};

    wxStaticText* m_lblDiffSummary{nullptr};
    wxListCtrl* m_diffListCtrl{nullptr};
    wxChoice* m_choiceResolutionStrategy{nullptr};
    wxTextCtrl* m_txtFormattedReport{nullptr};
};

} // namespace xLights
