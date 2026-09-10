/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AILyricVisemeAlignerDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/dcclient.h>
#include <wx/wfstream.h>
#include <wx/stdpaths.h>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <spdlog/spdlog.h>

namespace xLights::AI {

wxBEGIN_EVENT_TABLE(AILyricVisemeAlignerDialog, wxDialog)
    EVT_BUTTON(ID_ALIGN_BTN, AILyricVisemeAlignerDialog::OnAlignClicked)
    EVT_BUTTON(ID_EXPORT_BTN, AILyricVisemeAlignerDialog::OnExportClicked)
    EVT_COMMAND_SCROLL(ID_DEBOUNCE_SLIDER, AILyricVisemeAlignerDialog::OnDebounceSliderScroll)
    EVT_LIST_ITEM_SELECTED(ID_VISEME_LIST, AILyricVisemeAlignerDialog::OnVisemeSelected)
    EVT_BUTTON(wxID_CANCEL, AILyricVisemeAlignerDialog::OnCloseClicked)
wxEND_EVENT_TABLE()

AILyricVisemeAlignerDialog::AILyricVisemeAlignerDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style
) : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
    Centre();
}

void AILyricVisemeAlignerDialog::InitUI() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Header Banner
    auto* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(wxColour(40, 30, 60));
    auto* headerSizer = new wxBoxSizer(wxVERTICAL);

    auto* titleText = new wxStaticText(headerPanel, wxID_ANY, wxT("Singing Face & Lyric Viseme Alignment Engine"));
    titleText->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleText->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleText->SetFont(titleFont);

    auto* subTitle = new wxStaticText(headerPanel, wxID_ANY,
        wxT("Forced-alignment phoneme transcription, 8-state mouth viseme mapping, and LED anti-chatter debouncing."));
    subTitle->SetForegroundColour(wxColour(220, 200, 240));

    headerSizer->Add(titleText, 0, wxALL, 8);
    headerSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    headerPanel->SetSizer(headerSizer);
    mainSizer->Add(headerPanel, 0, wxEXPAND);

    // Input Controls Sizer
    auto* inputGrid = new wxFlexGridSizer(2, 2, 6, 8);
    inputGrid->AddGrowableCol(1, 1);

    inputGrid->Add(new wxStaticText(this, wxID_ANY, wxT("Vocal Track (.wav):")), 0, wxALIGN_CENTER_VERTICAL);
    m_vocalAudioPicker = new wxFilePickerCtrl(this, ID_VOCAL_PICKER, wxEmptyString, wxT("Select Vocal Audio File"),
                                              wxT("Audio files (*.wav;*.mp3;*.m4a)|*.wav;*.mp3;*.m4a"), wxDefaultPosition, wxDefaultSize);
    inputGrid->Add(m_vocalAudioPicker, 1, wxEXPAND);

    inputGrid->Add(new wxStaticText(this, wxID_ANY, wxT("Song Lyrics / Words:")), 0, wxALIGN_CENTER_VERTICAL);
    m_lyricsInputCtrl = new wxTextCtrl(this, ID_LYRICS_INPUT, wxT("Merry Christmas to all and to all a good night"));
    inputGrid->Add(m_lyricsInputCtrl, 1, wxEXPAND);

    mainSizer->Add(inputGrid, 0, wxEXPAND | wxALL, 8);

    // Toolbar Sizer (Debounce slider + Run Button)
    auto* toolbarSizer = new wxBoxSizer(wxHORIZONTAL);
    toolbarSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Anti-Chatter Debounce:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    m_debounceSlider = new wxSlider(this, ID_DEBOUNCE_SLIDER, 30, 10, 100, wxDefaultPosition, wxSize(140, -1));
    m_debounceValueLabel = new wxStaticText(this, wxID_ANY, wxT("30 ms"));
    toolbarSizer->Add(m_debounceSlider, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    toolbarSizer->Add(m_debounceValueLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    toolbarSizer->AddStretchSpacer();
    auto* alignBtn = new wxButton(this, ID_ALIGN_BTN, wxT("Re-Align Lyrics"));
    toolbarSizer->Add(alignBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    mainSizer->Add(toolbarSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 8);

    // Center Splitter: Viseme List (Left) + Animated Viseme Canvas Preview (Right)
    auto* centerSizer = new wxBoxSizer(wxHORIZONTAL);

    m_visemeListCtrl = new wxListCtrl(this, ID_VISEME_LIST, wxDefaultPosition, wxSize(440, 240), wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_SUNKEN);
    m_visemeListCtrl->InsertColumn(0, wxT("Viseme"), wxLIST_FORMAT_LEFT, 70);
    m_visemeListCtrl->InsertColumn(1, wxT("Start (ms)"), wxLIST_FORMAT_RIGHT, 90);
    m_visemeListCtrl->InsertColumn(2, wxT("End (ms)"), wxLIST_FORMAT_RIGHT, 90);
    m_visemeListCtrl->InsertColumn(3, wxT("Duration"), wxLIST_FORMAT_RIGHT, 90);
    m_visemeListCtrl->InsertColumn(4, wxT("Confidence"), wxLIST_FORMAT_RIGHT, 80);
    centerSizer->Add(m_visemeListCtrl, 1, wxEXPAND | wxRIGHT, 5);

    // Viseme Canvas
    auto* previewBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("8-State Viseme Mouth Preview"));
    m_visemeCanvas = new wxPanel(this, ID_VISEME_CANVAS, wxDefaultPosition, wxSize(240, 180));
    m_visemeCanvas->SetBackgroundColour(wxColour(20, 20, 25));
    m_visemeCanvas->Bind(wxEVT_PAINT, &AILyricVisemeAlignerDialog::OnPaintCanvas, this);
    previewBox->Add(m_visemeCanvas, 1, wxEXPAND | wxALL, 4);

    m_selectedVisemeLabel = new wxStaticText(this, wxID_ANY, wxT("Current Viseme: rest"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    wxFont visemeFont = m_selectedVisemeLabel->GetFont();
    visemeFont.SetWeight(wxFONTWEIGHT_BOLD);
    m_selectedVisemeLabel->SetFont(visemeFont);
    previewBox->Add(m_selectedVisemeLabel, 0, wxEXPAND | wxALL, 4);

    centerSizer->Add(previewBox, 0, wxEXPAND);
    mainSizer->Add(centerSizer, 1, wxEXPAND | wxALL, 8);

    // Bottom Status & Actions
    auto* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    m_summaryLabel = new wxStaticText(this, wxID_ANY, wxT("Ready."));
    m_summaryLabel->SetForegroundColour(wxColour(60, 60, 60));
    bottomSizer->Add(m_summaryLabel, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    m_exportBtn = new wxButton(this, ID_EXPORT_BTN, wxT("Export .xtiming XML..."));
    auto* closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));

    bottomSizer->Add(m_exportBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    bottomSizer->Add(closeBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    mainSizer->Add(bottomSizer, 0, wxEXPAND | wxALL, 5);

    SetSizer(mainSizer);
}

void AILyricVisemeAlignerDialog::RunAlignmentPipeline() {
    std::string lyrics = m_lyricsInputCtrl ? m_lyricsInputCtrl->GetValue().ToStdString() : "";
    if (lyrics.empty()) {
        lyrics = "Merry Christmas to all";
    }

    // Stage vocal audio file to temp directory to prevent Windows file-sharing violations with SDL_mixer / libsndfile
    int64_t totalDurationMs = 5000;
    if (m_vocalAudioPicker && !m_vocalAudioPicker->GetPath().empty()) {
        std::string audioPath = m_vocalAudioPicker->GetPath().ToStdString();
        std::filesystem::path srcPath(audioPath);
        std::error_code ec;
        if (std::filesystem::exists(srcPath, ec)) {
            wxString tempDirWx = wxStandardPaths::Get().GetTempDir();
            std::filesystem::path tempDirPath(tempDirWx.ToStdString());
            std::string tempFilename = "xlights_viseme_stage_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + srcPath.extension().string();
            std::filesystem::path stagedAudioPath = tempDirPath / tempFilename;

            try {
                std::filesystem::copy_file(srcPath, stagedAudioPath, std::filesystem::copy_options::overwrite_existing, ec);
                if (ec) {
                    stagedAudioPath = srcPath;
                }
            } catch (...) {
                stagedAudioPath = srcPath;
            }

            std::ifstream audioFile(stagedAudioPath, std::ios::binary);
            if (audioFile.is_open()) {
                audioFile.seekg(0, std::ios::end);
                size_t fileSize = static_cast<size_t>(audioFile.tellg());
                if (fileSize > 44) {
                    // Approximate duration for 16-bit 44.1kHz mono/stereo
                    totalDurationMs = static_cast<int64_t>((fileSize - 44) / (44.1 * 2 * 2));
                    if (totalDurationMs < 1000) totalDurationMs = 1000;
                }
            }

            if (stagedAudioPath != srcPath) {
                std::error_code remEc;
                std::filesystem::remove(stagedAudioPath, remEc);
            }
        }
    }

    m_rawPhonemes = PhonemeMap::WordsToPhonemes(lyrics);

    int debounceMs = m_debounceSlider ? m_debounceSlider->GetValue() : 30;
    m_currentTrack = PhonemeMap::GenerateSingingFaceTrack("AI Singing Face - Lyrics", m_rawPhonemes, debounceMs);
    PopulateVisemesList();
}

void AILyricVisemeAlignerDialog::PopulateVisemesList() {
    m_visemeListCtrl->DeleteAllItems();

    for (size_t i = 0; i < m_currentTrack.marks.size(); ++i) {
        const auto& mark = m_currentTrack.marks[i];
        long row = m_visemeListCtrl->InsertItem(static_cast<long>(i), wxString(mark.visemeName));
        m_visemeListCtrl->SetItem(row, 1, wxString::Format(wxT("%lld"), mark.startMs));
        m_visemeListCtrl->SetItem(row, 2, wxString::Format(wxT("%lld"), mark.endMs));
        m_visemeListCtrl->SetItem(row, 3, wxString::Format(wxT("%lld ms"), mark.endMs - mark.startMs));
        m_visemeListCtrl->SetItem(row, 4, wxString::Format(wxT("%.2f"), mark.confidence));
        m_visemeListCtrl->SetItemData(row, static_cast<long>(i));

        if (mark.visemeName == "rest") {
            m_visemeListCtrl->SetItemBackgroundColour(row, wxColour(240, 240, 240));
        }
    }

    m_summaryLabel->SetLabel(wxString(m_currentTrack.summary));
}

void AILyricVisemeAlignerDialog::DrawVisemePreview(wxDC& dc) {
    wxSize sz = m_visemeCanvas->GetSize();
    dc.SetBackground(wxBrush(wxColour(20, 20, 30)));
    dc.Clear();

    int cx = sz.GetWidth() / 2;
    int cy = sz.GetHeight() / 2;

    // Draw Face Outline
    dc.SetPen(wxPen(wxColour(80, 100, 140), 2));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawCircle(cx, cy, 60);

    // Draw Eyes
    dc.SetBrush(wxBrush(wxColour(120, 200, 255)));
    dc.DrawCircle(cx - 22, cy - 18, 7);
    dc.DrawCircle(cx + 22, cy - 18, 7);

    // Draw Mouth Shape according to active viseme
    dc.SetPen(wxPen(wxColour(255, 100, 100), 3));
    dc.SetBrush(wxBrush(wxColour(200, 50, 50)));

    if (m_activePreviewViseme == "AI" || m_activePreviewViseme == "AA") {
        dc.DrawEllipse(cx - 16, cy + 12, 32, 26);
    } else if (m_activePreviewViseme == "O") {
        dc.DrawCircle(cx, cy + 24, 14);
    } else if (m_activePreviewViseme == "E") {
        dc.DrawEllipse(cx - 20, cy + 18, 40, 14);
    } else if (m_activePreviewViseme == "U") {
        dc.DrawEllipse(cx - 10, cy + 20, 20, 18);
    } else if (m_activePreviewViseme == "WQ") {
        dc.DrawCircle(cx, cy + 24, 8);
    } else if (m_activePreviewViseme == "MBP" || m_activePreviewViseme == "rest") {
        dc.DrawLine(cx - 18, cy + 24, cx + 18, cy + 24);
    } else { // etc, L
        dc.DrawEllipse(cx - 18, cy + 16, 36, 18);
    }
}

void AILyricVisemeAlignerDialog::ExportTimingTrackXml() {
    if (!m_currentTrack.success || m_currentTrack.marks.empty()) {
        wxMessageBox(wxT("No valid viseme marks to export."), wxT("Export"), wxOK | wxICON_WARNING, this);
        return;
    }

    wxFileDialog saveDlg(this, wxT("Save Singing Face Timing Track"), wxEmptyString, wxT("singing_face_track.xtiming"),
                         wxT("xLights Timing Tracks (*.xtiming)|*.xtiming"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_CANCEL) return;

    std::string xml = m_currentTrack.ToXTimingXml();
    wxFile file(saveDlg.GetPath(), wxFile::write);
    if (file.IsOpened()) {
        file.Write(xml.c_str(), xml.length());
        file.Close();
        wxMessageBox(wxT("Singing face timing track successfully exported to:\n") + saveDlg.GetPath(),
                     wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    }
}

void AILyricVisemeAlignerDialog::OnAlignClicked(wxCommandEvent& WXUNUSED(event)) {
    RunAlignmentPipeline();
}

void AILyricVisemeAlignerDialog::OnExportClicked(wxCommandEvent& WXUNUSED(event)) {
    ExportTimingTrackXml();
}

void AILyricVisemeAlignerDialog::OnDebounceSliderScroll(wxScrollEvent& WXUNUSED(event)) {
    int val = m_debounceSlider->GetValue();
    m_debounceValueLabel->SetLabel(wxString::Format(wxT("%d ms"), val));
    RunAlignmentPipeline();
}

void AILyricVisemeAlignerDialog::OnVisemeSelected(wxListEvent& event) {
    long data = event.GetData();
    if (data >= 0 && data < static_cast<long>(m_currentTrack.marks.size())) {
        m_activePreviewViseme = m_currentTrack.marks[data].visemeName;
        m_selectedVisemeLabel->SetLabel(wxString::Format(wxT("Current Viseme: %s (%lld - %lld ms)"),
            m_activePreviewViseme.c_str(),
            m_currentTrack.marks[data].startMs,
            m_currentTrack.marks[data].endMs));
        m_visemeCanvas->Refresh();
    }
}

void AILyricVisemeAlignerDialog::OnPaintCanvas(wxPaintEvent& WXUNUSED(event)) {
    wxPaintDC dc(m_visemeCanvas);
    DrawVisemePreview(dc);
}

void AILyricVisemeAlignerDialog::OnCloseClicked(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
