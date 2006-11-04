#include "GalaxySetupWnd.h"

#include "CUIControls.h"
#include "CUISpin.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../universe/Universe.h"

#include "../util/Directories.h"
#include <boost/filesystem/fstream.hpp>

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>


namespace {
    const int CONTROL_MARGIN = 5;
    const int CONTROL_HEIGHT = 30;
    const int PANEL_CONTROL_SPACING = 33;
    const int GAL_SETUP_PANEL_HT = PANEL_CONTROL_SPACING * 6;
    const int GAL_SETUP_WND_WD = 645;
    const int GAL_SETUP_WND_HT = 326;
    const GG::Pt PREVIEW_SZ(248, 186);
    const bool ALLOW_NO_STARLANES = false;

    // persistant between-executions galaxy setup settings, mainly so I don't have to redo these settings to what I want every time I run FO to test something
    void AddOptions(OptionsDB& db) {
        db.Add("GameSetup.stars", "OPTIONS_DB_GAMESETUP_STARS", 100, RangedValidator<int>(10, 500));
        db.Add("GameSetup.galaxy-shape", "OPTIONS_DB_GAMESETUP_GALAXY_SHAPE", SPIRAL_3, RangedValidator<Shape>(SPIRAL_2, RING));
        db.Add("GameSetup.galaxy-age", "OPTIONS_DB_GAMESETUP_GALAXY_AGE", AGE_MATURE, RangedValidator<Age>(AGE_YOUNG, AGE_ANCIENT));
        db.Add("GameSetup.planet-density", "OPTIONS_DB_GAMESETUP_PLANET_DENSITY", PD_AVERAGE, RangedValidator<PlanetDensity>(PD_LOW, PD_HIGH));
        db.Add("GameSetup.starlane-frequency", "OPTIONS_DB_GAMESETUP_STARLANE_FREQUENCY", LANES_SEVERAL, RangedValidator<StarlaneFrequency>(ALLOW_NO_STARLANES ? LANES_NONE : LANES_FEW, LANES_VERY_MANY));
        db.Add("GameSetup.specials-frequency", "OPTIONS_DB_GAMESETUP_SPECIALS_FREQUENCY", SPECIALS_UNCOMMON, RangedValidator<SpecialsFrequency>(SPECIALS_NONE, SPECIALS_COMMON));
    }
    bool temp_bool = RegisterOptions(&AddOptions);
}

////////////////////////////////////////////////
// GalaxySetupPanel
////////////////////////////////////////////////
GalaxySetupPanel::GalaxySetupPanel(int x, int y, int w/* = DEFAULT_WIDTH*/) :
    GG::Control(x, y, w, GAL_SETUP_PANEL_HT, 0),
    m_stars_spin(0),
    m_galaxy_shapes_list(0),
    m_galaxy_ages_list(0),
    m_starlane_freq_list(0),
    m_planet_density_list(0),
    m_specials_freq_list(0)
{
    TempUISoundDisabler sound_disabler;

    const int LABELS_WIDTH = (w - CONTROL_MARGIN) / 2;
    const int DROPLIST_WIDTH = LABELS_WIDTH;
    const int DROPLIST_HEIGHT = ClientUI::Pts() + 4;
    const int TEXT_ROW_HEIGHT = CUISimpleDropDownListRow::DEFAULT_ROW_HEIGHT;
    const int MAX_DROPLIST_DROP_HEIGHT = TEXT_ROW_HEIGHT * 5;
    const int TOTAL_LISTBOX_MARGIN = 4;
    int row = -1;

    boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());

    AttachChild(new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_STARS"), font, ClientUI::TextColor(), GG::TF_RIGHT));
    m_stars_spin = new CUISpin<int>(LABELS_WIDTH + 2 * CONTROL_MARGIN, row * PANEL_CONTROL_SPACING, 75, 100, 1, 10, 500, true);
    m_stars_spin->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_stars_spin->Height()) / 2));

    AttachChild(new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_SHAPE"), font, ClientUI::TextColor(), GG::TF_RIGHT));
    int drop_height = std::min(TEXT_ROW_HEIGHT * GALAXY_SHAPES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_galaxy_shapes_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row * PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_galaxy_shapes_list->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_galaxy_shapes_list->Height()) / 2));
    m_galaxy_shapes_list->SetStyle(GG::LB_NOSORT);

    AttachChild(new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_AGE"), font, ClientUI::TextColor(), GG::TF_RIGHT));
    drop_height = std::min(TEXT_ROW_HEIGHT * NUM_UNIVERSE_AGES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_galaxy_ages_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row * PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_galaxy_ages_list->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_galaxy_ages_list->Height()) / 2));
    m_galaxy_ages_list->SetStyle(GG::LB_NOSORT);

    AttachChild(new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_STARLANE_FREQ"), font, ClientUI::TextColor(), GG::TF_RIGHT));
    drop_height = std::min(TEXT_ROW_HEIGHT * NUM_STARLANE_FREQENCIES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_starlane_freq_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row * PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_starlane_freq_list->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_starlane_freq_list->Height()) / 2));
    m_starlane_freq_list->SetStyle(GG::LB_NOSORT);

    AttachChild(new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_PLANET_DENSITY"), font, ClientUI::TextColor(), GG::TF_RIGHT));
    drop_height = std::min(TEXT_ROW_HEIGHT * NUM_UNIVERSE_PLANET_DENSITIES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_planet_density_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row* PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_planet_density_list->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_planet_density_list->Height()) / 2));
    m_planet_density_list->SetStyle(GG::LB_NOSORT);

    AttachChild(new GG::TextControl(CONTROL_MARGIN, ++row* PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_SPECIALS_FREQ"), font, ClientUI::TextColor(), GG::TF_RIGHT));
    drop_height = std::min(TEXT_ROW_HEIGHT * NUM_SPECIALS_FREQENCIES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_specials_freq_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row* PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_specials_freq_list->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_specials_freq_list->Height()) / 2));
    m_specials_freq_list->SetStyle(GG::LB_NOSORT);

    Init();
}

