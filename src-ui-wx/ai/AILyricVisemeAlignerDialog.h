/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/filepicker.h>
#include <string>
#include <vector>
#include "AI/PhonemeMap.h"

namespace xLights::AI {

class AILyricVisemeAlignerDialog : public wxDialog {
public:
    AILyricVisemeAlignerDialog(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& title = wxT("AI Singing Face & Lyric Viseme Aligner"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(860, 620),
        long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    );

    virtual ~AILyricVisemeAlignerDialog() = default;

private:
    void InitUI();
    void RunAlignmentPipeline();
    void PopulateVisemesList();
    void ExportTimingTrackXml();
    void DrawVisemePreview(wxDC& dc);

    // Event Handlers
    void OnAlignClicked(wxCommandEvent& event);
    void OnExportClicked(wxCommandEvent& event);
    void OnDebounceSliderScroll(wxScrollEvent& event);
    void OnVisemeSelected(wxListEvent& event);
    void OnPaintCanvas(wxPaintEvent& event);
    void OnCloseClicked(wxCommandEvent& event);

    // Controls
    wxFilePickerCtrl* m_vocalAudioPicker{nullptr};
    wxTextCtrl* m_lyricsInputCtrl{nullptr};
    wxSlider* m_debounceSlider{nullptr};
    wxStaticText* m_debounceValueLabel{nullptr};
    wxListCtrl* m_visemeListCtrl{nullptr};
    wxPanel* m_visemeCanvas{nullptr};
    wxStaticText* m_selectedVisemeLabel{nullptr};
    wxStaticText* m_summaryLabel{nullptr};
    wxButton* m_exportBtn{nullptr};

    std::vector<PhonemeTimeInterval> m_rawPhonemes;
    SingingFaceTrackResult m_currentTrack;
    std::string m_activePreviewViseme{"rest"};

    enum {
        ID_VOCAL_PICKER = 12001,
        ID_LYRICS_INPUT,
        ID_ALIGN_BTN,
        ID_EXPORT_BTN,
        ID_DEBOUNCE_SLIDER,
        ID_VISEME_LIST,
        ID_VISEME_CANVAS
    };

    wxDECLARE_EVENT_TABLE();
};

} // namespace xLights::AI
