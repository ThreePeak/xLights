// Copyright (c) xLights Project
#include "src-ui-wx/ai/AICopilotSpotlightDialog.h"
#include "src-ui-wx/ai/AICustomPropDesignerDialog.h"
#include "src-ui-wx/ai/AISequenceValidatorDialog.h"
#include "src-ui-wx/ai/AIPowerInjectionDialog.h"
#include "src-ui-wx/ai/AIFPPSyncDialog.h"
#include "src-ui-wx/ai/AIDMXAddressDialog.h"
#include "src-ui-wx/ai/AISubmodelDetectorDialog.h"
#include "src-ui-wx/ai/AIAudioChoreographerDialog.h"
#include "src-ui-wx/ai/AILyricVisemeAlignerDialog.h"
#include "src-ui-wx/ai/AIColorPaletteDialog.h"
#include "src-ui-wx/ai/AISparseFSEQOptimizerDialog.h"
#include "src-ui-wx/ai/AIPhoto3DPropReconstructorDialog.h"
#include "src-ui-wx/ai/AIGrayCodePixelMapperDialog.h"
#include "src-ui-wx/ai/AIPacketLossInterpolatorDialog.h"
#include "src-ui-wx/ai/AIThermalSafetyThrottlerDialog.h"
#include "src-ui-wx/ai/AIEngineSettingsDialog.h"
#include "src-ui-wx/ai/AIVideoSequenceEmulatorDialog.h"
#include <spdlog/spdlog.h>
#include <wx/msgdlg.h>
#include <algorithm>

namespace xLights::AI {

enum {
    ID_SEARCH_TEXT = 18001,
    ID_RESULTS_LIST = 18002
};

BEGIN_EVENT_TABLE(AICopilotSpotlightDialog, wxDialog)
    EVT_TEXT(ID_SEARCH_TEXT, AICopilotSpotlightDialog::OnSearchTextChanged)
    EVT_LIST_ITEM_ACTIVATED(ID_RESULTS_LIST, AICopilotSpotlightDialog::OnItemActivated)
    EVT_BUTTON(wxID_CANCEL, AICopilotSpotlightDialog::OnCloseClick)
END_EVENT_TABLE()

AICopilotSpotlightDialog::AICopilotSpotlightDialog(wxWindow* parent, wxWindowID id,
                                                   const wxString& title,
                                                   const wxPoint& pos,
                                                   const wxSize& size,
                                                   long style)
    : wxDialog(parent, id, title, pos, size, style)
{
    InitUI();
    PopulateCommands();
    FilterCommands("");
}

void AICopilotSpotlightDialog::InitUI()
{
    SetMinSize(wxSize(680, 420));
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Header banner with search
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(wxColour(30, 34, 45));
    wxBoxSizer* headerSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* titleTxt = new wxStaticText(headerPanel, wxID_ANY, "⬡ AI Copilot Universal Spotlight");
    titleTxt->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleTxt->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 3);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(titleFont);
    headerSizer->Add(titleTxt, 0, wxALL, 8);

