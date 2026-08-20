/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-ui-wx/ai/AISequenceVisualGitDialog.h"
#include <wx/filedlg.h>
#include <wx/filepicker.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace xLights {

enum {
    ID_BTN_COMPARE_DIFF = wxID_HIGHEST + 1101,
    ID_BTN_MERGE_EXPORT,
    ID_CHOICE_RESOLUTION,
    ID_BTN_UNDO,
    ID_BTN_REDO,
    ID_BTN_SWAP,
    ID_CTX_STRAT_BASE,
    ID_CTX_STRAT_INCOMING,
    ID_CTX_STRAT_SUB_LAYERS,
    ID_CTX_STRAT_BLEND
};

AISequenceVisualGitDialog::AISequenceVisualGitDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style
) : wxDialog(parent, id, title, pos, size, style),
    m_commandHistory(100) {
    CreateControls();
}

void AISequenceVisualGitDialog::CreateControls() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(40, 32, 54));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Sequence Visual Git & Timeline Diff Merge Assistant"));
    titleTxt->SetForegroundColour(wxColour(255, 255, 255));
    auto font = titleTxt->GetFont();
    font.SetPointSize(12);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(font);
    bannerSizer->Add(titleTxt, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    banner->SetSizer(bannerSizer);
    mainSizer->Add(banner, 0, wxEXPAND);

    // Top Selector
    auto* topPanel = new wxPanel(this, wxID_ANY);
    BuildTopSequenceSelector(topPanel);
    mainSizer->Add(topPanel, 0, wxEXPAND | wxALL, 8);

    // Center Diff View
    auto* centerPanel = new wxPanel(this, wxID_ANY);
    BuildCenterDiffView(centerPanel);
    mainSizer->Add(centerPanel, 1, wxEXPAND | wxALL, 8);

    // Bottom Action Bar
    auto* botBar = new wxPanel(this, wxID_ANY);
    auto* botSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* btnCompare = new wxButton(botBar, ID_BTN_COMPARE_DIFF, wxT("🔍 Compare Sequence Diff"));
    btnCompare->SetBackgroundColour(wxColour(140, 40, 200));
    btnCompare->SetForegroundColour(wxColour(255, 255, 255));

    m_btnUndo = new wxButton(botBar, ID_BTN_UNDO, wxT("↶ Undo (Ctrl+Z)"));
    m_btnRedo = new wxButton(botBar, ID_BTN_REDO, wxT("↷ Redo (Ctrl+Y)"));
    m_btnUndo->Enable(false);
    m_btnRedo->Enable(false);

    auto* btnMerge = new wxButton(botBar, ID_BTN_MERGE_EXPORT, wxT("🔀 Resolve & Export Merged .xsq..."));
    auto* btnClose = new wxButton(botBar, wxID_CANCEL, wxT("Close"));

    botSizer->Add(btnCompare, 0, wxALL, 4);
    botSizer->Add(m_btnUndo, 0, wxALL, 4);
    botSizer->Add(m_btnRedo, 0, wxALL, 4);
    botSizer->Add(btnMerge, 0, wxALL, 4);
    botSizer->AddStretchSpacer();
    botSizer->Add(btnClose, 0, wxALL, 4);

    botBar->SetSizer(botSizer);
    mainSizer->Add(botBar, 0, wxEXPAND | wxALL, 6);

    SetSizer(mainSizer);

    // Event Bindings
    Bind(wxEVT_BUTTON, &AISequenceVisualGitDialog::OnCompareSequences, this, ID_BTN_COMPARE_DIFF);
    Bind(wxEVT_BUTTON, &AISequenceVisualGitDialog::OnMergeAndExport, this, ID_BTN_MERGE_EXPORT);
    Bind(wxEVT_CHOICE, &AISequenceVisualGitDialog::OnConflictStrategyChanged, this, ID_CHOICE_RESOLUTION);
    Bind(wxEVT_BUTTON, &AISequenceVisualGitDialog::OnUndo, this, ID_BTN_UNDO);
    Bind(wxEVT_BUTTON, &AISequenceVisualGitDialog::OnRedo, this, ID_BTN_REDO);
    Bind(wxEVT_BUTTON, &AISequenceVisualGitDialog::OnSwapSequences, this, ID_BTN_SWAP);
    
    if (m_diffListCtrl) {
        m_diffListCtrl->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &AISequenceVisualGitDialog::OnListRightClick, this);
    }
    
    Bind(wxEVT_MENU, &AISequenceVisualGitDialog::OnContextMenuAction, this, ID_CTX_STRAT_BASE);
    Bind(wxEVT_MENU, &AISequenceVisualGitDialog::OnContextMenuAction, this, ID_CTX_STRAT_INCOMING);
    Bind(wxEVT_MENU, &AISequenceVisualGitDialog::OnContextMenuAction, this, ID_CTX_STRAT_SUB_LAYERS);
    Bind(wxEVT_MENU, &AISequenceVisualGitDialog::OnContextMenuAction, this, ID_CTX_STRAT_BLEND);
}

