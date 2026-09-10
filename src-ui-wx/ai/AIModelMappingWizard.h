/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/wizard.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/checkbox.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/filepicker.h>

#include "AI/ModelMappingAIGenerator.h"

namespace xLights::AI {

class AIModelMappingWizard : public wxWizard {
public:
    AIModelMappingWizard(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("AI Vendor Model Mapping Wizard"));
    virtual ~AIModelMappingWizard() = default;

    bool RunWizardUI();

private:
    void OnPageChanged(wxWizardEvent& event);
    void OnWizardFinished(wxWizardEvent& event);
    void RunMappingAnalysis();

    wxWizardPageSimple* m_page1 = nullptr;
    wxWizardPageSimple* m_page2 = nullptr;

    wxFilePickerCtrl* m_pickerVendorSeq = nullptr;
    wxSlider* m_confidenceThresholdSlider = nullptr;
    wxCheckBox* m_pass1ExactChk = nullptr;
    wxCheckBox* m_pass2FuzzyChk = nullptr;
    wxCheckBox* m_pass3SemanticChk = nullptr;
    wxCheckBox* m_pass4TaxonomyChk = nullptr;
    wxButton* m_taxonomyEditorBtn = nullptr;

    wxListCtrl* m_mappingsList = nullptr;

    ModelMappingResult m_mappingResult;
};

} // namespace xLights::AI
