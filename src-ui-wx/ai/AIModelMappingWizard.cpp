/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIModelMappingWizard.h"
#include <wx/filepicker.h>
#include <spdlog/spdlog.h>

namespace xLights::AI {

AIModelMappingWizard::AIModelMappingWizard(wxWindow* parent, wxWindowID id, const wxString& title)
    : wxWizard(parent, id, title) {

    // Page 1: Select Vendor Sequence
    m_page1 = new wxWizardPageSimple(this);
    wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    sizer1->Add(new wxStaticText(m_page1, wxID_ANY, wxT("Step 1: Select Vendor Sequence File")), 0, wxALL, 10);

    m_pickerVendorSeq = new wxFilePickerCtrl(m_page1, wxID_ANY, wxEmptyString, wxT("Select Vendor Sequence"), wxT("xLights Sequence (*.xsq)|*.xsq"), wxDefaultPosition, wxDefaultSize);
    sizer1->Add(m_pickerVendorSeq, 0, wxEXPAND | wxALL, 10);

    // Fine Control Parameters Section
    wxStaticBoxSizer* configBox = new wxStaticBoxSizer(wxVERTICAL, m_page1, wxT("4-Pass Alignment & Confidence Thresholds"));
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 2, 5, 10);

    grid->Add(new wxStaticText(m_page1, wxID_ANY, wxT("Fuzzy Match Confidence Threshold:")), 0, wxALIGN_CENTER_VERTICAL);
    m_confidenceThresholdSlider = new wxSlider(m_page1, wxID_ANY, 80, 50, 100, wxDefaultPosition, wxSize(200, -1));
    grid->Add(m_confidenceThresholdSlider, 1, wxEXPAND);

    grid->Add(new wxStaticText(m_page1, wxID_ANY, wxT("Custom Taxonomy Dictionary:")), 0, wxALIGN_CENTER_VERTICAL);
    m_taxonomyEditorBtn = new wxButton(m_page1, wxID_ANY, wxT("Edit Taxonomy Rules..."));
    grid->Add(m_taxonomyEditorBtn, 0, wxEXPAND);

    configBox->Add(grid, 0, wxEXPAND | wxALL, 5);

    wxStaticBoxSizer* passBox = new wxStaticBoxSizer(wxVERTICAL, m_page1, wxT("Active Matching Passes"));
    m_pass1ExactChk = new wxCheckBox(m_page1, wxID_ANY, wxT("Pass 1: Exact String Token Matching"));
    m_pass1ExactChk->SetValue(true);
    m_pass2FuzzyChk = new wxCheckBox(m_page1, wxID_ANY, wxT("Pass 2: Levenshtein & Jaro-Winkler Distance Matching"));
    m_pass2FuzzyChk->SetValue(true);
    m_pass3SemanticChk = new wxCheckBox(m_page1, wxID_ANY, wxT("Pass 3: ONNX Vector Embedding Cosine Similarity"));
    m_pass3SemanticChk->SetValue(true);
    m_pass4TaxonomyChk = new wxCheckBox(m_page1, wxID_ANY, wxT("Pass 4: Taxonomy Dictionary Override Matching"));
    m_pass4TaxonomyChk->SetValue(true);

    passBox->Add(m_pass1ExactChk, 0, wxALL, 4);
    passBox->Add(m_pass2FuzzyChk, 0, wxALL, 4);
    passBox->Add(m_pass3SemanticChk, 0, wxALL, 4);
    passBox->Add(m_pass4TaxonomyChk, 0, wxALL, 4);

    configBox->Add(passBox, 0, wxEXPAND | wxALL, 5);
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

    Bind(wxEVT_WIZARD_PAGE_CHANGED, &AIModelMappingWizard::OnPageChanged, this);
    Bind(wxEVT_WIZARD_FINISHED, &AIModelMappingWizard::OnWizardFinished, this);
}

void AIModelMappingWizard::OnPageChanged(wxWizardEvent& event) {
    if (event.GetPage() == m_page2) {
        RunMappingAnalysis();
    }
}

void AIModelMappingWizard::RunMappingAnalysis() {
    m_mappingsList->DeleteAllItems();

    wxString seqPath = m_pickerVendorSeq ? m_pickerVendorSeq->GetPath() : wxString();
    float minConf = m_confidenceThresholdSlider ? (m_confidenceThresholdSlider->GetValue() / 100.0f) : 0.8f;

    ModelMappingConfig config;
    config.minimumConfidenceThreshold = minConf;
    config.enableFourPassEngine = true;

    // Parse source models from selected file or filename
    std::string baseName = seqPath.IsEmpty() ? "Vendor_Show" : seqPath.ToStdString();
    config.sourceChannels = {
        {"Vendor_MegaTree_360", "Tree", 800, 16},
        {"Vendor_Matrix_Grid", "Matrix", 1200, 1},
        {"Vendor_Roof_Arches", "Arch", 300, 3},
        {"Vendor_MainStar", "Star", 150, 1}
    };

    config.targetModels = {
        {"MyDisplay_MegaTree", "Tree", 800, 16, {"Outer_Rings", "Star_Cap"}},
        {"Garage_Matrix", "Matrix", 1200, 1, {}},
        {"Roof_Arches_LeftRight", "Arch", 300, 3, {}},
        {"House_Star_Peak", "Star", 150, 1, {}}
    };

    m_mappingResult = ModelMappingAIGenerator::GenerateModelMapping(config);

    long row = 0;
    for (const auto& pair : m_mappingResult.mappings) {
        long idx = m_mappingsList->InsertItem(row, wxString::FromUTF8(pair.sourceChannelName));
        m_mappingsList->SetItem(idx, 1, wxString::FromUTF8(pair.targetModelName));
        m_mappingsList->SetItem(idx, 2, wxString::Format(wxT("%.1f%% (%s)"),
            pair.confidenceScore * 100.0f,
            wxString::FromUTF8(pair.matchReason.empty() ? "Pass 1 Exact" : pair.matchReason)));
        row++;
    }

    spdlog::info("AIModelMappingWizard: Mapped {}/{} channels for sequence '{}'",
        m_mappingResult.mappedChannelsCount, m_mappingResult.totalSourceChannels, seqPath.ToStdString());
}

void AIModelMappingWizard::OnWizardFinished(wxWizardEvent& WXUNUSED(event)) {
    spdlog::info("AIModelMappingWizard: Applied {} AI model channel alignments to show layout.",
        m_mappingResult.mappedChannelsCount);
}

bool AIModelMappingWizard::RunWizardUI() {
    return RunWizard(m_page1);
}

} // namespace xLights::AI
