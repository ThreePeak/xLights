/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/wx.h>
#include <wx/filepicker.h>
#include <wx/gauge.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <string>
#include <vector>
#include "src-core/media/PhotorealisticVisualizerRenderer.h"

namespace xLights::AI {

class AIPhotorealisticPreviewDialog : public wxDialog {
public:
    AIPhotorealisticPreviewDialog(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& title = wxT("3D Photorealistic Visualizer (ControlNet / SD Mode)"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(980, 720),
        long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    );

    virtual ~AIPhotorealisticPreviewDialog() = default;

private:
    void InitUI();
    void UpdatePromptPreview();
    void RenderCurrentSnapshot();
    void RenderFullSequenceVideo();
    void DrawPreviewCanvas(wxDC& dc);

    // Event Handlers
    void OnRenderSnapshotClicked(wxCommandEvent& event);
    void OnRenderVideoClicked(wxCommandEvent& event);
    void OnSceneStyleChanged(wxCommandEvent& event);
    void OnConditioningChanged(wxCommandEvent& event);
    void OnSliderParamScroll(wxScrollEvent& event);
    void OnViewModeChanged(wxCommandEvent& event);
    void OnPaintCanvas(wxPaintEvent& event);
    void OnCloseClicked(wxCommandEvent& event);

    // Controls
    wxFilePickerCtrl* m_houseBgPicker{nullptr};
    wxChoice* m_sceneStyleChoice{nullptr};
    wxChoice* m_conditioningChoice{nullptr};
    wxChoice* m_viewModeChoice{nullptr};
    wxSlider* m_bloomSlider{nullptr};
    wxStaticText* m_bloomValueLabel{nullptr};
    wxSlider* m_reflectionSlider{nullptr};
    wxStaticText* m_reflectionValueLabel{nullptr};
    wxSlider* m_ambientSlider{nullptr};
    wxStaticText* m_ambientValueLabel{nullptr};
    wxSlider* m_timelineFrameSlider{nullptr};
    wxStaticText* m_timelineFrameLabel{nullptr};
    wxTextCtrl* m_promptTextCtrl{nullptr};
    wxPanel* m_previewCanvas{nullptr};
    wxGauge* m_renderProgressBar{nullptr};
    wxStaticText* m_statusLabel{nullptr};
    wxButton* m_renderVideoBtn{nullptr};
    wxButton* m_renderSnapshotBtn{nullptr};

    PhotorealisticRenderConfig m_config;
    PhotorealisticRenderResult m_lastResult;
    int m_currentViewMode{0}; // 0 = Split Slider, 1 = Side by Side, 2 = Cinematic Full

    enum {
        ID_HOUSE_BG_PICKER = 13001,
        ID_SCENE_STYLE_CHOICE,
        ID_CONDITIONING_CHOICE,
        ID_VIEW_MODE_CHOICE,
        ID_BLOOM_SLIDER,
        ID_REFLECTION_SLIDER,
        ID_AMBIENT_SLIDER,
        ID_TIMELINE_FRAME_SLIDER,
        ID_RENDER_SNAPSHOT_BTN,
        ID_RENDER_VIDEO_BTN,
        ID_PREVIEW_CANVAS
    };

    wxDECLARE_EVENT_TABLE();
};

} // namespace xLights::AI
