/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AISnapshotHistoryDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace xLights {

enum {
    ID_BTN_JUMP_SNAPSHOT = wxID_HIGHEST + 1301,
    ID_CHK_ENABLE_HISTORY,
    ID_SPIN_CAPACITY,
    ID_BTN_EXPORT_FULL_SNAP,
    ID_BTN_EXPORT_CHECKLIST,
    ID_BTN_IMPORT_PACKAGE,
    ID_BTN_CLEAR_HISTORY
};

AISnapshotHistoryDialog::AISnapshotHistoryDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style
) : wxDialog(parent, id, title, pos, size, style) {
    CreateControls();
    RefreshHistoryList();
}

void AISnapshotHistoryDialog::CreateControls() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(32, 40, 52));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("Photoshop-Style AI Visual History Timeline & State Manager"));
    titleTxt->SetForegroundColour(wxColour(255, 255, 255));
    auto font = titleTxt->GetFont();
    font.SetPointSize(12);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);
    bannerSizer->Add(titleTxt, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Top Controls Bar (Enable switch + Capacity limiter)
    auto* topPanel = new wxPanel(this, wxID_ANY);
    auto* topSizer = new wxBoxSizer(wxHORIZONTAL);

    m_chkEnableHistory = new wxCheckBox(topPanel, ID_CHK_ENABLE_HISTORY, wxT("Enable Background Snapshot Tracking"));
    m_chkEnableHistory->SetValue(AI::AISnapshotHistoryManager::Instance().IsEnabled());
    m_chkEnableHistory->SetForegroundColour(wxColour(0, 220, 255));
    topSizer->Add(m_chkEnableHistory, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);

    topSizer->Add(new wxStaticText(topPanel, wxID_ANY, wxT("Max History Memory Steps:")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 16);
    m_spinCapacity = new wxSpinCtrl(topPanel, ID_SPIN_CAPACITY, wxEmptyString, wxDefaultPosition, wxSize(80, -1),
                                    wxSP_ARROW_KEYS, 5, 250, static_cast<int>(AI::AISnapshotHistoryManager::Instance().GetMaxHistoryCapacity()));
    topSizer->Add(m_spinCapacity, 0, wxALIGN_CENTER_VERTICAL | wxALL, 4);

    topSizer->AddStretchSpacer();
    auto* btnClear = new wxButton(topPanel, ID_BTN_CLEAR_HISTORY, wxT("🗑️ Clear Timeline"));
    topSizer->Add(btnClear, 0, wxALIGN_CENTER_VERTICAL | wxALL, 4);

    topPanel->SetSizer(topSizer);
    mainSizer->Add(topPanel, 0, wxEXPAND | wxALL, 6);

    // Center Workspace (Left: History Timeline list | Right: Granular Item Checklist & State Payload Preview)
    auto* workSizer = new wxBoxSizer(wxHORIZONTAL);

    // Left History List
    auto* leftBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Timeline States (Click to Jump)"));
    m_historyListCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(360, 480), wxLC_REPORT | wxLC_SINGLE_SEL);
    m_historyListCtrl->InsertColumn(0, wxT("ID"), wxLIST_FORMAT_LEFT, 50);
    m_historyListCtrl->InsertColumn(1, wxT("Action Description"), wxLIST_FORMAT_LEFT, 180);
    m_historyListCtrl->InsertColumn(2, wxT("Time"), wxLIST_FORMAT_LEFT, 70);
    m_historyListCtrl->InsertColumn(3, wxT("Module"), wxLIST_FORMAT_LEFT, 90);
    leftBox->Add(m_historyListCtrl, 1, wxEXPAND | wxALL, 4);
    workSizer->Add(leftBox, 0, wxEXPAND | wxALL, 6);

    // Right Granular Export & Payload
    auto* rightBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Selected Snapshot Details & Granular Component Checklist"));
    m_granularChecklistCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(400, 180), wxLC_REPORT);
    m_granularChecklistCtrl->InsertColumn(0, wxT("Export [x]"), wxLIST_FORMAT_LEFT, 80);
    m_granularChecklistCtrl->InsertColumn(1, wxT("Category"), wxLIST_FORMAT_LEFT, 100);
    m_granularChecklistCtrl->InsertColumn(2, wxT("Component Name"), wxLIST_FORMAT_LEFT, 200);
    rightBox->Add(m_granularChecklistCtrl, 0, wxEXPAND | wxALL, 4);

    rightBox->Add(new wxStaticText(this, wxID_ANY, wxT("State Payload XML Preview:")), 0, wxTOP | wxLEFT, 4);
    m_txtPayloadPreview = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    m_txtPayloadPreview->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    rightBox->Add(m_txtPayloadPreview, 1, wxEXPAND | wxALL, 4);

    workSizer->Add(rightBox, 1, wxEXPAND | wxALL, 6);
    mainSizer->Add(workSizer, 1, wxEXPAND);

    // Bottom Action Bar
    auto* botBar = new wxPanel(this, wxID_ANY);
    auto* botSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* btnJump = new wxButton(botBar, ID_BTN_JUMP_SNAPSHOT, wxT("⏱️ Jump to Selected State"));
    btnJump->SetBackgroundColour(wxColour(30, 140, 240));
    btnJump->SetForegroundColour(wxColour(255, 255, 255));

    auto* btnExportFull = new wxButton(botBar, ID_BTN_EXPORT_FULL_SNAP, wxT("💾 Export Full Snapshot (.xsnap)..."));
    auto* btnExportCheck = new wxButton(botBar, ID_BTN_EXPORT_CHECKLIST, wxT("📦 Export Granular Components..."));
    auto* btnImport = new wxButton(botBar, ID_BTN_IMPORT_PACKAGE, wxT("📂 Import Snapshot Package..."));
    auto* btnClose = new wxButton(botBar, wxID_CANCEL, wxT("Close"));

    botSizer->Add(btnJump, 0, wxALL, 4);
    botSizer->Add(btnExportFull, 0, wxALL, 4);
    botSizer->Add(btnExportCheck, 0, wxALL, 4);
    botSizer->Add(btnImport, 0, wxALL, 4);
    botSizer->AddStretchSpacer();
    botSizer->Add(btnClose, 0, wxALL, 4);

    botBar->SetSizer(botSizer);
    mainSizer->Add(botBar, 0, wxEXPAND | wxALL, 6);

    SetSizer(mainSizer);

    // Bindings
    Bind(wxEVT_BUTTON, &AISnapshotHistoryDialog::OnJumpToSelectedSnapshot, this, ID_BTN_JUMP_SNAPSHOT);
    Bind(wxEVT_CHECKBOX, &AISnapshotHistoryDialog::OnToggleEnabled, this, ID_CHK_ENABLE_HISTORY);
    Bind(wxEVT_SPINCTRL, &AISnapshotHistoryDialog::OnCapacityChanged, this, ID_SPIN_CAPACITY);
    Bind(wxEVT_BUTTON, &AISnapshotHistoryDialog::OnExportFullSnapshot, this, ID_BTN_EXPORT_FULL_SNAP);
    Bind(wxEVT_BUTTON, &AISnapshotHistoryDialog::OnExportGranularChecklist, this, ID_BTN_EXPORT_CHECKLIST);
    Bind(wxEVT_BUTTON, &AISnapshotHistoryDialog::OnImportSnapshotPackage, this, ID_BTN_IMPORT_PACKAGE);
    Bind(wxEVT_BUTTON, &AISnapshotHistoryDialog::OnClearHistory, this, ID_BTN_CLEAR_HISTORY);
}

