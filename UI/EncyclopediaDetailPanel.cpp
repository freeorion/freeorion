#include "EncyclopediaDetailPanel.h"

#include "CUIControls.h"
#include "DesignWnd.h"
#include "FleetWnd.h"
#include "GraphControl.h"
#include "Hotkeys.h"
#include "LinkText.h"
#include "CUILinkTextBlock.h"
#include "MapWnd.h"
#include "../universe/Condition.h"
#include "../universe/Encyclopedia.h"
#include "../universe/Universe.h"
#include "../universe/Tech.h"
#include "../universe/Building.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Ship.h"
#include "../universe/Fleet.h"
#include "../universe/Special.h"
#include "../universe/Species.h"
#include "../universe/Field.h"
#include "../universe/Effect.h"
#include "../universe/Predicates.h"
#include "../universe/ValueRef.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../util/EnumText.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/GameRules.h"
#include "../util/Directories.h"
#include "../util/VarText.h"
#include "../util/ScopedTimer.h"
#include "../client/human/HumanClientApp.h"
#include "../combat/CombatLogManager.h"

#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/RichText/RichText.h>
#include <GG/ScrollPanel.h>
#include <GG/StaticGraphic.h>
#include <GG/Texture.h>

#include <boost/algorithm/clamp.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <unordered_map>

FO_COMMON_API extern const int INVALID_DESIGN_ID;

using boost::io::str;

namespace {
    const GG::X TEXT_MARGIN_X(3);
    const GG::Y TEXT_MARGIN_Y(0);
    const int DESCRIPTION_PADDING(3);

    void    AddOptions(OptionsDB& db) {
        db.Add("resource.effects.description.shown", UserStringNop("OPTIONS_DB_DUMP_EFFECTS_GROUPS_DESC"), false, Validator<bool>());
        db.Add("ui.pedia.search.articles.enabled", UserStringNop("OPTIONS_DB_UI_ENC_SEARCH_ARTICLE"), true, Validator<bool>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    const std::string EMPTY_STRING;
    const std::string INCOMPLETE_DESIGN = "incomplete design";
    const std::string UNIVERSE_OBJECT = "universe object";
    const std::string PLANET_SUITABILITY_REPORT = "planet suitability report";
    const std::string GRAPH = "data graph";
    const std::string TEXT_SEARCH_RESULTS = "dynamic generated text";

    /** @content_tag{CTRL_ALWAYS_REPORT} Always display a species on a planet suitability report. **/
    const std::string TAG_ALWAYS_REPORT = "CTRL_ALWAYS_REPORT";
    /** @content_tag{CTRL_EXTINCT} Added to both a species and their enabling tech.  Handles display in planet suitability report. **/
    const std::string TAG_EXTINCT = "CTRL_EXTINCT";
    /** @content_tag{PEDIA_} Defines an encyclopedia category for the generated article of the containing content definition.  The category name should be postfixed to this tag. **/
    const std::string TAG_PEDIA_PREFIX = "PEDIA_";
}

namespace {
    /** @brief Checks content tags for a custom defined pedia category.
     * 
     * @param[in,out] tags content tags to check for a matching pedia prefix tag
     * 
     * @return The first matched pedia category for this set of tags,
     *          or empty string if there are no matches.
     */
    std::string DetermineCustomCategory(const std::set<std::string>& tags) {
        if (tags.empty())
            return EMPTY_STRING;

        // for each tag, check if it starts with the prefix
        // when a match is found, return the match (without the prefix portion)
        for (const std::string& tag : tags)
            if (boost::starts_with(tag, TAG_PEDIA_PREFIX))
                return boost::replace_first_copy(tag, TAG_PEDIA_PREFIX, EMPTY_STRING);

        return EMPTY_STRING;
    }

    /** Retreive a value label and general string representation for @a meter_type */
    std::pair<std::string, std::string> MeterValueLabelAndString(const MeterType& meter_type) {
        std::pair<std::string, std::string> retval;

        retval.second = boost::lexical_cast<std::string>(meter_type);

        retval.first = retval.second + "_VALUE_LABEL";
        if (UserStringExists(retval.first)) {
            retval.first = UserString(retval.first);
        } else {
            DebugLogger() << "No pedia entry found for value of Meter Type: "
                          << retval.second << "(" << meter_type << ")";
            retval.first = UserString(retval.second);
        }

        return retval;
    }

    void MeterTypeDirEntry(const MeterType& meter_type,
                           std::multimap<std::string,
                                         std::pair<std::string,
                                                   std::string>>& list)
    {
        std::pair<std::string, std::string> meter_name = MeterValueLabelAndString(meter_type);

        if (meter_name.first.empty() || meter_name.second.empty())
            return;

        std::string link_string = LinkTaggedPresetText(VarText::METER_TYPE_TAG,
                                                       meter_name.second,
                                                       meter_name.first) + "\n";

        list.insert({meter_name.first, {link_string, meter_name.second}});
    }

    const std::vector<std::string>& GetSearchTextDirNames() {
        static std::vector<std::string> dir_names {
            "ENC_INDEX",        "ENC_SHIP_PART",    "ENC_SHIP_HULL",    "ENC_TECH",
            "ENC_BUILDING_TYPE","ENC_SPECIAL",      "ENC_SPECIES",      "ENC_FIELD_TYPE",
            "ENC_METER_TYPE",   "ENC_EMPIRE",       "ENC_SHIP_DESIGN",  "ENC_SHIP",
            "ENC_MONSTER",      "ENC_MONSTER_TYPE", "ENC_FLEET",        "ENC_PLANET",
            "ENC_BUILDING",     "ENC_SYSTEM",       "ENC_FIELD",        "ENC_GRAPH",
            "ENC_GALAXY_SETUP", "ENC_GAME_RULES"};
        //  "ENC_HOMEWORLDS" omitted due to weird formatting of article titles
        return dir_names;
    }

    /** Find Encyclopedia article with given name
     * @param[in] name name entry of the article
     */
    const EncyclopediaArticle& GetPediaArticle(const std::string& name) {
        const auto& articles = GetEncyclopedia().Articles();
        for (const auto& entry : articles) {
            for (const EncyclopediaArticle& article : entry.second) {
                if (article.name == name) {
                    return article;
                }
            }
        }
        return GetEncyclopedia().empty_article;
    }

    /** Returns map from (Human-readable and thus sorted article category) to
        pair of (article link tag text, stringtable key for article category or
        subcategorization of it). Category is something like "ENC_TECH" and
        subcategorization is something like a tech category. */
    void GetSortedPediaDirEntires(const std::string& dir_name,
                                  std::multimap<std::string,
                                                std::pair<std::string,
                                                          std::string>>& sorted_entries_list)
    {
        ScopedTimer subdir_timer("GetSortedPediaDirEntires(" + dir_name + ")",
                                 true, std::chrono::milliseconds(20));

        const Encyclopedia& encyclopedia = GetEncyclopedia();
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        if (dir_name == "ENC_INDEX") {
            // add entries consisting of links to pedia page lists of
            // articles of various types
            for (const std::string& str : GetSearchTextDirNames()) {
                if (str == "ENC_INDEX")
                    continue;
                sorted_entries_list.insert({UserString(str),
                                            {LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, str) + "\n",
                                             str}});
            }

            for (const std::string& str : {"ENC_TEXTURES", "ENC_HOMEWORLDS"}) {
                sorted_entries_list.insert({UserString(str),
                                            {LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, str) + "\n",
                                             str}});
            }

            for (const auto& entry : encyclopedia.Articles()) {
                // Do not add sub-categories
                const EncyclopediaArticle& article = GetPediaArticle(entry.first);
                // No article found or specifically a top-level category
                if (!article.category.empty() && article.category != "ENC_INDEX")
                    continue;

                sorted_entries_list.insert({UserString(entry.first),
                                            {LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, entry.first) + "\n",
                                             entry.first}});
            }

        } else if (dir_name == "ENC_SHIP_PART") {
            for (const auto& entry : GetPartTypeManager()) {
                std::string custom_category = DetermineCustomCategory(entry.second->Tags());
                if (custom_category.empty()) {
                    sorted_entries_list.insert({UserString(entry.first),
                                                {LinkTaggedText(VarText::SHIP_PART_TAG, entry.first) + "\n",
                                                 entry.first}});
                }
            }

        } else if (dir_name == "ENC_SHIP_HULL") {
            for (const auto& entry : GetHullTypeManager()) {
                std::string custom_category = DetermineCustomCategory(entry.second->Tags());
                if (custom_category.empty()) {
                    sorted_entries_list.insert({UserString(entry.first),
                                                {LinkTaggedText(VarText::SHIP_HULL_TAG, entry.first) + "\n",
                                                 entry.first}});
                }
            }

        } else if (dir_name == "ENC_TECH") {
            std::map<std::string, std::string> userstring_tech_names;
            // sort tech names by user-visible name, so names are shown alphabetically in UI
            for (const auto& tech_name : GetTechManager().TechNames()) {
                userstring_tech_names[UserString(tech_name)] = tech_name;
            }
            for (const auto& tech_name : userstring_tech_names) {
                std::string custom_category = DetermineCustomCategory(GetTech(tech_name.second)->Tags());
                if (custom_category.empty()) {
                    // already iterating over userstring-looked-up names, so don't need to re-look-up-here
                    sorted_entries_list.insert({tech_name.first,
                                                {LinkTaggedText(VarText::TECH_TAG, tech_name.second) + "\n",
                                                 tech_name.second}});
                }
            }

        } else if (dir_name == "ENC_BUILDING_TYPE") {
            for (const auto& entry : GetBuildingTypeManager()) {
                std::string custom_category = DetermineCustomCategory(entry.second->Tags());
                if (custom_category.empty()) {
                    sorted_entries_list.insert({UserString(entry.first),
                                                {LinkTaggedText(VarText::BUILDING_TYPE_TAG, entry.first) + "\n",
                                                 entry.first}});
                }
            }

        } else if (dir_name == "ENC_SPECIAL") {
            for (const std::string& special_name : SpecialNames()) {
                sorted_entries_list.insert({UserString(special_name),
                                            {LinkTaggedText(VarText::SPECIAL_TAG, special_name) + "\n",
                                             special_name}});
            }

        } else if (dir_name == "ENC_SPECIES") {
            for (const auto& entry : GetSpeciesManager()) {
                std::string custom_category = DetermineCustomCategory(entry.second->Tags());
                if (custom_category.empty()) {
                    sorted_entries_list.insert({UserString(entry.first),
                                                {LinkTaggedText(VarText::SPECIES_TAG, entry.first) + "\n",
                                                 entry.first}});
                }
            }

        } else if (dir_name == "ENC_HOMEWORLDS") {
            const SpeciesManager& species_manager = GetSpeciesManager();
            for (const auto& entry : species_manager) {
                const auto& species = entry.second;
                std::set<int> known_homeworlds;
                //std::string species_entry = UserString(entry.first) + ":  ";
                std::string species_entry;
                std::string homeworld_info;
                species_entry += LinkTaggedText(VarText::SPECIES_TAG, entry.first) + " ";
                // homeworld
                if (species->Homeworlds().empty()) {
                    continue;
                } else {
                    species_entry += "(" + std::to_string(species->Homeworlds().size()) + "):  ";
                    for (int homeworld_id : species->Homeworlds()) {
                        if (std::shared_ptr<const Planet> homeworld = GetPlanet(homeworld_id)) {
                            known_homeworlds.insert(homeworld_id);
                            // if known, add to beginning
                            homeworld_info = LinkTaggedIDText(VarText::PLANET_ID_TAG, homeworld_id, homeworld->PublicName(client_empire_id)) + "   " + homeworld_info;
                        } else { 
                            // add to end
                            homeworld_info += UserString("UNKNOWN_PLANET") + "   ";
                        }
                    }
                    species_entry += homeworld_info;
                }

                // occupied planets
                std::vector<std::shared_ptr<const Planet>> species_occupied_planets;
                for (auto& planet : Objects().FindObjects<Planet>()) {
                    if ((planet->SpeciesName() == entry.first) && !known_homeworlds.count(planet->ID()))
                        species_occupied_planets.push_back(planet);
                }
                if (!species_occupied_planets.empty()) {
                    if (species_occupied_planets.size() >= 5) {
                        species_entry += "  |   " + std::to_string(species_occupied_planets.size()) + " " + UserString("OCCUPIED_PLANETS");
                    } else {
                        species_entry += "  |   " + UserString("OCCUPIED_PLANETS") + ":  ";
                        for (auto& planet : species_occupied_planets) {
                            species_entry += LinkTaggedIDText(VarText::PLANET_ID_TAG, planet->ID(), planet->PublicName(client_empire_id)) + "   ";
                        }
                    }
                }
                sorted_entries_list.insert({UserString(entry.first),
                {species_entry + "\n", entry.first}});
            }
            sorted_entries_list.insert({"⃠ ", {"\n\n", "  "}});
            for (const auto& entry : species_manager) {
                const auto& species = entry.second;
                if (species->Homeworlds().empty()) {
                    std::string species_entry = LinkTaggedText(VarText::SPECIES_TAG, entry.first) + ":  ";
                    species_entry += UserString("NO_HOMEWORLD");
                    sorted_entries_list.insert({ "⃠⃠" + std::string( "⃠ ") + UserString(entry.first),
                                                {species_entry + "\n", entry.first}});
                }
            } 

        } else if (dir_name == "ENC_FIELD_TYPE") {
            for (const auto& entry : GetFieldTypeManager()) {
                std::string custom_category = DetermineCustomCategory(entry.second->Tags());
                if (custom_category.empty()) {
                    sorted_entries_list.insert({UserString(entry.first),
                                                {LinkTaggedText(VarText::FIELD_TYPE_TAG, entry.first) + "\n",
                                                 entry.first}});
                }
            }

        } else if (dir_name == "ENC_METER_TYPE") {
            for (MeterType meter_type = METER_POPULATION;
                 meter_type != NUM_METER_TYPES;
                 meter_type = static_cast<MeterType>(static_cast<int>(meter_type) + 1))
            {
                if (meter_type > INVALID_METER_TYPE && meter_type < NUM_METER_TYPES)
                    MeterTypeDirEntry(meter_type, sorted_entries_list);
            }

        } else if (dir_name == "ENC_EMPIRE") {
            for (auto& entry : Empires()) {
                sorted_entries_list.insert({UserString(entry.second->Name()),
                                            {LinkTaggedIDText(VarText::EMPIRE_ID_TAG, entry.first, entry.second->Name()) + "\n",
                                             std::to_string(entry.first)}});
            }

        } else if (dir_name == "ENC_SHIP_DESIGN") {
            for (auto it = GetUniverse().beginShipDesigns();
                 it != GetUniverse().endShipDesigns(); ++it)
            {
                if (it->second->IsMonster())
                    continue;
                sorted_entries_list.insert(
                    {it->second->Name(),
                        {LinkTaggedIDText(VarText::DESIGN_ID_TAG, it->first,
                                          it->second->Name()) + "\n",
                                          std::to_string(it->first)}});
            }

        } else if (dir_name == "ENC_SHIP") {
            for (auto& ship : Objects().FindObjects<Ship>()) {
                const std::string& ship_name = ship->PublicName(client_empire_id);
                sorted_entries_list.insert({ship_name,
                                            {LinkTaggedIDText(VarText::SHIP_ID_TAG, ship->ID(), ship_name) + "  ",
                                             std::to_string(ship->ID())}});
            }

        } else if (dir_name == "ENC_MONSTER") {
            for (auto& ship : Objects().FindObjects<Ship>()) {
                if (!ship->IsMonster())
                    continue;
                const std::string& ship_name = ship->PublicName(client_empire_id);
                sorted_entries_list.insert({ship_name,
                                            {LinkTaggedIDText(VarText::SHIP_ID_TAG, ship->ID(), ship_name) + "  ",
                                             std::to_string(ship->ID())}});
            }

        } else if (dir_name == "ENC_MONSTER_TYPE") {
            for (auto it = GetUniverse().beginShipDesigns();
                 it != GetUniverse().endShipDesigns(); ++it)
            {
                if (it->second->IsMonster())
                    sorted_entries_list.insert(
                        {it->second->Name(),
                            {LinkTaggedIDText(VarText::DESIGN_ID_TAG, it->first,
                                              it->second->Name()) + "\n",
                                              std::to_string(it->first)}});
            }

        } else if (dir_name == "ENC_FLEET") {
            for (auto& fleet : Objects().FindObjects<Fleet>()) {
                const std::string& flt_name = fleet->PublicName(client_empire_id);
                sorted_entries_list.insert({flt_name,
                                            {LinkTaggedIDText(VarText::FLEET_ID_TAG, fleet->ID(), flt_name) + "  ",
                                             std::to_string(fleet->ID())}});
            }

        } else if (dir_name == "ENC_PLANET") {
            for (auto& planet : Objects().FindObjects<Planet>()) {
                const std::string& plt_name = planet->PublicName(client_empire_id);
                sorted_entries_list.insert({plt_name,
                                            {LinkTaggedIDText(VarText::PLANET_ID_TAG, planet->ID(), plt_name) + "  ",
                                             std::to_string(planet->ID())}});
            }

        } else if (dir_name == "ENC_BUILDING") {
            for (auto& building : Objects().FindObjects<Building>()) {
                const std::string& bld_name = building->PublicName(client_empire_id);
                sorted_entries_list.insert({bld_name,
                                            {LinkTaggedIDText(VarText::BUILDING_ID_TAG, building->ID(), bld_name) + "  ",
                                             std::to_string(building->ID())}});
            }

        } else if (dir_name == "ENC_SYSTEM") {
            for (auto& system : Objects().FindObjects<System>()) {
                const std::string& sys_name = system->ApparentName(client_empire_id);
                sorted_entries_list.insert({sys_name,
                                            {LinkTaggedIDText(VarText::SYSTEM_ID_TAG, system->ID(), sys_name) + "  ",
                                             std::to_string(system->ID())}});
            }

        } else if (dir_name == "ENC_FIELD") {
            for (auto& field : Objects().FindObjects<Field>()) {
                const std::string& field_name = field->Name();
                sorted_entries_list.insert({field_name,
                                            {LinkTaggedIDText(VarText::FIELD_ID_TAG, field->ID(), field_name) + "  ",
                                             std::to_string(field->ID())}});
            }

        } else if (dir_name == "ENC_GRAPH") {
            for (const auto& stat_record : GetUniverse().GetStatRecords()) {
                sorted_entries_list.insert({UserString(stat_record.first),
                                            {LinkTaggedText(TextLinker::GRAPH_TAG, stat_record.first) + "\n",
                                             stat_record.first}});
            }

        } else if (dir_name == "ENC_TEXTURES") {
             for (auto tex : GG::GetTextureManager().Textures()) {
                 std::string texture_info_str = boost::io::str(
                     FlexibleFormat(UserString("ENC_TEXTURE_INFO")) %
                     Value(tex.second->Width()) %
                     Value(tex.second->Height()) %
                     tex.second->BytesPP() %
                     tex.first);
                 sorted_entries_list.insert({tex.first, {texture_info_str, tex.first}});
             }

             for (auto tex: GG::GetVectorTextureManager().Textures()) {
                 std::string texture_info_str = boost::io::str(
                     FlexibleFormat(UserString("ENC_VECTOR_TEXTURE_INFO")) %
                     Value(tex.second->Size().x) %
                     Value(tex.second->Size().y) %
                     tex.second->NumShapes() %
                     tex.first);
                 sorted_entries_list.insert({tex.first, {texture_info_str, tex.first}});
             }

        } else if (dir_name == "ENC_STRINGS") {
            // TODO: show all stringable keys and values
            //for (auto str : GetStringTable().

        } else {
            // Any content definitions (FOCS files) that define a pedia category
            // should have their pedia article added to this category.
            std::map<std::string, std::pair<std::string, std::string>> dir_entries;

            // part types
            for (const auto& entry : GetPartTypeManager())
                if (DetermineCustomCategory(entry.second->Tags()) == dir_name)
                    dir_entries[UserString(entry.first)] =
                        std::make_pair(VarText::SHIP_PART_TAG, entry.first);

            // hull types
            for (const auto& entry : GetHullTypeManager())
                if (DetermineCustomCategory(entry.second->Tags()) == dir_name)
                    dir_entries[UserString(entry.first)] =
                        std::make_pair(VarText::SHIP_HULL_TAG, entry.first);

            // techs
            for (const auto& tech_name : GetTechManager().TechNames())
                if (DetermineCustomCategory(GetTech(tech_name)->Tags()) == dir_name)
                    dir_entries[UserString(tech_name)] =
                        std::make_pair(VarText::TECH_TAG, tech_name);

            // building types
            for (const auto& entry : GetBuildingTypeManager())
                if (DetermineCustomCategory(entry.second->Tags()) == dir_name)
                    dir_entries[UserString(entry.first)] =
                        std::make_pair(VarText::BUILDING_TYPE_TAG, entry.first);

            // species
            for (const auto& entry : GetSpeciesManager())
                if (DetermineCustomCategory(entry.second->Tags()) == dir_name)
                    dir_entries[UserString(entry.first)] =
                        std::make_pair(VarText::SPECIES_TAG, entry.first);

            // field types
            for (const auto& entry : GetFieldTypeManager())
                if (DetermineCustomCategory(entry.second->Tags()) == dir_name)
                    dir_entries[UserString(entry.first)] =
                        std::make_pair(VarText::FIELD_TYPE_TAG, entry.first);

            // Add sorted entries
            for (const auto& entry : dir_entries) {
                sorted_entries_list.insert({entry.first,
                                            {LinkTaggedText(entry.second.first, entry.second.second) + "\n",
                                             entry.second.second}});
            }

        }