void AISequenceVisualGitDialog::BuildTopSequenceSelector(wxPanel* parent) {
    auto* sizer = new wxStaticBoxSizer(wxHORIZONTAL, parent, wxT("Sequence Version Control Source Pair"));

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Base Sequence (Master A):")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    m_pickerBaseSeq = new wxFilePickerCtrl(parent, wxID_ANY, wxEmptyString, wxT("Select Base Sequence"), wxT("xLights Sequence (*.xsq)|*.xsq"), wxDefaultPosition, wxSize(250, -1));
    sizer->Add(m_pickerBaseSeq, 1, wxALIGN_CENTER_VERTICAL | wxALL, 4);
    
    auto* btnSwap = new wxButton(parent, ID_BTN_SWAP, wxT("⇄"), wxDefaultPosition, wxSize(30, -1));
    sizer->Add(btnSwap, 0, wxALIGN_CENTER_VERTICAL | wxALL, 4);

    sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Incoming Branch (B):")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 12);
    m_pickerIncomingSeq = new wxFilePickerCtrl(parent, wxID_ANY, wxEmptyString, wxT("Select Incoming Sequence"), wxT("xLights Sequence (*.xsq)|*.xsq"), wxDefaultPosition, wxSize(250, -1));
    sizer->Add(m_pickerIncomingSeq, 1, wxALIGN_CENTER_VERTICAL | wxALL, 4);

    parent->SetSizer(sizer);
}

