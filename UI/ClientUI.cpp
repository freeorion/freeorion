#include "ClientUI.h"

#include "CUIControls.h"
#include "FleetWnd.h"
#include "IntroScreen.h"
#include "MapWnd.h"
#include "SidePanel.h"
#include "../util/AppInterface.h"
#include "../util/Random.h"
#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Ship.h"
#include "../universe/Tech.h"
#include "../universe/Special.h"
#include "../client/human/HumanClientApp.h"
#include "../util/Directories.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"

#include <GG/GUI.h>
#include <GG/Clr.h>
#include <GG/DrawUtil.h>
#include <GG/dialogs/ThreeButtonDlg.h>

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/cerrno.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/spirit.hpp>

#include <string>

const Tech* GetTech(const std::string& name);

namespace fs = boost::filesystem;

// static members
fs::path ClientUI::ArtDir()                    { return GetSettingsDir() / "data" / "art"; }
fs::path ClientUI::SoundDir()                  { return GetSettingsDir() / "data" / "sound"; }

std::string ClientUI::Font()                   { return (GetGlobalDir() / GetOptionsDB().Get<std::string>("UI.font")).native_file_string(); }
std::string ClientUI::FontBold()               { return (GetGlobalDir() / GetOptionsDB().Get<std::string>("UI.font-bold")).native_file_string(); }
std::string ClientUI::FontItalic()             { return (GetGlobalDir() / GetOptionsDB().Get<std::string>("UI.font-italic")).native_file_string(); }
std::string ClientUI::FontBoldItalic()         { return (GetGlobalDir() / GetOptionsDB().Get<std::string>("UI.font-bold-italic")).native_file_string(); }
std::string ClientUI::TitleFont()              { return (GetGlobalDir() / GetOptionsDB().Get<std::string>("UI.title-font")).native_file_string(); }

int         ClientUI::Pts()                    { return GetOptionsDB().Get<int>("UI.font-size"); }
int         ClientUI::TitlePts()               { return GetOptionsDB().Get<int>("UI.title-font-size"); }

GG::Clr     ClientUI::TextColor()              { return GetOptionsDB().Get<StreamableColor>("UI.text-color").ToClr(); }

// windows
GG::Clr     ClientUI::WndColor()               { return GetOptionsDB().Get<StreamableColor>("UI.wnd-color").ToClr(); }
GG::Clr     ClientUI::WndBorderColor()         { return GetOptionsDB().Get<StreamableColor>("UI.wnd-border-color").ToClr(); }
GG::Clr     ClientUI::WndOuterBorderColor()    { return GetOptionsDB().Get<StreamableColor>("UI.wnd-outer-border-color").ToClr(); }
GG::Clr     ClientUI::WndInnerBorderColor()    { return GetOptionsDB().Get<StreamableColor>("UI.wnd-inner-border-color").ToClr(); }

// controls
GG::Clr     ClientUI::CtrlColor()              { return GetOptionsDB().Get<StreamableColor>("UI.ctrl-color").ToClr(); }
GG::Clr     ClientUI::CtrlBorderColor()        { return GetOptionsDB().Get<StreamableColor>("UI.ctrl-border-color").ToClr(); }

GG::Clr     ClientUI::ButtonColor()            { return GetOptionsDB().Get<StreamableColor>("UI.wnd-border-color").ToClr(); }

GG::Clr     ClientUI::StateButtonColor()       { return GetOptionsDB().Get<StreamableColor>("UI.state-button-color").ToClr(); }

GG::Clr     ClientUI::ScrollTabColor()         { return GetOptionsDB().Get<StreamableColor>("UI.scroll-tab-color").ToClr(); }
int         ClientUI::ScrollWidth()            { return GetOptionsDB().Get<int>("UI.scroll-width"); }

GG::Clr     ClientUI::DropDownListIntColor()   { return GetOptionsDB().Get<StreamableColor>("UI.dropdownlist-interior-color").ToClr(); }
GG::Clr     ClientUI::DropDownListArrowColor() { return GetOptionsDB().Get<StreamableColor>("UI.dropdownlist-arrow-color").ToClr(); }

GG::Clr     ClientUI::EditHiliteColor()        { return GetOptionsDB().Get<StreamableColor>("UI.edit-hilite").ToClr(); }
GG::Clr     ClientUI::EditIntColor()           { return GetOptionsDB().Get<StreamableColor>("UI.edit-interior").ToClr(); }
GG::Clr     ClientUI::MultieditIntColor()      { return GetOptionsDB().Get<StreamableColor>("UI.multiedit-interior").ToClr(); }

GG::Clr     ClientUI::StatIncrColor()          { return GetOptionsDB().Get<StreamableColor>("UI.stat-increase-color").ToClr(); }
GG::Clr     ClientUI::StatDecrColor()          { return GetOptionsDB().Get<StreamableColor>("UI.stat-decrease-color").ToClr(); }

int         ClientUI::SystemIconSize()                  { return GetOptionsDB().Get<int>("UI.system-icon-size"); }
double      ClientUI::FleetButtonSize()                 { return GetOptionsDB().Get<double>("UI.fleet-button-size"); }
double      ClientUI::SystemSelectionIndicatorSize()    { return GetOptionsDB().Get<double>("UI.system-selection-indicator-size"); }