        // Add any defined entries for this directory
        const auto& articles = encyclopedia.Articles();
        auto category_it = articles.find(dir_name);
        if (category_it != articles.end()) {
            for (const EncyclopediaArticle& article : category_it->second) {
                // Prevent duplicate addition of hard-coded directories that also have a content definition
                if (article.name == dir_name)
                    continue;
                sorted_entries_list.insert({UserString(article.name),
                    {LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, article.name) + "\n", article.name}});
            }
        }
    }

    std::string PediaDirText(const std::string& dir_name) {
        std::string retval;

        // get sorted list of entries for requested directory
        std::multimap<std::string, std::pair<std::string, std::string>> sorted_entries_list;
        GetSortedPediaDirEntires(dir_name, sorted_entries_list);

        // add sorted entries to page text
        for (const auto& entry : sorted_entries_list)
        { retval += entry.second.first; }

        return retval;
    }
}

namespace {
    class SearchEdit : public CUIEdit {
    public:
        SearchEdit() :
            CUIEdit("")
        {}

        void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override {
            switch (key) {
            case GG::GGK_RETURN:
            case GG::GGK_KP_ENTER:
                TextEnteredSignal();
                break;
            default:
                break;
            }
            CUIEdit::KeyPress(key, key_code_point, mod_keys);
        }

        mutable boost::signals2::signal<void ()> TextEnteredSignal;
    };
}

std::list<std::pair<std::string, std::string>>
    EncyclopediaDetailPanel::m_items = std::list<std::pair<std::string, std::string>>(0);
std::list<std::pair<std::string, std::string>>::iterator
    EncyclopediaDetailPanel::m_items_it = m_items.begin();

EncyclopediaDetailPanel::EncyclopediaDetailPanel(GG::Flags<GG::WndFlag> flags,
                                                 const std::string& config_name) :
    CUIWnd(UserString("MAP_BTN_PEDIA"), flags, config_name, false)
{}

void EncyclopediaDetailPanel::CompleteConstruction() {
    CUIWnd::CompleteConstruction();

    const int PTS = ClientUI::Pts();
    const int NAME_PTS = PTS*3/2;
    const int SUMMARY_PTS = PTS*4/3;

    m_name_text =    GG::Wnd::Create<CUILabel>("");
    m_cost_text =    GG::Wnd::Create<CUILabel>("");
    m_summary_text = GG::Wnd::Create<CUILabel>("");

    m_name_text->SetFont(ClientUI::GetBoldFont(NAME_PTS));
    m_summary_text->SetFont(ClientUI::GetFont(SUMMARY_PTS));

    boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";

    m_index_button = Wnd::Create<CUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "uparrownormal.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "uparrowclicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "uparrowmouseover.png")));

    m_back_button = Wnd::Create<CUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "leftarrownormal.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "leftarrowclicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "leftarrowmouseover.png")));

    m_next_button = Wnd::Create<CUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "rightarrownormal.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "rightarrowclicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "rightarrowmouseover.png")));

    m_back_button->Disable();
    m_next_button->Disable();

    m_index_button->LeftClickedSignal.connect(
        boost::bind(&EncyclopediaDetailPanel::OnIndex, this));
    m_back_button->LeftClickedSignal.connect(
        boost::bind(&EncyclopediaDetailPanel::OnBack, this));
    m_next_button->LeftClickedSignal.connect(
        boost::bind(&EncyclopediaDetailPanel::OnNext, this));

    m_description_rich_text = GG::Wnd::Create<GG::RichText>(
        GG::X(0), GG::Y(0), ClientWidth(), ClientHeight(), "",
        ClientUI::GetFont(), ClientUI::TextColor(),
        GG::FORMAT_TOP | GG::FORMAT_LEFT | GG::FORMAT_LINEWRAP | GG::FORMAT_WORDBREAK,
        GG::INTERACTIVE);
    m_scroll_panel = GG::Wnd::Create<GG::ScrollPanel>(GG::X(0), GG::Y(0), ClientWidth(),
                                                      ClientHeight(), m_description_rich_text);

    // Copy default block factory.
    std::shared_ptr<GG::RichText::BLOCK_FACTORY_MAP>
        factory_map(new GG::RichText::BLOCK_FACTORY_MAP(*GG::RichText::DefaultBlockFactoryMap()));
    auto factory = new CUILinkTextBlock::Factory();
    // Wire this factory to produce links that talk to us.
    factory->LinkClickedSignal.connect(
        boost::bind(&EncyclopediaDetailPanel::HandleLinkClick, this, _1, _2));
    factory->LinkDoubleClickedSignal.connect(
        boost::bind(&EncyclopediaDetailPanel::HandleLinkDoubleClick, this, _1, _2));
    factory->LinkRightClickedSignal.connect(
        boost::bind(&EncyclopediaDetailPanel::HandleLinkDoubleClick, this, _1, _2));
    (*factory_map)[GG::RichText::PLAINTEXT_TAG] =
        std::shared_ptr<GG::RichText::IBlockControlFactory>(factory);
    m_description_rich_text->SetBlockFactoryMap(factory_map);
    m_description_rich_text->SetPadding(DESCRIPTION_PADDING);

    m_scroll_panel->SetBackgroundColor(ClientUI::CtrlColor());
    m_scroll_panel->InstallEventFilter(shared_from_this());

    m_graph = GG::Wnd::Create<GraphControl>();
    m_graph->ShowPoints(false);

    auto search_edit = GG::Wnd::Create<SearchEdit>();
    m_search_edit = search_edit;
    search_edit->TextEnteredSignal.connect(
        boost::bind(&EncyclopediaDetailPanel::HandleSearchTextEntered, this));

    AttachChild(m_search_edit);
    AttachChild(m_graph);
    AttachChild(m_name_text);
    AttachChild(m_cost_text);
    AttachChild(m_summary_text);
    AttachChild(m_scroll_panel);
    AttachChild(m_index_button);
    AttachChild(m_back_button);
    AttachChild(m_next_button);

    SetChildClippingMode(ClipToWindow);
    DoLayout();
    MoveChildUp(m_graph);
    SaveDefaultedOptions();

    AddItem(TextLinker::ENCYCLOPEDIA_TAG, "ENC_INDEX");
}

EncyclopediaDetailPanel::~EncyclopediaDetailPanel()
{}

void EncyclopediaDetailPanel::DoLayout() {
    const int PTS = ClientUI::Pts();
    const int NAME_PTS = PTS*3/2;
    const int COST_PTS = PTS;
    const int SUMMARY_PTS = PTS*4/3;

    const int ICON_SIZE = 12 + NAME_PTS + COST_PTS + SUMMARY_PTS;

    const int BTN_WIDTH = 24;
    const int Y_OFFSET = 22;

    SectionedScopedTimer timer("EncyclopediaDetailPanel::DoLayout",
                               std::chrono::milliseconds(1));

    // name
    GG::Pt ul = GG::Pt(GG::X(6), GG::Y(Y_OFFSET));
    GG::Pt lr = ul + GG::Pt(Width() - 1, GG::Y(NAME_PTS + 4));
    m_name_text->SetTextFormat(GG::FORMAT_LEFT);
    m_name_text->SizeMove(ul, lr);

    // cost / turns
    ul += GG::Pt(GG::X0, m_name_text->Height());
    lr = ul + GG::Pt(Width(), GG::Y(COST_PTS + 4));
    m_cost_text->SetTextFormat(GG::FORMAT_LEFT);
    m_cost_text->SizeMove(ul, lr);

    // one line summary
    ul += GG::Pt(GG::X0, m_cost_text->Height());
    lr = ul + GG::Pt(Width(), GG::Y(SUMMARY_PTS + 4));
    m_summary_text->SetTextFormat(GG::FORMAT_LEFT);
    m_summary_text->SizeMove(ul, lr);

    // main verbose description (fluff, effects, unlocks, ...)
    ul = GG::Pt(BORDER_LEFT, ICON_SIZE + BORDER_BOTTOM + TEXT_MARGIN_Y + Y_OFFSET + 1);
    lr = GG::Pt(Width() - BORDER_RIGHT, Height() - BORDER_BOTTOM*3 - PTS - 4);
    timer.EnterSection("description->SizeMove");
    m_scroll_panel->SizeMove(ul, lr);
    timer.EnterSection("");

    // graph
    m_graph->SizeMove(ul + GG::Pt(GG::X1, GG::Y1), lr - GG::Pt(GG::X1, GG::Y1));

    // "back" button
    ul = GG::Pt(Width() - BORDER_RIGHT*3 - BTN_WIDTH * 3 - 8,   Height() - BORDER_BOTTOM*2 - PTS);
    lr = GG::Pt(Width() - BORDER_RIGHT*3 - BTN_WIDTH * 2 - 8,   Height() - BORDER_BOTTOM*2);
    m_back_button->SizeMove(ul, lr);

    // "up" button
    ul = GG::Pt(Width() - BORDER_RIGHT*3 - BTN_WIDTH * 2 - 4,   Height() - BORDER_BOTTOM*3 - PTS);
    lr = GG::Pt(Width() - BORDER_RIGHT*3 - BTN_WIDTH - 4,       Height() - BORDER_BOTTOM*3);
    m_index_button->SizeMove(ul, lr);

    // "next" button
    ul = GG::Pt(Width() - BORDER_RIGHT*3 - BTN_WIDTH,           Height() - BORDER_BOTTOM*2 - PTS);
    lr = GG::Pt(Width() - BORDER_RIGHT*3,                       Height() - BORDER_BOTTOM*2);
    m_next_button->SizeMove(ul, lr);

    // search edit box
    ul = GG::Pt(BORDER_LEFT + 2,                                Height() - BORDER_BOTTOM*3 - PTS - 3);
    lr = GG::Pt(Width() - BORDER_RIGHT*3 - BTN_WIDTH * 3 - 12,  Height() - BORDER_BOTTOM - 3);
    m_search_edit->SizeMove(ul, lr);


    // icon
    if (m_icon) {
        lr = GG::Pt(Width() - BORDER_RIGHT, GG::Y(ICON_SIZE + 1 + Y_OFFSET));
        ul = lr - GG::Pt(GG::X(ICON_SIZE), GG::Y(ICON_SIZE));
        m_icon->SizeMove(ul, lr);
    }

    PositionButtons();
    MoveChildUp(m_close_button);    // so it's over top of the top-right icon
}

void EncyclopediaDetailPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    CUIWnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        RequirePreRender();
}

void EncyclopediaDetailPanel::KeyPress(GG::Key key, std::uint32_t key_code_point,
                                       GG::Flags<GG::ModKey> mod_keys)
{
    if (key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER) {
        GG::GUI::GetGUI()->SetFocusWnd(m_search_edit);
    } else if (key == GG::GGK_BACKSPACE) {
        this->OnBack();
    } else {
        m_scroll_panel->KeyPress(key, key_code_point, mod_keys);
    }
}

GG::Pt EncyclopediaDetailPanel::ClientUpperLeft() const
{ return GG::Wnd::UpperLeft(); }

void EncyclopediaDetailPanel::Render() {
    CUIWnd::Render();

    // title underline
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.0f);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    m_vertex_buffer.activate();
    if (!m_minimized) {
        glColor(ClientUI::WndInnerBorderColor());
        glDrawArrays(GL_LINES,  m_buffer_indices[4].first, m_buffer_indices[4].second);
    }

    glEnable(GL_TEXTURE_2D);
    glPopClientAttrib();
}

void EncyclopediaDetailPanel::InitBuffers() {
    m_vertex_buffer.clear();
    m_vertex_buffer.reserve(19);
    m_buffer_indices.resize(5);
    std::size_t previous_buffer_size = m_vertex_buffer.size();

    const int Y_OFFSET = 22;

    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    const GG::Y ICON_SIZE = m_summary_text->Bottom() - m_name_text->Top();
    GG::Pt cl_ul = ul + GG::Pt(BORDER_LEFT, ICON_SIZE + BORDER_BOTTOM + Y_OFFSET); // BORDER_BOTTOM is the size of the border at the bottom of a standard CUIWnd
    GG::Pt cl_lr = lr - GG::Pt(BORDER_RIGHT, BORDER_BOTTOM);


    // within m_vertex_buffer:
    // [0] is the start and range for minimized background triangle fan and minimized border line loop
    // [1] is ... the background fan / outer border line loop
    // [2] is ... the inner border line loop
    // [3] is ... the resize tab line list

    // minimized background fan and border line loop
    m_vertex_buffer.store(Value(ul.x),  Value(ul.y));
    m_vertex_buffer.store(Value(lr.x),  Value(ul.y));
    m_vertex_buffer.store(Value(lr.x),  Value(lr.y));
    m_vertex_buffer.store(Value(ul.x),  Value(lr.y));
    m_buffer_indices[0].first = previous_buffer_size;
    m_buffer_indices[0].second = m_vertex_buffer.size() - previous_buffer_size;
    previous_buffer_size = m_vertex_buffer.size();

    // outer border, with optional corner cutout
    m_vertex_buffer.store(Value(ul.x),  Value(ul.y));
    m_vertex_buffer.store(Value(lr.x),  Value(ul.y));
    if (!m_resizable) {
        m_vertex_buffer.store(Value(lr.x),                            Value(lr.y) - OUTER_EDGE_ANGLE_OFFSET);
        m_vertex_buffer.store(Value(lr.x) - OUTER_EDGE_ANGLE_OFFSET,  Value(lr.y));
    } else {
        m_vertex_buffer.store(Value(lr.x),  Value(lr.y));
    }
    m_vertex_buffer.store(Value(ul.x),      Value(lr.y));
    m_buffer_indices[1].first = previous_buffer_size;
    m_buffer_indices[1].second = m_vertex_buffer.size() - previous_buffer_size;
    previous_buffer_size = m_vertex_buffer.size();

    // inner border, with optional corner cutout
    m_vertex_buffer.store(Value(cl_ul.x),       Value(cl_ul.y));
    m_vertex_buffer.store(Value(cl_lr.x),       Value(cl_ul.y));
    if (m_resizable) {
        m_vertex_buffer.store(Value(cl_lr.x),                             Value(cl_lr.y) - INNER_BORDER_ANGLE_OFFSET);
        m_vertex_buffer.store(Value(cl_lr.x) - INNER_BORDER_ANGLE_OFFSET, Value(cl_lr.y));
    } else {
        m_vertex_buffer.store(Value(cl_lr.x),   Value(cl_lr.y));
    }
    m_vertex_buffer.store(Value(cl_ul.x),       Value(cl_lr.y));
    m_buffer_indices[2].first = previous_buffer_size;
    m_buffer_indices[2].second = m_vertex_buffer.size() - previous_buffer_size;
    previous_buffer_size = m_vertex_buffer.size();

    // resize hash marks
    m_vertex_buffer.store(Value(cl_lr.x),                           Value(cl_lr.y) - RESIZE_HASHMARK1_OFFSET);
    m_vertex_buffer.store(Value(cl_lr.x) - RESIZE_HASHMARK1_OFFSET, Value(cl_lr.y));
    m_vertex_buffer.store(Value(cl_lr.x),                           Value(cl_lr.y) - RESIZE_HASHMARK2_OFFSET);
    m_vertex_buffer.store(Value(cl_lr.x) - RESIZE_HASHMARK2_OFFSET, Value(cl_lr.y));
    m_buffer_indices[3].first = previous_buffer_size;
    m_buffer_indices[3].second = m_vertex_buffer.size() - previous_buffer_size;
    previous_buffer_size = m_vertex_buffer.size();

    // title underline
    m_vertex_buffer.store(Value(cl_lr.x) + 2.0f,    Value(cl_ul.y - ICON_SIZE) - 7.0f);
    m_vertex_buffer.store(Value(cl_ul.x) - 2.0f,    Value(cl_ul.y - ICON_SIZE) - 7.0f);
    m_buffer_indices[4].first = previous_buffer_size;
    m_buffer_indices[4].second = m_vertex_buffer.size() - previous_buffer_size;
    //previous_buffer_size = m_vertex_buffer.size();

    m_vertex_buffer.createServerBuffer();
}

void EncyclopediaDetailPanel::HandleLinkClick(const std::string& link_type, const std::string& data) {
    using boost::lexical_cast;
    try {
        if (link_type == VarText::PLANET_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToPlanet(lexical_cast<int>(data));
            this->SetPlanet(lexical_cast<int>(data));

        } else if (link_type == VarText::SYSTEM_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToSystem(lexical_cast<int>(data));
        } else if (link_type == VarText::FLEET_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToFleet(lexical_cast<int>(data));
        } else if (link_type == VarText::SHIP_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToShip(lexical_cast<int>(data));
        } else if (link_type == VarText::BUILDING_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToBuilding(lexical_cast<int>(data));
        } else if (link_type == VarText::FIELD_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToField(lexical_cast<int>(data));

        } else if (link_type == VarText::COMBAT_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToCombatLog(lexical_cast<int>(data));

        } else if (link_type == VarText::EMPIRE_ID_TAG) {
            this->SetEmpire(lexical_cast<int>(data));
        } else if (link_type == VarText::DESIGN_ID_TAG) {
            this->SetDesign(lexical_cast<int>(data));
        } else if (link_type == VarText::PREDEFINED_DESIGN_TAG) {
            if (const ShipDesign* design = GetPredefinedShipDesign(data))
                this->SetDesign(design->ID());

        } else if (link_type == VarText::TECH_TAG) {
            this->SetTech(data);
        } else if (link_type == VarText::BUILDING_TYPE_TAG) {
            this->SetBuildingType(data);
        } else if (link_type == VarText::FIELD_TYPE_TAG) {
            this->SetFieldType(data);
        } else if (link_type == VarText::METER_TYPE_TAG) {
            this->SetMeterType(data);
        } else if (link_type == VarText::SPECIAL_TAG) {
            this->SetSpecial(data);
        } else if (link_type == VarText::SHIP_HULL_TAG) {
            this->SetHullType(data);
        } else if (link_type == VarText::SHIP_PART_TAG) {
            this->SetPartType(data);
        } else if (link_type == VarText::SPECIES_TAG) {
            this->SetSpecies(data);
        } else if (link_type == TextLinker::ENCYCLOPEDIA_TAG) {
            this->SetText(data, false);
        } else if (link_type == TextLinker::GRAPH_TAG) {
            this->SetGraph(data);
        } else if (link_type == TextLinker::URL_TAG) {
            HumanClientApp::GetApp()->OpenURL(data);
        } else if (link_type == TextLinker::BROWSE_PATH_TAG) {
            HumanClientApp::GetApp()->BrowsePath(FilenameToPath(data));
        }

    } catch (const boost::bad_lexical_cast&) {
        ErrorLogger() << "EncyclopediaDetailPanel::HandleLinkClick caught lexical cast exception for link type: " << link_type << " and data: " << data;
    }
}

void EncyclopediaDetailPanel::HandleLinkDoubleClick(const std::string& link_type, const std::string& data) {
    using boost::lexical_cast;
    try {
        if (link_type == VarText::PLANET_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToPlanet(lexical_cast<int>(data));
        } else if (link_type == VarText::SYSTEM_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToSystem(lexical_cast<int>(data));
        } else if (link_type == VarText::FLEET_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToFleet(lexical_cast<int>(data));
        } else if (link_type == VarText::SHIP_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToShip(lexical_cast<int>(data));
        } else if (link_type == VarText::BUILDING_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToBuilding(lexical_cast<int>(data));

        } else if (link_type == VarText::EMPIRE_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToEmpire(lexical_cast<int>(data));
        } else if (link_type == VarText::DESIGN_ID_TAG) {
            ClientUI::GetClientUI()->ZoomToShipDesign(lexical_cast<int>(data));
        } else if (link_type == VarText::PREDEFINED_DESIGN_TAG) {
            if (const ShipDesign* design = GetPredefinedShipDesign(data))
                ClientUI::GetClientUI()->ZoomToShipDesign(design->ID());

        } else if (link_type == VarText::TECH_TAG) {
            ClientUI::GetClientUI()->ZoomToTech(data);
        } else if (link_type == VarText::BUILDING_TYPE_TAG) {
            ClientUI::GetClientUI()->ZoomToBuildingType(data);
        } else if (link_type == VarText::SPECIAL_TAG) {
            ClientUI::GetClientUI()->ZoomToSpecial(data);
        } else if (link_type == VarText::SHIP_HULL_TAG) {
            ClientUI::GetClientUI()->ZoomToShipHull(data);
        } else if (link_type == VarText::SHIP_PART_TAG) {
            ClientUI::GetClientUI()->ZoomToShipPart(data);
        } else if (link_type == VarText::SPECIES_TAG) {
            ClientUI::GetClientUI()->ZoomToSpecies(data);

        } else if (link_type == TextLinker::ENCYCLOPEDIA_TAG) {
            this->SetText(data, false);
        } else if (link_type == TextLinker::GRAPH_TAG) {
            this->SetGraph(data);
        }
    } catch (const boost::bad_lexical_cast&) {
        ErrorLogger() << "EncyclopediaDetailPanel::HandleLinkDoubleClick caught lexical cast exception for link type: " << link_type << " and data: " << data;
    }
}

namespace {
    std::unordered_map<std::string, std::pair<std::string, std::string>>
        GetSubDirs(const std::string& dir_name, int depth = 0)
    {
        ScopedTimer subdir_timer("GetSubDirs(" + dir_name + ", " + std::to_string(depth) + ")",
                                 true, std::chrono::milliseconds(500));

        depth++;
        std::unordered_map<std::string, std::pair<std::string, std::string>> retval;
        // safety check to pre-empt potential infinite loop
        if (depth > 50) {
            WarnLogger() << "Exceeded recursive limit with lookup for pedia"
                         << " category " << dir_name;
            return retval;
        }


        std::unordered_map<std::string, std::pair<std::string, std::string>> sub_dirs;
        std::multimap<std::string, std::pair<std::string, std::string>> sorted_entries;
        GetSortedPediaDirEntires(dir_name, sorted_entries);
        for (auto& entry : sorted_entries) {
            // explicitly exclude textures
            if (entry.second.second == "ENC_TEXTURES")
                continue;
            sub_dirs.emplace(entry.second.second,
                             std::make_pair(entry.first, entry.second.first));
        }

        // Add each article for this directory
        for (auto& article : sub_dirs) {
            if (article.first == dir_name)
                continue;
            // If a sub-category, add this article and any nested categories
            for (const auto& sub_article : GetSubDirs(article.first, depth))
                retval.emplace(sub_article);
            retval.emplace(article);
        }

        return retval;
    }
}

void EncyclopediaDetailPanel::HandleSearchTextEntered() {
    // search lists of articles for typed text
    const std::string& search_text = m_search_edit->Text();
    if (search_text.empty())
        return;

    // assemble link text to all pedia entries, indexed by name
    std::unordered_map<std::string, std::pair<std::string, std::string>>
        all_pedia_entries_list;

    for (auto&& subdir_name : GetSubDirs("ENC_INDEX"))
        all_pedia_entries_list.emplace(subdir_name);

    // find distinct words in search text
    std::set<std::string> words_in_search_text;
    for (const auto& word_range : GG::GUI::GetGUI()->FindWordsStringIndices(search_text)) {
        if (word_range.first == word_range.second)
            continue;
        std::string word(search_text.begin() + Value(word_range.first), search_text.begin() + Value(word_range.second));
        if (word.empty())
            continue;
        words_in_search_text.insert(word);
    }


    ////
    // search through all articles for full or partial matches to search query
    ///


    std::multimap<std::string, std::string> exact_match_report;
    std::multimap<std::string, std::string> word_match_report;
    std::multimap<std::string, std::string> partial_match_report;
    std::multimap<std::string, std::string> article_match_report;

    bool search_desc = GetOptionsDB().Get<bool>("ui.pedia.search.articles.enabled");

    for (const auto& entry : all_pedia_entries_list) {
        // search for exact title matches
        if (boost::iequals(entry.second.first, search_text)) {
            exact_match_report.emplace(entry.second);
            continue;
        }

        bool listed = false;
        // search for full word matches in titles
        for (const std::string& word : words_in_search_text) {
            if (GG::GUI::GetGUI()->ContainsWord(entry.second.first, word)) {
                word_match_report.emplace(entry.second);
                listed = true;
                break;
            }
        }
        if (listed)
            continue;

        // search for partial word matches: searched-for words that appear
        // in the title text, not necessarily as a complete word
        for (const std::string& word : words_in_search_text) {
            // reject searches in text for words less than 3 characters
            if (word.size() < 3)
                continue;
            if (boost::icontains(entry.second.first, word)) {
                partial_match_report.emplace(entry.second);
                listed = true;
                break;
            }
        }
        if (listed || !search_desc)
            continue;

        // search for matches within article text
        EncyclopediaArticle article_entry = GetPediaArticle(entry.first);
        if (article_entry.description.empty()) {
            // NOTE Articles generated through GetRefreshDetailPanelInfo
            // will not be found, as no article currently exists.
            TraceLogger() << "No description found: " << entry.first;
            continue;
        }
        if (boost::icontains(UserString(article_entry.description), search_text))
            article_match_report.emplace(entry.second);
    }

    // compile list of articles into some dynamically generated search report text
    std::string match_report = "";
    if (!exact_match_report.empty()) {
        match_report += "\n" + UserString("ENC_SEARCH_EXACT_MATCHES") + "\n\n";
        for (auto&& match : exact_match_report)
            match_report += match.second;
    }

    if (!word_match_report.empty()) {
        match_report += "\n" + UserString("ENC_SEARCH_WORD_MATCHES") + "\n\n";
        for (auto&& match : word_match_report)
            match_report += match.second;
    }

    if (!partial_match_report.empty()) {
        match_report += "\n" + UserString("ENC_SEARCH_PARTIAL_MATCHES") + "\n\n";
        for (auto&& match : partial_match_report)
            match_report += match.second;
    }

    if (!article_match_report.empty()) {
        match_report += "\n" + UserString("ENC_SEARCH_ARTICLE_MATCHES") + "\n\n";
        for (auto&& match : article_match_report)
            match_report += match.second;
    }

    if (match_report.empty())
        match_report = UserString("ENC_SEARCH_NOTHING_FOUND");

    AddItem(TEXT_SEARCH_RESULTS, match_report);
}

