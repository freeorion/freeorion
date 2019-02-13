#include "ClientUI.h"

#include "CUIControls.h"
#include "FleetWnd.h"
#include "IntroScreen.h"
#include "MapWnd.h"
#include "DesignWnd.h"
#include "ChatWnd.h"
#include "PlayerListWnd.h"
#include "MultiplayerLobbyWnd.h"
#include "PasswordEnterWnd.h"
#include "Sound.h"
#include "Hotkeys.h"

#undef int64_t

#include "../util/Random.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/OptionsDB.h"
#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/Tech.h"
#include "../universe/Special.h"
#include "../universe/Species.h"
#include "../universe/Field.h"
#include "../universe/Enums.h"
#include "../combat/CombatLogManager.h"
#include "../client/human/HumanClientApp.h"

#include <GG/Clr.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/RichText/ImageBlock.h>
#include <GG/UnicodeCharsets.h>

// boost::spirit::classic pulls in windows.h which in turn defines the macros
// MessageBox and PlaySound. Undefining those should avoid name collisions with
// FreeOrion function names
#include <boost/spirit/include/classic.hpp>
#ifdef FREEORION_WIN32
#  undef MessageBox
#  undef PlaySound
#endif

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/system_error.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <string>
#include <algorithm>
#include <boost/locale/formatting.hpp>
#include <boost/locale/date_time.hpp>


bool TextureFileNameCompare(const std::shared_ptr<GG::Texture> t1, const std::shared_ptr<GG::Texture> t2)
{ return t1 && t2 && t1->Path() < t2->Path(); }

namespace fs = boost::filesystem;

// static members
fs::path    ClientUI::ArtDir()                  { return GetResourceDir() / "data" / "art"; }
fs::path    ClientUI::SoundDir()                { return GetResourceDir() / "data" / "sound"; }

int         ClientUI::Pts()                     { return GetOptionsDB().Get<int>("ui.font.size"); }
int         ClientUI::TitlePts()                { return GetOptionsDB().Get<int>("ui.font.title.size"); }

GG::Clr     ClientUI::TextColor()               { return GetOptionsDB().Get<GG::Clr>("ui.font.color"); }
GG::Clr     ClientUI::DefaultLinkColor()        { return GetOptionsDB().Get<GG::Clr>("ui.font.link.color"); }
GG::Clr     ClientUI::RolloverLinkColor()       { return GetOptionsDB().Get<GG::Clr>("ui.font.link.rollover.color"); }

// windows
GG::Clr     ClientUI::WndColor()                { return GetOptionsDB().Get<GG::Clr>("ui.window.background.color"); }
GG::Clr     ClientUI::WndOuterBorderColor()     { return GetOptionsDB().Get<GG::Clr>("ui.window.border.outer.color"); }
GG::Clr     ClientUI::WndInnerBorderColor()     { return GetOptionsDB().Get<GG::Clr>("ui.window.border.inner.color"); }

// controls
GG::Clr     ClientUI::CtrlColor()               { return GetOptionsDB().Get<GG::Clr>("ui.control.background.color"); }
GG::Clr     ClientUI::CtrlBorderColor()         { return GetOptionsDB().Get<GG::Clr>("ui.control.border.color"); }
GG::Clr     ClientUI::ButtonHiliteColor() {
    GG::Clr colour = CtrlColor();
    AdjustBrightness(colour, 50);
    return colour;
}

GG::Clr     ClientUI::ButtonHiliteBorderColor() {
    GG::Clr colour = CtrlBorderColor();
    AdjustBrightness(colour, 50);
    return colour;
}

int         ClientUI::ScrollWidth()             { return GetOptionsDB().Get<int>("ui.scroll.width"); }

GG::Clr     ClientUI::DropDownListArrowColor()  { return GetOptionsDB().Get<GG::Clr>("ui.dropdownlist.arrow.color"); }

GG::Clr     ClientUI::EditHiliteColor()         { return GetOptionsDB().Get<GG::Clr>("ui.control.edit.highlight.color"); }

GG::Clr     ClientUI::StatIncrColor()           { return GetOptionsDB().Get<GG::Clr>("ui.font.stat.increase.color"); }
GG::Clr     ClientUI::StatDecrColor()           { return GetOptionsDB().Get<GG::Clr>("ui.font.stat.decrease.color"); }

GG::Clr     ClientUI::StateButtonColor()        { return GetOptionsDB().Get<GG::Clr>("ui.button.state.color"); }

int         ClientUI::SystemIconSize()                  { return GetOptionsDB().Get<int>("ui.map.system.icon.size"); }
int         ClientUI::SystemTinyIconSizeThreshold()     { return GetOptionsDB().Get<int>("ui.map.system.icon.tiny.threshold"); }
int         ClientUI::SystemCircleSize()                { return static_cast<int>(SystemIconSize() * GetOptionsDB().Get<double>("ui.map.system.circle.size")); }
int         ClientUI::SystemSelectionIndicatorSize()    { return static_cast<int>(SystemIconSize() * GetOptionsDB().Get<double>("ui.map.system.select.indicator.size")); }
int         ClientUI::SystemSelectionIndicatorRPM()     { return GetOptionsDB().Get<int>("ui.map.system.select.indicator.rpm"); }

GG::Clr     ClientUI::SystemNameTextColor()             { return GetOptionsDB().Get<GG::Clr>("ui.map.system.unowned.name.color"); }

double      ClientUI::TinyFleetButtonZoomThreshold()    { return GetOptionsDB().Get<double>("ui.map.fleet.button.tiny.zoom.threshold"); }
double      ClientUI::SmallFleetButtonZoomThreshold()   { return GetOptionsDB().Get<double>("ui.map.fleet.button.small.zoom.threshold"); }
double      ClientUI::MediumFleetButtonZoomThreshold()  { return GetOptionsDB().Get<double>("ui.map.fleet.button.medium.zoom.threshold"); }

bool        ClientUI::DisplayTimestamp()                { return GetOptionsDB().Get<bool>("ui.map.messages.timestamp.shown"); }

// content texture getters
std::shared_ptr<GG::Texture> ClientUI::PlanetIcon(PlanetType planet_type) {
    std::string icon_filename;
    switch (planet_type) {
    case PT_SWAMP:
        icon_filename = "swamp.png";    break;
    case PT_TOXIC:
        icon_filename = "toxic.png";    break;
    case PT_INFERNO:
        icon_filename = "inferno.png";  break;
    case PT_RADIATED:
        icon_filename = "radiated.png"; break;
    case PT_BARREN:
        icon_filename = "barren.png";   break;
    case PT_TUNDRA:
        icon_filename = "tundra.png";   break;
    case PT_DESERT:
        icon_filename = "desert.png";   break;
    case PT_TERRAN:
        icon_filename = "terran.png";   break;
    case PT_OCEAN:
        icon_filename = "ocean.png";    break;
    case PT_ASTEROIDS:
        icon_filename = "asteroids.png";break;
    case PT_GASGIANT:
        icon_filename = "gasgiant.png"; break;
    default:
        break;
    }
    return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "planet" / icon_filename, true);
}

