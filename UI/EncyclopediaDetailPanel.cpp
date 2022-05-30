#include "EncyclopediaDetailPanel.h"

#include <unordered_map>
#include <boost/algorithm/clamp.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/locale/conversion.hpp>
#include <GG/GUI.h>
#include <GG/RichText/RichText.h>
#include <GG/ScrollPanel.h>
#include <GG/StaticGraphic.h>
#include <GG/Texture.h>
#include <GG/utf8/checked.h>
#include "CUIControls.h"
#include "CUILinkTextBlock.h"
#include "DesignWnd.h"
#include "FleetWnd.h"
#include "GraphControl.h"
#include "Hotkeys.h"
#include "LinkText.h"
#include "MapWnd.h"
#include "../client/human/GGHumanClientApp.h"
#include "../combat/CombatLogManager.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Government.h"
#include "../universe/Building.h"
#include "../universe/BuildingType.h"
#include "../universe/Condition.h"
#include "../universe/Effect.h"
#include "../universe/Encyclopedia.h"
#include "../universe/Field.h"
#include "../universe/FieldType.h"
#include "../universe/Fleet.h"
#include "../universe/NamedValueRefManager.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/Ship.h"
#include "../universe/ShipHull.h"
#include "../universe/ShipPart.h"
#include "../universe/Special.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/Tech.h"
#include "../universe/UniverseObjectVisitors.h"
#include "../universe/Universe.h"
#include "../universe/UnlockableItem.h"
#include "../universe/ValueRef.h"
#include "../util/Directories.h"
#include "../util/GameRules.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/ScopedTimer.h"
#include "../util/ThreadPool.h"
#include "../util/VarText.h"

using boost::io::str;

namespace {
    constexpr int DESCRIPTION_PADDING(3);

    void AddOptions(OptionsDB& db) {
        db.Add("resource.effects.description.shown", UserStringNop("OPTIONS_DB_DUMP_EFFECTS_GROUPS_DESC"), false);
        db.Add("ui.pedia.search.articles.enabled", UserStringNop("OPTIONS_DB_UI_ENC_SEARCH_ARTICLE"), true);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    const std::string EMPTY_STRING;
    constexpr std::string_view INCOMPLETE_DESIGN = "incomplete design";
    constexpr std::string_view UNIVERSE_OBJECT = "universe object";
    constexpr std::string_view PLANET_SUITABILITY_REPORT = "planet suitability report";
    constexpr std::string_view GRAPH = "data graph";
    constexpr std::string_view TEXT_SEARCH_RESULTS = "dynamic generated text";

    /** @content_tag{CTRL_ALWAYS_REPORT} Always display a species on a planet suitability report. **/
    constexpr std::string_view TAG_ALWAYS_REPORT = "CTRL_ALWAYS_REPORT";
    /** @content_tag{CTRL_EXTINCT} Added to both a species and their enabling tech.  Handles display in planet suitability report. **/
    constexpr std::string_view TAG_EXTINCT = "CTRL_EXTINCT";
    /** @content_tag{PEDIA_} Defines an encyclopedia category for the generated article of the containing content definition.  The category name should be postfixed to this tag. **/
    constexpr std::string_view TAG_PEDIA_PREFIX = "PEDIA_";
}

namespace {
    // Checks content \a tags for custom defined pedia category \a cat
    template <typename StringContainer, typename S>
    bool HasCustomCategory(const StringContainer& tags, const S& cat) {
        static constexpr auto len{TAG_PEDIA_PREFIX.length()};
        return std::any_of(tags.begin(), tags.end(), [&cat](std::string_view sv) {
            return sv.substr(0, len) == TAG_PEDIA_PREFIX &&
                sv.substr(len) == cat;
        });
    }

    // Checks content \a tags for any custom pedia categories
    template <typename StringContainer>
    bool HasCustomCategory(const StringContainer& tags) {
        static constexpr auto len{TAG_PEDIA_PREFIX.length()};
        return std::any_of(tags.begin(), tags.end(), [](std::string_view sv) {
            return sv.substr(0, len) == TAG_PEDIA_PREFIX;
        });
    }

    /** Retreive a value label and general string representation for @a meter_type
      * eg. {"METER_STEALTH_VALUE_LABEL", UserString("METER_STEALTH")} */
    auto MeterValueLabelAndString(MeterType meter_type) {
        std::pair<std::string_view, std::string_view> retval{"", to_string(meter_type)};

        if (meter_type == MeterType::INVALID_METER_TYPE)
            return retval;

        using signed_idx_type = std::underlying_type_t<MeterType>;
        static constexpr auto INVALID_IDX = static_cast<signed_idx_type>(MeterType::INVALID_METER_TYPE);
        static_assert(INVALID_IDX == -1);
        static constexpr auto NUM_IDX = static_cast<signed_idx_type>(MeterType::NUM_METER_TYPES);
        // TODO: additional static_assert that verifies that the numerical representation of
        //       all values of MeterType are -1 or higher. would work better with an
        //       IterateEnum implementation that works constexpr. as of this writing, it
        //       uses an std::vector, which is not constexpr OK in C++17

        static const auto label_value_strings = []() {
            std::array<std::string, NUM_IDX> retval{};
            for (size_t idx = 0; idx < NUM_IDX; ++idx)
                retval[idx] = std::string{to_string(MeterType(static_cast<signed_idx_type>(idx)))}
                                .append("_VALUE_LABEL");
            return retval;
        }();

        auto idx = static_cast<std::size_t>(static_cast<signed_idx_type>(meter_type));
        retval.first = label_value_strings[idx];

        if (UserStringExists(retval.first)) {
            retval.first = UserString(retval.first);
        } else {
            DebugLogger() << "No pedia entry found for value of Meter Type: "
                          << retval.second << "(" << meter_type << ")";
            retval.first = UserString(retval.second);
        }

        return retval;
    }

    void MeterTypeDirEntry(MeterType meter_type,
                           std::vector<std::pair<std::string_view,
                                                 std::pair<std::string, std::string>>>& list)
    {
        auto [value_label, string_rep] = MeterValueLabelAndString(meter_type);

        if (value_label.empty() || string_rep.empty())
            return;

        list.emplace_back(std::piecewise_construct,
                          std::forward_as_tuple(value_label),
                          std::forward_as_tuple(LinkTaggedPresetText(VarText::METER_TYPE_TAG, string_rep, value_label).append("\n"),
                                                string_rep));
    }

    const std::vector<std::string>& GetSearchTextDirNames() {
        static const std::vector<std::string> dir_names{
            "ENC_INDEX",        "ENC_SHIP_PART",    "ENC_SHIP_HULL",    "ENC_TECH",
            "ENC_POLICY",
            "ENC_BUILDING_TYPE","ENC_SPECIAL",      "ENC_SPECIES",      "ENC_FIELD_TYPE",
            "ENC_METER_TYPE",   "ENC_EMPIRE",       "ENC_SHIP_DESIGN",  "ENC_SHIP",
            "ENC_MONSTER",      "ENC_MONSTER_TYPE", "ENC_FLEET",        "ENC_PLANET",
            "ENC_BUILDING",     "ENC_SYSTEM",       "ENC_FIELD",        "ENC_GRAPH",
            "ENC_GALAXY_SETUP", "ENC_GAME_RULES",   "ENC_NAMED_VALUE_REF",
            "ENC_STRINGS"};
        //  "ENC_HOMEWORLDS" omitted due to weird formatting of article titles
        return dir_names;
    }

    /** Returns map from (Human-readable and thus sorted article name) to
        pair of (article link tag text, stringtable key for article category or
        subcategorization of it). Category is something like "ENC_TECH" and
        subcategorization is something like a tech category (eg. growth). */
    auto GetSortedPediaDirEntires(
        std::string_view dir_name,
        bool exclude_custom_categories_from_dir_name = true)
    {
        ScopedTimer subdir_timer(std::string{"GetSortedPediaDirEntires("}.append(dir_name).append(")"),
                                 true, std::chrono::milliseconds(20));

        std::vector<std::pair<std::string_view, std::pair<std::string, std::string>>> retval;

        const Encyclopedia& encyclopedia = GetEncyclopedia();
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
        const Universe& universe = GetUniverse();
        const ObjectMap& objects = universe.Objects();

        if (dir_name == "ENC_INDEX") {
            // add entries consisting of links to pedia page lists of
            // articles of various types
            for (const std::string& str : GetSearchTextDirNames()) {
                if (str == "ENC_INDEX")
                    continue;
                auto& us_name{UserString(str)};
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(us_name),
                                    std::forward_as_tuple(LinkTaggedPresetText(TextLinker::ENCYCLOPEDIA_TAG, str, us_name).append("\n"), str));
            }

            for (auto str : {"ENC_TEXTURES", "ENC_HOMEWORLDS"}) {
                auto& us_name{UserString(str)};
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(us_name),
                                    std::forward_as_tuple(LinkTaggedPresetText(TextLinker::ENCYCLOPEDIA_TAG, str, us_name).append("\n"), str));
            }

