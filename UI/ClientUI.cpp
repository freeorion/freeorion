#include "ClientUI.h"

#include "CUIControls.h"
#include "FleetWnd.h"
#include "IntroScreen.h"
#include "MapWnd.h"
#include "SidePanel.h"
#include "Sound.h"
#include "../util/AppInterface.h"

#undef int64_t

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
#include <GG/UnicodeCharsets.h>
#include <GG/dialogs/ThreeButtonDlg.h>

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include <boost/spirit/include/classic.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/system_error.hpp>

#include <string>

const Tech* GetTech(const std::string& name);

namespace fs = boost::filesystem;

// static members
fs::path    ClientUI::ArtDir()                  { return GetResourceDir() / "data" / "art"; }
fs::path    ClientUI::SoundDir()                { return GetResourceDir() / "data" / "sound"; }

std::string ClientUI::Font()                    { return (GetResourceDir() / GetOptionsDB().Get<std::string>("UI.font")).file_string(); }
std::string ClientUI::BoldFont()                { return (GetResourceDir() / GetOptionsDB().Get<std::string>("UI.font-bold")).file_string(); }
std::string ClientUI::TitleFont()               { return (GetResourceDir() / GetOptionsDB().Get<std::string>("UI.title-font")).file_string(); }

int         ClientUI::Pts()                     { return GetOptionsDB().Get<int>("UI.font-size"); }
int         ClientUI::TitlePts()                { return GetOptionsDB().Get<int>("UI.title-font-size"); }

GG::Clr     ClientUI::TextColor()               { return GetOptionsDB().Get<StreamableColor>("UI.text-color").ToClr(); }

// windows
GG::Clr     ClientUI::WndColor()                { return GetOptionsDB().Get<StreamableColor>("UI.wnd-color").ToClr(); }
GG::Clr     ClientUI::WndOuterBorderColor()     { return GetOptionsDB().Get<StreamableColor>("UI.wnd-outer-border-color").ToClr(); }
GG::Clr     ClientUI::WndInnerBorderColor()     { return GetOptionsDB().Get<StreamableColor>("UI.wnd-inner-border-color").ToClr(); }

// controls
GG::Clr     ClientUI::CtrlColor()               { return GetOptionsDB().Get<StreamableColor>("UI.ctrl-color").ToClr(); }
GG::Clr     ClientUI::CtrlBorderColor()         { return GetOptionsDB().Get<StreamableColor>("UI.ctrl-border-color").ToClr(); }

int         ClientUI::ScrollWidth()             { return GetOptionsDB().Get<int>("UI.scroll-width"); }

GG::Clr     ClientUI::DropDownListArrowColor()  { return GetOptionsDB().Get<StreamableColor>("UI.dropdownlist-arrow-color").ToClr(); }

GG::Clr     ClientUI::EditHiliteColor()         { return GetOptionsDB().Get<StreamableColor>("UI.edit-hilite").ToClr(); }

GG::Clr     ClientUI::StatIncrColor()           { return GetOptionsDB().Get<StreamableColor>("UI.stat-increase-color").ToClr(); }
GG::Clr     ClientUI::StatDecrColor()           { return GetOptionsDB().Get<StreamableColor>("UI.stat-decrease-color").ToClr(); }

GG::Clr     ClientUI::StateButtonColor()        { return GetOptionsDB().Get<StreamableColor>("UI.state-button-color").ToClr(); }

double      ClientUI::SystemCircleSize()        { return GetOptionsDB().Get<double>("UI.system-circle-size"); }
int         ClientUI::SystemIconSize()          { return GetOptionsDB().Get<int>("UI.system-icon-size"); }
GG::Clr     ClientUI::SystemNameTextColor()     { return GetOptionsDB().Get<StreamableColor>("UI.system-name-unowned-color").ToClr(); }
double      ClientUI::SystemSelectionIndicatorSize()    { return GetOptionsDB().Get<double>("UI.system-selection-indicator-size"); }

double      ClientUI::TinyFleetButtonZoomThreshold()    { return GetOptionsDB().Get<double>("UI.tiny-fleet-button-minimum-zoom"); }
double      ClientUI::SmallFleetButtonZoomThreshold()   { return GetOptionsDB().Get<double>("UI.small-fleet-button-minimum-zoom"); }
double      ClientUI::MediumFleetButtonZoomThreshold()  { return GetOptionsDB().Get<double>("UI.medium-fleet-button-minimum-zoom"); }