std::shared_ptr<GG::Texture> ClientUI::PlanetSizeIcon(PlanetSize planet_size) {
    std::string icon_filename;
    switch (planet_size) {
    case SZ_TINY:
        icon_filename = "tiny.png";    break;
    case SZ_SMALL:
        icon_filename = "small.png";    break;
    case SZ_MEDIUM:
        icon_filename = "medium.png";  break;
    case SZ_LARGE:
        icon_filename = "large.png"; break;
    case SZ_HUGE:
        icon_filename = "huge.png";   break;
    case SZ_ASTEROIDS:
        icon_filename = "asteroids.png";   break;
    case SZ_GASGIANT:
        icon_filename = "gasgiant.png";   break;
    default:
        break;
    }
    return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "planet" / icon_filename, true);
}

std::shared_ptr<GG::Texture> ClientUI::MeterIcon(MeterType meter_type) {
    std::string icon_filename;
    switch (meter_type) {
    case METER_POPULATION:
    case METER_TARGET_POPULATION:
        icon_filename = "pop.png";          break;
    case METER_INDUSTRY:
    case METER_TARGET_INDUSTRY:
        icon_filename = "industry.png";     break;
    case METER_RESEARCH:
    case METER_TARGET_RESEARCH:
        icon_filename = "research.png";     break;
    case METER_TRADE:
    case METER_TARGET_TRADE:
        icon_filename = "trade.png";        break;
    case METER_CONSTRUCTION:
    case METER_TARGET_CONSTRUCTION:
        icon_filename = "construction.png"; break;
    case METER_HAPPINESS:
    case METER_TARGET_HAPPINESS:
        icon_filename = "happiness.png";    break;
    case METER_CAPACITY:
    case METER_MAX_CAPACITY:
        icon_filename = "capacity.png";   break;
    case METER_SECONDARY_STAT:
    case METER_MAX_SECONDARY_STAT:
        icon_filename = "secondary.png";   break;
    case METER_STRUCTURE:
    case METER_MAX_STRUCTURE:
        icon_filename = "structure.png";    break;
    case METER_FUEL:
    case METER_MAX_FUEL:
        icon_filename = "fuel.png";         break;
    case METER_SUPPLY:
    case METER_MAX_SUPPLY:
        icon_filename = "supply.png";       break;
    case METER_STOCKPILE:
    case METER_MAX_STOCKPILE:
        icon_filename = "stockpile.png";    break;
    case METER_STEALTH:
        icon_filename = "stealth.png";      break;
    case METER_DETECTION:
        icon_filename = "detection.png";    break;
    case METER_SHIELD:
    case METER_MAX_SHIELD:
        icon_filename = "shield.png";       break;
    case METER_DEFENSE:
    case METER_MAX_DEFENSE:
        icon_filename = "defense.png";      break;
    case METER_TROOPS:
    case METER_MAX_TROOPS:
        icon_filename = "troops.png";       break;
    case METER_REBEL_TROOPS:
        icon_filename = "rebels.png";       break;
    case METER_SPEED:
        icon_filename = "speed.png";        break;
    default:
        break;
    }
    return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / icon_filename, true);
}

std::shared_ptr<GG::Texture> ClientUI::BuildingIcon(const std::string& building_type_name) {
    const BuildingType* building_type = GetBuildingType(building_type_name);
    std::string graphic_name;
    if (building_type)
        graphic_name = building_type->Icon();
    if (graphic_name.empty())
        return ClientUI::GetTexture(ArtDir() / "icons" / "building" / "generic_building.png", true);
    return ClientUI::GetTexture(ArtDir() / graphic_name, true);
}

std::shared_ptr<GG::Texture> ClientUI::CategoryIcon(const std::string& category_name) {
    std::string icon_filename;
    if (const TechCategory* category = GetTechCategory(category_name))
        return ClientUI::GetTexture(ArtDir() / "icons" / "tech" / "categories" / category->graphic, true);
    else
        return ClientUI::GetTexture(ClientUI::ArtDir() / "", true);
}

std::shared_ptr<GG::Texture> ClientUI::TechIcon(const std::string& tech_name) {
    const Tech* tech = GetTechManager().GetTech(tech_name);
    std::string texture_name;
    if (tech) {
        texture_name = tech->Graphic();
        if (texture_name.empty())
            return CategoryIcon(tech->Category());
    }
    return ClientUI::GetTexture(ArtDir() / texture_name, true);
}

std::shared_ptr<GG::Texture> ClientUI::SpecialIcon(const std::string& special_name) {
    const Special* special = GetSpecial(special_name);
    std::string texture_name;
    if (special)
        texture_name = special->Graphic();
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "icons" / "specials_huge" / "generic_special.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name, true);
}

std::shared_ptr<GG::Texture> ClientUI::SpeciesIcon(const std::string& species_name) {
    const Species* species = GetSpecies(species_name);
    std::string texture_name;
    if (species)
        texture_name = species->Graphic();
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "icons" / "meter" / "pop.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name, true);
}

std::shared_ptr<GG::Texture> ClientUI::FieldTexture(const std::string& field_type_name) {
    const FieldType* type = GetFieldType(field_type_name);
    std::string texture_name;
    if (type)
        texture_name = type->Graphic();
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "fields" / "rainbow_storm.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name, true);
}

std::shared_ptr<GG::Texture> ClientUI::PartIcon(const std::string& part_name) {
    const PartType* part = GetPartType(part_name);
    std::string texture_name;
    if (part)
        texture_name = part->Icon();
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "icons" / "ship_parts" / "generic_part.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name, false);
}

std::shared_ptr<GG::Texture> ClientUI::HullTexture(const std::string& hull_name) {
    const HullType* hull = GetHullType(hull_name);
    std::string texture_name;
    if (hull) {
        texture_name = hull->Graphic();
        if (texture_name.empty())
            texture_name = hull->Icon();
    }
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "hulls_design" / "generic_hull.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name, true);
}

std::shared_ptr<GG::Texture> ClientUI::HullIcon(const std::string& hull_name) {
    const HullType* hull = GetHullType(hull_name);
    std::string texture_name;
    if (hull) {
        texture_name = hull->Icon();
        if (texture_name.empty())
            texture_name = hull->Graphic();
    }
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "icons" / "ship_hulls"/ "generic_hull.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name, true);
}

std::shared_ptr<GG::Texture> ClientUI::ShipDesignIcon(int design_id) {
    if (const ShipDesign* design = GetShipDesign(design_id)) {
        const std::string& icon_name = design->Icon();
        if (icon_name.empty())
            return ClientUI::HullIcon(design->Hull());
        else
            return ClientUI::GetTexture(ArtDir() / icon_name, true);
    }
    return ClientUI::HullTexture("");
}


// tech screen
GG::Clr     ClientUI::KnownTechFillColor()                   { return GetOptionsDB().Get<GG::Clr>("ui.research.status.completed.background.color"); }
GG::Clr     ClientUI::KnownTechTextAndBorderColor()          { return GetOptionsDB().Get<GG::Clr>("ui.research.status.completed.border.color"); }
GG::Clr     ClientUI::ResearchableTechFillColor()            { return GetOptionsDB().Get<GG::Clr>("ui.research.status.researchable.background.color"); }
GG::Clr     ClientUI::ResearchableTechTextAndBorderColor()   { return GetOptionsDB().Get<GG::Clr>("ui.research.status.researchable.border.color"); }
GG::Clr     ClientUI::UnresearchableTechFillColor()          { return GetOptionsDB().Get<GG::Clr>("ui.research.status.unresearchable.background.color"); }
GG::Clr     ClientUI::UnresearchableTechTextAndBorderColor() { return GetOptionsDB().Get<GG::Clr>("ui.research.status.unresearchable.border.color"); }
GG::Clr     ClientUI::TechWndProgressBarBackgroundColor()    { return GetOptionsDB().Get<GG::Clr>("ui.research.status.progress.background.color"); }
GG::Clr     ClientUI::TechWndProgressBarColor()              { return GetOptionsDB().Get<GG::Clr>("ui.research.status.progress.color"); }

