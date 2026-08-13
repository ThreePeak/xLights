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
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/gauge.h>
#include <wx/sizer.h>

#include "AI/AudioStemExtractor.h"

namespace xLights::AI {

class AIAudioStemExtractorDialog : public wxDialog {
public:
    AIAudioStemExtractorDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("AI Audio Stem & Feature Extractor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(750, 560), long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AIAudioStemExtractorDialog() = default;

private:
    void InitUI();

    // Event Handlers
    void OnExtractButtonClick(wxCommandEvent& event);
    void OnPhonemeMapButtonClick(wxCommandEvent& event);
    void OnCloseButtonClick(wxCommandEvent& event);

    wxChoice* m_stemModelChoice = nullptr;
    wxSlider* m_transientSensitivitySlider = nullptr;
    wxSpinCtrl* m_framePeriodSpin = nullptr;
    wxButton* m_phonemeMapBtn = nullptr;

    wxCheckBox* m_vocalsChk = nullptr;
    wxCheckBox* m_drumsChk = nullptr;
    wxCheckBox* m_bassChk = nullptr;
    wxCheckBox* m_bpmTimingChk = nullptr;

    wxGauge* m_progressGauge = nullptr;
    wxStaticText* m_statusText = nullptr;

    wxButton* m_extractBtn = nullptr;
    wxButton* m_closeBtn = nullptr;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