// content texture getters
boost::shared_ptr<GG::Texture> ClientUI::ShipIcon(int design_id)
{
    std::string graphic_name = "";
    const ShipDesign* design = GetShipDesign(design_id);
    if (design) {
        graphic_name = design->Graphic();
        if (graphic_name.empty())
            if (const HullType* hull_type = design->GetHull())
                return ClientUI::HullTexture(hull_type->Name());
    } else {
        return ClientUI::HullTexture("");
    }
    return ClientUI::GetTexture(ArtDir() / graphic_name, true);
}

boost::shared_ptr<GG::Texture> ClientUI::BuildingTexture(const std::string& building_type_name)
{
    const BuildingType* building_type = GetBuildingType(building_type_name);
    std::string graphic_name = "";
    if (building_type)
        graphic_name = building_type->Graphic();
    if (graphic_name.empty())
        return ClientUI::GetTexture(ArtDir() / "icons" / "building" / "generic_building.png", true);
    return ClientUI::GetTexture(ArtDir() / graphic_name, true);
}

boost::shared_ptr<GG::Texture> ClientUI::CategoryIcon(const std::string& category_name)
{
    std::string icon_filename;
    const TechCategory* category = GetTechCategory(category_name);
    if (category)
        icon_filename = category->graphic;
    return ClientUI::GetTexture(ArtDir() / "icons" / "tech" / "categories" / icon_filename, true);
}

boost::shared_ptr<GG::Texture> ClientUI::TechTexture(const std::string& tech_name)
{
    const Tech* tech = GetTechManager().GetTech(tech_name);
    std::string texture_name = "";
    if (tech)
        texture_name = tech->Graphic();
    if (texture_name.empty())
        return CategoryIcon(tech->Category());
    return ClientUI::GetTexture(ArtDir() / texture_name, true);
}

boost::shared_ptr<GG::Texture> ClientUI::SpecialTexture(const std::string& special_name)
{
    const Special* special = GetSpecial(special_name);
    std::string texture_name = "";
    if (special)
        texture_name = special->Graphic();
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "icons" / "specials_huge" / "generic_special.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name, true);
}

boost::shared_ptr<GG::Texture> ClientUI::PartTexture(const std::string& part_name)
{
    const PartType* part = GetPartType(part_name);
    std::string texture_name = "";
    if (part)
        texture_name = part->Graphic();
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "icons" / "ship_parts" / "generic_part.png", false);
    return ClientUI::GetTexture(ArtDir() / texture_name, false);
}

boost::shared_ptr<GG::Texture> ClientUI::HullTexture(const std::string& hull_name)
{
    const HullType* hull = GetHullType(hull_name);
    std::string texture_name = "";
    if (hull)
        texture_name = hull->Graphic();
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "hulls_design" / "generic_hull.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name, true);
}

boost::shared_ptr<GG::Texture> ClientUI::MeterIcon(MeterType meter_type)
{
    std::string icon_filename;
    switch (meter_type) {
    case METER_POPULATION:
    case METER_TARGET_POPULATION:
        icon_filename = "pop.png";
        break;
    case METER_FARMING:
    case METER_TARGET_FARMING:
        icon_filename = "farming.png";
        break;
    case METER_INDUSTRY:
    case METER_TARGET_INDUSTRY:
        icon_filename = "industry.png";
        break;
    case METER_RESEARCH:
    case METER_TARGET_RESEARCH:
        icon_filename = "research.png";
        break;
    case METER_TRADE:
    case METER_TARGET_TRADE:
        icon_filename = "trade.png";
        break;
    case METER_MINING:
    case METER_TARGET_MINING:
        icon_filename = "mining.png";
        break;
    case METER_CONSTRUCTION:
    case METER_TARGET_CONSTRUCTION:
        icon_filename = "construction.png";
        break;
    case METER_HEALTH:
    case METER_TARGET_HEALTH:
        icon_filename = "health.png";
        break;
    case METER_FUEL:
    case METER_MAX_FUEL:
        icon_filename = "fuel.png";
        break;
    case METER_SUPPLY:
        icon_filename = "supply.png";
        break;
    case METER_STEALTH:
        icon_filename = "stealth.png";
        break;
    case METER_DETECTION:
        icon_filename = "detection.png";
        break;
    case METER_SHIELD:
    case METER_MAX_SHIELD:
        icon_filename = "shield.png";
        break;
    case METER_DEFENSE:
    case METER_MAX_DEFENSE:
        icon_filename = "defense.png";
        break;
    default:
        break;
    }
    return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / icon_filename, true);
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
    const TechCategory* category = GetTechCategory(category_name);
    if (category)
        return category->colour;
    return GG::Clr();
}

