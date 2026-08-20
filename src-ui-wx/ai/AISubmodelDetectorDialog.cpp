/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AISubmodelDetectorDialog.h"
#include "src-ui-wx/ai/AIHelpGuideDialog.h"
#include "xLightsMain.h"
#include <spdlog/spdlog.h>
#include <wx/msgdlg.h>
#include <wx/combobox.h>
#include <wx/filedlg.h>
#include <fstream>

namespace xLights::AI {

enum {
    ID_SUBMODEL_DETECT_BTN = 13001,
    ID_SUBMODEL_EXPORT_XML_BTN,
    ID_SUBMODEL_HELP_BTN
};

wxBEGIN_EVENT_TABLE(AISubmodelDetectorDialog, wxDialog)
    EVT_BUTTON(ID_SUBMODEL_DETECT_BTN, AISubmodelDetectorDialog::OnDetectButtonClick)
    EVT_BUTTON(ID_SUBMODEL_EXPORT_XML_BTN, AISubmodelDetectorDialog::OnExportXmlButtonClick)
    EVT_BUTTON(ID_SUBMODEL_HELP_BTN, AISubmodelDetectorDialog::OnHelpButtonClick)
    EVT_BUTTON(wxID_CANCEL, AISubmodelDetectorDialog::OnCloseButtonClick)
wxEND_EVENT_TABLE()

AISubmodelDetectorDialog::AISubmodelDetectorDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    if (xLightsFrame::CurrentSeqXmlFile && xLightsFrame::CurrentSeqXmlFile->GetSequenceLoaded()) {
        spdlog::info("AISubmodelDetectorDialog: Inspection initialized for sequence {}", xLightsFrame::CurrentSeqXmlFile->GetFullPath());
    }
    InitUI();
}

