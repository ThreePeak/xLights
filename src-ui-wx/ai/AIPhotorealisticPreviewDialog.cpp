/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIPhotorealisticPreviewDialog.h"
#include <wx/msgdlg.h>
#include <wx/dcclient.h>
#include <wx/filedlg.h>
#include <spdlog/spdlog.h>

namespace xLights::AI {

wxBEGIN_EVENT_TABLE(AIPhotorealisticPreviewDialog, wxDialog)
    EVT_BUTTON(ID_RENDER_SNAPSHOT_BTN, AIPhotorealisticPreviewDialog::OnRenderSnapshotClicked)
    EVT_BUTTON(ID_RENDER_VIDEO_BTN, AIPhotorealisticPreviewDialog::OnRenderVideoClicked)
    EVT_CHOICE(ID_SCENE_STYLE_CHOICE, AIPhotorealisticPreviewDialog::OnSceneStyleChanged)
    EVT_CHOICE(ID_CONDITIONING_CHOICE, AIPhotorealisticPreviewDialog::OnConditioningChanged)
    EVT_CHOICE(ID_VIEW_MODE_CHOICE, AIPhotorealisticPreviewDialog::OnViewModeChanged)
    EVT_COMMAND_SCROLL(ID_BLOOM_SLIDER, AIPhotorealisticPreviewDialog::OnSliderParamScroll)
    EVT_COMMAND_SCROLL(ID_REFLECTION_SLIDER, AIPhotorealisticPreviewDialog::OnSliderParamScroll)
    EVT_COMMAND_SCROLL(ID_AMBIENT_SLIDER, AIPhotorealisticPreviewDialog::OnSliderParamScroll)
    EVT_COMMAND_SCROLL(ID_TIMELINE_FRAME_SLIDER, AIPhotorealisticPreviewDialog::OnSliderParamScroll)
    EVT_BUTTON(wxID_CANCEL, AIPhotorealisticPreviewDialog::OnCloseClicked)
wxEND_EVENT_TABLE()

AIPhotorealisticPreviewDialog::AIPhotorealisticPreviewDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style
) : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
    UpdatePromptPreview();
    Centre();
}