    m_searchCtrl = new wxTextCtrl(headerPanel, ID_SEARCH_TEXT, "", wxDefaultPosition,
                                  wxSize(-1, 32), wxTE_PROCESS_ENTER);
    m_searchCtrl->SetHint("Type to search AI tools, CAD props, diagnostics, audio sync, power, or hardware...");
    m_searchCtrl->Bind(wxEVT_KEY_DOWN, &AICopilotSpotlightDialog::OnSearchKeyDown, this);
    headerSizer->Add(m_searchCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    headerPanel->SetSizer(headerSizer);

    mainSizer->Add(headerPanel, 0, wxEXPAND);

    // Results list
    m_resultsList = new wxListCtrl(this, ID_RESULTS_LIST, wxDefaultPosition, wxDefaultSize,
                                   wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_SUNKEN);
    m_resultsList->InsertColumn(0, "Feature", wxLIST_FORMAT_LEFT, 240);
    m_resultsList->InsertColumn(1, "Category", wxLIST_FORMAT_LEFT, 130);
    m_resultsList->InsertColumn(2, "Description", wxLIST_FORMAT_LEFT, 290);
    mainSizer->Add(m_resultsList, 1, wxEXPAND | wxALL, 6);

    // Bottom hint & button row
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    m_hintLabel = new wxStaticText(this, wxID_ANY, "Tip: Press Enter or double-click to launch | ↑ / ↓ to navigate | Esc to exit");
    m_hintLabel->SetForegroundColour(wxColour(140, 140, 140));
    bottomSizer->Add(m_hintLabel, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);

    wxButton* closeBtn = new wxButton(this, wxID_CANCEL, "Close");
    bottomSizer->Add(closeBtn, 0, wxRIGHT | wxBOTTOM, 6);
    mainSizer->Add(bottomSizer, 0, wxEXPAND);

    SetSizer(mainSizer);
    Layout();
    Center();
    m_searchCtrl->SetFocus();
}

void AICopilotSpotlightDialog::PopulateCommands()
{
    m_allCommands = {
        {1, "🎨", "AI Custom Prop Designer & 3D CAD", "Layout & CAD",
         "Generate, edit, and wire 3D custom props with interactive canvas & LLM prompts",
         {"prop", "custom", "cad", "tree", "matrix", "arch", "designer", "wire", "model", "shape"}},

        {2, "🩺", "AI Sequence Health & Diagnostic Audit", "Diagnostics",
         "Audit sequence for timing gaps, channel overlaps, and performance bottlenecks",
         {"validator", "audit", "health", "diagnostic", "error", "warning", "overlap", "timing", "gap"}},

        {3, "⚡", "AI Power Injection & Voltage Drop Inspector", "Hardware",
         "Simulate Ohm's law voltage drop, wire resistance, amperage, and injection points",
         {"power", "voltage", "drop", "injection", "awg", "amp", "watt", "psu", "wire", "resistance"}},

        {4, "📡", "AI Falcon Player (FPP) Multi-Sync", "Controllers",
         "Analyze controller layout and push channel configurations directly to FPP via REST",
         {"fpp", "falcon", "sync", "remote", "controller", "multisync", "rest", "api", "manifest"}},

        {5, "🔀", "AI DMX / E1.31 Address Conflict Advisor", "Controllers",
         "Detect overlapping universe boundaries and auto-remap fixture address channels",
         {"dmx", "address", "conflict", "universe", "e131", "artnet", "remap", "fixture"}},

        {6, "👁️", "AI Submodel Auto-Detector (SAM Vision)", "Layout & Vision",
         "Discover concentric rings, spokes, and geometric submodels using spatial density",
         {"submodel", "sam", "vision", "cluster", "dbscan", "rings", "spokes", "petals", "face"}},

        {7, "🎵", "AI Audio Stem Choreographer", "Audio & Sequencing",
         "Extract drum/vocal beats and synchronize native effects to audio transients",
         {"audio", "choreographer", "stem", "beat", "drums", "kick", "snare", "vocal", "transient"}},

        {8, "🎤", "AI Lyric Viseme & Singing Face Aligner", "Audio & Sequencing",
         "Forced alignment of audio to 8-state mouth visemes using neural Whisper phonemes",
         {"lyric", "viseme", "singing", "face", "mouth", "whisper", "phoneme", "vocals", "lip"}},

        {9, "🎭", "AI Harmonic Color Palette Generator", "Sequencing & Color",
         "Generate balanced 8-color schemes derived from song mood, genre, and tempo",
         {"palette", "color", "mood", "harmonic", "scheme", "wash", "theme", "genre"}},

        {10, "🚀", "AI Sparse FSEQ Optimizer", "Hardware & Export",
         "Compress sequence channels into sparse format for ESP32 and WiFi controllers",
         {"sparse", "fseq", "esp32", "microcontroller", "compression", "wifi", "bandwidth", "ota"}},

        {11, "📸", "AI 3D Photo Prop Reconstructor", "Layout & CAD",
         "Convert multi-angle physical prop photos into calibrated 3D point cloud models",
         {"photo", "reconstruct", "photogrammetry", "3d", "pointcloud", "mesh", "camera"}},

        {12, "📐", "AI Gray Code Camera Pixel Mapper", "Hardware & Vision",
         "Map physical LED pixel locations to 3D space using binary structured light projection",
         {"graycode", "camera", "pixel", "mapper", "calibration", "structured", "light"}},

        {13, "📶", "AI Packet Loss WiFi Interpolator", "Hardware & Network",
         "Predict and synthesize dropped E1.31 / DDP UDP lighting frames during WiFi jitter",
         {"packet", "loss", "wifi", "udp", "ddp", "interpolate", "jitter", "lag"}},

        {14, "🌡️", "AI Thermal Safety Throttler", "Hardware & Safety",
         "Calculate junction thermal dissipation and apply dynamic current throttling",
         {"thermal", "heat", "safety", "throttle", "temperature", "dissipation", "current"}},

        {15, "⚙️", "AI Engine & Inference Settings", "Configuration",
         "Configure local ONNX models, DirectML / GPU acceleration, and API keys",
         {"settings", "inference", "directml", "gpu", "npu", "onnx", "api", "key", "model", "config"}},

        {16, "🎬", "AI Video Sequence Emulation & Choreographer", "Choreography & Vision",
         "Multimodal sequence video analysis, layout adaptation, pre-flight consultation, and automated sequence generation",
         {"video", "youtube", "clip", "emulate", "film", "recording", "vision", "transcribe", "transduce", "choreo"}}
    };
}

void AICopilotSpotlightDialog::FilterCommands(const wxString& query)
{
    m_filteredCommands.clear();
    std::string q = query.Lower().ToStdString();

    for (const auto& cmd : m_allCommands) {
        if (q.empty()) {
            m_filteredCommands.push_back(cmd);
            continue;
        }

        std::string titleLower = cmd.title;
        std::transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);
        std::string descLower = cmd.description;
        std::transform(descLower.begin(), descLower.end(), descLower.begin(), ::tolower);

        bool matched = (titleLower.find(q) != std::string::npos || descLower.find(q) != std::string::npos);

        if (!matched) {
            for (const auto& kw : cmd.keywords) {
                if (kw.find(q) != std::string::npos || q.find(kw) != std::string::npos) {
                    matched = true;
                    break;
                }
            }
        }

        if (matched) {
            m_filteredCommands.push_back(cmd);
        }
    }

