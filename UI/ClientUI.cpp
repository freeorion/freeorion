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
#include "../universe/BuildingType.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/ShipPart.h"
#include "../universe/ShipHull.h"
#include "../universe/Tech.h"
#include "../universe/Special.h"
#include "../universe/Species.h"
#include "../universe/FieldType.h"
#include "../universe/Enums.h"
#include "../Empire/Government.h"
#include "../combat/CombatLogManager.h"
#include "../client/human/GGHumanClientApp.h"

#include <GG/Clr.h>
#include <GG/utf8/checked.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/GUI.h>
#include <GG/RichText/ImageBlock.h>
#include <GG/UnicodeCharsets.h>

#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix/operator.hpp>

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

#if __has_include(<charconv>)
#include <charconv>
#else
#include <stdio.h>
#endif

bool TextureFileNameCompare(const std::shared_ptr<GG::Texture>& t1,
                            const std::shared_ptr<GG::Texture>& t2)
{ return t1 && t2 && t1->Path() < t2->Path(); }

namespace fs = boost::filesystem;


fs::path    ClientUI::ArtDir()                  { return GetResourceDir() / "data" / "art"; }
fs::path    ClientUI::SoundDir()                { return GetResourceDir() / "data" / "sound"; }

int         ClientUI::Pts()                     { return GetOptionsDB().Get<int>("ui.font.size"); }
int         ClientUI::TitlePts()                { return GetOptionsDB().Get<int>("ui.font.title.size"); }

GG::Clr     ClientUI::TextColor()               { return GetOptionsDB().Get<GG::Clr>("ui.font.color"); }
GG::Clr     ClientUI::DefaultLinkColor()        { return GetOptionsDB().Get<GG::Clr>("ui.font.link.color"); }
GG::Clr     ClientUI::RolloverLinkColor()       { return GetOptionsDB().Get<GG::Clr>("ui.font.link.rollover.color"); }
GG::Clr     ClientUI::DefaultTooltipColor()     { return GetOptionsDB().Get<GG::Clr>("ui.font.tooltip.color"); }
GG::Clr     ClientUI::RolloverTooltipColor()    { return GetOptionsDB().Get<GG::Clr>("ui.font.tooltip.rollover.color"); }


GG::Clr     ClientUI::WndColor()                { return GetOptionsDB().Get<GG::Clr>("ui.window.background.color"); }
GG::Clr     ClientUI::WndOuterBorderColor()     { return GetOptionsDB().Get<GG::Clr>("ui.window.border.outer.color"); }
GG::Clr     ClientUI::WndInnerBorderColor()     { return GetOptionsDB().Get<GG::Clr>("ui.window.border.inner.color"); }


GG::Clr     ClientUI::CtrlColor()               { return GetOptionsDB().Get<GG::Clr>("ui.control.background.color"); }
GG::Clr     ClientUI::CtrlBorderColor()         { return GetOptionsDB().Get<GG::Clr>("ui.control.border.color"); }
GG::Clr     ClientUI::ButtonHiliteColor()       { return AdjustBrightness(CtrlColor(), 50); }
GG::Clr     ClientUI::ButtonHiliteBorderColor() { return AdjustBrightness(CtrlBorderColor(), 50); }

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
double      ClientUI::BigFleetButtonZoomThreshold()     { return GetOptionsDB().Get<double>("ui.map.fleet.button.big.zoom.threshold"); }

bool        ClientUI::DisplayTimestamp()                { return GetOptionsDB().Get<bool>("ui.map.messages.timestamp.shown"); }


std::shared_ptr<GG::Texture> ClientUI::PlanetIcon(PlanetType planet_type) {
    std::string_view icon_filename = "";
    switch (planet_type) {
    case PlanetType::PT_SWAMP:
        icon_filename = "swamp.png";    break;
    case PlanetType::PT_TOXIC:
        icon_filename = "toxic.png";    break;
    case PlanetType::PT_INFERNO:
        icon_filename = "inferno.png";  break;
    case PlanetType::PT_RADIATED:
        icon_filename = "radiated.png"; break;
    case PlanetType::PT_BARREN:
        icon_filename = "barren.png";   break;
    case PlanetType::PT_TUNDRA:
        icon_filename = "tundra.png";   break;
    case PlanetType::PT_DESERT:
        icon_filename = "desert.png";   break;
    case PlanetType::PT_TERRAN:
        icon_filename = "terran.png";   break;
    case PlanetType::PT_OCEAN:
        icon_filename = "ocean.png";    break;
    case PlanetType::PT_ASTEROIDS:
        icon_filename = "asteroids.png";break;
    case PlanetType::PT_GASGIANT:
        icon_filename = "gasgiant.png"; break;
    default:
        break;
    }
    return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "planet" / icon_filename.data(), true);
}

std::shared_ptr<GG::Texture> ClientUI::PlanetSizeIcon(PlanetSize planet_size) {
    std::string_view icon_filename = "";
    switch (planet_size) {
    case PlanetSize::SZ_TINY:
        icon_filename = "tiny.png";    break;
    case PlanetSize::SZ_SMALL:
        icon_filename = "small.png";    break;
    case PlanetSize::SZ_MEDIUM:
        icon_filename = "medium.png";  break;
    case PlanetSize::SZ_LARGE:
        icon_filename = "large.png"; break;
    case PlanetSize::SZ_HUGE:
        icon_filename = "huge.png";   break;
    case PlanetSize::SZ_ASTEROIDS:
        icon_filename = "asteroids.png";   break;
    case PlanetSize::SZ_GASGIANT:
        icon_filename = "gasgiant.png";   break;
    default:
        break;
    }
    return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "planet" / icon_filename.data(), true);
}

