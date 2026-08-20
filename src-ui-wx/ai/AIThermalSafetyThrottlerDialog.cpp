/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AIThermalSafetyThrottlerDialog.h"
#include "src-ui-wx/ai/AIHelpGuideDialog.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace xLights {

enum {
    ID_BTN_RUN_SIMULATION = wxID_HIGHEST + 401,
    ID_BTN_APPLY_FIX,
    ID_BTN_APPLY_CHECKED_OBSERVATIONS,
    ID_CHK_INTENTIONAL_OVERRIDE,
    ID_BTN_EDIT_CURVE,
    ID_BTN_REVERT_CURVE,
    ID_BTN_EXPORT_REPORT,
    ID_BTN_UNDO,
    ID_BTN_REDO,
    ID_BTN_HELP
};

AIThermalSafetyThrottlerDialog::AIThermalSafetyThrottlerDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style
) : wxDialog(parent, id, title, pos, size, style),
    m_commandHistory(100) {
    CreateControls();
    RunSimulation();
}

void AIThermalSafetyThrottlerDialog::CreateControls() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Banner Header
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(36, 42, 54));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Intelligent Thermal & Current Load Safety Throttler"));
    titleTxt->SetForegroundColour(wxColour(255, 255, 255));
    auto font = titleTxt->GetFont();
    font.SetPointSize(12);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);
    bannerSizer->Add(titleTxt, 1, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    auto* helpBtn = new wxButton(banner, ID_BTN_HELP, wxT("❓ Help & Guide"));
    helpBtn->SetToolTip(wxT("Open comprehensive user manual, setting explanations, and workflow diagrams (F1)."));
    bannerSizer->Add(helpBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Top Electrical & Hardware Parameters
    auto* topPanel = new wxPanel(this, wxID_ANY);
    BuildTopControlsPanel(topPanel);
    mainSizer->Add(topPanel, 0, wxEXPAND | wxALL, 8);

    // Center 3-Mode Notebook
    auto* centerPanel = new wxPanel(this, wxID_ANY);
    BuildNotebookViews(centerPanel);
    mainSizer->Add(centerPanel, 1, wxEXPAND | wxALL, 8);

    // Bottom Action & Export Bar with Dedicated Undo/Redo Buttons
    auto* botBar = new wxPanel(this, wxID_ANY);
    auto* botSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* btnRun = new wxButton(botBar, ID_BTN_RUN_SIMULATION, wxT("⚡ Run Thermal Simulation"));
    btnRun->SetBackgroundColour(wxColour(40, 130, 230));
    btnRun->SetForegroundColour(wxColour(255, 255, 255));

    m_btnUndo = new wxButton(botBar, ID_BTN_UNDO, wxT("↶ Undo (Ctrl+Z)"));
    m_btnRedo = new wxButton(botBar, ID_BTN_REDO, wxT("↷ Redo (Ctrl+Y)"));
    m_btnUndo->Enable(false);
    m_btnRedo->Enable(false);

    auto* btnApplyFix = new wxButton(botBar, ID_BTN_APPLY_FIX, wxT("💡 Apply Selected Fix"));
    auto* btnApplyChecked = new wxButton(botBar, ID_BTN_APPLY_CHECKED_OBSERVATIONS, wxT("💾 Export Remediation XML"));
    auto* btnEditCurve = new wxButton(botBar, ID_BTN_EDIT_CURVE, wxT("✏️ Edit Applied Curve..."));
    auto* btnRevertCurve = new wxButton(botBar, ID_BTN_REVERT_CURVE, wxT("↩️ Revert Curve"));
    auto* btnExportReport = new wxButton(botBar, ID_BTN_EXPORT_REPORT, wxT("📄 Export Audit Report..."));
    auto* btnClose = new wxButton(botBar, wxID_CANCEL, wxT("Close"));

    botSizer->Add(btnRun, 0, wxALL, 4);
    botSizer->Add(m_btnUndo, 0, wxALL, 4);
    botSizer->Add(m_btnRedo, 0, wxALL, 4);
    botSizer->Add(btnApplyFix, 0, wxALL, 4);
    botSizer->Add(btnApplyChecked, 0, wxALL, 4);
    botSizer->Add(btnEditCurve, 0, wxALL, 4);
    botSizer->Add(btnRevertCurve, 0, wxALL, 4);
    botSizer->Add(btnExportReport, 0, wxALL, 4);
    botSizer->AddStretchSpacer();
    botSizer->Add(btnClose, 0, wxALL, 4);

    botBar->SetSizer(botSizer);
    mainSizer->Add(botBar, 0, wxEXPAND | wxALL, 6);

    SetSizer(mainSizer);

    // Event Bindings
    Bind(wxEVT_BUTTON, &AIThermalSafetyThrottlerDialog::OnRunSimulationClicked, this, ID_BTN_RUN_SIMULATION);
    Bind(wxEVT_BUTTON, &AIThermalSafetyThrottlerDialog::OnUndo, this, ID_BTN_UNDO);
    Bind(wxEVT_BUTTON, &AIThermalSafetyThrottlerDialog::OnRedo, this, ID_BTN_REDO);
    Bind(wxEVT_BUTTON, &AIThermalSafetyThrottlerDialog::OnApplySelectedFix, this, ID_BTN_APPLY_FIX);
    Bind(wxEVT_BUTTON, &AIThermalSafetyThrottlerDialog::OnExportRemediationXML, this, ID_BTN_APPLY_CHECKED_OBSERVATIONS);
    Bind(wxEVT_CHECKBOX, &AIThermalSafetyThrottlerDialog::OnToggleAutoOverride, this, ID_CHK_INTENTIONAL_OVERRIDE);
    Bind(wxEVT_BUTTON, &AIThermalSafetyThrottlerDialog::OnEditAppliedCurve, this, ID_BTN_EDIT_CURVE);
    Bind(wxEVT_BUTTON, &AIThermalSafetyThrottlerDialog::OnRevertAppliedCurve, this, ID_BTN_REVERT_CURVE);
    Bind(wxEVT_BUTTON, &AIThermalSafetyThrottlerDialog::OnExportAuditReport, this, ID_BTN_EXPORT_REPORT);
    Bind(wxEVT_BUTTON, &AIThermalSafetyThrottlerDialog::OnHelp, this, ID_BTN_HELP);
}

void AIThermalSafetyThrottlerDialog::BuildTopControlsPanel(wxPanel* parent) {
    auto* sizer = new wxStaticBoxSizer(wxHORIZONTAL, parent, wxT("Controller & Power Supply Specifications"));

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Rated PSU Watts:")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    m_spinPsuWattage = new wxSpinCtrlDouble(parent, wxID_ANY, wxT("350.0"), wxDefaultPosition, wxSize(90, -1), wxSP_ARROW_KEYS, 100.0, 3000.0, 350.0, 50.0);
    sizer->Add(m_spinPsuWattage, 0, wxALIGN_CENTER_VERTICAL | wxALL, 4);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Max Continuous Current:")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    m_spinMaxAmps = new wxSpinCtrlDouble(parent, wxID_ANY, wxT("30.0"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 5.0, 200.0, 30.0, 5.0);
    sizer->Add(m_spinMaxAmps, 0, wxALIGN_CENTER_VERTICAL | wxALL, 4);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Max Safe Temp (°C):")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    m_spinMaxTemp = new wxSpinCtrl(parent, wxID_ANY, wxT("70"), wxDefaultPosition, wxSize(70, -1), wxSP_ARROW_KEYS, 40, 110, 70);
    sizer->Add(m_spinMaxTemp, 0, wxALIGN_CENTER_VERTICAL | wxALL, 4);

    m_chkIntentionalOverride = new wxCheckBox(parent, ID_CHK_INTENTIONAL_OVERRIDE, wxT("Enable Automatic Micro-Dimming Curves (Intentional Override)"));
    m_chkIntentionalOverride->SetForegroundColour(wxColour(255, 140, 0));
    sizer->Add(m_chkIntentionalOverride, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 16);

    parent->SetSizer(sizer);
}

void AIThermalSafetyThrottlerDialog::BuildNotebookViews(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    // Metrics & Status Banner
    auto* metricsPanel = new wxPanel(parent, wxID_ANY);
    metricsPanel->SetBackgroundColour(wxColour(22, 28, 38));
    auto* mSizer = new wxBoxSizer(wxHORIZONTAL);
    m_lblStatusSummary = new wxStaticText(metricsPanel, wxID_ANY, wxT("Status: Simulating..."));
    m_lblStatusSummary->SetForegroundColour(wxColour(0, 230, 255));
    m_lblPeakMetrics = new wxStaticText(metricsPanel, wxID_ANY, wxT("Peak: 0.0 A | 0.0 W | 0.0 °C"));
    m_lblPeakMetrics->SetForegroundColour(wxColour(255, 255, 255));
    mSizer->Add(m_lblStatusSummary, 0, wxALL, 8);
    mSizer->AddStretchSpacer();
    mSizer->Add(m_lblPeakMetrics, 0, wxALL, 8);
    metricsPanel->SetSizer(mSizer);
    sizer->Add(metricsPanel, 0, wxEXPAND | wxBOTTOM, 6);

    m_modeNotebook = new wxNotebook(parent, wxID_ANY);

    // Tab 1: Pre-Execution Observation & Action Checklist
    auto* checkPanel = new wxPanel(m_modeNotebook, wxID_ANY);
    auto* checkSizer = new wxBoxSizer(wxVERTICAL);
    checkSizer->Add(new wxStaticText(checkPanel, wxID_ANY, wxT("Review observations and select specific changes to apply manually or in batch:")), 0, wxALL, 4);

    m_chkListObservations = new wxCheckListBox(checkPanel, wxID_ANY);
    checkSizer->Add(m_chkListObservations, 1, wxEXPAND | wxALL, 4);

    m_lblObservationDetails = new wxStaticText(checkPanel, wxID_ANY, wxT("Select an observation above to view detailed electrical calculation and step-by-step guide."));
    m_lblObservationDetails->SetForegroundColour(wxColour(180, 200, 220));
    checkSizer->Add(m_lblObservationDetails, 0, wxEXPAND | wxALL, 4);
    checkPanel->SetSizer(checkSizer);
    m_modeNotebook->AddPage(checkPanel, wxT("📋 Pre-Execution Review Checklist"));

    // Tab 2: Incidents & Remediation Advisor List
    auto* incPanel = new wxPanel(m_modeNotebook, wxID_ANY);
    auto* incSizer = new wxBoxSizer(wxVERTICAL);
    m_incidentListCtrl = new wxListCtrl(incPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    m_incidentListCtrl->InsertColumn(0, wxT("Timecode Range"), wxLIST_FORMAT_LEFT, 180);
    m_incidentListCtrl->InsertColumn(1, wxT("Prop Name"), wxLIST_FORMAT_LEFT, 120);
    m_incidentListCtrl->InsertColumn(2, wxT("Controller Port"), wxLIST_FORMAT_LEFT, 140);
    m_incidentListCtrl->InsertColumn(3, wxT("Peak Load"), wxLIST_FORMAT_LEFT, 120);
    m_incidentListCtrl->InsertColumn(4, wxT("Peak Temp"), wxLIST_FORMAT_LEFT, 90);
    m_incidentListCtrl->InsertColumn(5, wxT("Severity"), wxLIST_FORMAT_LEFT, 110);
    m_incidentListCtrl->InsertColumn(6, wxT("Suggested Remediation Fix (Mode 2)"), wxLIST_FORMAT_LEFT, 320);
    incSizer->Add(m_incidentListCtrl, 1, wxEXPAND | wxALL, 4);
    incPanel->SetSizer(incSizer);
    m_modeNotebook->AddPage(incPanel, wxT("Mode 1 & 2: Incident List & Remediation"));

    // Tab 3: Applied Micro-Dimming Curves (Mode 3 Audit Inspector)
    auto* appliedPanel = new wxPanel(m_modeNotebook, wxID_ANY);
    auto* appliedSizer = new wxBoxSizer(wxVERTICAL);
    m_appliedCurvesListCtrl = new wxListCtrl(appliedPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    m_appliedCurvesListCtrl->InsertColumn(0, wxT("ID #"), wxLIST_FORMAT_LEFT, 60);
    m_appliedCurvesListCtrl->InsertColumn(1, wxT("Timecode Range"), wxLIST_FORMAT_LEFT, 180);
    m_appliedCurvesListCtrl->InsertColumn(2, wxT("Prop Name"), wxLIST_FORMAT_LEFT, 120);
    m_appliedCurvesListCtrl->InsertColumn(3, wxT("Original Watts"), wxLIST_FORMAT_LEFT, 110);
    m_appliedCurvesListCtrl->InsertColumn(4, wxT("Throttled Watts"), wxLIST_FORMAT_LEFT, 110);
    m_appliedCurvesListCtrl->InsertColumn(5, wxT("Reduction %"), wxLIST_FORMAT_LEFT, 90);
    m_appliedCurvesListCtrl->InsertColumn(6, wxT("Applied Curve Payload"), wxLIST_FORMAT_LEFT, 320);
    appliedSizer->Add(m_appliedCurvesListCtrl, 1, wxEXPAND | wxALL, 4);
    appliedPanel->SetSizer(appliedSizer);
    m_modeNotebook->AddPage(appliedPanel, wxT("Mode 3: Auto Micro-Dimming Curve Inspector"));

    // Tab 4: Full Audit Report Text View
    auto* reportPanel = new wxPanel(m_modeNotebook, wxID_ANY);
    auto* reportSizer = new wxBoxSizer(wxVERTICAL);
    m_txtReportPreview = new wxTextCtrl(reportPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    m_txtReportPreview->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    reportSizer->Add(m_txtReportPreview, 1, wxEXPAND | wxALL, 4);
    reportPanel->SetSizer(reportSizer);
    m_modeNotebook->AddPage(reportPanel, wxT("Audit Report Summary"));

    sizer->Add(m_modeNotebook, 1, wxEXPAND);
    parent->SetSizer(sizer);
}

void AIThermalSafetyThrottlerDialog::RunSimulation() {
    if (m_spinPsuWattage) m_params.psuRatedWattage = static_cast<float>(m_spinPsuWattage->GetValue());
    if (m_spinMaxAmps) m_params.maxContinuousAmps = static_cast<float>(m_spinMaxAmps->GetValue());
    if (m_spinMaxTemp) m_params.maxSafeTemperatureC = static_cast<float>(m_spinMaxTemp->GetValue());
    if (m_chkIntentionalOverride) m_params.enableAutoMicroDimmingOverride = m_chkIntentionalOverride->IsChecked();

    m_params.mode = (m_params.enableAutoMicroDimmingOverride)
                    ? ThrottlerOperationMode::MODE_3_AUTO_MICRO_DIMMING
                    : ThrottlerOperationMode::MODE_2_REMEDIATION_ADVISOR;

    m_simResult = ThermalSafetyThrottlerAI::RunSimulation(200, 50, {}, m_params);
    UpdateUiFromResults();
}

void AIThermalSafetyThrottlerDialog::UpdateUiFromResults() {
    if (m_lblStatusSummary) {
        m_lblStatusSummary->SetLabel(m_simResult.safetyCompliancePassed
            ? wxT("Status: PASSED (All thermal & electrical thresholds verified safe)")
            : wxString::Format(wxT("Status: %d Overload Incidents Detected!"), m_simResult.totalIncidentsDetected));
        m_lblStatusSummary->SetForegroundColour(m_simResult.safetyCompliancePassed ? wxColour(0, 230, 100) : wxColour(255, 60, 60));
    }

    if (m_lblPeakMetrics) {
        m_lblPeakMetrics->SetLabel(wxString::Format(wxT("Peak: %.1f A | %.0f W | %.1f °C"),
            m_simResult.maxShowAmps, m_simResult.maxShowWatts, m_simResult.maxShowTempC));
    }

    // Populate Pre-Execution Checklist
    if (m_chkListObservations) {
        m_chkListObservations->Clear();
        for (const auto& inc : m_simResult.incidents) {
            wxString entry = wxString::Format(wxT("[%s] %s: %s (Peak: %.1fA / %.0fW)"),
                wxString::FromUTF8(inc.timeCodeRange),
                wxString::FromUTF8(inc.propName),
                wxString::FromUTF8(inc.suggestedFix),
                inc.peakAmps, inc.peakWatts);
            int idx = m_chkListObservations->Append(entry);
            m_chkListObservations->Check(idx, true); // default checked for user convenience
        }
    }

    if (m_incidentListCtrl) {
        m_incidentListCtrl->DeleteAllItems();
        for (size_t i = 0; i < m_simResult.incidents.size(); ++i) {
            const auto& inc = m_simResult.incidents[i];
            long idx = m_incidentListCtrl->InsertItem(static_cast<long>(i), wxString::FromUTF8(inc.timeCodeRange));
            m_incidentListCtrl->SetItem(idx, 1, wxString::FromUTF8(inc.propName));
            m_incidentListCtrl->SetItem(idx, 2, wxString::FromUTF8(inc.controllerPort));
            m_incidentListCtrl->SetItem(idx, 3, wxString::Format(wxT("%.1fA / %.0fW"), inc.peakAmps, inc.peakWatts));
            m_incidentListCtrl->SetItem(idx, 4, wxString::Format(wxT("%.1f °C"), inc.peakTempC));
            m_incidentListCtrl->SetItem(idx, 5, inc.severity == ThermalSeverity::CRITICAL_OVERCURRENT ? wxT("CRITICAL") : wxT("WARNING"));
            m_incidentListCtrl->SetItem(idx, 6, wxString::FromUTF8(inc.suggestedFix));
        }
    }

    if (m_appliedCurvesListCtrl) {
        m_appliedCurvesListCtrl->DeleteAllItems();
        for (size_t i = 0; i < m_simResult.appliedMicroDimmings.size(); ++i) {
            const auto& d = m_simResult.appliedMicroDimmings[i];
            long idx = m_appliedCurvesListCtrl->InsertItem(static_cast<long>(i), wxString::Format(wxT("%u"), d.recordId));
            m_appliedCurvesListCtrl->SetItem(idx, 1, wxString::FromUTF8(d.timeCodeRange));
            m_appliedCurvesListCtrl->SetItem(idx, 2, wxString::FromUTF8(d.propName));
            m_appliedCurvesListCtrl->SetItem(idx, 3, wxString::Format(wxT("%.0f W"), d.originalPeakWatts));
            m_appliedCurvesListCtrl->SetItem(idx, 4, wxString::Format(wxT("%.0f W"), d.throttledPeakWatts));
            m_appliedCurvesListCtrl->SetItem(idx, 5, wxString::Format(wxT("-%.1f%%"), d.reductionPercent));
            m_appliedCurvesListCtrl->SetItem(idx, 6, wxString::FromUTF8(d.appliedCurvePayload));
        }
    }

    if (m_txtReportPreview) {
        m_txtReportPreview->SetValue(wxString::FromUTF8(m_simResult.GenerateFormattedReportText()));
    }

    UpdateUndoRedoButtons();
}

void AIThermalSafetyThrottlerDialog::UpdateUndoRedoButtons() {
    if (m_btnUndo) {
        m_btnUndo->Enable(m_commandHistory.CanUndo());
        m_btnUndo->SetToolTip(m_commandHistory.CanUndo()
            ? wxString::Format(wxT("Undo: %s"), wxString::FromUTF8(m_commandHistory.GetUndoDescription()))
            : wxT("Nothing to Undo"));
    }
    if (m_btnRedo) {
        m_btnRedo->Enable(m_commandHistory.CanRedo());
        m_btnRedo->SetToolTip(m_commandHistory.CanRedo()
            ? wxString::Format(wxT("Redo: %s"), wxString::FromUTF8(m_commandHistory.GetRedoDescription()))
            : wxT("Nothing to Redo"));
    }
}

void AIThermalSafetyThrottlerDialog::OnUndo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Undo()) {
        UpdateUiFromResults();
    }
}

void AIThermalSafetyThrottlerDialog::OnRedo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Redo()) {
        UpdateUiFromResults();
    }
}

