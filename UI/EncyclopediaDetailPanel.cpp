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
#include "../universe/ShipDesign.h"
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
#include "../util/Directories.h"
#include "../util/VarText.h"
#include "../util/ScopedTimer.h"
#include "../client/human/HumanClientApp.h"
#include "../combat/CombatLogManager.h"
#include "../combat/CombatEvents.h"
#include "../parse/Parse.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/GUI.h>
#include <GG/RichText/RichText.h>
#include <GG/ScrollPanel.h>

#include <boost/algorithm/clamp.hpp>
#include <boost/algorithm/string.hpp>

using boost::io::str;

namespace {
    const GG::X TEXT_MARGIN_X(3);
    const GG::Y TEXT_MARGIN_Y(0);
    const int DESCRIPTION_PADDING(3);

    void    AddOptions(OptionsDB& db) {
        db.Add("UI.dump-effects-descriptions", UserStringNop("OPTIONS_DB_DUMP_EFFECTS_GROUPS_DESC"),  false,  Validator<bool>());
        db.Add("UI.encyclopedia.search.articles", UserStringNop("OPTIONS_DB_ENC_SEARCH_ARTICLE"),     true,   Validator<bool>());
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
    /** @content_tag{CTRL_EXTINCT} Added to both a species and their colony building.  Handles display in planet suitability report. **/
    const std::string TAG_EXTINCT = "CTRL_EXTINCT";
}

namespace {
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
                                                   std::string> >& list)
    {
        std::pair<std::string, std::string> meter_name = MeterValueLabelAndString(meter_type);

        if (meter_name.first.empty() || meter_name.second.empty())
            return;

        std::string link_string = LinkTaggedPresetText(VarText::METER_TYPE_TAG,
                                                       meter_name.second,
                                                       meter_name.first) + "\n";

        list.insert(std::make_pair(meter_name.first,
                                   std::make_pair(link_string,
                                                  "ENC_METER_TYPE")));
    }

    const std::vector<std::string>& GetSearchTextDirNames() {
        static std::vector<std::string> dir_names;
        if (dir_names.empty()) {
            dir_names.push_back("ENC_INDEX");
            dir_names.push_back("ENC_SHIP_PART");
            dir_names.push_back("ENC_SHIP_HULL");
            dir_names.push_back("ENC_TECH");
            dir_names.push_back("ENC_BUILDING_TYPE");
            dir_names.push_back("ENC_SPECIAL");
            dir_names.push_back("ENC_SPECIES");
            dir_names.push_back("ENC_FIELD_TYPE");
            dir_names.push_back("ENC_METER_TYPE");
            dir_names.push_back("ENC_EMPIRE");
            dir_names.push_back("ENC_SHIP_DESIGN");
            dir_names.push_back("ENC_SHIP");
            dir_names.push_back("ENC_MONSTER");
            dir_names.push_back("ENC_MONSTER_TYPE");
            dir_names.push_back("ENC_FLEET");
            dir_names.push_back("ENC_PLANET");
            dir_names.push_back("ENC_BUILDING");
            dir_names.push_back("ENC_SYSTEM");
            dir_names.push_back("ENC_FIELD");
            dir_names.push_back("ENC_GRAPH");
            dir_names.push_back("ENC_GALAXY_SETUP");
            //  dir_names.push_back("ENC_HOMEWORLDS");  // omitted due to weird formatting of article titles
        }
        return dir_names;
    }

