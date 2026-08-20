/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/slider.h>
#include <wx/listctrl.h>
#include <wx/clrpicker.h>
#include <wx/filepicker.h>
#include "src-core/models/Photo3DPropReconstructorAI.h"
#include "src-core/ai/AICommandHistory.h"

namespace xLights {

class AIPhoto3DPropReconstructorDialog : public wxDialog {
public:
    AIPhoto3DPropReconstructorDialog(wxWindow* parent,
                                     wxWindowID id = wxID_ANY,
                                     const wxString& title = wxT("AI 3D Photo Prop Mesh Reconstructor"),
                                     const wxPoint& pos = wxDefaultPosition,
                                     const wxSize& size = wxSize(1080, 780),
                                     long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX);

    virtual ~AIPhoto3DPropReconstructorDialog() = default;

    /// Direct programmatic inspection for unit testing
    const ReconstructedPropModel& GetCurrentModel() const { return m_currentModel; }
    void SetTestImageSingle(const std::string& path) { m_singleImagePath = path; }

    void TriggerReconstruction();
    void OnEvenlySpace(wxCommandEvent& event);
    void OnStraighten(wxCommandEvent& event);
    void OnSnapGrid(wxCommandEvent& event);
    void OnSymmetrize(wxCommandEvent& event);
    void OnReverseWiring(wxCommandEvent& event);
    void OnAutoSubmodels(wxCommandEvent& event);

    // Undo / Redo handlers
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);

    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnSaveAsCopy(wxCommandEvent& event);
    void OnDuplicate(wxCommandEvent& event);
    void OnExportAs(wxCommandEvent& event);
    void OnCopyToClipboard(wxCommandEvent& event);

private:
    void CreateControls();
    void BuildLeftControlsPanel(wxPanel* parent);
    void BuildRightCanvasPanel(wxPanel* parent);
    void RefreshCanvas();
    void UpdateStats();
    void UpdateUndoRedoButtons();
    void PushModelMutation(const std::string& actionName, const ReconstructedPropModel& previousState);

    // Data members
    ReconstructorParameters m_params;
    ReconstructedPropModel m_currentModel;
    std::string m_singleImagePath{"test_prop.png"};
    std::vector<ReconstructorInputImage> m_multiImages;
    std::string m_currentSavedPath;
    AI::AICommandHistory m_commandHistory;

    // UI Widgets
    wxNotebook* m_modeNotebook{nullptr};
    wxFilePickerCtrl* m_singleFilePicker{nullptr};
    wxSlider* m_sliderEdgeSens{nullptr};
    wxSlider* m_sliderSmoothing{nullptr};
    wxSpinCtrlDouble* m_spinSpacing{nullptr};
    wxSlider* m_sliderDepthCurvature{nullptr};
    wxSpinCtrl* m_spinTargetNodes{nullptr};
    wxSpinCtrlDouble* m_spinGridSnap{nullptr};
    wxSpinCtrl* m_spinSymmetryFolds{nullptr};

    wxButton* m_btnUndo{nullptr};
    wxButton* m_btnRedo{nullptr};

    wxStaticText* m_lblNodeCount{nullptr};
    wxStaticText* m_lblDimensions{nullptr};
    wxStaticText* m_lblSubmodels{nullptr};
    wxPanel* m_canvasPanel{nullptr};
    wxListCtrl* m_nodeListCtrl{nullptr};

    void OnPaintCanvas(wxPaintEvent& event);
};

} // namespace xLights