            for ([[maybe_unused]] auto& [category_name, article_vec] : encyclopedia.Articles()) {
                // Do not add sub-categories
                const EncyclopediaArticle& article = encyclopedia.GetArticleByKey(category_name);
                // No article found or specifically a top-level category
                if (!article.category.empty() && article.category != "ENC_INDEX")
                    continue;
                (void)article_vec; // quiet unused variable warning
                auto& us_name{UserString(category_name)};
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(us_name),
                                    std::forward_as_tuple(LinkTaggedPresetText(TextLinker::ENCYCLOPEDIA_TAG, category_name, us_name).append("\n"), category_name));
            }

        }
        else if (dir_name == "ENC_SHIP_PART") {
            for (auto& [part_name, part] : GetShipPartManager()) {
                if (!exclude_custom_categories_from_dir_name || !HasCustomCategory(part->Tags())) {
                    auto& us_name{UserString(part_name)};
                    retval.emplace_back(std::piecewise_construct,
                                        std::forward_as_tuple(us_name),
                                        std::forward_as_tuple(LinkTaggedPresetText(VarText::SHIP_PART_TAG, part_name, us_name).append("\n"), part_name));
                }
            }

        }
        else if (dir_name == "ENC_SHIP_HULL") {
            for (auto& [hull_name, hull] : GetShipHullManager()) {
                if (!exclude_custom_categories_from_dir_name || !HasCustomCategory(hull->Tags())) {
                    auto& us_name{UserString(hull_name)};
                    retval.emplace_back(std::piecewise_construct,
                                        std::forward_as_tuple(us_name),
                                        std::forward_as_tuple(LinkTaggedPresetText(VarText::SHIP_HULL_TAG, hull_name, us_name).append("\n"), hull_name));
                }
            }

        }
        else if (dir_name == "ENC_TECH") {
            // sort tech names by user-visible name, so names are shown alphabetically in UI
            auto tech_names{GetTechManager().TechNames()};
            std::vector<std::pair<std::string_view, std::string>> userstring_tech_names;
            userstring_tech_names.reserve(tech_names.size());
            for (auto& tech_name : tech_names) {
                auto& us_name{UserString(tech_name)};   // use tech_name on line before moving from tech_name to avoid order of evaluation issues
                userstring_tech_names.emplace_back(us_name, std::move(tech_name));
            }
            std::sort(userstring_tech_names.begin(), userstring_tech_names.end());

            // second loop over alphabetically sorted names...
            for (auto& [us_name, tech_name] : userstring_tech_names) {
                if (!exclude_custom_categories_from_dir_name || !HasCustomCategory(GetTech(tech_name)->Tags())) {
                    // already iterating over userstring-looked-up names, so don't need to re-look-up-here
                    std::string tagged_text{LinkTaggedPresetText(VarText::TECH_TAG, tech_name, us_name).append("\n")};
                    retval.emplace_back(std::piecewise_construct,
                                        std::forward_as_tuple(us_name),
                                        std::forward_as_tuple(std::move(tagged_text), std::move(tech_name)));
                }
            }

        }
        else if (dir_name == "ENC_POLICY") {
            for (auto& policy_name : GetPolicyManager().PolicyNames()) {
                auto& us_name{UserString(policy_name)}; // line before to avoid order of evaluation issues when moving from policy_name
                std::string tagged_text{LinkTaggedPresetText(VarText::POLICY_TAG, policy_name, us_name).append("\n")};
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(us_name),
                                    std::forward_as_tuple(std::move(tagged_text), policy_name));
            }

        }
        else if (dir_name == "ENC_BUILDING_TYPE") {
            for (const auto& [building_name, building_type] : GetBuildingTypeManager()) {
                if (!exclude_custom_categories_from_dir_name || !HasCustomCategory(building_type->Tags())) {
                    auto& us_name{UserString(building_name)};
                    retval.emplace_back(std::piecewise_construct,
                                        std::forward_as_tuple(us_name),
                                        std::forward_as_tuple(LinkTaggedPresetText(VarText::BUILDING_TYPE_TAG, building_name, us_name).append("\n"), building_name));
                }
            }

        }
        else if (dir_name == "ENC_SPECIAL") {
            for (auto special_name : SpecialNames()) {
                auto& us_name{UserString(special_name)};    // line before to avoid order of operations issues when moving from special_name
                std::string tagged_text{LinkTaggedPresetText(VarText::SPECIAL_TAG, special_name, us_name).append("\n")};
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(us_name),
                                    std::forward_as_tuple(std::move(tagged_text), special_name));
            }

        }
        else if (dir_name == "ENC_SPECIES") {
            // directory populated with list of links to other articles that list species
            if (!exclude_custom_categories_from_dir_name) {
                for (const auto& [species_name, species] : GetSpeciesManager()) {
                    (void)species; // quiet warning
                    auto& us_name{UserString(species_name)};
                    retval.emplace_back(std::piecewise_construct,
                                        std::forward_as_tuple(us_name),
                                        std::forward_as_tuple(LinkTaggedPresetText(VarText::SPECIES_TAG, species_name, us_name).append("\n"), species_name));
                }
            }

        }
        else if (dir_name == "ENC_HOMEWORLDS") {
            const auto& homeworlds{GetSpeciesManager().GetSpeciesHomeworldsMap()};

            for (const auto& entry : GetSpeciesManager()) {
                std::set<int> known_homeworlds;
                std::string species_entry = LinkTaggedText(VarText::SPECIES_TAG, entry.first).append(" ");

                // TODO: stealthy worlds should be hidden on the server side and not show up as clear text here, only when the empire has sufficient detection strength
                // homeworld
                if (!homeworlds.count(entry.first) || homeworlds.at(entry.first).empty()) {
                    continue;
                } else {
                    const auto& this_species_homeworlds = homeworlds.at(entry.first);
                    std::string homeworld_info;
                    species_entry.append("(").append(std::to_string(this_species_homeworlds.size())).append("):  ");
                    bool first = true;
                    for (int homeworld_id : this_species_homeworlds) {
                        if (first) first = false;
                        else homeworld_info.append(",  ");
                        if (auto homeworld = objects.get<Planet>(homeworld_id)) {
                            known_homeworlds.insert(homeworld_id);
                            // if known, add to beginning
                            homeworld_info.append(
                                LinkTaggedIDText(VarText::PLANET_ID_TAG, homeworld_id,
                                                 homeworld->PublicName(client_empire_id, universe))
                            );
                        } else { 
                            // add to end
                            homeworld_info.append(UserString("UNKNOWN_PLANET"));
                        }
                    }
                    species_entry += homeworld_info;
                }

                // occupied planets
                std::vector<const Planet*> species_occupied_planets;
                for (const auto& planet : objects.all<Planet>()) {
                    if ((planet->SpeciesName() == entry.first) && !known_homeworlds.count(planet->ID()))
                        species_occupied_planets.emplace_back(planet.get());
                }
                if (!species_occupied_planets.empty()) {
                    if (species_occupied_planets.size() >= 5) {
                        species_entry.append("  |   ").append(std::to_string(species_occupied_planets.size()))
                                     .append(" ").append(UserString("OCCUPIED_PLANETS"));
                    } else {
                        species_entry.append("  |   ").append(UserString("OCCUPIED_PLANETS")).append(":  ");
                        for (auto& planet : species_occupied_planets) {
                            species_entry.append(LinkTaggedIDText(VarText::PLANET_ID_TAG, planet->ID(),
                                                                  planet->PublicName(client_empire_id, universe)))
                                .append("   ");
                        }
                    }
                }
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(UserString(entry.first)),
                                    std::forward_as_tuple(species_entry.append("\n"), entry.first));
            }
            retval.emplace_back(std::piecewise_construct,
                                std::forward_as_tuple("⃠ "),
                                std::forward_as_tuple("\n\n", "  "));
            for (const auto& entry : GetSpeciesManager()) {
                if (!homeworlds.count(entry.first) || homeworlds.at(entry.first).empty()) {
                    std::string species_entry{
                        LinkTaggedPresetText(VarText::SPECIES_TAG, entry.first, UserString(entry.first))
                        .append(":  ")
                        .append(UserString("NO_HOMEWORLD"))
                        .append("\n")
                    };
                    retval.emplace_back(std::piecewise_construct,
                                        std::forward_as_tuple("⃠⃠⃠ " + UserString(entry.first)),
                                        std::forward_as_tuple(std::move(species_entry), entry.first));
                }
            }

        }
        else if (dir_name == "ENC_FIELD_TYPE") {
            for (const auto& [field_name, field] : GetFieldTypeManager()) {
                if (!exclude_custom_categories_from_dir_name || !HasCustomCategory(field->Tags())) {
                    auto& us_name{UserString(field_name)};
                    retval.emplace_back(std::piecewise_construct,
                                        std::forward_as_tuple(us_name),
                                        std::forward_as_tuple(LinkTaggedPresetText(VarText::FIELD_TYPE_TAG, field_name, us_name).append("\n"),
                                                              field_name));
                }
            }

        }
        else if (dir_name == "ENC_METER_TYPE") {
            for (MeterType meter_type = MeterType::METER_POPULATION;
                 meter_type != MeterType::NUM_METER_TYPES;
                 meter_type = static_cast<MeterType>(static_cast<int>(meter_type) + 1))
            {
                if (meter_type > MeterType::INVALID_METER_TYPE && meter_type < MeterType::NUM_METER_TYPES)
                    MeterTypeDirEntry(meter_type, retval);
            }

        }
        else if (dir_name == "ENC_EMPIRE") {
            for (const auto& [id, empire] : Empires()) {
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(empire->Name()),
                                    std::forward_as_tuple(LinkTaggedIDText(VarText::EMPIRE_ID_TAG, id, empire->Name()).append("\n"),
                                                          std::to_string(id)));
            }

        }
        else if (dir_name == "ENC_SHIP_DESIGN") {
            for (auto it = universe.beginShipDesigns(); it != universe.endShipDesigns(); ++it) {
                const auto& [design_id, design] = *it;
                if (design->IsMonster())
                    continue;
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(design->Name()),
                                    std::forward_as_tuple(LinkTaggedIDText(VarText::DESIGN_ID_TAG, design_id, design->Name()).append("\n"),
                                                          std::to_string(design_id)));
            }

        }
        else if (dir_name == "ENC_SHIP") {
            for (auto& ship : objects.all<Ship>()) {
                auto& ship_name = ship->PublicName(client_empire_id, universe);
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(ship_name),
                                    std::forward_as_tuple(LinkTaggedIDText(VarText::SHIP_ID_TAG, ship->ID(), ship_name).append("  "),
                                                          std::to_string(ship->ID())));
            }

        }
        else if (dir_name == "ENC_MONSTER") {
            for (auto& ship : objects.all<Ship>()) {
                if (!ship->IsMonster(universe))
                    continue;
                auto& ship_name = ship->PublicName(client_empire_id, universe);
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(ship_name),
                                    std::forward_as_tuple(LinkTaggedIDText(VarText::SHIP_ID_TAG, ship->ID(), ship_name).append("  "),
                                                          std::to_string(ship->ID())));
            }

        }
        else if (dir_name == "ENC_MONSTER_TYPE") {
            for (auto it = universe.beginShipDesigns(); it != universe.endShipDesigns(); ++it) {
                auto&& [design_id, design] = *it;
                if (design && design->IsMonster())
                    retval.emplace_back(std::piecewise_construct,
                                        std::forward_as_tuple(design->Name()),
                                        std::forward_as_tuple(LinkTaggedIDText(VarText::DESIGN_ID_TAG, design_id, design->Name()).append("\n"),
                                                              std::to_string(design_id)));
            }

        }
        else if (dir_name == "ENC_FLEET") {
            for (auto& fleet : objects.all<Fleet>()) {
                auto& flt_name = fleet->PublicName(client_empire_id, universe);
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(flt_name),
                                    std::forward_as_tuple(LinkTaggedIDText(VarText::FLEET_ID_TAG, fleet->ID(), flt_name).append("  "),
                                                          std::to_string(fleet->ID())));
            }

        }
        else if (dir_name == "ENC_PLANET") {
            for (auto& planet : objects.all<Planet>()) {
                auto& plt_name = planet->PublicName(client_empire_id, universe);
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(plt_name),
                                    std::forward_as_tuple(LinkTaggedIDText(VarText::PLANET_ID_TAG, planet->ID(), plt_name).append("  "),
                                                          std::to_string(planet->ID())));
            }

        }
        else if (dir_name == "ENC_BUILDING") {
            for (auto& building : objects.all<Building>()) {
                auto& bld_name = building->PublicName(client_empire_id, universe);
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(bld_name),
                                    std::forward_as_tuple(LinkTaggedIDText(VarText::BUILDING_ID_TAG, building->ID(), bld_name).append("  "),
                                                          std::to_string(building->ID())));
            }

        }
        else if (dir_name == "ENC_SYSTEM") {
            for (auto& system : objects.all<System>()) {
                auto sys_name = system->ApparentName(client_empire_id, universe);
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(sys_name),
                                    std::forward_as_tuple(LinkTaggedIDText(VarText::SYSTEM_ID_TAG, system->ID(), sys_name).append("  "),
                                                          std::to_string(system->ID())));
            }

        }
        else if (dir_name == "ENC_FIELD") {
            for (auto& field : objects.all<Field>()) {
                const std::string& field_name = field->Name();
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(field_name),
                                    std::forward_as_tuple(LinkTaggedIDText(VarText::FIELD_ID_TAG, field->ID(), field_name).append("  "),
                                                          std::to_string(field->ID())));
            }

        }
        else if (dir_name == "ENC_GRAPH") {
            for (const auto& stat_record : universe.GetStatRecords()) {
                auto& us_name{UserString(stat_record.first)};
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(us_name),
                                    std::forward_as_tuple(LinkTaggedPresetText(TextLinker::GRAPH_TAG, stat_record.first, us_name).append("\n"),
                                                          stat_record.first));
            }

        }
        else if (dir_name == "ENC_TEXTURES") {
             for (auto& [tex_name, tex] : GG::GetTextureManager().Textures()) {
                 auto texture_info_str = boost::io::str(
                     FlexibleFormat(UserString("ENC_TEXTURE_INFO")) %
                     Value(tex->Width()) %
                     Value(tex->Height()) %
                     tex->BytesPP() %
                     tex_name);
                 retval.emplace_back(std::piecewise_construct,
                                     std::forward_as_tuple(tex_name),
                                     std::forward_as_tuple(std::move(texture_info_str), tex_name));
             }

             for (auto& [tex_name, tex] : GG::GetVectorTextureManager().Textures()) {
                 auto texture_info_str = boost::io::str(
                     FlexibleFormat(UserString("ENC_VECTOR_TEXTURE_INFO")) %
                     Value(tex->Size().x) %
                     Value(tex->Size().y) %
                     tex->NumShapes() %
                     tex_name);
                 retval.emplace_back(std::piecewise_construct,
                                     std::forward_as_tuple(tex_name),
                                     std::forward_as_tuple(std::move(texture_info_str), tex_name));
             }

        }
        else if (dir_name == "ENC_STRINGS") {
            // show all stringable keys and values
            for (auto& [str_key, str_val] : AllStringtableEntries())
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(str_key),
                                    std::forward_as_tuple(str_key + ": " + str_val + "\n", str_key));

        }
        else if  (dir_name == "ENC_NAMED_VALUE_REF") {
            retval.emplace_back(std::piecewise_construct,
                                std::forward_as_tuple("ENC_NAMED_VALUE_REF_DESC"),
                                std::forward_as_tuple(UserString("ENC_NAMED_VALUE_REF_DESC") + "\n\n", dir_name));

            for (auto& [ref_key, val_ref] : GetNamedValueRefManager().GetItems()) {
                auto& vref = val_ref.get();
                std::string_view value_type = dynamic_cast<ValueRef::ValueRef<int>*>(&vref)? " int " : (dynamic_cast<ValueRef::ValueRef<double>*>(&vref)?" real ":" any ");
                std::string_view key_str = UserStringExists(ref_key) ? UserString(ref_key) : "";

                // (human-readable article name) -> (link tag text, category stringtable key)
                retval.emplace_back(
                    std::piecewise_construct,
                    std::forward_as_tuple(ref_key),
                    std::forward_as_tuple(std::string{ref_key}.append(value_type)
                                          .append(LinkTaggedPresetText(VarText::FOCS_VALUE_TAG, ref_key, key_str))
                                          .append(key_str.empty() ? "" : (std::string{" '"}.append(key_str).append(" '")))
                                          .append(" ").append(vref.InvariancePattern().append("\n")),
                                          dir_name));
            }

        }
        else {
            // Any content definitions (FOCS files) that define a pedia category
            // should have their pedia article added to this category.
            std::vector<std::pair<std::string_view, std::pair<std::string_view, std::string_view>>> dir_entries;
            dir_entries.reserve(GetShipPartManager().size() + GetShipHullManager().size() +
                                GetTechManager().size() + GetBuildingTypeManager().NumBuildingTypes() +
                                GetSpeciesManager().NumSpecies() + GetFieldTypeManager().size());

            // part types
            for (auto& [part_name, part_type] : GetShipPartManager())
                if (HasCustomCategory(part_type->Tags(), dir_name))
                    dir_entries.emplace_back(std::piecewise_construct,
                                             std::forward_as_tuple(UserString(part_name)),
                                             std::forward_as_tuple(VarText::SHIP_PART_TAG, part_name));

            // hull types
            for (auto& [hull_name, hull_type] : GetShipHullManager())
                if (HasCustomCategory(hull_type->Tags(), dir_name))
                    dir_entries.emplace_back(std::piecewise_construct,
                                             std::forward_as_tuple(UserString(hull_name)),
                                             std::forward_as_tuple(VarText::SHIP_HULL_TAG, hull_name));

            // techs
            for (const auto& tech : GetTechManager()) {
                const auto& tech_name = tech->Name();
                if (HasCustomCategory(tech->Tags(), dir_name))
                    dir_entries.emplace_back(std::piecewise_construct,
                                             std::forward_as_tuple(UserString(tech_name)),
                                             std::forward_as_tuple(VarText::TECH_TAG, tech_name));
            }

            // building types
            for (auto& [building_name, building_type] : GetBuildingTypeManager())
                if (HasCustomCategory(building_type->Tags(), dir_name))
                    dir_entries.emplace_back(std::piecewise_construct,
                                             std::forward_as_tuple(UserString(building_name)),
                                             std::forward_as_tuple(VarText::BUILDING_TYPE_TAG, building_name));

            // species
            for (auto& [species_name, species] : GetSpeciesManager())
                if (dir_name == "ALL_SPECIES" ||
                   (dir_name == "NATIVE_SPECIES" && species->Native()) ||
                   (dir_name == "PLAYABLE_SPECIES" && species->Playable()) ||
                    HasCustomCategory(species->Tags(), dir_name))
                {
                    dir_entries.emplace_back(std::piecewise_construct,
                                             std::forward_as_tuple(UserString(species_name)),
                                             std::forward_as_tuple(VarText::SPECIES_TAG, species_name));
                }

            // field types
            for (auto& [field_name, field_type] : GetFieldTypeManager())
                if (HasCustomCategory(field_type->Tags(), dir_name))
                    dir_entries.emplace_back(std::piecewise_construct,
                                             std::forward_as_tuple(UserString(field_name)),
                                             std::forward_as_tuple(VarText::FIELD_TYPE_TAG, field_name));

            // Sort entries, keyed by human-readable article name,
            // containing (tag, article stringtable key)
            std::sort(dir_entries.begin(), dir_entries.end());
            for (auto& [readable_name, tag_key] : dir_entries) {
                auto linked_text{LinkTaggedText(tag_key.first, tag_key.second).append("\n")};
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(readable_name),
                                    std::forward_as_tuple(std::move(linked_text), tag_key.second));
            }
        }

        // Add any defined entries for this directory
        const auto& articles = encyclopedia.Articles();
        auto category_it = articles.find(std::string{dir_name}); // TODO: make Articles support heterogenous comparison
        if (category_it != articles.end()) {
            for (const EncyclopediaArticle& article : category_it->second) {
                // Prevent duplicate addition of hard-coded directories that also have a content definition
                if (article.name == dir_name)
                    continue;
                auto& us_name{UserString(article.name)};
                retval.emplace_back(std::piecewise_construct,
                                    std::forward_as_tuple(us_name),
                                    std::forward_as_tuple(LinkTaggedPresetText(TextLinker::ENCYCLOPEDIA_TAG, article.name, us_name) + "\n", article.name));
            }
        }


        std::sort(retval.begin(), retval.end());
        return retval;
    }

    std::string PediaDirText(const std::string& dir_name) {
        // get sorted list of entries for requested directory
        auto sorted_entries = GetSortedPediaDirEntires(dir_name, true);

        std::string retval;
        retval.reserve(sorted_entries.size() * 128);   // rough guesstimate

        // add sorted entries linktext representation to page text
        for (const auto& entry : sorted_entries)
            retval += entry.second.first;

        return retval;
    }
}

namespace {
    class SearchEdit : public CUIEdit {
    public:
        SearchEdit() :
            CUIEdit("")
        { DisallowChars("\n\r"); }