GG::Clr     ClientUI::CategoryColor(const std::string& category_name) {
    const TechCategory* category = GetTechCategory(category_name);
    if (category)
        return category->colour;
    return GG::Clr();
}

std::map<PlanetType, std::string>& ClientUI::PlanetTypeFilePrefixes() {
    static std::map<PlanetType, std::string> prefixes;
    if (prefixes.empty()) {
        prefixes[PT_SWAMP] =    "Swamp";
        prefixes[PT_TOXIC] =    "Toxic";
        prefixes[PT_INFERNO] =  "Inferno";
        prefixes[PT_RADIATED] = "Radiated";
        prefixes[PT_BARREN] =   "Barren";
        prefixes[PT_TUNDRA] =   "Tundra";
        prefixes[PT_DESERT] =   "Desert";
        prefixes[PT_TERRAN] =   "Terran";
        prefixes[PT_OCEAN] =    "Ocean";
        prefixes[PT_GASGIANT] = "GasGiant";
    }
    return prefixes;
}

std::map<StarType, std::string>& ClientUI::StarTypeFilePrefixes() {
    static std::map<StarType, std::string> prefixes;
    if (prefixes.empty()) {
        prefixes[INVALID_STAR_TYPE] =   "unknown";
        prefixes[STAR_BLUE] =           "blue";
        prefixes[STAR_WHITE] =          "white";
        prefixes[STAR_YELLOW] =         "yellow";
        prefixes[STAR_ORANGE] =         "orange";
        prefixes[STAR_RED] =            "red";
        prefixes[STAR_NEUTRON] =        "neutron";
        prefixes[STAR_BLACK] =          "blackhole";
        prefixes[STAR_NONE] =           "nostar";
    }
    return prefixes;
}

std::map<StarType, std::string>& ClientUI::HaloStarTypeFilePrefixes() {
    static std::map<StarType, std::string> prefixes;
    if (prefixes.empty()) {
        prefixes[INVALID_STAR_TYPE] =   "halo_unknown";
        prefixes[STAR_BLUE] =           "halo_blue";
        prefixes[STAR_WHITE] =          "halo_white";
        prefixes[STAR_YELLOW] =         "halo_yellow";
        prefixes[STAR_ORANGE] =         "halo_orange";
        prefixes[STAR_RED] =            "halo_red";
        prefixes[STAR_NEUTRON] =        "halo_neutron";
        prefixes[STAR_BLACK] =          "halo_blackhole";
        prefixes[STAR_NONE] =           "halo_nostar";
    }
    return prefixes;
}

// private static members
ClientUI* ClientUI::s_the_UI = nullptr;

std::ostream& operator<< (std::ostream& os, const GG::UnicodeCharset& chset) {
    os << chset.m_script_name << " " << chset.m_first_char << " " << chset.m_last_char << "\n";
    return os;
}

namespace {
    const std::vector<GG::UnicodeCharset>& RequiredCharsets() {
        static std::vector<GG::UnicodeCharset> retval;
        if (retval.empty()) {
            // Basic Latin, Latin-1 Supplement, and Latin Extended-A
            // (character sets needed to display the credits page)
            const std::string CREDITS_STR = "AöŁ";
            std::set<GG::UnicodeCharset> credits_charsets = GG::UnicodeCharsetsToRender(CREDITS_STR);

            std::set<GG::UnicodeCharset> stringtable_charsets;
            {
                std::string file_name = GetOptionsDB().Get<std::string>("resource.stringtable.path");
                std::string stringtable_str;
                boost::filesystem::ifstream ifs(file_name);
                while (ifs) {
                    std::string line;
                    std::getline(ifs, line);
                    stringtable_str += line;
                    stringtable_str += '\n';
                }
                stringtable_charsets = GG::UnicodeCharsetsToRender(stringtable_str);
                DebugLogger() << "loading " << stringtable_charsets.size() << " charsets for current stringtable characters";
            }

            if (!GetOptionsDB().IsDefaultValue("resource.stringtable.path")) {
                DebugLogger() << "Non-default stringtable!";
                std::string file_name = GetOptionsDB().GetDefault<std::string>("resource.stringtable.path");
                std::string stringtable_str;
                boost::filesystem::ifstream ifs(file_name);
                while (ifs) {
                    std::string line;
                    std::getline(ifs, line);
                    stringtable_str += line;
                    stringtable_str += '\n';
                }
                std::set<GG::UnicodeCharset> default_stringtable_charsets = GG::UnicodeCharsetsToRender(stringtable_str);
                DebugLogger() << "loading " << default_stringtable_charsets.size() << " charsets for default stringtable characters";

                stringtable_charsets.insert(default_stringtable_charsets.begin(), default_stringtable_charsets.end());
                DebugLogger() << "combined stringtable charsets have " << stringtable_charsets.size() << " charsets";
            }

            std::set_union(credits_charsets.begin(), credits_charsets.end(),
                           stringtable_charsets.begin(), stringtable_charsets.end(),
                           std::back_inserter(retval));

            std::string message_text = "Loading " + std::to_string(retval.size()) + " Unicode charsets: ";
            for (const GG::UnicodeCharset& cs : retval)
            { message_text += cs.m_script_name + ", "; }

            DebugLogger() << message_text;
        }
        return retval;
    }