int GalaxySetupPanel::Systems() const
{
    return m_stars_spin->Value();
}

Shape GalaxySetupPanel::GetShape() const
{
    return Shape(m_galaxy_shapes_list->CurrentItemIndex());
}

Age GalaxySetupPanel::GetAge() const
{
    return Age(m_galaxy_ages_list->CurrentItemIndex());
}

StarlaneFrequency GalaxySetupPanel::GetStarlaneFrequency() const
{
    return StarlaneFrequency(m_starlane_freq_list->CurrentItemIndex() + (ALLOW_NO_STARLANES ? 0 : 1));
}

PlanetDensity GalaxySetupPanel::GetPlanetDensity() const
{
    return PlanetDensity(m_planet_density_list->CurrentItemIndex());
}

SpecialsFrequency GalaxySetupPanel::GetSpecialsFrequency() const
{
    return SpecialsFrequency(m_specials_freq_list->CurrentItemIndex());
}

boost::shared_ptr<GG::Texture> GalaxySetupPanel::PreviewImage() const
{
    return m_textures[GetShape()];
}

XMLElement GalaxySetupPanel::XMLEncode() const
{
    using boost::lexical_cast;
    using std::string;

    XMLElement retval("universe_params");
    retval.AppendChild(XMLElement("size", lexical_cast<string>(Systems())));
    retval.AppendChild(XMLElement("shape", lexical_cast<string>(GetShape())));
    retval.AppendChild(XMLElement("age", lexical_cast<string>(GetAge())));
    retval.AppendChild(XMLElement("starlane_freq", lexical_cast<string>(GetStarlaneFrequency())));
    retval.AppendChild(XMLElement("planet_density", lexical_cast<string>(GetPlanetDensity())));
    retval.AppendChild(XMLElement("specials_freq", lexical_cast<string>(GetSpecialsFrequency())));
    return retval;
}

void GalaxySetupPanel::Disable(bool b/* = true*/)
{
    for (std::list<GG::Wnd*>::const_iterator it = Children().begin(); it != Children().end(); ++it) {
        static_cast<GG::Control*>(*it)->Disable(b);
    }
}

void GalaxySetupPanel::SetFromLobbyData(const MultiplayerLobbyData& lobby_data)
{
    m_stars_spin->SetValue(lobby_data.m_size);
    m_galaxy_shapes_list->Select(lobby_data.m_shape);
    m_galaxy_ages_list->Select(lobby_data.m_age);
    m_starlane_freq_list->Select(lobby_data.m_starlane_freq - (ALLOW_NO_STARLANES ? 0 : 1));
    m_planet_density_list->Select(lobby_data.m_planet_density);
    m_specials_freq_list->Select(lobby_data.m_specials_freq);
}

