/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/dialog.h>
#include <wx/notebook.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/listctrl.h>
#include <wx/checklst.h>
#include <wx/radiobox.h>
#include <wx/spinctrl.h>
#include <wx/slider.h>
#include <wx/filepicker.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include "AI/VideoSequenceEmulator.h"

namespace xLights::AI {

class AIVideoSequenceEmulatorDialog : public wxDialog {
public:
    AIVideoSequenceEmulatorDialog(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& title = wxT("AI Video Sequence Emulation & Choreographer"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(980, 740),
        long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    );
    virtual ~AIVideoSequenceEmulatorDialog() = default;

private:
    void InitUI();
    void PopulateModelList();
    void RefreshCuesList();

    void OnAnalyzeVideoClick(wxCommandEvent& evt);
    void OnGenerateSequenceClick(wxCommandEvent& evt);
    void OnInsertIntoActiveSeqClick(wxCommandEvent& evt);
    void OnExportXsqClick(wxCommandEvent& evt);
    void OnApplyPolishClick(wxCommandEvent& evt);
    void OnCloseClick(wxCommandEvent& evt);

    wxNotebook* m_notebook = nullptr;

    // Tab 1: Source & Scope
    wxFilePickerCtrl* m_videoFilePicker = nullptr;
    wxTextCtrl* m_videoUrlCtrl = nullptr;
    wxCheckBox* m_timeRangeCheck = nullptr;
    wxSpinCtrl* m_startTimeSpin = nullptr;
    wxSpinCtrl* m_endTimeSpin = nullptr;
    wxCheckListBox* m_propsCheckList = nullptr;
    wxTextCtrl* m_userPromptCtrl = nullptr;
    wxButton* m_analyzeBtn = nullptr;

    // Tab 2: Pre-Flight Consultation
    wxTextCtrl* m_analysisSummaryCtrl = nullptr;
    wxRadioBox* m_strategyRadio = nullptr;
    wxRadioBox* m_paletteChoiceRadio = nullptr;
    wxRadioBox* m_densityChoiceRadio = nullptr;
    wxRadioBox* m_missingPropChoiceRadio = nullptr;
    wxButton* m_generatePlanBtn = nullptr;

    // Tab 3: Generated Cues & Polish
    wxListCtrl* m_cuesListCtrl = nullptr;
    wxButton* m_insertSequenceBtn = nullptr;
    wxButton* m_exportXsqBtn = nullptr;
    wxTextCtrl* m_refinePromptCtrl = nullptr;
    wxSlider* m_speedSlider = nullptr;
    wxStaticText* m_speedValLabel = nullptr;
    wxButton* m_applyPolishBtn = nullptr;

    // Bottom Bar
    wxGauge* m_progressGauge = nullptr;
    wxStaticText* m_statusLabel = nullptr;
    wxButton* m_closeBtn = nullptr;

    // State
    VideoSequenceEmulator m_emulator;
    VideoSourceInput m_currentInput;
    VideoAnalysisResult m_currentAnalysis;
    std::vector<AdaptationStrategy> m_currentStrategies;
    std::vector<ConsultationQuestion> m_currentQuestions;
    VideoSequencePlan m_currentPlan;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
