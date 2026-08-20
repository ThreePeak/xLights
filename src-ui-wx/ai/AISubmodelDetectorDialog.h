/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>

#include "AI/SubmodelDetector.h"

namespace xLights::AI {

class AISubmodelDetectorDialog : public wxDialog {
public:
    AISubmodelDetectorDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("AI Submodel SAM Vision & Spatial Auto-Detector"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(750, 560), long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AISubmodelDetectorDialog() = default;

private:
    void InitUI();

    // Event Handlers
    void OnDetectButtonClick(wxCommandEvent& event);
    void OnExportXmlButtonClick(wxCommandEvent& event);
    void OnHelpButtonClick(wxCommandEvent& event);
    void OnCloseButtonClick(wxCommandEvent& event);

    class wxComboBox;
    wxComboBox* m_parentModelCombo = nullptr;
    wxSpinCtrl* m_totalNodesSpin = nullptr;

    wxSlider* m_clusterRadiusSlider = nullptr;
    wxSpinCtrl* m_minPtsSpin = nullptr;
    wxSlider* m_confidenceSlider = nullptr;
    wxChoice* m_exportModeChoice = nullptr;

    wxListCtrl* m_detectedSubmodelsList = nullptr;

    wxButton* m_detectBtn = nullptr;
    wxButton* m_exportXmlBtn = nullptr;
    wxButton* m_closeBtn = nullptr;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