std::map<PlanetType, std::string>& ClientUI::PlanetTypeFilePrefixes()
{
    static std::map<PlanetType, std::string> prefixes;
    if (prefixes.empty()) {
        prefixes[PT_SWAMP] = "Swamp";
        prefixes[PT_TOXIC] = "Toxic";
        prefixes[PT_INFERNO] = "Inferno";
        prefixes[PT_RADIATED] = "Radiated";
        prefixes[PT_BARREN] = "Barren";
        prefixes[PT_TUNDRA] = "Tundra";
        prefixes[PT_DESERT] = "Desert";
        prefixes[PT_TERRAN] = "Terran";
        prefixes[PT_OCEAN] = "Ocean";
        prefixes[PT_GASGIANT] = "GasGiant";
    }
    return prefixes;
}

std::map<StarType, std::string>& ClientUI::StarTypeFilePrefixes()
{
    static std::map<StarType, std::string> prefixes;
    if (prefixes.empty()) {
        prefixes[STAR_BLUE] = "blue";
        prefixes[STAR_WHITE] = "white";
        prefixes[STAR_YELLOW] = "yellow";
        prefixes[STAR_ORANGE] = "orange";
        prefixes[STAR_RED] = "red";
        prefixes[STAR_NEUTRON] = "neutron";
        prefixes[STAR_BLACK] = "blackhole";
    }
    return prefixes;
}

std::map<StarType, std::string>& ClientUI::HaloStarTypeFilePrefixes()
{
    static std::map<StarType, std::string> prefixes;
    if (prefixes.empty()) {
        prefixes[STAR_BLUE] = "halo_blue";
        prefixes[STAR_WHITE] = "halo_white";
        prefixes[STAR_YELLOW] = "halo_yellow";
        prefixes[STAR_ORANGE] = "halo_orange";
        prefixes[STAR_RED] = "halo_red";
        prefixes[STAR_NEUTRON] = "halo_neutron";
        prefixes[STAR_BLACK] = "halo_blackhole";
    }
    return prefixes;
}

// private static members
ClientUI* ClientUI::s_the_UI = 0;

std::ostream& operator<< (std::ostream& os, const GG::UnicodeCharset& chset)
{
    os << chset.m_script_name << " " << chset.m_first_char << " " << chset.m_last_char << "\n";
    return os;
}

namespace {
    const std::vector<GG::UnicodeCharset>& RequiredCharsets()
    {
        static std::vector<GG::UnicodeCharset> retval;
        if (retval.empty()) {
            // Basic Latin, Latin-1 Supplement, and Latin Extended-A
            // (character sets needed to display the credits page)
            const std::string CREDITS_STR = "AöŁ";
            std::set<GG::UnicodeCharset> credits_charsets = GG::UnicodeCharsetsToRender(CREDITS_STR);

            std::string file_name = GetOptionsDB().Get<std::string>("stringtable-filename");
            std::string stringtable_str;

            boost::filesystem::ifstream ifs(GetResourceDir() / file_name);
            while (ifs) {
                std::string line;
                std::getline(ifs, line);
                stringtable_str += line;
                stringtable_str += '\n';
            }
            std::set<GG::UnicodeCharset> stringtable_charsets = GG::UnicodeCharsetsToRender(stringtable_str);

            std::set_union(credits_charsets.begin(), credits_charsets.end(),
                           stringtable_charsets.begin(), stringtable_charsets.end(),
                           std::back_inserter(retval));
        }
        return retval;
    }