void GalaxySetupPanel::GetLobbyData(MultiplayerLobbyData& lobby_data)
{
    lobby_data.m_size = Systems();
    lobby_data.m_shape = GetShape();
    lobby_data.m_age = GetAge();
    lobby_data.m_starlane_freq = GetStarlaneFrequency();
    lobby_data.m_planet_density = GetPlanetDensity();
    lobby_data.m_specials_freq = GetSpecialsFrequency();
}

void GalaxySetupPanel::Init()
{
    AttachSignalChildren();

    GG::Connect(m_stars_spin->ValueChangedSignal, &GalaxySetupPanel::SettingChanged, this);
    GG::Connect(m_galaxy_shapes_list->SelChangedSignal, &GalaxySetupPanel::SettingChanged, this);
    GG::Connect(m_galaxy_ages_list->SelChangedSignal, &GalaxySetupPanel::SettingChanged, this);
    GG::Connect(m_starlane_freq_list->SelChangedSignal, &GalaxySetupPanel::SettingChanged, this);
    GG::Connect(m_planet_density_list->SelChangedSignal, &GalaxySetupPanel::SettingChanged, this);
    GG::Connect(m_specials_freq_list->SelChangedSignal, &GalaxySetupPanel::SettingChanged, this);
    GG::Connect(m_galaxy_shapes_list->SelChangedSignal, &GalaxySetupPanel::ShapeChanged, this);

    // create and load textures
    m_textures.clear();
    m_textures.resize(GALAXY_SHAPES);
    m_textures[SPIRAL_2] = ClientUI::GetTexture(ClientUI::ArtDir() / "gp_spiral2.png");
    m_textures[SPIRAL_3] = ClientUI::GetTexture(ClientUI::ArtDir() / "gp_spiral3.png");
    m_textures[SPIRAL_4] = ClientUI::GetTexture(ClientUI::ArtDir() / "gp_spiral4.png");
    m_textures[CLUSTER] = ClientUI::GetTexture(ClientUI::ArtDir() / "gp_cluster.png");
    m_textures[ELLIPTICAL] = ClientUI::GetTexture(ClientUI::ArtDir() / "gp_elliptical.png");
    m_textures[IRREGULAR] = ClientUI::GetTexture(ClientUI::ArtDir() / "gp_irregular.png");
    m_textures[RING] = ClientUI::GetTexture(ClientUI::ArtDir() / "gp_ring.png");

    // fill droplists
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_2ARM")));
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_3ARM")));
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_4ARM")));
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_CLUSTER")));
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_ELLIPTICAL")));
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_IRREGULAR")));
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_RING")));

    m_galaxy_ages_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_YOUNG")));
    m_galaxy_ages_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_MATURE")));
    m_galaxy_ages_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_ANCIENT")));

    if (ALLOW_NO_STARLANES)
        m_starlane_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_NONE")));
    m_starlane_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_FEW")));
    m_starlane_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_SOME")));
    m_starlane_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_SEVERAL")));
    m_starlane_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_MANY")));
    m_starlane_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_VERY_MANY")));

    m_planet_density_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_LOW")));
    m_planet_density_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_MEDIUM")));
    m_planet_density_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_HIGH")));

    m_specials_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_NONE")));
    m_specials_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_RARE")));
    m_specials_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_UNCOMMON")));
    m_specials_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_COMMON")));

    // default settings
    m_stars_spin->SetValue(GetOptionsDB().Get<int>("GameSetup.stars"));
    m_galaxy_shapes_list->Select(GetOptionsDB().Get<Shape>("GameSetup.galaxy-shape"));
    m_galaxy_ages_list->Select(GetOptionsDB().Get<Age>("GameSetup.galaxy-age"));
    m_starlane_freq_list->Select(GetOptionsDB().Get<StarlaneFrequency>("GameSetup.starlane-frequency") - (ALLOW_NO_STARLANES ? 0 : 1));
    m_planet_density_list->Select(GetOptionsDB().Get<PlanetDensity>("GameSetup.planet-density"));
    m_specials_freq_list->Select(GetOptionsDB().Get<SpecialsFrequency>("GameSetup.specials-frequency"));
}