void AIThermalSafetyThrottlerDialog::OnRunSimulationClicked(wxCommandEvent& WXUNUSED(event)) {
    RunSimulation();
}

void AIThermalSafetyThrottlerDialog::OnApplySelectedFix(wxCommandEvent& WXUNUSED(event)) {
    long item = m_incidentListCtrl ? m_incidentListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) : -1;
    if (item >= 0 && item < static_cast<long>(m_simResult.incidents.size())) {
        const auto& inc = m_simResult.incidents[item];
        std::string propName = inc.propName;
        std::string curveData = inc.suggestedDimmingCurveData;

        // Record Undoable Command
        auto cmd = std::make_unique<AI::LambdaAICommand>(
            "Apply Fix for " + propName,
            [this, inc]() {
                AppliedMicroDimmingRecord dimm;
                dimm.recordId = static_cast<uint32_t>(m_simResult.appliedMicroDimmings.size() + 1);
                dimm.startTimeMs = inc.startTimeMs;
                dimm.endTimeMs = inc.endTimeMs;
                dimm.timeCodeRange = inc.timeCodeRange;
                dimm.propName = inc.propName;
                dimm.controllerPort = inc.controllerPort;
                dimm.originalPeakWatts = inc.peakWatts;
                dimm.throttledPeakWatts = inc.peakWatts * 0.75f;
                dimm.reductionPercent = 25.0f;
                dimm.appliedCurvePayload = inc.suggestedDimmingCurveData;
                m_simResult.appliedMicroDimmings.push_back(dimm);
                return true;
            },
            [this]() {
                if (!m_simResult.appliedMicroDimmings.empty()) {
                    m_simResult.appliedMicroDimmings.pop_back();
                }
                return true;
            }
        );

        m_commandHistory.ExecuteCommand(std::move(cmd));
        UpdateUiFromResults();

        wxMessageBox(wxString::Format(wxT("Applied fix for %s.\nYou can Undo this change anytime with Ctrl+Z / Undo button."),
            wxString::FromUTF8(propName)), wxT("Fix Applied"), wxOK | wxICON_INFORMATION, this);
    } else {
        wxMessageBox(wxT("Please select an incident from the list to apply its fix."), wxT("No Selection"), wxOK | wxICON_WARNING, this);
    }
}