std::shared_ptr<GG::Texture> ClientUI::MeterIcon(MeterType meter_type) {
    std::string_view icon_filename = "";
    switch (meter_type) {
    case MeterType::METER_POPULATION:
    case MeterType::METER_TARGET_POPULATION:
        icon_filename = "pop.png";          break;
    case MeterType::METER_INDUSTRY:
    case MeterType::METER_TARGET_INDUSTRY:
        icon_filename = "industry.png";     break;
    case MeterType::METER_RESEARCH:
    case MeterType::METER_TARGET_RESEARCH:
        icon_filename = "research.png";     break;
    case MeterType::METER_INFLUENCE:
    case MeterType::METER_TARGET_INFLUENCE:
        icon_filename = "influence.png";    break;
    case MeterType::METER_CONSTRUCTION:
    case MeterType::METER_TARGET_CONSTRUCTION:
        icon_filename = "construction.png"; break;
    case MeterType::METER_HAPPINESS:
    case MeterType::METER_TARGET_HAPPINESS:
        icon_filename = "happiness.png";    break;
    case MeterType::METER_CAPACITY:
    case MeterType::METER_MAX_CAPACITY:
        icon_filename = "capacity.png";     break;
    case MeterType::METER_SECONDARY_STAT:
    case MeterType::METER_MAX_SECONDARY_STAT:
        icon_filename = "secondary.png";    break;
    case MeterType::METER_STRUCTURE:
    case MeterType::METER_MAX_STRUCTURE:
        icon_filename = "structure.png";    break;
    case MeterType::METER_FUEL:
    case MeterType::METER_MAX_FUEL:
        icon_filename = "fuel.png";         break;
    case MeterType::METER_SUPPLY:
    case MeterType::METER_MAX_SUPPLY:
        icon_filename = "supply.png";       break;
    case MeterType::METER_STOCKPILE:
    case MeterType::METER_MAX_STOCKPILE:
        icon_filename = "stockpile.png";    break;
    case MeterType::METER_STEALTH:
        icon_filename = "stealth.png";      break;
    case MeterType::METER_DETECTION:
        icon_filename = "detection.png";    break;
    case MeterType::METER_SHIELD:
    case MeterType::METER_MAX_SHIELD:
        icon_filename = "shield.png";       break;
    case MeterType::METER_DEFENSE:
    case MeterType::METER_MAX_DEFENSE:
        icon_filename = "defense.png";      break;
    case MeterType::METER_TROOPS:
    case MeterType::METER_MAX_TROOPS:
        icon_filename = "troops.png";       break;
    case MeterType::METER_REBEL_TROOPS:
        icon_filename = "rebels.png";       break;
    case MeterType::METER_SPEED:
        icon_filename = "speed.png";        break;
    default:
        return ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "missing.png", true); break;
    }
    return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / icon_filename.data(), true);
}

std::shared_ptr<GG::Texture> ClientUI::BuildingIcon(std::string_view building_type_name) {
    const BuildingType* building_type = GetBuildingType(building_type_name);
    std::string_view graphic_name = "";
    if (building_type)
        graphic_name = building_type->Icon();
    if (graphic_name.empty())
        return ClientUI::GetTexture(ArtDir() / "icons" / "building" / "generic_building.png", true);
    return ClientUI::GetTexture(ArtDir() / graphic_name.data(), true);
}

std::shared_ptr<GG::Texture> ClientUI::CategoryIcon(std::string_view category_name) {
    if (const TechCategory* category = GetTechCategory(category_name))
        return ClientUI::GetTexture(ArtDir() / "icons" / "tech" / "categories" / category->graphic, true);
    else
        return ClientUI::GetTexture(ClientUI::ArtDir() / "", true);
}

std::shared_ptr<GG::Texture> ClientUI::TechIcon(std::string_view tech_name) {
    const Tech* tech = GetTechManager().GetTech(tech_name);
    std::string_view texture_name = "";
    if (tech) {
        texture_name = tech->Graphic();
        if (texture_name.empty())
            return CategoryIcon(tech->Category());
    }
    return ClientUI::GetTexture(ArtDir() / texture_name.data(), true);
}

std::shared_ptr<GG::Texture> ClientUI::PolicyIcon(std::string_view policy_name) {
    const Policy* policy = GetPolicyManager().GetPolicy(policy_name);
    std::string_view texture_name = "";
    if (policy)
        texture_name = policy->Graphic();
    return ClientUI::GetTexture(ArtDir() / texture_name.data(), true);
}

std::shared_ptr<GG::Texture> ClientUI::SpecialIcon(std::string_view special_name) {
    const Special* special = GetSpecial(special_name);
    std::string_view texture_name = "";
    if (special)
        texture_name = special->Graphic();
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "icons" / "specials_huge" / "generic_special.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name.data(), true);
}

std::shared_ptr<GG::Texture> ClientUI::SpeciesIcon(std::string_view species_name) {
    std::string_view texture_name = "";
    if (const Species* species = ClientApp::GetApp()->GetSpeciesManager().GetSpecies(species_name))
        texture_name = species->Graphic();
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "icons" / "meter" / "pop.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name.data(), true);
}

std::shared_ptr<GG::Texture> ClientUI::FieldTexture(std::string_view field_type_name) {
    const FieldType* type = GetFieldType(field_type_name);
    std::string_view texture_name = "";
    if (type)
        texture_name = type->Graphic();
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "fields" / "ion_storm.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name.data(), true);
}

std::shared_ptr<GG::Texture> ClientUI::PartIcon(std::string_view part_name) {
    const ShipPart* part = GetShipPart(part_name);
    std::string_view texture_name = "";
    if (part)
        texture_name = part->Icon();
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "icons" / "ship_parts" / "generic_part.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name.data(), false);
}

std::shared_ptr<GG::Texture> ClientUI::HullTexture(std::string_view hull_name) {
    const ShipHull* hull = GetShipHull(hull_name);
    std::string_view texture_name = "";
    if (hull) {
        texture_name = hull->Graphic();
        if (texture_name.empty())
            texture_name = hull->Icon();
    }
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "hulls_design" / "generic_hull.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name.data(), true);
}

std::shared_ptr<GG::Texture> ClientUI::HullIcon(std::string_view hull_name) {
    const ShipHull* hull = GetShipHull(hull_name);
    std::string_view texture_name = "";
    if (hull) {
        texture_name = hull->Icon();
        if (texture_name.empty())
            texture_name = hull->Graphic();
    }
    if (texture_name.empty())
        return ClientUI::GetTexture(ArtDir() / "icons" / "ship_hulls"/ "generic_hull.png", true);
    return ClientUI::GetTexture(ArtDir() / texture_name.data(), true);
}

std::shared_ptr<GG::Texture> ClientUI::ShipDesignIcon(int design_id) {
    const ScriptingContext& context = IApp::GetApp()->GetContext();
    if (const ShipDesign* design = context.ContextUniverse().GetShipDesign(design_id)) {
        std::string_view icon_name = design->Icon();
        if (icon_name.empty())
            return ClientUI::HullIcon(design->Hull());
        else
            return ClientUI::GetTexture(ArtDir() / icon_name.data(), true);
    }
    return ClientUI::HullTexture("");
}


GG::Clr ClientUI::KnownTechFillColor()                   { return GetOptionsDB().Get<GG::Clr>("ui.research.status.completed.background.color"); }
GG::Clr ClientUI::KnownTechTextAndBorderColor()          { return GetOptionsDB().Get<GG::Clr>("ui.research.status.completed.border.color"); }
GG::Clr ClientUI::ResearchableTechFillColor()            { return GetOptionsDB().Get<GG::Clr>("ui.research.status.researchable.background.color"); }
GG::Clr ClientUI::ResearchableTechTextAndBorderColor()   { return GetOptionsDB().Get<GG::Clr>("ui.research.status.researchable.border.color"); }
GG::Clr ClientUI::UnresearchableTechFillColor()          { return GetOptionsDB().Get<GG::Clr>("ui.research.status.unresearchable.background.color"); }
GG::Clr ClientUI::UnresearchableTechTextAndBorderColor() { return GetOptionsDB().Get<GG::Clr>("ui.research.status.unresearchable.border.color"); }
GG::Clr ClientUI::TechWndProgressBarBackgroundColor()    { return GetOptionsDB().Get<GG::Clr>("ui.research.status.progress.background.color"); }
GG::Clr ClientUI::TechWndProgressBarColor()              { return GetOptionsDB().Get<GG::Clr>("ui.research.status.progress.color"); }