void GalaxySetupPanel::AttachSignalChildren()
{
    AttachChild(m_stars_spin);
    AttachChild(m_galaxy_shapes_list);
    AttachChild(m_galaxy_ages_list);
    AttachChild(m_starlane_freq_list);
    AttachChild(m_planet_density_list);
    AttachChild(m_specials_freq_list);
}

void GalaxySetupPanel::DetachSignalChildren()
{
    DetachChild(m_stars_spin);
    DetachChild(m_galaxy_shapes_list);
    DetachChild(m_galaxy_ages_list);
    DetachChild(m_starlane_freq_list);
    DetachChild(m_planet_density_list);
    DetachChild(m_specials_freq_list);
}

void GalaxySetupPanel::SettingChanged(int)
{
    SettingsChangedSignal();
}

void GalaxySetupPanel::ShapeChanged(int index)
{
    ImageChangedSignal(m_textures[index]);
}


////////////////////////////////////////////////
// GalaxySetupWnd
////////////////////////////////////////////////
GalaxySetupWnd::GalaxySetupWnd() : 
    CUIWnd(UserString("GSETUP_WINDOW_TITLE"), (HumanClientApp::GetApp()->AppWidth() - GAL_SETUP_WND_WD) / 2, 
           (HumanClientApp::GetApp()->AppHeight() - GAL_SETUP_WND_HT) / 2, GAL_SETUP_WND_WD, GAL_SETUP_WND_HT, 
           GG::CLICKABLE | GG::MODAL),
    m_ended_with_ok(false),
    m_galaxy_setup_panel(0),
    m_empire_name_label(0),
    m_empire_name_edit(0),
    m_empire_color_label(0),
    m_empire_color_selector(0),
    m_preview_image(0),
    m_ok(0),
    m_cancel(0)
{
    TempUISoundDisabler sound_disabler;

    m_galaxy_setup_panel = new GalaxySetupPanel(0, 4);

    boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());

    const int LABELS_WIDTH = (GalaxySetupPanel::DEFAULT_WIDTH - 5) / 2;
    m_empire_color_label = new GG::TextControl(CONTROL_MARGIN, m_galaxy_setup_panel->LowerRight().y + PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_EMPIRE_COLOR"), font, ClientUI::TextColor(), GG::TF_RIGHT);
    m_empire_color_selector = new EmpireColorSelector(ClientUI::Pts() + 4);
    m_empire_color_selector->MoveTo(GG::Pt(LABELS_WIDTH + 2 * CONTROL_MARGIN, m_galaxy_setup_panel->LowerRight().y + PANEL_CONTROL_SPACING + (PANEL_CONTROL_SPACING - m_empire_color_selector->Height()) / 2));
    m_empire_color_selector->Select(0);
    m_empire_name_label = new GG::TextControl(CONTROL_MARGIN, m_galaxy_setup_panel->LowerRight().y, LABELS_WIDTH, m_empire_color_selector->Height(), UserString("GSETUP_EMPIRE_NAME"), font, ClientUI::TextColor(), GG::TF_RIGHT);
    m_empire_name_edit = new CUIEdit(LABELS_WIDTH + 2 * CONTROL_MARGIN, m_galaxy_setup_panel->LowerRight().y,
                                     LABELS_WIDTH, "Human");
    m_empire_name_label->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_empire_name_label->Height()) / 2));
    m_empire_name_edit->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_empire_name_edit->Height()) / 2));

    m_preview_ul = GG::Pt(ClientWidth() - PREVIEW_SZ.x - 7, 7);

    // create a temporary texture and static graphic
    boost::shared_ptr<GG::Texture> temp_tex(new GG::Texture());
    m_preview_image =  new GG::StaticGraphic(m_preview_ul.x, m_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, temp_tex, GG::GR_FITGRAPHIC); // create a blank graphic

    m_ok = new CUIButton(10, m_empire_color_selector->LowerRight().y + 10, 75, UserString("OK"));
    m_cancel = new CUIButton(10 + m_ok->Size().x + 15, m_empire_color_selector->LowerRight().y + 10, 75, UserString("CANCEL"));

    Init();
}

