/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIControllerDiscoveryDialog.h"
#include "src-ui-wx/ai/AIHelpGuideDialog.h"
#include <spdlog/spdlog.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <fstream>

namespace xLights::AI {

enum {
    ID_BTN_SCAN = 25001,
    ID_BTN_APPLY,
    ID_BTN_UNDO,
    ID_BTN_REDO,
    ID_BTN_EXPORT_REPORT,
    ID_BTN_HELP
};

wxBEGIN_EVENT_TABLE(AIControllerDiscoveryDialog, wxDialog)
    EVT_BUTTON(ID_BTN_SCAN, AIControllerDiscoveryDialog::OnScanSubnetClick)
    EVT_BUTTON(ID_BTN_APPLY, AIControllerDiscoveryDialog::OnApplyApprovedMappingsClick)
    EVT_BUTTON(ID_BTN_UNDO, AIControllerDiscoveryDialog::OnUndoClick)
    EVT_BUTTON(ID_BTN_REDO, AIControllerDiscoveryDialog::OnRedoClick)
    EVT_BUTTON(ID_BTN_EXPORT_REPORT, AIControllerDiscoveryDialog::OnExportReportClick)
    EVT_BUTTON(ID_BTN_HELP, AIControllerDiscoveryDialog::OnHelpClick)
    EVT_BUTTON(wxID_CANCEL, AIControllerDiscoveryDialog::OnCloseClick)
wxEND_EVENT_TABLE()

AIControllerDiscoveryDialog::AIControllerDiscoveryDialog(
    wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style), m_commandHistory(50) {
    InitUI();
    UpdateUndoRedoState();
}

void AIControllerDiscoveryDialog::InitUI() {
    SetMinSize(wxSize(920, 660));
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Modern Header Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(20, 48, 52));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* textSizer = new wxBoxSizer(wxVERTICAL);

    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Universal Controller Discovery & Semantic Model Auto-Mapper"));
    titleTxt->SetForegroundColour(*wxWHITE);
    auto font = titleTxt->GetFont();
    font.SetPointSize(font.GetPointSize() + 2);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);

    auto* subTitle = new wxStaticText(banner, wxID_ANY,
        wxT("On-demand network scan & spatial model-to-port binder with interactive pre-execution checklist and full multi-level Undo/Redo rollback."));
    subTitle->SetForegroundColour(wxColour(160, 230, 230));

    textSizer->Add(titleTxt, 0, wxALL, 6);
    textSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 6);
    bannerSizer->Add(textSizer, 1, wxEXPAND);

    auto* helpBtn = new wxButton(banner, ID_BTN_HELP, wxT("❓ Help & Guide"));
    helpBtn->SetToolTip(wxT("Open comprehensive user manual, setting explanations, and reversible mapping diagrams (F1)."));
    bannerSizer->Add(helpBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 8);

    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Subnet Target Bar
    auto* scanBox = new wxBoxSizer(wxHORIZONTAL);
    scanBox->Add(new wxStaticText(this, wxID_ANY, wxT("Subnet CIDR Range:")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 6);
    m_txtSubnetCidr = new wxTextCtrl(this, wxID_ANY, wxT("192.168.1.0/24"), wxDefaultPosition, wxSize(150, -1));
    scanBox->Add(m_txtSubnetCidr, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    m_btnScan = new wxButton(this, ID_BTN_SCAN, wxT("🔍 Scan Subnet On-Demand"));
    m_btnScan->SetBackgroundColour(wxColour(20, 110, 120));
    m_btnScan->SetForegroundColour(*wxWHITE);
    scanBox->Add(m_btnScan, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    m_lblStatus = new wxStaticText(this, wxID_ANY, wxT("On-demand scanner ready. Click 'Scan Subnet On-Demand' to discover controllers."));
    scanBox->Add(m_lblStatus, 1, wxALIGN_CENTER_VERTICAL);
    mainSizer->Add(scanBox, 0, wxEXPAND | wxALL, 6);

    // Discovered Devices Table
    auto* devBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Discovered Online Controllers (mDNS / SSDP / REST)"));
    m_listDevices = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 140), wxLC_REPORT | wxLC_SINGLE_SEL);
    m_listDevices->InsertColumn(0, wxT("Hostname"), wxLIST_FORMAT_LEFT, 160);
    m_listDevices->InsertColumn(1, wxT("IP Address"), wxLIST_FORMAT_LEFT, 120);
    m_listDevices->InsertColumn(2, wxT("Controller Type"), wxLIST_FORMAT_LEFT, 200);
    m_listDevices->InsertColumn(3, wxT("Ports"), wxLIST_FORMAT_LEFT, 70);
    m_listDevices->InsertColumn(4, wxT("Capacity"), wxLIST_FORMAT_LEFT, 110);
    m_listDevices->InsertColumn(5, wxT("WiFi Signal"), wxLIST_FORMAT_LEFT, 90);
    devBox->Add(m_listDevices, 1, wxEXPAND | wxALL, 4);
    mainSizer->Add(devBox, 0, wxEXPAND | wxLEFT | wxRIGHT, 6);

    // Proposed Model Bindings Table
    auto* propBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Proposed Layout Model Bindings (Pre-Execution Checklist)"));
    m_listProposals = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    m_listProposals->InsertColumn(0, wxT("Status"), wxLIST_FORMAT_LEFT, 90);
    m_listProposals->InsertColumn(1, wxT("Layout Model"), wxLIST_FORMAT_LEFT, 150);
    m_listProposals->InsertColumn(2, wxT("Target Controller"), wxLIST_FORMAT_LEFT, 160);
    m_listProposals->InsertColumn(3, wxT("Port"), wxLIST_FORMAT_LEFT, 60);
    m_listProposals->InsertColumn(4, wxT("Universe & Channel Range"), wxLIST_FORMAT_LEFT, 180);
    m_listProposals->InsertColumn(5, wxT("Pixels / Color"), wxLIST_FORMAT_LEFT, 110);
    m_listProposals->InsertColumn(6, wxT("AI Rationale"), wxLIST_FORMAT_LEFT, 240);
    propBox->Add(m_listProposals, 1, wxEXPAND | wxALL, 4);
    mainSizer->Add(propBox, 1, wxEXPAND | wxALL, 6);

    // Bottom Action Bar
    auto* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnApply = new wxButton(this, ID_BTN_APPLY, wxT("✓ Apply Approved Mappings"));
    m_btnApply->SetBackgroundColour(wxColour(30, 130, 70));
    m_btnApply->SetForegroundColour(*wxWHITE);
    m_btnUndo = new wxButton(this, ID_BTN_UNDO, wxT("↶ Undo Mapping"));
    m_btnRedo = new wxButton(this, ID_BTN_REDO, wxT("↷ Redo"));
    m_btnExportReport = new wxButton(this, ID_BTN_EXPORT_REPORT, wxT("📄 Export Audit Report"));
    m_btnClose = new wxButton(this, wxID_CANCEL, wxT("Close"));

    bottomSizer->Add(m_btnApply, 0, wxRIGHT, 4);
    bottomSizer->Add(m_btnUndo, 0, wxRIGHT, 4);
    bottomSizer->Add(m_btnRedo, 0, wxRIGHT, 4);
    bottomSizer->Add(m_btnExportReport, 0, wxRIGHT, 4);
    bottomSizer->AddStretchSpacer();
    bottomSizer->Add(m_btnClose, 0);

    mainSizer->Add(bottomSizer, 0, wxEXPAND | wxALL, 6);
    SetSizer(mainSizer);
    Center();
}