GG::Clr ClientUI::CategoryColor(std::string_view category_name) {
    if (auto category = GetTechCategory(category_name))
        return category->colour;
    return {};
}

namespace {
    static_assert(static_cast<int>(PlanetType::INVALID_PLANET_TYPE) + 1 == 0);
    static_assert(static_cast<int>(PlanetType::NUM_PLANET_TYPES) == 11);
    constexpr std::array<std::string_view, 13> planet_prefixes {
        "", "Swamp", "Toxic", "Inferno","Radiated", "Barren",
        "Tundra", "Desert", "Terran", "Ocean", "", "GasGiant", ""};
    constexpr auto PlanetPrefix(PlanetType star_type)
    { return planet_prefixes[static_cast<size_t>(static_cast<int>(star_type)+1)]; }
    static_assert(PlanetPrefix(PlanetType::PT_SWAMP) == "Swamp");
    static_assert(PlanetPrefix(PlanetType::PT_GASGIANT) == "GasGiant");

    static_assert(static_cast<int>(StarType::INVALID_STAR_TYPE) + 1 == 0);
    static_assert(static_cast<int>(StarType::NUM_STAR_TYPES) == 8);
    constexpr std::array<std::string_view, 10> star_prefixes{
        "unknown", "blue", "white", "yellow", "orange",
        "red", "neutron", "blackhole", "nostar", ""};
    constexpr auto StarPrefix(StarType star_type)
    { return star_prefixes[static_cast<size_t>(static_cast<int>(star_type)+1)]; }
    static_assert(StarPrefix(StarType::INVALID_STAR_TYPE) == "unknown");
    static_assert(StarPrefix(StarType::STAR_RED) == "red");
    static_assert(StarPrefix(StarType::NUM_STAR_TYPES).empty());

    constexpr std::array<std::string_view, 10> halo_prefixes{
        "halo_unknown", "halo_blue", "halo_white", "halo_yellow", "halo_orange",
        "halo_red", "halo_neutron", "halo_blackhole", "halo_nostar", "halo_unknown"};
    constexpr auto HaloPrefix(StarType star_type)
    { return halo_prefixes[static_cast<size_t>(static_cast<int>(star_type)+1)]; }
    static_assert(HaloPrefix(StarType::INVALID_STAR_TYPE) == "halo_unknown");
    static_assert(HaloPrefix(StarType::STAR_NONE) == "halo_nostar");
}

std::string_view ClientUI::PlanetTypeFilePrefix(PlanetType planet_type) noexcept
{ return PlanetPrefix(planet_type); }

std::string_view ClientUI::StarTypeFilePrefix(StarType star_type) noexcept
{ return StarPrefix(star_type); }

std::string_view ClientUI::HaloStarTypeFilePrefix(StarType star_type) noexcept
{ return HaloPrefix(star_type); }

constinit ClientUI* ClientUI::s_the_UI = nullptr;

std::ostream& operator<< (std::ostream& os, const GG::UnicodeCharset& chset) {
    os << chset.m_script_name << " " << chset.m_first_char << " " << chset.m_last_char << "\n";
    return os;
}

namespace {
    const auto& RequiredCharsets() {
        static const auto retval = []() {
            std::vector<GG::UnicodeCharset> retval;

            // Basic Latin, Latin-1 Supplement, and Latin Extended-A
            // (character sets needed to display the credits page)
            const std::string_view CREDITS_STR = "AöŁ";
            const auto credits_charsets = GG::UnicodeCharsetsToRender(CREDITS_STR);

            std::set<GG::UnicodeCharset> stringtable_charsets;
            {
                std::string file_name = GetOptionsDB().Get<std::string>("resource.stringtable.path");
                std::string stringtable_str;
                boost::filesystem::ifstream ifs(FilenameToPath(file_name));
                while (ifs) {
                    std::string line;
                    std::getline(ifs, line);
                    stringtable_str += line;
                    stringtable_str += '\n';
                }
                const auto stcs = GG::UnicodeCharsetsToRender(stringtable_str);
                stringtable_charsets.insert(stcs.begin(), stcs.end());
                DebugLogger() << "loading " << stringtable_charsets.size() << " charsets for current stringtable characters";
            }

            if (!GetOptionsDB().IsDefaultValue("resource.stringtable.path")) {
                DebugLogger() << "Non-default stringtable!";
                std::string file_name = GetOptionsDB().GetDefault<std::string>("resource.stringtable.path");
                std::string stringtable_str;
                boost::filesystem::ifstream ifs(FilenameToPath(file_name));
                while (ifs) {
                    std::string line;
                    std::getline(ifs, line);
                    stringtable_str += line;
                    stringtable_str += '\n';
                }
                const auto default_stringtable_charsets = GG::UnicodeCharsetsToRender(stringtable_str);
                DebugLogger() << "loading " << default_stringtable_charsets.size() << " charsets for default stringtable characters";

                stringtable_charsets.insert(default_stringtable_charsets.begin(), default_stringtable_charsets.end());
                DebugLogger() << "combined stringtable charsets have " << stringtable_charsets.size() << " charsets";
            }

            std::set_union(credits_charsets.begin(), credits_charsets.end(),
                           stringtable_charsets.begin(), stringtable_charsets.end(),
                           std::back_inserter(retval));

            std::string message_text = "Loading " + std::to_string(retval.size()) + " Unicode charsets: ";
            for (const GG::UnicodeCharset& cs : retval)
                message_text.append(cs.m_script_name).append(", ");

            DebugLogger() << message_text;

            return retval;
        }();
        return retval;
    }