    // an internal LUT of string IDs for each SitRep type
    // It's in this module because SitReps know nothing about how they
    // should be rendered - this is up to the client UI
    const char* g_string_id_lut[ SitRepEntry::NUM_SITREP_TYPES ] =
    {
        "SITREP_SHIP_BUILT",
        "SITREP_BUILDING_BUILT",
        "SITREP_TECH_RESEARCHED",
        "SITREP_COMBAT_SYSTEM",
        "SITREP_PLANET_CAPTURED",
        "SITREP_PLANET_LOST_STARVED_TO_DEATH",
        "SITREP_PLANET_COLONIZED",
        "SITREP_FLEET_ARRIVED_AT_DESTINATION",
        "SITREP_EMPIRE_ELIMINATED",
        "SITREP_VICTORY"
    };
    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add("app-width",             "OPTIONS_DB_APP_WIDTH",             1024,   RangedValidator<int>(800, 2560));
        db.Add("app-height",            "OPTIONS_DB_APP_HEIGHT",            768,    RangedValidator<int>(600, 1600));
        db.Add("app-width-windowed",    "OPTIONS_DB_APP_WIDTH_WINDOWED",    1024,   RangedValidator<int>(800, 2560));
        db.Add("app-height-windowed",   "OPTIONS_DB_APP_HEIGHT_WINDOWED",   768,    RangedValidator<int>(600, 1600));

        db.Add('c', "color-depth",      "OPTIONS_DB_COLOR_DEPTH",           32,     RangedStepValidator<int>(8, 16, 32));
        db.Add("show-fps",              "OPTIONS_DB_SHOW_FPS",              false);
        db.Add("limit-fps",             "OPTIONS_DB_LIMIT_FPS",             true);
        db.Add("max-fps",               "OPTIONS_DB_MAX_FPS",               60.0,   RangedStepValidator<double>(1.0, 10.0, 200.0));

        // sound
        db.Add("UI.sound.enabled",                              "OPTIONS_DB_UI_SOUND_ENABLED",                  true,                   Validator<bool>());
        db.Add("UI.sound.volume",                               "OPTIONS_DB_UI_SOUND_VOLUME",                   255,                    RangedValidator<int>(0, 255));
        db.Add<std::string>("UI.sound.button-rollover",         "OPTIONS_DB_UI_SOUND_BUTTON_ROLLOVER",          "button_rollover.wav");
        db.Add<std::string>("UI.sound.button-click",            "OPTIONS_DB_UI_SOUND_BUTTON_CLICK",             "button_click.wav");
        db.Add<std::string>("UI.sound.turn-button-click",       "OPTIONS_DB_UI_SOUND_TURN_BUTTON_CLICK",        "turn_button_click.wav");
        db.Add<std::string>("UI.sound.list-select",             "OPTIONS_DB_UI_SOUND_LIST_SELECT",              "list_select.wav");
        db.Add<std::string>("UI.sound.item-drop",               "OPTIONS_DB_UI_SOUND_ITEM_DROP",                "item_drop.wav");
        db.Add<std::string>("UI.sound.list-pulldown",           "OPTIONS_DB_UI_SOUND_LIST_PULLDOWN",            "list_pulldown.wav");
        db.Add<std::string>("UI.sound.text-typing",             "OPTIONS_DB_UI_SOUND_TEXT_TYPING",              "text_typing.wav");
        db.Add<std::string>("UI.sound.window-maximize",         "OPTIONS_DB_UI_SOUND_WINDOW_MAXIMIZE",          "window_maximize.wav");
        db.Add<std::string>("UI.sound.window-minimize",         "OPTIONS_DB_UI_SOUND_WINDOW_MINIMIZE",          "window_minimize.wav");
        db.Add<std::string>("UI.sound.window-close",            "OPTIONS_DB_UI_SOUND_WINDOW_CLOSE",             "window_close.wav");
        db.Add<std::string>("UI.sound.alert",                   "OPTIONS_DB_UI_SOUND_ALERT",                    "alert.wav");
        db.Add<std::string>("UI.sound.planet-button-click",     "OPTIONS_DB_UI_SOUND_PLANET_BUTTON_CLICK",      "button_click.wav");
        db.Add<std::string>("UI.sound.fleet-button-rollover",   "OPTIONS_DB_UI_SOUND_FLEET_BUTTON_ROLLOVER",    "fleet_button_rollover.wav");
        db.Add<std::string>("UI.sound.fleet-button-click",      "OPTIONS_DB_UI_SOUND_FLEET_BUTTON_CLICK",       "fleet_button_click.wav");
        db.Add<std::string>("UI.sound.system-icon-rollover",    "OPTIONS_DB_UI_SOUND_SYSTEM_ICON_ROLLOVER",     "fleet_button_rollover.wav");
        db.Add<std::string>("UI.sound.sidepanel-open",          "OPTIONS_DB_UI_SOUND_SIDEPANEL_OPEN",           "sidepanel_open.wav");
        db.Add<std::string>("UI.sound.farming-focus",           "OPTIONS_DB_UI_SOUND_FARMING_FOCUS",            "farm_select.wav");
        db.Add<std::string>("UI.sound.industry-focus",          "OPTIONS_DB_UI_SOUND_INDUSTRY_FOCUS",           "industry_select.wav");
        db.Add<std::string>("UI.sound.research-focus",          "OPTIONS_DB_UI_SOUND_RESEARCH_FOCUS",           "research_select.wav");
        db.Add<std::string>("UI.sound.mining-focus",            "OPTIONS_DB_UI_SOUND_MINING_FOCUS",             "mining_select.wav");
        db.Add<std::string>("UI.sound.trade-focus",             "OPTIONS_DB_UI_SOUND_TRADE_FOCUS",              "trade_select.wav");
        db.Add<std::string>("UI.sound.balanced-focus",          "OPTIONS_DB_UI_SOUND_BALANCED_FOCUS",           "balanced_select.wav");