    m_resultsList->DeleteAllItems();
    long row = 0;
    for (const auto& cmd : m_filteredCommands) {
        long idx = m_resultsList->InsertItem(row, wxString::Format("%s  %s", cmd.icon.c_str(), cmd.title.c_str()));
        m_resultsList->SetItem(idx, 1, wxString(cmd.category));
        m_resultsList->SetItem(idx, 2, wxString(cmd.description));
        row++;
    }

    if (m_resultsList->GetItemCount() > 0) {
        m_resultsList->SetItemState(0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                                    wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    }
}

void AICopilotSpotlightDialog::OnSearchTextChanged(wxCommandEvent& event)
{
    FilterCommands(event.GetString());
}

void AICopilotSpotlightDialog::OnSearchKeyDown(wxKeyEvent& event)
{
    int key = event.GetKeyCode();
    if (key == WXK_DOWN || key == WXK_UP) {
        long sel = m_resultsList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (sel == -1 && m_resultsList->GetItemCount() > 0) sel = 0;
        else if (key == WXK_DOWN && sel < m_resultsList->GetItemCount() - 1) sel++;
        else if (key == WXK_UP && sel > 0) sel--;

        if (sel >= 0 && sel < m_resultsList->GetItemCount()) {
            m_resultsList->SetItemState(sel, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                                        wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
            m_resultsList->EnsureVisible(sel);
        }
        return;
    } else if (key == WXK_RETURN || key == WXK_NUMPAD_ENTER) {
        ExecuteSelectedCommand();
        return;
    } else if (key == WXK_ESCAPE) {
        EndModal(wxID_CANCEL);
        return;
    }
    event.Skip();
}

void AICopilotSpotlightDialog::OnItemActivated(wxListEvent& WXUNUSED(event))
{
    ExecuteSelectedCommand();
}

void AICopilotSpotlightDialog::ExecuteSelectedCommand()
{
    long sel = m_resultsList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel < 0 || sel >= (long)m_filteredCommands.size()) {
        if (!m_filteredCommands.empty()) sel = 0;
        else return;
    }

    int cmdId = m_filteredCommands[sel].id;
    spdlog::info("AICopilotSpotlightDialog: Executing command ID {}", cmdId);

    // Hide spotlight dialog before showing modal tool
    Hide();

    wxWindow* p = GetParent();
    switch (cmdId) {
        case 1: {
            AICustomPropDesignerDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 2: {
            AISequenceValidatorDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 3: {
            AIPowerInjectionDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 4: {
            AIFPPSyncDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 5: {
            AIDMXAddressDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 6: {
            AISubmodelDetectorDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 7: {
            AIAudioChoreographerDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 8: {
            AILyricVisemeAlignerDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 9: {
            AIColorPaletteDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 10: {
            AISparseFSEQOptimizerDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 11: {
            AIPhoto3DPropReconstructorDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 12: {
            AIGrayCodePixelMapperDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 13: {
            AIPacketLossInterpolatorDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 14: {
            AIThermalSafetyThrottlerDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 15: {
            AIEngineSettingsDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        case 16: {
            AIVideoSequenceEmulatorDialog dlg(p);
            dlg.ShowModal();
            break;
        }
        default:
            break;
    }

    EndModal(wxID_OK);
}

void AICopilotSpotlightDialog::OnCloseClick(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
