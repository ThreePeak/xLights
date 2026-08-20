/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AISequenceRemapDialog.h"
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/wfstream.h>
#include <wx/dir.h>
#include <spdlog/spdlog.h>

namespace xLights::AI {

wxBEGIN_EVENT_TABLE(AISequenceRemapDialog, wxDialog)
    EVT_FILEPICKER_CHANGED(ID_SOURCE_SEQ_PICKER, AISequenceRemapDialog::OnSourceFileChanged)
    EVT_BUTTON(ID_GENERATE_PLAN_BTN, AISequenceRemapDialog::OnGeneratePlanClicked)
    EVT_BUTTON(ID_APPLY_REMAP_BTN, AISequenceRemapDialog::OnApplyClicked)
    EVT_BUTTON(ID_BATCH_REMAP_BTN, AISequenceRemapDialog::OnBatchClicked)
    EVT_COMMAND_SCROLL(ID_THRESHOLD_SLIDER, AISequenceRemapDialog::OnThresholdChanged)
    EVT_LIST_ITEM_SELECTED(ID_MAPPINGS_LIST, AISequenceRemapDialog::OnMappingSelected)
    EVT_BUTTON(wxID_CANCEL, AISequenceRemapDialog::OnCloseClicked)
wxEND_EVENT_TABLE()

AISequenceRemapDialog::AISequenceRemapDialog(
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

void AISequenceRemapDialog::InitUI() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Header Banner
    auto* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(wxColour(25, 45, 60));
    auto* headerSizer = new wxBoxSizer(wxVERTICAL);

    auto* titleText = new wxStaticText(headerPanel, wxID_ANY, wxT("Cross-Display Sequence Remapping Copilot"));
    titleText->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleText->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleText->SetFont(titleFont);

    auto* subTitle = new wxStaticText(headerPanel, wxID_ANY,
        wxT("16-Dimensional vector embedding & cosine similarity model alignment for sequence porting."));
    subTitle->SetForegroundColour(wxColour(180, 210, 230));

    headerSizer->Add(titleText, 0, wxALL, 8);
    headerSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    headerPanel->SetSizer(headerSizer);
    mainSizer->Add(headerPanel, 0, wxEXPAND);

    // Source File & Threshold Toolbar
    auto* toolbarSizer = new wxBoxSizer(wxHORIZONTAL);
    toolbarSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Source Sequence:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    m_sourceSeqPicker = new wxFilePickerCtrl(this, ID_SOURCE_SEQ_PICKER, wxEmptyString, wxT("Select Source Sequence (.xsq)"),
                                             wxT("xLights Sequences (*.xsq)|*.xsq"), wxDefaultPosition, wxSize(300, -1));
    toolbarSizer->Add(m_sourceSeqPicker, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    toolbarSizer->AddStretchSpacer();

    toolbarSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Match Threshold:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    m_thresholdSlider = new wxSlider(this, ID_THRESHOLD_SLIDER, 50, 10, 95, wxDefaultPosition, wxSize(120, -1));
    m_thresholdValueLabel = new wxStaticText(this, wxID_ANY, wxT("50%"));
    toolbarSizer->Add(m_thresholdSlider, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    toolbarSizer->Add(m_thresholdValueLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    auto* planBtn = new wxButton(this, ID_GENERATE_PLAN_BTN, wxT("Recalculate Remap"));
    toolbarSizer->Add(planBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    mainSizer->Add(toolbarSizer, 0, wxEXPAND | wxALL, 5);

    // Model Mapping List Grid
    m_mappingsListCtrl = new wxListCtrl(this, ID_MAPPINGS_LIST, wxDefaultPosition, wxSize(-1, 240), wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_SUNKEN);
    m_mappingsListCtrl->InsertColumn(0, wxT("Source Model"), wxLIST_FORMAT_LEFT, 180);
    m_mappingsListCtrl->InsertColumn(1, wxT("Target Match"), wxLIST_FORMAT_LEFT, 180);
    m_mappingsListCtrl->InsertColumn(2, wxT("Similarity"), wxLIST_FORMAT_RIGHT, 90);
    m_mappingsListCtrl->InsertColumn(3, wxT("Confidence"), wxLIST_FORMAT_LEFT, 110);
    m_mappingsListCtrl->InsertColumn(4, wxT("Rationale"), wxLIST_FORMAT_LEFT, 320);
    mainSizer->Add(m_mappingsListCtrl, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    // Details & Submodel Alignment Panel
    auto* detailBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Selected Model & Submodel Alignment"));
    m_rationaleTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 60), wxTE_MULTILINE | wxTE_READONLY);
    m_submodelsTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 60), wxTE_MULTILINE | wxTE_READONLY);
    detailBox->Add(m_rationaleTextCtrl, 0, wxEXPAND | wxALL, 3);
    detailBox->Add(m_submodelsTextCtrl, 0, wxEXPAND | wxALL, 3);
    mainSizer->Add(detailBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    // Summary & Bottom Actions
    auto* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    m_summaryLabel = new wxStaticText(this, wxID_ANY, wxT("Ready."));
    m_summaryLabel->SetForegroundColour(wxColour(60, 60, 60));
    bottomSizer->Add(m_summaryLabel, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    m_batchBtn = new wxButton(this, ID_BATCH_REMAP_BTN, wxT("Batch Folder Remap..."));
    m_applyBtn = new wxButton(this, ID_APPLY_REMAP_BTN, wxT("Apply Remap to Sequence"));
    m_applyBtn->SetDefault();
    auto* closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));

    bottomSizer->Add(m_batchBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    bottomSizer->Add(m_applyBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    bottomSizer->Add(closeBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    mainSizer->Add(bottomSizer, 0, wxEXPAND | wxALL, 5);
    SetSizer(mainSizer);
}

void AISequenceRemapDialog::GenerateMappingPlan() {
    // Generate layout models
    ModelSpatialDescriptor srcTree;
    srcTree.name = "MegaTree 16x50";
    srcTree.category = ModelCategory::TREE_360;
    srcTree.width = 3.0f; srcTree.height = 6.0f; srcTree.depth = 3.0f;
    srcTree.totalNodes = 800; srcTree.strandCount = 16; srcTree.is3D = true;
    srcTree.submodelNames = {"Star", "Tree Top", "Tree Bottom"};

    ModelSpatialDescriptor srcArch;
    srcArch.name = "Arch Left";
    srcArch.category = ModelCategory::ARCH;
    srcArch.width = 2.5f; srcArch.height = 1.2f; srcArch.depth = 0.1f;
    srcArch.totalNodes = 50; srcArch.strandCount = 1;

    ModelSpatialDescriptor srcFace;
    srcFace.name = "Singing Tree Face";
    srcFace.category = ModelCategory::SINGING_FACE;
    srcFace.width = 1.5f; srcFace.height = 1.5f; srcFace.depth = 0.1f;
    srcFace.totalNodes = 120; srcFace.strandCount = 1;
    srcFace.submodelNames = {"Eyes Open", "Mouth AI", "Mouth O", "Outline"};

    m_sourceModels = {srcTree, srcArch, srcFace};

    ModelSpatialDescriptor tgtTree;
    tgtTree.name = "MegaTree 32x100";
    tgtTree.category = ModelCategory::TREE_360;
    tgtTree.width = 4.0f; tgtTree.height = 8.0f; tgtTree.depth = 4.0f;
    tgtTree.totalNodes = 3200; tgtTree.strandCount = 32; tgtTree.is3D = true;
    tgtTree.submodelNames = {"Star", "Tree Top"};

    ModelSpatialDescriptor tgtArch;
    tgtArch.name = "Arch 1";
    tgtArch.category = ModelCategory::ARCH;
    tgtArch.width = 2.4f; tgtArch.height = 1.2f; tgtArch.depth = 0.1f;
    tgtArch.totalNodes = 50; tgtArch.strandCount = 1;

    ModelSpatialDescriptor tgtFace;
    tgtFace.name = "Singing Monster Face";
    tgtFace.category = ModelCategory::SINGING_FACE;
    tgtFace.width = 1.8f; tgtFace.height = 1.8f; tgtFace.depth = 0.1f;
    tgtFace.totalNodes = 150; tgtFace.strandCount = 1;
    tgtFace.submodelNames = {"Eyes Open", "Mouth AI", "Mouth O", "Outline"};

    m_targetModels = {tgtTree, tgtArch, tgtFace};

    float thresh = static_cast<float>(m_thresholdSlider->GetValue()) / 100.0f;
    m_currentPlan = SequenceRemappingAgent::GenerateRemapPlan(m_sourceModels, m_targetModels, thresh);
    PopulateMappingsList();
}

void AISequenceRemapDialog::PopulateMappingsList() {
    m_mappingsListCtrl->DeleteAllItems();

    for (size_t i = 0; i < m_currentPlan.mappings.size(); ++i) {
        const auto& m = m_currentPlan.mappings[i];
        long row = m_mappingsListCtrl->InsertItem(static_cast<long>(i), wxString(m.sourceModel));
        m_mappingsListCtrl->SetItem(row, 1, wxString(m.targetModel));
        m_mappingsListCtrl->SetItem(row, 2, wxString::Format(wxT("%.1f%%"), m.similarityScore * 100.0f));
        m_mappingsListCtrl->SetItem(row, 3, m.confidence >= 0.85f ? wxT("High") : m.confidence >= 0.65f ? wxT("Medium") : wxT("Low"));
        m_mappingsListCtrl->SetItem(row, 4, wxString(m.rationale));
        m_mappingsListCtrl->SetItemData(row, static_cast<long>(i));

        if (m.confidence >= 0.85f) {
            m_mappingsListCtrl->SetItemBackgroundColour(row, wxColour(235, 255, 235));
        } else if (m.confidence < 0.65f) {
            m_mappingsListCtrl->SetItemBackgroundColour(row, wxColour(255, 250, 230));
        }
    }

    m_summaryLabel->SetLabel(wxString(m_currentPlan.summaryMessage));
}

void AISequenceRemapDialog::ApplyRemappingToSequence() {
    if (!m_currentPlan.success || m_currentPlan.mappings.empty()) {
        wxMessageBox(wxT("No valid model remappings to apply."), wxT("Remap"), wxOK | wxICON_WARNING, this);
        return;
    }

    wxString sourcePath = m_sourceSeqPicker->GetPath();
    if (sourcePath.empty()) {
        wxMessageBox(wxT("Please choose a source sequence (.xsq) file to remap."), wxT("No Sequence Selected"), wxOK | wxICON_INFORMATION, this);
        return;
    }

    wxFile inFile(sourcePath);
    if (!inFile.IsOpened()) {
        wxMessageBox(wxT("Could not read sequence file."), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    wxString xmlContent;
    inFile.ReadAll(&xmlContent);
    inFile.Close();

    std::string remapped = SequenceRemappingAgent::ApplyRemappingToSequenceXML(std::string(xmlContent.mb_str()), m_currentPlan);
    
    wxString outPath = sourcePath.BeforeLast('.') + wxT("_remapped.xsq");
    wxFile outFile(outPath, wxFile::write);
    if (outFile.IsOpened()) {
        outFile.Write(remapped.c_str(), remapped.length());
        outFile.Close();
        wxMessageBox(wxT("Remapped sequence successfully saved to:\n") + outPath, wxT("Remap Complete"), wxOK | wxICON_INFORMATION, this);
    }
}

void AISequenceRemapDialog::BatchProcessFolder(const wxString& folderPath) {
    wxDir dir(folderPath);
    if (!dir.IsOpened()) return;

    wxString filename;
    int count = 0;
    bool cont = dir.GetFirst(&filename, wxT("*.xsq"), wxDIR_FILES);
    while (cont) {
        wxString fullPath = folderPath + wxFileName::GetPathSeparator() + filename;
        wxFile file(fullPath);
        if (file.IsOpened()) {
            wxString content;
            file.ReadAll(&content);
            file.Close();

            std::string remapped = SequenceRemappingAgent::ApplyRemappingToSequenceXML(std::string(content.mb_str()), m_currentPlan);
            wxString outPath = folderPath + wxFileName::GetPathSeparator() + wxT("remapped_") + filename;
            wxFile outFile(outPath, wxFile::write);
            if (outFile.IsOpened()) {
                outFile.Write(remapped.c_str(), remapped.length());
                outFile.Close();
                count++;
            }
        }
        cont = dir.GetNext(&filename);
    }
    wxMessageBox(wxString::Format(wxT("Batch remapped %d sequence files successfully."), count), wxT("Batch Complete"), wxOK | wxICON_INFORMATION, this);
}

void AISequenceRemapDialog::OnSourceFileChanged(wxFileDirPickerEvent& WXUNUSED(event)) {
    GenerateMappingPlan();
}

void AISequenceRemapDialog::OnGeneratePlanClicked(wxCommandEvent& WXUNUSED(event)) {
    GenerateMappingPlan();
}

void AISequenceRemapDialog::OnApplyClicked(wxCommandEvent& WXUNUSED(event)) {
    ApplyRemappingToSequence();
}

void AISequenceRemapDialog::OnBatchClicked(wxCommandEvent& WXUNUSED(event)) {
    wxDirDialog dirDlg(this, wxT("Select Folder Containing Sequence (.xsq) Files"), wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dirDlg.ShowModal() == wxID_CANCEL) return;
    BatchProcessFolder(dirDlg.GetPath());
}

void AISequenceRemapDialog::OnThresholdChanged(wxScrollEvent& WXUNUSED(event)) {
    int val = m_thresholdSlider->GetValue();
    m_thresholdValueLabel->SetLabel(wxString::Format(wxT("%d%%"), val));
    GenerateMappingPlan();
}

void AISequenceRemapDialog::OnMappingSelected(wxListEvent& event) {
    long data = event.GetData();
    if (data >= 0 && data < static_cast<long>(m_currentPlan.mappings.size())) {
        m_selectedMappingIdx = static_cast<int>(data);
        const auto& match = m_currentPlan.mappings[m_selectedMappingIdx];

        m_rationaleTextCtrl->SetValue(wxString::Format(
            wxT("Mapping: %s -> %s (Similarity: %.2f%%, Confidence: %.2f)\n%s"),
            match.sourceModel.c_str(), match.targetModel.c_str(),
            match.similarityScore * 100.0f, match.confidence,
            match.rationale.c_str()
        ));

        wxString submodelsStr = wxT("Submodel Alignments:\n");
        if (match.submodelMap.empty()) {
            submodelsStr += wxT("  No direct inner submodel name matches.");
        } else {
            for (const auto& [srcSub, tgtSub] : match.submodelMap) {
                submodelsStr += wxString::Format(wxT("  • %s  ==>  %s\n"), srcSub.c_str(), tgtSub.c_str());
            }
        }
        m_submodelsTextCtrl->SetValue(submodelsStr);
    }
}

void AISequenceRemapDialog::OnCloseClicked(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