        void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override {
            switch (key) {
            case GG::Key::GGK_RETURN:
            case GG::Key::GGK_KP_ENTER:
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
                                                 std::string_view config_name) :
    CUIWnd(UserString("MAP_BTN_PEDIA"), flags, config_name, false)
{}

void EncyclopediaDetailPanel::CompleteConstruction() {
    CUIWnd::CompleteConstruction();

    const int PTS = ClientUI::Pts();
    const int NAME_PTS = PTS*3/2;
    const int SUMMARY_PTS = PTS*4/3;
    static constexpr GG::X CONTROL_WIDTH{54};
    static constexpr GG::Y CONTROL_HEIGHT{74};
    const GG::Pt PALETTE_MIN_SIZE{GG::X{CONTROL_WIDTH + 70}, GG::Y{CONTROL_HEIGHT + 70}};

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

    m_index_button->LeftClickedSignal.connect(boost::bind(&EncyclopediaDetailPanel::OnIndex, this));
    m_back_button->LeftClickedSignal.connect(boost::bind(&EncyclopediaDetailPanel::OnBack, this));
    m_next_button->LeftClickedSignal.connect(boost::bind(&EncyclopediaDetailPanel::OnNext, this));

    m_description_rich_text = GG::Wnd::Create<GG::RichText>(
        GG::X(0), GG::Y(0), ClientWidth(), ClientHeight(), "",
        ClientUI::GetFont(), ClientUI::TextColor(),
        GG::FORMAT_TOP | GG::FORMAT_LEFT | GG::FORMAT_LINEWRAP | GG::FORMAT_WORDBREAK,
        GG::INTERACTIVE);
    m_scroll_panel = GG::Wnd::Create<GG::ScrollPanel>(GG::X(0), GG::Y(0), ClientWidth(),
                                                      ClientHeight(), m_description_rich_text);

    namespace ph = boost::placeholders;

    // Create a block factory that handles link clicks via this panel
    auto factory = std::make_shared<CUILinkTextBlock::Factory>();
    factory->LinkClickedSignal.connect(
        boost::bind(&EncyclopediaDetailPanel::HandleLinkClick, this, ph::_1, ph::_2));
    factory->LinkDoubleClickedSignal.connect(
        boost::bind(&EncyclopediaDetailPanel::HandleLinkDoubleClick, this, ph::_1, ph::_2));
    factory->LinkRightClickedSignal.connect(
        boost::bind(&EncyclopediaDetailPanel::HandleLinkDoubleClick, this, ph::_1, ph::_2));

    // make local copy of default block factories map
    const auto& default_block_factory_map{*GG::RichText::DefaultBlockFactoryMap()};
    auto factory_map = std::make_shared<GG::RichText::BLOCK_FACTORY_MAP>(default_block_factory_map);

    // set the plaintext block factory to the one handling link clicks via this panel
    (*factory_map)[std::string{GG::RichText::PLAINTEXT_TAG}] = std::move(factory);

    // use block factory map modified copy for this control
    m_description_rich_text->SetBlockFactoryMap(std::move(factory_map));


    m_description_rich_text->SetPadding(DESCRIPTION_PADDING);

    m_scroll_panel->SetBackgroundColor(ClientUI::CtrlColor());
    m_scroll_panel->InstallEventFilter(shared_from_this());

    m_graph = GG::Wnd::Create<GraphControl>();
    m_graph->ShowPoints(false);

    auto search_edit = GG::Wnd::Create<SearchEdit>();
    search_edit->TextEnteredSignal.connect(boost::bind(&EncyclopediaDetailPanel::HandleSearchTextEntered, this));
    m_search_edit = std::move(search_edit);

    AttachChild(m_search_edit);
    AttachChild(m_graph);
    AttachChild(m_name_text);
    AttachChild(m_cost_text);
    AttachChild(m_summary_text);
    AttachChild(m_scroll_panel);
    AttachChild(m_index_button);
    AttachChild(m_back_button);
    AttachChild(m_next_button);

    SetChildClippingMode(ChildClippingMode::ClipToWindow);
    DoLayout();

    SetMinSize(PALETTE_MIN_SIZE);

    MoveChildUp(m_graph);
    SaveDefaultedOptions();

    AddItem(TextLinker::ENCYCLOPEDIA_TAG, "ENC_INDEX");
}

namespace {
    constexpr int BTN_WIDTH = 36;
    constexpr int PAD = 2;

    int IconSize() {
        const int NAME_PTS = ClientUI::TitlePts();
        const int COST_PTS = ClientUI::Pts();
        const int SUMMARY_PTS = ClientUI::Pts()*4/3;
        return 12 + NAME_PTS + COST_PTS + SUMMARY_PTS;
    }
}

void EncyclopediaDetailPanel::DoLayout() {
    const int NAME_PTS = ClientUI::TitlePts();
    const int COST_PTS = ClientUI::Pts();
    const int SUMMARY_PTS = ClientUI::Pts()*4/3;

    const int ICON_SIZE = IconSize();

    SectionedScopedTimer timer("EncyclopediaDetailPanel::DoLayout");

    // name
    GG::Pt ul = GG::Pt(BORDER_LEFT, CUIWnd::TopBorder() + PAD);
    GG::Pt lr = ul + GG::Pt(Width(), GG::Y(NAME_PTS + 2*PAD));
    m_name_text->SetTextFormat(GG::FORMAT_LEFT);
    m_name_text->SizeMove(ul, lr);

    // cost / turns
    ul += GG::Pt(GG::X0, m_name_text->Height() + PAD);
    lr = ul + GG::Pt(Width(), GG::Y(COST_PTS + 2*PAD));
    m_cost_text->SetTextFormat(GG::FORMAT_LEFT);
    m_cost_text->SizeMove(ul, lr);

    // one line summary
    ul += GG::Pt(GG::X0, m_cost_text->Height());
    lr = ul + GG::Pt(Width(), GG::Y(SUMMARY_PTS + 2*PAD));
    m_summary_text->SetTextFormat(GG::FORMAT_LEFT);
    m_summary_text->SizeMove(ul, lr);


    // "back" button
    ul = GG::Pt{BORDER_LEFT, ICON_SIZE + CUIWnd::TopBorder() + PAD*2};
    lr = ul + GG::Pt{GG::X{BTN_WIDTH}, GG::Y{BTN_WIDTH}};
    m_back_button->SizeMove(ul, lr);
    ul += GG::Pt{GG::X{BTN_WIDTH}, GG::Y0};

    // "up" button
    lr = ul + GG::Pt{GG::X{BTN_WIDTH}, GG::Y{BTN_WIDTH}};
    m_index_button->SizeMove(ul, lr);
    ul += GG::Pt{GG::X{BTN_WIDTH}, GG::Y0};

    // "next" button
    lr = ul + GG::Pt{GG::X{BTN_WIDTH}, GG::Y{BTN_WIDTH}};
    m_next_button->SizeMove(ul, lr);
    ul += GG::Pt{GG::X{BTN_WIDTH}, GG::Y0};

    // search edit box
    lr = GG::Pt{ClientWidth(), ul.y + GG::Y{BTN_WIDTH}};
    m_search_edit->SizeMove(ul, lr);

    // main verbose description (fluff, effects, unlocks, ...)
    ul = GG::Pt{BORDER_LEFT, lr.y + PAD*3};
    lr = ClientSize() - GG::Pt{GG::X0, GG::Y{CUIWnd::INNER_BORDER_ANGLE_OFFSET}};
    timer.EnterSection("m_scroll_panel->SizeMove");
    m_scroll_panel->SizeMove(ul, lr);
    timer.EnterSection("");

    // graph
    m_graph->SizeMove(ul + GG::Pt(GG::X1, GG::Y1), lr - GG::Pt(GG::X1, GG::Y1));

    // icon
    if (m_icon) {
        lr = GG::Pt(Width() - BORDER_RIGHT, GG::Y(ICON_SIZE + CUIWnd::TopBorder()));
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
    if (key == GG::Key::GGK_RETURN || key == GG::Key::GGK_KP_ENTER) {
        GG::GUI::GetGUI()->SetFocusWnd(m_search_edit);
    } else if (key == GG::Key::GGK_BACKSPACE) {
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

    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    const GG::Y ICON_SIZE = m_summary_text->Bottom() - m_name_text->Top();
    GG::Pt cl_ul = ul + GG::Pt(BORDER_LEFT, ICON_SIZE + CUIWnd::TopBorder() + BTN_WIDTH + PAD*3);
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
    GG::Pt underline_ul{UpperLeft() + GG::Pt{BORDER_LEFT, CUIWnd::TopBorder() - 2}};
    GG::Pt underline_lr{underline_ul + GG::Pt{Width() - BORDER_RIGHT*3, GG::Y0}};
    m_vertex_buffer.store(Value(underline_ul.x),    Value(underline_ul.y));
    m_vertex_buffer.store(Value(underline_lr.x),    Value(underline_lr.y));
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
            if (const ShipDesign* design = GetUniverse().GetGenericShipDesign(data))
                this->SetDesign(design->ID());

        } else if (link_type == VarText::TECH_TAG) {
            this->SetTech(data);
        } else if (link_type == VarText::POLICY_TAG) {
            this->SetPolicy(data);
        } else if (link_type == VarText::BUILDING_TYPE_TAG) {
            this->SetBuildingType(data);
        } else if (link_type == VarText::FIELD_TYPE_TAG) {
            this->SetFieldType(data);
        } else if (link_type == VarText::METER_TYPE_TAG) {
            this->SetMeterType(data);
        } else if (link_type == VarText::SPECIAL_TAG) {
            this->SetSpecial(data);
        } else if (link_type == VarText::SHIP_HULL_TAG) {
            this->SetShipHull(data);
        } else if (link_type == VarText::SHIP_PART_TAG) {
            this->SetShipPart(data);
        } else if (link_type == VarText::SPECIES_TAG) {
            this->SetSpecies(data);
        } else if (link_type == TextLinker::ENCYCLOPEDIA_TAG) {
            this->SetText(data, false);
        } else if (link_type == TextLinker::GRAPH_TAG) {
            this->SetGraph(data);
        } else if (link_type == TextLinker::URL_TAG) {
            GGHumanClientApp::GetApp()->OpenURL(data);
        } else if (link_type == TextLinker::BROWSE_PATH_TAG) {
            GGHumanClientApp::GetApp()->BrowsePath(FilenameToPath(data));
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
            if (const ShipDesign* design = GetUniverse().GetGenericShipDesign(data))
                ClientUI::GetClientUI()->ZoomToShipDesign(design->ID());

        } else if (link_type == VarText::TECH_TAG) {
            ClientUI::GetClientUI()->ZoomToTech(data);
        } else if (link_type == VarText::POLICY_TAG) {
            ClientUI::GetClientUI()->ZoomToPolicy(data);
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
    /** Recursively searches pedia directory \a dir_name for articles and
      * sub-directories. Returns a map from
      * (category_str_key, dir_name) to (readable_article_name, link_text) */
    [[nodiscard]] auto GetSubDirs(std::string_view dir_name,
                                  bool exclude_custom_categories_from_dir_name = true,
                                  int depth = 0)
    {
        std::map<std::pair<std::string, std::string>, std::pair<std::string, std::string>> retval;

        if (dir_name == "ENC_STRINGS")
            return retval;

        ScopedTimer subdir_timer(std::string{"GetSubDirs("}.append(dir_name).append(", ")
                                 .append(std::to_string(exclude_custom_categories_from_dir_name))
                                 .append(", ").append(std::to_string(depth)).append(")"),
                                 std::chrono::milliseconds(5));

        depth++;
        // safety check to pre-empt potential infinite loop
        if (depth > 50) {
            WarnLogger() << "Exceeded recursive limit with lookup for pedia category " << dir_name;
            return retval;
        }


        // map from (human readable article name) to (article-link-tag-text, article name stringtable key)
        auto sorted_entries = GetSortedPediaDirEntires(dir_name, exclude_custom_categories_from_dir_name);


        std::vector<std::future<decltype(GetSubDirs(""))>> futures;
        futures.reserve(sorted_entries.size());

        for (auto& [readable_article_name, link_category] : sorted_entries) {
            auto& [link_text, category_str_key] = link_category;

            // explicitly exclude textures and input directory itself
            if (category_str_key == "ENC_TEXTURES" || category_str_key == dir_name)
                continue;

            futures.push_back(std::async(std::launch::async,
                                         GetSubDirs,
                                         category_str_key, exclude_custom_categories_from_dir_name, depth));

            retval.emplace(std::pair{std::move(category_str_key), dir_name},
                           std::pair{readable_article_name, std::move(link_text)});
        }

        for (auto& fut : futures)
            retval.merge(fut.get());

        return retval;
    }

    [[nodiscard]] int DefaultLocationForEmpire(int empire_id) {
        if (empire_id == ALL_EMPIRES)
            return INVALID_OBJECT_ID;

        const Empire* empire = GetEmpire(empire_id);
        if (!empire) {
            DebugLogger() << "DefaultLocationForEmpire: Unable to get empire with ID: " << empire_id;
            return INVALID_OBJECT_ID;
        }
        // get a location where the empire might build something.
        auto location = Objects().get(empire->CapitalID());
        // no capital?  scan through all objects to find one owned by this empire
        // TODO: only loop over planets?
        // TODO: pass in a location condition, and pick a location that matches it if possible
        if (!location) {
            for (const auto& obj : Objects().all()) {
                if (obj->OwnedBy(empire_id)) {
                    location = obj;
                    break;
                }
            }
        }
        return location ? location->ID() : INVALID_OBJECT_ID;
    }

    [[nodiscard]] std::vector<std::string> TechsThatUnlockItem(const UnlockableItem& item) {
        std::vector<std::string> retval;
        retval.reserve(GetTechManager().size()); // rough guesstimate

        for (const auto& tech : GetTechManager()) {
            if (!tech) continue;

            bool found_item = false;
            for (const UnlockableItem& unlocked_item : tech->UnlockedItems()) {
                if (unlocked_item == item) {
                    found_item = true;
                    break;
                }
            }
            if (found_item)
                retval.push_back(tech->Name());
        }

        return retval;
    }

    [[nodiscard]] std::vector<std::string> PoliciesThatUnlockItem(const UnlockableItem& item) {
        std::vector<std::string> retval;

        for (auto& [policy_name, policy] : GetPolicyManager()) {
            if (!policy) continue;

            bool found_item = false;
            for (const UnlockableItem& unlocked_item : policy->UnlockedItems()) {
                if (unlocked_item == item) {
                    found_item = true;
                    break;
                }
            }
            if (found_item)
                retval.push_back(policy_name);
        }

        return retval;
    }

    [[nodiscard]] const std::string& GeneralTypeOfObject(UniverseObjectType obj_type) {
        switch (obj_type) {
        case UniverseObjectType::OBJ_SHIP:          return UserString("ENC_SHIP");          break;
        case UniverseObjectType::OBJ_FLEET:         return UserString("ENC_FLEET");         break;
        case UniverseObjectType::OBJ_PLANET:        return UserString("ENC_PLANET");        break;
        case UniverseObjectType::OBJ_BUILDING:      return UserString("ENC_BUILDING");      break;
        case UniverseObjectType::OBJ_SYSTEM:        return UserString("ENC_SYSTEM");        break;
        case UniverseObjectType::OBJ_FIELD:         return UserString("ENC_FIELD");         break;
        case UniverseObjectType::OBJ_POP_CENTER:    return UserString("ENC_POP_CENTER");    break;
        case UniverseObjectType::OBJ_PROD_CENTER:   return UserString("ENC_PROD_CENTER");   break;
        case UniverseObjectType::OBJ_FIGHTER:       return UserString("ENC_FIGHTER");       break;
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
                % gsd.seed
                % std::to_string(gsd.size)
                % TextForGalaxyShape(gsd.shape)
                % TextForGalaxySetupSetting(gsd.age)
                % TextForGalaxySetupSetting(gsd.starlane_freq)
                % TextForGalaxySetupSetting(gsd.planet_density)
                % TextForGalaxySetupSetting(gsd.specials_freq)
                % TextForGalaxySetupSetting(gsd.monster_freq)
                % TextForGalaxySetupSetting(gsd.native_freq)
                % TextForAIAggression(gsd.ai_aggr)
                % gsd.game_uid);

            return;
        }
        else if (item_name == "ENC_GAME_RULES") {
            for (auto& [rule_name, rule] : GetGameRules()) {
                if (rule.ValueIsDefault())
                    detailed_description += UserString(rule_name) + " : " + rule.ValueToString() + "\n";
                else
                    detailed_description += "<u>" + UserString(rule_name) + " : " + rule.ValueToString() + "</u>\n";
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
        auto dir_text = PediaDirText(item_name);
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
                                            GG::Clr& color, bool only_description = false)
    {
        const ShipPart* part = GetShipPart(item_name);
        if (!part) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find part with name " << item_name;
            return;
        }
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();

        // Ship Parts
        if (!only_description) {
            const ScriptingContext context;
            name = UserString(item_name);
            texture = ClientUI::PartIcon(item_name);
            int default_location_id = DefaultLocationForEmpire(client_empire_id);
            turns = part->ProductionTime(client_empire_id, default_location_id, context);
            cost = part->ProductionCost(client_empire_id, default_location_id, context);
            cost_units = UserString("ENC_PP");
            general_type = UserString("ENC_SHIP_PART");
            specific_type = UserString(to_string(part->Class()));
        }

        detailed_description += UserString(part->Description()) + "\n\n" + part->CapacityDescription();

        std::string slot_types_list;
        if (part->CanMountInSlotType(ShipSlotType::SL_EXTERNAL))
            slot_types_list += UserString("SL_EXTERNAL") + "   ";
        if (part->CanMountInSlotType(ShipSlotType::SL_INTERNAL))
            slot_types_list += UserString("SL_INTERNAL") + "   ";
        if (part->CanMountInSlotType(ShipSlotType::SL_CORE))
            slot_types_list += UserString("SL_CORE");
        if (!slot_types_list.empty())
            detailed_description += "\n\n" + UserString("ENC_SHIP_PART_CAN_MOUNT_IN_SLOT_TYPES") + slot_types_list;

        const auto& exclusions = part->Exclusions();
        if (!exclusions.empty()) {
            detailed_description += "\n\n" + UserString("ENC_SHIP_EXCLUSIONS");
            detailed_description.append(LinkList(exclusions));
        }

        auto unlocked_by_techs = TechsThatUnlockItem(UnlockableItem(UnlockableItemType::UIT_SHIP_PART, item_name));
        if (!unlocked_by_techs.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            detailed_description.append(LinkList(unlocked_by_techs));
        }

        auto unlocked_by_policies = PoliciesThatUnlockItem(UnlockableItem(UnlockableItemType::UIT_SHIP_PART, item_name));
        if (!unlocked_by_policies.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            detailed_description.append(LinkList(unlocked_by_policies));
        }

        // species that like / dislike part
        auto species_that_like = GetSpeciesManager().SpeciesThatLike(item_name);
        auto species_that_dislike = GetSpeciesManager().SpeciesThatDislike(item_name);
        if (!species_that_like.empty()) {
            detailed_description += "\n\n" + UserString("SPECIES_THAT_LIKE");
            detailed_description.append(LinkList(species_that_like));
        }
        if (!species_that_dislike.empty()) {
            detailed_description += "\n\n" + UserString("SPECIES_THAT_DISLIKE");
            detailed_description.append(LinkList(species_that_dislike));
        }

        if (GetOptionsDB().Get<bool>("resource.effects.description.shown")) {
            detailed_description += "\n\n";
            if (part->Location())
                detailed_description += "\n" + part->Location()->Dump();
            if (!part->Effects().empty())
                detailed_description += "\n" + Dump(part->Effects());
        }
        detailed_description.append("\n");
    }

    void RefreshDetailPanelShipHullTag(     const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, bool only_description = false)
    {
        const ShipHull* hull = GetShipHull(item_name);
        if (!hull) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find hull with name " << item_name;
            return;
        }
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();

        // Ship Hulls
        if (!only_description) {
            name = UserString(item_name);
            texture = ClientUI::HullTexture(item_name);
            int default_location_id = DefaultLocationForEmpire(client_empire_id);
            turns = hull->ProductionTime(client_empire_id, default_location_id);
            cost = hull->ProductionCost(client_empire_id, default_location_id);
            cost_units = UserString("ENC_PP");
            general_type = UserString("ENC_SHIP_HULL");
        }

        std::string slots_list;
        for (auto slot_type : {ShipSlotType::SL_EXTERNAL, ShipSlotType::SL_INTERNAL, ShipSlotType::SL_CORE})
            slots_list += UserString(to_string((slot_type))) + ": " + std::to_string(hull->NumSlots(slot_type)) + "\n";
        detailed_description += UserString(hull->Description()) + "\n\n" + str(FlexibleFormat(UserString("HULL_DESC"))
            % hull->Speed()
            % hull->Fuel()
            % hull->Stealth()
            % hull->Structure()
            % slots_list);

        static std::vector<std::string> hull_tags_to_describe = UserStringList("FUNCTIONAL_HULL_DESC_TAGS_LIST");
        for (const std::string& tag : hull_tags_to_describe) {
            if (hull->HasTag(tag)) {
                if (UserStringExists("HULL_DESC_" + tag)) {
                    detailed_description += "\n\n" + UserString("HULL_DESC_" + tag);
                } else {
                    detailed_description += "\n\n" + tag;
                }
            }
        }

        const auto& exclusions = hull->Exclusions();
        if (!exclusions.empty()) {
            detailed_description += "\n\n" + UserString("ENC_SHIP_EXCLUSIONS");
            detailed_description.append(LinkList(exclusions));
        }

        auto unlocked_by_techs = TechsThatUnlockItem(UnlockableItem(UnlockableItemType::UIT_SHIP_HULL, item_name));
        if (!unlocked_by_techs.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            detailed_description.append(LinkList(unlocked_by_techs));
            detailed_description += "\n\n";
        }

        auto unlocked_by_policies = PoliciesThatUnlockItem(UnlockableItem(UnlockableItemType::UIT_SHIP_HULL, item_name));
        if (!unlocked_by_policies.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            detailed_description.append(LinkList(unlocked_by_policies));
        }

        // species that like / dislike hull
        auto species_that_like = GetSpeciesManager().SpeciesThatLike(item_name);
        auto species_that_dislike = GetSpeciesManager().SpeciesThatDislike(item_name);
        if (!species_that_like.empty()) {
            detailed_description += "\n\n" + UserString("SPECIES_THAT_LIKE");
            detailed_description.append(LinkList(species_that_like));
        }
        if (!species_that_dislike.empty()) {
            detailed_description += "\n\n" + UserString("SPECIES_THAT_DISLIKE");
            detailed_description.append(LinkList(species_that_dislike));
        }

        if (GetOptionsDB().Get<bool>("resource.effects.description.shown")) {
            detailed_description += "\n\n";
            if (hull->Location())
                detailed_description += "\n" + hull->Location()->Dump();
            if (!hull->Effects().empty())
                detailed_description += "\n" + Dump(hull->Effects());
        }
        detailed_description.append("\n");
    }

    void RefreshDetailPanelTechTag(         const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, bool only_description = false)
    {
        const Tech* tech = GetTech(item_name);
        if (!tech) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find tech with name " << item_name;
            return;
        }
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();

        // Technologies
        if (!only_description) {
            name = UserString(item_name);
            texture = ClientUI::TechIcon(item_name);
            other_texture = ClientUI::CategoryIcon(tech->Category());
            color = ClientUI::CategoryColor(tech->Category());
            ScriptingContext context;
            turns = tech->ResearchTime(client_empire_id, context);
            cost = tech->ResearchCost(client_empire_id, context);
            cost_units = UserString("ENC_RP");
            general_type = UserString(tech->ShortDescription());
            specific_type = str(FlexibleFormat(UserString("ENC_TECH_DETAIL_TYPE_STR")) % UserString(tech->Category()));
        }

        const auto& unlocked_techs = tech->UnlockedTechs();
        const auto& unlocked_items = tech->UnlockedItems();
        if (!unlocked_techs.empty() || !unlocked_items.empty())
            detailed_description += UserString("ENC_UNLOCKS");

        if (!unlocked_techs.empty()) {
            for (const auto& tech_name : unlocked_techs) {
                std::string link_text = LinkTaggedPresetText(VarText::TECH_TAG, tech_name, UserString(tech_name));
                detailed_description += str(FlexibleFormat(UserString("ENC_TECH_DETAIL_UNLOCKED_ITEM_STR"))
                    % UserString("UIT_TECH")
                    % link_text);
            }
        }

        if (!unlocked_items.empty()) {
            for (const UnlockableItem& item : unlocked_items) {
                auto TAG = [type{item.type}]() -> std::string_view {
                    switch (type) {
                    case UnlockableItemType::UIT_BUILDING:    return VarText::BUILDING_TYPE_TAG;     break;
                    case UnlockableItemType::UIT_SHIP_PART:   return VarText::SHIP_PART_TAG;         break;
                    case UnlockableItemType::UIT_SHIP_HULL:   return VarText::SHIP_HULL_TAG;         break;
                    case UnlockableItemType::UIT_SHIP_DESIGN: return VarText::PREDEFINED_DESIGN_TAG; break;
                    case UnlockableItemType::UIT_TECH:        return VarText::TECH_TAG;              break;
                    case UnlockableItemType::UIT_POLICY:      return VarText::POLICY_TAG;            break;
                    default:                                  return "";
                    }
                }();

                std::string link_text = TAG.empty()
                    ? UserString(item.name)
                    : LinkTaggedPresetText(TAG, item.name, UserString(item.name));

                detailed_description += str(FlexibleFormat(UserString("ENC_TECH_DETAIL_UNLOCKED_ITEM_STR"))
                    % UserString(to_string(item.type))
                    % link_text);
            }
        }

        if (!unlocked_techs.empty() || !unlocked_items.empty())
            detailed_description += "\n";

        detailed_description += UserString(tech->Description());

        const auto& unlocked_by_techs = tech->Prerequisites();
        if (!unlocked_by_techs.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            detailed_description.append(LinkList(unlocked_by_techs));
            detailed_description += "\n\n";
        }

        if (GetOptionsDB().Get<bool>("resource.effects.description.shown") && !tech->Effects().empty())
            detailed_description += "\n" + Dump(tech->Effects());
    }

    void RefreshDetailPanelPolicyTag(       const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, bool only_description = false)
    {
        const Policy* policy = GetPolicy(item_name);
        if (!policy) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find policy with name " << item_name;
            return;
        }
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();

        // Policies
        if (!only_description) {
            name = UserString(item_name);
            texture = ClientUI::PolicyIcon(item_name);
            const ScriptingContext context;
            cost = policy->AdoptionCost(client_empire_id, context);
            cost_units = UserString("ENC_IP");
            general_type = UserString(policy->ShortDescription());
            specific_type = str(FlexibleFormat(UserString("ENC_POLICY_DETAIL_TYPE_STR")) % UserString(policy->Category()));
        }
        detailed_description += UserString(policy->Description());

        const auto& unlocked_items = policy->UnlockedItems();
        if (!unlocked_items.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKS");
            for (const UnlockableItem& item : unlocked_items) {
                std::string TAG;
                switch (item.type) {
                case UnlockableItemType::UIT_BUILDING:    TAG = VarText::BUILDING_TYPE_TAG;     break;
                case UnlockableItemType::UIT_SHIP_PART:   TAG = VarText::SHIP_PART_TAG;         break;
                case UnlockableItemType::UIT_SHIP_HULL:   TAG = VarText::SHIP_HULL_TAG;         break;
                case UnlockableItemType::UIT_SHIP_DESIGN: TAG = VarText::PREDEFINED_DESIGN_TAG; break;
                case UnlockableItemType::UIT_TECH:        TAG = VarText::TECH_TAG;              break;
                case UnlockableItemType::UIT_POLICY:      TAG = VarText::POLICY_TAG;            break;
                default: break;
                }

                std::string link_text = TAG.empty()
                    ? UserString(item.name)
                    : LinkTaggedPresetText(TAG, item.name, UserString(item.name));

                detailed_description += str(FlexibleFormat(UserString("ENC_TECH_DETAIL_UNLOCKED_ITEM_STR"))
                    % UserString(to_string(item.type))
                    % link_text);
            }
        }

        auto unlocked_by_techs = TechsThatUnlockItem(UnlockableItem(UnlockableItemType::UIT_POLICY, item_name));
        if (!unlocked_by_techs.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            detailed_description.append(LinkList(unlocked_by_techs));
        }

        auto unlocked_by_policies = PoliciesThatUnlockItem(UnlockableItem(UnlockableItemType::UIT_POLICY, item_name));
        if (!unlocked_by_policies.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            detailed_description.append(LinkList(unlocked_by_policies));
        }

        if (!policy->Prerequisites().empty()) {
            detailed_description += "\n\n" + UserString("ENC_POICY_PREREQUISITES");
            detailed_description.append(LinkList(policy->Prerequisites()));
        }

        if (!policy->Exclusions().empty()) {
            detailed_description += "\n\n" + UserString("ENC_POICY_EXCLUSIONS");
            detailed_description.append(LinkList(policy->Exclusions()));
        }

        // species that like / dislike policy
        auto species_that_like = GetSpeciesManager().SpeciesThatLike(item_name);
        auto species_that_dislike = GetSpeciesManager().SpeciesThatDislike(item_name);
        if (!species_that_like.empty()) {
            detailed_description += "\n\n" + UserString("SPECIES_THAT_LIKE");
            detailed_description.append(LinkList(species_that_like));
        }
        if (!species_that_dislike.empty()) {
            detailed_description += "\n\n" + UserString("SPECIES_THAT_DISLIKE");
            detailed_description.append(LinkList(species_that_dislike));
        }
        detailed_description += "\n";

        if (GetOptionsDB().Get<bool>("resource.effects.description.shown") &&
            !policy->Effects().empty())
        { detailed_description += "\n" + Dump(policy->Effects()); }
        detailed_description.append("\n");
    }

    void RefreshDetailPanelBuildingTypeTag( const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, bool only_description = false)
    {
        const BuildingType* building_type = GetBuildingType(item_name);
        if (!building_type) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find building type with name " << item_name;
            return;
        }
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();

        int this_location_id = ClientUI::GetClientUI()->GetMapWnd()->SelectedPlanetID();
        if (this_location_id == INVALID_OBJECT_ID)
            this_location_id = DefaultLocationForEmpire(client_empire_id);

        // Building types
        if (!only_description) {
            name = UserString(item_name);
            texture = ClientUI::BuildingIcon(item_name);
            turns = building_type->ProductionTime(client_empire_id, this_location_id);
            cost = building_type->ProductionCost(client_empire_id, this_location_id);
            cost_units = UserString("ENC_PP");
            general_type = UserString("ENC_BUILDING_TYPE");
        }


        detailed_description += UserString(building_type->Description());

        if (building_type->ProductionCostTimeLocationInvariant()) {
            detailed_description += str(FlexibleFormat(UserString("ENC_AUTO_TIME_COST_INVARIANT_STR")) % UserString("ENC_VERB_PRODUCE_STR"));
        } else {
            detailed_description += str(FlexibleFormat(UserString("ENC_AUTO_TIME_COST_VARIABLE_STR")) % UserString("ENC_VERB_PRODUCE_STR"));
            if (auto planet = Objects().get<Planet>(this_location_id)) {
                int local_cost = building_type->ProductionCost(client_empire_id, this_location_id);
                int local_time = building_type->ProductionTime(client_empire_id, this_location_id);
                auto& local_name = planet->Name();
                detailed_description += str(FlexibleFormat(UserString("ENC_AUTO_TIME_COST_VARIABLE_DETAIL_STR")) 
                                        % local_name % local_cost % cost_units % local_time);
            }
        }

        auto unlocked_by_techs = TechsThatUnlockItem(UnlockableItem(UnlockableItemType::UIT_BUILDING, item_name));
        if (!unlocked_by_techs.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            detailed_description.append(LinkList(unlocked_by_techs));
            detailed_description += "\n\n";
        }

        auto unlocked_by_policies = PoliciesThatUnlockItem(UnlockableItem(UnlockableItemType::UIT_BUILDING, item_name));
        if (!unlocked_by_policies.empty()) {
            detailed_description += "\n\n" + UserString("ENC_UNLOCKED_BY");
            detailed_description.append(LinkList(unlocked_by_policies));
        }

        // species that like / dislike building type
        auto species_that_like = GetSpeciesManager().SpeciesThatLike(item_name);
        auto species_that_dislike = GetSpeciesManager().SpeciesThatDislike(item_name);
        if (!species_that_like.empty()) {
            detailed_description += "\n\n" + UserString("SPECIES_THAT_LIKE");
            detailed_description.append(LinkList(species_that_like));
        }
        if (!species_that_dislike.empty()) {
            detailed_description += "\n\n" + UserString("SPECIES_THAT_DISLIKE");
            detailed_description.append(LinkList(species_that_dislike));
        }

        if (GetOptionsDB().Get<bool>("resource.effects.description.shown")) {
            if (!building_type->ProductionCostTimeLocationInvariant()) {
                auto& cost_template{UserString("PRODUCTION_WND_TOOLTIP_PROD_COST")};
                auto& time_template{UserString("PRODUCTION_WND_TOOLTIP_PROD_TIME_MINIMUM")};

                if (building_type->Cost() && !building_type->Cost()->ConstantExpr())
                    detailed_description += "\n\n" + str(FlexibleFormat(cost_template) % building_type->Cost()->Dump());
                if (building_type->Time() && !building_type->Time()->ConstantExpr())
                    detailed_description += "\n\n" + str(FlexibleFormat(time_template) % building_type->Time()->Dump());
            }

            // TODO: Consider using UserString instead of hard-coded english text here...
            // Not a high priority as it's mainly inteded for debugging.
            if (building_type->EnqueueLocation())
                detailed_description += "\n\nEnqueue Requirement:\n" + building_type->EnqueueLocation()->Dump();
            if (building_type->Location())
                detailed_description += "\n\nLocation Requirement:\n" + building_type->Location()->Dump();
            if (!building_type->Effects().empty())
                detailed_description += "\n\nEffects:\n" + Dump(building_type->Effects());
        }
        detailed_description.append("\n");
    }

