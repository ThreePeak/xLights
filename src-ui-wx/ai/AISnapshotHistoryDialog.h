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
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include "src-core/ai/AISnapshotHistoryManager.h"

namespace xLights {

class AISnapshotHistoryDialog : public wxDialog {
public:
    AISnapshotHistoryDialog(wxWindow* parent,
                            wxWindowID id = wxID_ANY,
                            const wxString& title = wxT("AI Visual History Timeline & Snapshot State Manager"),
                            const wxPoint& pos = wxDefaultPosition,
                            const wxSize& size = wxSize(1080, 780),
                            long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);

    virtual ~AISnapshotHistoryDialog() = default;

    void OnJumpToSelectedSnapshot(wxCommandEvent& event);
    void OnToggleEnabled(wxCommandEvent& event);
    void OnCapacityChanged(wxSpinEvent& event);
    void OnExportFullSnapshot(wxCommandEvent& event);
    void OnExportGranularChecklist(wxCommandEvent& event);
    void OnImportSnapshotPackage(wxCommandEvent& event);
    void OnClearHistory(wxCommandEvent& event);

private:
    void CreateControls();
    void RefreshHistoryList();

    wxCheckBox* m_chkEnableHistory{nullptr};
    wxSpinCtrl* m_spinCapacity{nullptr};
    wxListCtrl* m_historyListCtrl{nullptr};
    wxListCtrl* m_granularChecklistCtrl{nullptr};
    wxTextCtrl* m_txtPayloadPreview{nullptr};
    wxStaticText* m_lblStatus{nullptr};
};

} // namespace xLights
