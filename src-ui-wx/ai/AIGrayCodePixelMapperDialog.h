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
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/sizer.h>

#include "AI/GrayCodePixelMapper.h"

namespace xLights::AI {

class AIGrayCodePixelMapperDialog : public wxDialog {
public:
    AIGrayCodePixelMapperDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("AI 3D Pixel Map Camera Solver"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(700, 500), long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AIGrayCodePixelMapperDialog() = default;

private:
    void InitUI();

    // Event Handlers
    void OnStartCaptureButtonClick(wxCommandEvent& event);
    void OnSolvePointCloudButtonClick(wxCommandEvent& event);
    void OnCloseButtonClick(wxCommandEvent& event);

    wxChoice* m_cameraChoice = nullptr;
    wxGauge* m_captureProgress = nullptr;
    wxStaticText* m_statusText = nullptr;

    wxButton* m_startCaptureBtn = nullptr;
    wxButton* m_solveBtn = nullptr;
    wxButton* m_closeBtn = nullptr;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