namespace {
    int DefaultLocationForEmpire(int empire_id) {
        const Empire* empire = GetEmpire(empire_id);
        if (!empire) {
            DebugLogger() << "DefaultLocationForEmpire: Unable to get empire with ID: " << empire_id;
            return INVALID_OBJECT_ID;
        }
        // get a location where the empire might build something.
        auto location = GetUniverseObject(empire->CapitalID());
        // no capital?  scan through all objects to find one owned by this empire
        // TODO: only loop over planets?
        // TODO: pass in a location condition, and pick a location that matches it if possible
        if (!location) {
            for (const auto& obj : Objects()) {
                if (obj->OwnedBy(empire_id)) {
                    location = obj;
                    break;
                }
            }
        }
        return location ? location->ID() : INVALID_OBJECT_ID;
    }

    std::vector<std::string> TechsThatUnlockItem(const ItemSpec& item) {
        std::vector<std::string> retval;

        for (const auto& tech : GetTechManager()) {
            if (!tech) continue;
            const std::string& tech_name = tech->Name();

            bool found_item = false;
            for (const ItemSpec& unlocked_item : tech->UnlockedItems()) {
                if (unlocked_item == item) {
                    found_item = true;
                    break;
                }
            }
            if (found_item)
                retval.push_back(tech_name);
        }

        return retval;
    }

    const std::string& GeneralTypeOfObject(UniverseObjectType obj_type) {
        switch (obj_type) {
        case OBJ_SHIP:          return UserString("ENC_SHIP");          break;
        case OBJ_FLEET:         return UserString("ENC_FLEET");         break;
        case OBJ_PLANET:        return UserString("ENC_PLANET");        break;
        case OBJ_BUILDING:      return UserString("ENC_BUILDING");      break;
        case OBJ_SYSTEM:        return UserString("ENC_SYSTEM");        break;
        case OBJ_FIELD:         return UserString("ENC_FIELD");         break;
        case OBJ_POP_CENTER:    return UserString("ENC_POP_CENTER");    break;
        case OBJ_PROD_CENTER:   return UserString("ENC_PROD_CENTER");   break;
        case OBJ_FIGHTER:       return UserString("ENC_FIGHTER");       break;
        default:                return EMPTY_STRING;
        }
    }

    void RefreshDetailPanelPediaTag(        const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        name = UserString(item_name);

        // special case for galaxy setup data: display info
        if (item_name == "ENC_GALAXY_SETUP") {
            const GalaxySetupData& gsd = ClientApp::GetApp()->GetGalaxySetupData();

            detailed_description += str(FlexibleFormat(UserString("ENC_GALAXY_SETUP_SETTINGS"))
                % gsd.m_seed
                % std::to_string(gsd.m_size)
                % TextForGalaxyShape(gsd.m_shape)
                % TextForGalaxySetupSetting(gsd.m_age)
                % TextForGalaxySetupSetting(gsd.m_starlane_freq)
                % TextForGalaxySetupSetting(gsd.m_planet_density)
                % TextForGalaxySetupSetting(gsd.m_specials_freq)
                % TextForGalaxySetupSetting(gsd.m_monster_freq)
                % TextForGalaxySetupSetting(gsd.m_native_freq)
                % TextForAIAggression(gsd.m_ai_aggr)
                % gsd.m_game_uid);

            return;
        } else if (item_name == "ENC_GAME_RULES") {
            const GameRules& rules = GetGameRules();

            for (auto& rule : rules) {
                if (rule.second.ValueIsDefault()) {
                    detailed_description += UserString(rule.first) + " : "
                                         + rule.second.ValueToString() + "\n";
                } else {
                    detailed_description += "<u>" + UserString(rule.first) + " : "
                                         + rule.second.ValueToString() + "</u>\n";
                }
            }
            return;
        }

        // search for article in custom pedia entries.
        for (const auto& entry : GetEncyclopedia().Articles()) {
            bool done = false;
            for (const EncyclopediaArticle& article : entry.second) {
                if (article.name != item_name)
                    continue;

                detailed_description = UserString(article.description);

                const std::string& article_cat = article.category;
                if (article_cat != "ENC_INDEX" && !article_cat.empty())
                    general_type = UserString(article_cat);

                const std::string& article_brief = article.short_description;
                if (!article_brief.empty())
                    specific_type = UserString(article_brief);

                texture = ClientUI::GetTexture(ClientUI::ArtDir() / article.icon, true);

                done = true;
                break;
            }
            if (done)
                break;
        }

        // add listing of articles in this category
        std::string dir_text = PediaDirText(item_name);
        if (dir_text.empty())
            return;

        if (!detailed_description.empty())
            detailed_description += "\n\n";

        detailed_description += dir_text;
    }

    void RefreshDetailPanelShipPartTag(     const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        const PartType* part = GetPartType(item_name);
        if (!part) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find part with name " << item_name;
            return;
        }
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        // Ship Parts
        name = UserString(item_name);
        texture = ClientUI::PartIcon(item_name);
        int default_location_id = DefaultLocationForEmpire(client_empire_id);
        turns = part->ProductionTime(client_empire_id, default_location_id);
        cost = part->ProductionCost(client_empire_id, default_location_id);
        cost_units = UserString("ENC_PP");
        general_type = UserString("ENC_SHIP_PART");
        specific_type = UserString(boost::lexical_cast<std::string>(part->Class()));

        detailed_description += UserString(part->Description()) + "\n\n" + part->CapacityDescription();

        std::string slot_types_list;
        if (part->CanMountInSlotType(SL_EXTERNAL))
            slot_types_list += UserString("SL_EXTERNAL") + "   ";
        if (part->CanMountInSlotType(SL_INTERNAL))
            slot_types_list += UserString("SL_INTERNAL") + "   ";
        if (part->CanMountInSlotType(SL_CORE))
            slot_types_list += UserString("SL_CORE");
        if (!slot_types_list.empty())
            detailed_description += "\n\n" + UserString("ENC_SHIP_PART_CAN_MOUNT_IN_SLOT_TYPES") + slot_types_list;

        const auto& exclusions = part->Exclusions();
        if (!exclusions.empty()) {
            detailed_description += "\n\n" + UserString("ENC_SHIP_EXCLUSIONS");
            for (const auto& exclusion : exclusions) {
                if (GetPartType(exclusion)) {
                    detailed_description += LinkTaggedText(VarText::SHIP_PART_TAG, exclusion) + "  ";
                } else if (GetHullType(exclusion)) {
                    detailed_description += LinkTaggedText(VarText::SHIP_HULL_TAG, exclusion) + "  ";
                } else {
                    // unknown exclusion...?
                }
            }
        }

        auto unlocked_by_techs = TechsThatUnlockItem(ItemSpec(UIT_SHIP_PART, item_name));
        if (!unlocked_by_techs.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            for (const auto& tech_name : unlocked_by_techs)
            { detailed_description += LinkTaggedText(VarText::TECH_TAG, tech_name) + "  "; }
            detailed_description += "\n\n";
        }