    void RefreshDetailPanelSpecialTag(      const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, bool only_description = false)
    {
        const Special* special = GetSpecial(item_name);
        if (!special) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find special with name " << item_name;
            return;
        }
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
        const Universe& u = GetUniverse();
        const ObjectMap& objects = u.Objects();


        // Specials
        if (!only_description) {
            name = UserString(item_name);
            texture = ClientUI::SpecialIcon(item_name);
            detailed_description = special->Description();
            general_type = UserString("ENC_SPECIAL");
        }


        // objects that have special
        std::vector<std::shared_ptr<const UniverseObject>> objects_with_special;
        objects_with_special.reserve(objects.size());
        for (const auto& obj : objects.all())
            if (obj->Specials().count(item_name))
                objects_with_special.push_back(obj);

        if (!objects_with_special.empty()) {
            detailed_description += "\n\n" + UserString("OBJECTS_WITH_SPECIAL");
            bool first = true;
            for (auto& obj : objects_with_special) {
                if (first) first = false;
                else detailed_description.append(", ");
                auto TEXT_TAG = [&obj]() -> std::string_view {
                    switch (obj->ObjectType()) {
                    case UniverseObjectType::OBJ_SHIP:      return VarText::SHIP_ID_TAG;    break;
                    case UniverseObjectType::OBJ_FLEET:     return VarText::FLEET_ID_TAG;   break;
                    case UniverseObjectType::OBJ_PLANET:    return VarText::PLANET_ID_TAG;  break;
                    case UniverseObjectType::OBJ_BUILDING:  return VarText::BUILDING_ID_TAG;break;
                    case UniverseObjectType::OBJ_SYSTEM:    return VarText::SYSTEM_ID_TAG;  break;
                    default:                                return "";
                    }
                }();

                if (!TEXT_TAG.empty())
                    detailed_description.append(
                        LinkTaggedIDText(TEXT_TAG, obj->ID(), obj->PublicName(client_empire_id, u))
                    );
                else
                    detailed_description.append(obj->PublicName(client_empire_id, u));
            }
            detailed_description += "\n";
        }