void AISequenceVisualGitDialog::BuildCenterDiffView(wxPanel* parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    // Summary Metric Banner
    auto* summaryPanel = new wxPanel(parent, wxID_ANY);
    summaryPanel->SetBackgroundColour(wxColour(24, 20, 32));
    auto* smSizer = new wxBoxSizer(wxHORIZONTAL);
    m_lblDiffSummary = new wxStaticText(summaryPanel, wxID_ANY, wxT("Diff: +1 Added | -0 Deleted | ~1 Modified | !1 Conflict"));
    m_lblDiffSummary->SetForegroundColour(wxColour(220, 160, 255));
    smSizer->Add(m_lblDiffSummary, 0, wxALL, 8);

    smSizer->AddStretchSpacer();
    smSizer->Add(new wxStaticText(summaryPanel, wxID_ANY, wxT("Global Conflict Strategy:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    wxArrayString strategies;
    strategies.Add(wxT("Keep Base Layer A"));
    strategies.Add(wxT("Accept Incoming Branch B"));
    strategies.Add(wxT("Non-Destructive Stack (Separate Sub-Layers)"));
    strategies.Add(wxT("AI Intelligent Crossfade Blend"));
    m_choiceResolutionStrategy = new wxChoice(summaryPanel, ID_CHOICE_RESOLUTION, wxDefaultPosition, wxDefaultSize, strategies);
    m_choiceResolutionStrategy->SetSelection(2);
    smSizer->Add(m_choiceResolutionStrategy, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    summaryPanel->SetSizer(smSizer);
    sizer->Add(summaryPanel, 0, wxEXPAND | wxBOTTOM, 6);

    auto* notebook = new wxNotebook(parent, wxID_ANY);

    // Tab 1: Timeline Diff List
    auto* listPanel = new wxPanel(notebook, wxID_ANY);
    auto* listSizer = new wxBoxSizer(wxVERTICAL);
    m_diffListCtrl = new wxListCtrl(listPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    m_diffListCtrl->InsertColumn(0, wxT("Change Type"), wxLIST_FORMAT_LEFT, 110);
    m_diffListCtrl->InsertColumn(1, wxT("Target Model"), wxLIST_FORMAT_LEFT, 120);
    m_diffListCtrl->InsertColumn(2, wxT("Layer"), wxLIST_FORMAT_LEFT, 60);
    m_diffListCtrl->InsertColumn(3, wxT("Time Interval"), wxLIST_FORMAT_LEFT, 130);
    m_diffListCtrl->InsertColumn(4, wxT("Effect Type"), wxLIST_FORMAT_LEFT, 110);
    m_diffListCtrl->InsertColumn(5, wxT("Resolution Action"), wxLIST_FORMAT_LEFT, 240);
    listSizer->Add(m_diffListCtrl, 1, wxEXPAND | wxALL, 4);
    listPanel->SetSizer(listSizer);
    notebook->AddPage(listPanel, wxT("📋 Effect Timeline Diff Elements"));

    // Tab 2: Formatted Semantic Diff Log
    auto* logPanel = new wxPanel(notebook, wxID_ANY);
    auto* logSizer = new wxBoxSizer(wxVERTICAL);
    m_txtFormattedReport = new wxTextCtrl(logPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    m_txtFormattedReport->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    logSizer->Add(m_txtFormattedReport, 1, wxEXPAND | wxALL, 4);
    logPanel->SetSizer(logSizer);
    notebook->AddPage(logPanel, wxT("Semantic Merge Audit Log"));

    sizer->Add(notebook, 1, wxEXPAND);
    parent->SetSizer(sizer);
}

void AISequenceVisualGitDialog::OnCompareSequences(wxCommandEvent& WXUNUSED(event)) {
    std::string base = (m_pickerBaseSeq ? m_pickerBaseSeq->GetPath().ToStdString() : "");
    std::string incoming = (m_pickerIncomingSeq ? m_pickerIncomingSeq->GetPath().ToStdString() : "");
    
    if (base.empty() || incoming.empty()) {
        wxMessageBox(wxT("Please select both a Base Sequence and an Incoming Branch to compare."), wxT("Missing Files"), wxOK | wxICON_WARNING, this);
        return;
    }

    auto prev = m_report;
    m_report = AI::SequenceVisualGitAI::CompareSequences("<base/>", "<incoming/>", base, incoming);

    auto cmd = std::make_unique<AI::LambdaAICommand>(
        "Compare Sequence Diff: " + base + " vs " + incoming,
        [this]() { return true; },
        [this, prev]() {
            m_report = prev;
            UpdateUiFromResults();
            return true;
        }
    );

    m_commandHistory.ExecuteCommand(std::move(cmd));
    UpdateUiFromResults();
}

void AISequenceVisualGitDialog::UpdateUiFromResults() {
    if (m_lblDiffSummary) {
        m_lblDiffSummary->SetLabel(wxString::Format(wxT("Diff: +%zu Added | -%zu Deleted | ~%zu Modified | !%zu Conflict"),
            m_report.additionsCount, m_report.deletionsCount, m_report.modificationsCount, m_report.conflictsCount));
    }

    if (m_diffListCtrl) {
        m_diffListCtrl->DeleteAllItems();
        for (size_t i = 0; i < m_report.diffItems.size(); ++i) {
            const auto& item = m_report.diffItems[i];
            wxString typeStr;
            switch (item.changeType) {
                case AI::DiffChangeType::ADDED: typeStr = wxT("+ ADDED"); break;
                case AI::DiffChangeType::DELETED: typeStr = wxT("- DELETED"); break;
                case AI::DiffChangeType::MODIFIED: typeStr = wxT("~ MODIFIED"); break;
                case AI::DiffChangeType::CONFLICT: typeStr = wxT("! CONFLICT"); break;
                default: typeStr = wxT("UNCHANGED"); break;
            }

            long idx = m_diffListCtrl->InsertItem(static_cast<long>(i), typeStr);
            m_diffListCtrl->SetItem(idx, 1, wxString::FromUTF8(item.modelName));
            m_diffListCtrl->SetItem(idx, 2, wxString::Format(wxT("%d"), item.layerIndex));
            m_diffListCtrl->SetItem(idx, 3, wxString::Format(wxT("%.1fs - %.1fs"), item.startMs / 1000.0f, item.endMs / 1000.0f));
            m_diffListCtrl->SetItem(idx, 4, wxString::FromUTF8(item.effectType));

            wxString strat;
            switch (item.chosenStrategy) {
                case AI::MergeStrategy::KEEP_BASE: strat = wxT("Keep Base Layer A"); break;
                case AI::MergeStrategy::KEEP_INCOMING: strat = wxT("Accept Incoming Branch B"); break;
                case AI::MergeStrategy::SPLIT_SUB_LAYERS: strat = wxT("Non-Destructive Stack (Sub-Layer)"); break;
                case AI::MergeStrategy::INTELLIGENT_BLEND: strat = wxT("AI Crossfade Blend"); break;
            }
            m_diffListCtrl->SetItem(idx, 5, strat);
        }
    }

    if (m_txtFormattedReport) {
        m_txtFormattedReport->SetValue(wxString::FromUTF8(m_report.GenerateFormattedReport()));
    }

    UpdateUndoRedoButtons();
}

void AISequenceVisualGitDialog::UpdateUndoRedoButtons() {
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

void AISequenceVisualGitDialog::OnUndo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Undo()) {
        UpdateUiFromResults();
    }
}

void AISequenceVisualGitDialog::OnRedo(wxCommandEvent& WXUNUSED(event)) {
    if (m_commandHistory.Redo()) {
        UpdateUiFromResults();
    }
}

void AISequenceVisualGitDialog::OnConflictStrategyChanged(wxCommandEvent& WXUNUSED(event)) {
    if (!m_choiceResolutionStrategy) return;
    auto strat = static_cast<AI::MergeStrategy>(m_choiceResolutionStrategy->GetSelection());
    for (auto& item : m_report.diffItems) {
        if (item.changeType == AI::DiffChangeType::CONFLICT) {
            item.chosenStrategy = strat;
        }
    }
    UpdateUiFromResults();
}

void AISequenceVisualGitDialog::OnMergeAndExport(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog saveDlg(this, wxT("Export Merged Sequence File"), wxEmptyString,
                         wxT("Merged_Show.xsq"),
                         wxT("xLights Sequence (*.xsq)|*.xsq"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDlg.ShowModal() == wxID_OK) {
        std::ofstream out(saveDlg.GetPath().ToStdString());
        out << AI::SequenceVisualGitAI::ResolveAndMergeSequence(m_report, "<base/>");
        spdlog::info("AISequenceVisualGitDialog: Exported merged sequence to '{}'", saveDlg.GetPath().ToStdString());
    }
}

void AISequenceVisualGitDialog::OnSwapSequences(wxCommandEvent& WXUNUSED(event)) {
    if (m_pickerBaseSeq && m_pickerIncomingSeq) {
        wxString temp = m_pickerBaseSeq->GetPath();
        m_pickerBaseSeq->SetPath(m_pickerIncomingSeq->GetPath());
        m_pickerIncomingSeq->SetPath(temp);
    }
}

void AISequenceVisualGitDialog::OnListRightClick(wxListEvent& event) {
    m_contextItemIndex = event.GetIndex();
    if (m_contextItemIndex >= 0 && m_contextItemIndex < static_cast<long>(m_report.diffItems.size())) {
        wxMenu menu;
        menu.Append(ID_CTX_STRAT_BASE, wxT("Keep Base Layer A"));
        menu.Append(ID_CTX_STRAT_INCOMING, wxT("Accept Incoming Branch B"));
        menu.Append(ID_CTX_STRAT_SUB_LAYERS, wxT("Non-Destructive Stack (Separate Sub-Layers)"));
        menu.Append(ID_CTX_STRAT_BLEND, wxT("AI Intelligent Crossfade Blend"));
        PopupMenu(&menu);
    }
}

void AISequenceVisualGitDialog::OnContextMenuAction(wxCommandEvent& event) {
    if (m_contextItemIndex >= 0 && m_contextItemIndex < static_cast<long>(m_report.diffItems.size())) {
        auto prev = m_report;
        
        switch (event.GetId()) {
            case ID_CTX_STRAT_BASE: m_report.diffItems[m_contextItemIndex].chosenStrategy = AI::MergeStrategy::KEEP_BASE; break;
            case ID_CTX_STRAT_INCOMING: m_report.diffItems[m_contextItemIndex].chosenStrategy = AI::MergeStrategy::KEEP_INCOMING; break;
            case ID_CTX_STRAT_SUB_LAYERS: m_report.diffItems[m_contextItemIndex].chosenStrategy = AI::MergeStrategy::SPLIT_SUB_LAYERS; break;
            case ID_CTX_STRAT_BLEND: m_report.diffItems[m_contextItemIndex].chosenStrategy = AI::MergeStrategy::INTELLIGENT_BLEND; break;
        }

        auto cmd = std::make_unique<AI::LambdaAICommand>(
            "Change Row Resolution Strategy",
            [this]() { return true; },
            [this, prev]() {
                m_report = prev;
                UpdateUiFromResults();
                return true;
            }
        );

        m_commandHistory.ExecuteCommand(std::move(cmd));
        UpdateUiFromResults();
    }
}

} // namespace xLights