// SidePanel
GG::Clr     ClientUI::SidePanelColor()         { return GetOptionsDB().Get<StreamableColor>("UI.sidepanel-color").ToClr(); }

// content texture getters
boost::shared_ptr<GG::Texture> ClientUI::ShipIcon(int design_id)
{
    const ShipDesign* design = GetShipDesign(design_id);
    boost::shared_ptr<GG::Texture> texture = ClientUI::GetTexture(ArtDir() / design->Graphic(), true);
    if (texture) return texture;
    return ClientUI::GetTexture(ArtDir() / "icons" / "Scout.png", true);
}

boost::shared_ptr<GG::Texture> ClientUI::BuildingTexture(const std::string& building_type_name)
{
    const BuildingType* building_type = GetBuildingType(building_type_name);
    const std::string graphic_name = building_type->Graphic();
    if (graphic_name.empty())
        return ClientUI::GetTexture(ArtDir() / "building_icons" / "Generic_Building.png", true);
    return ClientUI::GetTexture(ArtDir() / graphic_name, true);
}

boost::shared_ptr<GG::Texture> ClientUI::CategoryIcon(const std::string& category_name)
{
    std::string icon_filename;
    if (category_name == "CONSTRUCTION_CATEGORY")
        icon_filename = "construction.png";
    if (category_name == "ECONOMICS_CATEGORY")
        icon_filename = "economics.png";
    if (category_name == "GROWTH_CATEGORY")
        icon_filename = "growth.png";
    if (category_name == "LEARNING_CATEGORY")
        icon_filename = "learning.png";
    if (category_name == "PRODUCTION_CATEGORY")
        icon_filename = "production.png";
    return ClientUI::GetTexture(ArtDir() / "tech_icons" / "categories" / icon_filename, true);
}

boost::shared_ptr<GG::Texture> ClientUI::TechTexture(const std::string& tech_name)
{
    const Tech* tech = GetTechManager().GetTech(tech_name);
    std::string texture_name = tech->Graphic();
    if (texture_name.empty()) {
        return CategoryIcon(tech->Category());
    }
    return ClientUI::GetTexture(ArtDir() / texture_name, true);
}

boost::shared_ptr<GG::Texture> ClientUI::SpecialTexture(const std::string& special_name)
{
    const Special* special = GetSpecial(special_name);
    std::string texture_name = special->Graphic();
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "special_icons" / "Generic_Special.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name);
}

boost::shared_ptr<GG::Texture> ClientUI::MeterIcon(MeterType meter_type)
{
    std::string icon_filename;
    switch (meter_type) {
    case METER_POPULATION:
        icon_filename = "pop.png";
        break;
    case METER_FARMING:
        icon_filename = "farming.png";
        break;
    case METER_INDUSTRY:
        icon_filename = "industry.png";
        break;
    case METER_RESEARCH:
        icon_filename = "research.png";
        break;
    case METER_TRADE:
        icon_filename = "trade.png";
        break;
    case METER_MINING:
        icon_filename = "mining.png";
        break;
    case METER_CONSTRUCTION:
        icon_filename = "construction.png";
        break;
    case METER_HEALTH:
        icon_filename = "health.png";
        break;
    default:
        break;
    }
    return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / icon_filename, true);
}


// tech screen
GG::Clr     ClientUI::KnownTechFillColor()                   { return GetOptionsDB().Get<StreamableColor>("UI.known-tech").ToClr(); }
GG::Clr     ClientUI::KnownTechTextAndBorderColor()          { return GetOptionsDB().Get<StreamableColor>("UI.known-tech-border").ToClr(); }
GG::Clr     ClientUI::ResearchableTechFillColor()            { return GetOptionsDB().Get<StreamableColor>("UI.researchable-tech").ToClr(); }
GG::Clr     ClientUI::ResearchableTechTextAndBorderColor()   { return GetOptionsDB().Get<StreamableColor>("UI.researchable-tech-border").ToClr(); }
GG::Clr     ClientUI::UnresearchableTechFillColor()          { return GetOptionsDB().Get<StreamableColor>("UI.unresearchable-tech").ToClr(); }
GG::Clr     ClientUI::UnresearchableTechTextAndBorderColor() { return GetOptionsDB().Get<StreamableColor>("UI.unresearchable-tech-border").ToClr(); }
GG::Clr     ClientUI::TechWndProgressBarBackground()         { return GetOptionsDB().Get<StreamableColor>("UI.tech-progress-background").ToClr(); }
GG::Clr     ClientUI::TechWndProgressBar()                   { return GetOptionsDB().Get<StreamableColor>("UI.tech-progress").ToClr(); }

GG::Clr     ClientUI::CategoryColor(const std::string& category_name)
{
    const std::vector<std::string>& tech_categories = GetTechManager().CategoryNames();
    std::vector<std::string>::const_iterator it = std::find(tech_categories.begin(), tech_categories.end(), category_name);
    if (it != tech_categories.end()) {
        int category_index = std::distance(tech_categories.begin(), it) + 1;
        return GetOptionsDB().Get<StreamableColor>("UI.tech-category-" + boost::lexical_cast<std::string>(category_index)).ToClr();
    }
    return GG::Clr();
}