        // species that like / dislike special
        auto species_that_like = GetSpeciesManager().SpeciesThatLike(item_name);
        auto species_that_dislike = GetSpeciesManager().SpeciesThatDislike(item_name);
        if (!species_that_like.empty()) {
            detailed_description.append("\n\n").append(UserString("SPECIES_THAT_LIKE"));
            detailed_description.append(LinkList(species_that_like));
        }
        if (!species_that_dislike.empty()) {
            detailed_description.append("\n\n").append(UserString("SPECIES_THAT_DISLIKE"));
            detailed_description.append(LinkList(species_that_dislike));
        }


        if (GetOptionsDB().Get<bool>("resource.effects.description.shown")) {
            if (special->Location())
                detailed_description.append("\n").append(special->Location()->Dump());
            if (!special->Effects().empty())
                detailed_description.append("\n").append(Dump(special->Effects()));
        }
        detailed_description.append("\n");
    }

    void RefreshDetailPanelEmpireTag(       const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, bool only_description = false)
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

        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
        const Universe& universe = GetUniverse();
        const ObjectMap& objects = universe.Objects();

        if (!only_description)
            name = empire->Name();

        // Capital
        auto capital = objects.get<Planet>(empire->CapitalID());
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


        // Policies
        auto policies = empire->AdoptedPolicies();
        if (!policies.empty()) {
            // re-sort by adoption turn
            std::multimap<int, std::string_view> turns_policies_adopted;
            for (auto& policy_name : policies) {
                int turn = empire->TurnPolicyAdopted(policy_name);
                turns_policies_adopted.emplace(turn, policy_name);
            }

            detailed_description.append("\n").append(UserString("ADOPTED_POLICIES"));
            for (auto& [adoption_turn, policy_name] : turns_policies_adopted) {
                detailed_description += "\n";
                std::string turn_text{adoption_turn == BEFORE_FIRST_TURN ? UserString("BEFORE_FIRST_TURN") :
                    (UserString("TURN") + " " + std::to_string(adoption_turn))};
                detailed_description.append(LinkTaggedPresetText(VarText::POLICY_TAG, policy_name, UserString(policy_name)))
                    .append(" : ").append(turn_text);
            }
        } else {
            detailed_description.append("\n\n").append(UserString("NO_POLICIES_ADOPTED"));
        }


        // Planets
        auto empire_planets = objects.find<Planet>(OwnedVisitor(empire_id));
        if (!empire_planets.empty()) {
            detailed_description.append("\n\n").append(UserString("OWNED_PLANETS"));
            for (auto& obj : empire_planets) {
                detailed_description += LinkTaggedIDText(VarText::PLANET_ID_TAG, obj->ID(),
                                                         obj->PublicName(client_empire_id, universe)) + ",  ";
            }
        } else {
            detailed_description.append("\n\n").append(UserString("NO_OWNED_PLANETS_KNOWN"));
        }

        // Fleets
        std::vector<const Fleet*> nonempty_empire_fleets;
        auto&& empire_owned_fleets{objects.find<Fleet>(OwnedVisitor(empire_id))};
        nonempty_empire_fleets.reserve(empire_owned_fleets.size());
        for (const auto& fleet : empire_owned_fleets) {
            if (!fleet->Empty())
                nonempty_empire_fleets.emplace_back(fleet.get());
        }
        if (!nonempty_empire_fleets.empty()) {
            detailed_description.append("\n\n").append(UserString("OWNED_FLEETS")).append("\n");
            for (auto* obj : nonempty_empire_fleets) {
                auto&& fleet_link = LinkTaggedIDText(VarText::FLEET_ID_TAG, obj->ID(),
                                                     obj->PublicName(client_empire_id, universe));
                if (auto system = objects.getRaw<System>(obj->SystemID())) {
                    auto sys_name = system->ApparentName(client_empire_id, universe);
                    auto&& system_link = LinkTaggedIDText(VarText::SYSTEM_ID_TAG, system->ID(), sys_name);
                    detailed_description += str(FlexibleFormat(UserString("OWNED_FLEET_AT_SYSTEM"))
                                            % fleet_link % system_link);
                } else {
                    detailed_description += fleet_link;
                }
                detailed_description += "\n";
            }
        } else {
            detailed_description.append("\n\n").append(UserString("NO_OWNED_FLEETS_KNOWN"));
        }

        // Issued orders this turn
        detailed_description.append("\n\n").append(UserString("ISSUED_ORDERS"))
            .append("\n").append(GGHumanClientApp::GetApp()->Orders().Dump());

        // Techs
        auto& techs = empire->ResearchedTechs();
        if (!techs.empty()) {
            detailed_description.append("\n\n").append(UserString("RESEARCHED_TECHS"));
            std::multimap<int, std::string> sorted_techs;
            for (auto& [tech_name, researched_turn] : techs)
                sorted_techs.emplace(researched_turn, tech_name);

            for (const auto& [researched_turn, tech_name] : sorted_techs) {
                detailed_description += "\n";
                std::string turn_text;
                if (researched_turn == BEFORE_FIRST_TURN)
                    turn_text = UserString("BEFORE_FIRST_TURN");
                else
                    turn_text = UserString("TURN") + " " + std::to_string(researched_turn);
                detailed_description.append(LinkTaggedPresetText(VarText::TECH_TAG, tech_name, UserString(tech_name)))
                    .append(" : ").append(turn_text);
            }
        } else {
            detailed_description.append("\n\n").append(UserString("NO_TECHS_RESEARCHED"));
        }

        // WIP: Parts, Hulls, Buildings, ... available
        auto& parts = empire->AvailableShipParts();
        if (!parts.empty()) {
            detailed_description.append("\n\n").append(UserString("AVAILABLE_PARTS"));
            for (const auto& part_name : parts) {
                detailed_description.append("\n")
                    .append(LinkTaggedPresetText(VarText::SHIP_PART_TAG, part_name, UserString(part_name)));
            }
        } else {
            detailed_description.append("\n\n").append(UserString("NO_PARTS_AVAILABLE"));
        }
        auto& hulls = empire->AvailableShipHulls();
        if (!hulls.empty()) {
            detailed_description.append("\n\n").append(UserString("AVAILABLE_HULLS"));
            for (const auto& hull_name : hulls) {
                detailed_description.append("\n")
                    .append(LinkTaggedPresetText(VarText::SHIP_HULL_TAG, hull_name, UserString(hull_name)));
            }
        } else {
            detailed_description.append("\n\n").append(UserString("NO_HULLS_AVAILABLE"));
        }
        auto& buildings = empire->AvailableBuildingTypes();
        if (!buildings.empty()) {
            detailed_description.append("\n\n").append(UserString("AVAILABLE_BUILDINGS"));
            for (const auto& building_name : buildings) {
                detailed_description.append("\n")
                    .append(LinkTaggedPresetText(VarText::BUILDING_TYPE_TAG, building_name, UserString(building_name)));
            }
        } else {
            detailed_description.append("\n\n").append(UserString("NO_BUILDINGS_AVAILABLE"));
        }

        // Misc. Statistics

        // empire destroyed ships...
        const auto& empire_ships_destroyed = empire->EmpireShipsDestroyed();
        if (!empire_ships_destroyed.empty())
            detailed_description.append("\n\n").append(UserString("EMPIRE_SHIPS_DESTROYED"));
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
            detailed_description.append("\n\n").append(UserString("SHIP_DESIGNS_DESTROYED"));
        for (const auto& entry : empire_designs_destroyed) {
            std::string num_str = std::to_string(entry.second);
            const ShipDesign* design = GetUniverse().GetShipDesign(entry.first);
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
            detailed_description.append("\n\n").append(UserString("SPECIES_SHIPS_DESTROYED"));
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
            detailed_description.append("\n\n").append(UserString("SPECIES_PLANETS_INVADED"));
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
            detailed_description.append("\n\n").append(UserString("SPECIES_SHIPS_PRODUCED"));
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
            detailed_description.append("\n\n").append(UserString("SHIP_DESIGNS_PRODUCED"));
        for (const auto& entry : ship_designs_produced) {
            std::string num_str = std::to_string(entry.second);
            const ShipDesign* design = GetUniverse().GetShipDesign(entry.first);
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
            detailed_description.append("\n\n").append(UserString("SPECIES_SHIPS_LOST"));
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
            detailed_description.append("\n\n").append(UserString("SHIP_DESIGNS_LOST"));
        for (const auto& entry : ship_designs_lost) {
            std::string num_str = std::to_string(entry.second);
            const ShipDesign* design = GetUniverse().GetShipDesign(entry.first);
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
            detailed_description.append("\n\n").append(UserString("SPECIES_SHIPS_SCRAPPED"));
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
            detailed_description.append("\n\n").append(UserString("SHIP_DESIGNS_SCRAPPED"));
        for (const auto& entry : ship_designs_scrapped) {
            std::string num_str = std::to_string(entry.second);
            const ShipDesign* design = GetUniverse().GetShipDesign(entry.first);
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
            detailed_description.append("\n\n").append(UserString("SPECIES_PLANETS_DEPOPED"));
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
            detailed_description.append("\n\n").append(UserString("SPECIES_PLANETS_BOMBED"));
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
            detailed_description.append("\n\n").append(UserString("BUILDING_TYPES_PRODUCED"));
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
            detailed_description.append("\n\n").append(UserString("BUILDING_TYPES_SCRAPPED"));
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
                                            GG::Clr& color, bool only_description = false)
    {
        const SpeciesManager& sm = GetSpeciesManager();
        const Species* species = sm.GetSpecies(item_name);
        if (!species) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find species with name " << item_name;
            return;
        }
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();

        const Universe& universe = GetUniverse();
        const ObjectMap& objects = universe.Objects();

        if (!only_description) {
            name = UserString(item_name);
            texture = ClientUI::SpeciesIcon(item_name);
            general_type = UserString("ENC_SPECIES");
        }

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
        if (!species->DefaultFocus().empty()) {
            detailed_description.append("\n\n").append(UserString("FOCUS_PREFERENCE"))
                .append(UserString(species->DefaultFocus()));
        }

        // likes
        if (!species->Likes().empty()) {
            detailed_description += "\n\n" + UserString("LIKES");
            detailed_description.append(LinkList(species->Likes()));
        }

        // dislikes
        if (!species->Dislikes().empty()) {
            detailed_description.append("\n\n").append(UserString("DISLIKES"));
            int count = 0;
            for (const auto& s : species->Dislikes())
                detailed_description
                .append(count++ == 0 ? "" : ",  ")
                .append(LinkStringIfPossible(s,UserString(s)))
                //.append(UserString(s))
                ;
        }

        // environmental preferences
        detailed_description += "\n\n";
        const auto& pt_env_map = species->PlanetEnvironments();
        if (!pt_env_map.empty()) {
            detailed_description.append(UserString("ENVIRONMENTAL_PREFERENCES")).append("\n");
            for (auto& [ptype, penv] : pt_env_map) {
                detailed_description += UserString(to_string(ptype)) + " : " +
                                        UserString(to_string(penv)) + "\n";
                // add blank line between cyclical environments and asteroids and gas giants
                if (ptype == PlanetType::PT_OCEAN)
                    detailed_description += "\n";
            }
        } else {
            detailed_description += "\n";
        }

        // homeworld
        detailed_description += "\n";
        const auto& homeworlds = GetSpeciesManager().GetSpeciesHomeworldsMap();
        if (!homeworlds.count(species->Name()) || homeworlds.at(species->Name()).empty()) {
            detailed_description.append(UserString("NO_HOMEWORLD")).append("\n");
        } else {
            detailed_description.append(UserString("HOMEWORLD")).append("\n");
            bool first = true;
            // TODO: alphabetical sorting order to make the list better readable
            // TODO: stealthy worlds should be hidden on the server side and not show up as clear text here, only when the empire has sufficient detection strength
            for (int hw_id : homeworlds.at(species->Name())) {
                if (first) first = false;
                else detailed_description.append(",  ");
                if (auto homeworld = objects.get<Planet>(hw_id))
                    detailed_description
                    .append(LinkTaggedIDText(VarText::PLANET_ID_TAG, hw_id, homeworld->PublicName(client_empire_id, universe)));
                else
                    detailed_description
                    .append(UserString("UNKNOWN_PLANET"));
            }
            detailed_description.append("\n");
        }

        // occupied planets
        std::vector<std::shared_ptr<const Planet>> species_occupied_planets;
        const auto& species_object_populations = sm.SpeciesObjectPopulations();
        species_occupied_planets.reserve(species_object_populations.size());
        auto sp_op_it = species_object_populations.find(item_name);
        if (sp_op_it != species_object_populations.end()) {
            const auto& object_pops = sp_op_it->second;
            for (const auto& object_pop : object_pops) {
                auto plt = objects.get<Planet>(object_pop.first);
                if (!plt)
                    continue;
                if (plt->SpeciesName() != item_name) {
                    ErrorLogger() << "SpeciesManager SpeciesObjectPopulations suggested planet had a species, but it doesn't?";
                    continue;
                }
                species_occupied_planets.push_back(std::move(plt));
            }
        }

        if (!species_occupied_planets.empty()) {
            detailed_description.append("\n").append(UserString("OCCUPIED_PLANETS")).append("\n");
            bool first = true;
            // TODO: alphabetical sorting order to make the list better readable
            for (auto& planet : species_occupied_planets) {
                if (first) first = false;
                else detailed_description.append(",  ");
                detailed_description
                .append(LinkTaggedIDText(VarText::PLANET_ID_TAG, planet->ID(), planet->PublicName(client_empire_id, universe)));
            }
            detailed_description.append("\n");
        }

        // empire opinions
        const auto& seom = GetSpeciesManager().GetSpeciesEmpireOpinionsMap();
        auto species_it = seom.find(species->Name());
        if (species_it != seom.end()) {
            detailed_description.append("\n").append(UserString("OPINIONS_OF_EMPIRES")).append("\n");
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
            detailed_description.append("\n").append(UserString("OPINIONS_OF_OTHER_SPECIES")).append("\n");
            for (const auto& entry : species_it2->second) {
                const Species* species2 = GetSpecies(entry.first);
                if (!species2)
                    continue;

                detailed_description += UserString(species2->Name()) + " : " + DoubleToString(entry.second, 3, false) + "\n";
            }
        }

        // species that like / dislike other species
        auto species_that_like = GetSpeciesManager().SpeciesThatLike(item_name);
        auto species_that_dislike = GetSpeciesManager().SpeciesThatDislike(item_name);
        if (!species_that_like.empty()) {
            detailed_description.append("\n\n").append(UserString("SPECIES_THAT_LIKE"));
            detailed_description.append(LinkList(species_that_like));
        }
        if (!species_that_dislike.empty()) {
            detailed_description.append("\n\n").append(UserString("SPECIES_THAT_DISLIKE"));
            detailed_description.append(LinkList(species_that_dislike));
        }

        // Long description
        detailed_description.append("\n\n").append(UserString(species->Description()));

        // autogenerated dump text of parsed scripted species effects, if enabled in options
        if (GetOptionsDB().Get<bool>("resource.effects.description.shown") && !species->Effects().empty())
            detailed_description.append("\n").append(Dump(species->Effects()));
        detailed_description.append("\n");
    }

    void RefreshDetailPanelFieldTypeTag(const std::string&                  item_type,
                                        const std::string&                  item_name,
                                              std::string&                  name,
                                              std::shared_ptr<GG::Texture>& texture,
                                              std::shared_ptr<GG::Texture>& other_texture,
                                              int&                          turns,
                                              float&                        cost,
                                              std::string&                  cost_units,
                                              std::string&                  general_type,
                                              std::string&                  specific_type,
                                              std::string&                  detailed_description,
                                              GG::Clr&                      color,
                                              bool                          only_description = false
    ) {
        // DONE: list current known occurances of the fieldtype with id / galactic coordinates to be clickable like instances of ship designs
        // TODO: second method for detailled view of particular field instances, listing affected systems, fleets, etc.
        const FieldType* field_type = GetFieldType(item_name);
        if (!field_type) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find fiedl type with name " << item_name;
            return;
        }

        // Field types
        if (!only_description) {
            name = UserString(item_name);
            texture = ClientUI::FieldTexture(item_name);
            general_type = UserString("ENC_FIELD_TYPE");
        }

        detailed_description += UserString(field_type->Description());


        // species that like / dislike field
        auto species_that_like = GetSpeciesManager().SpeciesThatLike(item_name);
        auto species_that_dislike = GetSpeciesManager().SpeciesThatDislike(item_name);
        if (!species_that_like.empty()) {
            detailed_description.append("\n\n").append(UserString("SPECIES_THAT_LIKE"));
            detailed_description.append(LinkList(species_that_like));
        }
        if (!species_that_dislike.empty()) {
            detailed_description.append("\n\n").append(UserString("SPECIES_THAT_DISLIKE"));
            detailed_description.append(LinkList(species_that_dislike));
        }


        if (GetOptionsDB().Get<bool>("resource.effects.description.shown")) {
            if (!field_type->Effects().empty())
                detailed_description.append("\n").append(Dump(field_type->Effects()));
        }

        // get current fields from map
        const ScriptingContext context;
        const Universe& u = GetUniverse();
        const ObjectMap& objects = u.Objects();
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
        std::vector<std::shared_ptr<const UniverseObject>> current_fields;
        current_fields.reserve(objects.size());
        for (const auto& obj : objects.all())
            if (
                obj->ObjectType() == UniverseObjectType::OBJ_FIELD
                && obj->PublicName(client_empire_id, u) == name
            ) {
                current_fields.push_back(obj);
            }
        detailed_description.append("\n\n").append(UserString("KNOWN_FIELDS_OF_THIS_TYPE")).append("\n");
        if (!current_fields.empty()) {
            for (auto& obj : current_fields) {
                auto TEXT_TAG = VarText::FIELD_ID_TAG;
                detailed_description.append(
                    LinkTaggedIDText(
                        TEXT_TAG, obj->ID(),
                        obj->PublicName(client_empire_id, u)
                    )
                );
                std::stringstream ssx,ssy;
                ssx << std::fixed << std::setprecision(0) << obj->X();
                ssy << std::fixed << std::setprecision(0) << obj->Y();
                detailed_description.append(" @ X=").append(ssx.str());
                detailed_description.append(", Y=").append(ssy.str());
                detailed_description.append("\n");
            }
        } else detailed_description.append(UserString("NO_KNOWN_FIELDS"));
        detailed_description.append("\n");
    }

    void RefreshDetailPanelMeterTypeTag(    const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, bool only_description = false)
    {
        MeterType meter_type = MeterType::INVALID_METER_TYPE;


        std::istringstream item_ss(item_name);
        item_ss >> meter_type;
        auto [meter_value_label, meter_name] = MeterValueLabelAndString(meter_type);
        std::string meter_name_value_desc{std::string{meter_name}.append("_VALUE_DESC")};

        if (!only_description) {
            texture = ClientUI::MeterIcon(meter_type);
            general_type = UserString("ENC_METER_TYPE");
            name = meter_value_label;
        }

        if (UserStringExists(meter_name_value_desc))
            detailed_description += UserString(meter_name_value_desc);
        else
            detailed_description += meter_value_label;
    }

    std::string GetDetailedDescriptionBase(const ShipDesign* design) {
        std::string hull_link;
        if (!design->Hull().empty())
            hull_link = LinkTaggedPresetText(VarText::SHIP_HULL_TAG, design->Hull(), UserString(design->Hull()));

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
            auto& [part_name, part_count] = *part_it;
            parts_list += LinkTaggedPresetText(VarText::SHIP_PART_TAG, part_name, UserString(part_name));
            if (part_it->second > 1)
                parts_list.append(" x").append(std::to_string(part_count));
        }
        return str(FlexibleFormat(UserString("ENC_SHIP_DESIGN_DESCRIPTION_BASE_STR"))
            % design->Description()
            % hull_link
            % parts_list);
    }