    /** Find Encyclopedia article with given name
     * @param[in] name name entry of the article
     */
    const EncyclopediaArticle& GetPediaArticle(const std::string& name) {
        const std::map<std::string, std::vector<EncyclopediaArticle> >& articles = GetEncyclopedia().articles;
        for (const std::map<std::string, std::vector<EncyclopediaArticle>>::value_type& entry : articles) {
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
                                                          std::string> >& sorted_entries_list)
    {
        const Encyclopedia& encyclopedia = GetEncyclopedia();
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        if (dir_name == "ENC_INDEX") {
            // add entries consisting of links to pedia page lists of
            // articles of various types
            sorted_entries_list.insert(std::make_pair(UserString("ENC_SHIP_PART"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_SHIP_PART") + "\n",
                               "ENC_SHIP_PART")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_SHIP_HULL"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_SHIP_HULL") + "\n",
                               "ENC_SHIP_HULL")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_TECH"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_TECH") + "\n",
                               "ENC_TECH")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_BUILDING_TYPE"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_BUILDING_TYPE") + "\n",
                               "ENC_BUILDING_TYPE")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_SPECIAL"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_SPECIAL") + "\n",
                               "ENC_SPECIAL")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_SPECIES"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_SPECIES") + "\n",
                               "ENC_SPECIES")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_HOMEWORLDS"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_HOMEWORLDS") + "\n",
                               "ENC_HOMEWORLDS")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_FIELD_TYPE"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_FIELD_TYPE") + "\n",
                               "ENC_FIELD_TYPE")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_METER_TYPE"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_METER_TYPE") + "\n",
                               "ENC_METER_TYPE")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_EMPIRE"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_EMPIRE") + "\n",
                               "ENC_EMPIRE")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_SHIP_DESIGN"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_SHIP_DESIGN") + "\n",
                               "ENC_SHIP_DESIGN")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_SHIP"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_SHIP") + "\n",
                               "ENC_SHIP")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_MONSTER"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_MONSTER") + "\n",
                               "ENC_MONSTER")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_MONSTER_TYPE"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_MONSTER_TYPE") + "\n",
                               "ENC_MONSTER_TYPE")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_FLEET"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_FLEET") + "\n",
                               "ENC_FLEET")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_PLANET"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_PLANET") + "\n",
                               "ENC_PLANET")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_BUILDING"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_BUILDING") + "\n",
                               "ENC_BUILDING")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_SYSTEM"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_SYSTEM") + "\n",
                               "ENC_SYSTEM")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_FIELD"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_FIELD") + "\n",
                               "ENC_FIELD")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_GRAPH"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_GRAPH") + "\n",
                               "ENC_GRAPH")));
            sorted_entries_list.insert(std::make_pair(UserString("ENC_GALAXY_SETUP"),
                std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, "ENC_GALAXY_SETUP") + "\n",
                               "ENC_GALAXY_SETUP")));

            for (const std::map<std::string, std::vector<EncyclopediaArticle>>::value_type& entry : encyclopedia.articles) {
                // Do not add sub-categories
                const EncyclopediaArticle& article = GetPediaArticle(entry.first);
                // No article found or specifically a top-level category
                if (!article.category.empty() && article.category != "ENC_INDEX")
                    continue;

                sorted_entries_list.insert(std::make_pair(UserString(entry.first),
                    std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, entry.first) + "\n",
                                   entry.first)));
            }

        } else if (dir_name == "ENC_SHIP_PART") {
            for (const std::map<std::string, PartType*>::value_type& entry : GetPartTypeManager()) {
                sorted_entries_list.insert(std::make_pair(UserString(entry.first),
                    std::make_pair(LinkTaggedText(VarText::SHIP_PART_TAG, entry.first) + "\n",
                                   entry.first)));
            }

        } else if (dir_name == "ENC_SHIP_HULL") {
            for (const std::map<std::string, HullType*>::value_type& entry : GetHullTypeManager()) {
                sorted_entries_list.insert(std::make_pair(UserString(entry.first),
                    std::make_pair(LinkTaggedText(VarText::SHIP_HULL_TAG, entry.first) + "\n",
                                   entry.first)));
            }

        } else if (dir_name == "ENC_TECH") {
            std::map<std::string, std::string> userstring_tech_names;
            // sort tech names by user-visible name, so names are shown alphabetically in UI
            for (const std::string& tech_name : GetTechManager().TechNames()) {
                userstring_tech_names[UserString(tech_name)] = tech_name;
            }
            for (std::map<std::string, std::string>::value_type& tech_name : userstring_tech_names) {
                sorted_entries_list.insert(std::make_pair(tech_name.first,    // already iterating over userstring-looked-up names, so don't need to re-look-up-here
                    std::make_pair(LinkTaggedText(VarText::TECH_TAG, tech_name.second) + "\n",
                                   tech_name.second)));
            }

            } else if (dir_name == "ENC_BUILDING_TYPE") {
                for (const std::map<std::string, BuildingType*>::value_type& entry : GetBuildingTypeManager()) {
                    sorted_entries_list.insert(std::make_pair(UserString(entry.first),
                        std::make_pair(LinkTaggedText(VarText::BUILDING_TYPE_TAG, entry.first) + "\n",
                                       entry.first)));
                }

        } else if (dir_name == "ENC_SPECIAL") {
            for (const std::string& special_name : SpecialNames()) {
                sorted_entries_list.insert(std::make_pair(UserString(special_name),
                    std::make_pair(LinkTaggedText(VarText::SPECIAL_TAG, special_name) + "\n",
                                   special_name)));
            }

        } else if (dir_name == "ENC_SPECIES") {
            for (const std::map<std::string, Species*>::value_type& entry : GetSpeciesManager()) {
                sorted_entries_list.insert(std::make_pair(UserString(entry.first),
                    std::make_pair(LinkTaggedText(VarText::SPECIES_TAG, entry.first) + "\n",
                                   entry.first)));
            }

        } else if (dir_name == "ENC_HOMEWORLDS") {
            int client_empire_id = HumanClientApp::GetApp()->EmpireID();
            const SpeciesManager& species_manager = GetSpeciesManager();
            for (const std::map<std::string, Species*>::value_type& entry : species_manager) {
                Species* species = entry.second;
                std::set<int> known_homeworlds;
                //std::string species_entry = UserString(entry.first) + ":  ";
                std::string species_entry;
                std::string homeworld_info;
                species_entry += LinkTaggedText(VarText::SPECIES_TAG, entry.first) + " ";
                // homeworld
                if (species->Homeworlds().empty()) {
                    continue;
                } else {
                    species_entry += "(" + boost::lexical_cast<std::string>(species->Homeworlds().size()) + "):  ";
                    for (int homeworld_id : species->Homeworlds()) {
                        if (TemporaryPtr<const Planet> homeworld = GetPlanet(homeworld_id)) {
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
                std::vector<TemporaryPtr<const Planet> > species_occupied_planets;
                for (TemporaryPtr<Planet> planet : Objects().FindObjects<Planet>()) {
                    if ((planet->SpeciesName() == entry.first) && (known_homeworlds.find(planet->ID()) == known_homeworlds.end()))
                        species_occupied_planets.push_back(planet);
                }
                if (!species_occupied_planets.empty()) {
                    if (species_occupied_planets.size() >= 5) {
                        species_entry += "  |   " + boost::lexical_cast<std::string>(species_occupied_planets.size()) + " " + UserString("OCCUPIED_PLANETS");
                    } else {
                        species_entry += "  |   " + UserString("OCCUPIED_PLANETS") + ":  ";
                        for (TemporaryPtr<const Planet> planet : species_occupied_planets) {
                            species_entry += LinkTaggedIDText(VarText::PLANET_ID_TAG, planet->ID(), planet->PublicName(client_empire_id)) + "   ";
                        }
                    }
                }
                sorted_entries_list.insert(std::make_pair(UserString(entry.first),
                    std::make_pair(species_entry + "\n", entry.first)));
            }
            sorted_entries_list.insert(std::make_pair("⃠ ",
                std::make_pair("\n\n", "  ")));
            for (const std::map<std::string, Species*>::value_type& entry : species_manager) {
                Species* species = entry.second;
                if (species->Homeworlds().empty()) {
                    std::string species_entry = LinkTaggedText(VarText::SPECIES_TAG, entry.first) + ":  ";
                    species_entry += UserString("NO_HOMEWORLD");
                    sorted_entries_list.insert(std::make_pair( "⃠⃠" + std::string( "⃠ ") + UserString(entry.first),
                        std::make_pair(species_entry + "\n", entry.first)));
                }
            } 

        } else if (dir_name == "ENC_FIELD_TYPE") {
            for (const std::map<std::string, FieldType*>::value_type& entry : GetFieldTypeManager()) {
                sorted_entries_list.insert(std::make_pair(UserString(entry.first),
                    std::make_pair(LinkTaggedText(VarText::FIELD_TYPE_TAG, entry.first) + "\n",
                                   entry.first)));
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
            for (std::map<int, Empire*>::value_type& entry : Empires()) {
                sorted_entries_list.insert(std::make_pair(UserString(entry.second->Name()),
                    std::make_pair(LinkTaggedIDText(VarText::EMPIRE_ID_TAG, entry.first, entry.second->Name()) + "\n",
                                   boost::lexical_cast<std::string>(entry.first))));
            }

        } else if (dir_name == "ENC_SHIP_DESIGN") {
            for (Universe::ship_design_iterator it = GetUniverse().beginShipDesigns();
                 it != GetUniverse().endShipDesigns(); ++it)
            {
                if (it->second->IsMonster())
                    continue;
                sorted_entries_list.insert(std::make_pair(it->second->Name(),
                    std::make_pair(LinkTaggedIDText(VarText::DESIGN_ID_TAG, it->first, it->second->Name()) + "\n",
                                   boost::lexical_cast<std::string>(it->first))));
            }

        } else if (dir_name == "ENC_SHIP") {
            for (TemporaryPtr<const Ship> ship : Objects().FindObjects<Ship>()) {
                const std::string& ship_name = ship->PublicName(client_empire_id);
                sorted_entries_list.insert(std::make_pair(ship_name,
                    std::make_pair(LinkTaggedIDText(VarText::SHIP_ID_TAG, ship->ID(), ship_name) + "  ",
                                   boost::lexical_cast<std::string>(ship->ID()))));
            }

        } else if (dir_name == "ENC_MONSTER") {
            for (TemporaryPtr<const Ship> ship : Objects().FindObjects<Ship>()) {
                if (!ship->IsMonster())
                    continue;
                const std::string& ship_name = ship->PublicName(client_empire_id);
                sorted_entries_list.insert(std::make_pair(ship_name,
                    std::make_pair(LinkTaggedIDText(VarText::SHIP_ID_TAG, ship->ID(), ship_name) + "  ",
                                   boost::lexical_cast<std::string>(ship->ID()))));
            }

        } else if (dir_name == "ENC_MONSTER_TYPE") {
            for (Universe::ship_design_iterator it = GetUniverse().beginShipDesigns(); it != GetUniverse().endShipDesigns(); ++it)
                if (it->second->IsMonster())
                    sorted_entries_list.insert(std::make_pair(it->second->Name(),
                        std::make_pair(LinkTaggedIDText(VarText::DESIGN_ID_TAG, it->first, it->second->Name()) + "\n",
                                       boost::lexical_cast<std::string>(it->first))));

        } else if (dir_name == "ENC_FLEET") {
            for (TemporaryPtr<const Fleet> fleet : Objects().FindObjects<Fleet>()) {
                const std::string& flt_name = fleet->PublicName(client_empire_id);
                sorted_entries_list.insert(std::make_pair(flt_name,
                    std::make_pair(LinkTaggedIDText(VarText::FLEET_ID_TAG, fleet->ID(), flt_name) + "  ",
                                   boost::lexical_cast<std::string>(fleet->ID()))));
            }

        } else if (dir_name == "ENC_PLANET") {
            for (TemporaryPtr<const Planet> planet : Objects().FindObjects<Planet>()) {
                const std::string& plt_name = planet->PublicName(client_empire_id);
                sorted_entries_list.insert(std::make_pair(plt_name,
                    std::make_pair(LinkTaggedIDText(VarText::PLANET_ID_TAG, planet->ID(), plt_name) + "  ",
                                   boost::lexical_cast<std::string>(planet->ID()))));
            }

        } else if (dir_name == "ENC_BUILDING") {
            for (TemporaryPtr<const Building> building : Objects().FindObjects<Building>()) {
                const std::string& bld_name = building->PublicName(client_empire_id);
                sorted_entries_list.insert(std::make_pair(bld_name,
                    std::make_pair(LinkTaggedIDText(VarText::BUILDING_ID_TAG, building->ID(), bld_name) + "  ",
                                   boost::lexical_cast<std::string>(building->ID()))));
            }

        } else if (dir_name == "ENC_SYSTEM") {
            for (TemporaryPtr<const System> system : Objects().FindObjects<System>()) {
                const std::string& sys_name = system->ApparentName(client_empire_id);
                sorted_entries_list.insert(std::make_pair(sys_name,
                    std::make_pair(LinkTaggedIDText(VarText::SYSTEM_ID_TAG, system->ID(), sys_name) + "  ",
                                   boost::lexical_cast<std::string>(system->ID()))));
            }

        } else if (dir_name == "ENC_FIELD") {
            for (TemporaryPtr<const Field> field : Objects().FindObjects<Field>()) {
                const std::string& field_name = field->Name();
                sorted_entries_list.insert(std::make_pair(field_name,
                    std::make_pair(LinkTaggedIDText(VarText::FIELD_ID_TAG, field->ID(), field_name) + "  ",
                                   boost::lexical_cast<std::string>(field->ID()))));
            }

        } else if (dir_name == "ENC_GRAPH") {
            for (const std::map<std::string, std::map<int, std::map<int, double>>>::value_type& stat_record : GetUniverse().GetStatRecords()) {
                sorted_entries_list.insert(std::make_pair(UserString(stat_record.first),
                    std::make_pair(LinkTaggedText(TextLinker::GRAPH_TAG, stat_record.first) + "\n",
                                   stat_record.first)));
            }

        } else {
            // list categories
            std::map<std::string, std::vector<EncyclopediaArticle> >::const_iterator category_it =
                encyclopedia.articles.find(dir_name);
            if (category_it != encyclopedia.articles.end()) {
                for (const EncyclopediaArticle& article : category_it->second) {
                    sorted_entries_list.insert(std::make_pair(UserString(article.name),
                        std::make_pair(LinkTaggedText(TextLinker::ENCYCLOPEDIA_TAG, article.name) + "\n",
                                       article.name)));
                }
            }
        }
    }

    std::string PediaDirText(const std::string& dir_name) {
        std::string retval;

        // get sorted list of entries for requested directory
        std::multimap<std::string, std::pair<std::string, std::string> > sorted_entries_list;
        GetSortedPediaDirEntires(dir_name, sorted_entries_list);

        // add sorted entries to page text
        for (std::multimap<std::string, std::pair<std::string, std::string>>::value_type& entry : sorted_entries_list)
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

        virtual void    KeyPress(GG::Key key, boost::uint32_t key_code_point,
                                 GG::Flags<GG::ModKey> mod_keys)
        {
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

std::list <std::pair<std::string, std::string> >            EncyclopediaDetailPanel::m_items = std::list<std::pair<std::string, std::string> >(0);
std::list <std::pair<std::string, std::string> >::iterator  EncyclopediaDetailPanel::m_items_it = m_items.begin();

EncyclopediaDetailPanel::EncyclopediaDetailPanel(GG::Flags<GG::WndFlag> flags, const std::string& config_name) :
    CUIWnd(UserString("MAP_BTN_PEDIA"), flags, config_name, false),
    m_name_text(0),
    m_cost_text(0),
    m_summary_text(0),
    m_description_box(0),
    m_description_panel(0),
    m_icon(0),
    m_search_edit(0),
    m_graph(0),
    m_needs_refresh(false)
{
    const int PTS = ClientUI::Pts();
    const int NAME_PTS = PTS*3/2;
    const int SUMMARY_PTS = PTS*4/3;

    m_name_text =    new CUILabel("");
    m_cost_text =    new CUILabel("");
    m_summary_text = new CUILabel("");

    m_name_text->SetFont(ClientUI::GetBoldFont(NAME_PTS));
    m_summary_text->SetFont(ClientUI::GetFont(SUMMARY_PTS));

    boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";

    m_index_button = new CUIButton(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "uparrownormal.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "uparrowclicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "uparrowmouseover.png")));

    m_back_button = new CUIButton(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "leftarrownormal.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "leftarrowclicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "leftarrowmouseover.png")));

    m_next_button = new CUIButton(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "rightarrownormal.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "rightarrowclicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "rightarrowmouseover.png")));

    m_back_button->Disable();
    m_next_button->Disable();

    GG::Connect(m_index_button->LeftClickedSignal,  &EncyclopediaDetailPanel::OnIndex,                  this);
    GG::Connect(m_back_button->LeftClickedSignal,   &EncyclopediaDetailPanel::OnBack,                   this);
    GG::Connect(m_next_button->LeftClickedSignal,   &EncyclopediaDetailPanel::OnNext,                   this);

    m_description_box = new GG::RichText(GG::X(0), GG::Y(0), ClientWidth(), ClientHeight(), "", ClientUI::GetFont(), ClientUI::TextColor(),
                                         GG::FORMAT_TOP | GG::FORMAT_LEFT | GG::FORMAT_LINEWRAP | GG::FORMAT_WORDBREAK, GG::INTERACTIVE);
    m_description_panel = new GG::ScrollPanel(GG::X(0), GG::Y(0), ClientWidth(), ClientHeight(), m_description_box);

    // Copy default block factory.
    boost::shared_ptr<GG::RichText::BLOCK_FACTORY_MAP> factory_map(new GG::RichText::BLOCK_FACTORY_MAP(*GG::RichText::DefaultBlockFactoryMap()));
    CUILinkTextBlock::Factory* factory = new CUILinkTextBlock::Factory();
    // Wire this factory to produce links that talk to us.
    GG::Connect(factory->LinkClickedSignal,        &EncyclopediaDetailPanel::HandleLinkClick,          this);
    GG::Connect(factory->LinkDoubleClickedSignal,  &EncyclopediaDetailPanel::HandleLinkDoubleClick,    this);
    GG::Connect(factory->LinkRightClickedSignal,   &EncyclopediaDetailPanel::HandleLinkDoubleClick,    this);
    (*factory_map)[GG::RichText::PLAINTEXT_TAG] = factory;
    m_description_box->SetBlockFactoryMap(factory_map);
    m_description_box->SetPadding(DESCRIPTION_PADDING);

    m_description_panel->SetBackgroundColor(ClientUI::CtrlColor());

    m_graph = new GraphControl();
    m_graph->ShowPoints(false);

    SearchEdit* search_edit = new SearchEdit();
    m_search_edit = search_edit;
    GG::Connect(search_edit->TextEnteredSignal,     &EncyclopediaDetailPanel::HandleSearchTextEntered,  this);

    AttachChild(m_search_edit);
    AttachChild(m_graph);
    AttachChild(m_name_text);
    AttachChild(m_cost_text);
    AttachChild(m_summary_text);
    AttachChild(m_description_panel);
    AttachChild(m_index_button);
    AttachChild(m_back_button);
    AttachChild(m_next_button);

    SetChildClippingMode(ClipToWindow);
    DoLayout();
    MoveChildUp(m_graph);

    AddItem(TextLinker::ENCYCLOPEDIA_TAG, "ENC_INDEX");
}

