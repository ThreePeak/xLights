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
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/gauge.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/filepicker.h>

#include "AI/SequenceValidatorAI.h"

namespace xLights::AI {

class AISequenceValidatorDialog : public wxDialog {
public:
    AISequenceValidatorDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("AI Sequence Health & Diagnostic Copilot"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(820, 620), long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AISequenceValidatorDialog() = default;

    void SetValidationConfig(const SequenceValidationConfig& config);
    [[nodiscard]] const SequenceValidationResult& GetValidationResult() const { return m_lastResult; }

private:
    void InitUI();
    void UpdateScorecardUI();
    void PopulateIssuesList();
    void UpdateCritiquePanel();

    // Event Handlers
    void OnRunAuditButtonClick(wxCommandEvent& event);
    void OnAutoRemediateButtonClick(wxCommandEvent& event);
    void OnExportJSONButtonClick(wxCommandEvent& event);
    void OnPersonaChoiceSelected(wxCommandEvent& event);
    void OnCloseButtonClick(wxCommandEvent& event);

    SequenceValidationConfig m_config;
    SequenceValidationResult m_lastResult;

    // UI Controls
    wxFilePickerCtrl* m_sequenceFilePicker = nullptr;
    wxChoice* m_categoryChoice = nullptr;
    wxChoice* m_personaChoice = nullptr;
    wxChoice* m_severityFilterChoice = nullptr;

    wxCheckBox* m_checkXmlChk = nullptr;
    wxCheckBox* m_checkBoundsChk = nullptr;
    wxCheckBox* m_checkRhythmChk = nullptr;
    wxCheckBox* m_checkHardwareChk = nullptr;
    wxCheckBox* m_checkHarmonyChk = nullptr;

    wxGauge* m_overallHealthGauge = nullptr;
    wxStaticText* m_healthScoreText = nullptr;

    wxGauge* m_timingGridGauge = nullptr;
    wxGauge* m_channelOverlapGauge = nullptr;
    wxGauge* m_hardwareSafetyGauge = nullptr;
    wxGauge* m_visualHarmonyGauge = nullptr;

    wxListCtrl* m_issuesListCtrl = nullptr;
    wxTextCtrl* m_critiqueTextCtrl = nullptr;

    wxButton* m_runAuditBtn = nullptr;
    wxButton* m_autoRemediateBtn = nullptr;
    wxButton* m_exportJsonBtn = nullptr;
    wxButton* m_closeBtn = nullptr;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