    std::string GetDetailedDescriptionStats(const std::shared_ptr<Ship>& ship,
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
        const Universe& universe = GetUniverse();
        const ScriptingContext context{universe, Empires()};

        auto& species = ship->SpeciesName().empty() ? "Generic" : UserString(ship->SpeciesName());
        float structure = ship->GetMeter(MeterType::METER_MAX_STRUCTURE)->Current();
        float shield = ship->GetMeter(MeterType::METER_MAX_SHIELD)->Current();
        float attack = ship->TotalWeaponsShipDamage(context);
        float destruction = ship->TotalWeaponsFighterDamage(context);
        float strength = std::pow(attack * structure, 0.6f);
        float typical_shot = enemy_shots.empty() ? 0.0f : *std::max_element(enemy_shots.begin(), enemy_shots.end()); // TODO: cbegin, cend (also elsewhere)
        float typical_strength = std::pow(ship->TotalWeaponsShipDamage(context, enemy_DR) * structure * typical_shot / std::max(typical_shot - shield, 0.001f), 0.6f); // FIXME: TotalWeaponsFighterDamage 
        return (FlexibleFormat(UserString("ENC_SHIP_DESIGN_DESCRIPTION_STATS_STR"))
            % species
            % attack
            % destruction
            % structure
            % shield
            % ship->GetMeter(MeterType::METER_DETECTION)->Current()
            % ship->GetMeter(MeterType::METER_STEALTH)->Current()
            % ship->GetMeter(MeterType::METER_SPEED)->Current()
            % ship->GetMeter(MeterType::METER_MAX_FUEL)->Current()
            % ship->SumCurrentPartMeterValuesForPartClass(MeterType::METER_CAPACITY, ShipPartClass::PC_COLONY, universe)
            % ship->SumCurrentPartMeterValuesForPartClass(MeterType::METER_CAPACITY, ShipPartClass::PC_TROOPS, universe)
            % ship->FighterMax()
            % (attack - ship->TotalWeaponsShipDamage(context, 0.0f, false)) // FIXME: TotalWeaponsFighterDamage
            % ship->SumCurrentPartMeterValuesForPartClass(MeterType::METER_MAX_CAPACITY, ShipPartClass::PC_FIGHTER_BAY, universe)
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
                                            GG::Clr& color, bool only_description = false)
    {
        int design_id = INVALID_DESIGN_ID;
        try {
            design_id = boost::lexical_cast<int>(item_name);
        } catch (...) {
            ErrorLogger() << "RefreshDetailPanelShipDesignTag couldn't convert name to design ID: " << item_name;
            return;
        }
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
        ScriptingContext context;
        Universe& universe = context.ContextUniverse();
        ObjectMap& objects = context.ContextObjects();
        const SpeciesManager& species_manager = context.species;

        const ShipDesign* design = universe.GetShipDesign(design_id);
        if (!design) {
            ErrorLogger() << "RefreshDetailPanelShipDesignTag couldn't find ShipDesign with id " << item_name;
            return;
        }

        universe.InhibitUniverseObjectSignals(true);


        // Ship Designs
        if (!only_description) {
            name = design->Name();
            texture = ClientUI::ShipDesignIcon(design_id);
            int default_location_id = DefaultLocationForEmpire(client_empire_id);
            turns = design->ProductionTime(client_empire_id, default_location_id);
            cost = design->ProductionCost(client_empire_id, default_location_id);
            cost_units = UserString("ENC_PP");
            general_type = design->IsMonster() ? UserString("ENC_MONSTER") : UserString("ENC_SHIP_DESIGN");
        }

        float tech_level = boost::algorithm::clamp(context.current_turn / 400.0f, 0.0f, 1.0f);
        float typical_shot = 3 + 27 * tech_level;
        float enemy_DR = 20 * tech_level;
        TraceLogger() << "RefreshDetailPanelShipDesignTag default enemy stats:: tech_level: "
                      << tech_level << "   DR: " << enemy_DR << "   attack: " << typical_shot;
        std::set<float> enemy_shots{typical_shot};


        // select which species to show info for

        // TODO: can this be a vector of string_view ?
        std::set<std::string> additional_species; // from currently selected planet and fleets, if any
        const auto& map_wnd = ClientUI::GetClientUI()->GetMapWnd();
        if (const auto planet = objects.get<Planet>(map_wnd->SelectedPlanetID())) {
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
                if (auto selected_fleet = objects.get<Fleet>(selected_fleet_id))
                    if (!selected_fleet->ShipIDs().empty())
                        selected_ship = *selected_fleet->ShipIDs().begin();
            }
        }

        if (selected_ship != INVALID_OBJECT_ID) {
            chosen_ships.insert(selected_ship);
            if (const auto this_ship = objects.get<Ship>(selected_ship)) {
                if (!this_ship->SpeciesName().empty())
                    additional_species.insert(this_ship->SpeciesName());
                if (!this_ship->OwnedBy(client_empire_id)) {
                    enemy_DR = this_ship->GetMeter(MeterType::METER_MAX_SHIELD)->Initial();
                    DebugLogger() << "Using selected ship for enemy values, DR: " << enemy_DR;
                    enemy_shots.clear();
                    auto this_damage = this_ship->AllWeaponsMaxShipDamage(context); // FIXME: FighterDamage
                    for (float shot : this_damage)
                        DebugLogger() << "Weapons Dmg " << shot;
                    enemy_shots.insert(this_damage.begin(), this_damage.end());
                }
            }
        } else if (fleet_manager.ActiveFleetWnd()) {
            for (const auto& fleet : objects.find<Fleet>(fleet_manager.ActiveFleetWnd()->SelectedFleetIDs())) {
                if (!fleet)
                    continue;
                chosen_ships.insert(fleet->ShipIDs().begin(), fleet->ShipIDs().end());
            }
        }
        for (const auto& this_ship : objects.find<Ship>(chosen_ships)) {
            if (!this_ship || !this_ship->SpeciesName().empty())
                continue;
            additional_species.emplace(this_ship->SpeciesName());
        }
        std::vector<std::string> species_list(additional_species.begin(), additional_species.end());
        detailed_description = GetDetailedDescriptionBase(design);


        if (!only_description) { // don't generate detailed stat description involving adding / removing temporary ship from universe
            // temporary ship to use for estimating design's meter values
            auto temp = universe.InsertTemp<Ship>(client_empire_id, design_id, "", universe,
                                                  species_manager, client_empire_id,
                                                  context.current_turn);

            // apply empty species for 'Generic' entry
            universe.UpdateMeterEstimates(temp->ID(), context);
            temp->Resupply();
            detailed_description.append(GetDetailedDescriptionStats(temp, design, enemy_DR, enemy_shots, cost));

            // apply various species to ship, re-calculating the meter values for each
            for (std::string& species_name : species_list) {
                temp->SetSpecies(std::move(species_name));
                universe.UpdateMeterEstimates(temp->ID(), context);
                temp->Resupply();
                detailed_description.append(GetDetailedDescriptionStats(temp, design, enemy_DR, enemy_shots, cost));
            }

            universe.Delete(temp->ID());
            universe.InhibitUniverseObjectSignals(false);
        }


        // ships of this design
        std::vector<const Ship*> design_ships;
        design_ships.reserve(objects.ExistingShips().size());
        for (const auto& entry : objects.ExistingShips()) {
            auto ship = static_cast<const Ship*>(entry.second.get());
            if (ship && ship->DesignID() == design_id)
                design_ships.emplace_back(ship);
        }
        if (!design_ships.empty()) {
            detailed_description += "\n\n" + UserString("SHIPS_OF_DESIGN");
            for (auto& ship : design_ships) {
                detailed_description += LinkTaggedIDText(VarText::SHIP_ID_TAG, ship->ID(),
                                                         ship->PublicName(client_empire_id, universe)) + ",  ";
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
                                            GG::Clr& color, std::weak_ptr<const ShipDesign>& inc_design,
                                            bool only_description = false)
    {
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
        general_type = UserString("ENC_INCOMPETE_SHIP_DESIGN");

        auto incomplete_design = inc_design.lock();
        if (!incomplete_design)
            return;

        if (only_description) {
            detailed_description = GetDetailedDescriptionBase(incomplete_design.get());
            return;
        }

        ScriptingContext context;
        Universe& universe = context.ContextUniverse();
        ObjectMap& objects = context.ContextObjects();
        const SpeciesManager& species_manager = context.species;

        universe.InhibitUniverseObjectSignals(true);


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


        universe.InsertShipDesignID(new ShipDesign(*incomplete_design), client_empire_id, incomplete_design->ID());
        detailed_description = GetDetailedDescriptionBase(incomplete_design.get());
        float tech_level = boost::algorithm::clamp(context.current_turn / 400.0f, 0.0f, 1.0f);
        float typical_shot = 3 + 27 * tech_level;
        float enemy_DR = 20 * tech_level;
        DebugLogger() << "default enemy stats:: tech_level: " << tech_level
                      << "   DR: " << enemy_DR << "   attack: " << typical_shot;
        std::set<float> enemy_shots;
        enemy_shots.insert(typical_shot);
        std::set<std::string> additional_species; // TODO: from currently selected planet and ship, if any
        const auto& map_wnd = ClientUI::GetClientUI()->GetMapWnd();
        if (const auto planet = objects.get<Planet>(map_wnd->SelectedPlanetID())) {
            if (!planet->SpeciesName().empty())
                additional_species.insert(planet->SpeciesName());
        }
        FleetUIManager& fleet_manager = FleetUIManager::GetFleetUIManager();
        std::set<int> chosen_ships;
        int selected_ship = fleet_manager.SelectedShipID();
        if (selected_ship != INVALID_OBJECT_ID) {
            chosen_ships.insert(selected_ship);
            if (auto this_ship = objects.get<const Ship>(selected_ship)) {
                if (!additional_species.empty() && (
                        (this_ship->GetMeter(MeterType::METER_MAX_SHIELD)->Initial() > 0) ||
                         !this_ship->OwnedBy(client_empire_id)))
                {
                    enemy_DR = this_ship->GetMeter(MeterType::METER_MAX_SHIELD)->Initial();
                    DebugLogger() << "Using selected ship for enemy values, DR: " << enemy_DR;
                    enemy_shots.clear();
                    auto this_damage = this_ship->AllWeaponsMaxShipDamage(context); // FIXME: MaxFighterDamage
                    for (float shot : this_damage)
                        DebugLogger() << "Weapons Dmg " << shot;
                    enemy_shots.insert(this_damage.begin(), this_damage.end());
                }
            }
        } else if (fleet_manager.ActiveFleetWnd()) {
            for (const auto& fleet : objects.find<const Fleet>(fleet_manager.ActiveFleetWnd()->SelectedFleetIDs())) {
                if (!fleet)
                    continue;
                const auto& fleet_ship_ids{fleet->ShipIDs()};
                chosen_ships.insert(fleet_ship_ids.begin(), fleet_ship_ids.end());
            }
        }
        for (const auto& this_ship : objects.find<const Ship>(chosen_ships)) {
            if (!this_ship || !this_ship->SpeciesName().empty())
                continue;
            additional_species.insert(this_ship->SpeciesName());
        }

        std::vector<std::string> species_list(additional_species.begin(), additional_species.end());


        // temporary ship to use for estimating design's meter values
        auto temp = universe.InsertTemp<Ship>(client_empire_id, incomplete_design->ID(), "",
                                              universe, species_manager, client_empire_id,
                                              context.current_turn);

        // apply empty species for 'Generic' entry
        universe.UpdateMeterEstimates(temp->ID(), context);
        temp->Resupply();
        detailed_description.append(GetDetailedDescriptionStats(temp, incomplete_design.get(),
                                                                enemy_DR, enemy_shots, cost));

        // apply various species to ship, re-calculating the meter values for each
        for (std::string& species_name : species_list) {
            temp->SetSpecies(std::move(species_name));
            GetUniverse().UpdateMeterEstimates(temp->ID(), context);
            temp->Resupply();
            detailed_description.append(GetDetailedDescriptionStats(temp, incomplete_design.get(),
                                                                    enemy_DR, enemy_shots, cost));
        }

        universe.Delete(temp->ID());
        universe.DeleteShipDesign(TEMPORARY_OBJECT_ID);
        universe.InhibitUniverseObjectSignals(false);
    }

    void RefreshDetailPanelObjectTag(       const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, bool only_description = false)
    {
        int id = boost::lexical_cast<int>(item_name);
        if (id == INVALID_OBJECT_ID)
            return;
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();

        const Universe& universe = GetUniverse();
        const ObjectMap& objects = universe.Objects();

        auto obj = objects.get(id);
        if (!obj) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't find UniverseObject with id " << item_name;
            return;
        }

        detailed_description = obj->Dump();

        if (only_description)
            return;

        name = obj->PublicName(client_empire_id, universe);
        general_type = GeneralTypeOfObject(obj->ObjectType());
        if (general_type.empty()) {
            ErrorLogger() << "EncyclopediaDetailPanel::Refresh couldn't interpret object: " << obj->Name()
                          << " (" << item_name << ")";
            return;
        }
    }

    std::vector<std::string_view> ReportedSpeciesForPlanet(Planet& planet) {
        std::vector<std::string_view> retval;

        const ObjectMap& objects = Objects();
        const SpeciesManager& species_manager = GetSpeciesManager();
        const EmpireManager& empires = Empires();

        auto empire_id = GGHumanClientApp::GetApp()->EmpireID();
        auto empire = empires.GetEmpire(empire_id);
        if (!empire)
            return retval;

        const auto& planet_current_species{planet.SpeciesName()};

        // Collect species colonizing/environment hospitality information
        // start by building roster-- any species tagged as 'ALWAYS_REPORT' plus any species
        // represented in this empire's PopCenters
        for (auto& [species_str, species] : species_manager) {
            if (!species)
                continue;
            if (species->HasTag(TAG_ALWAYS_REPORT)) {
                retval.push_back(species_str);
                continue;
            }
            if (species->HasTag(TAG_EXTINCT)) {
                for (auto& [tech_name, turn_researched] : empire->ResearchedTechs()) {
                    // Check for presence of tags in tech
                    auto tech = GetTech(tech_name);
                    if (!tech) {
                        ErrorLogger() << "ReportedSpeciesForPlanet couldn't get tech " << tech_name
                                      << " (researched on turn " << turn_researched << ")";
                        continue;
                    }
                    if (tech->HasTag(TAG_EXTINCT) && tech->HasTag(species_str)) {
                        retval.push_back(species_str);
                        break;
                    }
                }
            }
        }

        for (const auto& pop_center : objects.find<PopCenter>(empire->GetPopulationPool().PopCenterIDs())) {
            if (!pop_center)
                continue;

            const std::string& species_name = pop_center->SpeciesName();
            if (species_name.empty() ||
                std::any_of(retval.begin(), retval.end(),
                            [&species_name](std::string_view s) { return s == species_name; }))
            { continue; }

            const Species* species = species_manager.GetSpecies(species_name);
            if (!species)
                continue;

            // Exclude species that can't colonize UNLESS they
            // are already here (aka: it's their home planet). Showing them on
            // their own planet allows comparison vs other races, which might
            // be better suited to this planet. 
            if (species->CanColonize() || species_name == planet_current_species)
                // The planet's species may change, so better create a string_view
                // from the species->Name(), not species_name
                retval.push_back(species->Name());
        }

        return retval;
    }

    std::multimap<float, std::pair<std::string_view, PlanetEnvironment>>
        SpeciesEnvByTargetPop(const std::shared_ptr<Planet>& planet,
                              const std::vector<std::string_view>& species_names)
    {
        std::multimap<float, std::pair<std::string_view, PlanetEnvironment>> retval;

        if (species_names.empty() || !planet)
            return retval;

        // store original state of planet
        auto original_planet_species{planet->SpeciesName()}; // intentional copy
        auto original_owner_id = planet->Owner();
        auto orig_initial_target_pop = planet->GetMeter(MeterType::METER_TARGET_POPULATION)->Initial();

        std::vector<int> planet_id_vec{planet->ID()};
        auto empire_id = GGHumanClientApp::GetApp()->EmpireID();

        Universe& universe = GetUniverse();
        ScriptingContext context{universe, Empires(), GetGalaxySetupData(), GetSpeciesManager(), GetSupplyManager()};
        universe.InhibitUniverseObjectSignals(true);

        for (const auto& species_name : species_names) { // TODO: parallelize somehow? tricky since an existing planet is being modified, rather than adding a test planet...
            // Setting the planet's species allows all of it meters to reflect
            // species (and empire) properties, such as environment type
            // preferences and tech.
            // @see also: MapWnd::ApplyMeterEffectsAndUpdateMeters
            // NOTE: Overridding current or initial value of MeterType::METER_TARGET_POPULATION prior to update
            //       results in incorrect estimates for at least effects with a min target population of 0
            try {
                planet->SetSpecies(std::string{species_name});
                planet->SetOwner(empire_id);
                universe.ApplyMeterEffectsAndUpdateMeters(planet_id_vec, context, false);
            } catch (const std::exception& e) {
                ErrorLogger() << "Caught exception applying species name " << species_name
                              << " and owner empire id " << empire_id << " : " << e.what();
            }

            try {
                const auto species = context.species.GetSpecies(species_name);
                auto planet_environment = PlanetEnvironment::PE_UNINHABITABLE;
                if (species)
                    planet_environment = species->GetPlanetEnvironment(planet->Type());

                float planet_capacity = ((planet_environment == PlanetEnvironment::PE_UNINHABITABLE) ?
                                         0.0f : planet->GetMeter(MeterType::METER_TARGET_POPULATION)->Current()); // want value after temporary meter update, so get current, not initial value of meter

                retval.emplace(std::piecewise_construct, std::forward_as_tuple(planet_capacity),
                               std::forward_as_tuple(species_name, planet_environment));
            } catch (const std::exception& e) {
                ErrorLogger() << "Caught exception emplacing into species env by target pop : " << e.what();
            }
        }

        try {
            // restore planet to original state
            planet->SetSpecies(original_planet_species);
            planet->SetOwner(original_owner_id);
            planet->GetMeter(MeterType::METER_TARGET_POPULATION)->Set(orig_initial_target_pop, orig_initial_target_pop);
        } catch (const std::exception& e) {
            ErrorLogger() << "Caught exception restoring planet to original state after setting test species / owner : " << e.what();
        }

        try {
            universe.InhibitUniverseObjectSignals(false);
            universe.ApplyMeterEffectsAndUpdateMeters(planet_id_vec, context, false);
        } catch (const std::exception& e) {
            ErrorLogger() << "Caught exception re-applying meter effects to planet after temporary species / owner : " << e.what();
        }

        return retval;
    }

    GG::Pt HairSpaceExtent() {
        static GG::Pt retval;
        if (retval > GG::Pt(GG::X0, GG::Y0))
            return retval;

        GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE;
        auto font = ClientUI::GetFont();

#if defined(__cpp_lib_char8_t)
        static constexpr std::u8string_view hair_space_chars{u8"\u200A"};
        static const std::string hair_space_str{hair_space_chars.begin(), hair_space_chars.end()};
        DebugLogger() << "HairSpaceExtext with __cpp_lib_char8_t defined";
#else
        static constexpr std::string_view hair_space_chars{u8"\u200A"};
        static const std::string hair_space_str{hair_space_chars};
        DebugLogger() << "HairSpaceExtext without __cpp_lib_char8_t defined";
#endif
        DebugLogger() << "hair_space_str: " << hair_space_str << " valid UTF8?: "
                      << utf8::is_valid(hair_space_str.begin(), hair_space_str.end());

        try {
            auto elems = font->ExpensiveParseFromTextToTextElements(hair_space_str, format);
            auto lines = font->DetermineLines(hair_space_str, format, GG::X(1 << 15), elems);
            retval = font->TextExtent(lines);
        } catch (const std::exception& e) {
            ErrorLogger() << "Caught exception in HairSpaceExtext parsing or determining lines: " << e.what();
        }
        return retval;
    }

    std::unordered_map<std::string_view, std::string> SpeciesSuitabilityColumn1(
        const std::vector<std::string_view>& species_names)
    {
        std::unordered_map<std::string_view, std::string> retval;
        auto font = ClientUI::GetFont();

        for (const auto& species_name : species_names) {
            try {
                retval.emplace(species_name, str(
                    FlexibleFormat(UserString("ENC_SPECIES_PLANET_TYPE_SUITABILITY_COLUMN1"))
                        % LinkTaggedPresetText(VarText::SPECIES_TAG, species_name, UserString(species_name))));
            } catch (const std::exception& e) {
                ErrorLogger() << "Caught exception in SpeciesSuitabilityColumn1 when handling species " << species_name << " : " << e.what();
            }
        }

        // determine widest column, storing extents of each row for later alignment
        GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE;
        GG::X longest_width{0};
        std::unordered_map<std::string_view, GG::Pt> column1_species_extents;
        for (auto& [species_name, formatted_col1] : retval) {
            std::vector<std::shared_ptr<GG::Font::TextElement>> text_elements;
            try {
                text_elements = font->ExpensiveParseFromTextToTextElements(formatted_col1, format);
            } catch (const std::exception& e) {
                ErrorLogger() << "Caught exception in SpeciesSuitabilityColumn1 when during expensive parse: " << e.what();
                ErrorLogger() << " ... text was: " << formatted_col1;
            }
            try {
                auto lines = font->DetermineLines(formatted_col1, format, GG::X(1 << 15), text_elements);
                GG::Pt extent = font->TextExtent(lines);
                column1_species_extents[species_name] = extent;
                longest_width = std::max(longest_width, extent.x);
            } catch (const std::exception& e) {
                ErrorLogger() << "Caught exception in SpeciesSuitabilityColumn1 when  determining lines: " << e.what();
            }
        }

        try {
            // align end of column with end of longest row
            auto hair_space_width = HairSpaceExtent().x;
            auto hair_space_width_str = std::to_string(Value(hair_space_width));
#if defined(__cpp_lib_char8_t)
            std::u8string hair_space_chars{u8"\u200A"};
            std::string hair_space_str{hair_space_chars.begin(), hair_space_chars.end()};
#else
            static constexpr auto hair_space_str{u8"\u200A"};
#endif

            for (auto& it : retval) { // TODO [[]]
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
                    std::string out;
                    auto col_val = Value(column1_species_extents.at(it.first).x);
                    out.append("\n\t\t(" + std::to_string(col_val) + " + (" + std::to_string(num_spaces) + " * " + hair_space_width_str);
                    out.append(") = " + std::to_string(col_val + (num_spaces * Value(hair_space_width))) + ")");
                    auto text_elements = font->ExpensiveParseFromTextToTextElements(it.second, format);
                    auto lines = font->DetermineLines(it.second, format, GG::X(1 << 15), text_elements);
                    out.append(" = " + std::to_string(Value(font->TextExtent(lines).x)));
                    return out;
                }();
            }
        } catch (const std::exception& e) {
            ErrorLogger() << "Caught exception in SpeciesSuitabilityColumn1 when doing hair space and maybe trace stuff?: " << e.what();
        }

        return retval;
    }