        if (GetOptionsDB().Get<bool>("resource.effects.description.shown")) {
            if (part->Location())
                detailed_description += "\n" + part->Location()->Dump();
            if (!part->Effects().empty())
                detailed_description += "\n" + Dump(part->Effects());
        }
    }

    void RefreshDetailPanelShipHullTag(     const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        const HullType* hull = GetHullType(item_name);
        if (!hull) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find hull with name " << item_name;
            return;
        }
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        // Ship Hulls
        name = UserString(item_name);
        texture = ClientUI::HullTexture(item_name);
        int default_location_id = DefaultLocationForEmpire(client_empire_id);
        turns = hull->ProductionTime(client_empire_id, default_location_id);
        cost = hull->ProductionCost(client_empire_id, default_location_id);
        cost_units = UserString("ENC_PP");
        general_type = UserString("ENC_SHIP_HULL");

        detailed_description += UserString(hull->Description()) + "\n\n" + str(FlexibleFormat(UserString("HULL_DESC"))
            % hull->Speed()
            % hull->Fuel()
            % hull->Speed()
            % hull->Structure());

        const auto& exclusions = hull->Exclusions();
        if (!exclusions.empty()) {
            detailed_description += "\n\n" + UserString("ENC_SHIP_EXCLUSIONS");
            for (const std::string& exclusion : exclusions) {
                if (GetPartType(exclusion)) {
                    detailed_description += LinkTaggedText(VarText::SHIP_PART_TAG, exclusion) + "  ";
                } else if (GetHullType(exclusion)) {
                    detailed_description += LinkTaggedText(VarText::SHIP_HULL_TAG, exclusion) + "  ";
                } else {
                    // unknown exclusion...?
                }
            }
        }

        auto unlocked_by_techs = TechsThatUnlockItem(ItemSpec(UIT_SHIP_HULL, item_name));
        if (!unlocked_by_techs.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            for (const auto& tech_name : unlocked_by_techs)
            { detailed_description += LinkTaggedText(VarText::TECH_TAG, tech_name) + "  "; }
            detailed_description += "\n\n";
        }

        if (GetOptionsDB().Get<bool>("resource.effects.description.shown")) {
            if (hull->Location())
                detailed_description += "\n" + hull->Location()->Dump();
            if (!hull->Effects().empty())
                detailed_description += "\n" + Dump(hull->Effects());
        }
    }

    void RefreshDetailPanelTechTag(         const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        const Tech* tech = GetTech(item_name);
        if (!tech) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find tech with name " << item_name;
            return;
        }
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        // Technologies
        name = UserString(item_name);
        texture = ClientUI::TechIcon(item_name);
        other_texture = ClientUI::CategoryIcon(tech->Category()); 
        color = ClientUI::CategoryColor(tech->Category());
        turns = tech->ResearchTime(client_empire_id);
        cost = tech->ResearchCost(client_empire_id);
        cost_units = UserString("ENC_RP");
        general_type = str(FlexibleFormat(UserString("ENC_TECH_DETAIL_TYPE_STR"))
            % UserString(tech->Category())
            % ""
            % UserString(tech->ShortDescription()));

        const auto& unlocked_techs = tech->UnlockedTechs();
        const auto& unlocked_items = tech->UnlockedItems();
        if (!unlocked_techs.empty() || !unlocked_items.empty())
            detailed_description += UserString("ENC_UNLOCKS");

        if (!unlocked_techs.empty()) {
            for (const auto& tech_name : unlocked_techs) {
                std::string link_text = LinkTaggedText(VarText::TECH_TAG, tech_name);
                detailed_description += str(FlexibleFormat(UserString("ENC_TECH_DETAIL_UNLOCKED_ITEM_STR"))
                    % UserString("UIT_TECH")
                    % link_text);
            }
        }

        if (!unlocked_items.empty()) {
            for (const ItemSpec& item : unlocked_items) {
                std::string TAG;
                switch (item.type) {
                case UIT_BUILDING:      TAG = VarText::BUILDING_TYPE_TAG;       break;
                case UIT_SHIP_PART:     TAG = VarText::SHIP_PART_TAG;           break;
                case UIT_SHIP_HULL:     TAG = VarText::SHIP_HULL_TAG;           break;
                case UIT_SHIP_DESIGN:   TAG = VarText::PREDEFINED_DESIGN_TAG;   break;
                case UIT_TECH:          TAG = VarText::TECH_TAG;                break;
                default: break;
                }

                std::string link_text;
                if (!TAG.empty())
                    link_text = LinkTaggedText(TAG, item.name);
                else
                    link_text = UserString(item.name);

                detailed_description += str(FlexibleFormat(UserString("ENC_TECH_DETAIL_UNLOCKED_ITEM_STR"))
                    % UserString(boost::lexical_cast<std::string>(item.type))
                    % link_text);
            }
        }

        if (!unlocked_techs.empty() || !unlocked_items.empty())
            detailed_description += "\n";

        detailed_description += UserString(tech->Description());

        if (GetOptionsDB().Get<bool>("resource.effects.description.shown") && !tech->Effects().empty()) {
            detailed_description += "\n" + Dump(tech->Effects());
        }

        const auto& unlocked_by_techs = tech->Prerequisites();
        if (!unlocked_by_techs.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            for (const std::string& tech_name : unlocked_by_techs)
            { detailed_description += LinkTaggedText(VarText::TECH_TAG, tech_name) + "  "; }
            detailed_description += "\n\n";
        }
    }

    void RefreshDetailPanelBuildingTypeTag( const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        const BuildingType* building_type = GetBuildingType(item_name);
        if (!building_type) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find building type with name " << item_name;
            return;
        }
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        // Building types
        name = UserString(item_name);
        texture = ClientUI::BuildingIcon(item_name);
        int default_location_id = DefaultLocationForEmpire(client_empire_id);
        const auto& map_wnd = ClientUI::GetClientUI()->GetMapWnd();
        int this_location_id = map_wnd->SelectedPlanetID();
        turns = building_type->ProductionTime(client_empire_id, default_location_id);
        cost = building_type->ProductionCost(client_empire_id, default_location_id);
        cost_units = UserString("ENC_PP");
        general_type = UserString("ENC_BUILDING_TYPE");

        detailed_description += UserString(building_type->Description());

        if (building_type->ProductionCostTimeLocationInvariant()) {
            detailed_description += str(FlexibleFormat(UserString("ENC_AUTO_TIME_COST_INVARIANT_STR")) % UserString("ENC_VERB_PRODUCE_STR"));
        } else {
            detailed_description += str(FlexibleFormat(UserString("ENC_AUTO_TIME_COST_VARIABLE_STR")) % UserString("ENC_VERB_PRODUCE_STR"));
            if (auto planet = GetPlanet(this_location_id)) {
                int local_cost = building_type->ProductionCost(client_empire_id, this_location_id);
                int local_time = building_type->ProductionTime(client_empire_id, this_location_id);
                std::string local_name = planet->Name();
                detailed_description += str(FlexibleFormat(UserString("ENC_AUTO_TIME_COST_VARIABLE_DETAIL_STR")) 
                                        % local_name % local_cost % cost_units % local_time);
            }
        }
        if (GetOptionsDB().Get<bool>("resource.effects.description.shown")) {
            if (!building_type->ProductionCostTimeLocationInvariant()) {
                if (building_type->Cost() && !building_type->Cost()->ConstantExpr())
                    detailed_description += "\n\nProduction Cost:\n" + building_type->Cost()->Dump();
                if (building_type->Time() && !building_type->Time()->ConstantExpr())
                    detailed_description += "\n\n Production Time:\n" + building_type->Time()->Dump();
            }
            if (building_type->EnqueueLocation())
                detailed_description += "\n\nEnqueue Requirement:\n" + building_type->EnqueueLocation()->Dump();
            if (building_type->Location())
                detailed_description += "\n\nLocation Requirement:\n" + building_type->Location()->Dump();
            if (!building_type->Effects().empty())
                detailed_description += "\n\nEffects:\n" + Dump(building_type->Effects());
        }

        auto unlocked_by_techs = TechsThatUnlockItem(ItemSpec(UIT_BUILDING, item_name));
        if (!unlocked_by_techs.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            for (const std::string& tech_name : unlocked_by_techs)
            { detailed_description += LinkTaggedText(VarText::TECH_TAG, tech_name) + "  "; }
            detailed_description += "\n\n";
        }
    }

    void RefreshDetailPanelSpecialTag(      const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        const Special* special = GetSpecial(item_name);
        if (!special) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find special with name " << item_name;
            return;
        }
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        // Specials
        name = UserString(item_name);
        texture = ClientUI::SpecialIcon(item_name);
        detailed_description = special->Description();
        general_type = UserString("ENC_SPECIAL");

        // objects that have special
        std::vector<std::shared_ptr<const UniverseObject>> objects_with_special;
        for (const auto& obj : Objects())
            if (obj->Specials().count(item_name))
                objects_with_special.push_back(obj);

        if (!objects_with_special.empty()) {
            detailed_description += "\n\n" + UserString("OBJECTS_WITH_SPECIAL");
            for (auto& obj : objects_with_special) {
                if (auto ship = std::dynamic_pointer_cast<const Ship>(obj))
                    detailed_description += LinkTaggedIDText(VarText::SHIP_ID_TAG, ship->ID(), ship->PublicName(client_empire_id)) + "  ";

                else if (auto fleet = std::dynamic_pointer_cast<const Fleet>(obj))
                    detailed_description += LinkTaggedIDText(VarText::FLEET_ID_TAG, fleet->ID(), fleet->PublicName(client_empire_id)) + "  ";

                else if (auto planet = std::dynamic_pointer_cast<const Planet>(obj))
                    detailed_description += LinkTaggedIDText(VarText::PLANET_ID_TAG, planet->ID(), planet->PublicName(client_empire_id)) + "  ";

                else if (auto building = std::dynamic_pointer_cast<const Building>(obj))
                    detailed_description += LinkTaggedIDText(VarText::BUILDING_ID_TAG, building->ID(), building->PublicName(client_empire_id)) + "  ";

                else if (auto system = std::dynamic_pointer_cast<const System>(obj))
                    detailed_description += LinkTaggedIDText(VarText::SYSTEM_ID_TAG, system->ID(), system->PublicName(client_empire_id)) + "  ";

                else
                    detailed_description += obj->PublicName(client_empire_id) + "  ";
            }
            detailed_description += "\n";
        }

        if (GetOptionsDB().Get<bool>("resource.effects.description.shown")) {
            if (special->Location())
                detailed_description += "\n" + special->Location()->Dump();
            if (!special->Effects().empty())
                detailed_description += "\n" + Dump(special->Effects());
        }
    }

    void RefreshDetailPanelEmpireTag(       const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        int empire_id = ALL_EMPIRES;
        try {
            empire_id = boost::lexical_cast<int>(item_name);
        } catch(...)
        {}
        Empire* empire = GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find empire with id " << item_name;
            return;
        }

        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        // Capital
        name = empire->Name();
        auto capital = GetPlanet(empire->CapitalID());
        if (capital)
            detailed_description += UserString("EMPIRE_CAPITAL") +
                LinkTaggedIDText(VarText::PLANET_ID_TAG, capital->ID(), capital->Name());
        else
            detailed_description += UserString("NO_CAPITAL");

        // to facilitate AI debugging
        detailed_description += "\n" + UserString("EMPIRE_ID") + ": " + item_name;

        // Empire meters
        if (empire->meter_begin() != empire->meter_end()) {
            detailed_description += "\n\n";
            for (auto meter_it = empire->meter_begin();
                 meter_it != empire->meter_end(); ++meter_it)
            {
                detailed_description += UserString(meter_it->first) + ": "
                                     + DoubleToString(meter_it->second.Initial(), 3, false)
                                     + "\n";
            }
        }


        // Planets
        auto empire_planets = Objects().FindObjects(OwnedVisitor<Planet>(empire_id));
        if (!empire_planets.empty()) {
            detailed_description += "\n\n" + UserString("OWNED_PLANETS");
            for (auto& obj : empire_planets) {
                detailed_description += LinkTaggedIDText(VarText::PLANET_ID_TAG, obj->ID(), obj->PublicName(client_empire_id)) + "  ";
            }
        } else {
            detailed_description += "\n\n" + UserString("NO_OWNED_PLANETS_KNOWN");
        }

        // Fleets
        std::vector<std::shared_ptr<const UniverseObject>> nonempty_empire_fleets;
        for (auto& maybe_fleet : Objects().FindObjects(OwnedVisitor<Fleet>(empire_id))) {
            if (auto fleet = std::dynamic_pointer_cast<const Fleet>(maybe_fleet))
                if (!fleet->Empty())
                    nonempty_empire_fleets.push_back(fleet);
        }
        if (!nonempty_empire_fleets.empty()) {
            detailed_description += "\n\n" + UserString("OWNED_FLEETS") + "\n";
            for (auto& obj : nonempty_empire_fleets) {
                std::string fleet_link = LinkTaggedIDText(VarText::FLEET_ID_TAG, obj->ID(), obj->PublicName(client_empire_id));
                std::string system_link;
                if (auto system = GetSystem(obj->SystemID())) {
                    std::string sys_name = system->ApparentName(client_empire_id);
                    system_link = LinkTaggedIDText(VarText::SYSTEM_ID_TAG, system->ID(), sys_name);
                    detailed_description += str(FlexibleFormat(UserString("OWNED_FLEET_AT_SYSTEM"))
                                            % fleet_link % system_link);
                } else {
                    detailed_description += fleet_link;
                }
                detailed_description += "\n";
            }
        } else {
            detailed_description += "\n\n" + UserString("NO_OWNED_FLEETS_KNOWN");
        }


        // Techs
        auto techs = empire->ResearchedTechs();
        if (!techs.empty()) {
            detailed_description += "\n\n" + UserString("RESEARCHED_TECHS");
            std::multimap<int, std::string> sorted_techs;
            for (const auto& tech_entry : techs) {
                sorted_techs.emplace(tech_entry.second, tech_entry.first);
            }
            for (const auto& sorted_tech_entry : sorted_techs) {
                detailed_description += "\n";
                std::string turn_text;
                if (sorted_tech_entry.first == BEFORE_FIRST_TURN)
                    turn_text = UserString("BEFORE_FIRST_TURN");
                else
                    turn_text = UserString("TURN") + " " + std::to_string(sorted_tech_entry.first);
                detailed_description += LinkTaggedText(VarText::TECH_TAG, sorted_tech_entry.second)
                                     + " : " + turn_text;
            }
        } else {
            detailed_description += "\n\n" + UserString("NO_TECHS_RESEARCHED");
        }

        // WIP: Parts, Hulls, Buildings, ... available
        auto parts = empire->AvailableShipParts();
        if (!parts.empty()) {
            detailed_description += "\n\n" + UserString("AVAILABLE_PARTS");
            for (const auto& part_name : parts) {
                detailed_description += "\n";
                detailed_description += LinkTaggedText(VarText::SHIP_PART_TAG, part_name);
            }
        } else {
            detailed_description += "\n\n" + UserString("NO_PARTS_AVAILABLE");
        }
        auto hulls = empire->AvailableShipHulls();
        if (!hulls.empty()) {
            detailed_description += "\n\n" + UserString("AVAILABLE_HULLS");
            for (const auto& hull_name : hulls) {
                detailed_description += "\n";
                detailed_description += LinkTaggedText(VarText::SHIP_HULL_TAG, hull_name);
            }
        } else {
            detailed_description += "\n\n" + UserString("NO_HULLS_AVAILABLE");
        }
        auto buildings = empire->AvailableBuildingTypes();
        if (!buildings.empty()) {
            detailed_description += "\n\n" + UserString("AVAILABLE_BUILDINGS");
            for (const auto& building_name : buildings) {
                detailed_description += "\n";
                detailed_description += LinkTaggedText(VarText::BUILDING_TYPE_TAG, building_name);
            }
        } else {
            detailed_description += "\n\n" + UserString("NO_BUILDINGS_AVAILABLE");
        }

        // Misc. Statistics

        // empire destroyed ships...
        const auto& empire_ships_destroyed = empire->EmpireShipsDestroyed();
        if (!empire_ships_destroyed.empty())
            detailed_description += "\n\n" + UserString("EMPIRE_SHIPS_DESTROYED");
        for (const auto& entry : empire_ships_destroyed) {
            std::string num_str = std::to_string(entry.second);
            const Empire* target_empire = GetEmpire(entry.first);
            std::string target_empire_name;
            if (target_empire)
                target_empire_name = target_empire->Name();
            else
                target_empire_name = UserString("UNOWNED");

            detailed_description += "\n" + target_empire_name + " : " + num_str;
        }


        // ship designs destroyed
        const auto& empire_designs_destroyed = empire->ShipDesignsDestroyed();
        if (!empire_designs_destroyed.empty())
            detailed_description += "\n\n" + UserString("SHIP_DESIGNS_DESTROYED");
        for (const auto& entry : empire_designs_destroyed) {
            std::string num_str = std::to_string(entry.second);
            const ShipDesign* design = GetShipDesign(entry.first);
            std::string design_name;
            if (design)
                design_name = design->Name();
            else
                design_name = UserString("UNKNOWN");

            detailed_description += "\n" + design_name + " : " + num_str;
        }


        // species ships destroyed
        const auto& species_ships_destroyed = empire->SpeciesShipsDestroyed();
        if (!species_ships_destroyed.empty())
            detailed_description += "\n\n" + UserString("SPECIES_SHIPS_DESTROYED");
        for (const auto& entry : species_ships_destroyed) {
            std::string num_str = std::to_string(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;;
        }


        // species planets invaded
        const auto& species_planets_invaded = empire->SpeciesPlanetsInvaded();
        if (!species_planets_invaded.empty())
            detailed_description += "\n\n" + UserString("SPECIES_PLANETS_INVADED");
        for (const auto& entry : species_planets_invaded) {
            std::string num_str = std::to_string(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;
        }


        // species ships produced
        const auto& species_ships_produced = empire->SpeciesShipsProduced();
        if (!species_ships_produced.empty())
            detailed_description += "\n\n" + UserString("SPECIES_SHIPS_PRODUCED");
        for (const auto& entry : species_ships_produced) {
            std::string num_str = std::to_string(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;
        }


        // ship designs produced
        const auto& ship_designs_produced = empire->ShipDesignsProduced();
        if (!ship_designs_produced.empty())
            detailed_description += "\n\n" + UserString("SHIP_DESIGNS_PRODUCED");
        for (const auto& entry : ship_designs_produced) {
            std::string num_str = std::to_string(entry.second);
            const ShipDesign* design = GetShipDesign(entry.first);
            std::string design_name;
            if (design)
                design_name = design->Name();
            else
                design_name = UserString("UNKNOWN");

            detailed_description += "\n" + design_name + " : " + num_str;
        }


        // species ships lost
        const auto& species_ships_lost = empire->SpeciesShipsLost();
        if (!species_ships_lost.empty())
            detailed_description += "\n\n" + UserString("SPECIES_SHIPS_LOST");
        for (const auto& entry : species_ships_lost) {
            std::string num_str = std::to_string(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;
        }


        // ship designs lost
        const auto& ship_designs_lost = empire->ShipDesignsLost();
        if (!ship_designs_lost.empty())
            detailed_description += "\n\n" + UserString("SHIP_DESIGNS_LOST");
        for (const auto& entry : ship_designs_lost) {
            std::string num_str = std::to_string(entry.second);
            const ShipDesign* design = GetShipDesign(entry.first);
            std::string design_name;
            if (design)
                design_name = design->Name();
            else
                design_name = UserString("UNKNOWN");

            detailed_description += "\n" + design_name + " : " + num_str;
        }


        // species ships scrapped
        const auto& species_ships_scrapped = empire->SpeciesShipsScrapped();
        if (!species_ships_scrapped.empty())
            detailed_description += "\n\n" + UserString("SPECIES_SHIPS_SCRAPPED");
        for (const auto& entry : species_ships_scrapped) {
            std::string num_str = std::to_string(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;
        }


        // ship designs scrapped
        const auto& ship_designs_scrapped = empire->ShipDesignsScrapped();
        if (!ship_designs_scrapped.empty())
            detailed_description += "\n\n" + UserString("SHIP_DESIGNS_SCRAPPED");
        for (const auto& entry : ship_designs_scrapped) {
            std::string num_str = std::to_string(entry.second);
            const ShipDesign* design = GetShipDesign(entry.first);
            std::string design_name;
            if (design)
                design_name = design->Name();
            else
                design_name = UserString("UNKNOWN");

            detailed_description += "\n" + design_name + " : " + num_str;
        }


        // species planets depopulated
        const auto& species_planets_depoped = empire->SpeciesPlanetsDepoped();
        if (!species_planets_depoped.empty())
            detailed_description += "\n\n" + UserString("SPECIES_PLANETS_DEPOPED");
        for (const auto& entry : species_planets_depoped) {
            std::string num_str = std::to_string(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;
        }


        // species planets bombed
        const auto& species_planets_bombed = empire->SpeciesPlanetsBombed();
        if (!species_planets_bombed.empty())
            detailed_description += "\n\n" + UserString("SPECIES_PLANETS_BOMBED");
        for (const auto& entry : species_planets_bombed) {
            std::string num_str = std::to_string(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;
        }


        // buildings produced
        const auto& building_types_produced = empire->BuildingTypesProduced();
        if (!building_types_produced.empty())
            detailed_description += "\n\n" + UserString("BUILDING_TYPES_PRODUCED");
        for (const auto& entry : building_types_produced) {
            std::string num_str = std::to_string(entry.second);
            std::string building_type_name;
            if (entry.first.empty())
                building_type_name = UserString("NONE");
            else
                building_type_name = UserString(entry.first);
            detailed_description += "\n" + building_type_name + " : " + num_str;
        }


        // buildings scrapped
        const auto& building_types_scrapped = empire->BuildingTypesScrapped();
        if (!building_types_scrapped.empty())
            detailed_description += "\n\n" + UserString("BUILDING_TYPES_SCRAPPED");
        for (const auto& entry : building_types_scrapped) {
            std::string num_str = std::to_string(entry.second);
            std::string building_type_name;
            if (entry.first.empty())
                building_type_name = UserString("NONE");
            else
                building_type_name = UserString(entry.first);
            detailed_description += "\n" + building_type_name + " : " + num_str;
        }

        detailed_description += "\n";
    }

    void RefreshDetailPanelSpeciesTag(      const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        const Species* species = GetSpecies(item_name);
        if (!species) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find species with name " << item_name;
            return;
        }
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        // Species
        name = UserString(item_name);
        texture = ClientUI::SpeciesIcon(item_name);
        general_type = UserString("ENC_SPECIES");
        detailed_description = species->GameplayDescription();

        // inherent species limitations
        detailed_description += "\n";
        if (species->CanProduceShips())
            detailed_description += UserString("CAN_PRODUCE_SHIPS");
        else
            detailed_description += UserString("CANNOT_PRODUCE_SHIPS");
        detailed_description += "\n";
        if (species->CanColonize())
            detailed_description += UserString("CAN_COLONIZE");
        else
            detailed_description += UserString("CANNNOT_COLONIZE");

        // focus preference
        if (!species->PreferredFocus().empty()) {
            detailed_description += "\n\n";
            detailed_description += UserString("FOCUS_PREFERENCE");
            detailed_description += UserString(species->PreferredFocus());
        }

        // environmental preferences
        detailed_description += "\n\n";
        const auto& pt_env_map = species->PlanetEnvironments();
        if (!pt_env_map.empty()) {
            detailed_description += UserString("ENVIRONMENTAL_PREFERENCES") + "\n";
            for (const auto& pt_env : pt_env_map) {
                detailed_description += UserString(boost::lexical_cast<std::string>(pt_env.first)) + " : " +
                                        UserString(boost::lexical_cast<std::string>(pt_env.second)) + "\n";
                // add blank line between cyclical environments and asteroids and gas giants
                if (pt_env.first == PT_OCEAN)
                    detailed_description += "\n";
            }
        } else {
            detailed_description += "\n";
        }

        // homeworld
        detailed_description += "\n";
        if (species->Homeworlds().empty()) {
            detailed_description += UserString("NO_HOMEWORLD") + "\n";
        } else {
            detailed_description += UserString("HOMEWORLD") + "\n";
            for (int hw_id : species->Homeworlds()) {
                if (auto homeworld = GetPlanet(hw_id))
                    detailed_description += LinkTaggedIDText(VarText::PLANET_ID_TAG, hw_id,
                                                             homeworld->PublicName(client_empire_id)) + "\n";
                else
                    detailed_description += UserString("UNKNOWN_PLANET") + "\n";
            }
        }

        // occupied planets
        std::vector<std::shared_ptr<const Planet>> species_occupied_planets;
        auto& species_object_populations = GetSpeciesManager().SpeciesObjectPopulations();
        auto sp_op_it = species_object_populations.find(item_name);
        if (sp_op_it != species_object_populations.end()) {
            const auto& object_pops = sp_op_it->second;
            for (const auto& object_pop : object_pops) {
                auto plt = GetPlanet(object_pop.first);
                if (!plt)
                    continue;
                if (plt->SpeciesName() != item_name) {
                    ErrorLogger() << "SpeciesManager SpeciesObjectPopulations suggested planet had a species, but it doesn't?";
                    continue;
                }
                species_occupied_planets.push_back(plt);
            }
        }

        if (!species_occupied_planets.empty()) {
            detailed_description += "\n" + UserString("OCCUPIED_PLANETS") + "\n";
            for (auto& planet : species_occupied_planets) {
                detailed_description += LinkTaggedIDText(VarText::PLANET_ID_TAG, planet->ID(),
                                                         planet->PublicName(client_empire_id)) + "  ";
            }
            detailed_description += "\n";
        }

        // empire opinions
        const auto& seom = GetSpeciesManager().GetSpeciesEmpireOpinionsMap();
        auto species_it = seom.find(species->Name());
        if (species_it != seom.end()) {
            detailed_description += "\n" + UserString("OPINIONS_OF_EMPIRES") + "\n";
            for (const auto& entry : species_it->second) {
                const Empire* empire = GetEmpire(entry.first);
                if (!empire)
                    continue;
            }
        }

        // species opinions
        const auto& ssom = GetSpeciesManager().GetSpeciesSpeciesOpinionsMap();
        auto species_it2 = ssom.find(species->Name());
        if (species_it2 != ssom.end()) {
            detailed_description += "\n" + UserString("OPINIONS_OF_OTHER_SPECIES") + "\n";
            for (const auto& entry : species_it2->second) {
                const Species* species2 = GetSpecies(entry.first);
                if (!species2)
                    continue;

                detailed_description += UserString(species2->Name()) + " : " + DoubleToString(entry.second, 3, false) + "\n";
            }
        }

        // Long description
        detailed_description += "\n" + UserString(species->Description());

        // autogenerated dump text of parsed scripted species effects, if enabled in options
        if (GetOptionsDB().Get<bool>("resource.effects.description.shown") && !species->Effects().empty())
            detailed_description += "\n" + Dump(species->Effects());
    }

    void RefreshDetailPanelFieldTypeTag(    const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        const FieldType* field_type = GetFieldType(item_name);
        if (!field_type) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find fiedl type with name " << item_name;
            return;
        }

        // Field types
        name = UserString(item_name);
        texture = ClientUI::FieldTexture(item_name);
        general_type = UserString("ENC_FIELD_TYPE");

        detailed_description += UserString(field_type->Description());

        if (GetOptionsDB().Get<bool>("resource.effects.description.shown")) {
            if (!field_type->Effects().empty())
                detailed_description += "\n" + Dump(field_type->Effects());
        }
    }


    void RefreshDetailPanelMeterTypeTag(const std::string& item_type, const std::string& item_name,
                                        std::string& name, std::shared_ptr<GG::Texture>& texture,
                                        std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                        float& cost, std::string& cost_units, std::string& general_type,
                                        std::string& specific_type, std::string& detailed_description,
                                        GG::Clr& color)
    {
        MeterType meter_type = INVALID_METER_TYPE;
        std::istringstream item_ss(item_name);
        item_ss >> meter_type;

        texture = ClientUI::MeterIcon(meter_type);
        general_type = UserString("ENC_METER_TYPE");

        auto meter_name = MeterValueLabelAndString(meter_type);

        name = meter_name.first;

        if (UserStringExists(meter_name.second + "_VALUE_DESC"))
            detailed_description += UserString(meter_name.second + "_VALUE_DESC");
        else
            detailed_description += meter_name.first;

    }

    std::string GetDetailedDescriptionBase(const ShipDesign* design) {
        std::string hull_link;
        if (!design->Hull().empty())
            hull_link = LinkTaggedText(VarText::SHIP_HULL_TAG, design->Hull());

        std::string parts_list;
        std::map<std::string, int> non_empty_parts_count;
        for (const std::string& part_name : design->Parts()) {
            if (part_name.empty())
                continue;
            non_empty_parts_count[part_name]++;
        }
        for (auto part_it = non_empty_parts_count.begin();
             part_it != non_empty_parts_count.end(); ++part_it)
        {
            if (part_it != non_empty_parts_count.begin())
                parts_list += ", ";
            parts_list += LinkTaggedText(VarText::SHIP_PART_TAG, part_it->first);
            if (part_it->second > 1)
                parts_list += " x" + std::to_string(part_it->second);
        }
         return str(FlexibleFormat(UserString("ENC_SHIP_DESIGN_DESCRIPTION_BASE_STR"))
        % design->Description()
        % hull_link
        % parts_list);
    }

    std::string GetDetailedDescriptionStats(const std::shared_ptr<Ship> ship,
                                            const ShipDesign* design,
                                            float enemy_DR,
                                            std::set<float> enemy_shots, float cost)
    {
        //The strength of a fleet is approximately weapons * armor, or
        //(weapons - enemyShield) * armor / (enemyWeapons - shield). This
        //depends on the enemy's weapons and shields, so we estimate the
        //enemy technology from the turn.

        // use the current meter values here, not initial, as this is used
        // within a loop that sets the species, updates meter, then checks
        // meter values for display

        auto& species = ship->SpeciesName().empty() ? "Generic" : UserString(ship->SpeciesName());
        float structure = ship->CurrentMeterValue(METER_MAX_STRUCTURE);
        float shield = ship->CurrentMeterValue(METER_MAX_SHIELD);
        float attack = ship->TotalWeaponsDamage();
        float strength = std::pow(attack * structure, 0.6f);
        float typical_shot = *std::max_element(enemy_shots.begin(), enemy_shots.end());
        float typical_strength = std::pow(ship->TotalWeaponsDamage(enemy_DR) * structure * typical_shot / std::max(typical_shot - shield, 0.001f), 0.6f);
        return (FlexibleFormat(UserString("ENC_SHIP_DESIGN_DESCRIPTION_STATS_STR"))
            % species
            % attack
            % ship->SumCurrentPartMeterValuesForPartClass(METER_MAX_SECONDARY_STAT, PC_DIRECT_WEAPON)
            % structure
            % shield
            % ship->CurrentMeterValue(METER_DETECTION)
            % ship->CurrentMeterValue(METER_STEALTH)
            % ship->CurrentMeterValue(METER_SPEED)
            % ship->CurrentMeterValue(METER_MAX_FUEL)
            % ship->SumCurrentPartMeterValuesForPartClass(METER_CAPACITY, PC_COLONY)
            % ship->SumCurrentPartMeterValuesForPartClass(METER_CAPACITY, PC_TROOPS)
            % ship->FighterMax()
            % (attack - ship->TotalWeaponsDamage(0.0f, false))
            % ship->SumCurrentPartMeterValuesForPartClass(METER_MAX_CAPACITY, PC_FIGHTER_BAY)
            % strength
            % (strength / cost)
            % typical_shot
            % enemy_DR
            % typical_strength
            % (typical_strength / cost)).str();
    }

    void RefreshDetailPanelShipDesignTag(   const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        int design_id = boost::lexical_cast<int>(item_name);
        const ShipDesign* design = GetShipDesign(boost::lexical_cast<int>(item_name));
        if (!design) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find ShipDesign with id " << item_name;
            return;
        }

        GetUniverse().InhibitUniverseObjectSignals(true);

        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        // Ship Designs
        name = design->Name();
        texture = ClientUI::ShipDesignIcon(design_id);
        int default_location_id = DefaultLocationForEmpire(client_empire_id);
        turns = design->ProductionTime(client_empire_id, default_location_id);
        cost = design->ProductionCost(client_empire_id, default_location_id);
        cost_units = UserString("ENC_PP");
        general_type = design->IsMonster() ? UserString("ENC_MONSTER") : UserString("ENC_SHIP_DESIGN");

        float tech_level = boost::algorithm::clamp(CurrentTurn() / 400.0f, 0.0f, 1.0f);
        float typical_shot = 3 + 27 * tech_level;
        float enemy_DR = 20 * tech_level;
        DebugLogger() << "default enemy stats:: tech_level: " << tech_level << "   DR: " << enemy_DR << "   attack: " << typical_shot;
        std::set<float> enemy_shots;
        enemy_shots.insert(typical_shot);


        // select which species to show info for

        std::set<std::string> additional_species; // from currently selected planet and fleets, if any
        const auto& map_wnd = ClientUI::GetClientUI()->GetMapWnd();
        if (const auto planet = GetPlanet(map_wnd->SelectedPlanetID())) {
            if (!planet->SpeciesName().empty())
                additional_species.insert(planet->SpeciesName());
        }

        FleetUIManager& fleet_manager = FleetUIManager::GetFleetUIManager();
        std::set<int> chosen_ships;
        int selected_ship = fleet_manager.SelectedShipID();
        const FleetWnd* fleet_wnd = FleetUIManager::GetFleetUIManager().ActiveFleetWnd();
        if ((selected_ship == INVALID_OBJECT_ID) && fleet_wnd) {
            auto selected_fleets = fleet_wnd->SelectedFleetIDs();
            auto selected_ships = fleet_wnd->SelectedShipIDs();
            if (selected_ships.size() > 0)
                selected_ship = *selected_ships.begin();
            else {
                int selected_fleet_id = INVALID_OBJECT_ID;
                if (selected_fleets.size() == 1)
                    selected_fleet_id = *selected_fleets.begin();
                else if (fleet_wnd->FleetIDs().size() > 0)
                    selected_fleet_id = *fleet_wnd->FleetIDs().begin();
                if (auto selected_fleet = GetFleet(selected_fleet_id))
                    if (!selected_fleet->ShipIDs().empty())
                        selected_ship = *selected_fleet->ShipIDs().begin();
            }
        }

        if (selected_ship != INVALID_OBJECT_ID) {
            chosen_ships.insert(selected_ship);
            if (const auto this_ship = GetShip(selected_ship)) {
                if (!this_ship->SpeciesName().empty())
                    additional_species.insert(this_ship->SpeciesName());
                if (!this_ship->OwnedBy(client_empire_id)) {
                    enemy_DR = this_ship->InitialMeterValue(METER_MAX_SHIELD);
                    DebugLogger() << "Using selected ship for enemy values, DR: " << enemy_DR;
                    enemy_shots.clear();
                    auto this_damage = this_ship->AllWeaponsMaxDamage();
                    for (float shot : this_damage)
                        DebugLogger() << "Weapons Dmg " << shot;
                    enemy_shots.insert(this_damage.begin(), this_damage.end());
                }
            }
        } else if (fleet_manager.ActiveFleetWnd()) {
            for (int fleet_id : fleet_manager.ActiveFleetWnd()->SelectedFleetIDs()) {
                if (const auto this_fleet = GetFleet(fleet_id)) {
                    chosen_ships.insert(this_fleet->ShipIDs().begin(), this_fleet->ShipIDs().end());
                }
            }
        }
        for (int ship_id : chosen_ships)
            if (const auto this_ship = GetShip(ship_id))
                if (!this_ship->SpeciesName().empty())
                    additional_species.insert(this_ship->SpeciesName());
        std::vector<std::string> species_list(additional_species.begin(), additional_species.end());
        detailed_description = GetDetailedDescriptionBase(design);


        // temporary ship to use for estimating design's meter values
        auto temp = GetUniverse().InsertTemp<Ship>(client_empire_id, design_id, "", client_empire_id);

        // apply empty species for 'Generic' entry
        GetUniverse().UpdateMeterEstimates(temp->ID());
        temp->Resupply();
        detailed_description.append(GetDetailedDescriptionStats(temp, design, enemy_DR, enemy_shots, cost));

        // apply various species to ship, re-calculating the meter values for each
        for (const std::string& species_name : species_list) {
            temp->SetSpecies(species_name);
            GetUniverse().UpdateMeterEstimates(temp->ID());
            temp->Resupply();
            detailed_description.append(GetDetailedDescriptionStats(temp, design, enemy_DR, enemy_shots, cost));
        }

        GetUniverse().Delete(temp->ID());
        GetUniverse().InhibitUniverseObjectSignals(false);



        // ships of this design
        std::vector<std::shared_ptr<const Ship>> design_ships;
        for (const auto& entry : Objects().ExistingShips()) {
            auto ship = std::dynamic_pointer_cast<const Ship>(entry.second);
            if (ship && ship->DesignID() == design_id)
                design_ships.push_back(ship);
        }
        if (!design_ships.empty()) {
            detailed_description += "\n\n" + UserString("SHIPS_OF_DESIGN");
            for (auto& ship : design_ships) {
                detailed_description += LinkTaggedIDText(VarText::SHIP_ID_TAG, ship->ID(),
                                                         ship->PublicName(client_empire_id)) + "  ";
            }
        } else {
            detailed_description += "\n\n" + UserString("NO_SHIPS_OF_DESIGN");
        }
    }

    void RefreshDetailPanelIncomplDesignTag(const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, std::weak_ptr<const ShipDesign>& inc_design)
    {
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        general_type = UserString("ENC_INCOMPETE_SHIP_DESIGN");

        auto incomplete_design = inc_design.lock();
        if (!incomplete_design)
            return;

        GetUniverse().InhibitUniverseObjectSignals(true);


        // incomplete design.  not yet in game universe; being created on design screen
        name = incomplete_design->Name();

        const std::string& design_icon = incomplete_design->Icon();
        if (design_icon.empty())
            texture = ClientUI::HullIcon(incomplete_design->Hull());
        else
            texture = ClientUI::GetTexture(ClientUI::ArtDir() / design_icon, true);

        int default_location_id = DefaultLocationForEmpire(client_empire_id);
        turns = incomplete_design->ProductionTime(client_empire_id, default_location_id);
        cost = incomplete_design->ProductionCost(client_empire_id, default_location_id);
        cost_units = UserString("ENC_PP");

        GetUniverse().InsertShipDesignID(new ShipDesign(*incomplete_design), client_empire_id, TEMPORARY_OBJECT_ID);

        float tech_level = boost::algorithm::clamp(CurrentTurn() / 400.0f, 0.0f, 1.0f);
        float typical_shot = 3 + 27 * tech_level;
        float enemy_DR = 20 * tech_level;
        DebugLogger() << "default enemy stats:: tech_level: " << tech_level << "   DR: " << enemy_DR << "   attack: " << typical_shot;
        std::set<float> enemy_shots;
        enemy_shots.insert(typical_shot);
        std::set<std::string> additional_species; // TODO: from currently selected planet and ship, if any
        const auto& map_wnd = ClientUI::GetClientUI()->GetMapWnd();
        if (const auto planet = GetPlanet(map_wnd->SelectedPlanetID())) {
            if (!planet->SpeciesName().empty())
                additional_species.insert(planet->SpeciesName());
        }
        FleetUIManager& fleet_manager = FleetUIManager::GetFleetUIManager();
        std::set<int> chosen_ships;
        int selected_ship = fleet_manager.SelectedShipID();
        if (selected_ship != INVALID_OBJECT_ID) {
            chosen_ships.insert(selected_ship);
            if (const auto this_ship = GetShip(selected_ship)) {
                if (!additional_species.empty() && ((this_ship->InitialMeterValue(METER_MAX_SHIELD) > 0) || !this_ship->OwnedBy(client_empire_id))) {
                    enemy_DR = this_ship->InitialMeterValue(METER_MAX_SHIELD);
                    DebugLogger() << "Using selected ship for enemy values, DR: " << enemy_DR;
                    enemy_shots.clear();
                    auto this_damage = this_ship->AllWeaponsMaxDamage();
                    for (float shot : this_damage)
                        DebugLogger() << "Weapons Dmg " << shot;
                    enemy_shots.insert(this_damage.begin(), this_damage.end());
                }
            }
        } else if (fleet_manager.ActiveFleetWnd()) {
            for (int fleet_id : fleet_manager.ActiveFleetWnd()->SelectedFleetIDs()) {
                if (const auto this_fleet = GetFleet(fleet_id)) {
                    chosen_ships.insert(this_fleet->ShipIDs().begin(), this_fleet->ShipIDs().end());
                }
            }
        }
        for (int ship_id : chosen_ships)
            if (const auto this_ship = GetShip(ship_id))
                if (!this_ship->SpeciesName().empty())
                    additional_species.insert(this_ship->SpeciesName());
        std::vector<std::string> species_list(additional_species.begin(), additional_species.end());
        detailed_description = GetDetailedDescriptionBase(incomplete_design.get());


        // temporary ship to use for estimating design's meter values
        auto temp = GetUniverse().InsertTemp<Ship>(client_empire_id, TEMPORARY_OBJECT_ID, "",
                                                   client_empire_id);

        // apply empty species for 'Generic' entry
        GetUniverse().UpdateMeterEstimates(temp->ID());
        temp->Resupply();
        detailed_description.append(GetDetailedDescriptionStats(temp, incomplete_design.get(), enemy_DR, enemy_shots, cost));

        // apply various species to ship, re-calculating the meter values for each
        for (const std::string& species_name : species_list) {
            temp->SetSpecies(species_name);
            GetUniverse().UpdateMeterEstimates(temp->ID());
            temp->Resupply();
            detailed_description.append(GetDetailedDescriptionStats(temp, incomplete_design.get(), enemy_DR, enemy_shots, cost));
        }


        GetUniverse().Delete(temp->ID());
        GetUniverse().DeleteShipDesign(TEMPORARY_OBJECT_ID);
        GetUniverse().InhibitUniverseObjectSignals(false);
    }

    void RefreshDetailPanelObjectTag(       const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        int id = boost::lexical_cast<int>(item_name);
        if (id == INVALID_OBJECT_ID)
            return;
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        auto obj = GetUniverseObject(id);
        if (!obj) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find UniverseObject with id " << item_name;
            return;
        }

        detailed_description = obj->Dump();
        name = obj->PublicName(client_empire_id);
        general_type = GeneralTypeOfObject(obj->ObjectType());
        if (general_type.empty()) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't interpret object: " << obj->Name() << " (" << item_name << ")";
            return;
        }
    }

    std::unordered_set<std::string> ReportedSpeciesForPlanet(std::shared_ptr<Planet> planet) {
        std::unordered_set<std::string> retval;

        auto empire_id = HumanClientApp::GetApp()->EmpireID();
        auto empire = HumanClientApp::GetApp()->GetEmpire(empire_id);
        if (!empire) {
            return retval;
        }

        // Collect species colonizing/environment hospitality information
        // start by building roster-- any species tagged as 'ALWAYS_REPORT' plus any species
        // represented in this empire's PopCenters
        for (const auto& entry : GetSpeciesManager()) {
            if (!entry.second)
                continue;
            const std::string& species_str = entry.first;
            const auto& species_tags = entry.second->Tags();
            if (species_tags.count(TAG_ALWAYS_REPORT)) {
                retval.insert(species_str);
                continue;
            }
            // Add extinct species if their tech is known
            // Extinct species and enabling tech should have an EXTINCT tag
            if (species_tags.count(TAG_EXTINCT)) {
                for (const auto& tech : empire->ResearchedTechs()) {
                    // Check for presence of tags in tech
                    const auto& tech_tags = GetTech(tech.first)->Tags();
                    if (tech_tags.count(species_str) && tech_tags.count(TAG_EXTINCT))
                    {
                        // Add the species and exit loop
                        retval.insert(species_str);
                        break;
                    }
                }
            }
        }

        for (const auto& pop_center_id : empire->GetPopulationPool().PopCenterIDs()) {
            auto obj = GetUniverseObject(pop_center_id);
            auto pc = std::dynamic_pointer_cast<const PopCenter>(obj);
            if (!pc)
                continue;

            const std::string& species_name = pc->SpeciesName();
            if (species_name.empty())
                continue;

            const Species* species = GetSpecies(species_name);
            if (!species)
                continue;

            // Exclude species that can't colonize UNLESS they
            // are already here (aka: it's their home planet). Showing them on
            // their own planet allows comparison vs other races, which might
            // be better suited to this planet. 
            if (species->CanColonize() || (planet && species_name == planet->SpeciesName()))
                retval.insert(species_name);
        }

        return retval;
    }

    std::multimap<float, std::pair<std::string, PlanetEnvironment>>
        SpeciesEnvByTargetPop(Planet& planet, const std::unordered_set<std::string>& species_names)
    {
        std::multimap<float, std::pair<std::string, PlanetEnvironment>> retval;

        if (species_names.empty())
            return retval;

        // store original state of planet
        auto original_planet_species = planet.SpeciesName();
        auto original_owner_id = planet.Owner();
        auto orig_initial_target_pop = planet.GetMeter(METER_TARGET_POPULATION)->Initial();

        auto planet_id = planet.ID();
        std::vector<int> planet_id_vec { planet_id };
        auto empire_id = HumanClientApp::GetApp()->EmpireID();

        GetUniverse().InhibitUniverseObjectSignals(true);

        for (const auto& species_name : species_names) {
            // Setting the planet's species allows all of it meters to reflect
            // species (and empire) properties, such as environment type
            // preferences and tech.
            // @see also: MapWnd::ApplyMeterEffectsAndUpdateMeters
            // NOTE: Overridding current or initial value of METER_TARGET_POPULATION prior to update
            //       results in incorrect estimates for at least effects with a min target population of 0
            planet.SetSpecies(species_name);
            planet.SetOwner(empire_id);
            GetUniverse().ApplyMeterEffectsAndUpdateMeters(planet_id_vec, false);

            const auto species = GetSpecies(species_name);
            auto planet_environment = PE_UNINHABITABLE;
            if (species)
                planet_environment = species->GetPlanetEnvironment(planet.Type());

            float planet_capacity = ((planet_environment == PE_UNINHABITABLE) ?
                                     0.0f : planet.CurrentMeterValue(METER_TARGET_POPULATION)); // want value after temporary meter update, so get current, not initial value of meter

            retval.insert({planet_capacity, {species_name, planet_environment}});
        }

        // restore planet to original state
        planet.SetSpecies(original_planet_species);
        planet.SetOwner(original_owner_id);
        planet.GetMeter(METER_TARGET_POPULATION)->Set(orig_initial_target_pop, orig_initial_target_pop);

        GetUniverse().InhibitUniverseObjectSignals(false);
        GetUniverse().ApplyMeterEffectsAndUpdateMeters(planet_id_vec, false);

        return retval;
    }

    GG::Pt HairSpaceExtent() {
        static GG::Pt retval;
        if (retval > GG::Pt(GG::X0, GG::Y0))
            return retval;

        GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE;
        auto font = ClientUI::GetFont();

        const std::string hair_space_str { u8"\u200A" };
        auto elems = font->ExpensiveParseFromTextToTextElements(hair_space_str, format);
        auto lines = font->DetermineLines(hair_space_str, format, GG::X(1 << 15), elems);
        retval = font->TextExtent(lines);
        return retval;
    }

    std::unordered_map<std::string, std::string> SpeciesSuitabilityColumn1(const std::unordered_set<std::string>& species_names) {
        std::unordered_map<std::string, std::string> retval;
        auto font = ClientUI::GetFont();

        for (const auto& species_name : species_names) {
            retval[species_name] = str(FlexibleFormat(UserString("ENC_SPECIES_PLANET_TYPE_SUITABILITY_COLUMN1"))
                                       % LinkTaggedText(VarText::SPECIES_TAG, species_name));
        }

        // determine widest column, storing extents of each row for later alignment
        GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE;
        GG::X longest_width { 0 };
        std::unordered_map<std::string, GG::Pt> column1_species_extents;
        for (const auto& it : retval) {
            auto text_elements = font->ExpensiveParseFromTextToTextElements(it.second, format);
            auto lines = font->DetermineLines(it.second, format, GG::X(1 << 15), text_elements);
            GG::Pt extent = font->TextExtent(lines);
            column1_species_extents[it.first] = extent;
            longest_width = std::max(longest_width, extent.x);
        }

        // align end of column with end of longest row
        auto hair_space_width = HairSpaceExtent().x;
        auto hair_space_width_str = std::to_string(Value(hair_space_width));
        const std::string hair_space_str { u8"\u200A" };
        for (auto& it : retval) {
            if (column1_species_extents.count(it.first) != 1) {
                ErrorLogger() << "No column1 extent stored for " << it.first;
                continue;
            }
            auto distance = longest_width - column1_species_extents.at(it.first).x;
            std::size_t num_spaces = Value(distance) / Value(hair_space_width);
            TraceLogger() << it.first << " Num spaces: " << std::to_string(Value(longest_width))
                          << " - " << std::to_string(Value(column1_species_extents.at(it.first).x))
                          << " = " << std::to_string(Value(distance))
                          << " / " << std::to_string(Value(hair_space_width))
                          << " = " << std::to_string(num_spaces);;
            for (std::size_t i = 0; i < num_spaces; ++i)
                it.second.append(hair_space_str);

            TraceLogger() << "Species Suitability Column 1:\n\t" << it.first << " \"" << it.second << "\"" << [&]() {
                    std::string out("");
                    auto col_val = Value(column1_species_extents.at(it.first).x);
                    out.append("\n\t\t(" + std::to_string(col_val) + " + (" + std::to_string(num_spaces) + " * " + hair_space_width_str);
                    out.append(") = " + std::to_string(col_val + (num_spaces * Value(hair_space_width))) + ")");
                    auto text_elements = font->ExpensiveParseFromTextToTextElements(it.second, format);
                    auto lines = font->DetermineLines(it.second, format, GG::X(1 << 15), text_elements);
                    out.append(" = " + std::to_string(Value(font->TextExtent(lines).x)));
                    return out;
                }();
        }

        return retval;
    }

    const std::vector<std::string>& PlanetEnvFilenames(PlanetType planet_type) {
        static std::unordered_map<int, std::vector<std::string>> filenames_by_type {
            std::make_pair(INVALID_PLANET_TYPE, std::vector<std::string>())
        };
        std::string planet_type_str = boost::lexical_cast<std::string>(planet_type);
        boost::algorithm::to_lower(planet_type_str);

        if (!filenames_by_type.count(planet_type)) {
            auto pe_type_func = [planet_type_str](const boost::filesystem::path& path) {
                return IsExistingFile(path) && boost::algorithm::starts_with(path.filename().string(), planet_type_str);
            };

            auto pe_path = ClientUI::ArtDir() / "encyclopedia/planet_environments";
            // retain only the filenames of each path
            for (const auto& file_path : PathsInDir(pe_path, pe_type_func, false))
                filenames_by_type[planet_type].emplace_back(PathToString(file_path.filename()));
        }

        if (filenames_by_type.count(planet_type))
            return filenames_by_type.at(planet_type);
        return filenames_by_type.at(INVALID_PLANET_TYPE);
    }

    void RefreshDetailPanelSuitabilityTag(const std::string& item_type, const std::string& item_name,
                                          std::string& name, std::shared_ptr<GG::Texture>& texture,
                                          std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                          float& cost, std::string& cost_units, std::string& general_type,
                                          std::string& specific_type, std::string& detailed_description,
                                          GG::Clr& color, GG::X width)
    {
        general_type = UserString("SP_PLANET_SUITABILITY");

        int planet_id = boost::lexical_cast<int>(item_name);
        auto planet = GetPlanet(planet_id);

        // show image of planet environment at the top of the suitability report
        const auto& filenames = PlanetEnvFilenames(planet->Type());
        if (!filenames.empty()) {
            auto env_img_tag = "<img src=\"encyclopedia/planet_environments/"
                               + filenames[planet_id % filenames.size()] + "\"></img>";
            TraceLogger() << "Suitability report env image tag \"" << env_img_tag << "\"";
            detailed_description.append(env_img_tag);
        }

        name = planet->PublicName(planet_id);

        auto species_names = ReportedSpeciesForPlanet(planet);
        auto target_population_species = SpeciesEnvByTargetPop(*planet.get(), species_names);
        auto species_suitability_column1 = SpeciesSuitabilityColumn1(species_names);

        bool positive_header_placed = false;
        bool negative_header_placed = false;

        for (auto it = target_population_species.rbegin(); it != target_population_species.rend(); ++it) {
            auto species_name_column1_it = species_suitability_column1.find(it->second.first);
            if (species_name_column1_it == species_suitability_column1.end())
                continue;

            if (it->first > 0) {
                if (!positive_header_placed) {
                    auto pos_header = str(FlexibleFormat(UserString("ENC_SUITABILITY_REPORT_POSITIVE_HEADER"))
                                          % planet->PublicName(planet_id));
                    TraceLogger() << "Suitability report positive header \"" << pos_header << "\"";
                    detailed_description.append(pos_header);
                    positive_header_placed = true;
                }

                auto pos_row = str(FlexibleFormat(UserString("ENC_SPECIES_PLANET_TYPE_SUITABILITY"))
                    % species_name_column1_it->second
                    % UserString(boost::lexical_cast<std::string>(it->second.second))
                    % (GG::RgbaTag(ClientUI::StatIncrColor()) + DoubleToString(it->first, 2, true) + "</rgba>"));
                TraceLogger() << "Suitability report positive row \"" << pos_row << "\"";
                detailed_description.append(pos_row);

            } else if (it->first <= 0) {
                if (!negative_header_placed) {
                    if (positive_header_placed)
                        detailed_description += "\n\n";

                    auto neg_header = str(FlexibleFormat(UserString("ENC_SUITABILITY_REPORT_NEGATIVE_HEADER"))
                                          % planet->PublicName(planet_id));
                    TraceLogger() << "Suitability report regative header \"" << neg_header << "\"";
                    detailed_description.append(neg_header);
                    negative_header_placed = true;
                }

                auto neg_row = str(FlexibleFormat(UserString("ENC_SPECIES_PLANET_TYPE_SUITABILITY"))
                    % species_name_column1_it->second
                    % UserString(boost::lexical_cast<std::string>(it->second.second))
                    % (GG::RgbaTag(ClientUI::StatDecrColor()) + DoubleToString(it->first, 2, true) + "</rgba>"));
                TraceLogger() << "Suitability report negative row \"" << neg_row << "\"";
                detailed_description.append(neg_row);
            }

            detailed_description += "\n";
        }

        detailed_description += UserString("ENC_SUITABILITY_REPORT_WHEEL_INTRO")
                                + "<img src=\"encyclopedia/EP_wheel.png\"></img>";
    }

    void RefreshDetailPanelSearchResultsTag(const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        general_type = UserString("SEARCH_RESULTS");
        texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "search.png");
        detailed_description = item_name;
    }

    void GetRefreshDetailPanelInfo(         const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, std::weak_ptr<const ShipDesign>& incomplete_design,
                                            GG::X width)
    {
        if (item_type == TextLinker::ENCYCLOPEDIA_TAG) {
            RefreshDetailPanelPediaTag(         item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);
        } else if (item_type == "ENC_SHIP_PART") {
            RefreshDetailPanelShipPartTag(      item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);
        } else if (item_type == "ENC_SHIP_HULL") {
            RefreshDetailPanelShipHullTag(      item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);
        } else if (item_type == "ENC_TECH") {
            RefreshDetailPanelTechTag(          item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);
        } else if (item_type == "ENC_BUILDING_TYPE") {
            RefreshDetailPanelBuildingTypeTag(  item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);
        } else if (item_type == "ENC_SPECIAL") {
            RefreshDetailPanelSpecialTag(       item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);
        } else if (item_type == "ENC_EMPIRE") {
            RefreshDetailPanelEmpireTag(        item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);
        } else if (item_type == "ENC_SPECIES") {
            RefreshDetailPanelSpeciesTag(       item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);
        } else if (item_type == "ENC_FIELD_TYPE") {
            RefreshDetailPanelFieldTypeTag(     item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);
        } else if (item_type == "ENC_METER_TYPE") {
            RefreshDetailPanelMeterTypeTag(     item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);

        } else if (item_type == "ENC_SHIP_DESIGN") {
            RefreshDetailPanelShipDesignTag(    item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);
        } else if (item_type == INCOMPLETE_DESIGN) {
            RefreshDetailPanelIncomplDesignTag( item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color,
                                                incomplete_design);
        } else if (item_type == UNIVERSE_OBJECT         || item_type == "ENC_BUILDING"  ||
                   item_type == "ENC_FIELD"             || item_type == "ENC_FLEET"     ||
                   item_type == "ENC_PLANET"            || item_type == "ENC_SHIP"      ||
                   item_type == "ENC_SYSTEM")
        {
            RefreshDetailPanelObjectTag(        item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);
        } else if (item_type == PLANET_SUITABILITY_REPORT) {
            RefreshDetailPanelSuitabilityTag(   item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color, width);
        } else if (item_type == TEXT_SEARCH_RESULTS) {
            RefreshDetailPanelSearchResultsTag( item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);
        } else if (item_type == TextLinker::GRAPH_TAG) {
            // should be handled externally...
        }
    }
}