std::map<StarType, std::string>& ClientUI::StarTypeFilePrefixes()
{
    static std::map<StarType, std::string> prefixes;
    prefixes[STAR_BLUE] = "blue";
    prefixes[STAR_WHITE] = "white";
    prefixes[STAR_YELLOW] = "yellow";
    prefixes[STAR_ORANGE] = "orange";
    prefixes[STAR_RED] = "red";
    prefixes[STAR_NEUTRON] = "neutron";
    prefixes[STAR_BLACK] = "blackhole";
    return prefixes;
}

std::map<StarType, std::string>& ClientUI::HaloStarTypeFilePrefixes()
{
    static std::map<StarType, std::string> prefixes;
    prefixes[STAR_BLUE] = "halo_blue";
    prefixes[STAR_WHITE] = "halo_white";
    prefixes[STAR_YELLOW] = "halo_yellow";
    prefixes[STAR_ORANGE] = "halo_orange";
    prefixes[STAR_RED] = "halo_red";
    prefixes[STAR_NEUTRON] = "halo_neutron";
    prefixes[STAR_BLACK] = "halo_blackhole";
    return prefixes;
}

// private static members
ClientUI* ClientUI::s_the_UI = 0;

namespace {
    // an internal LUT of string IDs for each SitRep type
    // It's in this module becaue SitReps know nothing about how they
    // should be rendered - this is up to the client UI
    const char* g_string_id_lut[ SitRepEntry::NUM_SITREP_TYPES ] =
    {
        "SITREP_BASE_BUILT",
        "SITREP_SHIP_BUILT",
        "SITREP_BUILDING_BUILT",
        "SITREP_TECH_RESEARCHED",
        "SITREP_COMBAT_SYSTEM_WON",
        "SITREP_COMBAT_SYSTEM_LOST",
        "SITREP_COMBAT_SYSTEM_NO_VICTOR",
        "SITREP_PLANET_LOST_STARVED_TO_DEATH",
        "SITREP_PLANET_COLONIZED",
        "SITREP_FLEET_ARRIVED_AT_DESTINATION"
    };
    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add("app-width", "OPTIONS_DB_APP_WIDTH", 1024, RangedValidator<int>(800, 2048));
        db.Add("app-height", "OPTIONS_DB_APP_HEIGHT", 768, RangedValidator<int>(600, 1536));
        db.Add('c', "color-depth", "OPTIONS_DB_COLOR_DEPTH", 32, RangedStepValidator<int>(8, 16, 32));
        db.Add("show-fps", "OPTIONS_DB_SHOW_FPS", false);
        db.Add("limit-fps", "OPTIONS_DB_LIMIT_FPS", true);
        db.Add("max-fps", "OPTIONS_DB_MAX_FPS", 60.0, RangedValidator<double>(10.0, 200.0));

        // sound
        db.Add("UI.sound.enabled", "OPTIONS_DB_UI_SOUND_ENABLED", true, Validator<bool>());
        db.Add("UI.sound.volume", "OPTIONS_DB_UI_SOUND_VOLUME", 255, RangedValidator<int>(0, 255));
        db.Add<std::string>("UI.sound.button-rollover", "OPTIONS_DB_UI_SOUND_BUTTON_ROLLOVER", "button_rollover.wav");
        db.Add<std::string>("UI.sound.button-click", "OPTIONS_DB_UI_SOUND_BUTTON_CLICK", "button_click.wav");
        db.Add<std::string>("UI.sound.turn-button-click", "OPTIONS_DB_UI_SOUND_TURN_BUTTON_CLICK", "turn_button_click.wav");
        db.Add<std::string>("UI.sound.list-select", "OPTIONS_DB_UI_SOUND_LIST_SELECT", "list_select.wav");
        db.Add<std::string>("UI.sound.item-drop", "OPTIONS_DB_UI_SOUND_ITEM_DROP", "item_drop.wav");
        db.Add<std::string>("UI.sound.list-pulldown", "OPTIONS_DB_UI_SOUND_LIST_PULLDOWN", "list_pulldown.wav");
        db.Add<std::string>("UI.sound.text-typing", "OPTIONS_DB_UI_SOUND_TEXT_TYPING", "text_typing.wav");
        db.Add<std::string>("UI.sound.window-maximize", "OPTIONS_DB_UI_SOUND_WINDOW_MAXIMIZE", "window_maximize.wav");
        db.Add<std::string>("UI.sound.window-minimize", "OPTIONS_DB_UI_SOUND_WINDOW_MINIMIZE", "window_minimize.wav");
        db.Add<std::string>("UI.sound.window-close", "OPTIONS_DB_UI_SOUND_WINDOW_CLOSE", "window_close.wav");
        db.Add<std::string>("UI.sound.alert", "OPTIONS_DB_UI_SOUND_ALERT", "alert.wav");
        db.Add<std::string>("UI.sound.planet-button-click", "OPTIONS_DB_UI_SOUND_PLANET_BUTTON_CLICK", "button_click.wav");
        db.Add<std::string>("UI.sound.fleet-button-rollover", "OPTIONS_DB_UI_SOUND_FLEET_BUTTON_ROLLOVER", "fleet_button_rollover.wav");
        db.Add<std::string>("UI.sound.fleet-button-click", "OPTIONS_DB_UI_SOUND_FLEET_BUTTON_CLICK", "fleet_button_click.wav");
        db.Add<std::string>("UI.sound.sidepanel-open", "OPTIONS_DB_UI_SOUND_SIDEPANEL_OPEN", "sidepanel_open.wav");
        db.Add<std::string>("UI.sound.farming-focus", "OPTIONS_DB_UI_SOUND_FARMING_FOCUS", "farm_select.wav");
        db.Add<std::string>("UI.sound.industry-focus", "OPTIONS_DB_UI_SOUND_INDUSTRY_FOCUS", "industry_select.wav");
        db.Add<std::string>("UI.sound.research-focus", "OPTIONS_DB_UI_SOUND_RESEARCH_FOCUS", "research_select.wav");
        db.Add<std::string>("UI.sound.mining-focus", "OPTIONS_DB_UI_SOUND_MINING_FOCUS", "mining_select.wav");
        db.Add<std::string>("UI.sound.trade-focus", "OPTIONS_DB_UI_SOUND_TRADE_FOCUS", "trade_select.wav");
        db.Add<std::string>("UI.sound.balanced-focus", "OPTIONS_DB_UI_SOUND_BALANCED_FOCUS", "balanced_select.wav");

