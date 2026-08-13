/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIModelMappingWizard.h"
#include <spdlog/spdlog.h>

namespace xLights::AI {

AIModelMappingWizard::AIModelMappingWizard(wxWindow* parent, wxWindowID id, const wxString& title)
    : wxWizard(parent, id, title) {

    // Page 1: Select Vendor Sequence
    m_page1 = new wxWizardPageSimple(this);
    wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    sizer1->Add(new wxStaticText(m_page1, wxID_ANY, wxT("Step 1: Select Vendor Sequence File")), 0, wxALL, 10);

    m_vendorSeqCtrl = new wxTextCtrl(m_page1, wxID_ANY, wxT("vendor_sequence.xsq"), wxDefaultPosition, wxDefaultSize);
    sizer1->Add(m_vendorSeqCtrl, 0, wxEXPAND | wxALL, 10);

    // Fine Control Parameters Section
    wxStaticBoxSizer* configBox = new wxStaticBoxSizer(wxVERTICAL, m_page1, wxT("4-Pass Alignment & Confidence Thresholds"));
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 2, 5, 10);

    grid->Add(new wxStaticText(m_page1, wxID_ANY, wxT("Fuzzy Match Confidence Threshold:")), 0, wxALIGN_CENTER_VERTICAL);
    m_confidenceThresholdSlider = new wxSlider(m_page1, wxID_ANY, 80, 50, 100, wxDefaultPosition, wxSize(200, -1));
    grid->Add(m_confidenceThresholdSlider, 1, wxEXPAND);

    grid->Add(new wxStaticText(m_page1, wxID_ANY, wxT("Custom Taxonomy Dictionary:")), 0, wxALIGN_CENTER_VERTICAL);
    m_taxonomyEditorBtn = new wxButton(m_page1, wxID_ANY, wxT("Edit Taxonomy Rules..."));
    grid->Add(m_taxonomyEditorBtn, 0, wxEXPAND);

    configBox->GetSizer()->Add(grid, 0, wxEXPAND | wxALL, 5);

    wxStaticBoxSizer* passBox = new wxStaticBoxSizer(wxVERTICAL, m_page1, wxT("Active Matching Passes"));
    m_pass1ExactChk = new wxCheckBox(m_page1, wxID_ANY, wxT("Pass 1: Exact String Token Matching"));
    m_pass1ExactChk->SetValue(true);
    m_pass2FuzzyChk = new wxCheckBox(m_page1, wxID_ANY, wxT("Pass 2: Levenshtein & Jaro-Winkler Distance Matching"));
    m_pass2FuzzyChk->SetValue(true);
    m_pass3SemanticChk = new wxCheckBox(m_page1, wxID_ANY, wxT("Pass 3: ONNX Vector Embedding Cosine Similarity"));
    m_pass3SemanticChk->SetValue(true);
    m_pass4TaxonomyChk = new wxCheckBox(m_page1, wxID_ANY, wxT("Pass 4: Taxonomy Dictionary Override Matching"));
    m_pass4TaxonomyChk->SetValue(true);

    passBox->GetSizer()->Add(m_pass1ExactChk, 0, wxALL, 4);
    passBox->GetSizer()->Add(m_pass2FuzzyChk, 0, wxALL, 4);
    passBox->GetSizer()->Add(m_pass3SemanticChk, 0, wxALL, 4);
    passBox->GetSizer()->Add(m_pass4TaxonomyChk, 0, wxALL, 4);

    configBox->GetSizer()->Add(passBox, 0, wxEXPAND | wxALL, 5);
    sizer1->Add(configBox, 0, wxEXPAND | wxALL, 10);
    m_page1->SetSizer(sizer1);

    // Page 2: 4-Pass LLM Auto-Mapping Results
    m_page2 = new wxWizardPageSimple(this);
    wxBoxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);
    sizer2->Add(new wxStaticText(m_page2, wxID_ANY, wxT("Step 2: AI Model Channel Alignment Preview")), 0, wxALL, 10);

    m_mappingsList = new wxListCtrl(m_page2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxBORDER_SUNKEN);
    m_mappingsList->InsertColumn(0, wxT("Vendor Model Name"), wxLIST_FORMAT_LEFT, 180);
    m_mappingsList->InsertColumn(1, wxT("Matched User Prop"), wxLIST_FORMAT_LEFT, 180);
    m_mappingsList->InsertColumn(2, wxT("Confidence Score"), wxLIST_FORMAT_LEFT, 120);

    sizer2->Add(m_mappingsList, 1, wxEXPAND | wxALL, 10);
    m_page2->SetSizer(sizer2);

    wxWizardPageSimple::Chain(m_page1, m_page2);

    // Populate Mock Data
    long r1 = m_mappingsList->InsertItem(0, wxT("Vendor_MegaTree_360"));
    m_mappingsList->SetItem(r1, 1, wxT("MyDisplay_MegaTree"));
    m_mappingsList->SetItem(r1, 2, wxT("98.5% (Exact Match)"));

    long r2 = m_mappingsList->InsertItem(1, wxT("Vendor_Matrix_Grid"));
    m_mappingsList->SetItem(r2, 1, wxT("Garage_Matrix"));
    m_mappingsList->SetItem(r2, 2, wxT("94.2% (Fuzzy Match)"));
}

bool AIModelMappingWizard::RunWizardUI() {
    return RunWizard(m_page1);
}

} // namespace xLights::AI