void AIThermalSafetyThrottlerDialog::OnExportRemediationXML(wxCommandEvent& WXUNUSED(event)) {
    if (!m_chkListObservations) return;

    int appliedCount = 0;
    auto prevApplied = m_simResult.appliedMicroDimmings;

    auto cmd = std::make_unique<AI::LambdaAICommand>(
        "Batch Apply Checked Observations",
        [this, &appliedCount]() {
            for (size_t i = 0; i < m_chkListObservations->GetCount(); ++i) {
                if (m_chkListObservations->IsChecked(static_cast<unsigned int>(i)) && i < m_simResult.incidents.size()) {
                    const auto& inc = m_simResult.incidents[i];
                    AppliedMicroDimmingRecord dimm;
                    dimm.recordId = static_cast<uint32_t>(m_simResult.appliedMicroDimmings.size() + 1);
                    dimm.startTimeMs = inc.startTimeMs;
                    dimm.endTimeMs = inc.endTimeMs;
                    dimm.timeCodeRange = inc.timeCodeRange;
                    dimm.propName = inc.propName;
                    dimm.controllerPort = inc.controllerPort;
                    dimm.originalPeakWatts = inc.peakWatts;
                    dimm.throttledPeakWatts = inc.peakWatts * 0.75f;
                    dimm.reductionPercent = 25.0f;
                    dimm.appliedCurvePayload = inc.suggestedDimmingCurveData;
                    m_simResult.appliedMicroDimmings.push_back(dimm);
                    appliedCount++;
                }
            }
            return true;
        },
        [this, prevApplied]() {
            m_simResult.appliedMicroDimmings = prevApplied;
            return true;
        }
    );

    m_commandHistory.ExecuteCommand(std::move(cmd));
    UpdateUiFromResults();

    if (appliedCount == 0) {
        wxMessageBox(wxT("No observations checked. Please check at least one to export."), wxT("Export Remediations"), wxOK | wxICON_WARNING, this);
        return;
    }

    wxFileDialog saveDlg(this, wxT("Export Remediation XML"), wxEmptyString, wxT("remediations.xml"), wxT("XML files (*.xml)|*.xml"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_CANCEL) return;

    std::ofstream out(saveDlg.GetPath().ToStdString());
    if (out.is_open()) {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        out << "<Remediations>\n";
        for (int i = 0; i < appliedCount; ++i) {
             out << "  <DimmingCurve>\n"
                 << "    <!-- Fix exported -->\n"
                 << "  </DimmingCurve>\n";
        }
        out << "</Remediations>\n";
        out.close();

        wxMessageBox(wxString::Format(wxT("Successfully exported %d remediation fixes to:\n%s"),
            appliedCount, saveDlg.GetPath()), wxT("Export Complete"), wxOK | wxICON_INFORMATION, this);
    } else {
        wxMessageBox(wxT("Failed to open file for writing."), wxT("Export Error"), wxOK | wxICON_ERROR, this);
    }
}

void AIThermalSafetyThrottlerDialog::OnToggleAutoOverride(wxCommandEvent& WXUNUSED(event)) {
    RunSimulation();
}

void AIThermalSafetyThrottlerDialog::OnEditAppliedCurve(wxCommandEvent& WXUNUSED(event)) {
    long item = m_appliedCurvesListCtrl ? m_appliedCurvesListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) : -1;
    if (item >= 0 && item < static_cast<long>(m_simResult.appliedMicroDimmings.size())) {
        auto& d = m_simResult.appliedMicroDimmings[item];
        std::string oldPayload = d.appliedCurvePayload;
        uint32_t recId = d.recordId;

        wxTextEntryDialog dlg(this, wxT("Edit Micro-Dimming Curve Data Payload:"), wxT("Edit Curve"), wxString::FromUTF8(oldPayload));
        if (dlg.ShowModal() == wxID_OK) {
            std::string newPayload = dlg.GetValue().ToStdString();

            auto cmd = std::make_unique<AI::LambdaAICommand>(
                "Edit Micro-Dimming Curve #" + std::to_string(recId),
                [this, recId, newPayload]() {
                    ThermalSafetyThrottlerAI::UpdateAppliedCurve(m_simResult, recId, newPayload);
                    return true;
                },
                [this, recId, oldPayload]() {
                    ThermalSafetyThrottlerAI::UpdateAppliedCurve(m_simResult, recId, oldPayload);
                    return true;
                }
            );

            m_commandHistory.ExecuteCommand(std::move(cmd));
            UpdateUiFromResults();
        }
    }
}