void AISnapshotHistoryDialog::RefreshHistoryList() {
    if (!m_historyListCtrl) return;
    m_historyListCtrl->DeleteAllItems();

    const auto& snapshots = AI::AISnapshotHistoryManager::Instance().GetAllSnapshots();
    uint32_t activeId = AI::AISnapshotHistoryManager::Instance().GetCurrentActiveSnapshotId();

    for (size_t i = 0; i < snapshots.size(); ++i) {
        const auto& s = snapshots[i];
        wxString idStr = wxString::Format(wxT("#%u%s"), s.snapshotId, s.snapshotId == activeId ? wxT(" (*)") : wxT(""));
        long idx = m_historyListCtrl->InsertItem(static_cast<long>(i), idStr);
        m_historyListCtrl->SetItem(idx, 1, wxString::FromUTF8(s.label));
        m_historyListCtrl->SetItem(idx, 2, wxString::FromUTF8(s.timestampStr));
        m_historyListCtrl->SetItem(idx, 3, wxString::FromUTF8(s.sourceSubsystem));
    }

    if (!snapshots.empty() && m_txtPayloadPreview) {
        m_txtPayloadPreview->SetValue(wxString::FromUTF8(snapshots.back().fullStateXml));
    }
}

void AISnapshotHistoryDialog::OnToggleEnabled(wxCommandEvent& WXUNUSED(event)) {
    if (m_chkEnableHistory) {
        AI::AISnapshotHistoryManager::Instance().SetEnabled(m_chkEnableHistory->IsChecked());
    }
}