    const std::vector<std::string>& PlanetEnvFilenames(PlanetType planet_type) {
        static std::unordered_map<PlanetType, std::vector<std::string>>
            filenames_by_type{{PlanetType::INVALID_PLANET_TYPE, {}}};
        std::string planet_type_str{to_string(planet_type)};
        boost::algorithm::to_lower(planet_type_str);

        if (!filenames_by_type.count(planet_type)) {
            auto pe_path = ClientUI::ArtDir() / "encyclopedia" / "planet_environments";

            auto pe_type_func = [planet_type_str, pe_path](const boost::filesystem::path& path) {
                return IsExistingFile(path)
                    && (pe_path == path.parent_path())
                    && boost::algorithm::starts_with(PathToString(path.filename()), planet_type_str);
            };

            // retain only the filenames of each path
            for (const auto& file_path : ListDir(pe_path, pe_type_func))
                filenames_by_type[planet_type].push_back(PathToString(file_path.filename()));

            for (const auto& filename : filenames_by_type[planet_type])
                DebugLogger() << "PlanetEnvFilename for " << to_string(planet_type) <<" : " << filename;
        }

        if (filenames_by_type.count(planet_type))
            return filenames_by_type.at(planet_type);
        return filenames_by_type.at(PlanetType::INVALID_PLANET_TYPE);
    }

