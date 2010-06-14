#include "GalaxySetupWnd.h"

#include "CUIControls.h"
#include "CUISpin.h"
#include "Sound.h"
#include "../universe/Universe.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"

#include <boost/filesystem/fstream.hpp>

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>


namespace {
    const GG::X CONTROL_MARGIN(5);
    const GG::Y CONTROL_HEIGHT(30);
    const GG::Y PANEL_CONTROL_SPACING(33);
    const GG::Y GAL_SETUP_PANEL_HT(PANEL_CONTROL_SPACING * 6);
    const GG::X GAL_SETUP_WND_WD(645);
    const GG::Y GAL_SETUP_WND_HT(29 + (PANEL_CONTROL_SPACING * 6) + GAL_SETUP_PANEL_HT);
    const GG::Pt PREVIEW_SZ(GG::X(248), GG::Y(186));
    const bool ALLOW_NO_STARLANES = false;
    const int MAX_AI_PLAYERS = 12;

    // persistant between-executions galaxy setup settings, mainly so I don't have to redo these settings to what I want every time I run FO to test something
    void AddOptions(OptionsDB& db) {
        db.Add("GameSetup.stars",               "OPTIONS_DB_GAMESETUP_STARS",                   60,                 RangedValidator<int>(10, 500));
        db.Add("GameSetup.galaxy-shape",        "OPTIONS_DB_GAMESETUP_GALAXY_SHAPE",            SPIRAL_3,           RangedValidator<Shape>(SPIRAL_2, RING));
        db.Add("GameSetup.galaxy-age",          "OPTIONS_DB_GAMESETUP_GALAXY_AGE",              AGE_MATURE,         RangedValidator<Age>(AGE_YOUNG, AGE_ANCIENT));
        db.Add("GameSetup.planet-density",      "OPTIONS_DB_GAMESETUP_PLANET_DENSITY",          PD_AVERAGE,         RangedValidator<PlanetDensity>(PD_LOW, PD_HIGH));
        db.Add("GameSetup.starlane-frequency",  "OPTIONS_DB_GAMESETUP_STARLANE_FREQUENCY",      LANES_MANY,         RangedValidator<StarlaneFrequency>(ALLOW_NO_STARLANES ? LANES_NONE : LANES_FEW, LANES_VERY_MANY));
        db.Add("GameSetup.specials-frequency",  "OPTIONS_DB_GAMESETUP_SPECIALS_FREQUENCY",      SPECIALS_COMMON,    RangedValidator<SpecialsFrequency>(SPECIALS_NONE, SPECIALS_COMMON));
        db.Add("GameSetup.empire-name",         "OPTIONS_DB_GAMESETUP_EMPIRE_NAME",             std::string(""),    Validator<std::string>());
        db.Add("GameSetup.player-name",         "OPTIONS_DB_GAMESETUP_PLAYER_NAME",             std::string(""),    Validator<std::string>());
        db.Add("GameSetup.empire-color",        "OPTIONS_DB_GAMESETUP_EMPIRE_COLOR",            0,                  RangedValidator<int>(0, 100));
        db.Add("GameSetup.starting-species",    "OPTIONS_DB_GAMESETUP_STARTING_SPECIES_NAME",   std::string(""),    Validator<std::string>());
        db.Add("GameSetup.ai-players",          "OPTIONS_DB_GAMESETUP_NUM_AI_PLAYERS",          3,                  RangedValidator<int>(0, MAX_AI_PLAYERS));
    }
    bool temp_bool = RegisterOptions(&AddOptions);
}

////////////////////////////////////////////////
// GalaxySetupPanel
////////////////////////////////////////////////
const GG::X GalaxySetupPanel::DEFAULT_WIDTH(305);

