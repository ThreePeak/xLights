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
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/slider.h>
#include <wx/gauge.h>
#include <wx/choice.h>
#include <wx/listctrl.h>
#include <thread>
#include <atomic>
#include <vector>
#include <string>

class xLightsFrame;

namespace xLights::AI {

class AIAudioLyricStudioDialog : public wxDialog {
public:
    AIAudioLyricStudioDialog(wxWindow* parent, xLightsFrame* frame = nullptr, wxWindowID id = wxID_ANY,
                             const wxString& title = wxT("AI Audio & Singing Face Studio"),
                             const wxPoint& pos = wxDefaultPosition,
                             const wxSize& size = wxSize(880, 640),
                             long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AIAudioLyricStudioDialog();

private:
    void InitUI();

    // Tab 1: Stem Separation
    wxPanel* CreateStemTab(wxWindow* parent);
    void OnBrowseAudioClick(wxCommandEvent& evt);
    void OnExtractStemsClick(wxCommandEvent& evt);

    // Tab 2: Transient Choreography
    wxPanel* CreateChoreoTab(wxWindow* parent);
    void OnGenerateChoreoClick(wxCommandEvent& evt);

    // Tab 3: Singing Face & Lip-Sync
    wxPanel* CreateLipSyncTab(wxWindow* parent);
    void OnAlignLyricsClick(wxCommandEvent& evt);

    // Tab 4: Harmonic Palettes
    wxPanel* CreatePaletteTab(wxWindow* parent);
    void OnGeneratePaletteClick(wxCommandEvent& evt);
    void OnPaintPalette(wxPaintEvent& evt);

    void OnCloseClick(wxCommandEvent& evt);

    xLightsFrame* m_frame = nullptr;
    wxNotebook* m_notebook = nullptr;
    wxStaticText* m_statusLabel = nullptr;

    // Stem Controls
    wxTextCtrl* m_stemAudioPathCtrl = nullptr;
    wxChoice* m_stemModelChoice = nullptr;
    wxGauge* m_stemGauge = nullptr;
    wxStaticText* m_stemStatusLabel = nullptr;
    wxButton* m_extractStemsBtn = nullptr;

    // Choreo Controls
    wxChoice* m_choreoStemChoice = nullptr;
    wxSlider* m_choreoSensitivitySlider = nullptr;
    wxChoice* m_choreoModelChoice = nullptr;
    wxListCtrl* m_choreoList = nullptr;
    wxButton* m_generateChoreoBtn = nullptr;

    // LipSync Controls
    wxTextCtrl* m_lyricsCtrl = nullptr;
    wxChoice* m_faceModelChoice = nullptr;
    wxListCtrl* m_visemeList = nullptr;
    wxButton* m_alignLyricsBtn = nullptr;

    // Palette Controls
    wxTextCtrl* m_vibePromptCtrl = nullptr;
    wxSlider* m_valenceSlider = nullptr;
    wxSlider* m_arousalSlider = nullptr;
    wxPanel* m_paletteCanvas = nullptr;
    std::vector<wxColour> m_currentPalette;

    // Concurrency
    std::atomic<bool> m_workerCancel{false};
    std::thread m_workerThread;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