    // command-line options
    void AddOptions(OptionsDB& db) {
        db.Add("video.fps.shown",                          UserStringNop("OPTIONS_DB_SHOW_FPS"),                       false);
        db.Add("video.fps.max.enabled",                    UserStringNop("OPTIONS_DB_LIMIT_FPS"),                      true);
        db.Add("video.fps.max",                            UserStringNop("OPTIONS_DB_MAX_FPS"),                        60.0,                           RangedStepValidator<double>(1.0, 0.0, 240.0));
        db.Add("video.fps.unfocused.enabled",              UserStringNop("OPTIONS_DB_LIMIT_FPS_NO_FOCUS"),             true);
        db.Add("video.fps.unfocused",                      UserStringNop("OPTIONS_DB_MAX_FPS_NO_FOCUS"),               15.0,                           RangedStepValidator<double>(0.125, 0.125, 30.0));

        // sound and music
        db.Add("audio.music.path",                         UserStringNop("OPTIONS_DB_BG_MUSIC"),                       (GetRootDataDir() / "default" / "data" / "sound" / "artificial_intelligence_v3.ogg").string());
        db.Add("audio.music.volume",                       UserStringNop("OPTIONS_DB_MUSIC_VOLUME"),                   127,                            RangedValidator<int>(1, 255));
        db.Add("audio.effects.volume",                     UserStringNop("OPTIONS_DB_UI_SOUND_VOLUME"),                255,                            RangedValidator<int>(0, 255));
        db.Add("ui.button.rollover.sound.path",            UserStringNop("OPTIONS_DB_UI_SOUND_BUTTON_ROLLOVER"),       (GetRootDataDir() / "default" / "data" / "sound" / "button_rollover.ogg").string());
        db.Add("ui.button.press.sound.path",               UserStringNop("OPTIONS_DB_UI_SOUND_BUTTON_CLICK"),          (GetRootDataDir() / "default" / "data" / "sound" / "button_click.ogg").string());
        db.Add("ui.button.turn.press.sound.path",          UserStringNop("OPTIONS_DB_UI_SOUND_TURN_BUTTON_CLICK"),     (GetRootDataDir() / "default" / "data" / "sound" / "turn_button_click.ogg").string());
        db.Add("ui.listbox.select.sound.path",             UserStringNop("OPTIONS_DB_UI_SOUND_LIST_SELECT"),           (GetRootDataDir() / "default" / "data" / "sound" / "list_select.ogg").string());
        db.Add("ui.listbox.drop.sound.path",               UserStringNop("OPTIONS_DB_UI_SOUND_ITEM_DROP"),             (GetRootDataDir() / "default" / "data" / "sound" / "list_select.ogg").string());//TODO: replace with dedicated 'item_drop' sound
        db.Add("ui.dropdownlist.select.sound.path",        UserStringNop("OPTIONS_DB_UI_SOUND_LIST_PULLDOWN"),         (GetRootDataDir() / "default" / "data" / "sound" / "list_pulldown.ogg").string());
        db.Add("ui.input.keyboard.sound.path",             UserStringNop("OPTIONS_DB_UI_SOUND_TEXT_TYPING"),           (GetRootDataDir() / "default" / "data" / "sound" / "text_typing.ogg").string());
        db.Add("ui.window.minimize.sound.path",            UserStringNop("OPTIONS_DB_UI_SOUND_WINDOW_MAXIMIZE"),       (GetRootDataDir() / "default" / "data" / "sound" / "window_maximize.ogg").string());
        db.Add("ui.window.maximize.sound.path",            UserStringNop("OPTIONS_DB_UI_SOUND_WINDOW_MINIMIZE"),       (GetRootDataDir() / "default" / "data" / "sound" / "window_minimize.ogg").string());
        db.Add("ui.window.close.sound.path",               UserStringNop("OPTIONS_DB_UI_SOUND_WINDOW_CLOSE"),          (GetRootDataDir() / "default" / "data" / "sound" / "window_close.ogg").string());
        db.Add("ui.alert.sound.path",                      UserStringNop("OPTIONS_DB_UI_SOUND_ALERT"),                 (GetRootDataDir() / "default" / "data" / "sound" / "alert.ogg").string());
        db.Add("ui.map.fleet.button.rollover.sound.path",  UserStringNop("OPTIONS_DB_UI_SOUND_FLEET_BUTTON_ROLLOVER"), (GetRootDataDir() / "default" / "data" / "sound" / "fleet_button_rollover.ogg").string());
        db.Add("ui.map.fleet.button.press.sound.path",     UserStringNop("OPTIONS_DB_UI_SOUND_FLEET_BUTTON_CLICK"),    (GetRootDataDir() / "default" / "data" / "sound" / "fleet_button_click.ogg").string());
        db.Add("ui.map.system.icon.rollover.sound.path",   UserStringNop("OPTIONS_DB_UI_SOUND_SYSTEM_ICON_ROLLOVER"),  (GetRootDataDir() / "default" / "data" / "sound" / "fleet_button_rollover.ogg").string());
        db.Add("ui.map.sidepanel.open.sound.path",         UserStringNop("OPTIONS_DB_UI_SOUND_SIDEPANEL_OPEN"),        (GetRootDataDir() / "default" / "data" / "sound" / "sidepanel_open.ogg").string());
        db.Add("ui.turn.start.sound.enabled",              UserStringNop("OPTIONS_DB_UI_SOUND_NEWTURN_TOGGLE"),        false);
        db.Add("ui.turn.start.sound.path",                 UserStringNop("OPTIONS_DB_UI_SOUND_NEWTURN_FILE"),          (GetRootDataDir() / "default" / "data" / "sound" / "newturn.ogg").string());

        // fonts
        db.Add("ui.font.path",                             UserStringNop("OPTIONS_DB_UI_FONT"),                        (GetRootDataDir() / "default/data/fonts/Roboto-Regular.ttf").string());
        db.Add("ui.font.bold.path",                        UserStringNop("OPTIONS_DB_UI_FONT_BOLD"),                   (GetRootDataDir() / "default" / "data" / "fonts" / "Roboto-Bold.ttf").string());
#ifdef FREEORION_MACOSX
        db.Add("ui.font.size",                             UserStringNop("OPTIONS_DB_UI_FONT_SIZE"),                   15,                             RangedValidator<int>(4, 40));
#else
        db.Add("ui.font.size",                             UserStringNop("OPTIONS_DB_UI_FONT_SIZE"),                   16,                             RangedValidator<int>(4, 40));
#endif
        db.Add("ui.font.title.path",                       UserStringNop("OPTIONS_DB_UI_TITLE_FONT"),                  (GetRootDataDir() / "default/data/fonts/Roboto-Regular.ttf").string());
#ifdef FREEORION_MACOSX
        db.Add("ui.font.title.size",                       UserStringNop("OPTIONS_DB_UI_TITLE_FONT_SIZE"),             16,                             RangedValidator<int>(4, 40));
#else
        db.Add("ui.font.title.size",                       UserStringNop("OPTIONS_DB_UI_TITLE_FONT_SIZE"),             17,                             RangedValidator<int>(4, 40));
#endif

        // colors
        db.Add("ui.window.background.color",               UserStringNop("OPTIONS_DB_UI_WND_COLOR"),                   GG::Clr(35, 35, 35, 240),       Validator<GG::Clr>());
        db.Add("ui.window.border.outer.color",             UserStringNop("OPTIONS_DB_UI_WND_OUTER_BORDER_COLOR"),      GG::Clr(64, 64, 64, 255),       Validator<GG::Clr>());
        db.Add("ui.window.border.inner.color",             UserStringNop("OPTIONS_DB_UI_WND_INNER_BORDER_COLOR"),      GG::Clr(192, 192, 192, 255),    Validator<GG::Clr>());

        db.Add("ui.control.background.color",              UserStringNop("OPTIONS_DB_UI_CTRL_COLOR"),                  GG::Clr(15, 15, 15, 255),       Validator<GG::Clr>());
        db.Add("ui.control.border.color",                  UserStringNop("OPTIONS_DB_UI_CTRL_BORDER_COLOR"),           GG::Clr(124, 124, 124, 255),    Validator<GG::Clr>());

        db.Add("ui.dropdownlist.arrow.color",              UserStringNop("OPTIONS_DB_UI_DROPDOWNLIST_ARROW_COLOR"),    GG::Clr(130, 130, 0, 255),      Validator<GG::Clr>());

        db.Add("ui.control.edit.highlight.color",          UserStringNop("OPTIONS_DB_UI_EDIT_HILITE"),                 GG::Clr(43, 81, 102, 255),      Validator<GG::Clr>());

        db.Add("ui.font.stat.increase.color",              UserStringNop("OPTIONS_DB_UI_STAT_INCREASE_COLOR"),         GG::Clr(0, 255, 0, 255),        Validator<GG::Clr>());
        db.Add("ui.font.stat.decrease.color",              UserStringNop("OPTIONS_DB_UI_STAT_DECREASE_COLOR"),         GG::Clr(255, 0, 0, 255),        Validator<GG::Clr>());

        db.Add("ui.button.state.color",                    UserStringNop("OPTIONS_DB_UI_STATE_BUTTON_COLOR"),          GG::Clr(0, 127, 0, 255),        Validator<GG::Clr>());

        db.Add("ui.font.color",                            UserStringNop("OPTIONS_DB_UI_TEXT_COLOR"),                  GG::Clr(255, 255, 255, 255),    Validator<GG::Clr>());
        db.Add("ui.font.link.color",                       UserStringNop("OPTIONS_DB_UI_DEFAULT_LINK_COLOR"),          GG::Clr(80, 255, 128, 255),     Validator<GG::Clr>());
        db.Add("ui.font.link.rollover.color",              UserStringNop("OPTIONS_DB_UI_ROLLOVER_LINK_COLOR"),         GG::Clr(192, 80, 255, 255),     Validator<GG::Clr>());
        db.Add("ui.font.tooltip.color",                    UserStringNop("OPTIONS_DB_UI_DEFAULT_TOOLTIP_COLOR"),       GG::Clr(180, 220, 200, 255),     Validator<GG::Clr>());
        db.Add("ui.font.tooltip.rollover.color",           UserStringNop("OPTIONS_DB_UI_ROLLOVER_TOOLTIP_COLOR"),      GG::Clr(200, 100, 255, 255),     Validator<GG::Clr>());

        db.Add("ui.research.status.completed.background.color",     UserStringNop("OPTIONS_DB_UI_KNOWN_TECH"),                  GG::Clr(72, 72, 72, 255),       Validator<GG::Clr>());
        db.Add("ui.research.status.completed.border.color",         UserStringNop("OPTIONS_DB_UI_KNOWN_TECH_BORDER"),           GG::Clr(164, 164, 164, 255),    Validator<GG::Clr>());
        db.Add("ui.research.status.researchable.background.color",  UserStringNop("OPTIONS_DB_UI_RESEARCHABLE_TECH"),           GG::Clr(48, 48, 48, 255),       Validator<GG::Clr>());
        db.Add("ui.research.status.researchable.border.color",      UserStringNop("OPTIONS_DB_UI_RESEARCHABLE_TECH_BORDER"),    GG::Clr(164, 164, 164, 255),    Validator<GG::Clr>());
        db.Add("ui.research.status.unresearchable.background.color",UserStringNop("OPTIONS_DB_UI_UNRESEARCHABLE_TECH"),         GG::Clr(30, 30, 30, 255),       Validator<GG::Clr>());
        db.Add("ui.research.status.unresearchable.border.color",    UserStringNop("OPTIONS_DB_UI_UNRESEARCHABLE_TECH_BORDER"),  GG::Clr(86, 86, 86, 255),       Validator<GG::Clr>());
        db.Add("ui.research.status.progress.background.color",      UserStringNop("OPTIONS_DB_UI_TECH_PROGRESS_BACKGROUND"),    GG::Clr(72, 72, 72, 255),       Validator<GG::Clr>());
        db.Add("ui.research.status.progress.color",                 UserStringNop("OPTIONS_DB_UI_TECH_PROGRESS"),               GG::Clr(40, 40, 40, 255),       Validator<GG::Clr>());

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

    constexpr GG::Y PANEL_HEIGHT{160};
    constexpr GG::X MESSAGE_PANEL_WIDTH{345};
    constexpr GG::X PLAYER_LIST_PANEL_WIDTH{445};

    const std::string MESSAGE_WND_NAME = "map.messages";
    const std::string PLAYER_LIST_WND_NAME = "map.empires";
}


////////////////////////////////////////////////
// ClientUI
////////////////////////////////////////////////
ClientUI::ClientUI() :
    m_ship_designs(std::make_unique<ShipDesignManager>())
{
    s_the_UI = this;
    Hotkey::ReadFromOptions(GetOptionsDB());

    // Remove all window properties if asked to
    if (GetOptionsDB().Get<bool>("window-reset"))
        CUIWnd::InvalidateUnusedOptions();

    InitializeWindows();

    GetOptionsDB().OptionChangedSignal("video.fullscreen.width").connect(
        boost::bind(&ClientUI::HandleSizeChange, this, true));
    GetOptionsDB().OptionChangedSignal("video.fullscreen.height").connect(
        boost::bind(&ClientUI::HandleSizeChange, this, true));
    GetOptionsDB().OptionChangedSignal("video.windowed.width").connect(
        boost::bind(&ClientUI::HandleSizeChange, this, false));
    GetOptionsDB().OptionChangedSignal("video.windowed.height").connect(
        boost::bind(&ClientUI::HandleSizeChange, this, false));
    GGHumanClientApp::GetApp()->RepositionWindowsSignal.connect(
        boost::bind(&ClientUI::InitializeWindows, this));
    GGHumanClientApp::GetApp()->RepositionWindowsSignal.connect(
        &CUIWnd::InvalidateUnusedOptions,
        boost::signals2::at_front);

    // Connected at front to make sure CUIWnd::LoadOptions() doesn't overwrite
    // the values we're checking here...
    GGHumanClientApp::GetApp()->FullscreenSwitchSignal.connect(
        boost::bind(&ClientUI::HandleFullscreenSwitch, this),
        boost::signals2::at_front);

    GetOptionsDB().OptionChangedSignal("ui.reposition.auto.enabled").connect(
        []() {
            if (GetOptionsDB().Get<bool>("ui.reposition.auto.enabled"))
                GGHumanClientApp::GetApp()->RepositionWindowsSignal();
        }
    );

    // Set the root path for image tags in rich text.
    GG::ImageBlock::SetDefaultImagePath(ArtDir().string());
}

ClientUI::~ClientUI()
{ s_the_UI = nullptr; }

MapWnd* ClientUI::GetMapWnd(bool construct) {
    if (!m_map_wnd && construct)
        m_map_wnd = GG::Wnd::Create<MapWnd>();
    return m_map_wnd.get();
}

std::shared_ptr<MapWnd> ClientUI::GetMapWndShared() {
    if (!m_map_wnd)
        m_map_wnd = GG::Wnd::Create<MapWnd>();
    return m_map_wnd;
}

std::shared_ptr<MessageWnd> ClientUI::GetMessageWnd() {
    if (!m_message_wnd)
        m_message_wnd = GG::Wnd::Create<MessageWnd>(GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | GG::RESIZABLE |
                                                    CLOSABLE | PINABLE, MESSAGE_WND_NAME);
    return m_message_wnd;
}

std::shared_ptr<PlayerListWnd> ClientUI::GetPlayerListWnd() {
    if (!m_player_list_wnd)
        m_player_list_wnd = GG::Wnd::Create<PlayerListWnd>(PLAYER_LIST_WND_NAME);
    return m_player_list_wnd;
}

std::shared_ptr<IntroScreen> ClientUI::GetIntroScreen() {
    if (!m_intro_screen)
        m_intro_screen = GG::Wnd::Create<IntroScreen>();
    return m_intro_screen;
}

void ClientUI::ShowIntroScreen() {
    if (m_map_wnd) {
        GGHumanClientApp::GetApp()->Remove(m_map_wnd);
        m_map_wnd->RemoveWindows();
        m_map_wnd->Hide();
    }

    // Update intro screen Load & Continue buttons if all savegames are deleted.
    GetIntroScreen()->RequirePreRender();

    GGHumanClientApp::GetApp()->Register(GetIntroScreen());
    GGHumanClientApp::GetApp()->Remove(m_message_wnd);
    GGHumanClientApp::GetApp()->Remove(m_player_list_wnd);
    GGHumanClientApp::GetApp()->Remove(m_multiplayer_lobby_wnd);
}

void ClientUI::ShowMultiPlayerLobbyWnd() {
    if (m_map_wnd) {
        GGHumanClientApp::GetApp()->Remove(m_map_wnd);
        m_map_wnd->RemoveWindows();
        m_map_wnd->Hide();
    }

    GGHumanClientApp::GetApp()->Register(GetMultiPlayerLobbyWnd());
    GGHumanClientApp::GetApp()->Remove(m_message_wnd);
    GGHumanClientApp::GetApp()->Remove(m_player_list_wnd);
    GGHumanClientApp::GetApp()->Remove(m_intro_screen);
}

std::shared_ptr<MultiPlayerLobbyWnd> ClientUI::GetMultiPlayerLobbyWnd() {
    if (!m_multiplayer_lobby_wnd)
        m_multiplayer_lobby_wnd = GG::Wnd::Create<MultiPlayerLobbyWnd>();
    return m_multiplayer_lobby_wnd;
}

std::shared_ptr<PasswordEnterWnd> ClientUI::GetPasswordEnterWnd() {
    if (!m_password_enter_wnd)
        m_password_enter_wnd = GG::Wnd::Create<PasswordEnterWnd>();
    return m_password_enter_wnd;
}

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
    if (!m_savefile_dialog)
        return "";