const std::string& GalaxySetupWnd::EmpireName() const
{
    return m_empire_name_edit->WindowText();
}

GG::Clr GalaxySetupWnd::EmpireColor() const
{
    return m_empire_color_selector->CurrentColor();
}

void GalaxySetupWnd::Render()
{
    CUIWnd::Render();
    GG::FlatRectangle(ClientUpperLeft().x + m_preview_ul.x - 2, ClientUpperLeft().y + m_preview_ul.y - 2, ClientUpperLeft().x + m_preview_ul.x + PREVIEW_SZ.x + 2, 
                      ClientUpperLeft().y + m_preview_ul.y + PREVIEW_SZ.y + 2, GG::CLR_BLACK, ClientUI::WndInnerBorderColor(), 1);
}

void GalaxySetupWnd::KeyPress (GG::Key key, Uint32 key_mods)
{
    if (!m_ok->Disabled() && (key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER)) // Same behaviour as if "OK" was pressed
        OkClicked();
    else if (key == GG::GGK_ESCAPE) // Same behaviour as if "Cancel" was pressed
        CancelClicked();
}

void GalaxySetupWnd::Init()
{
    AttachSignalChildren();

    GG::Connect(m_galaxy_setup_panel->ImageChangedSignal, &GalaxySetupWnd::PreviewImageChanged, this);
    GG::Connect(m_empire_name_edit->EditedSignal, &GalaxySetupWnd::EmpireNameChanged, this);
    GG::Connect(m_ok->ClickedSignal, &GalaxySetupWnd::OkClicked, this);
    GG::Connect(m_cancel->ClickedSignal, &GalaxySetupWnd::CancelClicked, this);

    PreviewImageChanged(m_galaxy_setup_panel->PreviewImage());
}

void GalaxySetupWnd::AttachSignalChildren()
{
    AttachChild(m_galaxy_setup_panel);
    AttachChild(m_empire_name_label);
    AttachChild(m_empire_name_edit);
    AttachChild(m_empire_color_label);
    AttachChild(m_empire_color_selector);
    AttachChild(m_preview_image);
    AttachChild(m_ok);
    AttachChild(m_cancel);
}

void GalaxySetupWnd::DetachSignalChildren()
{
    DetachChild(m_galaxy_setup_panel);
    DetachChild(m_empire_name_label);
    DetachChild(m_empire_name_edit);
    DetachChild(m_empire_color_label);
    DetachChild(m_empire_color_selector);
    DetachChild(m_preview_image);
    DetachChild(m_ok);
    DetachChild(m_cancel);
}

void GalaxySetupWnd::PreviewImageChanged(boost::shared_ptr<GG::Texture> new_image)
{
    if (m_preview_image) {
        DeleteChild(m_preview_image);
        m_preview_image = 0;
    }
    m_preview_image = new GG::StaticGraphic(m_preview_ul.x, m_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, new_image, GG::GR_FITGRAPHIC);
    AttachChild(m_preview_image);
}

void GalaxySetupWnd::EmpireNameChanged(const std::string& name)
{
    m_ok->Disable(name.empty());
}

void GalaxySetupWnd::OkClicked()
{
    // record selected galaxy setup options as new defaults
    GetOptionsDB().Set("GameSetup.stars", m_galaxy_setup_panel->Systems());
    GetOptionsDB().Set("GameSetup.galaxy-shape", m_galaxy_setup_panel->GetShape());
    GetOptionsDB().Set("GameSetup.galaxy-age", m_galaxy_setup_panel->GetAge());
    GetOptionsDB().Set("GameSetup.starlane-frequency", m_galaxy_setup_panel->GetStarlaneFrequency());
    GetOptionsDB().Set("GameSetup.planet-density", m_galaxy_setup_panel->GetPlanetDensity());
    GetOptionsDB().Set("GameSetup.specials-frequency", m_galaxy_setup_panel->GetSpecialsFrequency());

    // Save the changes:
    boost::filesystem::ofstream ofs(GetConfigPath());
    GetOptionsDB().GetXML().WriteDoc(ofs);

    m_ended_with_ok = true;
    m_done = true;
}

void GalaxySetupWnd::CancelClicked()
{
    m_ended_with_ok = false;
    m_done = true;
}
