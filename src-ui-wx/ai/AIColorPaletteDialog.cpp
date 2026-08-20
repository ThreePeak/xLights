#include "AIColorPaletteDialog.h"
#include "src-ui-wx/ai/AIHelpGuideDialog.h"

//(*InternalHeaders(AIColorPaletteDialog)
#include <wx/string.h>
//*)

//(*IdInit(AIColorPaletteDialog)
const wxWindowID AIColorPaletteDialog::ID_STATICTEXT1 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_CHOICE1 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_RADIOBUTTON1 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_TEXTCTRL1 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_RADIOBUTTON2 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_TEXTCTRL2 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_HTMLWINDOW1 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_BUTTON1 = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_OK = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_CANCEL = wxNewId();
const wxWindowID AIColorPaletteDialog::ID_HELP = wxNewId();
//*)

BEGIN_EVENT_TABLE(AIColorPaletteDialog,wxDialog)
    //(*EventTable(AIColorPaletteDialog)
    //*)
END_EVENT_TABLE()

#include "../xLightsMain.h"
#include "../xLightsApp.h"
#include "../sequencer/MainSequencer.h"
#include "../ai/aiBase.h"

AIColorPaletteDialog::AIColorPaletteDialog(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(AIColorPaletteDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxStaticBoxSizer* StaticBoxSizer2;

    Create(parent, id, _T("AI Color Palette Generator"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER, _T("id"));
    SetMinSize(wxSize(760, 600));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(2);

    // Modern Header Banner
    auto* banner = new wxPanel(this, wxID_ANY);
    banner->SetBackgroundColour(wxColour(40, 26, 44));
    auto* bannerSizer = new wxBoxSizer(wxHORIZONTAL);
    
    auto* textSizer = new wxBoxSizer(wxVERTICAL);
    auto* titleTxt = new wxStaticText(banner, wxID_ANY, wxT("AI Color Palette & Mood Theme Generator"));
    titleTxt->SetForegroundColour(*wxWHITE);
    wxFont titleFont = titleTxt->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleTxt->SetFont(titleFont);

    auto* subTitle = new wxStaticText(banner, wxID_ANY,
        wxT("Synthesize harmonious multi-layer color palettes tailored to song mood, BPM tempo, and sequencing style."));
    subTitle->SetForegroundColour(wxColour(230, 190, 230));

    textSizer->Add(titleTxt, 0, wxALL, 8);
    textSizer->Add(subTitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    bannerSizer->Add(textSizer, 1, wxEXPAND);

    auto* helpBtn = new wxButton(banner, ID_HELP, wxT("❓ Help & Guide"));
    helpBtn->SetToolTip(wxT("Open comprehensive user manual, setting explanations, and workflow diagrams (F1)."));
    bannerSizer->Add(helpBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);

    banner->SetSizer(bannerSizer);
    FlexGridSizer1->Add(banner, 0, wxEXPAND | wxBOTTOM, 5);

    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Parameters"));
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer2->AddGrowableCol(1);

    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _T("AI Service:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    AIServiceChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    AIServiceChoice->SetToolTip(wxT("Select AI provider service for color palette synthesis."));
    FlexGridSizer2->Add(AIServiceChoice, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

    SongRadioButton = new wxRadioButton(this, ID_RADIOBUTTON1, _T("Song Title / Artist:"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
    SongRadioButton->SetValue(true);
    FlexGridSizer2->Add(SongRadioButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SongTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(500,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    SongTextCtrl->SetMaxLength(250);
    SongTextCtrl->SetToolTip(wxT("Song title and artist used to extract emotion and musical tone."));
    FlexGridSizer2->Add(SongTextCtrl, 1, wxALL|wxEXPAND, 5);

    FreeFormRadioButton = new wxRadioButton(this, ID_RADIOBUTTON2, _T("Free Form Prompt:"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
    FlexGridSizer2->Add(FreeFormRadioButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
    FreeFormText = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FreeFormText->SetToolTip(wxT("Custom description of desired colors, gradients, and lighting moods."));
    FreeFormText->Disable();
    FlexGridSizer2->Add(FreeFormText, 1, wxALL|wxEXPAND, 5);

    StaticBoxSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);

    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Synthesized Palettes"));
    ResultHTMLCtrl = new wxHtmlWindow(this, ID_HTMLWINDOW1, wxDefaultPosition, wxSize(-1,260), wxHW_SCROLLBAR_AUTO, _T("ID_HTMLWINDOW1"));
    StaticBoxSizer2->Add(ResultHTMLCtrl, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);

    FlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
    GenerateButton = new wxButton(this, ID_BUTTON1, _T("🎨 Generate Palette"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    GenerateButton->SetBackgroundColour(wxColour(180, 50, 140));
    GenerateButton->SetForegroundColour(*wxWHITE);
    GenerateButton->SetToolTip(wxT("Synthesize harmonious color swatches and gradients."));
    GenerateButton->SetDefault();
    FlexGridSizer3->Add(GenerateButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    OkButon = new wxButton(this, ID_OK, _T("Apply Palette"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_OK"));
    FlexGridSizer3->Add(OkButon, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    CancelButton = new wxButton(this, ID_CANCEL, _T("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CANCEL"));
    FlexGridSizer3->Add(CancelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->SetSizeHints(this);
    Center();

    Connect(ID_RADIOBUTTON1, wxEVT_COMMAND_RADIOBUTTON_SELECTED, (wxObjectEventFunction)&AIColorPaletteDialog::OnSongRadioButtonSelect);
    Connect(ID_TEXTCTRL1, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&AIColorPaletteDialog::OnSongTextCtrlText);
    Connect(ID_TEXTCTRL1, wxEVT_COMMAND_TEXT_ENTER, (wxObjectEventFunction)&AIColorPaletteDialog::OnSongTextCtrlTextEnter);
    Connect(ID_RADIOBUTTON2, wxEVT_COMMAND_RADIOBUTTON_SELECTED, (wxObjectEventFunction)&AIColorPaletteDialog::OnFreeFormRadioButtonSelect);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIColorPaletteDialog::OnGenerateButtonClick);
    Connect(ID_OK, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIColorPaletteDialog::OnOkButonClick);
    Connect(ID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIColorPaletteDialog::OnCancelButtonClick);
    Connect(ID_HELP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIColorPaletteDialog::OnHelpButtonClick);
    //*)

    if (xLightsFrame::CurrentSeqXmlFile && xLightsFrame::CurrentSeqXmlFile->GetMedia()) {
        auto title = xLightsFrame::CurrentSeqXmlFile->GetMedia()->Title();
        auto artist = xLightsFrame::CurrentSeqXmlFile->GetMedia()->Artist();
        if (!artist.empty()) {
            title = title + " by " + artist;
        }
        SongTextCtrl->SetValue(title);
        createFreeFormFromSong();
    }
    
    auto services = xLightsApp::GetFrame() ? xLightsApp::GetFrame()->GetAIServices(aiType::COLORPALETTES) : std::vector<aiBase*>();
    for (auto s : services) {
        AIServiceChoice->Append(s->GetLLMName());
    }
    if (!services.empty()) {
        AIServiceChoice->SetSelection(0);
    }
}

AIColorPaletteDialog::~AIColorPaletteDialog()
{
    //(*Destroy(AIColorPaletteDialog)
    //*)
}
void AIColorPaletteDialog::createFreeFormFromSong() {
    auto song = SongTextCtrl->GetValue();
    if (!song.empty()) {
        song = "from the song " + song;
    }
    FreeFormText->SetValue(song);
}


void AIColorPaletteDialog::OnSongRadioButtonSelect(wxCommandEvent& event)
{
    SongTextCtrl->Enable();
    FreeFormText->Disable();
}

void AIColorPaletteDialog::OnSongTextCtrlTextEnter(wxCommandEvent& event)
{
    createFreeFormFromSong();
}

void AIColorPaletteDialog::OnFreeFormRadioButtonSelect(wxCommandEvent& event)
{
    SongTextCtrl->Disable();
    FreeFormText->Enable();
}

void AIColorPaletteDialog::OnGenerateButtonClick(wxCommandEvent& event)
{
    auto prompt = FreeFormText->GetValue();
    auto services = xLightsApp::GetFrame() ? xLightsApp::GetFrame()->GetAIServices(aiType::COLORPALETTES) : std::vector<aiBase*>();
    int sel = AIServiceChoice->GetSelection();
    if (services.empty() || sel < 0 || sel >= static_cast<int>(services.size())) {
        wxMessageBox(wxT("No AI Color Palette service is currently configured or available. Please configure an AI Provider in Settings."),
                     wxT("AI Service Unavailable"), wxOK | wxICON_WARNING, this);
        return;
    }

    aiBase::AIColorPalette cp = services[sel]->GenerateColorPalette(prompt);
    colors.clear();
    std::string html;
    html += "<html><body>\n";
    if (cp.error.empty()) {
        html += "<b>Description:</b> " + cp.description + "<br>\n";
        html += "<br><ul>\n";
        for (size_t x  = 0; x < cp.colors.size(); x++) {
            html += "<li>";
            html += "<span style='background-color: black; color: " + cp.colors[x].hexValue + "'><b>" + cp.colors[x].name + "</b></span> (" + cp.colors[x].hexValue + ")- " + cp.colors[x].description;
            html += "</li>\n";
            colors.push_back(cp.colors[x].hexValue);
        }
        
        html += "</ul>";
    } else {
        html += "<b>Error: </b> "  + cp.error;
    }
    html += "</body></html>";
    ResultHTMLCtrl->SetPage(html);
}

void AIColorPaletteDialog::OnOkButonClick(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}

void AIColorPaletteDialog::OnCancelButtonClick(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

void AIColorPaletteDialog::OnHelpButtonClick(wxCommandEvent& event)
{
    xLights::AI::AIHelpGuideDialog::ShowHelp(this, "COLOR_PALETTES");
}

wxArrayString AIColorPaletteDialog::GetColorStrings() {
    return colors;
}

void AIColorPaletteDialog::OnSongTextCtrlText(wxCommandEvent& event)
{
    createFreeFormFromSong();
}