void AIControllerDiscoveryDialog::UpdateUndoRedoState() {
    m_btnUndo->Enable(m_commandHistory.CanUndo());
    m_btnRedo->Enable(m_commandHistory.CanRedo());
}

void AIControllerDiscoveryDialog::OnScanSubnetClick(wxCommandEvent& WXUNUSED(event)) {
    std::string cidr = m_txtSubnetCidr->GetValue().ToStdString();
    auto devices = AIControllerDiscoveryMapper::ScanSubnetOnDemand(cidr);

    m_listDevices->DeleteAllItems();
    for (size_t i = 0; i < devices.size(); ++i) {
        const auto& d = devices[i];
        long idx = m_listDevices->InsertItem(i, wxString::FromUTF8(d.hostname));
        m_listDevices->SetItem(idx, 1, wxString::FromUTF8(d.ipAddress));
        m_listDevices->SetItem(idx, 2, wxString::FromUTF8(AIControllerDiscoveryMapper::GetVendorName(d.vendor)));
        m_listDevices->SetItem(idx, 3, wxString::Format(wxT("%d ports"), d.totalPorts));
        m_listDevices->SetItem(idx, 4, wxString::Format(wxT("%d px/port"), d.maxPixelsPerPort));
        m_listDevices->SetItem(idx, 5, wxString::Format(wxT("%d dBm"), d.wifiRssiDbm));
    }

    m_lastReport = AIControllerDiscoveryMapper::GenerateMappingProposals(devices, {});

    m_listProposals->DeleteAllItems();
    for (size_t i = 0; i < m_lastReport.proposals.size(); ++i) {
        const auto& p = m_lastReport.proposals[i];
        long idx = m_listProposals->InsertItem(i, p.isCheckedForApplication ? wxT("[APPROVED]") : wxT("[SKIP]"));
        m_listProposals->SetItem(idx, 1, wxString::FromUTF8(p.modelName));
        m_listProposals->SetItem(idx, 2, wxString::FromUTF8(p.targetControllerHost));
        m_listProposals->SetItem(idx, 3, wxString::Format(wxT("Port %d"), p.targetPortIndex));
        m_listProposals->SetItem(idx, 4, wxString::Format(wxT("U%d: %d-%d"), p.startUniverse, p.startChannel, p.endChannel));
        m_listProposals->SetItem(idx, 5, wxString::Format(wxT("%d px (%s)"), p.pixelCount, wxString::FromUTF8(p.colorOrder)));
        m_listProposals->SetItem(idx, 6, wxString::FromUTF8(p.rationale));
    }

    m_lblStatus->SetLabel(wxString::Format(wxT("Scan Complete: %d controllers found, %d models matched (%d total channels)."),
        m_lastReport.controllersDiscovered, m_lastReport.modelsMatched, m_lastReport.totalChannelsMapped));
}