    // command-line options
    void AddOptions(OptionsDB& db) {
        db.Add("video.fps.shown",                                       UserStringNop("OPTIONS_DB_SHOW_FPS"),                       false);
        db.Add("video.fps.max.enabled",                                 UserStringNop("OPTIONS_DB_LIMIT_FPS"),                      true);
        db.Add("video.fps.max",                                         UserStringNop("OPTIONS_DB_MAX_FPS"),                        60.0,                           RangedStepValidator<double>(1.0, 0.0, 240.0));
        db.Add("video.fps.unfocused.enabled",                           UserStringNop("OPTIONS_DB_LIMIT_FPS_NO_FOCUS"),             true);
        db.Add("video.fps.unfocused",                                   UserStringNop("OPTIONS_DB_MAX_FPS_NO_FOCUS"),               15.0,                           RangedStepValidator<double>(1.0, 1.0, 30.0));

        // sound and music
        db.Add<std::string>("audio.music.path",                         UserStringNop("OPTIONS_DB_BG_MUSIC"),                       (GetRootDataDir() / "default" / "data" / "sound" / "artificial_intelligence_v3.ogg").string());
        db.Add("audio.music.volume",                                    UserStringNop("OPTIONS_DB_MUSIC_VOLUME"),                   127,                            RangedValidator<int>(1, 255));
        db.Add("audio.effects.volume",                                  UserStringNop("OPTIONS_DB_UI_SOUND_VOLUME"),                255,                            RangedValidator<int>(0, 255));
        db.Add<std::string>("ui.button.rollover.sound.path",            UserStringNop("OPTIONS_DB_UI_SOUND_BUTTON_ROLLOVER"),       (GetRootDataDir() / "default" / "data" / "sound" / "button_rollover.ogg").string());
        db.Add<std::string>("ui.button.press.sound.path",               UserStringNop("OPTIONS_DB_UI_SOUND_BUTTON_CLICK"),          (GetRootDataDir() / "default" / "data" / "sound" / "button_click.ogg").string());
        db.Add<std::string>("ui.button.turn.press.sound.path",          UserStringNop("OPTIONS_DB_UI_SOUND_TURN_BUTTON_CLICK"),     (GetRootDataDir() / "default" / "data" / "sound" / "turn_button_click.ogg").string());
        db.Add<std::string>("ui.listbox.select.sound.path",             UserStringNop("OPTIONS_DB_UI_SOUND_LIST_SELECT"),           (GetRootDataDir() / "default" / "data" / "sound" / "list_select.ogg").string());
        db.Add<std::string>("ui.listbox.drop.sound.path",               UserStringNop("OPTIONS_DB_UI_SOUND_ITEM_DROP"),             (GetRootDataDir() / "default" / "data" / "sound" / "list_select.ogg").string());//TODO: replace with dedicated 'item_drop' sound
        db.Add<std::string>("ui.dropdownlist.select.sound.path",        UserStringNop("OPTIONS_DB_UI_SOUND_LIST_PULLDOWN"),         (GetRootDataDir() / "default" / "data" / "sound" / "list_pulldown.ogg").string());
        db.Add<std::string>("ui.input.keyboard.sound.path",             UserStringNop("OPTIONS_DB_UI_SOUND_TEXT_TYPING"),           (GetRootDataDir() / "default" / "data" / "sound" / "text_typing.ogg").string());
        db.Add<std::string>("ui.window.minimize.sound.path",            UserStringNop("OPTIONS_DB_UI_SOUND_WINDOW_MAXIMIZE"),       (GetRootDataDir() / "default" / "data" / "sound" / "window_maximize.ogg").string());
        db.Add<std::string>("ui.window.maximize.sound.path",            UserStringNop("OPTIONS_DB_UI_SOUND_WINDOW_MINIMIZE"),       (GetRootDataDir() / "default" / "data" / "sound" / "window_minimize.ogg").string());
        db.Add<std::string>("ui.window.close.sound.path",               UserStringNop("OPTIONS_DB_UI_SOUND_WINDOW_CLOSE"),          (GetRootDataDir() / "default" / "data" / "sound" / "window_close.ogg").string());
        db.Add<std::string>("ui.alert.sound.path",                      UserStringNop("OPTIONS_DB_UI_SOUND_ALERT"),                 (GetRootDataDir() / "default" / "data" / "sound" / "alert.ogg").string());
        db.Add<std::string>("ui.map.fleet.button.rollover.sound.path",  UserStringNop("OPTIONS_DB_UI_SOUND_FLEET_BUTTON_ROLLOVER"), (GetRootDataDir() / "default" / "data" / "sound" / "fleet_button_rollover.ogg").string());
        db.Add<std::string>("ui.map.fleet.button.press.sound.path",     UserStringNop("OPTIONS_DB_UI_SOUND_FLEET_BUTTON_CLICK"),    (GetRootDataDir() / "default" / "data" / "sound" / "fleet_button_click.ogg").string());
        db.Add<std::string>("ui.map.system.icon.rollover.sound.path",   UserStringNop("OPTIONS_DB_UI_SOUND_SYSTEM_ICON_ROLLOVER"),  (GetRootDataDir() / "default" / "data" / "sound" / "fleet_button_rollover.ogg").string());
        db.Add<std::string>("ui.map.sidepanel.open.sound.path",         UserStringNop("OPTIONS_DB_UI_SOUND_SIDEPANEL_OPEN"),        (GetRootDataDir() / "default" / "data" / "sound" / "sidepanel_open.ogg").string());
        db.Add("ui.turn.start.sound.enabled",                           UserStringNop("OPTIONS_DB_UI_SOUND_NEWTURN_TOGGLE"),        false);
        db.Add<std::string>("ui.turn.start.sound.path",                 UserStringNop("OPTIONS_DB_UI_SOUND_NEWTURN_FILE"),          (GetRootDataDir() / "default" / "data" / "sound" / "newturn.ogg").string());

        // fonts
        db.Add<std::string>("ui.font.path",                             UserStringNop("OPTIONS_DB_UI_FONT"),                        (GetRootDataDir() / "default/data/fonts/Roboto-Regular.ttf").string());
        db.Add<std::string>("ui.font.bold.path",                        UserStringNop("OPTIONS_DB_UI_FONT_BOLD"),                   (GetRootDataDir() / "default" / "data" / "fonts" / "Roboto-Bold.ttf").string());
#ifdef FREEORION_MACOSX
        db.Add("ui.font.size",                                          UserStringNop("OPTIONS_DB_UI_FONT_SIZE"),                   15,                             RangedValidator<int>(4, 40));
#else
        db.Add("ui.font.size",                                          UserStringNop("OPTIONS_DB_UI_FONT_SIZE"),                   16,                             RangedValidator<int>(4, 40));
#endif
        db.Add<std::string>("ui.font.title.path",                       UserStringNop("OPTIONS_DB_UI_TITLE_FONT"),                  (GetRootDataDir() / "default/data/fonts/Roboto-Regular.ttf").string());
#ifdef FREEORION_MACOSX
        db.Add("ui.font.title.size",                                    UserStringNop("OPTIONS_DB_UI_TITLE_FONT_SIZE"),             16,                             RangedValidator<int>(4, 40));
#else
        db.Add("ui.font.title.size",                                    UserStringNop("OPTIONS_DB_UI_TITLE_FONT_SIZE"),             17,                             RangedValidator<int>(4, 40));
#endif

        // colors
        db.Add("ui.window.background.color",                            UserStringNop("OPTIONS_DB_UI_WND_COLOR"),                   GG::Clr(35, 35, 35, 240),       Validator<GG::Clr>());
        db.Add("ui.window.border.outer.color",                          UserStringNop("OPTIONS_DB_UI_WND_OUTER_BORDER_COLOR"),      GG::Clr(64, 64, 64, 255),       Validator<GG::Clr>());
        db.Add("ui.window.border.inner.color",                          UserStringNop("OPTIONS_DB_UI_WND_INNER_BORDER_COLOR"),      GG::Clr(192, 192, 192, 255),    Validator<GG::Clr>());

        db.Add("ui.control.background.color",                           UserStringNop("OPTIONS_DB_UI_CTRL_COLOR"),                  GG::Clr(15, 15, 15, 255),       Validator<GG::Clr>());
        db.Add("ui.control.border.color",                               UserStringNop("OPTIONS_DB_UI_CTRL_BORDER_COLOR"),           GG::Clr(124, 124, 124, 255),    Validator<GG::Clr>());

        db.Add("ui.dropdownlist.arrow.color",                           UserStringNop("OPTIONS_DB_UI_DROPDOWNLIST_ARROW_COLOR"),    GG::Clr(130, 130, 0, 255),      Validator<GG::Clr>());

        db.Add("ui.control.edit.highlight.color",                       UserStringNop("OPTIONS_DB_UI_EDIT_HILITE"),                 GG::Clr(43, 81, 102, 255),      Validator<GG::Clr>());

        db.Add("ui.font.stat.increase.color",                           UserStringNop("OPTIONS_DB_UI_STAT_INCREASE_COLOR"),         GG::Clr(0, 255, 0, 255),        Validator<GG::Clr>());
        db.Add("ui.font.stat.decrease.color",                           UserStringNop("OPTIONS_DB_UI_STAT_DECREASE_COLOR"),         GG::Clr(255, 0, 0, 255),        Validator<GG::Clr>());

        db.Add("ui.button.state.color",                                 UserStringNop("OPTIONS_DB_UI_STATE_BUTTON_COLOR"),          GG::Clr(0, 127, 0, 255),        Validator<GG::Clr>());

        db.Add("ui.font.color",                                         UserStringNop("OPTIONS_DB_UI_TEXT_COLOR"),                  GG::Clr(255, 255, 255, 255),    Validator<GG::Clr>());
        db.Add("ui.font.link.color",                                    UserStringNop("OPTIONS_DB_UI_DEFAULT_LINK_COLOR"),          GG::Clr(80, 255, 128, 255),     Validator<GG::Clr>());
        db.Add("ui.font.link.rollover.color",                           UserStringNop("OPTIONS_DB_UI_ROLLOVER_LINK_COLOR"),         GG::Clr(192, 80, 255, 255),     Validator<GG::Clr>());

        db.Add("ui.research.status.completed.background.color",         UserStringNop("OPTIONS_DB_UI_KNOWN_TECH"),                  GG::Clr(72, 72, 72, 255),       Validator<GG::Clr>());
        db.Add("ui.research.status.completed.border.color",             UserStringNop("OPTIONS_DB_UI_KNOWN_TECH_BORDER"),           GG::Clr(164, 164, 164, 255),    Validator<GG::Clr>());
        db.Add("ui.research.status.researchable.background.color",      UserStringNop("OPTIONS_DB_UI_RESEARCHABLE_TECH"),           GG::Clr(48, 48, 48, 255),       Validator<GG::Clr>());
        db.Add("ui.research.status.researchable.border.color",          UserStringNop("OPTIONS_DB_UI_RESEARCHABLE_TECH_BORDER"),    GG::Clr(164, 164, 164, 255),    Validator<GG::Clr>());
        db.Add("ui.research.status.unresearchable.background.color",    UserStringNop("OPTIONS_DB_UI_UNRESEARCHABLE_TECH"),         GG::Clr(30, 30, 30, 255),       Validator<GG::Clr>());
        db.Add("ui.research.status.unresearchable.border.color",        UserStringNop("OPTIONS_DB_UI_UNRESEARCHABLE_TECH_BORDER"),  GG::Clr(86, 86, 86, 255),       Validator<GG::Clr>());
        db.Add("ui.research.status.progress.background.color",          UserStringNop("OPTIONS_DB_UI_TECH_PROGRESS_BACKGROUND"),    GG::Clr(72, 72, 72, 255),       Validator<GG::Clr>());
        db.Add("ui.research.status.progress.color",                     UserStringNop("OPTIONS_DB_UI_TECH_PROGRESS"),               GG::Clr(40, 40, 40, 255),       Validator<GG::Clr>());

        // misc
        db.Add("ui.scroll.width",                                       UserStringNop("OPTIONS_DB_UI_SCROLL_WIDTH"),                14,                             RangedValidator<int>(8, 30));

        // UI behavior
        db.Add("ui.tooltip.delay",                                      UserStringNop("OPTIONS_DB_UI_TOOLTIP_DELAY"),               500,                            RangedValidator<int>(0, 3000));
        db.Add("ui.tooltip.extended.delay",                             UserStringNop("OPTIONS_DB_UI_TOOLTIP_LONG_DELAY"),          3500,                           RangedValidator<int>(0, 30000));
        db.Add("ui.fleet.multiple.enabled",                             UserStringNop("OPTIONS_DB_UI_MULTIPLE_FLEET_WINDOWS"),      false);
        db.Add("ui.quickclose.enabled",                                 UserStringNop("OPTIONS_DB_UI_WINDOW_QUICKCLOSE"),           true);
        db.Add("ui.reposition.auto.enabled",                            UserStringNop("OPTIONS_DB_UI_AUTO_REPOSITION_WINDOWS"),     true);

        // UI behavior, hidden options
        // currently lacking an options page widget, so can only be user-adjusted by manually editing config file or specifying on command line
        db.Add("ui.design.pedia.title.dynamic.enabled",                 UserStringNop("OPTIONS_DB_DESIGN_PEDIA_DYNAMIC"),           false);
        db.Add("ui.map.fleet.eta.shown",                                UserStringNop("OPTIONS_DB_SHOW_FLEET_ETA"),                 true);
        db.Add("ui.name.id.shown",                                      UserStringNop("OPTIONS_DB_SHOW_IDS_AFTER_NAMES"),           false);

        // Other
        db.Add("resource.shipdesign.saved.enabled",                     UserStringNop("OPTIONS_DB_AUTO_ADD_SAVED_DESIGNS"),         true);
        db.Add("resource.shipdesign.default.enabled",                   UserStringNop("OPTIONS_DB_ADD_DEFAULT_DESIGNS"),            true);

    }
    bool temp_bool = RegisterOptions(&AddOptions);