void AISnapshotHistoryDialog::OnCapacityChanged(wxSpinEvent& WXUNUSED(event)) {
    if (m_spinCapacity) {
        AI::AISnapshotHistoryManager::Instance().SetMaxHistoryCapacity(m_spinCapacity->GetValue());
        RefreshHistoryList();
    }
}

void AISnapshotHistoryDialog::OnJumpToSelectedSnapshot(wxCommandEvent& WXUNUSED(event)) {
    long item = m_historyListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (item != -1) {
        const auto& snapshots = AI::AISnapshotHistoryManager::Instance().GetAllSnapshots();
        if (static_cast<size_t>(item) < snapshots.size()) {
            std::string xml;
            AI::AISnapshotHistoryManager::Instance().JumpToSnapshot(snapshots[item].snapshotId, xml);
            if (m_txtPayloadPreview) {
                m_txtPayloadPreview->SetValue(wxString::FromUTF8(xml));
            }
            RefreshHistoryList();
            wxMessageBox(wxString::Format(wxT("Restored sequence and layout state to snapshot #%u: '%s'!"),
                snapshots[item].snapshotId, wxString::FromUTF8(snapshots[item].label)),
                wxT("State Restored"), wxOK | wxICON_INFORMATION, this);
        }
    }
}

void AISnapshotHistoryDialog::OnExportFullSnapshot(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Export Full State Snapshot"), wxEmptyString,
                         wxT("Show_State_Snapshot.xsnap"),
                         wxT("xLights Snapshot (*.xsnap)|*.xsnap|JSON (*.json)|*.json"),
                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        uint32_t activeId = AI::AISnapshotHistoryManager::Instance().GetCurrentActiveSnapshotId();
        std::string pkg = AI::AISnapshotHistoryManager::Instance().ExportPackage(activeId, {});
        std::ofstream out(saveDlg.GetPath().ToStdString());
        out << pkg;
        spdlog::info("AISnapshotHistoryDialog: Exported full snapshot to '{}'", saveDlg.GetPath().ToStdString());
    }
}

void AISnapshotHistoryDialog::OnExportGranularChecklist(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Export Granular Component Package"), wxEmptyString,
                         wxT("Granular_Components.xsnap"),
                         wxT("xLights Snapshot (*.xsnap)|*.xsnap|JSON (*.json)|*.json"),
                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        uint32_t activeId = AI::AISnapshotHistoryManager::Instance().GetCurrentActiveSnapshotId();
        std::string pkg = AI::AISnapshotHistoryManager::Instance().ExportPackage(activeId, {});
        std::ofstream out(saveDlg.GetPath().ToStdString());
        out << pkg;
        spdlog::info("AISnapshotHistoryDialog: Exported granular components to '{}'", saveDlg.GetPath().ToStdString());
    }
}

void AISnapshotHistoryDialog::OnImportSnapshotPackage(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog openDlg(this, wxT("Import Snapshot Package"), wxEmptyString, wxEmptyString,
                         wxT("xLights Snapshot (*.xsnap;*.json)|*.xsnap;*.json"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openDlg.ShowModal() == wxID_OK) {
        std::ifstream in(openDlg.GetPath().ToStdString());
        std::stringstream ss;
        ss << in.rdbuf();
        std::string mergedXml;
        if (AI::AISnapshotHistoryManager::Instance().ImportPackage(ss.str(), mergedXml)) {
            RefreshHistoryList();
            wxMessageBox(wxT("Successfully imported snapshot state into history timeline!"),
                wxT("Import Successful"), wxOK | wxICON_INFORMATION, this);
        }
    }
}

void AISnapshotHistoryDialog::OnClearHistory(wxCommandEvent& WXUNUSED(event)) {
    AI::AISnapshotHistoryManager::Instance().ClearHistory();
    RefreshHistoryList();
}

} // namespace xLights