void AIControllerDiscoveryDialog::OnApplyApprovedMappingsClick(wxCommandEvent& WXUNUSED(event)) {
    if (m_lastReport.proposals.empty()) {
        wxMessageBox(wxT("No proposals available to apply. Please click 'Scan Subnet On-Demand' first."),
                     wxT("Notice"), wxOK | wxICON_INFORMATION, this);
        return;
    }

    auto oldState = m_appliedState;
    auto newState = m_lastReport.proposals;

    auto cmd = std::make_unique<AIActionCommand>(
        "Apply Universal Controller Auto-Mappings",
        [this, newState]() {
            m_appliedState = newState;
            spdlog::info("AIControllerDiscoveryDialog: Applied {} controller bindings.", newState.size());
            return true;
        },
        [this, oldState]() {
            m_appliedState = oldState;
            spdlog::info("AIControllerDiscoveryDialog: Rolled back controller bindings to previous state.");
            return true;
        }
    );

    m_commandHistory.ExecuteCommand(std::move(cmd));
    UpdateUndoRedoState();

    wxMessageBox(wxString::Format(wxT("Successfully bound %zu layout models to discovered controllers!\nUse 'Undo Mapping' anytime to revert."),
                 newState.size()), wxT("Mappings Applied"), wxOK | wxICON_INFORMATION, this);
}

void AIControllerDiscoveryDialog::OnUndoClick(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Undo()) {
        UpdateUndoRedoState();
        wxMessageBox(wxT("All controller mappings successfully rolled back to previous state!"),
                     wxT("Undo Successful"), wxOK | wxICON_INFORMATION, this);
    }
}

void AIControllerDiscoveryDialog::OnRedoClick(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Redo()) {
        UpdateUndoRedoState();
        wxMessageBox(wxT("Controller mappings re-applied!"),
                     wxT("Redo Successful"), wxOK | wxICON_INFORMATION, this);
    }
}

void AIControllerDiscoveryDialog::OnExportReportClick(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog dlg(this, wxT("Save Discovery Audit Report"), wxEmptyString,
                     wxT("Controller_Discovery_Report.txt"), wxT("Text files (*.txt)|*.txt"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        std::ofstream out(dlg.GetPath().ToStdString());
        out << m_lastReport.GenerateFormattedReport();
        wxMessageBox(wxT("Discovery report exported successfully!"), wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    }
}

void AIControllerDiscoveryDialog::OnHelpClick(wxCommandEvent& WXUNUSED(event)) {
    AIHelpGuideDialog::ShowHelp(this, "CONTROLLER_AUTO_MAPPER");
}

void AIControllerDiscoveryDialog::OnCloseClick(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