    const GG::Y PANEL_HEIGHT(160); // Also formerly "UI.chat-panel-height" default
    const GG::X MESSAGE_PANEL_WIDTH(345); // Formerly "UI.chat-panel-width" default
    const GG::X PLAYER_LIST_PANEL_WIDTH(445);

    const std::string MESSAGE_WND_NAME = "map.messages";
    const std::string PLAYER_LIST_WND_NAME = "map.empires";

    template <class OptionType, class PredicateType>
    void ConditionalForward(const std::string& option_name,
                            const OptionsDB::OptionChangedSignalType::slot_type& slot,
                            OptionType ref_val,
                            PredicateType pred)
    {
        if (pred(GetOptionsDB().Get<OptionType>(option_name), ref_val))
            slot();
    }

    template <class OptionType, class PredicateType>
    void ConditionalConnectOption(const std::string& option_name,
                                  const OptionsDB::OptionChangedSignalType::slot_type& slot,
                                  OptionType ref_val,
                                  PredicateType pred)
    {
        GetOptionsDB().OptionChangedSignal(option_name).connect(
            boost::bind(&ConditionalForward<OptionType, PredicateType>,
                        option_name, slot, ref_val, pred));
    }
}


////////////////////////////////////////////////
// ClientUI
////////////////////////////////////////////////
ClientUI::ClientUI() :
    m_ship_designs(new ShipDesignManager())
{
    s_the_UI = this;
    Hotkey::ReadFromOptions(GetOptionsDB());

    // Remove all window properties if asked to
    if (GetOptionsDB().Get<bool>("window-reset"))
        CUIWnd::InvalidateUnusedOptions();

    m_message_wnd = GG::Wnd::Create<MessageWnd>(GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | GG::RESIZABLE |
                                                CLOSABLE | PINABLE, MESSAGE_WND_NAME);
    m_player_list_wnd = GG::Wnd::Create<PlayerListWnd>(PLAYER_LIST_WND_NAME);
    InitializeWindows();

    m_intro_screen = GG::Wnd::Create<IntroScreen>();
    m_multiplayer_lobby_wnd = GG::Wnd::Create<MultiPlayerLobbyWnd>();
    m_password_enter_wnd = GG::Wnd::Create<PasswordEnterWnd>();

    GetOptionsDB().OptionChangedSignal("video.fullscreen.width").connect(
        boost::bind(&ClientUI::HandleSizeChange, this, true));
    GetOptionsDB().OptionChangedSignal("video.fullscreen.height").connect(
        boost::bind(&ClientUI::HandleSizeChange, this, true));
    GetOptionsDB().OptionChangedSignal("video.windowed.width").connect(
        boost::bind(&ClientUI::HandleSizeChange, this, false));
    GetOptionsDB().OptionChangedSignal("video.windowed.height").connect(
        boost::bind(&ClientUI::HandleSizeChange, this, false));
    HumanClientApp::GetApp()->RepositionWindowsSignal.connect(
        boost::bind(&ClientUI::InitializeWindows, this));
    HumanClientApp::GetApp()->RepositionWindowsSignal.connect(
        &CUIWnd::InvalidateUnusedOptions,
        boost::signals2::at_front);

    // Connected at front to make sure CUIWnd::LoadOptions() doesn't overwrite
    // the values we're checking here...
    HumanClientApp::GetApp()->FullscreenSwitchSignal.connect(
        boost::bind(&ClientUI::HandleFullscreenSwitch, this),
        boost::signals2::at_front);

    ConditionalConnectOption("ui.reposition.auto.enabled",
                             HumanClientApp::GetApp()->RepositionWindowsSignal,
                             true, std::equal_to<bool>());

    // Set the root path for image tags in rich text.
    GG::ImageBlock::SetDefaultImagePath(ArtDir().string());
}