        // fonts
        db.Add<std::string>("UI.font",          "OPTIONS_DB_UI_FONT",                       "DejaVuSans.ttf");
        db.Add<std::string>("UI.font-bold",     "OPTIONS_DB_UI_FONT_BOLD",                  "DejaVuSans-Bold.ttf");
        db.Add("UI.font-size",                  "OPTIONS_DB_UI_FONT_SIZE",                  12,                     RangedValidator<int>(4, 40));
        db.Add<std::string>("UI.title-font",    "OPTIONS_DB_UI_TITLE_FONT",                 "DejaVuSans.ttf");
        db.Add("UI.title-font-size",            "OPTIONS_DB_UI_TITLE_FONT_SIZE",            12,                     RangedValidator<int>(4, 40));

        // colors
        db.Add("UI.wnd-color",                  "OPTIONS_DB_UI_WND_COLOR",                  StreamableColor(GG::Clr(35, 35, 35, 240)),      Validator<StreamableColor>());
        db.Add("UI.wnd-outer-border-color",     "OPTIONS_DB_UI_WND_OUTER_BORDER_COLOR",     StreamableColor(GG::Clr(64, 64, 64, 255)),      Validator<StreamableColor>());
        db.Add("UI.wnd-inner-border-color",     "OPTIONS_DB_UI_WND_INNER_BORDER_COLOR",     StreamableColor(GG::Clr(192, 192, 192, 255)),   Validator<StreamableColor>());

        db.Add("UI.ctrl-color",                 "OPTIONS_DB_UI_CTRL_COLOR",                 StreamableColor(GG::Clr(15, 15, 15, 255)),      Validator<StreamableColor>());
        db.Add("UI.ctrl-border-color",          "OPTIONS_DB_UI_CTRL_BORDER_COLOR",          StreamableColor(GG::Clr(124, 124, 124, 255)),   Validator<StreamableColor>());

        db.Add("UI.dropdownlist-arrow-color",   "OPTIONS_DB_UI_DROPDOWNLIST_ARROW_COLOR",   StreamableColor(GG::Clr(130, 130, 0, 255)),     Validator<StreamableColor>());

        db.Add("UI.edit-hilite",                "OPTIONS_DB_UI_EDIT_HILITE",                StreamableColor(GG::Clr(43, 81, 102, 255)),     Validator<StreamableColor>());

        db.Add("UI.stat-increase-color",        "OPTIONS_DB_UI_STAT_INCREASE_COLOR",        StreamableColor(GG::Clr(0, 255, 0, 255)),       Validator<StreamableColor>());
        db.Add("UI.stat-decrease-color",        "OPTIONS_DB_UI_STAT_DECREASE_COLOR",        StreamableColor(GG::Clr(255, 0, 0, 255)),       Validator<StreamableColor>());

        db.Add("UI.state-button-color",         "OPTIONS_DB_UI_STATE_BUTTON_COLOR",         StreamableColor(GG::Clr(0, 127, 0, 255)),       Validator<StreamableColor>());

        db.Add("UI.text-color",                 "OPTIONS_DB_UI_TEXT_COLOR",                 StreamableColor(GG::Clr(255, 255, 255, 255)),   Validator<StreamableColor>());