void EncyclopediaDetailPanel::Refresh() {
    m_needs_refresh = true;
    RequirePreRender();
}

void EncyclopediaDetailPanel::PreRender() {
    CUIWnd::PreRender();

    if (m_needs_refresh) {
        m_needs_refresh = false;
        RefreshImpl();
    }

    DoLayout();
}

void EncyclopediaDetailPanel::RefreshImpl() {
    if (m_icon) {
        DetachChild(m_icon);
        m_icon = nullptr;
    }
    m_name_text->Clear();
    m_summary_text->Clear();
    m_cost_text->Clear();

    m_description_rich_text->SetText("");

    DetachChild(m_graph);

    // get details of item as applicable in order to set summary, cost, description TextControls
    std::string name;
    std::shared_ptr<GG::Texture> texture;
    std::shared_ptr<GG::Texture> other_texture;
    int turns = -1;
    float cost = 0.0f;
    std::string cost_units;             // "PP" or "RP" or empty string, depending on whether and what something costs
    std::string general_type;           // general type of thing being shown, eg. "Building" or "Ship Part"
    std::string specific_type;          // specific type of thing; thing's purpose.  eg. "Farming" or "Colonization".  May be left blank for things without specific types (eg. specials)
    std::string detailed_description;
    GG::Clr color(GG::CLR_ZERO);

    if (m_items.empty())
        return;

    GetRefreshDetailPanelInfo(m_items_it->first, m_items_it->second,
                              name, texture, other_texture, turns, cost, cost_units,
                              general_type, specific_type, detailed_description, color,
                              m_incomplete_design, ClientWidth());

    if (m_items_it->first == TextLinker::GRAPH_TAG) {
        const std::string& graph_id = m_items_it->second;

        const auto& stat_records = GetUniverse().GetStatRecords();

        auto stat_name_it = stat_records.find(graph_id);
        if (stat_name_it != stat_records.end()) {
            const auto& empire_lines = stat_name_it->second;
            m_graph->Clear();

            // add lines for each empire
            for (const auto& empire_linemap : empire_lines) {
                int empire_id = empire_linemap.first;

                GG::Clr empire_clr = GG::CLR_WHITE;
                if (const Empire* empire = GetEmpire(empire_id))
                    empire_clr = empire->Color();

                // convert formats...
                std::vector<std::pair<double, double>> line_data_pts;
                for (const auto& entry : empire_linemap.second)
                { line_data_pts.push_back({entry.first, entry.second}); }

                m_graph->AddSeries(line_data_pts, empire_clr);
            }

            m_graph->AutoSetRange();
            AttachChild(m_graph);
            m_graph->Show();
        }

        name = UserString(graph_id);
        general_type = UserString("ENC_GRAPH");
    }

    // Create Icons
    if (texture) {
        m_icon = GG::Wnd::Create<GG::StaticGraphic>(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        if (color != GG::CLR_ZERO)
            m_icon->SetColor(color);
    }

    if (m_icon) {
        m_icon->Show();
        AttachChild(m_icon);
    }

    // Set Text
    if (!name.empty())
        m_name_text->SetText(name);

    m_summary_text->SetText(str(FlexibleFormat(UserString("ENC_DETAIL_TYPE_STR"))
        % specific_type
        % general_type));

    if (color != GG::CLR_ZERO)
        m_summary_text->SetColor(color);

    if (cost != 0.0 && turns != -1) {
        m_cost_text->SetText(str(FlexibleFormat(UserString("ENC_COST_AND_TURNS_STR"))
            % DoubleToString(cost, 3, false)
            % cost_units
            % turns));
    }

    if (!detailed_description.empty())
        m_description_rich_text->SetText(detailed_description);

    m_scroll_panel->ScrollTo(GG::Y0);
}

void EncyclopediaDetailPanel::AddItem(const std::string& type, const std::string& name) {
    // if the actual item is not the last one, all aubsequented items are deleted
    if (!m_items.empty()) {
        if (m_items_it->first == type && m_items_it->second == name)
            return;
        auto end = m_items.end();
        --end;
        if (m_items_it != end) {
            auto i = m_items_it;
            ++i;
            m_items.erase(i, m_items.end());
        }
    }

    m_items.push_back({type, name});
    if (m_items.size() == 1)
        m_items_it = m_items.begin();
    else
        ++m_items_it;

    if (m_back_button->Disabled() && m_items.size() > 1) // enable Back button
        m_back_button->Disable(false); 

    if (!m_next_button->Disabled())                      // disable Next button
        m_next_button->Disable(true);

    Refresh();
    m_scroll_panel->ScrollTo(GG::Y0);   // revert to top for new screen
}

void EncyclopediaDetailPanel::PopItem() {
    if (!m_items.empty()) {
        m_items.pop_back();
        if (m_items_it == m_items.end() && m_items_it != m_items.begin())
            --m_items_it;
        Refresh();
        m_scroll_panel->ScrollTo(GG::Y0);   // revert to top for new screen
    }
}

void EncyclopediaDetailPanel::ClearItems() {
    m_items.clear();
    m_items_it = m_items.end();
    Refresh();
    m_scroll_panel->ScrollTo(GG::Y0);   // revert to top for new screen
}

void EncyclopediaDetailPanel::SetText(const std::string& text, bool lookup_in_stringtable) {
    if (m_items_it != m_items.end() && text == m_items_it->second)
        return;
    if (text == "ENC_INDEX")
        SetIndex();
    else
        AddItem(TextLinker::ENCYCLOPEDIA_TAG, (text.empty() || !lookup_in_stringtable) ? text : UserString(text));
}

void EncyclopediaDetailPanel::SetPlanet(int planet_id) {
    int current_item_id = INVALID_OBJECT_ID;
    if (m_items_it != m_items.end()) {
        try {
            current_item_id = boost::lexical_cast<int>(m_items_it->second);
        } catch (...) {
        }
    }
    if (planet_id == current_item_id)
        return;

    AddItem(PLANET_SUITABILITY_REPORT, std::to_string(planet_id));
}

void EncyclopediaDetailPanel::SetTech(const std::string& tech_name) {
    if (m_items_it != m_items.end() && tech_name == m_items_it->second)
        return;
    AddItem("ENC_TECH", tech_name);
}

void EncyclopediaDetailPanel::SetPartType(const std::string& part_name) {
    if (m_items_it != m_items.end() && part_name == m_items_it->second)
        return;
    AddItem("ENC_SHIP_PART", part_name);
}

void EncyclopediaDetailPanel::SetHullType(const std::string& hull_name) {
    if (m_items_it != m_items.end() && hull_name == m_items_it->second)
        return;
    AddItem("ENC_SHIP_HULL", hull_name);
}

void EncyclopediaDetailPanel::SetBuildingType(const std::string& building_name) {
    if (m_items_it != m_items.end() && building_name == m_items_it->second)
        return;
    AddItem("ENC_BUILDING_TYPE", building_name);
}

void EncyclopediaDetailPanel::SetSpecial(const std::string& special_name) {
    if (m_items_it != m_items.end() && special_name == m_items_it->second)
        return;
    AddItem("ENC_SPECIAL", special_name);
}

void EncyclopediaDetailPanel::SetSpecies(const std::string& species_name) {
    if (m_items_it != m_items.end() && species_name == m_items_it->second)
        return;
    AddItem("ENC_SPECIES", species_name);
}

void EncyclopediaDetailPanel::SetFieldType(const std::string& field_type_name) {
    if (m_items_it != m_items.end() && field_type_name == m_items_it->second)
        return;
    AddItem("ENC_FIELD_TYPE", field_type_name);
}

void EncyclopediaDetailPanel::SetMeterType(const std::string& meter_string) {
    if (meter_string.empty())
        return;
    AddItem("ENC_METER_TYPE", meter_string);
}

void EncyclopediaDetailPanel::SetObject(int object_id) {
    int current_item_id = INVALID_OBJECT_ID;
    if (m_items_it != m_items.end()) {
        try {
            current_item_id = boost::lexical_cast<int>(m_items_it->second);
        } catch (...) {
        }
    }
    if (object_id == current_item_id)
        return;
    AddItem(UNIVERSE_OBJECT, std::to_string(object_id));
}

void EncyclopediaDetailPanel::SetObject(const std::string& object_id) {
    if (m_items_it != m_items.end() && object_id == m_items_it->second)
        return;
    AddItem(UNIVERSE_OBJECT, object_id);
}

void EncyclopediaDetailPanel::SetEmpire(int empire_id) {
    int current_item_id = ALL_EMPIRES;
    if (m_items_it != m_items.end()) {
        try {
            current_item_id = boost::lexical_cast<int>(m_items_it->second);
        } catch (...) {
        }
    }
    if (empire_id == current_item_id)
        return;
    AddItem("ENC_EMPIRE", std::to_string(empire_id));
}

void EncyclopediaDetailPanel::SetEmpire(const std::string& empire_id) {
    if (m_items_it != m_items.end() && empire_id == m_items_it->second)
        return;
    AddItem("ENC_EMPIRE", empire_id);
}

void EncyclopediaDetailPanel::SetDesign(int design_id) {
    int current_item_id = INVALID_DESIGN_ID;
    if (m_items_it != m_items.end()) {
        try {
            current_item_id = boost::lexical_cast<int>(m_items_it->second);
        } catch (...) {
        }
    }
    if (design_id == current_item_id)
        return;
    AddItem("ENC_SHIP_DESIGN", std::to_string(design_id));
}

void EncyclopediaDetailPanel::SetDesign(const std::string& design_id) {
    if (m_items_it != m_items.end() && design_id == m_items_it->second)
        return;
    AddItem("ENC_SHIP_DESIGN", design_id);
}

void EncyclopediaDetailPanel::SetIncompleteDesign(std::weak_ptr<const ShipDesign> incomplete_design) {
    m_incomplete_design = incomplete_design;

    if (m_items_it == m_items.end() ||
        m_items_it->first != INCOMPLETE_DESIGN) {
        AddItem(INCOMPLETE_DESIGN, EMPTY_STRING);
    } else {
        Refresh();
    }
}

void EncyclopediaDetailPanel::SetGraph(const std::string& graph_id)
{ AddItem(TextLinker::GRAPH_TAG, graph_id); }

void EncyclopediaDetailPanel::SetIndex()
{ AddItem(TextLinker::ENCYCLOPEDIA_TAG, "ENC_INDEX"); }

void EncyclopediaDetailPanel::SetItem(std::shared_ptr<const Planet> planet)
{ SetPlanet(planet ? planet->ID() : INVALID_OBJECT_ID); }

void EncyclopediaDetailPanel::SetItem(const Tech* tech)
{ SetTech(tech ? tech->Name() : EMPTY_STRING); }

void EncyclopediaDetailPanel::SetItem(const PartType* part)
{ SetPartType(part ? part->Name() : EMPTY_STRING); }

void EncyclopediaDetailPanel::SetItem(const HullType* hull_type)
{ SetHullType(hull_type ? hull_type->Name() : EMPTY_STRING); }

void EncyclopediaDetailPanel::SetItem(const BuildingType* building_type)
{ SetBuildingType(building_type ? building_type->Name() : EMPTY_STRING); }

void EncyclopediaDetailPanel::SetItem(const Special* special)
{ SetSpecial(special ? special->Name() : EMPTY_STRING); }

void EncyclopediaDetailPanel::SetItem(const Species* species)
{ SetSpecies(species ? species->Name() : EMPTY_STRING); }

void EncyclopediaDetailPanel::SetItem(const FieldType* field_type)
{ SetFieldType(field_type ? field_type->Name() : EMPTY_STRING); }

void EncyclopediaDetailPanel::SetItem(std::shared_ptr<const UniverseObject> obj)
{ SetObject(obj ? obj->ID() : INVALID_OBJECT_ID); }

void EncyclopediaDetailPanel::SetItem(const Empire* empire)
{ SetEmpire(empire ? empire->EmpireID() : ALL_EMPIRES); }

void EncyclopediaDetailPanel::SetItem(const ShipDesign* design)
{ SetDesign(design ? design->ID() : INVALID_DESIGN_ID); }

void EncyclopediaDetailPanel::SetItem(const MeterType& meter_type)
{ SetMeterType(boost::lexical_cast<std::string>(meter_type)); }

void EncyclopediaDetailPanel::SetEncyclopediaArticle(const std::string& name)
{ AddItem(TextLinker::ENCYCLOPEDIA_TAG, name); }

void EncyclopediaDetailPanel::OnIndex()
{ AddItem(TextLinker::ENCYCLOPEDIA_TAG, "ENC_INDEX"); }

void EncyclopediaDetailPanel::OnBack() {
    if (m_items_it != m_items.begin())
        --m_items_it;

    if (m_items_it == m_items.begin())              // disable Back button, if the beginning is reached
        m_back_button->Disable(true);
    if (m_next_button->Disabled())                  // enable Next button
        m_next_button->Disable(false);

    Refresh();
    m_scroll_panel->ScrollTo(GG::Y0);   // revert to top for new screen
}

void EncyclopediaDetailPanel::OnNext() {
    auto end = m_items.end();
    --end;
    if (m_items_it != end && !m_items.empty())
        ++m_items_it;

    if (m_items_it == end)                          // disable Next button, if the end is reached;
        m_next_button->Disable(true);
    if (m_back_button->Disabled())                  // enable Back button
        m_back_button->Disable(false);

    Refresh();
    m_scroll_panel->ScrollTo(GG::Y0);   // revert to top for new screen
}

void EncyclopediaDetailPanel::CloseClicked()
{ ClosingSignal(); }

bool EncyclopediaDetailPanel::EventFilter(GG::Wnd* w, const GG::WndEvent& event) {
    if (w == this)
        return false;

    if (event.Type() != GG::WndEvent::KeyPress)
        return false;

    this->HandleEvent(event);
    return true;
}