    m_savefile_dialog->Run();
    auto filename = m_savefile_dialog->ResultString();
    m_savefile_dialog.reset();
    return filename;
}

void ClientUI::GetSaveGameUIData(SaveGameUIData& data) {
    auto mapwnd = GetMapWnd(true);
    if (!mapwnd) {
        ErrorLogger() << "GetSaveGameUIData couldn't get mapwnd";
        return;
    }
    mapwnd->GetSaveGameUIData(data);

    if (m_ship_designs)
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
        auto date_format_str = date_format_sstream.str();
        TraceLogger() << "ClientUI::FormatTimestamp date formatted: " << date_format_str
                      << " Valid utf8?: " << (utf8::is_valid(date_format_str.begin(), date_format_str.end()) ? "yes" : "no");

        return date_format_str;
    }
    return "";
}

bool ClientUI::ZoomToObject(const std::string& name) {
    const ScriptingContext& context = IApp::GetApp()->GetContext();
    // try first by finding the object by name TODO: use getRaw or find or somesuch
    for (auto obj : context.ContextObjects().allRaw<UniverseObject>())
        if (boost::iequals(obj->Name(), name))
            return ZoomToObject(obj->ID());

    // try again by converting string to an ID
    try {
#if defined(__cpp_lib_to_chars)
        int id = INVALID_OBJECT_ID;
        std::from_chars(name.data(), name.data() + name.size(), id);
#else
        int id = boost::lexical_cast<int>(name);
#endif
        return ZoomToObject(id);
    } catch (...) {
    }

    return false;
}

