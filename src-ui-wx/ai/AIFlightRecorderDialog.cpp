#include "src-ui-wx/ai/AIFlightRecorderDialog.h"
#include <wx/sizer.h>
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/utils.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace xLights::AI {

enum {
    ID_REC_START = 27001,
    ID_REC_STOP,
    ID_REC_EXPORT,
    ID_REC_VIEW_HTML,
    ID_REC_CLEAR,
    ID_REC_TIMER,
    ID_REC_STEP_LIST
};

BEGIN_EVENT_TABLE(AIFlightRecorderDialog, wxDialog)
    EVT_BUTTON(ID_REC_START, AIFlightRecorderDialog::OnStartRecord)
    EVT_BUTTON(ID_REC_STOP, AIFlightRecorderDialog::OnStopRecord)
    EVT_BUTTON(ID_REC_EXPORT, AIFlightRecorderDialog::OnExportPackage)
    EVT_BUTTON(ID_REC_VIEW_HTML, AIFlightRecorderDialog::OnViewHtmlReport)
    EVT_BUTTON(ID_REC_CLEAR, AIFlightRecorderDialog::OnClearSession)
    EVT_BUTTON(wxID_CANCEL, AIFlightRecorderDialog::OnCloseClick)
    EVT_LIST_ITEM_SELECTED(ID_REC_STEP_LIST, AIFlightRecorderDialog::OnStepSelected)
    EVT_TIMER(ID_REC_TIMER, AIFlightRecorderDialog::OnTimerTick)
END_EVENT_TABLE()

AIFlightRecorderDialog::AIFlightRecorderDialog(wxWindow* parent,
                                               wxWindowID id,
                                               const wxString& title,
                                               const wxPoint& pos,
                                               const wxSize& size,
                                               long style)
    : wxDialog(parent, id, title, pos, size, style)
    , m_updateTimer(this, ID_REC_TIMER)
{
    InitUI();
    UpdateControls();
    RefreshStepList();
    m_updateTimer.Start(500); // 500ms tick for live timer and counter
}

AIFlightRecorderDialog::~AIFlightRecorderDialog() {
    m_updateTimer.Stop();
}

void AIFlightRecorderDialog::InitUI() {
    SetBackgroundColour(wxColour(22, 27, 34));
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Top Header Card
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);
    headerPanel->SetBackgroundColour(wxColour(13, 17, 23));
    wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* title = new wxStaticText(headerPanel, wxID_ANY, wxT("📋 AI Problem Steps & Flight Recorder"));
    title->SetForegroundColour(*wxWHITE);
    wxFont tf = title->GetFont();
    tf.SetWeight(wxFONTWEIGHT_BOLD);
    tf.SetPointSize(tf.GetPointSize() + 2);
    title->SetFont(tf);
    headerSizer->Add(title, 1, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    m_statusBadge = new wxStaticText(headerPanel, wxID_ANY, wxT("⏹ STANDBY"));
    m_statusBadge->SetForegroundColour(wxColour(140, 150, 160));
    wxFont bf = m_statusBadge->GetFont();
    bf.SetWeight(wxFONTWEIGHT_BOLD);
    m_statusBadge->SetFont(bf);
    headerSizer->Add(m_statusBadge, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    m_timerLabel = new wxStaticText(headerPanel, wxID_ANY, wxT("00:00 [0 steps]"));
    m_timerLabel->SetForegroundColour(wxColour(88, 166, 255));
    headerSizer->Add(m_timerLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    headerPanel->SetSizer(headerSizer);
    mainSizer->Add(headerPanel, 0, wxEXPAND | wxBOTTOM, 8);

    // Control Toolbar
    wxBoxSizer* toolSizer = new wxBoxSizer(wxHORIZONTAL);
    m_startBtn = new wxButton(this, ID_REC_START, wxT("⏺ Start Recording Session"));
    m_startBtn->SetBackgroundColour(wxColour(46, 160, 67));
    m_startBtn->SetForegroundColour(*wxWHITE);

    m_stopBtn = new wxButton(this, ID_REC_STOP, wxT("⏹ Stop Recording"));
    m_stopBtn->SetBackgroundColour(wxColour(218, 54, 51));
    m_stopBtn->SetForegroundColour(*wxWHITE);

    m_viewHtmlBtn = new wxButton(this, ID_REC_VIEW_HTML, wxT("🌐 View HTML Report"));
    m_exportBtn = new wxButton(this, ID_REC_EXPORT, wxT("📦 Export Diagnostic Bundle (.zip)"));
    m_clearBtn = new wxButton(this, ID_REC_CLEAR, wxT("🗑 Clear"));

    toolSizer->Add(m_startBtn, 0, wxRIGHT, 6);
    toolSizer->Add(m_stopBtn, 0, wxRIGHT, 6);
    toolSizer->Add(m_viewHtmlBtn, 0, wxRIGHT, 6);
    toolSizer->Add(m_exportBtn, 0, wxRIGHT, 6);
    toolSizer->Add(m_clearBtn, 0, wxRIGHT, 6);

    mainSizer->Add(toolSizer, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Steps List (Report View)
    m_stepList = new wxListCtrl(this, ID_REC_STEP_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    m_stepList->InsertColumn(0, wxT("#"), wxLIST_FORMAT_LEFT, 45);
    m_stepList->InsertColumn(1, wxT("Elapsed"), wxLIST_FORMAT_LEFT, 75);
    m_stepList->InsertColumn(2, wxT("Category"), wxLIST_FORMAT_LEFT, 110);
    m_stepList->InsertColumn(3, wxT("Subsystem"), wxLIST_FORMAT_LEFT, 140);
    m_stepList->InsertColumn(4, wxT("Action / Details"), wxLIST_FORMAT_LEFT, 380);
    m_stepList->InsertColumn(5, wxT("Latency"), wxLIST_FORMAT_RIGHT, 80);

    mainSizer->Add(m_stepList, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Payload / State Dump Inspector
    wxStaticText* inspLabel = new wxStaticText(this, wxID_ANY, wxT("Step State & Payload Inspector:"));
    inspLabel->SetForegroundColour(wxColour(140, 150, 160));
    mainSizer->Add(inspLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 4);

    m_payloadViewer = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 120),
                                     wxTE_MULTILINE | wxTE_READONLY | wxHSCROLL);
    m_payloadViewer->SetBackgroundColour(wxColour(13, 17, 23));
    m_payloadViewer->SetForegroundColour(wxColour(121, 192, 255));
    mainSizer->Add(m_payloadViewer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Bottom Action Row
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    m_closeBtn = new wxButton(this, wxID_CANCEL, wxT("Close"));
    bottomSizer->AddStretchSpacer(1);
    bottomSizer->Add(m_closeBtn, 0, wxALIGN_CENTER_VERTICAL, 0);

    mainSizer->Add(bottomSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
}

void AIFlightRecorderDialog::UpdateControls() {
    bool recording = AIFlightRecorder::Instance().IsRecording();
    uint32_t stepCount = AIFlightRecorder::Instance().GetStepCount();

    m_startBtn->Enable(!recording);
    m_stopBtn->Enable(recording);
    m_viewHtmlBtn->Enable(stepCount > 0);
    m_exportBtn->Enable(stepCount > 0 && !recording);
    m_clearBtn->Enable(!recording && stepCount > 0);

    if (recording) {
        m_statusBadge->SetLabel(wxT("⏺ RECORDING ACTIVE"));
        m_statusBadge->SetForegroundColour(wxColour(248, 81, 73));
    } else {
        m_statusBadge->SetLabel(stepCount > 0 ? wxT("⏹ SESSION READY") : wxT("⏹ STANDBY"));
        m_statusBadge->SetForegroundColour(wxColour(140, 150, 160));
    }

    int64_t ms = AIFlightRecorder::Instance().GetCurrentElapsedMs();
    int totalSec = static_cast<int>(ms / 1000);
    int min = totalSec / 60;
    int sec = totalSec % 60;
    m_timerLabel->SetLabel(wxString::Format(wxT("%02d:%02d [%u steps]"), min, sec, stepCount));
}

void AIFlightRecorderDialog::RefreshStepList() {
    m_stepList->DeleteAllItems();
    auto steps = AIFlightRecorder::Instance().GetSteps();

    for (size_t i = 0; i < steps.size(); ++i) {
        const auto& s = steps[i];
        long itemIdx = m_stepList->InsertItem(static_cast<long>(i), wxString::Format(wxT("%u"), s.stepNumber));
        m_stepList->SetItem(itemIdx, 1, wxString::Format(wxT("+%lld ms"), s.elapsedMs));
        m_stepList->SetItem(itemIdx, 2, wxString::FromUTF8(FlightCategoryToString(s.category)));
        m_stepList->SetItem(itemIdx, 3, wxString::FromUTF8(s.subsystem));
        m_stepList->SetItem(itemIdx, 4, wxString::FromUTF8(s.action + " — " + s.details));
        m_stepList->SetItem(itemIdx, 5, s.durationMs > 0 ? wxString::Format(wxT("%lld ms"), s.durationMs) : wxString(wxT("-")));

        if (s.isError) {
            m_stepList->SetItemBackgroundColour(itemIdx, wxColour(60, 20, 20));
            m_stepList->SetItemTextColour(itemIdx, wxColour(255, 120, 120));
        }
    }

    if (!steps.empty()) {
        m_stepList->EnsureVisible(static_cast<long>(steps.size() - 1));
    }
}

void AIFlightRecorderDialog::OnStartRecord(wxCommandEvent& event) {
    AIFlightRecorder::Instance().StartSession("Problem Steps Recording");
    UpdateControls();
    RefreshStepList();
}

void AIFlightRecorderDialog::OnStopRecord(wxCommandEvent& event) {
    AIFlightRecorder::Instance().StopSession();
    UpdateControls();
    RefreshStepList();
}

void AIFlightRecorderDialog::OnViewHtmlReport(wxCommandEvent& event) {
    std::string html = AIFlightRecorder::Instance().GenerateHtmlReport();
    wxString tempPath = wxFileName::CreateTempFileName("xlights_ai_flight_record");
    tempPath += ".html";

    std::ofstream out(tempPath.ToStdString());
    if (out.is_open()) {
        out << html;
        out.close();
        wxLaunchDefaultBrowser(tempPath);
    } else {
        wxMessageBox(wxT("Failed to generate temporary HTML report."), wxT("Error"), wxOK | wxICON_ERROR, this);
    }
}

void AIFlightRecorderDialog::OnExportPackage(wxCommandEvent& event) {
    auto meta = AIFlightRecorder::Instance().GetMetadata();
    wxString defaultName = wxString::FromUTF8(meta.sessionId) + ".zip";

    wxFileDialog fd(this, wxT("Export Diagnostic Flight Bundle"), wxEmptyString, defaultName,
                     wxT("ZIP Archive (*.zip)|*.zip"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (fd.ShowModal() != wxID_OK) return;

    wxString targetPath = fd.GetPath();

    // Create zip archive
    wxFileOutputStream outStream(targetPath);
    if (!outStream.IsOk()) {
        wxMessageBox(wxT("Could not open destination file for writing."), wxT("Export Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    wxZipOutputStream zipStream(outStream);

    // 1. session_trace.json
    std::string jsonStr = AIFlightRecorder::Instance().GenerateJsonTrace();
    zipStream.PutNextEntry(wxT("session_trace.json"));
    zipStream.Write(jsonStr.data(), jsonStr.size());

    // 2. session_report.html
    std::string htmlStr = AIFlightRecorder::Instance().GenerateHtmlReport();
    zipStream.PutNextEntry(wxT("session_report.html"));
    zipStream.Write(htmlStr.data(), htmlStr.size());

    // 3. session_summary.txt
    std::ostringstream ss;
    ss << "xLights AI Flight Record Summary\n"
       << "================================\n"
       << "Session ID: " << meta.sessionId << "\n"
       << "Session Name: " << meta.sessionName << "\n"
       << "xLights Build: " << meta.xLightsVersion << "\n"
       << "Start Time: " << meta.startTimeIso << "\n"
       << "Duration: " << (meta.totalDurationMs / 1000.0f) << " s\n"
       << "Total Steps: " << meta.totalSteps << "\n"
       << "Total Errors: " << meta.totalErrors << "\n"
       << "Active Subsystems: " << meta.activeSubsystems.size() << "\n";
    std::string sumStr = ss.str();
    zipStream.PutNextEntry(wxT("session_summary.txt"));
    zipStream.Write(sumStr.data(), sumStr.size());

    zipStream.Close();
    outStream.Close();

    wxMessageBox(wxString::Format(wxT("Diagnostic Flight Bundle successfully exported to:\n%s"), targetPath),
                 wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
}

void AIFlightRecorderDialog::OnClearSession(wxCommandEvent& event) {
    if (AIFlightRecorder::Instance().IsRecording()) return;
    AIFlightRecorder::Instance().StartSession("New Session");
    AIFlightRecorder::Instance().StopSession();
    UpdateControls();
    RefreshStepList();
    m_payloadViewer->Clear();
}

void AIFlightRecorderDialog::OnCloseClick(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}

void AIFlightRecorderDialog::OnStepSelected(wxListEvent& event) {
    long sel = event.GetIndex();
    auto steps = AIFlightRecorder::Instance().GetSteps();
    if (sel >= 0 && static_cast<size_t>(sel) < steps.size()) {
        const auto& s = steps[sel];
        std::ostringstream ss;
        ss << "Step #" << s.stepNumber << " [" << FlightCategoryToString(s.category) << "] "
           << s.subsystem << " :: " << s.action << "\n"
           << "Timestamp: " << s.timestampIso << " (+ " << s.elapsedMs << " ms)\n"
           << "Latency: " << s.durationMs << " ms\n"
           << "Details: " << s.details << "\n\n"
           << "Payload State Dump:\n"
           << (s.payload.empty() ? "{}" : s.payload.dump(2));
        m_payloadViewer->SetValue(wxString::FromUTF8(ss.str()));
    }
}

void AIFlightRecorderDialog::OnTimerTick(wxTimerEvent& event) {
    if (AIFlightRecorder::Instance().IsRecording()) {
        UpdateControls();
        auto steps = AIFlightRecorder::Instance().GetSteps();
        if (m_stepList->GetItemCount() != static_cast<int>(steps.size())) {
            RefreshStepList();
        }
    }
}

} // namespace xLights::AI
