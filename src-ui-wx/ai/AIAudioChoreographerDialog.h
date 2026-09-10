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
#include <wx/slider.h>
#include <wx/checkbox.h>
#include <wx/filepicker.h>
#include <wx/combobox.h>
#include "src-core/media/AudioChoreographerAI.h"
#include "src-core/ai/AICommandHistory.h"

namespace xLights {

class AIAudioChoreographerDialog : public wxDialog {
public:
    AIAudioChoreographerDialog(wxWindow* parent,
                               wxWindowID id = wxID_ANY,
                               const wxString& title = wxT("AI Audio Stem Intelligence & Effect Choreographer"),
                               const wxPoint& pos = wxDefaultPosition,
                               const wxSize& size = wxSize(1080, 780),
                               long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);

    virtual ~AIAudioChoreographerDialog() = default;

    const AI::GeneratedChoreographyResult& GetChoreographyResult() const { return m_result; }

    void OnAnalyzeAndChoreograph(wxCommandEvent& event);
    void OnExportTimingTrack(wxCommandEvent& event);
    void OnApplyDeltaEffects(wxCommandEvent& event);

    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);

private:
    void CreateControls();
    void BuildLeftStemSetupPanel(wxPanel* parent);
    void BuildRightOutputPanel(wxPanel* parent);
    void UpdateUiFromResults();
    void UpdateUndoRedoButtons();

    AI::ChoreographyParameters m_params;
    AI::GeneratedChoreographyResult m_result;
    AI::AICommandHistory m_commandHistory;

    // UI Widgets
    wxFilePickerCtrl* m_pickerAudioFile{nullptr};
    
    wxChoice* m_choiceStem{nullptr};
    wxComboBox* m_comboTargetProp{nullptr};
    wxChoice* m_choiceEffectType{nullptr};
    wxSlider* m_sliderSensitivity{nullptr};
    wxCheckBox* m_chkNonDestructiveDelta{nullptr};
    wxTextCtrl* m_txtRefinementPrompt{nullptr};

    wxButton* m_btnUndo{nullptr};
    wxButton* m_btnRedo{nullptr};

    wxListCtrl* m_onsetsListCtrl{nullptr};
    wxTextCtrl* m_txtTimingXmlPreview{nullptr};
    wxTextCtrl* m_txtEffectsXmlPreview{nullptr};
    wxStaticText* m_lblStatusStats{nullptr};
};

} // namespace xLights