        db.Add("UI.known-tech",                 "OPTIONS_DB_UI_KNOWN_TECH",                 StreamableColor(GG::Clr(72, 72, 72, 255)),      Validator<StreamableColor>());
        db.Add("UI.known-tech-border",          "OPTIONS_DB_UI_KNOWN_TECH_BORDER",          StreamableColor(GG::Clr(164, 164, 164, 255)),   Validator<StreamableColor>());
        db.Add("UI.researchable-tech",          "OPTIONS_DB_UI_RESEARCHABLE_TECH",          StreamableColor(GG::Clr(48, 48, 48, 255)),      Validator<StreamableColor>());
        db.Add("UI.researchable-tech-border",   "OPTIONS_DB_UI_RESEARCHABLE_TECH_BORDER",   StreamableColor(GG::Clr(164, 164, 164, 255)),   Validator<StreamableColor>());
        db.Add("UI.unresearchable-tech",        "OPTIONS_DB_UI_UNRESEARCHABLE_TECH",        StreamableColor(GG::Clr(30, 30, 30, 255)),      Validator<StreamableColor>());
        db.Add("UI.unresearchable-tech-border", "OPTIONS_DB_UI_UNRESEARCHABLE_TECH_BORDER", StreamableColor(GG::Clr(86, 86, 86, 255)),      Validator<StreamableColor>());
        db.Add("UI.tech-progress-background",   "OPTIONS_DB_UI_TECH_PROGRESS_BACKGROUND",   StreamableColor(GG::Clr(72, 72, 72, 255)),      Validator<StreamableColor>());
        db.Add("UI.tech-progress",              "OPTIONS_DB_UI_TECH_PROGRESS",              StreamableColor(GG::Clr(40, 40, 40, 255)),      Validator<StreamableColor>());

        // misc
        db.Add("UI.scroll-width",               "OPTIONS_DB_UI_SCROLL_WIDTH",               14,         RangedValidator<int>(8, 30));

        // UI behavior
        db.Add("UI.tooltip-delay",              "OPTIONS_DB_UI_TOOLTIP_DELAY",              100,        RangedValidator<int>(0, 3000));
        db.Add("UI.multiple-fleet-windows",     "OPTIONS_DB_UI_MULTIPLE_FLEET_WINDOWS",     false);
        db.Add("UI.fleet-autoselect",           "OPTIONS_DB_UI_FLEET_AUTOSELECT",           true);
        db.Add("UI.window-quickclose",          "OPTIONS_DB_UI_WINDOW_QUICKCLOSE",          true);
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
    if (const Planet* planet = GetMainObjectMap().Object<Planet>(id))
        return ZoomToSystem(planet->SystemID());
    return false;
}

bool ClientUI::ZoomToSystem(int id)
{
    if (const System* system = GetMainObjectMap().Object<System>(id)) {
        ZoomToSystem(system);
        return true;
    }
    return false;
}

bool ClientUI::ZoomToFleet(int id)
{
    if (const Fleet* fleet = GetObject<Fleet>(id)) {
        ZoomToFleet(fleet);
        return true;
    }
    return false;
}

bool ClientUI::ZoomToShip(int id)
{
    if (const Ship* ship = GetObject<Ship>(id))
        return ZoomToFleet(ship->FleetID());
    return false;
}

bool ClientUI::ZoomToBuilding(int id)
{
    if (const Building* building = GetMainObjectMap().Object<Building>(id))
        return ZoomToPlanet(building->PlanetID());
    return false;
}

void ClientUI::ZoomToSystem(const System* system)
{
    if (!system)
        return;

    m_map_wnd->CenterOnObject(system->ID());
    m_map_wnd->SelectSystem(system->ID());
}