bool ClientUI::ZoomToObject(int id) {
    return ZoomToSystem(id)     || ZoomToPlanet(id) || ZoomToBuilding(id)   ||
           ZoomToFleet(id)      || ZoomToShip(id)   || ZoomToField(id);
}

bool ClientUI::ZoomToPlanet(int id) {
    if (auto planet = ClientApp::GetApp()->GetContext().ContextObjects().get<Planet>(id)) {
        if (auto mapwnd = GetMapWnd(false)) {
            mapwnd->CenterOnObject(planet->SystemID());
            mapwnd->SelectSystem(planet->SystemID());
            mapwnd->SelectPlanet(id);
        }
        return true;
    }
    return false;
}

bool ClientUI::ZoomToPlanetPedia(int id) {
    if (ClientApp::GetApp()->GetContext().ContextObjects().get<Planet>(id))
        if (auto mapwnd = GetMapWnd(false))
            mapwnd->ShowPlanet(id);
    return false;
}

bool ClientUI::ZoomToSystem(int id) {
    if (auto system = ClientApp::GetApp()->GetContext().ContextObjects().get<System>(id)) {
        ZoomToSystem(system);
        return true;
    }
    return false;
}

bool ClientUI::ZoomToFleet(int id) {
    if (auto fleet = ClientApp::GetApp()->GetContext().ContextObjects().get<Fleet>(id)) {
        ZoomToFleet(fleet);
        return true;
    }
    return false;
}

bool ClientUI::ZoomToShip(int id) {
    if (auto ship = ClientApp::GetApp()->GetContext().ContextObjects().get<Ship>(id))
        return ZoomToFleet(ship->FleetID());
    return false;
}

bool ClientUI::ZoomToBuilding(int id) {
    if (auto building = ClientApp::GetApp()->GetContext().ContextObjects().get<Building>(id)) {
        ZoomToBuildingType(building->BuildingTypeName());
        return ZoomToPlanet(building->PlanetID());
    }
    return false;
}

bool ClientUI::ZoomToField(int id) {
    if (auto field = ClientApp::GetApp()->GetContext().ContextObjects().get<Field>(id))
        if (auto mapwnd = GetMapWnd(false))
            mapwnd->CenterOnObject(id);
    return false;
}