EncyclopediaDetailPanel::~EncyclopediaDetailPanel() {
    if (m_graph && m_graph->Parent() != this)
    delete m_graph;
}

void EncyclopediaDetailPanel::DoLayout() {
    const int PTS = ClientUI::Pts();
    const int NAME_PTS = PTS*3/2;
    const int COST_PTS = PTS;
    const int SUMMARY_PTS = PTS*4/3;

    const int ICON_SIZE = 12 + NAME_PTS + COST_PTS + SUMMARY_PTS;

    const int BTN_WIDTH = 24;
    const int Y_OFFSET = 22;

    SectionedScopedTimer timer("EncyclopediaDetailPanel::DoLayout", boost::chrono::milliseconds(1));

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
    m_description_panel->SizeMove(ul, lr);
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

void EncyclopediaDetailPanel::HandleSearchTextEntered() {
    // search lists of articles for typed text
    const std::string& search_text = m_search_edit->Text();
    if (search_text.empty())
        return;

    std::string match_report;
    std::set<std::pair<std::string, std::string> > already_listed_results;  // pair of category / article name

    // assemble link text to all pedia entries, indexed by name
    std::vector<std::string> dir_names = GetSearchTextDirNames();
    for (const std::map<std::string, std::vector<EncyclopediaArticle>>::value_type& article : GetEncyclopedia().articles) {
        dir_names.push_back(article.first);
    }

    // map from human-readable-name to (link-text, category name)
    std::multimap<std::string, std::pair<std::string, std::string> > all_pedia_entries_list;


    // for every directory, get all entries, add to common multimap for ease of
    // repeated searching for different types of match...
    for (const std::string& type_text : dir_names) {
        std::multimap<std::string, std::pair<std::string, std::string> > sorted_entries_list;
        GetSortedPediaDirEntires(type_text, sorted_entries_list);
        for (std::multimap<std::string, std::pair<std::string, std::string>>::value_type& entry : sorted_entries_list) {
            all_pedia_entries_list.insert(entry);
        }
    }

    // find distinct words in search text
    std::set<std::string> words_in_search_text;
    for (const std::pair<GG::StrSize, GG::StrSize>& word_range : GG::GUI::GetGUI()->FindWordsStringIndices(search_text)) {
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


    // search for exact title matches
    match_report += UserString("ENC_SEARCH_EXACT_MATCHES") + "\n\n";
    for (const std::multimap<std::string, std::pair<std::string, std::string>>::value_type& entry : all_pedia_entries_list) {
        if (boost::iequals(entry.first, search_text)) {
            match_report += entry.second.first;
            already_listed_results.insert(std::make_pair(entry.second.second, entry.first));
        }
    }

    // search for full word matches in titles
    match_report += "\n" + UserString("ENC_SEARCH_WORD_MATCHES") + "\n\n";
    for (const std::multimap<std::string, std::pair<std::string, std::string>>::value_type& entry : all_pedia_entries_list) {
        std::set<std::pair<std::string, std::string> >::const_iterator dupe_it =
            already_listed_results.find(std::make_pair(entry.second.second, entry.first));
        if (dupe_it != already_listed_results.end())
            continue;

        for (const std::string& word : words_in_search_text) {
            if (GG::GUI::GetGUI()->ContainsWord(entry.first, word)) {
                match_report += entry.second.first;
                already_listed_results.insert(std::make_pair(entry.second.second, entry.first));
            }
        }
    }

    // search for partial word matches: searched-for words that appear in the
    // title text, not necessarily as a complete word
    match_report += "\n" + UserString("ENC_SEARCH_PARTIAL_MATCHES") + "\n\n";
    for (std::multimap<std::string, std::pair<std::string, std::string>>::value_type& entry : all_pedia_entries_list) {
        std::set<std::pair<std::string, std::string> >::const_iterator dupe_it =
            already_listed_results.find(std::make_pair(entry.second.second, entry.first));
        if (dupe_it != already_listed_results.end())
            continue;

        for (const std::string& word : words_in_search_text) {
            // reject searches in text for words less than 3 characters
            if (word.size() < 3)
                continue;
            if (boost::icontains(entry.first, word)) {
                match_report += entry.second.first;
                already_listed_results.insert(std::make_pair(entry.second.second, entry.first));
            }
        }
    }

    // search for matches within article text
    if (GetOptionsDB().Get<bool>("UI.encyclopedia.search.articles")) {
        match_report += "\n" + UserString("ENC_SEARCH_ARTICLE_MATCHES") + "\n\n";
        for (std::multimap<std::string, std::pair<std::string, std::string>>::value_type& entry : all_pedia_entries_list) {
            std::set<std::pair<std::string, std::string> >::const_iterator dupe_it =
                already_listed_results.find(std::make_pair(entry.second.second, entry.first));
            if (dupe_it != already_listed_results.end())
                continue;

            EncyclopediaArticle article_entry = GetPediaArticle(entry.second.second);
            if (boost::icontains(UserString(article_entry.description), search_text)) {
                match_report += entry.second.first;
                already_listed_results.insert(std::make_pair(entry.second.second, entry.first));
                break;
            }
        }
    }

    // compile list of articles into some dynamically generated search report text
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
        TemporaryPtr<const UniverseObject> location = GetUniverseObject(empire->CapitalID());
        // no capital?  scan through all objects to find one owned by this empire
        // TODO: only loop over planets?
        // TODO: pass in a location condition, and pick a location that matches it if possible
        if (!location) {
            for (TemporaryPtr<const UniverseObject> obj : Objects()) {
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

        for (const Tech* tech : GetTechManager()) {
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
        default:                return EMPTY_STRING;
        }
    }

    void RefreshDetailPanelPediaTag(        const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
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
                % boost::lexical_cast<std::string>(gsd.m_size)
                % TextForGalaxyShape(gsd.m_shape)
                % TextForGalaxySetupSetting(gsd.m_age)
                % TextForGalaxySetupSetting(gsd.m_starlane_freq)
                % TextForGalaxySetupSetting(gsd.m_planet_density)
                % TextForGalaxySetupSetting(gsd.m_specials_freq)
                % TextForGalaxySetupSetting(gsd.m_monster_freq)
                % TextForGalaxySetupSetting(gsd.m_native_freq)
                % TextForAIAggression(gsd.m_ai_aggr));

            return;
        }

        // search for article in custom pedia entries.
        for (const std::map<std::string, std::vector<EncyclopediaArticle>>::value_type& entry : GetEncyclopedia().articles) {
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

                break;
            }
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
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
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

        const std::set<std::string>& exclusions = part->Exclusions();
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

        std::vector<std::string> unlocked_by_techs = TechsThatUnlockItem(ItemSpec(UIT_SHIP_PART, item_name));
        if (!unlocked_by_techs.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            for (const std::string& tech_name : unlocked_by_techs)
            { detailed_description += LinkTaggedText(VarText::TECH_TAG, tech_name) + "  "; }
            detailed_description += "\n\n";
        }

        if (GetOptionsDB().Get<bool>("UI.dump-effects-descriptions")) {
            if (part->Location())
                detailed_description += "\n" + part->Location()->Dump();
            if (!part->Effects().empty())
                detailed_description += "\n" + Dump(part->Effects());
        }
    }

    void RefreshDetailPanelShipHullTag(     const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
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

        const std::set<std::string>& exclusions = hull->Exclusions();
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

        std::vector<std::string> unlocked_by_techs = TechsThatUnlockItem(ItemSpec(UIT_SHIP_HULL, item_name));
        if (!unlocked_by_techs.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            for (const std::string& tech_name : unlocked_by_techs)
            { detailed_description += LinkTaggedText(VarText::TECH_TAG, tech_name) + "  "; }
            detailed_description += "\n\n";
        }

        if (GetOptionsDB().Get<bool>("UI.dump-effects-descriptions")) {
            if (hull->Location())
                detailed_description += "\n" + hull->Location()->Dump();
            if (!hull->Effects().empty())
                detailed_description += "\n" + Dump(hull->Effects());
        }
    }

    void RefreshDetailPanelTechTag(         const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
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

        const std::set<std::string>& unlocked_techs = tech->UnlockedTechs();
        const std::vector<ItemSpec>& unlocked_items = tech->UnlockedItems();
        if (!unlocked_techs.empty() || !unlocked_items.empty())
            detailed_description += UserString("ENC_UNLOCKS");

        if (!unlocked_techs.empty()) {
            for (const std::string& tech_name : unlocked_techs) {
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

        if (GetOptionsDB().Get<bool>("UI.dump-effects-descriptions") && !tech->Effects().empty()) {
            detailed_description += "\n" + Dump(tech->Effects());
        }

        const std::set<std::string>& unlocked_by_techs = tech->Prerequisites();
        if (!unlocked_by_techs.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            for (const std::string& tech_name : unlocked_by_techs)
            { detailed_description += LinkTaggedText(VarText::TECH_TAG, tech_name) + "  "; }
            detailed_description += "\n\n";
        }
    }

    void RefreshDetailPanelBuildingTypeTag( const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
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
        const MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
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
            if (TemporaryPtr< const Planet > planet = GetPlanet(this_location_id)) {
                int local_cost = building_type->ProductionCost(client_empire_id, this_location_id);
                int local_time = building_type->ProductionTime(client_empire_id, this_location_id);
                std::string local_name = planet->Name();
                detailed_description += str(FlexibleFormat(UserString("ENC_AUTO_TIME_COST_VARIABLE_DETAIL_STR")) 
                                        % local_name % local_cost % cost_units % local_time);
            }
        }
        if (GetOptionsDB().Get<bool>("UI.dump-effects-descriptions")) {
            if (!building_type->ProductionCostTimeLocationInvariant()) {
                if (building_type->Cost() && !building_type->Cost()->ConstantExpr())
                    detailed_description += "\n" + building_type->Cost()->Dump();
                if (building_type->Time() && !building_type->Time()->ConstantExpr())
                    detailed_description += "\n" + building_type->Time()->Dump();
            }
            if (building_type->Location())
                detailed_description += "\n" + building_type->Location()->Dump();
            if (!building_type->Effects().empty())
                detailed_description += "\n" + Dump(building_type->Effects());
        }

        std::vector<std::string> unlocked_by_techs = TechsThatUnlockItem(ItemSpec(UIT_BUILDING, item_name));
        if (!unlocked_by_techs.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            for (const std::string& tech_name : unlocked_by_techs)
            { detailed_description += LinkTaggedText(VarText::TECH_TAG, tech_name) + "  "; }
            detailed_description += "\n\n";
        }
    }

    void RefreshDetailPanelSpecialTag(      const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
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
        std::vector<TemporaryPtr<const UniverseObject> > objects_with_special;
        for (TemporaryPtr<const UniverseObject> obj : Objects())
            if (obj->Specials().find(item_name) != obj->Specials().end())
                objects_with_special.push_back(obj);

        if (!objects_with_special.empty()) {
            detailed_description += "\n\n" + UserString("OBJECTS_WITH_SPECIAL");
            for (TemporaryPtr<const UniverseObject> obj : objects_with_special) {
                if (TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(obj))
                    detailed_description += LinkTaggedIDText(VarText::SHIP_ID_TAG, ship->ID(), ship->PublicName(client_empire_id)) + "  ";

                else if (TemporaryPtr<const Fleet> fleet = boost::dynamic_pointer_cast<const Fleet>(obj))
                    detailed_description += LinkTaggedIDText(VarText::FLEET_ID_TAG, fleet->ID(), fleet->PublicName(client_empire_id)) + "  ";

                else if (TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(obj))
                    detailed_description += LinkTaggedIDText(VarText::PLANET_ID_TAG, planet->ID(), planet->PublicName(client_empire_id)) + "  ";

                else if (TemporaryPtr<const Building> building = boost::dynamic_pointer_cast<const Building>(obj))
                    detailed_description += LinkTaggedIDText(VarText::BUILDING_ID_TAG, building->ID(), building->PublicName(client_empire_id)) + "  ";

                else if (TemporaryPtr<const System> system = boost::dynamic_pointer_cast<const System>(obj))
                    detailed_description += LinkTaggedIDText(VarText::SYSTEM_ID_TAG, system->ID(), system->PublicName(client_empire_id)) + "  ";

                else
                    detailed_description += obj->PublicName(client_empire_id) + "  ";
            }
            detailed_description += "\n";
        }

        if (GetOptionsDB().Get<bool>("UI.dump-effects-descriptions")) {
            if (special->Location())
                detailed_description += "\n" + special->Location()->Dump();
            if (!special->Effects().empty())
                detailed_description += "\n" + Dump(special->Effects());
        }
    }

    void RefreshDetailPanelEmpireTag(       const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
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

        // Empires
        name = empire->Name();
        TemporaryPtr<const Planet> capital = GetPlanet(empire->CapitalID());
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
            for (std::map<std::string, Meter>::const_iterator meter_it = empire->meter_begin();
                 meter_it != empire->meter_end(); ++meter_it)
            { detailed_description += UserString(meter_it->first) + ": " + DoubleToString(meter_it->second.Initial(), 3, false); }
        }

        // Planets
        std::vector<TemporaryPtr<UniverseObject> > empire_planets = Objects().FindObjects(OwnedVisitor<Planet>(empire_id));
        if (!empire_planets.empty()) {
            detailed_description += "\n\n" + UserString("OWNED_PLANETS");
            for (TemporaryPtr<const UniverseObject> obj : empire_planets) {
                detailed_description += LinkTaggedIDText(VarText::PLANET_ID_TAG, obj->ID(), obj->PublicName(client_empire_id)) + "  ";
            }
        } else {
            detailed_description += "\n\n" + UserString("NO_OWNED_PLANETS_KNOWN");
        }

        // Fleets
        std::vector<TemporaryPtr<const UniverseObject> > nonempty_empire_fleets;
        for (TemporaryPtr<UniverseObject> maybe_fleet : Objects().FindObjects(OwnedVisitor<Fleet>(empire_id))) {
            if (TemporaryPtr<const Fleet> fleet = boost::dynamic_pointer_cast<const Fleet>(maybe_fleet))
                if (!fleet->Empty())
                    nonempty_empire_fleets.push_back(fleet);
        }
        if (!nonempty_empire_fleets.empty()) {
            detailed_description += "\n\n" + UserString("OWNED_FLEETS") + "\n";
            for (TemporaryPtr<const UniverseObject> obj : nonempty_empire_fleets) {
                std::string fleet_link = LinkTaggedIDText(VarText::FLEET_ID_TAG, obj->ID(), obj->PublicName(client_empire_id));
                std::string system_link;
                if (TemporaryPtr<const System> system = GetSystem(obj->SystemID())) {
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

        // Misc. Statistics

        // empire destroyed ships...
        const std::map<int, int>&           empire_ships_destroyed = empire->EmpireShipsDestroyed();
        if (!empire_ships_destroyed.empty())
            detailed_description += "\n\n" + UserString("EMPIRE_SHIPS_DESTROYED");
        for (const std::map<int, int>::value_type& entry : empire_ships_destroyed) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            const Empire* target_empire = GetEmpire(entry.first);
            std::string target_empire_name;
            if (target_empire)
                target_empire_name = target_empire->Name();
            else
                target_empire_name = UserString("UNOWNED");

            detailed_description += "\n" + target_empire_name + " : " + num_str;
        }


        // ship designs destroyed
        const std::map<int, int>&           empire_designs_destroyed = empire->ShipDesignsDestroyed();
        if (!empire_designs_destroyed.empty())
            detailed_description += "\n\n" + UserString("SHIP_DESIGNS_DESTROYED");
        for (const std::map<int, int>::value_type& entry : empire_designs_destroyed) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            const ShipDesign* design = GetShipDesign(entry.first);
            std::string design_name;
            if (design)
                design_name = design->Name();
            else
                design_name = UserString("UNKNOWN");

            detailed_description += "\n" + design_name + " : " + num_str;
        }


        // species ships destroyed
        const std::map<std::string, int>&   species_ships_destroyed = empire->SpeciesShipsDestroyed();
        if (!species_ships_destroyed.empty())
            detailed_description += "\n\n" + UserString("SPECIES_SHIPS_DESTROYED");
        for (const std::map<std::string, int>::value_type& entry : species_ships_destroyed) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;;
        }


        // species planets invaded
        const std::map<std::string, int>&   species_planets_invaded = empire->SpeciesPlanetsInvaded();
        if (!species_planets_invaded.empty())
            detailed_description += "\n\n" + UserString("SPECIES_PLANETS_INVADED");
        for (const std::map<std::string, int>::value_type& entry : species_planets_invaded) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;
        }


        // species ships produced
        const std::map<std::string, int>&   species_ships_produced = empire->SpeciesShipsProduced();
        if (!species_ships_produced.empty())
            detailed_description += "\n\n" + UserString("SPECIES_SHIPS_PRODUCED");
        for (const std::map<std::string, int>::value_type& entry : species_ships_produced) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;
        }


        // ship designs produced
        const std::map<int, int>&           ship_designs_produced = empire->ShipDesignsProduced();
        if (!ship_designs_produced.empty())
            detailed_description += "\n\n" + UserString("SHIP_DESIGNS_PRODUCED");
        for (const std::map<int, int>::value_type& entry : ship_designs_produced) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            const ShipDesign* design = GetShipDesign(entry.first);
            std::string design_name;
            if (design)
                design_name = design->Name();
            else
                design_name = UserString("UNKNOWN");

            detailed_description += "\n" + design_name + " : " + num_str;
        }


        // species ships lost
        const std::map<std::string, int>&   species_ships_lost = empire->SpeciesShipsLost();
        if (!species_ships_lost.empty())
            detailed_description += "\n\n" + UserString("SPECIES_SHIPS_LOST");
        for (const std::map<std::string, int>::value_type& entry : species_ships_lost) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;
        }


        // ship designs lost
        const std::map<int, int>&           ship_designs_lost = empire->ShipDesignsLost();
        if (!ship_designs_lost.empty())
            detailed_description += "\n\n" + UserString("SHIP_DESIGNS_LOST");
        for (const std::map<int, int>::value_type& entry : ship_designs_lost) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            const ShipDesign* design = GetShipDesign(entry.first);
            std::string design_name;
            if (design)
                design_name = design->Name();
            else
                design_name = UserString("UNKNOWN");

            detailed_description += "\n" + design_name + " : " + num_str;
        }


        // species ships scrapped
        const std::map<std::string, int>&   species_ships_scrapped = empire->SpeciesShipsScrapped();
        if (!species_ships_scrapped.empty())
            detailed_description += "\n\n" + UserString("SPECIES_SHIPS_SCRAPPED");
        for (const std::map<std::string, int>::value_type& entry : species_ships_scrapped) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;
        }


        // ship designs scrapped
        const std::map<int, int>&           ship_designs_scrapped = empire->ShipDesignsScrapped();
        if (!ship_designs_scrapped.empty())
            detailed_description += "\n\n" + UserString("SHIP_DESIGNS_SCRAPPED");
        for (const std::map<int, int>::value_type& entry : ship_designs_scrapped) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            const ShipDesign* design = GetShipDesign(entry.first);
            std::string design_name;
            if (design)
                design_name = design->Name();
            else
                design_name = UserString("UNKNOWN");

            detailed_description += "\n" + design_name + " : " + num_str;
        }


        // species planets depopulated
        const std::map<std::string, int>&   species_planets_depoped = empire->SpeciesPlanetsDepoped();
        if (!species_planets_depoped.empty())
            detailed_description += "\n\n" + UserString("SPECIES_PLANETS_DEPOPED");
        for (const std::map<std::string, int>::value_type& entry : species_planets_depoped) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;
        }


        // species planets bombed
        const std::map<std::string, int>&   species_planets_bombed = empire->SpeciesPlanetsBombed();
        if (!species_planets_bombed.empty())
            detailed_description += "\n\n" + UserString("SPECIES_PLANETS_BOMBED");
        for (const std::map<std::string, int>::value_type& entry : species_planets_bombed) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            std::string species_name;
            if (entry.first.empty())
                species_name = UserString("NONE");
            else
                species_name = UserString(entry.first);
            detailed_description += "\n" + species_name + " : " + num_str;
        }


        // buildings produced
        const std::map<std::string, int>&   building_types_produced = empire->BuildingTypesProduced();
        if (!building_types_produced.empty())
            detailed_description += "\n\n" + UserString("BUILDING_TYPES_PRODUCED");
        for (const std::map<std::string, int>::value_type& entry : building_types_produced) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            std::string building_type_name;
            if (entry.first.empty())
                building_type_name = UserString("NONE");
            else
                building_type_name = UserString(entry.first);
            detailed_description += "\n" + building_type_name + " : " + num_str;
        }


        // buildings scrapped
        const std::map<std::string, int>&   building_types_scrapped = empire->BuildingTypesScrapped();
        if (!building_types_scrapped.empty())
            detailed_description += "\n\n" + UserString("BUILDING_TYPES_SCRAPPED");
        for (const std::map<std::string, int>::value_type& entry : building_types_scrapped) {
            std::string num_str = boost::lexical_cast<std::string>(entry.second);
            std::string building_type_name;
            if (entry.first.empty())
                building_type_name = UserString("NONE");
            else
                building_type_name = UserString(entry.first);
            detailed_description += "\n" + building_type_name + " : " + num_str;
        }
    }

    void RefreshDetailPanelSpeciesTag(      const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
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
        const std::map<PlanetType, PlanetEnvironment>& pt_env_map = species->PlanetEnvironments();
        if (!pt_env_map.empty()) {
            detailed_description += UserString("ENVIRONMENTAL_PREFERENCES") + "\n";
            for (const std::map<PlanetType, PlanetEnvironment>::value_type& pt_env : pt_env_map) {
                detailed_description += UserString(boost::lexical_cast<std::string>(pt_env.first)) + " : " +
                                        UserString(boost::lexical_cast<std::string>(pt_env.second)) + "\n";
                // add blank line between cyclical environments and asteroids and gas giants
                if (pt_env.first == PT_OCEAN) {
                    detailed_description += "\n";
                }
            }
        } else {
            detailed_description += "\n";
        }

        if (GetOptionsDB().Get<bool>("UI.dump-effects-descriptions") && !species->Effects().empty()) {
            detailed_description += "\n" + Dump(species->Effects());
        }

        // Long description
        detailed_description += "\n";
        detailed_description += UserString(species->Description());

        // homeworld
        detailed_description += "\n";
        if (species->Homeworlds().empty()) {
            detailed_description += UserString("NO_HOMEWORLD") + "\n";
        } else {
            detailed_description += UserString("HOMEWORLD") + "\n";
            for (int hw_id : species->Homeworlds()) {
                if (TemporaryPtr<const Planet> homeworld = GetPlanet(hw_id))
                    detailed_description += LinkTaggedIDText(VarText::PLANET_ID_TAG, hw_id,
                                                             homeworld->PublicName(client_empire_id)) + "\n";
                else
                    detailed_description += UserString("UNKNOWN_PLANET") + "\n";
            }
        }

        // occupied planets
        std::vector<TemporaryPtr<const Planet> > species_occupied_planets;
        std::map<std::string, std::map<int, float> >& species_object_populations = GetSpeciesManager().SpeciesObjectPopulations();
        std::map<std::string, std::map<int, float> >::const_iterator sp_op_it = species_object_populations.find(item_name);
        if (sp_op_it != species_object_populations.end()) {
            const std::map<int, float>& object_pops = sp_op_it->second;
            for (const std::map<int, float>::value_type& object_pop : object_pops) {
                TemporaryPtr<const Planet> plt = GetPlanet(object_pop.first);
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
            for (TemporaryPtr<const Planet> planet : species_occupied_planets) {
                detailed_description += LinkTaggedIDText(VarText::PLANET_ID_TAG, planet->ID(),
                                                         planet->PublicName(client_empire_id)) + "  ";
            }
            detailed_description += "\n";
        }

        // empire opinions
        const std::map<std::string, std::map<int, float> >& seom = GetSpeciesManager().GetSpeciesEmpireOpinionsMap();
        std::map<std::string, std::map<int, float> >::const_iterator species_it = seom.find(species->Name());
        if (species_it != seom.end()) {
            detailed_description += "\n" + UserString("OPINIONS_OF_EMPIRES") + "\n";
            for (const std::map<int, float>::value_type& entry : species_it->second) {
                const Empire* empire = GetEmpire(entry.first);
                if (!empire)
                    continue;
            }
        }

        // species opinions
        const std::map<std::string, std::map<std::string, float> >& ssom = GetSpeciesManager().GetSpeciesSpeciesOpinionsMap();
        std::map<std::string, std::map<std::string, float> >::const_iterator species_it2 = ssom.find(species->Name());
        if (species_it2 != ssom.end()) {
            detailed_description += "\n" + UserString("OPINIONS_OF_OTHER_SPECIES") + "\n";
            for (const std::map<std::string, float>::value_type& entry : species_it2->second) {
                const Species* species2 = GetSpecies(entry.first);
                if (!species2)
                    continue;

                detailed_description += UserString(species2->Name()) + " : " + DoubleToString(entry.second, 3, false) + "\n";
            }
        }

    }

    void RefreshDetailPanelFieldTypeTag(    const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
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

        if (GetOptionsDB().Get<bool>("UI.dump-effects-descriptions")) {
            if (!field_type->Effects().empty())
                detailed_description += "\n" + Dump(field_type->Effects());
        }
    }


    void RefreshDetailPanelMeterTypeTag(const std::string& item_type, const std::string& item_name,
                                        std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                        boost::shared_ptr<GG::Texture>& other_texture, int& turns,
                                        float& cost, std::string& cost_units, std::string& general_type,
                                        std::string& specific_type, std::string& detailed_description,
                                        GG::Clr& color)
    {
        MeterType meter_type = INVALID_METER_TYPE;
        std::istringstream item_ss(item_name);
        item_ss >> meter_type;

        texture = ClientUI::MeterIcon(meter_type);
        general_type = UserString("ENC_METER_TYPE");

        std::pair<std::string, std::string> meter_name = MeterValueLabelAndString(meter_type);

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
        for (std::map<std::string, int>::const_iterator part_it = non_empty_parts_count.begin();
                part_it != non_empty_parts_count.end(); ++part_it)
        {
            if (part_it != non_empty_parts_count.begin())
                parts_list += ", ";
            parts_list += LinkTaggedText(VarText::SHIP_PART_TAG, part_it->first);
            if (part_it->second > 1)
                parts_list += " x" + boost::lexical_cast<std::string>(part_it->second);
        }
         return str(FlexibleFormat(UserString("ENC_SHIP_DESIGN_DESCRIPTION_BASE_STR"))
        % design->Description()
        % hull_link
        % parts_list);
    }

    std::string GetDetailedDescriptionStats(const TemporaryPtr<Ship> ship, const ShipDesign* design,
                                            float enemy_DR, std::set<float> enemy_shots, float cost)
    {
        //The strength of a fleet is approximately weapons * armor, or
        //(weapons - enemyShield) * armor / (enemyWeapons - shield). This
        //depends on the enemy's weapons and shields, so we estimate the
        //enemy technology from the turn.
        const std::string& species = ship->SpeciesName().empty() ? "Generic" : UserString(ship->SpeciesName());
        float structure = ship->CurrentMeterValue(METER_MAX_STRUCTURE);
        float shield = ship->CurrentMeterValue(METER_MAX_SHIELD);
        float attack = ship->TotalWeaponsDamage();
        float strength = std::pow(attack * structure, 0.6f);
        float typical_shot = *std::max_element(enemy_shots.begin(), enemy_shots.end());
        float typical_strength = std::pow(ship->TotalWeaponsDamage(enemy_DR) * structure * typical_shot / std::max(typical_shot - shield, 0.001f), 0.6f);
        return (FlexibleFormat(UserString("ENC_SHIP_DESIGN_DESCRIPTION_STATS_STR"))
            % ""
            % ""
            % ""
            % ""
            % ""
            % ""
            % ""
            % ship->CurrentMeterValue(METER_MAX_STRUCTURE)
            % ship->CurrentMeterValue(METER_MAX_SHIELD)
            % ship->CurrentMeterValue(METER_DETECTION)
            % ship->CurrentMeterValue(METER_STEALTH)
            % ship->CurrentMeterValue(METER_SPEED)
            % ship->CurrentMeterValue(METER_SPEED)
            % ship->CurrentMeterValue(METER_MAX_FUEL)
            % design->ColonyCapacity()
            % ship->TroopCapacity()
            % attack
            % species
            % strength % (strength / cost)
            % typical_shot
            % enemy_DR
            % typical_strength % (typical_strength / cost)).str();
    }

    void RefreshDetailPanelShipDesignTag(   const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
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
        std::set<std::string> additional_species; // from currently selected planet and fleets, if any
        const MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
        if (const TemporaryPtr<Planet> planet = GetPlanet(map_wnd->SelectedPlanetID())) {
            if (!planet->SpeciesName().empty())
                additional_species.insert(planet->SpeciesName());
        }
        FleetUIManager& fleet_manager = FleetUIManager::GetFleetUIManager();
        std::set<int> chosen_ships;
        int selected_ship = fleet_manager.SelectedShipID();
        const FleetWnd* fleet_wnd = FleetUIManager::GetFleetUIManager().ActiveFleetWnd();
        if ((selected_ship == INVALID_OBJECT_ID) && fleet_wnd) {
            std::set< int > selected_fleets = fleet_wnd->SelectedFleetIDs();
            std::set< int > selected_ships = fleet_wnd->SelectedShipIDs();
            if (selected_ships.size() > 0)
                selected_ship = *selected_ships.begin();
            else {
                int selected_fleet_id = INVALID_OBJECT_ID;
                if (selected_fleets.size() == 1)
                    selected_fleet_id = *selected_fleets.begin();
                else if (fleet_wnd->FleetIDs().size() > 0)
                    selected_fleet_id = *fleet_wnd->FleetIDs().begin();
                if (TemporaryPtr< Fleet > selected_fleet = GetFleet(selected_fleet_id))
                    if (!selected_fleet->ShipIDs().empty())
                        selected_ship = *selected_fleet->ShipIDs().begin();
            }
        }

        if (selected_ship != INVALID_OBJECT_ID) {
            chosen_ships.insert(selected_ship);
            if (const TemporaryPtr< Ship > this_ship = GetShip(selected_ship)) {
                if (!this_ship->SpeciesName().empty())
                    additional_species.insert(this_ship->SpeciesName());
                if (!this_ship->OwnedBy(client_empire_id)) {
                    enemy_DR = this_ship->CurrentMeterValue(METER_MAX_SHIELD);
                    DebugLogger() << "Using selected ship for enemy values, DR: " << enemy_DR;
                    enemy_shots.clear();
                    std::vector< float > this_damage = this_ship->AllWeaponsMaxDamage();
                    for (float shot : this_damage)
                        DebugLogger() << "Weapons Dmg " << shot;
                    enemy_shots.insert(this_damage.begin(), this_damage.end());
                }
            }
        } else if (fleet_manager.ActiveFleetWnd()) {
            for (int fleet_id : fleet_manager.ActiveFleetWnd()->SelectedFleetIDs()) {
                if (const TemporaryPtr<Fleet> this_fleet = GetFleet(fleet_id)) {
                    chosen_ships.insert(this_fleet->ShipIDs().begin(), this_fleet->ShipIDs().end());
                }
            }
        }
        for (int ship_id : chosen_ships)
            if (const TemporaryPtr<Ship> this_ship = GetShip(ship_id))
                if (!this_ship->SpeciesName().empty())
                    additional_species.insert(this_ship->SpeciesName());
        std::vector<std::string> species_list(additional_species.begin(), additional_species.end());
        detailed_description = GetDetailedDescriptionBase(design);


        // temporary ship to use for estimating design's meter values
        TemporaryPtr<Ship> temp = GetUniverse().CreateShip(client_empire_id, design_id, "",
                                                           client_empire_id, TEMPORARY_OBJECT_ID);

        // apply empty species for 'Generic' entry
        GetUniverse().UpdateMeterEstimates(TEMPORARY_OBJECT_ID);
        temp->Resupply();
        detailed_description.append(GetDetailedDescriptionStats(temp, design, enemy_DR, enemy_shots, cost));

        // apply various species to ship, re-calculating the meter values for each
        for (const std::string& species_name : species_list) {
            temp->SetSpecies(species_name);
            GetUniverse().UpdateMeterEstimates(TEMPORARY_OBJECT_ID);
            temp->Resupply();
            detailed_description.append(GetDetailedDescriptionStats(temp, design, enemy_DR, enemy_shots, cost));
        }

        GetUniverse().Delete(TEMPORARY_OBJECT_ID);
        GetUniverse().InhibitUniverseObjectSignals(false);



        // ships of this design
        std::vector<TemporaryPtr<const Ship> > design_ships;
        for (std::map<int, TemporaryPtr<UniverseObject> >::iterator
             ship_it = Objects().ExistingShipsBegin();
             ship_it != Objects().ExistingShipsEnd(); ++ship_it)
        {
            TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(ship_it->second);
            if (ship && ship->DesignID() == design_id)
                design_ships.push_back(ship);
        }
        if (!design_ships.empty()) {
            detailed_description += "\n\n" + UserString("SHIPS_OF_DESIGN");
            for (TemporaryPtr<const Ship> ship : design_ships) {
                detailed_description += LinkTaggedIDText(VarText::SHIP_ID_TAG, ship->ID(),
                                                         ship->PublicName(client_empire_id)) + "  ";
            }
        } else {
            detailed_description += "\n\n" + UserString("NO_SHIPS_OF_DESIGN");
        }
    }

    void RefreshDetailPanelIncomplDesignTag(const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, boost::weak_ptr<const ShipDesign>& inc_design)
    {
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        general_type = UserString("ENC_INCOMPETE_SHIP_DESIGN");

        boost::shared_ptr<const ShipDesign> incomplete_design = inc_design.lock();
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

        GetUniverse().InsertShipDesignID(new ShipDesign(*incomplete_design), TEMPORARY_OBJECT_ID);

        float tech_level = boost::algorithm::clamp(CurrentTurn() / 400.0f, 0.0f, 1.0f);
        float typical_shot = 3 + 27 * tech_level;
        float enemy_DR = 20 * tech_level;
        DebugLogger() << "default enemy stats:: tech_level: " << tech_level << "   DR: " << enemy_DR << "   attack: " << typical_shot;
        std::set<float> enemy_shots;
        enemy_shots.insert(typical_shot);
        std::set<std::string> additional_species; // TODO: from currently selected planet and ship, if any
        const MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
        if (const TemporaryPtr<Planet> planet = GetPlanet(map_wnd->SelectedPlanetID())) {
            if (!planet->SpeciesName().empty())
                additional_species.insert(planet->SpeciesName());
        }
        FleetUIManager& fleet_manager = FleetUIManager::GetFleetUIManager();
        std::set<int> chosen_ships;
        int selected_ship = fleet_manager.SelectedShipID();
        if (selected_ship != INVALID_OBJECT_ID) {
            chosen_ships.insert(selected_ship);
            if (const TemporaryPtr< Ship > this_ship = GetShip(selected_ship)) {
                if (!additional_species.empty() && ((this_ship->CurrentMeterValue(METER_MAX_SHIELD) > 0) || !this_ship->OwnedBy(client_empire_id))) {
                    enemy_DR = this_ship->CurrentMeterValue(METER_MAX_SHIELD);
                    DebugLogger() << "Using selected ship for enemy values, DR: " << enemy_DR;
                    enemy_shots.clear();
                    std::vector< float > this_damage = this_ship->AllWeaponsMaxDamage();
                    for (float shot : this_damage)
                        DebugLogger() << "Weapons Dmg " << shot;
                    enemy_shots.insert(this_damage.begin(), this_damage.end());
                }
            }
        } else if (fleet_manager.ActiveFleetWnd()) {
            for (int fleet_id : fleet_manager.ActiveFleetWnd()->SelectedFleetIDs()) {
                if (const TemporaryPtr<Fleet> this_fleet = GetFleet(fleet_id)) {
                    chosen_ships.insert(this_fleet->ShipIDs().begin(), this_fleet->ShipIDs().end());
                }
            }
        }
        for (int ship_id : chosen_ships)
            if (const TemporaryPtr<Ship> this_ship = GetShip(ship_id))
                if (!this_ship->SpeciesName().empty())
                    additional_species.insert(this_ship->SpeciesName());
        std::vector<std::string> species_list(additional_species.begin(), additional_species.end());
        detailed_description = GetDetailedDescriptionBase(incomplete_design.get());


        // temporary ship to use for estimating design's meter values
        TemporaryPtr<Ship> temp = GetUniverse().CreateShip(client_empire_id, TEMPORARY_OBJECT_ID, "",
                                                           client_empire_id, TEMPORARY_OBJECT_ID);

        // apply empty species for 'Generic' entry
        GetUniverse().UpdateMeterEstimates(TEMPORARY_OBJECT_ID);
        temp->Resupply();
        detailed_description.append(GetDetailedDescriptionStats(temp, incomplete_design.get(), enemy_DR, enemy_shots, cost));

        // apply various species to ship, re-calculating the meter values for each
        for (const std::string& species_name : species_list) {
            temp->SetSpecies(species_name);
            GetUniverse().UpdateMeterEstimates(TEMPORARY_OBJECT_ID);
            temp->Resupply();
            detailed_description.append(GetDetailedDescriptionStats(temp, incomplete_design.get(), enemy_DR, enemy_shots, cost));
        }


        GetUniverse().Delete(TEMPORARY_OBJECT_ID);
        GetUniverse().DeleteShipDesign(TEMPORARY_OBJECT_ID);
        GetUniverse().InhibitUniverseObjectSignals(false);
    }

    void RefreshDetailPanelObjectTag(       const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        int id = boost::lexical_cast<int>(item_name);
        if (id == INVALID_OBJECT_ID)
            return;
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        TemporaryPtr<const UniverseObject> obj = GetUniverseObject(id);
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

    void RefreshDetailPanelSuitabilityTag(  const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, GG::X width)
    {
        general_type = UserString("SP_PLANET_SUITABILITY");

        int planet_id = boost::lexical_cast<int>(item_name);
        TemporaryPtr<Planet> planet = GetPlanet(planet_id);

        // show image of planet environment at the top of the suitability report
        std::string planet_type = boost::lexical_cast<std::string>(planet->Type());
        boost::algorithm::to_lower(planet_type);
        detailed_description += "<img src=\"encyclopedia/planet_environments/" + planet_type + ".png\"></img>";

        std::string original_planet_species = planet->SpeciesName();
        int original_owner_id = planet->Owner();
        float orig_initial_target_pop = planet->GetMeter(METER_TARGET_POPULATION)->Initial();
        name = planet->PublicName(planet_id);

        int empire_id = HumanClientApp::GetApp()->EmpireID();
        Empire* empire = HumanClientApp::GetApp()->GetEmpire(empire_id);
        if (!empire) {
            return;
        }
        const std::vector<int> pop_center_ids = empire->GetPopulationPool().PopCenterIDs();

        std::set<std::string> species_names;
        std::map<std::string, std::pair<PlanetEnvironment, float> > population_counts;

        // Collect species colonizing/environment hospitality information
        // start by building roster-- any species tagged as 'ALWAYS_REPORT' plus any species
        // represented in this empire's PopCenters
        for (const std::map<std::string, Species*>::value_type& entry : GetSpeciesManager()) {
            if (!entry.second)
                continue;
            const std::string& species_str = entry.first;
            const std::set<std::string>& species_tags = entry.second->Tags();
            if (species_tags.find(TAG_ALWAYS_REPORT) != species_tags.end()) {
                species_names.insert(species_str);
                continue;
            }
            // Add extinct species if their (extinct) colony building is available
            // Extinct species should have an EXTINCT tag
            // The colony building should have an EXTINCT tag unless it is a starting unlock
            if (species_tags.find(TAG_EXTINCT) != species_tags.end()) {
                for (const std::map<std::string, BuildingType*>::value_type& entry : GetBuildingTypeManager()) {
                    const std::set<std::string>& bld_tags = entry.second->Tags();
                    // check if building matches tag requirements
                    if ((bld_tags.find(TAG_EXTINCT) != bld_tags.end()) &&
                        (bld_tags.find(species_str) != bld_tags.end()))
                    {
                        const std::string& bld_str = entry.first;
                        // Add if colony building is available to empire
                        if (empire->BuildingTypeAvailable(bld_str)) {
                            species_names.insert(species_str);
                        }
                        continue;
                    }
                }
            }
        }

        for (int pop_center_id : pop_center_ids) {
            TemporaryPtr<const UniverseObject> obj = GetUniverseObject(pop_center_id);
            TemporaryPtr<const PopCenter> pc = boost::dynamic_pointer_cast<const PopCenter>(obj);
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
            if (species->CanColonize() || species_name == planet->SpeciesName())
                species_names.insert(species_name);
        }

        boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
        GG::X max_species_name_column1_width(0);

        GetUniverse().InhibitUniverseObjectSignals(true);

        for (const std::string& species_name : species_names) {
            std::string species_name_column1 = str(FlexibleFormat(UserString("ENC_SPECIES_PLANET_TYPE_SUITABILITY_COLUMN1")) % UserString(species_name)); 
            GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE;
            std::vector<boost::shared_ptr<GG::Font::TextElement> > text_elements =
                font->ExpensiveParseFromTextToTextElements(species_name_column1, format);
            std::vector<GG::Font::LineData> lines = font->DetermineLines(
                species_name_column1, format, GG::X(1 << 15), text_elements);
            GG::Pt extent = font->TextExtent(lines);
            max_species_name_column1_width = std::max(extent.x, max_species_name_column1_width);

            // Setting the planet's species allows all of it meters to reflect
            // species (and empire) properties, such as environment type
            // preferences and tech.
            // @see also: MapWnd::UpdateMeterEstimates()
            planet->SetSpecies(species_name);
            planet->SetOwner(empire_id);
            planet->GetMeter(METER_TARGET_POPULATION)->Set(0.0, 0.0);
            GetUniverse().UpdateMeterEstimates(planet_id);

            const Species* species = GetSpecies(species_name);
            PlanetEnvironment planet_environment = PE_UNINHABITABLE;
            if (species)
                planet_environment = species->GetPlanetEnvironment(planet->Type());

            double planet_capacity = ((planet_environment == PE_UNINHABITABLE) ? 0 : planet->CurrentMeterValue(METER_TARGET_POPULATION));

            population_counts[species_name].first = planet_environment;
            population_counts[species_name].second = planet_capacity;
        }

        std::multimap<float, std::pair<std::string, PlanetEnvironment> > target_population_species;
        for (std::map<std::string, std::pair<PlanetEnvironment, float>>::value_type& entry : population_counts) {
            target_population_species.insert(std::make_pair(
                entry.second.second, std::make_pair(entry.first, entry.second.first)));
        }

        bool positive_header_placed = false;
        bool negative_header_placed = false;

        for (std::multimap<float, std::pair<std::string, PlanetEnvironment>>::const_reverse_iterator
             it = target_population_species.rbegin(); it != target_population_species.rend(); ++it)
        {
            std::string user_species_name = UserString(it->second.first);
            std::string species_name_column1 = str(FlexibleFormat(UserString("ENC_SPECIES_PLANET_TYPE_SUITABILITY_COLUMN1")) % LinkTaggedText(VarText::SPECIES_TAG, it->second.first));

            GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE;
            std::vector<boost::shared_ptr<GG::Font::TextElement> > text_elements =
                font->ExpensiveParseFromTextToTextElements(species_name_column1, format);
            std::vector<GG::Font::LineData> lines = font->DetermineLines(
                species_name_column1, format, GG::X(1 << 15), text_elements);
            GG::Pt extent = font->TextExtent(lines);
            while (extent.x < max_species_name_column1_width) {
                species_name_column1 += "\t";
                text_elements = font->ExpensiveParseFromTextToTextElements(species_name_column1, format);
                lines = font->DetermineLines(species_name_column1, format, GG::X(1 << 15), text_elements);
                extent = font->TextExtent(lines);
            }

            if (it->first > 0) {
                if (!positive_header_placed) {
                    detailed_description += str(FlexibleFormat(UserString("ENC_SUITABILITY_REPORT_POSITIVE_HEADER")) % planet->PublicName(planet_id));                    
                    positive_header_placed = true;
                }

                detailed_description += str(FlexibleFormat(UserString("ENC_SPECIES_PLANET_TYPE_SUITABILITY"))
                    % species_name_column1
                    % UserString(boost::lexical_cast<std::string>(it->second.second))
                    % (GG::RgbaTag(ClientUI::StatIncrColor()) + DoubleToString(it->first, 2, true) + "</rgba>"));

            } else if (it->first <= 0) {
                if (!negative_header_placed) {
                    if (positive_header_placed)
                        detailed_description += "\n\n";

                    detailed_description += str(FlexibleFormat(UserString("ENC_SUITABILITY_REPORT_NEGATIVE_HEADER")) % planet->PublicName(planet_id));                    
                    negative_header_placed = true;
                }

                detailed_description += str(FlexibleFormat(UserString("ENC_SPECIES_PLANET_TYPE_SUITABILITY"))
                    % species_name_column1
                    % UserString(boost::lexical_cast<std::string>(it->second.second))
                    % (GG::RgbaTag(ClientUI::StatDecrColor()) + DoubleToString(it->first, 2, true) + "</rgba>"));
            }

            detailed_description += "\n";
        }

        planet->SetSpecies(original_planet_species);
        planet->SetOwner(original_owner_id);
        planet->GetMeter(METER_TARGET_POPULATION)->Set(orig_initial_target_pop, orig_initial_target_pop);

        GetUniverse().InhibitUniverseObjectSignals(false);

        GetUniverse().UpdateMeterEstimates(planet_id);

        detailed_description += UserString("ENC_SUITABILITY_REPORT_WHEEL_INTRO") + "<img src=\"encyclopedia/EP_wheel.png\"></img>";
    }

    void RefreshDetailPanelSearchResultsTag(const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color)
    {
        general_type = UserString("SEARCH_RESULTS");
        texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "search.png");
        detailed_description = item_name;
    }

    void GetRefreshDetailPanelInfo(         const std::string& item_type, const std::string& item_name,
                                            std::string& name, boost::shared_ptr<GG::Texture>& texture,
                                            boost::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, boost::weak_ptr<const ShipDesign>& incomplete_design,
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
        DeleteChild(m_icon);
        m_icon = 0;
    }
    m_name_text->Clear();
    m_summary_text->Clear();
    m_cost_text->Clear();

    m_description_box->SetText("");

    DetachChild(m_graph);

    // get details of item as applicable in order to set summary, cost, description TextControls
    std::string name;
    boost::shared_ptr<GG::Texture> texture;
    boost::shared_ptr<GG::Texture> other_texture;
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

        const std::map<std::string, std::map<int, std::map<int, double> > >&
            stat_records = GetUniverse().GetStatRecords();

        std::map<std::string, std::map<int, std::map<int, double> > >::const_iterator
            stat_name_it = stat_records.find(graph_id);
        if (stat_name_it != stat_records.end()) {
            const std::map<int, std::map<int, double> >& empire_lines = stat_name_it->second;
            m_graph->Clear();

            // add lines for each empire
            for (const std::map<int, std::map<int, double>>::value_type& entry : empire_lines) {
                int empire_id = entry.first;

                GG::Clr empire_clr = GG::CLR_WHITE;
                if (const Empire* empire = GetEmpire(empire_id))
                    empire_clr = empire->Color();

                const std::map<int, double>& empire_line = entry.second;
                // convert formats...
                std::vector<std::pair<double, double> > line_data_pts;
                for (const std::map<int, double>::value_type& entry : empire_line)
                { line_data_pts.push_back(std::make_pair(entry.first, entry.second)); }

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
        m_icon = new GG::StaticGraphic(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
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
        m_description_box->SetText(detailed_description);

    m_description_panel->ScrollTo(GG::Y0);
}

void EncyclopediaDetailPanel::AddItem(const std::string& type, const std::string& name) {
    // if the actual item is not the last one, all aubsequented items are deleted
    if (!m_items.empty()) {
        if (m_items_it->first == type && m_items_it->second == name)
            return;
        std::list<std::pair <std::string, std::string> >::iterator end = m_items.end();
        --end;
        if (m_items_it != end) {
            std::list<std::pair <std::string, std::string> >::iterator i = m_items_it;
            ++i;
            m_items.erase(i, m_items.end());
        }
    }

    m_items.push_back(std::pair<std::string, std::string>(type, name));
    if (m_items.size() == 1)
        m_items_it = m_items.begin();
    else
        ++m_items_it;

    if (m_back_button->Disabled() && m_items.size() > 1) // enable Back button
        m_back_button->Disable(false); 

    if (!m_next_button->Disabled())                      // disable Next button
        m_next_button->Disable(true);

    Refresh();
    m_description_panel->ScrollTo(GG::Y0);   // revert to top for new screen
}

void EncyclopediaDetailPanel::PopItem() {
    if (!m_items.empty()) {
        m_items.pop_back();
        if (m_items_it == m_items.end() && m_items_it != m_items.begin())
            --m_items_it;
        Refresh();
        m_description_panel->ScrollTo(GG::Y0);   // revert to top for new screen
    }
}

void EncyclopediaDetailPanel::ClearItems() {
    m_items.clear();
    m_items_it = m_items.end();
    Refresh();
    m_description_panel->ScrollTo(GG::Y0);   // revert to top for new screen
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

    AddItem(PLANET_SUITABILITY_REPORT, boost::lexical_cast<std::string>(planet_id));
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
    AddItem(UNIVERSE_OBJECT, boost::lexical_cast<std::string>(object_id));
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
    AddItem("ENC_EMPIRE", boost::lexical_cast<std::string>(empire_id));
}

void EncyclopediaDetailPanel::SetEmpire(const std::string& empire_id) {
    if (m_items_it != m_items.end() && empire_id == m_items_it->second)
        return;
    AddItem("ENC_EMPIRE", empire_id);
}

void EncyclopediaDetailPanel::SetDesign(int design_id) {
    int current_item_id = ShipDesign::INVALID_DESIGN_ID;
    if (m_items_it != m_items.end()) {
        try {
            current_item_id = boost::lexical_cast<int>(m_items_it->second);
        } catch (...) {
        }
    }
    if (design_id == current_item_id)
        return;
    AddItem("ENC_SHIP_DESIGN", boost::lexical_cast<std::string>(design_id));
}

void EncyclopediaDetailPanel::SetDesign(const std::string& design_id) {
    if (m_items_it != m_items.end() && design_id == m_items_it->second)
        return;
    AddItem("ENC_SHIP_DESIGN", design_id);
}

void EncyclopediaDetailPanel::SetIncompleteDesign(boost::weak_ptr<const ShipDesign> incomplete_design) {
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

void EncyclopediaDetailPanel::SetItem(TemporaryPtr<const Planet> planet)
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

void EncyclopediaDetailPanel::SetItem(TemporaryPtr<const UniverseObject> obj)
{ SetObject(obj ? obj->ID() : INVALID_OBJECT_ID); }

void EncyclopediaDetailPanel::SetItem(const Empire* empire)
{ SetEmpire(empire ? empire->EmpireID() : ALL_EMPIRES); }

void EncyclopediaDetailPanel::SetItem(const ShipDesign* design)
{ SetDesign(design ? design->ID() : ShipDesign::INVALID_DESIGN_ID); }

void EncyclopediaDetailPanel::SetItem(const MeterType& meter_type)
{ SetMeterType(boost::lexical_cast<std::string>(meter_type)); }

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
    m_description_panel->ScrollTo(GG::Y0);   // revert to top for new screen
}

void EncyclopediaDetailPanel::OnNext() {
    std::list<std::pair <std::string, std::string> >::iterator end = m_items.end();
    --end;
    if (m_items_it != end && !m_items.empty())
        ++m_items_it;

    if (m_items_it == end)                          // disable Next button, if the end is reached;
        m_next_button->Disable(true);
    if (m_back_button->Disabled())                  // enable Back button
        m_back_button->Disable(false);

    Refresh();
    m_description_panel->ScrollTo(GG::Y0);   // revert to top for new screen
}

void EncyclopediaDetailPanel::CloseClicked()
{ ClosingSignal(); }