ClientUI::~ClientUI()
{ s_the_UI = nullptr; }

std::shared_ptr<MapWnd> ClientUI::GetMapWnd() {
    static bool initialized = m_map_wnd ? true : (m_map_wnd = GG::Wnd::Create<MapWnd>()) != nullptr;
    (void)initialized; // Hide unused variable warning
    return m_map_wnd;
}

MapWnd const* ClientUI::GetMapWndConst() const {
    static bool initialized = m_map_wnd ? true : (m_map_wnd = GG::Wnd::Create<MapWnd>()) != nullptr;
    (void)initialized; // Hide unused variable warning
    return m_map_wnd.get();
}

std::shared_ptr<MessageWnd> ClientUI::GetMessageWnd()
{ return m_message_wnd; }

std::shared_ptr<PlayerListWnd> ClientUI::GetPlayerListWnd()
{ return m_player_list_wnd; }

std::shared_ptr<IntroScreen> ClientUI::GetIntroScreen()
{ return m_intro_screen; }

void ClientUI::ShowIntroScreen() {
    if (m_map_wnd) {
        HumanClientApp::GetApp()->Remove(m_map_wnd);
        m_map_wnd->RemoveWindows();
        m_map_wnd->Hide();
    }

    // Update intro screen Load & Continue buttons if all savegames are deleted.
    m_intro_screen->RequirePreRender();

    HumanClientApp::GetApp()->Register(m_intro_screen);
    HumanClientApp::GetApp()->Remove(m_message_wnd);
    HumanClientApp::GetApp()->Remove(m_player_list_wnd);
    HumanClientApp::GetApp()->Remove(m_multiplayer_lobby_wnd);
}

void ClientUI::ShowMultiPlayerLobbyWnd() {
    if (m_map_wnd) {
        HumanClientApp::GetApp()->Remove(m_map_wnd);
        m_map_wnd->RemoveWindows();
        m_map_wnd->Hide();
    }

    HumanClientApp::GetApp()->Register(m_multiplayer_lobby_wnd);
    HumanClientApp::GetApp()->Remove(m_message_wnd);
    HumanClientApp::GetApp()->Remove(m_player_list_wnd);
    HumanClientApp::GetApp()->Remove(m_intro_screen);
}

std::shared_ptr<MultiPlayerLobbyWnd> ClientUI::GetMultiPlayerLobbyWnd()
{ return m_multiplayer_lobby_wnd; }

std::shared_ptr<PasswordEnterWnd> ClientUI::GetPasswordEnterWnd()
{ return m_password_enter_wnd; }


std::shared_ptr<SaveFileDialog> ClientUI::GetSaveFileDialog()
{ return m_savefile_dialog; }

std::string ClientUI::GetFilenameWithSaveFileDialog(
    const SaveFileDialog::Purpose purpose, const SaveFileDialog::SaveType type)
{
    // There can only be a single savefile_dialog at a time, becauase it is for
    // a specific purpose.
    if (m_savefile_dialog)
        return "";

    m_savefile_dialog = GG::Wnd::Create<SaveFileDialog>(purpose, type);

    m_savefile_dialog->Run();
    auto filename = m_savefile_dialog->Result();

    m_savefile_dialog = nullptr;
    return filename;
}

void ClientUI::GetSaveGameUIData(SaveGameUIData& data) const {
    GetMapWndConst()->GetSaveGameUIData(data);
    m_ship_designs->Save(data);
}

std::string ClientUI::FormatTimestamp(boost::posix_time::ptime timestamp) {
    TraceLogger() << "ClientUI::FormatTimestamp(" << timestamp << ")";
    if (DisplayTimestamp()) {
        std::stringstream date_format_sstream;
        // Set facet to format timestamp in chat.
        static auto facet = new boost::posix_time::time_facet("[%d %b %H:%M:%S] ");
        static std::locale dt_locale(GetLocale(), facet);
        TraceLogger() << "ClientUI::FormatTimestamp locale: " << dt_locale.name();
        date_format_sstream.str("");
        date_format_sstream.clear();
        date_format_sstream.imbue(dt_locale);
        // Determine local time from provided UTC timestamp
        auto local_timestamp = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(timestamp);
        date_format_sstream << local_timestamp;
        TraceLogger() << "ClientUI::FormatTimestamp date formatted: " << date_format_sstream.str()
                      << " Valid utf8?: " << (IsValidUTF8(date_format_sstream.str()) ? "yes" : "no");

        return date_format_sstream.str();
    }
    return "";
}

bool ClientUI::ZoomToObject(const std::string& name) {
    for (auto& obj : GetUniverse().Objects().FindObjects<UniverseObject>())
        if (boost::iequals(obj->Name(), name))
            return ZoomToObject(obj->ID());
    return false;
}

bool ClientUI::ZoomToObject(int id) {
    return ZoomToSystem(id)     || ZoomToPlanet(id) || ZoomToBuilding(id)   ||
           ZoomToFleet(id)      || ZoomToShip(id);
}

bool ClientUI::ZoomToPlanet(int id) {
    if (auto planet = GetPlanet(id)) {
        GetMapWnd()->CenterOnObject(planet->SystemID());
        GetMapWnd()->SelectSystem(planet->SystemID());
        GetMapWnd()->SelectPlanet(id);
        return true;
    }
    return false;
}

bool ClientUI::ZoomToPlanetPedia(int id) {
    if (GetPlanet(id))
        GetMapWnd()->ShowPlanet(id);
    return false;
}

bool ClientUI::ZoomToSystem(int id) {
    if (auto system = GetSystem(id)) {
        ZoomToSystem(system);
        return true;
    }
    return false;
}

