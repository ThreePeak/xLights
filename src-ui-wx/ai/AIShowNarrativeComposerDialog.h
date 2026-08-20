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
#include <wx/slider.h>
#include <wx/choice.h>
#include "src-core/media/ShowNarrativeComposerAI.h"
#include "src-core/ai/AICommandHistory.h"

namespace xLights {

class AIShowNarrativeComposerDialog : public wxDialog {
public:
    AIShowNarrativeComposerDialog(wxWindow* parent,
                                  wxWindowID id = wxID_ANY,
                                  const wxString& title = wxT("AI Show Narrative & Voiceover Storyboard Composer"),
                                  const wxPoint& pos = wxDefaultPosition,
                                  const wxSize& size = wxSize(1080, 780),
                                  long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);

    virtual ~AIShowNarrativeComposerDialog() = default;

    const AI::GeneratedNarrativeResult& GetGeneratedResult() const { return m_result; }

    void OnGenerateScript(wxCommandEvent& event);
    void OnSynthesizeVoice(wxCommandEvent& event);
    void OnExportTimingTrack(wxCommandEvent& event);
    void OnExportAudio(wxCommandEvent& event);

    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);

private:
    void CreateControls();
    void BuildLeftPromptPanel(wxPanel* parent);
    void BuildRightOutputPanel(wxPanel* parent);
    void UpdateUiFromResults();
    void UpdateUndoRedoButtons();

    AI::NarrativePromptParameters m_params;
    AI::GeneratedNarrativeResult m_result;
    AI::AICommandHistory m_commandHistory;

    // UI Widgets
    wxTextCtrl* m_txtShowTitle{nullptr};
    wxTextCtrl* m_txtFamilyName{nullptr};
    wxChoice* m_choiceStyle{nullptr};
    wxChoice* m_choiceVoice{nullptr};
    wxSlider* m_sliderPace{nullptr};
    wxTextCtrl* m_txtCustomPrompt{nullptr};

    wxButton* m_btnUndo{nullptr};
    wxButton* m_btnRedo{nullptr};

    wxTextCtrl* m_txtScriptEditor{nullptr};
    wxListCtrl* m_timingMarksListCtrl{nullptr};
    wxListBox* m_lstLightingCues{nullptr};
    wxStaticText* m_lblDurationStats{nullptr};
};

} // namespace xLights