        // fonts
        db.Add<std::string>("UI.font", "OPTIONS_DB_UI_FONT", "DejaVuSans.ttf");
        db.Add<std::string>("UI.font-bold", "OPTIONS_DB_UI_FONT_BOLD", "DejaVuSans-Bold.ttf");
        db.Add<std::string>("UI.font-italic", "OPTIONS_DB_UI_FONT_ITALIC", "DejaVuSans-Oblique.ttf");
        db.Add<std::string>("UI.font-bold-italic", "OPTIONS_DB_UI_FONT_BOLD_ITALIC", "DejaVuSans-BoldOblique.ttf");
        db.Add("UI.font-size", "OPTIONS_DB_UI_FONT_SIZE", 12, RangedValidator<int>(4, 40));
        db.Add<std::string>("UI.title-font", "OPTIONS_DB_UI_TITLE_FONT", "DejaVuSans.ttf");
        db.Add("UI.title-font-size", "OPTIONS_DB_UI_TITLE_FONT_SIZE", 12, RangedValidator<int>(4, 40));

        // colors
        db.Add("UI.wnd-color", "OPTIONS_DB_UI_WND_COLOR", StreamableColor(GG::Clr(0, 0, 0, 210)), Validator<StreamableColor>());
        db.Add("UI.text-color", "OPTIONS_DB_UI_TEXT_COLOR", StreamableColor(GG::Clr(255, 255, 255, 255)), Validator<StreamableColor>());
        db.Add("UI.ctrl-color", "OPTIONS_DB_UI_CTRL_COLOR", StreamableColor(GG::Clr(30, 30, 30, 255)), Validator<StreamableColor>());
        db.Add("UI.ctrl-border-color", "OPTIONS_DB_UI_CTRL_BORDER_COLOR", StreamableColor(GG::Clr(124, 124, 124, 255)), Validator<StreamableColor>());
        db.Add("UI.button-color", "OPTIONS_DB_UI_BUTTON_COLOR", StreamableColor(GG::Clr(0, 0, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.state-button-color", "OPTIONS_DB_UI_STATE_BUTTON_COLOR", StreamableColor(GG::Clr(0, 127, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.scroll-tab-color", "OPTIONS_DB_UI_SCROLL_TAB_COLOR", StreamableColor(GG::Clr(60, 60, 60, 255)), Validator<StreamableColor>());
        db.Add("UI.dropdownlist-interior-color", "OPTIONS_DB_UI_DROPDOWNLIST_INTERIOR_COLOR", StreamableColor(GG::Clr(0, 0, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.dropdownlist-arrow-color", "OPTIONS_DB_UI_DROPDOWNLIST_ARROW_COLOR", StreamableColor(GG::Clr(130, 130, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.edit-hilite", "OPTIONS_DB_UI_EDIT_HILITE", StreamableColor(GG::Clr(43, 81, 102, 255)), Validator<StreamableColor>());
        db.Add("UI.edit-interior", "OPTIONS_DB_UI_EDIT_INTERIOR", StreamableColor(GG::Clr(0, 0, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.multiedit-interior", "OPTIONS_DB_UI_MULTIEDIT_INTERIOR", StreamableColor(GG::Clr(0, 0, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.stat-increase-color", "OPTIONS_DB_UI_STAT_INCREASE_COLOR", StreamableColor(GG::Clr(0, 255, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.stat-decrease-color", "OPTIONS_DB_UI_STAT_DECREASE_COLOR", StreamableColor(GG::Clr(255, 0, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.sidepanel-color", "OPTIONS_DB_UI_SIDEPANEL_COLOR", StreamableColor(GG::Clr(0, 0, 0, 220)), Validator<StreamableColor>());
        db.Add("UI.wnd-outer-border-color", "OPTIONS_DB_UI_WND_OUTER_BORDER_COLOR", StreamableColor(GG::Clr(64, 64, 64, 255)), Validator<StreamableColor>());
        db.Add("UI.wnd-border-color", "OPTIONS_DB_UI_WND_BORDER_COLOR", StreamableColor(GG::Clr(0, 0, 0, 255)), Validator<StreamableColor>());
        db.Add("UI.wnd-inner-border-color", "OPTIONS_DB_UI_WND_INNER_BORDER_COLOR", StreamableColor(GG::Clr(255, 255, 255, 255)), Validator<StreamableColor>());
        db.Add("UI.known-tech", "OPTIONS_DB_UI_KNOWN_TECH", StreamableColor(GG::Clr(72, 72, 72, 255)), Validator<StreamableColor>());
        db.Add("UI.known-tech-border", "OPTIONS_DB_UI_KNOWN_TECH_BORDER", StreamableColor(GG::Clr(164, 164, 164, 255)), Validator<StreamableColor>());
        db.Add("UI.researchable-tech", "OPTIONS_DB_UI_RESEARCHABLE_TECH", StreamableColor(GG::Clr(48, 48, 48, 255)), Validator<StreamableColor>());
        db.Add("UI.researchable-tech-border", "OPTIONS_DB_UI_RESEARCHABLE_TECH_BORDER", StreamableColor(GG::Clr(164, 164, 164, 255)), Validator<StreamableColor>());
        db.Add("UI.unresearchable-tech", "OPTIONS_DB_UI_UNRESEARCHABLE_TECH", StreamableColor(GG::Clr(30, 30, 30, 255)), Validator<StreamableColor>());
        db.Add("UI.unresearchable-tech-border", "OPTIONS_DB_UI_UNRESEARCHABLE_TECH_BORDER", StreamableColor(GG::Clr(86, 86, 86, 255)), Validator<StreamableColor>());
        db.Add("UI.tech-progress-background", "OPTIONS_DB_UI_TECH_PROGRESS_BACKGROUND", StreamableColor(GG::Clr(72, 72, 72, 255)), Validator<StreamableColor>());
        db.Add("UI.tech-progress", "OPTIONS_DB_UI_TECH_PROGRESS", StreamableColor(GG::Clr(40, 40, 40, 255)), Validator<StreamableColor>());

        // misc
        db.Add("UI.scroll-width", "OPTIONS_DB_UI_SCROLL_WIDTH", 14, RangedValidator<int>(8, 30));
        db.Add("UI.system-icon-size", "OPTIONS_DB_UI_SYSTEM_ICON_SIZE", 14, RangedValidator<int>(8, 50));
        db.Add("UI.fleet-button-size", "OPTIONS_DB_UI_FLEET_BUTTON_SIZE", 1.0, RangedValidator<double>(0.2, 2));
        db.Add("UI.system-selection-indicator-size", "OPTIONS_DB_UI_SYSTEM_SELECTION_INDICATOR_SIZE", 2.0, RangedValidator<double>(0.5, 5));
        
        // tech category colors
        const GG::Clr CLR_LEARNING_CATEGORY     (93,  155, 246, 255);
        const GG::Clr CLR_GROWTH_CATEGORY       (116, 225, 107, 255);
        const GG::Clr CLR_PRODUCTION_CATEGORY   (240, 106, 106, 255);
        const GG::Clr CLR_CONSTRUCTION_CATEGORY (241, 233, 87,  255);
        const GG::Clr CLR_ECONOMICS_CATEGORY    (255, 112, 247, 255);
        const GG::Clr CLR_CATEGORY6             (85,  170, 255, 255);
        const GG::Clr CLR_CATEGORY7             (170, 255, 85,  255);
        const GG::Clr CLR_CATEGORY8             (85,  255, 170, 255);
        const GG::Clr CLR_CATEGORY9             (255, 170, 85,  255);
        const GG::Clr CLR_CATEGORY10            (170, 170, 170, 255);

        db.Add("UI.tech-category-1", "OPTIONS_DB_UI_TECH_CATEGORY_1",   StreamableColor(CLR_LEARNING_CATEGORY),     Validator<StreamableColor>());
        db.Add("UI.tech-category-2", "OPTIONS_DB_UI_TECH_CATEGORY_2",   StreamableColor(CLR_GROWTH_CATEGORY),       Validator<StreamableColor>());
        db.Add("UI.tech-category-3", "OPTIONS_DB_UI_TECH_CATEGORY_3",   StreamableColor(CLR_PRODUCTION_CATEGORY),   Validator<StreamableColor>());
        db.Add("UI.tech-category-4", "OPTIONS_DB_UI_TECH_CATEGORY_4",   StreamableColor(CLR_CONSTRUCTION_CATEGORY), Validator<StreamableColor>());
        db.Add("UI.tech-category-5", "OPTIONS_DB_UI_TECH_CATEGORY_5",   StreamableColor(CLR_ECONOMICS_CATEGORY),    Validator<StreamableColor>());
        db.Add("UI.tech-category-6", "OPTIONS_DB_UI_TECH_CATEGORY_6",   StreamableColor(CLR_CATEGORY6),             Validator<StreamableColor>());
        db.Add("UI.tech-category-7", "OPTIONS_DB_UI_TECH_CATEGORY_7",   StreamableColor(CLR_CATEGORY7),             Validator<StreamableColor>());
        db.Add("UI.tech-category-8", "OPTIONS_DB_UI_TECH_CATEGORY_8",   StreamableColor(CLR_CATEGORY8),             Validator<StreamableColor>());
        db.Add("UI.tech-category-9", "OPTIONS_DB_UI_TECH_CATEGORY_9",   StreamableColor(CLR_CATEGORY9),             Validator<StreamableColor>());
        db.Add("UI.tech-category-10", "OPTIONS_DB_UI_TECH_CATEGORY_10", StreamableColor(CLR_CATEGORY10),            Validator<StreamableColor>());

        // UI behavior
        db.Add("UI.tooltip-delay", "OPTIONS_DB_UI_TOOLTIP_DELAY", 1000, RangedValidator<int>(0, 3000));
        db.Add("UI.multiple-fleet-windows", "OPTIONS_DB_UI_MULTIPLE_FLEET_WINDOWS", false);
        db.Add("UI.fleet-autoselect", "OPTIONS_DB_UI_FLEET_AUTOSELECT", true);
        db.Add("UI.window-quickclose", "OPTIONS_DB_UI_WINDOW_QUICKCLOSE", true);
    }
    bool temp_bool = RegisterOptions(&AddOptions);
}


////////////////////////////////////////////////
// ClientUI
////////////////////////////////////////////////
//Init and Cleanup//////////////////////////////////////
ClientUI::ClientUI() :
    m_map_wnd(new MapWnd)
{
    s_the_UI = this;
    GG::GUI::GetGUI()->Register(m_map_wnd);
    m_map_wnd->Hide();
}

ClientUI::~ClientUI() 
{
    delete m_map_wnd;
    s_the_UI = 0;
}

MapWnd* ClientUI::GetMapWnd()
{ return m_map_wnd; }

void ClientUI::GetSaveGameUIData(SaveGameUIData& data) const
{ m_map_wnd->GetSaveGameUIData(data); }

bool ClientUI::ZoomToPlanet(int id)
{
    // this just zooms to the appropriate system, until we create a planet window of some kind
    if (Planet* planet = GetUniverse().Object<Planet>(id)) {
        ZoomToSystem(planet->GetSystem());
        return true;
    }
    return false;
}

bool ClientUI::ZoomToSystem(int id)
{
    if (System* system = GetUniverse().Object<System>(id)) {
        ZoomToSystem(system);
        return true;
    }
    return false;
}

bool ClientUI::ZoomToFleet(int id)
{
    if (Fleet* fleet = GetUniverse().Object<Fleet>(id)) {
        ZoomToFleet(fleet);
        return true;
    }
    return false;
}

bool ClientUI::ZoomToShip(int id)
{
    // this just zooms to the appropriate fleet window, until we create a ship window of some kind
    if (Ship* ship = GetUniverse().Object<Ship>(id)) {
        ZoomToFleet(ship->GetFleet());
        return true;
    }
    return false;
}

bool ClientUI::ZoomToTech(const std::string& tech_name)
{
    if (!GetTech(tech_name))
        return false;
    m_map_wnd->ShowTech(tech_name);
    return true;
}

bool ClientUI::ZoomToBuildingType(const std::string& building_type_name)
{
    if (!GetBuildingType(building_type_name))
        return false;
    m_map_wnd->ShowBuildingType(building_type_name);
    return true;
}

bool ClientUI::ZoomToEncyclopediaEntry(const std::string& str)
{ 
    // TODO: Zooming code
    
    return false;
}

void ClientUI::ZoomToSystem(System* system)
{
    if (!system)
        return;

    m_map_wnd->CenterOnObject(system->ID());
    m_map_wnd->SelectSystem(system->ID());
}

void ClientUI::ZoomToFleet(Fleet* fleet)
{
    if (!fleet)
        return;

    m_map_wnd->CenterOnObject(fleet->ID());
    m_map_wnd->SelectFleet(fleet->ID());
    for (MapWnd::FleetWndIter it = m_map_wnd->FleetWndBegin(); it != m_map_wnd->FleetWndEnd(); ++it) {
        if ((*it)->ContainsFleet(fleet->ID())) {
            (*it)->SelectFleet(fleet);
            break;
        }
    }
}

boost::shared_ptr<GG::Texture> ClientUI::GetRandomTexture(const boost::filesystem::path& dir, const std::string& prefix)
{
    TexturesAndDist prefixed_textures_and_dist = PrefixedTexturesAndDist(dir, prefix);
    return prefixed_textures_and_dist.first[(*prefixed_textures_and_dist.second)()];
}

boost::shared_ptr<GG::Texture> ClientUI::GetModuloTexture(const boost::filesystem::path& dir, const std::string& prefix, int n)
{
    assert(0 <= n);
    TexturesAndDist prefixed_textures_and_dist = PrefixedTexturesAndDist(dir, prefix);
    return prefixed_textures_and_dist.first.empty() ? 
        boost::shared_ptr<GG::Texture>() : 
        prefixed_textures_and_dist.first[n % prefixed_textures_and_dist.first.size()];
}

void ClientUI::InitTurn(int turn_number)
{ m_map_wnd->InitTurn(turn_number); }

void ClientUI::RestoreFromSaveData(const SaveGameUIData& ui_data)
{ m_map_wnd->RestoreFromSaveData(ui_data); }

void ClientUI::ShowMap()
{ m_map_wnd->Show(); }

ClientUI* ClientUI::GetClientUI()
{ return s_the_UI; }

void ClientUI::MessageBox(const std::string& message, bool play_alert_sound/* = false*/)
{
    GG::ThreeButtonDlg dlg(320,200,message,GG::GUI::GetGUI()->GetFont(Font(),Pts()+2),WndColor(), WndBorderColor(), CtrlColor(), TextColor(), 1,
                           UserString("OK"));
    if (play_alert_sound && GetOptionsDB().Get<bool>("UI.sound.enabled"))
        HumanClientApp::GetApp()->PlaySound(SoundDir() / "alert.wav");
    dlg.Run();
}

void ClientUI::GenerateSitRepText(SitRepEntry *sit_rep)
{
    std::string template_str(UserString(g_string_id_lut[sit_rep->GetType()]));
    sit_rep->GenerateVarText(template_str);
}

boost::shared_ptr<GG::Texture> ClientUI::GetTexture(const boost::filesystem::path& path, bool mipmap/* = false*/)
{
    try {
        return HumanClientApp::GetApp()->GetTexture(path.native_file_string(), mipmap);
    } catch(...) {
        return HumanClientApp::GetApp()->GetTexture((ClientUI::ArtDir() / "misc" / "missing.png").native_file_string(), mipmap);
    }
}

ClientUI::TexturesAndDist ClientUI::PrefixedTexturesAndDist(const boost::filesystem::path& dir, const std::string& prefix)
{
    namespace fs = boost::filesystem;
    assert(fs::is_directory(dir));
    const std::string KEY = dir.native_directory_string() + "/" + prefix;
    PrefixedTextures::iterator prefixed_textures_it = m_prefixed_textures.find(KEY);
    if (prefixed_textures_it == m_prefixed_textures.end()) {
        prefixed_textures_it = m_prefixed_textures.insert(std::make_pair(KEY, TexturesAndDist())).first;
        std::vector<boost::shared_ptr<GG::Texture> >& textures = prefixed_textures_it->second.first;
        boost::shared_ptr<SmallIntDistType>& rand_int = prefixed_textures_it->second.second;
        fs::directory_iterator end_it;
        for (fs::directory_iterator it(dir); it != end_it; ++it) {
            try {
                if (fs::exists(*it) && !fs::is_directory(*it) && boost::algorithm::starts_with(it->leaf(), prefix))
                    textures.push_back(ClientUI::GetTexture(*it));
            } catch (const fs::filesystem_error& e) {
                // ignore files for which permission is denied, and rethrow other exceptions
                if (e.system_error() != EACCES)
                    throw;
            }
        }
        rand_int.reset(new SmallIntDistType(SmallIntDist(0, textures.size() - 1)));
    }
    return prefixed_textures_it->second;
}

TempUISoundDisabler::TempUISoundDisabler() :
    m_was_enabled(GetOptionsDB().Get<bool>("UI.sound.enabled"))
{
    if (m_was_enabled)
        GetOptionsDB().Set("UI.sound.enabled", false);
}

TempUISoundDisabler::~TempUISoundDisabler()
{
    if (m_was_enabled)
        GetOptionsDB().Set("UI.sound.enabled", true);
}

StreamableColor::StreamableColor() :
    r(0),
    g(0),
    b(0),
    a(0)    
{}

StreamableColor::StreamableColor(const GG::Clr& clr) :
    r(clr.r),
    g(clr.g),
    b(clr.b),
    a(clr.a)    
{}

GG::Clr StreamableColor::ToClr() const
{
    return GG::Clr(r, g, b, a);
}

std::ostream& operator<<(std::ostream& os, const StreamableColor& clr)
{
    os << "(" << clr.r << "," << clr.g << "," << clr.b << "," << clr.a << ")";
    return os;
}

std::istream& operator>>(std::istream& is, StreamableColor& clr)
{
    using namespace boost::spirit;
    rule<> color_p =
        ch_p('(') >> *space_p >>
        int_p[assign(clr.r)] >> *space_p >> ',' >> *space_p >>
        int_p[assign(clr.g)] >> *space_p >> ',' >> *space_p >>
        int_p[assign(clr.b)] >> *space_p >> ',' >> *space_p >>
        int_p[assign(clr.a)] >> *space_p >> ')';
    std::string str;
    char c;
    do {
        is >> c;
        str += c;
    } while (is && c != ')');
    if (!parse(str.c_str(), color_p).full ||
        clr.r < 0 || 255 < clr.r ||
        clr.g < 0 || 255 < clr.g ||
        clr.b < 0 || 255 < clr.b ||
        clr.a < 0 || 255 < clr.a)
        is.setstate(std::ios_base::failbit);
    return is;
}

boost::format FlexibleFormat(const std::string &string_to_format) {
    boost::format retval(string_to_format);
    retval.exceptions(boost::io::all_error_bits ^ (boost::io::too_many_args_bit | boost::io::too_few_args_bit));
    return retval;
}

const double SMALL_UI_DISPLAY_VALUE = 1.0e-6;
const double LARGE_UI_DISPLAY_VALUE = 9.99999999e+9;
const double UNKNOWN_UI_DISPLAY_VALUE = std::numeric_limits<double>::infinity();

int EffectiveSign(double val, bool integerize)
{
    if (val == UNKNOWN_UI_DISPLAY_VALUE)
        return 0;

    if (integerize)
        val = floor(val);

    if (std::abs(val) >= SMALL_UI_DISPLAY_VALUE) {
        if (val >= 0)
            return 1;
        else
            return -1;
    }
    else
        return 0;
}

std::string DoubleToString(double val, int digits, bool integerize, bool showsign)
{
    std::string text = "";

    // minimum digits is 2.  Less can't always be displayed with powers of 1000 base
    digits = std::max(digits, 2);

    // default result for sentinel value
    if (val == UNKNOWN_UI_DISPLAY_VALUE)
        return UserString("UNKNOWN_VALUE_SYMBOL");

    double mag = std::abs(val);

    // integerize?
    if (integerize) {
        mag = floor(mag + 0.499); // round magnitude to nearest integer (with slight down bias)
        if (mag == 0.0) return "0";
    }

    // early termination if magnitude is 0
    if (mag == 0.0) {
        std::string format;
        format += "%1." + boost::lexical_cast<std::string>(digits - 1) + "f";
        text += (boost::format(format) % mag).str();
        return text;
    }

    // prepend signs if neccessary
    int effectiveSign = EffectiveSign(val, integerize);
    if (effectiveSign == -1) {
        text += "-";
    } else {
        if (showsign) text += "+";
    }

    if (mag > LARGE_UI_DISPLAY_VALUE) mag = LARGE_UI_DISPLAY_VALUE;
    
    // if digits 0 or negative, return full precision value
    if (digits < 1) {
        text += boost::lexical_cast<std::string>(mag);
        return text;
    }

    // if value is effectively 0, avoid unnecessary later processing
    if (effectiveSign == 0) {
        if (integerize) {
            text = "0";
        } else {
            text = "0.0";
            for (int n = 2; n < digits; ++n) text += "0";  // fill in 0's to required number of digits
        }
        return text;
    }

    // power of 10 of highest valued digit in number
    int pow10 = static_cast<int>(floor(log10(mag))); // = 2 for 234.4 (100's),  = 4 for 45324 (10000's)

    // power of 10 of lowest digit to be included in number (limited by digits)
    int LDPow10 = pow10 - digits + 1; // = 1 for 234.4 and digits = 2 (10's)

    // Lowest Digit's (number of) Digits Above Next Lowest Power of 1000.  Can be 0, 1 or 2
    int LDDANLP1000;
    if (LDPow10 >= 0)
        LDDANLP1000 = (LDPow10 % 3);    // = 1 for 234.4 with 2 digits (23#.#)
    else
        LDDANLP1000 = (LDPow10 % 3) + 3;// = 2 for 3.25 with 2 digits (3.2##)   (+3 ensure positive result)

    // Lowest Digit's Next Lower Power of 1000
    int LDNLP1000 = LDPow10 - LDDANLP1000;

    // Lowest Digit's Next Higher Power of 1000
    int LDNHP1000 = LDNLP1000 + 3;

    /* Pick what power of 10 to use as base unit.  If lowest digit lines up with a power of 1000, use it.
       Otherwise, have to use next higher power of 1000 to avoid having too many digits.
       Also may set adjusting factor to remove a digit below the units digit if using the next
       higher power of 1000, as highest digit may be less than this, in which case extra 0. at
       start of number needs to be counted in digits */ 
    int unitPow10, digitCor = 0;
    if (LDDANLP1000 == 0)
        unitPow10 = LDNLP1000;
    else
        unitPow10 = LDNHP1000;  

    if (integerize && unitPow10 < 0) unitPow10 = 0;
    if (pow10 < unitPow10) digitCor = -1;   // if value is less than the base unit, there will be a leading 0 using up one digit

    /* round number down at lowest digit to be displayed, to prevent lexical_cast from rounding up
       in cases like 0.998k with 2 digits -> 1.00k */
    double roundingFactor = pow(10.0, static_cast<double>(pow10 - digits + 1));
    mag /= roundingFactor;
    mag = floor(mag);
    mag *= roundingFactor;
    
    // scale number by unit power of 10
    mag /= pow(10.0, static_cast<double>(unitPow10));  // if mag = 45324 and unitPow = 3, get mag = 45.324

    // total digits
    int totalDigits = digits + digitCor;
    // fraction digits:
    int fractionDigits = 0;
    if (!integerize) fractionDigits = unitPow10 - LDPow10;
    
    std::string format;
    format += "%" + boost::lexical_cast<std::string>(totalDigits) + "." + 
                    boost::lexical_cast<std::string>(fractionDigits) + "f";
    text += (boost::format(format) % mag).str();

    // append base scale SI prefix (as postfix)
    switch (unitPow10) {
    case -15:
        text += "f";    // femto
        break;
    case -12:
        text += "p";    // pico
        break;
    case -9:
        text += "n";    // nano
        break;
    case -6:
        text += "µ";    // micro
        break;
    case -3:
        text += "m";    // milli
        break;
    case 3:
        text += "k";    // kilo
        break;
    case 6:
        text += "M";    // Mega
        break;
    case 9:
        text += "G";    // Giga
        break;
    case 12:
        text += "T";    // Terra
        break;
    default:
        break;
    }
    return text;
}