bool ClientUI::ZoomToCombatLog(int id) {
    if (GetCombatLogManager().GetLog(id)) {
        if (auto mapwnd = GetMapWnd(false))
            mapwnd->ShowCombatLog(id);
        return true;
    }
    ErrorLogger() << "Unable to find combat log with id " << id;
    return false;
}

void ClientUI::ZoomToSystem(std::shared_ptr<const System> system) {
    if (!system)
        return;
    if (auto mapwnd = GetMapWnd(false)) {
        mapwnd->CenterOnObject(system->ID());
        mapwnd->SelectSystem(system->ID());
    }
}

void ClientUI::ZoomToFleet(std::shared_ptr<const Fleet> fleet) {
    if (!fleet)
        return;
    if (auto mapwnd = GetMapWnd(false)) {
        mapwnd->CenterOnObject(fleet->ID());
        mapwnd->SelectFleet(fleet->ID());
    }
    if (const auto& fleet_wnd = FleetUIManager::GetFleetUIManager().WndForFleetID(fleet->ID()))
        fleet_wnd->SelectFleet(fleet->ID());
}

bool ClientUI::ZoomToContent(const std::string& name, bool reverse_lookup) {
    if (reverse_lookup) {
        for (const auto& tech_name : GetTechManager() | range_keys) {
            if (boost::iequals(name, UserString(tech_name)))
                return ZoomToTech(tech_name);
        }

        for (const auto& building_name : GetBuildingTypeManager() | range_keys) {
            if (boost::iequals(name, UserString(building_name)))
                return ZoomToBuildingType(building_name);
        }

        for (const auto& special_name : SpecialNames())
            if (boost::iequals(name, UserString(special_name)))
                return ZoomToSpecial(std::string{special_name}); // TODO: pass just the string_view

        for (const auto& hull_name : GetShipHullManager() | range_keys) {
            if (boost::iequals(name, UserString(hull_name)))
                return ZoomToShipHull(hull_name);
        }

        for (const auto& part_name : GetShipPartManager() | range_keys) {
            if (boost::iequals(name, UserString(part_name)))
                return ZoomToShipPart(part_name);
        }

        for (const auto& species_name : ClientApp::GetApp()->GetSpeciesManager() | range_keys) {
            if (boost::iequals(name, UserString(species_name)))
                return ZoomToSpecies(species_name);
        }

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

bool ClientUI::ZoomToTech(std::string tech_name) {
    if (!GetTech(tech_name))
        return false;
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->ShowTech(std::move(tech_name));
    return true;
}

bool ClientUI::ZoomToPolicy(std::string policy_name) {
    if (!GetPolicy(policy_name))
        return false;
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->ShowPolicy(std::move(policy_name));
    return true;
}

bool ClientUI::ZoomToBuildingType(std::string building_type_name) {
    if (!GetBuildingType(building_type_name))
        return false;
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->ShowBuildingType(std::move(building_type_name));
    return true;
}

bool ClientUI::ZoomToSpecial(std::string special_name) {
    if (!GetSpecial(special_name))
        return false;
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->ShowSpecial(std::move(special_name));
    return true;
}

bool ClientUI::ZoomToShipHull(std::string hull_name) {
    if (!GetShipHull(hull_name))
        return false;
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->ShowShipHull(std::move(hull_name));
    return true;
}

bool ClientUI::ZoomToShipPart(std::string part_name) {
    if (!GetShipPart(part_name))
        return false;
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->ShowShipPart(std::move(part_name));
    return true;
}

bool ClientUI::ZoomToSpecies(std::string species_name) {
    if (!ClientApp::GetApp()->GetSpeciesManager().GetSpecies(species_name))
        return false;
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->ShowSpecies(std::move(species_name));
    return true;
}

bool ClientUI::ZoomToFieldType(std::string field_type_name) {
    if (!GetFieldType(field_type_name))
        return false;
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->ShowFieldType(std::move(field_type_name));
    return true;
}

bool ClientUI::ZoomToShipDesign(int design_id) {
    const ScriptingContext& context = IApp::GetApp()->GetContext();
    if (!context.ContextUniverse().GetShipDesign(design_id))
        return false;
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->ShowShipDesign(design_id);
    return true;
}

bool ClientUI::ZoomToEmpire(int empire_id) {
    if (!GetEmpire(empire_id))
        return false;
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->ShowEmpire(empire_id);
    return true;
}

bool ClientUI::ZoomToMeterTypeArticle(std::string meter_string) {
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->ShowMeterTypeArticle(std::move(meter_string));
    return true;
}

bool ClientUI::ZoomToMeterTypeArticle(MeterType meter_type) {
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->ShowMeterTypeArticle(meter_type);
    return true;
}

bool ClientUI::ZoomToEncyclopediaEntry(std::string str) {
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->ShowEncyclopediaEntry(std::move(str));
    return true;
}

void ClientUI::DumpObject(int object_id) {
    if (auto obj = ClientApp::GetApp()->GetContext().ContextObjects().get(object_id))
        m_message_wnd->HandleLogMessage(obj->Dump());
}

void ClientUI::InitializeWindows() {
    const GG::Pt message_ul(GG::X0, GG::GUI::GetGUI()->AppHeight() - PANEL_HEIGHT);
    static constexpr GG::Pt message_wh(MESSAGE_PANEL_WIDTH, PANEL_HEIGHT);

    const GG::Pt player_list_ul(MESSAGE_PANEL_WIDTH, GG::GUI::GetGUI()->AppHeight() - PANEL_HEIGHT);
    static constexpr GG::Pt player_list_wh(PLAYER_LIST_PANEL_WIDTH, PANEL_HEIGHT);

    GetMessageWnd()->InitSizeMove(message_ul, message_ul + message_wh);
    GetPlayerListWnd()->InitSizeMove(player_list_ul, player_list_ul + player_list_wh);
}

void ClientUI::HandleSizeChange(bool fullscreen) const {
    OptionsDB& db = GetOptionsDB();

    if (db.Get<bool>("ui.reposition.auto.enabled")) {
        std::string window_mode = fullscreen ? ".fullscreen" : ".windowed";
        std::string option_name = "ui." + MESSAGE_WND_NAME + window_mode + ".left";

        // Invalidate the message window position so that we know to
        // recalculate positions on the next resize or fullscreen switch...
        db.Set(option_name, db.GetDefault<int>(option_name));
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
    if (db.Get<int>(option_name) == db.GetDefault<int>(option_name))
        GGHumanClientApp::GetApp()->RepositionWindowsSignal();
}

std::shared_ptr<GG::Texture> ClientUI::GetRandomTexture(const boost::filesystem::path& dir,
                                                        std::string_view prefix, bool mipmap)
{
    const auto& prefixed_textures = GetPrefixedTextures(dir, prefix, mipmap);
    if (prefixed_textures.empty())
        return nullptr;
    return prefixed_textures.at(RandInt(0, prefixed_textures.size()));
}

std::shared_ptr<GG::Texture> ClientUI::GetModuloTexture(const boost::filesystem::path& dir,
                                                        std::string_view prefix, int n, bool mipmap)
{
    assert(0 <= n);
    const auto& prefixed_textures = GetPrefixedTextures(dir, prefix, mipmap);
    if (prefixed_textures.empty())
        return nullptr;
    return prefixed_textures[n % prefixed_textures.size()];
}

void ClientUI::RestoreFromSaveData(const SaveGameUIData& ui_data) {
    if (auto mapwnd = GetMapWnd(false))
        mapwnd->RestoreFromSaveData(ui_data);
    m_ship_designs->Load(ui_data);
}

ClientUI* ClientUI::GetClientUI()
{ return s_the_UI; }

void ClientUI::MessageBox(const std::string& message, bool play_alert_sound) {
    auto dlg = GG::GUI::GetGUI()->GetStyleFactory().NewThreeButtonDlg(
        GG::X(320), GG::Y(200), message, GetFont(Pts()+2),
        WndColor(), WndOuterBorderColor(), CtrlColor(), TextColor(),
        1, UserString("OK"));
    if (play_alert_sound)
        Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.alert.sound.path"), true);
    dlg->Run();
}

std::shared_ptr<GG::Texture> ClientUI::GetTexture(const boost::filesystem::path& path, bool mipmap) {
    std::shared_ptr<GG::Texture> retval;
    try {
        retval = GGHumanClientApp::GetApp()->GetTexture(path, mipmap);
    } catch (const std::exception& e) {
        ErrorLogger() << "Unable to load texture \"" + path.generic_string() + "\"\n"
            "reason: " << e.what();
        try {
            retval = GGHumanClientApp::GetApp()->GetTexture(ClientUI::ArtDir() / "misc" / "missing.png", mipmap);
        } catch (...) {
            return retval;
        }
    } catch (...) {
        ErrorLogger() << "Unable to load texture \"" + path.generic_string() + "\"\n"
            "reason unknown...?";
        try {
            retval = GGHumanClientApp::GetApp()->GetTexture(ClientUI::ArtDir() / "misc" / "missing.png", mipmap);
        } catch (...) {
            return retval;
        }
    }
#ifdef FREEORION_MACOSX
    if (!mipmap)
        retval->SetFilters(GL_LINEAR, GL_LINEAR);
#endif
    return retval;
}

std::shared_ptr<GG::Font> ClientUI::GetFont(int pts) {
    const auto& rqcs = RequiredCharsets();
    auto* gui = GG::GUI::GetGUI();
    try {
       return gui->GetFont(GetOptionsDB().Get<std::string>("ui.font.path"),
                           pts, rqcs.begin(), rqcs.end());
    } catch (...) {
        try {
            return gui->GetFont(GetOptionsDB().GetDefault<std::string>("ui.font.path"),
                                pts, rqcs.begin(), rqcs.end());
        } catch (...) {
             return gui->GetStyleFactory().DefaultFont(pts);
        }
    }
}

std::shared_ptr<GG::Font> ClientUI::GetBoldFont(int pts) {
    const auto& rqcs = RequiredCharsets();
    auto* gui = GG::GUI::GetGUI();
    try {
        return gui->GetFont(GetOptionsDB().Get<std::string>("ui.font.bold.path"),
                            pts, rqcs.begin(), rqcs.end());
    } catch (...) {
        try {
             return gui->GetFont(GetOptionsDB().GetDefault<std::string>("ui.font.bold.path"),
                                 pts, rqcs.begin(), rqcs.end());
        } catch (...) {
             return gui->GetStyleFactory().DefaultFont(pts);
        }
    }
}

std::shared_ptr<GG::Font> ClientUI::GetTitleFont(int pts) {
    const auto& rqcs = RequiredCharsets();
    auto* gui = GG::GUI::GetGUI();
    try {
        return gui->GetFont(GetOptionsDB().Get<std::string>("ui.font.title.path"),
                            pts, rqcs.begin(), rqcs.end());
    } catch (...) {
        try {
            return gui->GetFont(GetOptionsDB().GetDefault<std::string>("ui.font.title.path"),
                                pts, rqcs.begin(), rqcs.end());
        } catch (...) {
             return gui->GetStyleFactory().DefaultFont(pts);
        }
   }
}

const std::vector<std::shared_ptr<GG::Texture>>& ClientUI::GetPrefixedTextures(
    const boost::filesystem::path& dir, std::string_view prefix, bool mipmap)
{
    namespace fs = boost::filesystem;
    if (!fs::is_directory(dir)) {
        ErrorLogger() << "GetPrefixedTextures passed invalid dir: " << dir;
        static CONSTEXPR_VEC const std::vector<std::shared_ptr<GG::Texture>> EMPTY_VEC;
        return EMPTY_VEC;
    }

    std::string KEY{(dir / prefix.data()).string()};
    auto prefixed_textures_it = m_prefixed_textures.find(KEY);
    if (prefixed_textures_it != m_prefixed_textures.end())
        return prefixed_textures_it->second;

    // if not already loaded, load textures with requested key
    std::vector<std::shared_ptr<GG::Texture>> textures;
    fs::directory_iterator end_it;
    for (fs::directory_iterator it(dir); it != end_it; ++it) {
        try {
            if (fs::exists(*it) &&
                !fs::is_directory(*it) &&
                boost::algorithm::starts_with(it->path().filename().string(), prefix))
            { textures.push_back(ClientUI::GetTexture(*it, mipmap)); }
        } catch (const fs::filesystem_error& e) {
            // ignore files for which permission is denied, and rethrow other exceptions
            if (e.code() != boost::system::errc::permission_denied)
                throw;
        }
    }
    std::sort(textures.begin(), textures.end(), TextureFileNameCompare);
    auto emplace_it = m_prefixed_textures.emplace(std::move(KEY), std::move(textures)).first;
    return emplace_it->second;
}

int FontBasedUpscale(int x) {
    const int font_pts = ClientUI::Pts();
    return (font_pts <= 12) ? x : (font_pts * x / 12);
}

namespace GG {
    std::istream& operator>>(std::istream& is, Clr& clr) {
        namespace qi = boost::spirit::qi;
        namespace phx = boost::phoenix;
        std::string str;
        std::getline(is, str, ')');
        str.push_back(')');
        bool parsed = qi::phrase_parse(
            str.begin(), str.end(),
            (
                qi::lit('(') >>
                qi::int_[phx::ref(clr.r) = qi::_1] >> ',' >>
                qi::int_[phx::ref(clr.g) = qi::_1] >> ',' >>
                qi::int_[phx::ref(clr.b) = qi::_1] >> ',' >>
                qi::int_[phx::ref(clr.a) = qi::_1] >> ')'),
            qi::blank
        );
        if (!parsed ||
            clr.r < 0 || 255 < clr.r ||
            clr.g < 0 || 255 < clr.g ||
            clr.b < 0 || 255 < clr.b ||
            clr.a < 0 || 255 < clr.a)
            is.setstate(std::ios_base::failbit);
        return is;
    }
}
