/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/filepicker.h>
#include <string>
#include <vector>
#include "AI/SequenceRemappingAgent.h"

namespace xLights::AI {

class AISequenceRemapDialog : public wxDialog {
public:
    AISequenceRemapDialog(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& title = wxT("AI Cross-Display Sequence Remapper"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(920, 680),
        long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    );

    virtual ~AISequenceRemapDialog() = default;

private:
    void InitUI();
    void GenerateMappingPlan();
    void PopulateMappingsList();
    void ApplyRemappingToSequence();
    void BatchProcessFolder(const wxString& folderPath);

    // Event Handlers
    void OnSourceFileChanged(wxFileDirPickerEvent& event);
    void OnGeneratePlanClicked(wxCommandEvent& event);
    void OnApplyClicked(wxCommandEvent& event);
    void OnBatchClicked(wxCommandEvent& event);
    void OnThresholdChanged(wxScrollEvent& event);
    void OnMappingSelected(wxListEvent& event);
    void OnCloseClicked(wxCommandEvent& event);

    // Controls
    wxFilePickerCtrl* m_sourceSeqPicker{nullptr};
    wxListCtrl* m_mappingsListCtrl{nullptr};
    wxStaticText* m_summaryLabel{nullptr};
    wxSlider* m_thresholdSlider{nullptr};
    wxStaticText* m_thresholdValueLabel{nullptr};
    wxTextCtrl* m_rationaleTextCtrl{nullptr};
    wxTextCtrl* m_submodelsTextCtrl{nullptr};
    wxButton* m_applyBtn{nullptr};
    wxButton* m_batchBtn{nullptr};

    std::vector<ModelSpatialDescriptor> m_sourceModels;
    std::vector<ModelSpatialDescriptor> m_targetModels;
    RemapPlanResult m_currentPlan;
    int m_selectedMappingIdx{-1};

    enum {
        ID_SOURCE_SEQ_PICKER = 11001,
        ID_GENERATE_PLAN_BTN,
        ID_APPLY_REMAP_BTN,
        ID_BATCH_REMAP_BTN,
        ID_THRESHOLD_SLIDER,
        ID_MAPPINGS_LIST
    };

    wxDECLARE_EVENT_TABLE();
};

} // namespace xLights::AI
