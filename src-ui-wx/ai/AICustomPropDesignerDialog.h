#pragma once

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/sizer.h>

#include "AI/CustomPropDesignerAI.h"
#include "AI/AIConfigurationManager.h"

namespace xLights::AI {

class AICustomPropDesignerDialog : public wxDialog {
public:
    AICustomPropDesignerDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("AI Custom Prop Designer"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(820, 600), long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    virtual ~AICustomPropDesignerDialog() = default;

private:
    void InitUI();

    void OnGenerateClick(wxCommandEvent& event);
    void OnExportXmlClick(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);
    void OnPaint(wxPaintEvent& event);

    wxTextCtrl* m_descriptionCtrl = nullptr;
    wxSpinCtrl* m_nodeCountSpin = nullptr;
    wxChoice* m_shapePresetChoice = nullptr;
    wxButton* m_generateBtn = nullptr;
    wxButton* m_exportXmlBtn = nullptr;
    wxButton* m_closeBtn = nullptr;
    wxPanel* m_canvasPanel = nullptr;
    wxStaticText* m_statusLabel = nullptr;
    
    std::vector<PropNodeSuggestion> m_lastNodes;

    DECLARE_EVENT_TABLE()
};

} // namespace xLights::AI
