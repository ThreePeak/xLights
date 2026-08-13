#include "src-ui-wx/ai/AIDMXAddressDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <spdlog/spdlog.h>

namespace xLights::AI {

enum {
    ID_DMX_DETECT_BTN = 14001,
    ID_DMX_AUTOREMAP_BTN = 14002,
    ID_DMX_EXPORT_BTN = 14003
};

wxBEGIN_EVENT_TABLE(AIDMXAddressDialog, wxDialog)
    EVT_BUTTON(ID_DMX_DETECT_BTN, AIDMXAddressDialog::OnDetectClick)
    EVT_BUTTON(ID_DMX_AUTOREMAP_BTN, AIDMXAddressDialog::OnAutoRemapClick)
    EVT_BUTTON(ID_DMX_EXPORT_BTN, AIDMXAddressDialog::OnExportClick)
    EVT_BUTTON(wxID_CANCEL, AIDMXAddressDialog::OnCloseClick)
wxEND_EVENT_TABLE()

AIDMXAddressDialog::AIDMXAddressDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
}

void AIDMXAddressDialog::InitUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBoxSizer* inputSizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Universe XML Input"));
    m_universeXmlCtrl = new wxTextCtrl(this, wxID_ANY, wxT("<universe>...</universe>"), wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE);
    inputSizer->GetSizer()->Add(m_universeXmlCtrl, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(inputSizer, 0, wxEXPAND | wxALL, 10);

    wxStaticBoxSizer* resultsSizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Detected Conflicts"));
    m_conflictsListCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxBORDER_SUNKEN);
    m_conflictsListCtrl->InsertColumn(0, wxT("Channel"), wxLIST_FORMAT_LEFT, 80);
    m_conflictsListCtrl->InsertColumn(1, wxT("Fixture A"), wxLIST_FORMAT_LEFT, 180);
    m_conflictsListCtrl->InsertColumn(2, wxT("Fixture B"), wxLIST_FORMAT_LEFT, 180);
    m_conflictsListCtrl->InsertColumn(3, wxT("Suggested Resolution"), wxLIST_FORMAT_LEFT, 260);
    
    resultsSizer->GetSizer()->Add(m_conflictsListCtrl, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(resultsSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    m_statusLabel = new wxStaticText(this, wxID_ANY, wxT("Ready"));
    mainSizer->Add(m_statusLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_detectBtn = new wxButton(this, ID_DMX_DETECT_BTN, wxT("Detect Conflicts"));
    m_autoRemapBtn = new wxButton(this, ID_DMX_AUTOREMAP_BTN, wxT("Auto-Remap"));
    m_exportBtn = new wxButton(this, ID_DMX_EXPORT_BTN, wxT("Export Report"));
    m_closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));
    
    btnSizer->Add(m_detectBtn, 0, wxALL, 5);
    btnSizer->Add(m_autoRemapBtn, 0, wxALL, 5);
    btnSizer->Add(m_exportBtn, 0, wxALL, 5);
    btnSizer->AddStretchSpacer();
    btnSizer->Add(m_closeBtn, 0, wxALL, 5);
    
    mainSizer->Add(btnSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
    Layout();
    Center();
}

void AIDMXAddressDialog::OnDetectClick(wxCommandEvent& event) {
    DMXAddressAdvisor advisor;
    wxString xml = m_universeXmlCtrl->GetValue();
    m_lastConflicts = advisor.DetectConflicts(xml.ToStdString());

    m_conflictsListCtrl->DeleteAllItems();
    long row = 0;
    for (const auto& conflict : m_lastConflicts) {
        long itemIndex = m_conflictsListCtrl->InsertItem(row, wxString::Format(wxT("%d"), conflict.channel));
        m_conflictsListCtrl->SetItem(itemIndex, 1, wxString(conflict.fixtureA));
        m_conflictsListCtrl->SetItem(itemIndex, 2, wxString(conflict.fixtureB));
        m_conflictsListCtrl->SetItem(itemIndex, 3, wxString(conflict.resolution));
        row++;
    }

    m_statusLabel->SetLabel(wxString::Format(wxT("%ld conflicts detected"), row));
    spdlog::info("AIDMXAddressDialog: {} conflicts detected", row);
}

void AIDMXAddressDialog::OnAutoRemapClick(wxCommandEvent& event) {
    if (m_lastConflicts.empty()) {
        wxMessageBox(wxT("No conflicts to remap."), wxT("Info"), wxICON_INFORMATION | wxOK);
        return;
    }

    DMXAddressAdvisor advisor;
    std::string resultStr = advisor.SuggestRemapping(m_lastConflicts);
    wxString wxResultStr(resultStr);
    
    if (wxResultStr.Length() > 500) {
        wxResultStr = wxResultStr.Mid(0, 497) + wxT("...");
    }
    
    wxMessageBox(wxResultStr, wxT("Auto-Remap Suggestion"), wxICON_INFORMATION | wxOK);
}

void AIDMXAddressDialog::OnExportClick(wxCommandEvent& event) {
    if (m_lastConflicts.empty()) {
        wxMessageBox(wxT("No conflicts to export."), wxT("Export Error"), wxICON_WARNING | wxOK);
        return;
    }
    
    wxFileDialog saveFileDialog(this, wxT("Save Report file"), wxT(""), wxT("dmx_report.txt"), wxT("Text files (*.txt)|*.txt"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) return;

    DMXAddressAdvisor advisor;
    std::string reportStr = advisor.SuggestRemapping(m_lastConflicts);
    
    wxFile file;
    if (file.Open(saveFileDialog.GetPath(), wxFile::write)) {
        file.Write(reportStr);
        file.Close();
        spdlog::info("AIDMXAddressDialog: Exported report to {}", saveFileDialog.GetPath().ToStdString());
    } else {
        spdlog::error("AIDMXAddressDialog: Failed to open file for writing DMX report");
    }
}

void AIDMXAddressDialog::OnCloseClick(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