bool ClientUI::ZoomToFleet(int id) {
    if (auto fleet = GetFleet(id)) {
        ZoomToFleet(fleet);
        return true;
    }
    return false;
}

bool ClientUI::ZoomToShip(int id) {
    if (auto ship = GetShip(id))
        return ZoomToFleet(ship->FleetID());
    return false;
}

bool ClientUI::ZoomToBuilding(int id) {
    if (auto building = GetBuilding(id)) {
        ZoomToBuildingType(building->BuildingTypeName());
        return ZoomToPlanet(building->PlanetID());
    }
    return false;
}

bool ClientUI::ZoomToField(int id) {
    //if (auto field = GetField(id)) {
    //  // TODO: implement this
    //}
    return false;
}

bool ClientUI::ZoomToCombatLog(int id) {
    if (GetCombatLogManager().GetLog(id)) {
        GetMapWnd()->ShowCombatLog(id);
        return true;
    }
    return false;
}

void ClientUI::ZoomToSystem(std::shared_ptr<const System> system) {
    if (!system)
        return;

    GetMapWnd()->CenterOnObject(system->ID());
    GetMapWnd()->SelectSystem(system->ID());
}

void ClientUI::ZoomToFleet(std::shared_ptr<const Fleet> fleet) {
    if (!fleet)
        return;

    GetMapWnd()->CenterOnObject(fleet->ID());
    GetMapWnd()->SelectFleet(fleet->ID());
    if (const auto& fleet_wnd = FleetUIManager::GetFleetUIManager().WndForFleetID(fleet->ID()))
        fleet_wnd->SelectFleet(fleet->ID());
}

bool ClientUI::ZoomToContent(const std::string& name, bool reverse_lookup/* = false*/) {
    if (reverse_lookup) {
        for (const auto& tech : GetTechManager()) {
            if (boost::iequals(name, UserString(tech->Name())))
                return ZoomToTech(tech->Name());
        }

        for (const auto& entry : GetBuildingTypeManager())
            if (boost::iequals(name, UserString(entry.first)))
                return ZoomToBuildingType(entry.first);

        for (const auto& special_name : SpecialNames())
            if (boost::iequals(name, UserString(special_name)))
                return ZoomToSpecial(special_name);

        for (const auto& entry : GetHullTypeManager())
            if (boost::iequals(name, UserString(entry.first)))
                return ZoomToShipHull(entry.first);

        for (const auto& entry : GetPartTypeManager())
            if (boost::iequals(name, UserString(entry.first)))
                return ZoomToShipPart(entry.first);

        for (const auto& entry : GetSpeciesManager())
            if (boost::iequals(name, UserString(entry.first)))
                return ZoomToSpecies(entry.first);

        return false;
    } else {
        // attempt to zoom to named content
        bool success =  ZoomToTech(name)     || ZoomToBuildingType(name) || ZoomToSpecial(name) ||
                        ZoomToShipHull(name) || ZoomToShipPart(name)     || ZoomToSpecies(name);
        if (success)
            return true;
        // attempt to find a shipdesign with this name
        // attempt to find empire with this name
        return false;
    }
}

bool ClientUI::ZoomToTech(const std::string& tech_name) {
    if (!GetTech(tech_name))
        return false;
    GetMapWnd()->ShowTech(tech_name);
    return true;
}

bool ClientUI::ZoomToBuildingType(const std::string& building_type_name) {
    if (!GetBuildingType(building_type_name))
        return false;
    GetMapWnd()->ShowBuildingType(building_type_name);
    return true;
}

bool ClientUI::ZoomToSpecial(const std::string& special_name) {
    if (!GetSpecial(special_name))
        return false;
    GetMapWnd()->ShowSpecial(special_name);
    return true;
}

bool ClientUI::ZoomToShipHull(const std::string& hull_name) {
    if (!GetHullType(hull_name))
        return false;
    GetMapWnd()->ShowHullType(hull_name);
    return true;
}

bool ClientUI::ZoomToShipPart(const std::string& part_name) {
    if (!GetPartType(part_name))
        return false;
    GetMapWnd()->ShowPartType(part_name);
    return true;
}

bool ClientUI::ZoomToSpecies(const std::string& species_name) {
    if (!GetSpecies(species_name))
        return false;
    GetMapWnd()->ShowSpecies(species_name);
    return true;
}

bool ClientUI::ZoomToFieldType(const std::string& field_type_name) {
    if (!GetFieldType(field_type_name))
        return false;
    GetMapWnd()->ShowFieldType(field_type_name);
    return true;
}

bool ClientUI::ZoomToShipDesign(int design_id) {
    if (!GetShipDesign(design_id))
        return false;
    GetMapWnd()->ShowShipDesign(design_id);
    return true;
}

bool ClientUI::ZoomToEmpire(int empire_id) {
    if (!GetEmpire(empire_id))
        return false;
    GetMapWnd()->ShowEmpire(empire_id);
    return true;
}

bool ClientUI::ZoomToMeterTypeArticle(const std::string& meter_string) {
    GetMapWnd()->ShowMeterTypeArticle(meter_string);
    return true;
}

bool ClientUI::ZoomToEncyclopediaEntry(const std::string& str) {
    GetMapWnd()->ShowEncyclopediaEntry(str);
    return true;
}

void ClientUI::DumpObject(int object_id) {
    auto obj = GetUniverseObject(object_id);
    if (!obj)
        return;
    m_message_wnd->HandleLogMessage(obj->Dump() + "\n");
}

void ClientUI::InitializeWindows() {
    const GG::Pt message_ul(GG::X0, GG::GUI::GetGUI()->AppHeight() - PANEL_HEIGHT);
    const GG::Pt message_wh(MESSAGE_PANEL_WIDTH, PANEL_HEIGHT);

    const GG::Pt player_list_ul(MESSAGE_PANEL_WIDTH, GG::GUI::GetGUI()->AppHeight() - PANEL_HEIGHT);
    const GG::Pt player_list_wh(PLAYER_LIST_PANEL_WIDTH, PANEL_HEIGHT);

    m_message_wnd->    InitSizeMove(message_ul,     message_ul + message_wh);
    m_player_list_wnd->InitSizeMove(player_list_ul, player_list_ul + player_list_wh);
}

void ClientUI::HandleSizeChange(bool fullscreen) const {
    OptionsDB& db = GetOptionsDB();

    if (db.Get<bool>("ui.reposition.auto.enabled")) {
        std::string window_mode = fullscreen ? ".fullscreen" : ".windowed";
        std::string option_name = "ui." + MESSAGE_WND_NAME + window_mode + ".left";

        // Invalidate the message window position so that we know to
        // recalculate positions on the next resize or fullscreen switch...
        db.Set<int>(option_name, db.GetDefault<int>(option_name));
    }
}

void ClientUI::HandleFullscreenSwitch() const {
    OptionsDB& db = GetOptionsDB();

    std::string window_mode = db.Get<bool>("video.fullscreen.enabled") ? ".fullscreen" : ".windowed";

    // Check if the message window position has been invalidated as a stand-in
    // for actually checking if all windows have been given valid positions for
    // this video mode... (the default value is
    // std::numeric_limits<GG::X::value_type>::min(), defined in UI/CUIWnd.cpp).
    // This relies on the message window not supplying a default position to
    // the CUIWnd constructor...
    std::string option_name = "ui." + MESSAGE_WND_NAME + window_mode + ".left";
    if (db.Get<int>(option_name) == db.GetDefault<int>(option_name)) {
        HumanClientApp::GetApp()->RepositionWindowsSignal();
    }
}

