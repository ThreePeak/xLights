/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/slider.h>
#include <wx/sizer.h>

#include "AI/LocalInferenceEngine.h"

namespace xLights::AI {

class AIInferenceSettingsPanel : public wxPanel {
public:
    AIInferenceSettingsPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    virtual ~AIInferenceSettingsPanel() = default;

private:
    void InitUI();

    wxChoice* m_backendChoice = nullptr;
    wxSlider* m_memCapSlider = nullptr;
    wxStaticText* m_backendStatusLabel = nullptr;
};

} // namespace xLights::AI