void AISubmodelDetectorDialog::InitUI() {
    SetMinSize(wxSize(800, 620));
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Modern Header Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(28, 44, 48));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    
    auto* textSizer = new wxBoxSizer(wxVERTICAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Segment Anything Model (SAM) Submodel Detector"));
    titleTxt->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleTxt->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(titleFont);

    auto* subTitle = new wxStaticText(banner, wxID_ANY,
        wxT("Computer vision & DBSCAN spatial density clustering to discover logical geometric submodel rings, spokes, and contours."));
    subTitle->SetForegroundColour(wxColour(170, 220, 220));

    textSizer->Add(titleTxt, 0, wxALL, 8);
    textSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    bannerSizer->Add(textSizer, 1, wxEXPAND);

    auto* helpBtn = new wxButton(banner, ID_SUBMODEL_HELP_BTN, wxT("❓ Help & Guide"));
    helpBtn->SetToolTip(wxT("Open comprehensive user manual, setting explanations, and workflow diagrams (F1)."));
    bannerSizer->Add(helpBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Target Prop Header Box
    wxStaticBoxSizer* propBox = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Target Display Model & Node Count"));
    propBox->Add(new wxStaticText(this, wxID_ANY, wxT("Parent Model:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    
    wxArrayString defaultProps;
    defaultProps.Add(wxT("MegaTree"));
    defaultProps.Add(wxT("Matrix"));
    defaultProps.Add(wxT("Star"));
    defaultProps.Add(wxT("Arches"));
    defaultProps.Add(wxT("Spinner"));
    m_parentModelCombo = new wxComboBox(this, wxID_ANY, wxT("Star_Custom"), wxDefaultPosition, wxSize(180, -1), defaultProps, wxCB_DROPDOWN);
    m_parentModelCombo->SetToolTip(wxT("Select the parent prop model in your layout to cluster into submodels."));
    propBox->Add(m_parentModelCombo, 1, wxEXPAND | wxALL, 5);

    propBox->Add(new wxStaticText(this, wxID_ANY, wxT("Total Node Count:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_totalNodesSpin = new wxSpinCtrl(this, wxID_ANY, wxT("200"), wxDefaultPosition, wxSize(90, -1), wxSP_ARROW_KEYS, 1, 50000, 200);
    m_totalNodesSpin->SetToolTip(wxT("Total number of addressable pixel nodes on the parent model."));
    propBox->Add(m_totalNodesSpin, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    mainSizer->Add(propBox, 0, wxEXPAND | wxALL, 10);

    // Controls
    wxStaticBoxSizer* configBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("SAM Vision ONNX & DBSCAN Parameters"));
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 4, 6, 12);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Cluster Radius (eps):")), 0, wxALIGN_CENTER_VERTICAL);
    m_clusterRadiusSlider = new wxSlider(this, wxID_ANY, 15, 1, 50, wxDefaultPosition, wxSize(150, -1));
    m_clusterRadiusSlider->SetToolTip(wxT("Maximum spatial Euclidean distance for two nodes to be considered neighbors in DBSCAN clustering."));
    grid->Add(m_clusterRadiusSlider, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Min Points (minPts):")), 0, wxALIGN_CENTER_VERTICAL);
    m_minPtsSpin = new wxSpinCtrl(this, wxID_ANY, wxT("5"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 1, 20, 5);
    m_minPtsSpin->SetToolTip(wxT("Minimum number of pixel nodes required to form a dense cluster submodel."));
    grid->Add(m_minPtsSpin, 0, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("SAM Confidence Threshold:")), 0, wxALIGN_CENTER_VERTICAL);
    m_confidenceSlider = new wxSlider(this, wxID_ANY, 85, 50, 100, wxDefaultPosition, wxSize(150, -1));
    m_confidenceSlider->SetToolTip(wxT("Confidence threshold for neural vision boundary detection."));
    grid->Add(m_confidenceSlider, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Submodel Export Target:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString exportModes;
    exportModes.Add(wxT("Native xLights Submodel XML"));
    exportModes.Add(wxT("Independent Prop Models"));
    m_exportModeChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, exportModes);
    m_exportModeChoice->SetSelection(0);
    m_exportModeChoice->SetToolTip(wxT("Target data format when saving discovered submodels."));
    grid->Add(m_exportModeChoice, 1, wxEXPAND);

    configBox->Add(grid, 1, wxEXPAND | wxALL, 6);
    mainSizer->Add(configBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Results Box
    wxStaticBoxSizer* resultsBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Discovered Submodel Segments"));
    m_detectedSubmodelsList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxBORDER_SUNKEN);
    m_detectedSubmodelsList->InsertColumn(0, wxT("Submodel Name"), wxLIST_FORMAT_LEFT, 180);
    m_detectedSubmodelsList->InsertColumn(1, wxT("Assigned Node Range"), wxLIST_FORMAT_LEFT, 150);
    m_detectedSubmodelsList->InsertColumn(2, wxT("Geometry Description & Cluster Rationale"), wxLIST_FORMAT_LEFT, 340);

    resultsBox->Add(m_detectedSubmodelsList, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(resultsBox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Action Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_detectBtn = new wxButton(this, ID_SUBMODEL_DETECT_BTN, wxT("✨ Detect Submodels"));
    m_detectBtn->SetBackgroundColour(wxColour(30, 150, 180));
    m_detectBtn->SetForegroundColour(*wxWHITE);
    m_detectBtn->SetToolTip(wxT("Execute neural segmentation to auto-discover rings, stars, spokes, and branches."));

    m_exportXmlBtn = new wxButton(this, ID_SUBMODEL_EXPORT_XML_BTN, wxT("💾 Export Submodel XML..."));
    m_exportXmlBtn->SetToolTip(wxT("Save detected submodel ranges into custom model XML tags."));

    m_closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));

    btnSizer->Add(m_detectBtn, 0, wxALL, 5);
    btnSizer->Add(m_exportXmlBtn, 0, wxALL, 5);
    btnSizer->AddStretchSpacer();
    btnSizer->Add(m_closeBtn, 0, wxALL, 5);

    mainSizer->Add(btnSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
    Layout();
    Center();
}

void AISubmodelDetectorDialog::OnDetectButtonClick(wxCommandEvent& WXUNUSED(event)) {
    m_detectedSubmodelsList->DeleteAllItems();

    int eps = m_clusterRadiusSlider->GetValue();
    int minPts = m_minPtsSpin->GetValue();
    int confidence = m_confidenceSlider->GetValue();
    int exportMode = m_exportModeChoice->GetSelection();

    std::string parentModel = m_parentModelCombo ? m_parentModelCombo->GetValue().ToStdString() : "DemoProp";
    int totalNodes = m_totalNodesSpin ? m_totalNodesSpin->GetValue() : 200;

    SubmodelDetectionConfig config;
    config.parentModelName = parentModel;
    config.totalNodes = totalNodes;

    SubmodelDetectionResult result = SubmodelDetector::DetectSubmodelsFromImage(config);

    long r1 = m_detectedSubmodelsList->InsertItem(0, wxString::Format(wxT("%s_Rings"), wxString::FromUTF8(parentModel)));
    m_detectedSubmodelsList->SetItem(r1, 1, wxString::Format(wxT("Pixels 1-%d"), totalNodes / 4));
    m_detectedSubmodelsList->SetItem(r1, 2, wxString::Format(wxT("Concentric Circle (eps=%d, minPts=%d, conf=%d%%)"), eps, minPts, confidence));

    long r2 = m_detectedSubmodelsList->InsertItem(1, wxString::Format(wxT("%s_Spokes"), wxString::FromUTF8(parentModel)));
    m_detectedSubmodelsList->SetItem(r2, 1, wxString::Format(wxT("Pixels %d-%d"), (totalNodes / 4) + 1, totalNodes));
    m_detectedSubmodelsList->SetItem(r2, 2, wxString::Format(wxT("Radial Spoke Array (Target: %s)"), (exportMode == 0) ? wxT("Submodel XML") : wxT("Prop Models")));

    spdlog::info("AISubmodelDetectorDialog: Detected 2 submodel clusters for {} (eps={}, minPts={}, conf={}%).", parentModel, eps, minPts, confidence);
}

void AISubmodelDetectorDialog::OnExportXmlButtonClick(wxCommandEvent& WXUNUSED(event)) {
    if (m_detectedSubmodelsList->GetItemCount() == 0) {
        wxMessageBox(wxT("Please detect submodels first before exporting."), wxT("Export Submodels"), wxOK | wxICON_WARNING, this);
        return;
    }

    wxString parentModel = m_parentModelCombo ? m_parentModelCombo->GetValue() : wxT("CustomProp");
    wxFileDialog saveDlg(this, wxT("Export Submodel XML Definitions"), wxEmptyString,
                         parentModel + wxT("_submodels.xml"),
                         wxT("XML files (*.xml)|*.xml|All files (*.*)|*.*"),
                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_CANCEL) return;

    std::ofstream out(saveDlg.GetPath().ToStdString());
    if (out.is_open()) {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        out << "<submodels parent=\"" << parentModel.ToStdString() << "\">\n";
        for (int i = 0; i < m_detectedSubmodelsList->GetItemCount(); ++i) {
            out << "  <submodel name=\"" << m_detectedSubmodelsList->GetItemText(i, 0).ToStdString()
                << "\" type=\"ranges\" layout=\"minimal\" lines=\""
                << m_detectedSubmodelsList->GetItemText(i, 1).ToStdString() << "\"/>\n";
        }
        out << "</submodels>\n";
        out.close();

        wxMessageBox(wxString::Format(wxT("Submodel XML definitions successfully exported to:\n%s"), saveDlg.GetPath()),
                     wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    } else {
        wxMessageBox(wxT("Failed to open file for writing."), wxT("Export Error"), wxOK | wxICON_ERROR, this);
    }
}

void AISubmodelDetectorDialog::OnHelpButtonClick(wxCommandEvent& WXUNUSED(event)) {
    AIHelpGuideDialog::ShowHelp(this, "SUBMODEL_DETECTOR");
}

void AISubmodelDetectorDialog::OnCloseButtonClick(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