void ClientUI::ZoomToFleet(const Fleet* fleet)
{
    if (!fleet)
        return;

    m_map_wnd->CenterOnObject(fleet->ID());
    m_map_wnd->SelectFleet(fleet->ID());
    if (FleetWnd* fleet_wnd = FleetUIManager::GetFleetUIManager().WndForFleet(fleet))
        fleet_wnd->SelectFleet(fleet->ID());
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

boost::shared_ptr<GG::Texture> ClientUI::GetRandomTexture(const boost::filesystem::path& dir, const std::string& prefix, bool mipmap/* = false*/)
{
    TexturesAndDist prefixed_textures_and_dist = PrefixedTexturesAndDist(dir, prefix, mipmap);
    return prefixed_textures_and_dist.first[(*prefixed_textures_and_dist.second)()];
}

boost::shared_ptr<GG::Texture> ClientUI::GetModuloTexture(const boost::filesystem::path& dir, const std::string& prefix, int n, bool mipmap/* = false*/)
{
    assert(0 <= n);
    TexturesAndDist prefixed_textures_and_dist = PrefixedTexturesAndDist(dir, prefix, mipmap);
    return prefixed_textures_and_dist.first.empty() ?
        boost::shared_ptr<GG::Texture>() :
        prefixed_textures_and_dist.first[n % prefixed_textures_and_dist.first.size()];
}

std::vector<boost::shared_ptr<GG::Texture> > ClientUI::GetPrefixedTextures(const boost::filesystem::path& dir, const std::string& prefix, bool mipmap/* = false*/)
{
    TexturesAndDist prefixed_textures_and_dist = PrefixedTexturesAndDist(dir, prefix, mipmap);
    return prefixed_textures_and_dist.first;
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
    GG::ThreeButtonDlg dlg(GG::X(320), GG::Y(200), message,GetFont(Pts()+2),
                           WndColor(), WndOuterBorderColor(), CtrlColor(), TextColor(), 1,
                           UserString("OK"));
    if (play_alert_sound)
        Sound::GetSound().PlaySound(SoundDir() / "alert.wav", true);
    dlg.Run();
}

void ClientUI::GenerateSitRepText(SitRepEntry *sit_rep)
{
    std::string template_str(UserString(g_string_id_lut[sit_rep->GetType()]));
    sit_rep->GenerateVarText(template_str);
}

boost::shared_ptr<GG::Texture> ClientUI::GetTexture(const boost::filesystem::path& path, bool mipmap/* = false*/)
{
    boost::shared_ptr<GG::Texture> retval;
    try {
        retval = HumanClientApp::GetApp()->GetTexture(path.file_string(), mipmap);
    } catch(...) {
        retval = HumanClientApp::GetApp()->GetTexture((ClientUI::ArtDir() / "misc" / "missing.png").file_string(), mipmap);
    }
#ifdef FREEORION_MACOSX
    if (!mipmap)
        retval->SetFilters(GL_LINEAR, GL_LINEAR);
#endif
    return retval;
}

boost::shared_ptr<GG::Font> ClientUI::GetFont(int pts/* = Pts()*/)
{ return GG::GUI::GetGUI()->GetFont(Font(), pts, RequiredCharsets().begin(), RequiredCharsets().end()); }

boost::shared_ptr<GG::Font> ClientUI::GetBoldFont(int pts/* = Pts()*/)
{ return GG::GUI::GetGUI()->GetFont(BoldFont(), pts, RequiredCharsets().begin(), RequiredCharsets().end()); }

boost::shared_ptr<GG::Font> ClientUI::GetTitleFont(int pts/* = TitlePts()*/)
{ return GG::GUI::GetGUI()->GetFont(TitleFont(), pts, RequiredCharsets().begin(), RequiredCharsets().end()); }

ClientUI::TexturesAndDist ClientUI::PrefixedTexturesAndDist(const boost::filesystem::path& dir, const std::string& prefix, bool mipmap)
{
    namespace fs = boost::filesystem;
    assert(fs::is_directory(dir));
    const std::string KEY = dir.directory_string() + "/" + prefix;
    PrefixedTextures::iterator prefixed_textures_it = m_prefixed_textures.find(KEY);
    if (prefixed_textures_it == m_prefixed_textures.end()) {
        prefixed_textures_it = m_prefixed_textures.insert(std::make_pair(KEY, TexturesAndDist())).first;
        std::vector<boost::shared_ptr<GG::Texture> >& textures = prefixed_textures_it->second.first;
        boost::shared_ptr<SmallIntDistType>& rand_int = prefixed_textures_it->second.second;
        fs::directory_iterator end_it;
        for (fs::directory_iterator it(dir); it != end_it; ++it) {
            try {
                if (fs::exists(*it) && !fs::is_directory(*it) && boost::algorithm::starts_with(it->filename(), prefix))
                    textures.push_back(ClientUI::GetTexture(*it, mipmap));
            } catch (const fs::filesystem_error& e) {
                // ignore files for which permission is denied, and rethrow other exceptions
                if (e.code() != boost::system::posix_error::permission_denied)
                    throw;
            }
        }
        rand_int.reset(new SmallIntDistType(SmallIntDist(0, textures.size() - 1)));
    }
    return prefixed_textures_it->second;
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
    using namespace boost::spirit::classic;
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

const double SMALL_UI_DISPLAY_VALUE = 1.0e-6;
const double LARGE_UI_DISPLAY_VALUE = 9.99999999e+9;
const double UNKNOWN_UI_DISPLAY_VALUE = std::numeric_limits<double>::infinity();

int EffectiveSign(double val)
{
    if (val == UNKNOWN_UI_DISPLAY_VALUE)
        return 0;

    if (std::abs(val) >= SMALL_UI_DISPLAY_VALUE) {
        if (val >= 0)
            return 1;
        else
            return -1;
    }
    else
        return 0;
}

std::string DoubleToString(double val, int digits, bool always_show_sign)
{
    std::string text = "";

    // minimum digits is 2.  If digits was 1, then 30 couldn't be displayed,
    // as 0.1k is too much and 9 is too small and just 30 is 2 digits
    digits = std::max(digits, 2);

    // default result for sentinel value
    if (val == UNKNOWN_UI_DISPLAY_VALUE)
        return UserString("UNKNOWN_VALUE_SYMBOL");

    double mag = std::abs(val);

    // early termination if magnitude is 0
    if (mag == 0.0) {
        std::string format;
        format += "%1." + boost::lexical_cast<std::string>(digits - 1) + "f";
        text += (boost::format(format) % mag).str();
        return text;
    }

    // prepend signs if neccessary
    int effectiveSign = EffectiveSign(val);
    if (effectiveSign == -1) {
        text += "-";
    } else {
        if (always_show_sign) text += "+";
    }

    if (mag > LARGE_UI_DISPLAY_VALUE) mag = LARGE_UI_DISPLAY_VALUE;

    // if value is effectively 0, avoid unnecessary later processing
    if (effectiveSign == 0) {
        text = "0.0";
        for (int n = 2; n < digits; ++n)
            text += "0";  // fill in 0's to required number of digits
        return text;
    }

    // power of 10 of highest valued digit in number
    int pow10 = static_cast<int>(floor(log10(mag))); // = 2 (100's) for 234.4,  = 4 (10000's) for 45324 

    // power of 10 of lowest digit to be included in number
    int LDPow10 = std::max(pow10 - digits + 1, -digits + 1); // = 1 (10's) for 234.4 and digits = 2,  = -1 (0.1's) for anything smaller than 1.0

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

    if (pow10 < unitPow10) digitCor = -1;   // if value is less than the base unit, there will be a leading 0 using up one digit

    /* round number down at lowest digit to be displayed, to prevent lexical_cast from rounding up
       in cases like 0.998k with 2 digits -> 1.00k  instead of  0.99k  (as it should be) */
    double roundingFactor = pow(10.0, static_cast<double>(pow10 - digits + 1));
    mag /= roundingFactor;
    mag = floor(mag);
    mag *= roundingFactor;

    // scale number by unit power of 10
    mag /= pow(10.0, static_cast<double>(unitPow10));  // if mag = 45324 and unitPow = 3, get mag = 45.324

    // total digits
    int totalDigits = digits + digitCor;
    // fraction digits:
    int fractionDigits = unitPow10 - LDPow10;

    std::string format;
    format += "%" + boost::lexical_cast<std::string>(totalDigits) + "." +
                    boost::lexical_cast<std::string>(fractionDigits) + "f";
    text += (boost::format(format) % mag).str();

    // append base scale SI prefix (as postfix)
    switch (unitPow10) {
    case -15:
        text += "f";        // femto
        break;
    case -12:
        text += "p";        // pico
        break;
    case -9:
        text += "n";        // nano
        break;
    case -6:
        text += "\xC2\xB5"; // micro.  mu in UTF-8
        break;
    case -3:
        text += "m";        // milli
        break;
    case 3:
        text += "k";        // kilo
        break;
    case 6:
        text += "M";        // Mega
        break;
    case 9:
        text += "G";        // Giga
        break;
    case 12:
        text += "T";        // Terra
        break;
    default:
        break;
    }
    return text;
}