    void RefreshDetailPanelSuitabilityTag(const std::string& item_type, const std::string& item_name,
                                          std::string& name, std::shared_ptr<GG::Texture>& texture,
                                          std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                          float& cost, std::string& cost_units, std::string& general_type,
                                          std::string& specific_type, std::string& detailed_description,
                                          GG::Clr& color)
    {
        ScopedTimer suitability_timer{"RefreshDetailPanelSuitabilityTag"};

        general_type = UserString("SP_PLANET_SUITABILITY");

        Universe& universe = GetUniverse();
        ObjectMap& objects = universe.Objects();

        int planet_id = boost::lexical_cast<int>(item_name);
        auto planet = objects.get<Planet>(planet_id); // non-const so it can be test modified to check results for various species
        if (!planet) {
            ErrorLogger() << "RefreshDetailPlanetSuitability couldn't find planet with id " << planet_id;
            return;
        }

        // show image of planet environment at the top of the suitability report
        const auto& filenames = PlanetEnvFilenames(planet->Type());
        if (!filenames.empty()) {
            auto env_img_tag = "<img src=\"encyclopedia/planet_environments/"
                               + filenames[planet_id % filenames.size()] + "\"></img>";
            TraceLogger() << "Suitability report env image tag \"" << env_img_tag << "\"";
            detailed_description.append(env_img_tag);
        }

        try {
            name = planet->PublicName(planet_id, universe);

            const auto species_names = ReportedSpeciesForPlanet(*planet);
            const auto target_population_species = SpeciesEnvByTargetPop(planet, species_names);
            const auto species_suitability_column1 = SpeciesSuitabilityColumn1(species_names);

            bool positive_header_placed = false;
            bool negative_header_placed = false;

            for (auto it = target_population_species.rbegin(); it != target_population_species.rend(); ++it) {

                const auto& [target_pop, species_env] = *it;
                const auto& [species_name, env] = species_env;

                auto species_name_column1_it = species_suitability_column1.find(species_name);
                if (species_name_column1_it == species_suitability_column1.end())
                    continue;

                if (target_pop > 0) {
                    if (!positive_header_placed) {
                        auto pos_header = str(FlexibleFormat(UserString("ENC_SUITABILITY_REPORT_POSITIVE_HEADER"))
                                              % planet->PublicName(planet_id, universe));
                        TraceLogger() << "Suitability report positive header \"" << pos_header << "\"";
                        detailed_description.append(pos_header);
                        positive_header_placed = true;
                    }

                    auto pos_row = str(FlexibleFormat(UserString("ENC_SPECIES_PLANET_TYPE_SUITABILITY"))
                        % species_name_column1_it->second
                        % UserString(to_string(env))
                        % (GG::RgbaTag(ClientUI::StatIncrColor()) + DoubleToString(target_pop, 2, true) + "</rgba>"));
                    TraceLogger() << "Suitability report positive row \"" << pos_row << "\"";
                    detailed_description.append(pos_row);

                } else if (target_pop <= 0) {
                    if (!negative_header_placed) {
                        if (positive_header_placed)
                            detailed_description += "\n\n";

                        auto neg_header = str(FlexibleFormat(UserString("ENC_SUITABILITY_REPORT_NEGATIVE_HEADER"))
                                              % planet->PublicName(planet_id, universe));
                        TraceLogger() << "Suitability report regative header \"" << neg_header << "\"";
                        detailed_description.append(neg_header);
                        negative_header_placed = true;
                    }

                    auto neg_row = str(FlexibleFormat(UserString("ENC_SPECIES_PLANET_TYPE_SUITABILITY"))
                        % species_name_column1_it->second
                        % UserString(to_string(env))
                        % (GG::RgbaTag(ClientUI::StatDecrColor()) + DoubleToString(target_pop, 2, true) + "</rgba>"));
                    TraceLogger() << "Suitability report negative row \"" << neg_row << "\"";
                    detailed_description.append(neg_row);
                }

                detailed_description += "\n";
            }
        } catch (const std::exception& e) {
            ErrorLogger() << "Caught exception generating planet suitability info: " << e.what();
        }

        if (planet->Type() < PlanetType::PT_ASTEROIDS && planet->Type() > PlanetType::INVALID_PLANET_TYPE) {
            detailed_description.append(UserString("ENC_SUITABILITY_REPORT_WHEEL_INTRO"))
                                .append("<img src=\"encyclopedia/EP_wheel.png\"></img>");
        }
    }

    void RefreshDetailPanelSearchResultsTag(std::string item_name,
                                            std::shared_ptr<GG::Texture>& texture,
                                            std::string& general_type,
                                            std::string& detailed_description)
    {
        general_type = UserString("SEARCH_RESULTS");
        texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "search.png");
        detailed_description = std::move(item_name);
    }

    void GetRefreshDetailPanelInfo(         const std::string& item_type, const std::string& item_name,
                                            std::string& name, std::shared_ptr<GG::Texture>& texture,
                                            std::shared_ptr<GG::Texture>& other_texture, int& turns,
                                            float& cost, std::string& cost_units, std::string& general_type,
                                            std::string& specific_type, std::string& detailed_description,
                                            GG::Clr& color, std::weak_ptr<const ShipDesign>& incomplete_design,
                                            bool only_description = false)
    {
        if (item_type == TextLinker::ENCYCLOPEDIA_TAG) {
            RefreshDetailPanelPediaTag(         item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color);
        }
        else if (item_type == "ENC_SHIP_PART") {
            RefreshDetailPanelShipPartTag(      item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description,
                                                color, only_description);
        }
        else if (item_type == "ENC_SHIP_HULL") {
            RefreshDetailPanelShipHullTag(      item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description,
                                                color, only_description);
        }
        else if (item_type == "ENC_TECH") {
            RefreshDetailPanelTechTag(          item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description,
                                                color, only_description);
        }
        else if (item_type == "ENC_POLICY") {
            RefreshDetailPanelPolicyTag(        item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description,
                                                color, only_description);
        }
        else if (item_type == "ENC_BUILDING_TYPE") {
            RefreshDetailPanelBuildingTypeTag(  item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description,
                                                color, only_description);
        }
        else if (item_type == "ENC_SPECIAL") {
            RefreshDetailPanelSpecialTag(       item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description,
                                                color, only_description);
        }
        else if (item_type == "ENC_EMPIRE") {
            RefreshDetailPanelEmpireTag(        item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description,
                                                color, only_description);
        }
        else if (item_type == "ENC_SPECIES") {
            RefreshDetailPanelSpeciesTag(       item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description,
                                                color, only_description);
        }
        else if (item_type == "ENC_FIELD_TYPE") {
            RefreshDetailPanelFieldTypeTag(     item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description,
                                                color, only_description);
        }
        else if (item_type == "ENC_METER_TYPE") {
            RefreshDetailPanelMeterTypeTag(     item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description,
                                                color, only_description);

        }
        else if (item_type == "ENC_SHIP_DESIGN") {
            RefreshDetailPanelShipDesignTag(    item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description,
                                                color, only_description);
        }
        else if (item_type == INCOMPLETE_DESIGN) {
            RefreshDetailPanelIncomplDesignTag( item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description, color,
                                                incomplete_design, only_description);
        }
        else if (item_type == UNIVERSE_OBJECT         || item_type == "ENC_BUILDING"  ||
                 item_type == "ENC_FIELD"             || item_type == "ENC_FLEET"     ||
                 item_type == "ENC_PLANET"            || item_type == "ENC_SHIP"      ||
                 item_type == "ENC_SYSTEM")
        {
            RefreshDetailPanelObjectTag(        item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description,
                                                color, only_description);
        }
        else if (item_type == PLANET_SUITABILITY_REPORT) {
            RefreshDetailPanelSuitabilityTag(   item_type, item_name,
                                                name, texture, other_texture, turns, cost, cost_units,
                                                general_type, specific_type, detailed_description,
                                                color);
        }
        else if (item_type == TEXT_SEARCH_RESULTS) {
            RefreshDetailPanelSearchResultsTag(item_name, texture, general_type, detailed_description);
        }
        else if (item_type == TextLinker::GRAPH_TAG) {
            // should be handled externally...
        }
    }

    std::set<std::string> ExtractWords(const std::string& search_text) { // TODO: return vector<string_view> ?
        std::set<std::string> words_in_search_text;
        for (const auto& word_range : GG::GUI::GetGUI()->FindWordsStringIndices(search_text)) {
            if (word_range.first == word_range.second)
                continue;
            std::string word(search_text.begin() + Value(word_range.first), search_text.begin() + Value(word_range.second));
            if (word.empty())
                continue;
            words_in_search_text.insert(std::move(word));
        }
        return words_in_search_text;
    }

    void SearchPediaArticleForWords(        std::string article_key,
                                            std::string article_directory,
                                            std::pair<std::string, std::string> article_name_link,
                                            std::pair<std::string, std::string>& exact_match,
                                            std::pair<std::string, std::string>& word_match,
                                            std::pair<std::string, std::string>& partial_match,
                                            std::pair<std::string, std::string>& article_match,
                                            const std::string& search_text,
                                            const std::set<std::string>& words_in_search_text,
                                            std::size_t idx,
                                            bool search_article_text)
    {
        //std::cout << "start scanning article " << idx << ": " << article_name_link.first << std::endl;
        std::string article_name = boost::locale::to_lower(article_name_link.first, GetLocale("en_US.UTF-8"));
        // search for exact title matches
        if (article_name == search_text) {
            exact_match = std::move(article_name_link);
            return;
        }

        // search for full word matches in title
        auto title_words{ExtractWords(article_name)};
        for (const auto& title_word : title_words) {
            if (words_in_search_text.count(title_word)) {
                word_match = std::move(article_name_link);
                return;
            }
        }

        // search for partial word matches: searched-for words that appear
        // in the title text, not necessarily as a complete word
        for (const std::string& word : words_in_search_text) {
            // reject searches in text for words less than 3 characters
            if (word.size() < 3)
                continue;
            if (boost::contains(article_name, word)) {
                partial_match = std::move(article_name_link);
                return;
            }
        }

        if (!search_article_text)
            return;


        // search for matches within article text
        const auto& article_entry = GetEncyclopedia().GetArticleByCategoryAndKey(article_directory, article_key);
        if (!article_entry.description.empty()) {
            // article present in pedia directly
            const auto& article_text{UserString(article_entry.description)};
            std::string article_text_lower = boost::locale::to_lower(article_text, GetLocale("en_US.UTF-8"));
            if (boost::contains(article_text_lower, search_text))
                article_match = std::move(article_name_link);
            return;
        }


        // article not in pedia. may be generated by GetRefreshDetailPanelInfo

        // most of this disregarded in this case, but needs to be passed in...
        std::shared_ptr<GG::Texture> dummy1, dummy2;
        int dummyA;
        float dummyB;
        std::string dummy3, dummy4, dummy5, dummy6;
        std::string detailed_description;
        detailed_description.reserve(2000); // guessitmate
        GG::Clr dummyC;
        std::weak_ptr<const ShipDesign> dummyD;

        //std::cout << "cat: " << article_category << "  key: " << article_key << "\n";
        GetRefreshDetailPanelInfo(article_directory, article_key,
                                  dummy3, dummy1, dummy2, dummyA, dummyB, dummy4,
                                  dummy5, dummy6, detailed_description, dummyC,
                                  dummyD, true);
        if (boost::contains(detailed_description, search_text)) {
            article_match = std::move(article_name_link);
            return;
        }
        std::string desc_lower = boost::locale::to_lower(detailed_description, GetLocale("en_US.UTF-8"));
        if (boost::contains(desc_lower, search_text)) {
            article_match = std::move(article_name_link);
            return;
        }
    }
}

void EncyclopediaDetailPanel::HandleSearchTextEntered() {
    SectionedScopedTimer timer("HandleSearchTextEntered");
    timer.EnterSection("Find words in search text");

    const unsigned int num_threads = static_cast<unsigned int>(std::max(1, EffectsProcessingThreads()));
    boost::asio::thread_pool thread_pool(num_threads);

    // search lists of articles for typed text
    auto search_text = boost::algorithm::to_lower_copy(m_search_edit->Text());
    if (search_text.empty())
        return;

    // find distinct words in search text
    std::set<std::string> words_in_search_text = ExtractWords(search_text);
    if (words_in_search_text.empty())
        return;


    // search through all articles for full or partial matches to search query
    timer.EnterSection("get subdirs");
    auto pedia_entries = GetSubDirs("ENC_INDEX", false, 0);

    std::vector<std::pair<std::string, std::string>> exact_match_report;
    std::vector<std::pair<std::string, std::string>> word_match_report;
    std::vector<std::pair<std::string, std::string>> partial_match_report;
    std::vector<std::pair<std::string, std::string>> article_match_report;

    exact_match_report.resize(pedia_entries.size());   // each entry will be sorted into just one of these, but empties will be later ignored
    word_match_report.resize(pedia_entries.size());
    partial_match_report.resize(pedia_entries.size());
    article_match_report.resize(pedia_entries.size());

    bool search_desc = GetOptionsDB().Get<bool>("ui.pedia.search.articles.enabled");

    timer.EnterSection("search subdirs dispatch");
    // assemble link text to all pedia entries, indexed by name
    std::size_t idx = -1;
    for (auto& [article_key_directory, article_name_link] : pedia_entries) {
        idx++;
        auto& emr{exact_match_report[idx]};
        auto& wmr{word_match_report[idx]};
        auto& pmr{partial_match_report[idx]};
        auto& amr{article_match_report[idx]};

        boost::asio::post(
            thread_pool, [
                article_key{article_key_directory.first},
                article_dir{article_key_directory.second},
                article_name_link{std::move(article_name_link)},
                &emr, &wmr, &pmr, &amr,
                &search_text,
                &words_in_search_text,
                idx,
                search_desc
            ]() mutable {
                SearchPediaArticleForWords(article_key,
                                           article_dir,
                                           std::move(article_name_link),
                                           emr, wmr, pmr, amr,
                                           search_text,
                                           words_in_search_text,
                                           idx,
                                           search_desc);
            });
    }

    timer.EnterSection("search subdirs eval waiting");
    thread_pool.join();


    timer.EnterSection("sort");
    // sort results...
    std::sort(exact_match_report.begin(), exact_match_report.end());
    std::sort(word_match_report.begin(), word_match_report.end());
    std::sort(partial_match_report.begin(), partial_match_report.end());
    std::sort(article_match_report.begin(), article_match_report.end());


    timer.EnterSection("assemble report");
    // compile list of articles into some dynamically generated search report text
    std::string match_report;
    if (!exact_match_report.empty()) {
        match_report += "\n" + UserString("ENC_SEARCH_EXACT_MATCHES") + "\n\n";
        for (auto&& match : exact_match_report) {
            if (!match.second.empty())
                match_report += match.second;
        }
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

    auto duration = timer.Elapsed();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    match_report += "\n\n" + boost::io::str(FlexibleFormat(UserString("ENC_SEARCH_TOOK"))
                                            % search_text % (std::to_string(duration_ms) + " ms"));

    AddItem(TEXT_SEARCH_RESULTS, std::move(match_report));
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
    detailed_description.reserve(2000); // guesstimate
    GG::Clr color(GG::CLR_ZERO);

    if (m_items.empty())
        return;

    GetRefreshDetailPanelInfo(m_items_it->first, m_items_it->second,
                              name, texture, other_texture, turns, cost, cost_units,
                              general_type, specific_type, detailed_description, color,
                              m_incomplete_design);

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
                line_data_pts.reserve(empire_linemap.second.size());
                for (const auto& entry : empire_linemap.second)
                    line_data_pts.emplace_back(entry.first, entry.second);

                m_graph->AddSeries(std::move(line_data_pts), empire_clr);
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
        m_icon = GG::Wnd::Create<GG::StaticGraphic>(
            std::move(texture), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        if (color != GG::CLR_ZERO)
            m_icon->SetColor(color);
    }

    if (m_icon) {
        m_icon->Show();
        AttachChild(m_icon);
    }

    // Set Text
    if (!name.empty())
        m_name_text->SetText(std::move(name));

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
    } else if (cost != 0.0) {
        m_cost_text->SetText(str(FlexibleFormat(UserString("ENC_COST_STR"))
            % DoubleToString(cost, 3, false)
            % cost_units));
    }

    if (!detailed_description.empty())
        m_description_rich_text->SetText(std::move(detailed_description));

    m_scroll_panel->ScrollTo(GG::Y0);
}

void EncyclopediaDetailPanel::AddItem(std::string_view type, std::string name) {
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

    m_items.emplace_back(type, std::move(name));
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

void EncyclopediaDetailPanel::SetPolicy(const std::string& policy_name) {
    if (m_items_it != m_items.end() && policy_name == m_items_it->second)
        return;
    AddItem("ENC_POLICY", policy_name);
}

void EncyclopediaDetailPanel::SetShipPart(const std::string& part_name) {
    if (m_items_it != m_items.end() && part_name == m_items_it->second)
        return;
    AddItem("ENC_SHIP_PART", part_name);
}

void EncyclopediaDetailPanel::SetShipHull(const std::string& hull_name) {
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

void EncyclopediaDetailPanel::SetMeterType(std::string meter_string) {
    if (!meter_string.empty())
        AddItem("ENC_METER_TYPE", std::move(meter_string));
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
    if (m_items_it != m_items.end() && m_items_it->first == "ENC_SHIP_DESIGN") {
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
    m_incomplete_design = std::move(incomplete_design);

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

void EncyclopediaDetailPanel::SetItem(const Policy* policy)
{ SetPolicy(policy ? policy->Name() : EMPTY_STRING); }

void EncyclopediaDetailPanel::SetItem(const ShipPart* part)
{ SetShipPart(part ? part->Name() : EMPTY_STRING); }

void EncyclopediaDetailPanel::SetItem(const ShipHull* ship_hull)
{ SetShipHull(ship_hull ? ship_hull->Name() : EMPTY_STRING); }

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
{ SetMeterType(std::string{to_string(meter_type)}); }

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

    if (event.Type() != GG::WndEvent::EventType::KeyPress)
        return false;

    this->HandleEvent(event);
    return true;
}