void AIPhotorealisticPreviewDialog::InitUI() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Header Banner
    auto* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(wxColour(20, 30, 45));
    auto* headerSizer = new wxBoxSizer(wxVERTICAL);

    auto* titleText = new wxStaticText(headerPanel, wxID_ANY, wxT("3D Photorealistic Visualizer (ControlNet / SD Mode)"));
    titleText->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleText->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleText->SetFont(titleFont);

    auto* subTitle = new wxStaticText(headerPanel, wxID_ANY,
        wxT("On-demand high-fidelity rendering blending 3D house layout geometry with neural lighting physics & volumetric bloom."));
    subTitle->SetForegroundColour(wxColour(170, 200, 230));

    headerSizer->Add(titleText, 0, wxALL, 8);
    headerSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    headerPanel->SetSizer(headerSizer);
    mainSizer->Add(headerPanel, 0, wxEXPAND);

    // Main Content Split: Left Controls Panel (320px) + Right Viewport Canvas
    auto* contentSizer = new wxBoxSizer(wxHORIZONTAL);

    // Left Parameter Panel
    auto* leftPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(320, -1));
    auto* leftSizer = new wxBoxSizer(wxVERTICAL);

    // House Image Picker
    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("House Background Photo:")), 0, wxALL, 3);
    m_houseBgPicker = new wxFilePickerCtrl(leftPanel, ID_HOUSE_BG_PICKER, wxEmptyString, wxT("Select House Photo"),
                                           wxT("Image files (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg"), wxDefaultPosition, wxDefaultSize);
    leftSizer->Add(m_houseBgPicker, 0, wxEXPAND | wxBOTTOM, 6);

    // Atmosphere Style
    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("Atmospheric Scene Style:")), 0, wxALL, 3);
    wxArrayString styles;
    styles.Add(wxT("Holiday Twilight"));
    styles.Add(wxT("Deep Winter Midnight"));
    styles.Add(wxT("Crisp Snow Reflection"));
    styles.Add(wxT("Foggy Atmosphere"));
    styles.Add(wxT("Neighborhood Ambient Glow"));
    m_sceneStyleChoice = new wxChoice(leftPanel, ID_SCENE_STYLE_CHOICE, wxDefaultPosition, wxDefaultSize, styles);
    m_sceneStyleChoice->SetSelection(0);
    leftSizer->Add(m_sceneStyleChoice, 0, wxEXPAND | wxBOTTOM, 6);

    // Conditioning Type
    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("ControlNet Conditioning Mode:")), 0, wxALL, 3);
    wxArrayString conds;
    conds.Add(wxT("Depth Map (3D Geometry)"));
    conds.Add(wxT("Line Art (Architectural)"));
    conds.Add(wxT("Canny Edge (Structural)"));
    conds.Add(wxT("Soft Edge / HED"));
    conds.Add(wxT("Semantic Segmentation"));
    m_conditioningChoice = new wxChoice(leftPanel, ID_CONDITIONING_CHOICE, wxDefaultPosition, wxDefaultSize, conds);
    m_conditioningChoice->SetSelection(0);
    leftSizer->Add(m_conditioningChoice, 0, wxEXPAND | wxBOTTOM, 6);

    // Sliders: Bloom, Reflection, Ambient
    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("LED Bloom Intensity:")), 0, wxALL, 2);
    auto* bloomSizer = new wxBoxSizer(wxHORIZONTAL);
    m_bloomSlider = new wxSlider(leftPanel, ID_BLOOM_SLIDER, 120, 0, 200);
    m_bloomValueLabel = new wxStaticText(leftPanel, wxID_ANY, wxT("120%"), wxDefaultPosition, wxSize(40, -1));
    bloomSizer->Add(m_bloomSlider, 1, wxEXPAND);
    bloomSizer->Add(m_bloomValueLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    leftSizer->Add(bloomSizer, 0, wxEXPAND | wxBOTTOM, 4);

    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("Surface Light Bounce:")), 0, wxALL, 2);
    auto* reflSizer = new wxBoxSizer(wxHORIZONTAL);
    m_reflectionSlider = new wxSlider(leftPanel, ID_REFLECTION_SLIDER, 45, 0, 100);
    m_reflectionValueLabel = new wxStaticText(leftPanel, wxID_ANY, wxT("45%"), wxDefaultPosition, wxSize(40, -1));
    reflSizer->Add(m_reflectionSlider, 1, wxEXPAND);
    reflSizer->Add(m_reflectionValueLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    leftSizer->Add(reflSizer, 0, wxEXPAND | wxBOTTOM, 4);

    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("Ambient Street Light:")), 0, wxALL, 2);
    auto* ambSizer = new wxBoxSizer(wxHORIZONTAL);
    m_ambientSlider = new wxSlider(leftPanel, ID_AMBIENT_SLIDER, 15, 0, 100);
    m_ambientValueLabel = new wxStaticText(leftPanel, wxID_ANY, wxT("15%"), wxDefaultPosition, wxSize(40, -1));
    ambSizer->Add(m_ambientSlider, 1, wxEXPAND);
    ambSizer->Add(m_ambientValueLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    leftSizer->Add(ambSizer, 0, wxEXPAND | wxBOTTOM, 6);

    // Prompt Inspector
    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("Generated Neural Prompt:")), 0, wxALL, 2);
    m_promptTextCtrl = new wxTextCtrl(leftPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE | wxTE_READONLY);
    leftSizer->Add(m_promptTextCtrl, 1, wxEXPAND | wxBOTTOM, 6);

    leftPanel->SetSizer(leftSizer);
    contentSizer->Add(leftPanel, 0, wxEXPAND | wxALL, 8);

    // Right Viewport Area
    auto* rightPanel = new wxPanel(this);
    auto* rightSizer = new wxBoxSizer(wxVERTICAL);

    // Viewport Top Toolbar
    auto* viewToolbar = new wxBoxSizer(wxHORIZONTAL);
    viewToolbar->Add(new wxStaticText(rightPanel, wxID_ANY, wxT("Viewport Mode:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
    wxArrayString vmodes;
    vmodes.Add(wxT("Split Slider (OpenGL vs. Photorealistic)"));
    vmodes.Add(wxT("Side-by-Side Comparison"));
    vmodes.Add(wxT("Full Cinematic HD Preview"));
    m_viewModeChoice = new wxChoice(rightPanel, ID_VIEW_MODE_CHOICE, wxDefaultPosition, wxDefaultSize, vmodes);
    m_viewModeChoice->SetSelection(0);
    viewToolbar->Add(m_viewModeChoice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);

    viewToolbar->AddStretchSpacer();

    viewToolbar->Add(new wxStaticText(rightPanel, wxID_ANY, wxT("Timeline Frame:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
    m_timelineFrameSlider = new wxSlider(rightPanel, ID_TIMELINE_FRAME_SLIDER, 0, 0, 300, wxDefaultPosition, wxSize(120, -1));
    m_timelineFrameLabel = new wxStaticText(rightPanel, wxID_ANY, wxT("Frame 0 (0.00s)"), wxDefaultPosition, wxSize(90, -1));
    viewToolbar->Add(m_timelineFrameSlider, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
    viewToolbar->Add(m_timelineFrameLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);

    rightSizer->Add(viewToolbar, 0, wxEXPAND | wxBOTTOM, 4);

    // Preview Canvas
    m_previewCanvas = new wxPanel(rightPanel, ID_PREVIEW_CANVAS, wxDefaultPosition, wxSize(560, 340));
    m_previewCanvas->SetBackgroundColour(wxColour(12, 16, 24));
    m_previewCanvas->Bind(wxEVT_PAINT, &AIPhotorealisticPreviewDialog::OnPaintCanvas, this);
    rightSizer->Add(m_previewCanvas, 1, wxEXPAND | wxALL, 4);

    // Progress Bar & Status
    m_renderProgressBar = new wxGauge(rightPanel, wxID_ANY, 100);
    rightSizer->Add(m_renderProgressBar, 0, wxEXPAND | wxALL, 4);

    m_statusLabel = new wxStaticText(rightPanel, wxID_ANY, wxT("Photorealistic visualizer ready. Choose a frame or render sequence."));
    m_statusLabel->SetForegroundColour(wxColour(80, 80, 80));
    rightSizer->Add(m_statusLabel, 0, wxEXPAND | wxALL, 4);

    rightPanel->SetSizer(rightSizer);
    contentSizer->Add(rightPanel, 1, wxEXPAND | wxALL, 8);

    mainSizer->Add(contentSizer, 1, wxEXPAND);

    // Bottom Action Bar
    auto* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    bottomSizer->AddStretchSpacer();

    m_renderSnapshotBtn = new wxButton(this, ID_RENDER_SNAPSHOT_BTN, wxT("Render Single Snapshot"));
    m_renderVideoBtn = new wxButton(this, ID_RENDER_VIDEO_BTN, wxT("Export Photorealistic Video (.mp4)..."));
    auto* closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));

    bottomSizer->Add(m_renderSnapshotBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    bottomSizer->Add(m_renderVideoBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    bottomSizer->Add(closeBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    mainSizer->Add(bottomSizer, 0, wxEXPAND | wxALL, 5);
    SetSizer(mainSizer);
}

void AIPhotorealisticPreviewDialog::UpdatePromptPreview() {
    auto style = static_cast<AtmosphericSceneStyle>(m_sceneStyleChoice->GetSelection());
    float bloom = static_cast<float>(m_bloomSlider->GetValue()) / 100.0f;
    std::string prompt = PhotorealisticVisualizerRenderer::FormulateSDPrompt(style, bloom);
    m_promptTextCtrl->SetValue(wxString(prompt));
}

void AIPhotorealisticPreviewDialog::RenderCurrentSnapshot() {
    m_config.sceneStyle = static_cast<AtmosphericSceneStyle>(m_sceneStyleChoice->GetSelection());
    m_config.conditioningType = static_cast<ControlNetConditioningType>(m_conditioningChoice->GetSelection());
    m_config.bloomIntensity = static_cast<float>(m_bloomSlider->GetValue()) / 100.0f;
    m_config.surfaceReflection = static_cast<float>(m_reflectionSlider->GetValue()) / 100.0f;
    m_config.ambientStreetLight = static_cast<float>(m_ambientSlider->GetValue()) / 100.0f;
    
    int frame = m_timelineFrameSlider->GetValue();
    m_statusLabel->SetLabel(wxT("Rendering 4K photorealistic snapshot..."));
    m_renderProgressBar->SetValue(50);

    auto result = PhotorealisticVisualizerRenderer::RenderSingleSnapshot(m_config, frame, [this](int pct, const std::string& status) {
        m_renderProgressBar->SetValue(pct);
        m_statusLabel->SetLabel(wxString(status));
    });

    m_renderProgressBar->SetValue(100);
    m_statusLabel->SetLabel(wxString::Format(wxT("Snapshot frame %d rendered successfully to: %s"), frame, result.imagePngPath.c_str()));
    m_previewCanvas->Refresh();
}

void AIPhotorealisticPreviewDialog::RenderFullSequenceVideo() {
    wxFileDialog saveDlg(this, wxT("Save Photorealistic Show Video"), wxEmptyString, wxT("xlights_show_photorealistic.mp4"),
                         wxT("MP4 Video files (*.mp4)|*.mp4"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_CANCEL) return;

    m_config.sceneStyle = static_cast<AtmosphericSceneStyle>(m_sceneStyleChoice->GetSelection());
    m_config.conditioningType = static_cast<ControlNetConditioningType>(m_conditioningChoice->GetSelection());
    m_config.bloomIntensity = static_cast<float>(m_bloomSlider->GetValue()) / 100.0f;
    m_config.surfaceReflection = static_cast<float>(m_reflectionSlider->GetValue()) / 100.0f;
    m_config.ambientStreetLight = static_cast<float>(m_ambientSlider->GetValue()) / 100.0f;
    m_config.startFrame = 0;
    m_config.endFrame = 60; // demo preview range

    m_lastResult = PhotorealisticVisualizerRenderer::RenderSequenceVideo(m_config, [this](int pct, const std::string& status) {
        m_renderProgressBar->SetValue(pct);
        m_statusLabel->SetLabel(wxString(status));
    });

    wxMessageBox(wxString(m_lastResult.summaryMessage), wxT("Video Render Complete"), wxOK | wxICON_INFORMATION, this);
}

void AIPhotorealisticPreviewDialog::DrawPreviewCanvas(wxDC& dc) {
    wxSize sz = m_previewCanvas->GetSize();
    dc.SetBackground(wxBrush(wxColour(12, 16, 24)));
    dc.Clear();

    int w = sz.GetWidth();
    int h = sz.GetHeight();

    // Draw house silhouette & roofline
    dc.SetPen(wxPen(wxColour(30, 45, 65), 2));
    dc.SetBrush(wxBrush(wxColour(20, 28, 40)));
    wxPoint housePts[5] = {
        wxPoint(w * 0.15, h * 0.85),
        wxPoint(w * 0.15, h * 0.45),
        wxPoint(w * 0.50, h * 0.20),
        wxPoint(w * 0.85, h * 0.45),
        wxPoint(w * 0.85, h * 0.85)
    };
    dc.DrawPolygon(5, housePts);

    // Draw Simulated MegaTree with volumetric glow
    dc.SetPen(wxPen(wxColour(0, 255, 180), 2));
    dc.DrawLine(w * 0.30, h * 0.85, w * 0.30, h * 0.40);
    dc.DrawLine(w * 0.22, h * 0.85, w * 0.30, h * 0.40);
    dc.DrawLine(w * 0.38, h * 0.85, w * 0.30, h * 0.40);

    // Draw Volumetric Bloom halo
    dc.SetPen(wxPen(wxColour(0, 255, 200), 1));
    dc.SetBrush(wxBrush(wxColour(0, 200, 255)));
    dc.DrawCircle(w * 0.30, h * 0.40, 12);

    // Draw Roofline RGB pixels
    for (int i = 0; i < 20; ++i) {
        float t = i / 20.0f;
        int px = (w * 0.15) + t * (w * 0.35);
        int py = (h * 0.45) - t * (h * 0.25);
        dc.SetBrush(wxBrush(wxColour(255, 50 + i * 10, 100)));
        dc.DrawCircle(px, py, 4);
    }

    // Overlay View Mode Label
    dc.SetTextForeground(wxColour(200, 220, 255));
    dc.DrawText(wxT("ControlNet Mode: Active | Render Mode: On-Demand"), 10, 10);
}

void AIPhotorealisticPreviewDialog::OnRenderSnapshotClicked(wxCommandEvent& WXUNUSED(event)) {
    RenderCurrentSnapshot();
}

void AIPhotorealisticPreviewDialog::OnRenderVideoClicked(wxCommandEvent& WXUNUSED(event)) {
    RenderFullSequenceVideo();
}

void AIPhotorealisticPreviewDialog::OnSceneStyleChanged(wxCommandEvent& WXUNUSED(event)) {
    UpdatePromptPreview();
    m_previewCanvas->Refresh();
}

void AIPhotorealisticPreviewDialog::OnConditioningChanged(wxCommandEvent& WXUNUSED(event)) {
    UpdatePromptPreview();
}

void AIPhotorealisticPreviewDialog::OnViewModeChanged(wxCommandEvent& WXUNUSED(event)) {
    m_currentViewMode = m_viewModeChoice->GetSelection();
    m_previewCanvas->Refresh();
}

void AIPhotorealisticPreviewDialog::OnSliderParamScroll(wxScrollEvent& WXUNUSED(event)) {
    m_bloomValueLabel->SetLabel(wxString::Format(wxT("%d%%"), m_bloomSlider->GetValue()));
    m_reflectionValueLabel->SetLabel(wxString::Format(wxT("%d%%"), m_reflectionSlider->GetValue()));
    m_ambientValueLabel->SetLabel(wxString::Format(wxT("%d%%"), m_ambientSlider->GetValue()));
    
    int frame = m_timelineFrameSlider->GetValue();
    float timeSec = frame / 30.0f;
    m_timelineFrameLabel->SetLabel(wxString::Format(wxT("Frame %d (%.2fs)"), frame, timeSec));

    UpdatePromptPreview();
    m_previewCanvas->Refresh();
}

void AIPhotorealisticPreviewDialog::OnPaintCanvas(wxPaintEvent& WXUNUSED(event)) {
    wxPaintDC dc(m_previewCanvas);
    DrawPreviewCanvas(dc);
}

void AIPhotorealisticPreviewDialog::OnCloseClicked(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
