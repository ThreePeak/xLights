#include "src-ui-wx/ai/AICustomPropDesignerDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/dcclient.h>
#include <wx/dc.h>
#include <spdlog/spdlog.h>

namespace xLights::AI {

enum {
    ID_PROP_GENERATE_BTN = 15001,
    ID_PROP_EXPORT_BTN = 15002
};

wxBEGIN_EVENT_TABLE(AICustomPropDesignerDialog, wxDialog)
    EVT_BUTTON(ID_PROP_GENERATE_BTN, AICustomPropDesignerDialog::OnGenerateClick)
    EVT_BUTTON(ID_PROP_EXPORT_BTN, AICustomPropDesignerDialog::OnExportXmlClick)
    EVT_BUTTON(wxID_CANCEL, AICustomPropDesignerDialog::OnCloseClick)
wxEND_EVENT_TABLE()

AICustomPropDesignerDialog::AICustomPropDesignerDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
}

void AICustomPropDesignerDialog::InitUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Left Panel
    wxBoxSizer* leftPanelSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer* descSizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Prop Description"));
    m_descriptionCtrl = new wxTextCtrl(this, wxID_ANY, wxT("Describe your prop..."), wxDefaultPosition, wxSize(280, 100), wxTE_MULTILINE);
    descSizer->GetSizer()->Add(m_descriptionCtrl, 1, wxEXPAND | wxALL, 5);
    
    wxBoxSizer* settingsSizer = new wxBoxSizer(wxHORIZONTAL);
    settingsSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Node Count:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_nodeCountSpin = new wxSpinCtrl(this, wxID_ANY, wxT("50"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 512, 50);
    settingsSizer->Add(m_nodeCountSpin, 1, wxEXPAND);
    descSizer->GetSizer()->Add(settingsSizer, 0, wxEXPAND | wxALL, 5);
    
    wxBoxSizer* presetSizer = new wxBoxSizer(wxHORIZONTAL);
    presetSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Shape Preset:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    wxArrayString presets;
    presets.Add(wxT("Circle"));
    presets.Add(wxT("Grid"));
    presets.Add(wxT("Star"));
    presets.Add(wxT("Mega Tree"));
    presets.Add(wxT("Candy Cane"));
    presets.Add(wxT("Custom"));
    m_shapePresetChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, presets);
    m_shapePresetChoice->SetSelection(0);
    presetSizer->Add(m_shapePresetChoice, 1, wxEXPAND);
    descSizer->GetSizer()->Add(presetSizer, 0, wxEXPAND | wxALL, 5);
    
    leftPanelSizer->Add(descSizer, 1, wxEXPAND | wxALL, 5);
    topSizer->Add(leftPanelSizer, 0, wxEXPAND | wxALL, 5);

    // Right Panel
    wxStaticBoxSizer* canvasSizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Node Preview Canvas"));
    m_canvasPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
    m_canvasPanel->SetBackgroundStyle(wxBG_STYLE_PAINT);
    m_canvasPanel->Bind(wxEVT_PAINT, &AICustomPropDesignerDialog::OnPaint, this);
    canvasSizer->GetSizer()->Add(m_canvasPanel, 1, wxEXPAND | wxALL, 5);
    topSizer->Add(canvasSizer, 1, wxEXPAND | wxALL, 5);
    
    mainSizer->Add(topSizer, 1, wxEXPAND | wxALL, 5);

    m_statusLabel = new wxStaticText(this, wxID_ANY, wxT("Ready"));
    mainSizer->Add(m_statusLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_generateBtn = new wxButton(this, ID_PROP_GENERATE_BTN, wxT("Generate Layout"));
    m_exportXmlBtn = new wxButton(this, ID_PROP_EXPORT_BTN, wxT("Export Model XML"));
    m_closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));
    
    btnSizer->Add(m_generateBtn, 0, wxALL, 5);
    btnSizer->Add(m_exportXmlBtn, 0, wxALL, 5);
    btnSizer->AddStretchSpacer();
    btnSizer->Add(m_closeBtn, 0, wxALL, 5);
    
    mainSizer->Add(btnSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
    Layout();
    Center();
}

void AICustomPropDesignerDialog::OnGenerateClick(wxCommandEvent& event) {
    CustomPropDesignerAI ai;
    wxString desc = m_descriptionCtrl->GetValue();
    int nodeCount = m_nodeCountSpin->GetValue();
    
    m_lastNodes = ai.GenerateNodeLayout(desc.ToStdString(), nodeCount);
    m_canvasPanel->Refresh();
    
    m_statusLabel->SetLabel(wxString::Format(wxT("%zu nodes generated"), m_lastNodes.size()));
    spdlog::info("AICustomPropDesignerDialog: {} nodes generated", m_lastNodes.size());
}

void AICustomPropDesignerDialog::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(m_canvasPanel);
    wxSize canvasSize = m_canvasPanel->GetClientSize();
    
    dc.SetBrush(wxBrush(wxColour(30, 30, 30)));
    dc.DrawRectangle(0, 0, canvasSize.GetWidth(), canvasSize.GetHeight());
    
    dc.SetBrush(wxBrush(wxColour(255, 200, 0)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetTextForeground(*wxWHITE);
    wxFont font(7, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(font);
    
    int canvasW = canvasSize.GetWidth();
    int canvasH = canvasSize.GetHeight();
    
    for (const auto& node : m_lastNodes) {
        int x = static_cast<int>((node.x / 100.0) * canvasW);
        int y = static_cast<int>((node.y / 100.0) * canvasH);
        
        dc.DrawCircle(x, y, 4);
        dc.DrawText(wxString(node.label), x + 6, y - 6);
    }
}

void AICustomPropDesignerDialog::OnExportXmlClick(wxCommandEvent& event) {
    if (m_lastNodes.empty()) {
        wxMessageBox(wxT("No nodes to export."), wxT("Export Error"), wxICON_WARNING | wxOK);
        return;
    }
    
    wxFileDialog saveFileDialog(this, wxT("Save Model XML"), wxT(""), wxT("custom_prop.xmodel"), wxT("xLights Model files (*.xmodel)|*.xmodel"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) return;

    CustomPropDesignerAI ai;
    std::string xmlStr = ai.ExportToXLightsModelXML(m_lastNodes, m_descriptionCtrl->GetValue().ToStdString());
    
    wxFile file;
    if (file.Open(saveFileDialog.GetPath(), wxFile::write)) {
        file.Write(xmlStr);
        file.Close();
        spdlog::info("AICustomPropDesignerDialog: Exported XML to {}", saveFileDialog.GetPath().ToStdString());
    } else {
        spdlog::error("AICustomPropDesignerDialog: Failed to open file for writing model XML");
    }
}

void AICustomPropDesignerDialog::OnCloseClick(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