GalaxySetupPanel::GalaxySetupPanel(GG::X x, GG::Y y, GG::X w/* = DEFAULT_WIDTH*/) :
    GG::Control(x, y, w, GAL_SETUP_PANEL_HT, GG::Flags<GG::WndFlag>()),
    m_stars_spin(0),
    m_galaxy_shapes_list(0),
    m_galaxy_ages_list(0),
    m_starlane_freq_list(0),
    m_planet_density_list(0),
    m_specials_freq_list(0)
{
    Sound::TempUISoundDisabler sound_disabler;

    const GG::X LABELS_WIDTH = (w - CONTROL_MARGIN) / 2;
    const GG::X DROPLIST_WIDTH = LABELS_WIDTH;
    const GG::Y DROPLIST_HEIGHT(ClientUI::Pts() + 4);
    const GG::Y TEXT_ROW_HEIGHT = CUISimpleDropDownListRow::DEFAULT_ROW_HEIGHT;
    const GG::Y MAX_DROPLIST_DROP_HEIGHT = TEXT_ROW_HEIGHT * 5;
    const int TOTAL_LISTBOX_MARGIN = 4;
    int row = -1;

    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();

    // number of stars
    GG::TextControl* label = new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_STARS"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    label->SetBrowseText(UserString(GetOptionsDB().GetDescription("GameSetup.stars")));
    AttachChild(label);
    m_stars_spin = new CUISpin<int>(LABELS_WIDTH + 2 * CONTROL_MARGIN, row * PANEL_CONTROL_SPACING, GG::X(75), 100, 1, 10, 500, true);
    m_stars_spin->OffsetMove(GG::Pt(GG::X0, (PANEL_CONTROL_SPACING - m_stars_spin->Height()) / 2));

    // galaxy shape
    label = new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_SHAPE"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    label->SetBrowseText(UserString(GetOptionsDB().GetDescription("GameSetup.galaxy-shape")));
    AttachChild(label);
    GG::Y drop_height = std::min(TEXT_ROW_HEIGHT * GALAXY_SHAPES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_galaxy_shapes_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row * PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_galaxy_shapes_list->OffsetMove(GG::Pt(GG::X0, (PANEL_CONTROL_SPACING - m_galaxy_shapes_list->Height()) / 2));
    m_galaxy_shapes_list->SetStyle(GG::LIST_NOSORT);

    // galaxy age
    label = new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_AGE"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    label->SetBrowseText(UserString(GetOptionsDB().GetDescription("GameSetup.galaxy-age")));
    AttachChild(label);
    drop_height = std::min(TEXT_ROW_HEIGHT * NUM_UNIVERSE_AGES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_galaxy_ages_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row * PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_galaxy_ages_list->OffsetMove(GG::Pt(GG::X0, (PANEL_CONTROL_SPACING - m_galaxy_ages_list->Height()) / 2));
    m_galaxy_ages_list->SetStyle(GG::LIST_NOSORT);

    // starlane frequency
    label = new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_STARLANE_FREQ"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    label->SetBrowseText(UserString(GetOptionsDB().GetDescription("GameSetup.starlane-frequency")));
    AttachChild(label);
    drop_height = std::min(TEXT_ROW_HEIGHT * NUM_STARLANE_FREQENCIES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_starlane_freq_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row * PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_starlane_freq_list->OffsetMove(GG::Pt(GG::X0, (PANEL_CONTROL_SPACING - m_starlane_freq_list->Height()) / 2));
    m_starlane_freq_list->SetStyle(GG::LIST_NOSORT);

    // planet density
    label = new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_PLANET_DENSITY"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    label->SetBrowseText(UserString(GetOptionsDB().GetDescription("GameSetup.planet-density")));
    AttachChild(label);
    drop_height = std::min(TEXT_ROW_HEIGHT * NUM_UNIVERSE_PLANET_DENSITIES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_planet_density_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row* PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_planet_density_list->OffsetMove(GG::Pt(GG::X0, (PANEL_CONTROL_SPACING - m_planet_density_list->Height()) / 2));
    m_planet_density_list->SetStyle(GG::LIST_NOSORT);

    label = new GG::TextControl(CONTROL_MARGIN, ++row* PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_SPECIALS_FREQ"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    label->SetBrowseText(UserString(GetOptionsDB().GetDescription("GameSetup.specials-frequency")));
    AttachChild(label);
    drop_height = std::min(TEXT_ROW_HEIGHT * NUM_SPECIALS_FREQENCIES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_specials_freq_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row* PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_specials_freq_list->OffsetMove(GG::Pt(GG::X0, (PANEL_CONTROL_SPACING - m_specials_freq_list->Height()) / 2));
    m_specials_freq_list->SetStyle(GG::LIST_NOSORT);

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

void GalaxySetupPanel::Disable(bool b/* = true*/)
{
    for (std::list<GG::Wnd*>::const_iterator it = Children().begin(); it != Children().end(); ++it) {
        static_cast<GG::Control*>(*it)->Disable(b);
    }
}

void GalaxySetupPanel::SetFromSetupData(const GalaxySetupData& setup_data)
{
    m_stars_spin->SetValue(setup_data.m_size);
    m_galaxy_shapes_list->Select(setup_data.m_shape);
    ShapeChanged(m_galaxy_shapes_list->CurrentItem());
    m_galaxy_ages_list->Select(setup_data.m_age);
    m_starlane_freq_list->Select(setup_data.m_starlane_freq - (ALLOW_NO_STARLANES ? 0 : 1));
    m_planet_density_list->Select(setup_data.m_planet_density);
    m_specials_freq_list->Select(setup_data.m_specials_freq);
}

void GalaxySetupPanel::GetSetupData(GalaxySetupData& setup_data) const
{
    setup_data.m_size = Systems();
    setup_data.m_shape = GetShape();
    setup_data.m_age = GetAge();
    setup_data.m_starlane_freq = GetStarlaneFrequency();
    setup_data.m_planet_density = GetPlanetDensity();
    setup_data.m_specials_freq = GetSpecialsFrequency();
}

void GalaxySetupPanel::Init()
{
    AttachSignalChildren();

    GG::Connect(m_stars_spin->ValueChangedSignal, &GalaxySetupPanel::SettingChanged_, this);
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
    ShapeChanged(m_galaxy_shapes_list->CurrentItem());
    m_galaxy_ages_list->Select(GetOptionsDB().Get<Age>("GameSetup.galaxy-age"));
    m_starlane_freq_list->Select(GetOptionsDB().Get<StarlaneFrequency>("GameSetup.starlane-frequency") - (ALLOW_NO_STARLANES ? 0 : 1));
    m_planet_density_list->Select(GetOptionsDB().Get<PlanetDensity>("GameSetup.planet-density"));
    m_specials_freq_list->Select(GetOptionsDB().Get<SpecialsFrequency>("GameSetup.specials-frequency"));
    SettingsChangedSignal();
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

void GalaxySetupPanel::SettingChanged_(int)
{
    Sound::TempUISoundDisabler sound_disabler;
    SettingsChangedSignal();
}

void GalaxySetupPanel::SettingChanged(GG::DropDownList::iterator)
{ SettingChanged_(0); }

void GalaxySetupPanel::ShapeChanged(GG::DropDownList::iterator it)
{ ImageChangedSignal(m_textures[m_galaxy_shapes_list->IteratorToIndex(it)]); }


////////////////////////////////////////////////
// GalaxySetupWnd
////////////////////////////////////////////////
GalaxySetupWnd::GalaxySetupWnd() : 
    CUIWnd(UserString("GSETUP_WINDOW_TITLE"), (HumanClientApp::GetApp()->AppWidth() - GAL_SETUP_WND_WD) / 2, 
           (HumanClientApp::GetApp()->AppHeight() - GAL_SETUP_WND_HT) / 2, GAL_SETUP_WND_WD, GAL_SETUP_WND_HT, 
           GG::INTERACTIVE | GG::MODAL),
    m_ended_with_ok(false),
    m_galaxy_setup_panel(0),
    m_player_name_label(0),
    m_player_name_edit(0),
    m_empire_name_label(0),
    m_empire_name_edit(0),
    m_empire_color_label(0),
    m_empire_color_selector(0),
    m_starting_secies_selector(0),
    m_starting_species_label(0),
    m_number_ais_label(0),
    m_number_ais_spin(0),
    m_preview_image(0),
    m_ok(0),
    m_cancel(0)
{
    Sound::TempUISoundDisabler sound_disabler;
    GG::Y ypos;

    m_galaxy_setup_panel = new GalaxySetupPanel(GG::X0, GG::Y(4));

    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();

    const GG::X LABELS_WIDTH = (GalaxySetupPanel::DEFAULT_WIDTH - 5) / 2;

    ypos = m_galaxy_setup_panel->LowerRight().y;

    // player name
    m_player_name_label = new GG::TextControl(CONTROL_MARGIN, ypos, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_PLAYER_NAME"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_player_name_label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_player_name_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("GameSetup.player-name")));
    m_player_name_edit = new CUIEdit(LABELS_WIDTH + 2 * CONTROL_MARGIN, ypos, LABELS_WIDTH, GetOptionsDB().Get<std::string>("GameSetup.player-name"));
    m_player_name_label->OffsetMove(GG::Pt(GG::X0, (PANEL_CONTROL_SPACING - m_player_name_label->Height()) / 2));
    m_player_name_edit->OffsetMove(GG::Pt(GG::X0, (PANEL_CONTROL_SPACING - m_player_name_edit->Height()) / 2));

    // empire name
    ypos += PANEL_CONTROL_SPACING;
    m_empire_name_label = new GG::TextControl(CONTROL_MARGIN, ypos, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_EMPIRE_NAME"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_empire_name_label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_empire_name_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("GameSetup.empire-name")));
    m_empire_name_edit = new CUIEdit(LABELS_WIDTH + 2 * CONTROL_MARGIN, ypos, LABELS_WIDTH, GetOptionsDB().Get<std::string>("GameSetup.empire-name"));
    m_empire_name_label->OffsetMove(GG::Pt(GG::X0, (PANEL_CONTROL_SPACING - m_empire_name_label->Height()) / 2));
    m_empire_name_edit->OffsetMove(GG::Pt(GG::X0, (PANEL_CONTROL_SPACING - m_empire_name_edit->Height()) / 2));

    const GG::Y AUTO_CONTROL_HEIGHT = m_empire_name_edit->Height();

    // empire color
    ypos += PANEL_CONTROL_SPACING;
    m_empire_color_label = new GG::TextControl(CONTROL_MARGIN, ypos, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_EMPIRE_COLOR"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_empire_color_label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_empire_color_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("GameSetup.empire-color")));
    m_empire_color_selector = new EmpireColorSelector(AUTO_CONTROL_HEIGHT);
    m_empire_color_selector->MoveTo(GG::Pt(LABELS_WIDTH + 2 * CONTROL_MARGIN, ypos + (PANEL_CONTROL_SPACING - m_empire_color_selector->Height()) / 2));
    m_empire_color_selector->Select(GetOptionsDB().Get<int>("GameSetup.empire-color"));

    // starting species
    ypos += PANEL_CONTROL_SPACING;
    m_starting_species_label = new GG::TextControl(CONTROL_MARGIN, ypos, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_SPECIES"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_starting_species_label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_starting_species_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("GameSetup.starting-species")));
    m_starting_secies_selector = new SpeciesSelector(AUTO_CONTROL_HEIGHT);
    m_starting_secies_selector->MoveTo(GG::Pt(LABELS_WIDTH + 2 * CONTROL_MARGIN, ypos + (PANEL_CONTROL_SPACING - m_starting_secies_selector->Height()) / 2));
    m_starting_secies_selector->SelectSpecies(GetOptionsDB().Get<std::string>("GameSetup.starting-species"));

    // number of AIs
    ypos += PANEL_CONTROL_SPACING;
    m_number_ais_label = new GG::TextControl(CONTROL_MARGIN, ypos, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_NUMBER_AIS"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_number_ais_label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_number_ais_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("GameSetup.ai-players")));
    m_number_ais_spin = new CUISpin<int>(LABELS_WIDTH + 2 * CONTROL_MARGIN, ypos, GG::X(75), GetOptionsDB().Get<int>("GameSetup.ai-players"), 1, 0, MAX_AI_PLAYERS, true);
    m_number_ais_label->OffsetMove(GG::Pt(GG::X0, (PANEL_CONTROL_SPACING - m_number_ais_label->Height()) / 2));
    m_number_ais_spin->OffsetMove(GG::Pt(GG::X0, (PANEL_CONTROL_SPACING - m_number_ais_spin->Height()) / 2));

    m_preview_ul = GG::Pt(ClientWidth() - PREVIEW_SZ.x - 7, GG::Y(7));

    // create a temporary texture and static graphic
    boost::shared_ptr<GG::Texture> temp_tex(new GG::Texture());
    m_preview_image =  new GG::StaticGraphic(m_preview_ul.x, m_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, temp_tex, GG::GRAPHIC_FITGRAPHIC); // create a blank graphic

    ypos += PANEL_CONTROL_SPACING + 5;
    m_ok = new CUIButton(GG::X(10), ypos, GG::X(75), UserString("OK"));
    m_cancel = new CUIButton(10 + m_ok->Size().x + 15, ypos, GG::X(75), UserString("CANCEL"));

    Init();
}

std::string GalaxySetupWnd::EmpireName() const
{
    return m_empire_name_edit->Text();
}

GG::Clr GalaxySetupWnd::EmpireColor() const
{
    return m_empire_color_selector->CurrentColor();
}

int GalaxySetupWnd::NumberAIs() const
{
    return m_number_ais_spin->Value();
}

void GalaxySetupWnd::Render()
{
    CUIWnd::Render();
    GG::FlatRectangle(GG::Pt(ClientUpperLeft().x + m_preview_ul.x - 2,
                             ClientUpperLeft().y + m_preview_ul.y - 2),
                      GG::Pt(ClientUpperLeft().x + m_preview_ul.x + PREVIEW_SZ.x + 2,
                             ClientUpperLeft().y + m_preview_ul.y + PREVIEW_SZ.y + 2),
                      GG::CLR_BLACK, ClientUI::WndInnerBorderColor(), 1);
}

void GalaxySetupWnd::KeyPress (GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
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
    GG::Connect(m_player_name_edit->EditedSignal, &GalaxySetupWnd::PlayerNameChanged, this);
    GG::Connect(m_empire_name_edit->EditedSignal, &GalaxySetupWnd::EmpireNameChanged, this);
    GG::Connect(m_ok->ClickedSignal, &GalaxySetupWnd::OkClicked, this);
    GG::Connect(m_cancel->ClickedSignal, &GalaxySetupWnd::CancelClicked, this);

    PreviewImageChanged(m_galaxy_setup_panel->PreviewImage());
}

void GalaxySetupWnd::AttachSignalChildren()
{
    AttachChild(m_galaxy_setup_panel);
    AttachChild(m_player_name_label);
    AttachChild(m_player_name_edit);
    AttachChild(m_empire_name_label);
    AttachChild(m_empire_name_edit);
    AttachChild(m_empire_color_label);
    AttachChild(m_empire_color_selector);
    AttachChild(m_starting_species_label);
    AttachChild(m_starting_secies_selector);
    AttachChild(m_number_ais_label);
    AttachChild(m_number_ais_spin);
    AttachChild(m_preview_image);
    AttachChild(m_ok);
    AttachChild(m_cancel);
}

void GalaxySetupWnd::DetachSignalChildren()
{
    DetachChild(m_galaxy_setup_panel);
    DetachChild(m_player_name_label);
    DetachChild(m_player_name_edit);
    DetachChild(m_empire_name_label);
    DetachChild(m_empire_name_edit);
    DetachChild(m_empire_color_label);
    DetachChild(m_empire_color_selector);
    DetachChild(m_number_ais_label);
    DetachChild(m_number_ais_spin);
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
    m_preview_image = new GG::StaticGraphic(m_preview_ul.x, m_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, new_image, GG::GRAPHIC_FITGRAPHIC);
    AttachChild(m_preview_image);
}

void GalaxySetupWnd::EmpireNameChanged(const std::string& name)
{
    m_ok->Disable(name.empty());
}

void GalaxySetupWnd::PlayerNameChanged(const std::string& name)
{
    m_ok->Disable(name.empty());
}

void GalaxySetupWnd::OkClicked()
{
    // record selected galaxy setup options as new defaults
    GetOptionsDB().Set("GameSetup.stars",               m_galaxy_setup_panel->Systems());
    GetOptionsDB().Set("GameSetup.galaxy-shape",        m_galaxy_setup_panel->GetShape());
    GetOptionsDB().Set("GameSetup.galaxy-age",          m_galaxy_setup_panel->GetAge());
    GetOptionsDB().Set("GameSetup.starlane-frequency",  m_galaxy_setup_panel->GetStarlaneFrequency());
    GetOptionsDB().Set("GameSetup.planet-density",      m_galaxy_setup_panel->GetPlanetDensity());
    GetOptionsDB().Set("GameSetup.specials-frequency",  m_galaxy_setup_panel->GetSpecialsFrequency());
    GetOptionsDB().Set("GameSetup.player-name",         m_player_name_edit->Text());
    GetOptionsDB().Set("GameSetup.empire-name",         EmpireName());
    GetOptionsDB().Set("GameSetup.empire-color",        static_cast<int>(m_empire_color_selector->CurrentItemIndex()));
    GetOptionsDB().Set("GameSetup.starting-species",    m_starting_secies_selector->CurrentSpeciesName());
    GetOptionsDB().Set("GameSetup.ai-players",          m_number_ais_spin->Value());

    // Save the changes:
    {
        boost::filesystem::ofstream ofs(GetConfigPath());
        if (ofs) {
            GetOptionsDB().GetXML().WriteDoc(ofs);
        } else {
            std::cerr << UserString("UNABLE_TO_WRITE_CONFIG_XML") << std::endl;
            std::cerr << GetConfigPath().file_string() << std::endl;
            Logger().errorStream() << UserString("UNABLE_TO_WRITE_CONFIG_XML");
            Logger().errorStream() << GetConfigPath().file_string();
        }
    }


    m_ended_with_ok = true;
    m_done = true;
}

void GalaxySetupWnd::CancelClicked()
{
    m_ended_with_ok = false;
    m_done = true;
}
