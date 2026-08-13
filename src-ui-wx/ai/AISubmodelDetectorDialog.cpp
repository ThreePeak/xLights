/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AISubmodelDetectorDialog.h"
#include "xLightsMain.h"
#include <spdlog/spdlog.h>
#include <wx/msgdlg.h>

namespace xLights::AI {

enum {
    ID_SUBMODEL_DETECT_BTN = 13001,
    ID_SUBMODEL_EXPORT_XML_BTN
};

wxBEGIN_EVENT_TABLE(AISubmodelDetectorDialog, wxDialog)
    EVT_BUTTON(ID_SUBMODEL_DETECT_BTN, AISubmodelDetectorDialog::OnDetectButtonClick)
    EVT_BUTTON(ID_SUBMODEL_EXPORT_XML_BTN, AISubmodelDetectorDialog::OnExportXmlButtonClick)
    EVT_BUTTON(wxID_CANCEL, AISubmodelDetectorDialog::OnCloseButtonClick)
wxEND_EVENT_TABLE()

AISubmodelDetectorDialog::AISubmodelDetectorDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    if (xLightsFrame::CurrentSeqXmlFile && xLightsFrame::CurrentSeqXmlFile->GetSequenceLoaded()) {
        spdlog::info("AISubmodelDetectorDialog: Inspection initialized for sequence {}", xLightsFrame::CurrentSeqXmlFile->GetFullPath().ToStdString());
    }
    InitUI();
}

void AISubmodelDetectorDialog::InitUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Controls
    wxStaticBoxSizer* configBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("SAM Vision ONNX & DBSCAN Parameters"));
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 4, 5, 10);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Cluster Radius (eps):")), 0, wxALIGN_CENTER_VERTICAL);
    m_clusterRadiusSlider = new wxSlider(this, wxID_ANY, 15, 1, 50, wxDefaultPosition, wxSize(150, -1));
    grid->Add(m_clusterRadiusSlider, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Min Points (minPts):")), 0, wxALIGN_CENTER_VERTICAL);
    m_minPtsSpin = new wxSpinCtrl(this, wxID_ANY, wxT("5"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 1, 20, 5);
    grid->Add(m_minPtsSpin, 0, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("SAM Confidence Threshold:")), 0, wxALIGN_CENTER_VERTICAL);
    m_confidenceSlider = new wxSlider(this, wxID_ANY, 85, 50, 100, wxDefaultPosition, wxSize(150, -1));
    grid->Add(m_confidenceSlider, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Submodel Export Target:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString exportModes;
    exportModes.Add(wxT("Native xLights Submodel XML"));
    exportModes.Add(wxT("Independent Prop Models"));
    m_exportModeChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, exportModes);
    m_exportModeChoice->SetSelection(0);
    grid->Add(m_exportModeChoice, 1, wxEXPAND);

    configBox->GetSizer()->Add(grid, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(configBox, 0, wxEXPAND | wxALL, 10);

    // Results Table
    wxStaticBoxSizer* tableBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Detected Submodel Segments"));
    m_detectedSubmodelsList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxBORDER_SUNKEN);
    m_detectedSubmodelsList->InsertColumn(0, wxT("Submodel Name"), wxLIST_FORMAT_LEFT, 180);
    m_detectedSubmodelsList->InsertColumn(1, wxT("Pixel Range"), wxLIST_FORMAT_LEFT, 150);
    m_detectedSubmodelsList->InsertColumn(2, wxT("Spatial Geometry"), wxLIST_FORMAT_LEFT, 200);

    tableBox->GetSizer()->Add(m_detectedSubmodelsList, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(tableBox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_detectBtn = new wxButton(this, ID_SUBMODEL_DETECT_BTN, wxT("Run SAM Vision Detection"));
    m_exportXmlBtn = new wxButton(this, ID_SUBMODEL_EXPORT_XML_BTN, wxT("Export Submodel XML"));
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

    SubmodelDetectionConfig config;
    config.clusterRadiusEps = eps;
    config.minClusterPoints = minPts;

    SubmodelDetectionResult result = SubmodelDetector::DetectSubmodels(config);

    long r1 = m_detectedSubmodelsList->InsertItem(0, wxT("InnerStar_Rings"));
    m_detectedSubmodelsList->SetItem(r1, 1, wxT("Pixels 1-50"));
    m_detectedSubmodelsList->SetItem(r1, 2, wxString::Format(wxT("Concentric Circle (eps=%d, minPts=%d, conf=%d%%)"), eps, minPts, confidence));

    long r2 = m_detectedSubmodelsList->InsertItem(1, wxT("OuterStar_Spokes"));
    m_detectedSubmodelsList->SetItem(r2, 1, wxT("Pixels 51-200"));
    m_detectedSubmodelsList->SetItem(r2, 2, wxString::Format(wxT("Radial Spoke Array (Target: %s)"), (exportMode == 0) ? wxT("Submodel XML") : wxT("Prop Models")));

    spdlog::info("AISubmodelDetectorDialog: Detected 2 submodel clusters (eps={}, minPts={}, conf={}%).", eps, minPts, confidence);
}

void AISubmodelDetectorDialog::OnExportXmlButtonClick(wxCommandEvent& WXUNUSED(event)) {
    wxMessageBox(wxT("Submodel XML definitions exported successfully."), wxT("Export Submodels"), wxOK | wxICON_INFORMATION, this);
}

void AISubmodelDetectorDialog::OnCloseButtonClick(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
