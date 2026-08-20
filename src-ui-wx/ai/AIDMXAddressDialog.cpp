#include "src-ui-wx/ai/AIDMXAddressDialog.h"
#include "src-ui-wx/ai/AIHelpGuideDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>

namespace xLights::AI {

enum {
    ID_DMX_LOAD_FILE_BTN = 14000,
    ID_DMX_DETECT_BTN = 14001,
    ID_DMX_AUTOREMAP_BTN = 14002,
    ID_DMX_EXPORT_BTN = 14003,
    ID_DMX_HELP_BTN = 14004
};

wxBEGIN_EVENT_TABLE(AIDMXAddressDialog, wxDialog)
    EVT_BUTTON(ID_DMX_LOAD_FILE_BTN, AIDMXAddressDialog::OnLoadFileClick)
    EVT_BUTTON(ID_DMX_DETECT_BTN, AIDMXAddressDialog::OnDetectClick)
    EVT_BUTTON(ID_DMX_AUTOREMAP_BTN, AIDMXAddressDialog::OnAutoRemapClick)
    EVT_BUTTON(ID_DMX_EXPORT_BTN, AIDMXAddressDialog::OnExportClick)
    EVT_BUTTON(ID_DMX_HELP_BTN, AIDMXAddressDialog::OnHelpClick)
    EVT_BUTTON(wxID_CANCEL, AIDMXAddressDialog::OnCloseClick)
wxEND_EVENT_TABLE()

AIDMXAddressDialog::AIDMXAddressDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
    InitUI();
}

void AIDMXAddressDialog::InitUI() {
    SetMinSize(wxSize(740, 560));
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Modern Header Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(28, 40, 56));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    
    auto* textSizer = new wxBoxSizer(wxVERTICAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI DMX & Universe Address Conflict Advisor"));
    titleTxt->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleTxt->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(titleFont);

    auto* subTitle = new wxStaticText(banner, wxID_ANY,
        wxT("Automated detection and intelligent remapping of overlapping channels across DMX universes."));
    subTitle->SetForegroundColour(wxColour(170, 200, 230));

    textSizer->Add(titleTxt, 0, wxALL, 8);
    textSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    bannerSizer->Add(textSizer, 1, wxEXPAND);

    auto* helpBtn = new wxButton(banner, ID_DMX_HELP_BTN, wxT("❓ Help & Guide"));
    helpBtn->SetToolTip(wxT("Open comprehensive user manual, setting explanations, and workflow diagrams (F1)."));
    bannerSizer->Add(helpBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Universe XML Input Section
    wxStaticBoxSizer* inputSizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Universe XML Configuration Input"));
    m_universeXmlCtrl = new wxTextCtrl(this, wxID_ANY, wxT("<universes>\n  <!-- Paste or load XML here -->\n</universes>"), wxDefaultPosition, wxSize(-1, 90), wxTE_MULTILINE);
    m_universeXmlCtrl->SetToolTip(wxT("Paste or load universe XML definitions to check for channel overlaps."));
    inputSizer->Add(m_universeXmlCtrl, 1, wxEXPAND | wxALL, 5);

    auto* inputBtnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_loadFileBtn = new wxButton(this, ID_DMX_LOAD_FILE_BTN, wxT("📂 Load Universe XML File..."));
    m_loadFileBtn->SetToolTip(wxT("Load an xLights universe configuration XML file from disk."));
    inputBtnSizer->Add(m_loadFileBtn, 0, wxALL, 4);
    inputSizer->Add(inputBtnSizer, 0, wxALIGN_RIGHT);

    mainSizer->Add(inputSizer, 0, wxEXPAND | wxALL, 10);

    // Detected Conflicts Table
    wxStaticBoxSizer* resultsSizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Detected Conflicts & Resolutions"));
    m_conflictsListCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxBORDER_SUNKEN);
    m_conflictsListCtrl->InsertColumn(0, wxT("Channel"), wxLIST_FORMAT_LEFT, 80);
    m_conflictsListCtrl->InsertColumn(1, wxT("Fixture A"), wxLIST_FORMAT_LEFT, 180);
    m_conflictsListCtrl->InsertColumn(2, wxT("Fixture B"), wxLIST_FORMAT_LEFT, 180);
    m_conflictsListCtrl->InsertColumn(3, wxT("Suggested Resolution"), wxLIST_FORMAT_LEFT, 260);
    
    resultsSizer->Add(m_conflictsListCtrl, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(resultsSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    m_statusLabel = new wxStaticText(this, wxID_ANY, wxT("🟢 Ready. Load XML configuration and click Detect Conflicts."));
    mainSizer->Add(m_statusLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Action Bar
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_detectBtn = new wxButton(this, ID_DMX_DETECT_BTN, wxT("🔍 Detect Conflicts"));
    m_detectBtn->SetBackgroundColour(wxColour(30, 140, 220));
    m_detectBtn->SetForegroundColour(*wxWHITE);
    m_detectBtn->SetToolTip(wxT("Analyze the XML input to discover overlapping channel assignments."));

    m_autoRemapBtn = new wxButton(this, ID_DMX_AUTOREMAP_BTN, wxT("⚡ Auto-Remap"));
    m_autoRemapBtn->SetToolTip(wxT("Automatically compute conflict-free DMX universe and channel assignments."));

    m_exportBtn = new wxButton(this, ID_DMX_EXPORT_BTN, wxT("📄 Export Report"));
    m_exportBtn->SetToolTip(wxT("Export resolved configuration to XML or JSON manifest."));

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

void AIDMXAddressDialog::OnLoadFileClick(wxCommandEvent& event) {
    wxFileDialog openDlg(this, wxT("Select Universe XML File"), wxEmptyString, wxEmptyString,
                         wxT("XML files (*.xml)|*.xml|All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openDlg.ShowModal() == wxID_CANCEL) return;

    std::ifstream file(openDlg.GetPath().ToStdString());
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        m_universeXmlCtrl->SetValue(wxString::FromUTF8(buffer.str()));
        m_statusLabel->SetLabel(wxString::Format(wxT("Loaded: %s"), openDlg.GetFilename()));
    } else {
        wxMessageBox(wxT("Failed to open XML file."), wxT("Open Error"), wxOK | wxICON_ERROR, this);
    }
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
        wxMessageBox(wxString::Format(wxT("Report successfully exported to:\n%s"), saveFileDialog.GetPath()), wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    } else {
        spdlog::error("AIDMXAddressDialog: Failed to open file for writing DMX report");
        wxMessageBox(wxT("Failed to open file for writing."), wxT("Export Error"), wxOK | wxICON_ERROR, this);
    }
}

void AIDMXAddressDialog::OnHelpClick(wxCommandEvent& event) {
    AIHelpGuideDialog::ShowHelp(this, "DMX_ADDRESS_ADVISOR");
}

void AIDMXAddressDialog::OnCloseClick(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}

} // namespace xLights::AI
