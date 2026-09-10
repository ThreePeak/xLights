// Copyright (c) xLights Project
#pragma once
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>

class AIStatusBar : public wxPanel {
public:
    AIStatusBar(wxWindow* parent, wxWindowID id = wxID_ANY);
    void SetActiveModel(const wxString& modelName);
    void SetScanState(bool scanning);
    void SetDiagnosticWarningCount(int count, const wxString& summary = wxEmptyString);

private:
    void InitUI();
    void OnToggleExpand(wxCommandEvent& evt);
    void OnPinAlwaysVisible(wxCommandEvent& evt);
    void OnOpenPropDesigner(wxCommandEvent& evt);
    void OnOpenValidator(wxCommandEvent& evt);
    void OnOpenPowerInspector(wxCommandEvent& evt);
    void OnOpenFPPSync(wxCommandEvent& evt);
    void OnOpenDMXAdvisor(wxCommandEvent& evt);
    void OnOpenSpotlight(wxCommandEvent& evt);
    void OnOpenVideoEmulator(wxCommandEvent& evt);
    void OnClickWarningPill(wxCommandEvent& evt);

    wxButton* m_toggleBtn = nullptr;
    wxButton* m_warningPillBtn = nullptr;
    wxCheckBox* m_alwaysVisibleCheck = nullptr;
    wxStaticText* m_modelLabel = nullptr;
    wxStaticText* m_scanLabel = nullptr;
    wxPanel* m_expandPanel = nullptr;
    bool m_expanded = false;

    DECLARE_EVENT_TABLE()
};