std::shared_ptr<GG::Texture> ClientUI::GetRandomTexture(const boost::filesystem::path& dir,
                                                        const std::string& prefix, bool mipmap/* = false*/)
{
    TexturesAndDist prefixed_textures_and_dist = PrefixedTexturesAndDist(dir, prefix, mipmap);
    return prefixed_textures_and_dist.first[(*prefixed_textures_and_dist.second)()];
}

std::shared_ptr<GG::Texture> ClientUI::GetModuloTexture(const boost::filesystem::path& dir,
                                                        const std::string& prefix, int n, bool mipmap/* = false*/)
{
    assert(0 <= n);
    TexturesAndDist prefixed_textures_and_dist = PrefixedTexturesAndDist(dir, prefix, mipmap);
    return prefixed_textures_and_dist.first.empty() ?
        nullptr :
        prefixed_textures_and_dist.first[n % prefixed_textures_and_dist.first.size()];
}

std::vector<std::shared_ptr<GG::Texture>> ClientUI::GetPrefixedTextures(const boost::filesystem::path& dir,
                                                                        const std::string& prefix, bool mipmap/* = false*/)
{
    TexturesAndDist prefixed_textures_and_dist = PrefixedTexturesAndDist(dir, prefix, mipmap);
    return prefixed_textures_and_dist.first;
}

void ClientUI::RestoreFromSaveData(const SaveGameUIData& ui_data) {
    GetMapWnd()->RestoreFromSaveData(ui_data);
    m_ship_designs->Load(ui_data);
}

ClientUI* ClientUI::GetClientUI()
{ return s_the_UI; }

void ClientUI::MessageBox(const std::string& message, bool play_alert_sound/* = false*/) {
    auto dlg = GG::GUI::GetGUI()->GetStyleFactory()->NewThreeButtonDlg(
        GG::X(320), GG::Y(200), message, GetFont(Pts()+2),
        WndColor(), WndOuterBorderColor(), CtrlColor(), TextColor(),
        1, UserString("OK"));
    if (play_alert_sound)
        Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.alert.sound.path"), true);
    dlg->Run();
}

std::shared_ptr<GG::Texture> ClientUI::GetTexture(const boost::filesystem::path& path, bool mipmap/* = false*/) {
    std::shared_ptr<GG::Texture> retval;
    try {
        retval = HumanClientApp::GetApp()->GetTexture(path, mipmap);
    } catch(const std::exception& e) {
        ErrorLogger() << "Unable to load texture \"" + path.generic_string() + "\"\n"
            "reason: " << e.what();
        retval = HumanClientApp::GetApp()->GetTexture(ClientUI::ArtDir() / "misc" / "missing.png", mipmap);
    }
#ifdef FREEORION_MACOSX
    if (!mipmap)
        retval->SetFilters(GL_LINEAR, GL_LINEAR);
#endif
    return retval;
}

std::shared_ptr<GG::Font> ClientUI::GetFont(int pts/* = Pts()*/) {
     try {
        return GG::GUI::GetGUI()->GetFont(GetOptionsDB().Get<std::string>("ui.font.path"), pts, RequiredCharsets().begin(), RequiredCharsets().end());
     } catch (...) {
         try {
            return GG::GUI::GetGUI()->GetFont(GetOptionsDB().GetDefault<std::string>("ui.font.path"),
                                              pts, RequiredCharsets().begin(), RequiredCharsets().end());
        } catch (...) {
             return GG::GUI::GetGUI()->GetStyleFactory()->DefaultFont(pts);
        }
    }
}

std::shared_ptr<GG::Font> ClientUI::GetBoldFont(int pts/* = Pts()*/) {
    try {
        return GG::GUI::GetGUI()->GetFont(GetOptionsDB().Get<std::string>("ui.font.bold.path"), pts, RequiredCharsets().begin(), RequiredCharsets().end());
    } catch (...) {
        try {
             return GG::GUI::GetGUI()->GetFont(GetOptionsDB().GetDefault<std::string>("ui.font.bold.path"),
                                               pts, RequiredCharsets().begin(), RequiredCharsets().end());
        } catch (...) {
             return GG::GUI::GetGUI()->GetStyleFactory()->DefaultFont(pts);
        }
    }
}

std::shared_ptr<GG::Font> ClientUI::GetTitleFont(int pts/* = TitlePts()*/) {
    try {
        return GG::GUI::GetGUI()->GetFont(GetOptionsDB().Get<std::string>("ui.font.title.path"), pts, RequiredCharsets().begin(), RequiredCharsets().end());
    } catch (...) {
        try {
            return GG::GUI::GetGUI()->GetFont(GetOptionsDB().GetDefault<std::string>("ui.font.title.path"),
                                              pts, RequiredCharsets().begin(), RequiredCharsets().end());
        } catch (...) {
             return GG::GUI::GetGUI()->GetStyleFactory()->DefaultFont(pts);
        }
   }
}

ClientUI::TexturesAndDist ClientUI::PrefixedTexturesAndDist(const boost::filesystem::path& dir,
                                                            const std::string& prefix, bool mipmap)
{
    namespace fs = boost::filesystem;
    assert(fs::is_directory(dir));
    const std::string KEY = dir.string() + "/" + prefix;
    auto prefixed_textures_it = m_prefixed_textures.find(KEY);
    if (prefixed_textures_it == m_prefixed_textures.end()) {
        prefixed_textures_it = m_prefixed_textures.insert({KEY, TexturesAndDist()}).first;
        auto& textures = prefixed_textures_it->second.first;
        auto& rand_int = prefixed_textures_it->second.second;
        fs::directory_iterator end_it;
        for (fs::directory_iterator it(dir); it != end_it; ++it) {
            try {
                if (fs::exists(*it) && !fs::is_directory(*it) && boost::algorithm::starts_with(it->path().filename().string(), prefix))
                    textures.push_back(ClientUI::GetTexture(*it, mipmap));
            } catch (const fs::filesystem_error& e) {
                // ignore files for which permission is denied, and rethrow other exceptions
                if (e.code() != boost::system::errc::permission_denied)
                    throw;
            }
        }
        rand_int.reset(new SmallIntDistType(SmallIntDist(0, textures.size() - 1)));
        std::sort(textures.begin(), textures.end(), TextureFileNameCompare);
    }
    return prefixed_textures_it->second;
}

int FontBasedUpscale(int x) {
    int retval(x);
    int font_pts = ClientUI::Pts();
    if (font_pts > 12) {
        retval *= static_cast<float>(font_pts) / 12.0f;
    }
    return retval;
}

namespace GG {
    std::istream& operator>>(std::istream& is, Clr& clr) {
        namespace classic = boost::spirit::classic;
        using classic::space_p;
        using classic::int_p;
        using classic::assign;
        classic::rule<> color_p =
            classic::ch_p('(') >> *space_p >>
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
        if (!classic::parse(str.c_str(), color_p).full ||
            clr.r < 0 || 255 < clr.r ||
            clr.g < 0 || 255 < clr.g ||
            clr.b < 0 || 255 < clr.b ||
            clr.a < 0 || 255 < clr.a)
            is.setstate(std::ios_base::failbit);
        return is;
    }
}