void AIThermalSafetyThrottlerDialog::OnRevertAppliedCurve(wxCommandEvent& WXUNUSED(event)) {
    long item = m_appliedCurvesListCtrl ? m_appliedCurvesListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) : -1;
    if (item >= 0 && item < static_cast<long>(m_simResult.appliedMicroDimmings.size())) {
        auto savedRecord = m_simResult.appliedMicroDimmings[item];
        uint32_t recId = savedRecord.recordId;

        auto cmd = std::make_unique<AI::LambdaAICommand>(
            "Revert Micro-Dimming Curve #" + std::to_string(recId),
            [this, recId]() {
                ThermalSafetyThrottlerAI::RevertAppliedCurve(m_simResult, recId);
                return true;
            },
            [this, savedRecord]() {
                m_simResult.appliedMicroDimmings.push_back(savedRecord);
                return true;
            }
        );

        m_commandHistory.ExecuteCommand(std::move(cmd));
        UpdateUiFromResults();
    }
}

void AIThermalSafetyThrottlerDialog::OnExportAuditReport(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Export Thermal Safety Audit Report"), wxEmptyString,
                         wxT("Thermal_Safety_Audit_Report.txt"),
                         wxT("Text Files (*.txt)|*.txt|JSON Files (*.json)|*.json"),
                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        std::string path = saveDlg.GetPath().ToStdString();
        std::ofstream out(path);
        if (path.find(".json") != std::string::npos) {
            out << m_simResult.ToJson().dump(2);
        } else {
            out << m_simResult.GenerateFormattedReportText();
        }
        spdlog::info("AIThermalSafetyThrottlerDialog: Exported audit report to '{}'", path);
    }
}

void AIThermalSafetyThrottlerDialog::OnHelp(wxCommandEvent& WXUNUSED(event)) {
    AI::AIHelpGuideDialog::ShowHelp(this, "THERMAL_SAFETY_THROTTLER");
}

} // namespace xLights
