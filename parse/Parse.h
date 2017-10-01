#ifndef _Parse_h_
#define _Parse_h_

#if FREEORION_BUILD_PARSE && __GNUC__
#   define FO_PARSE_API __attribute__((__visibility__("default")))
#else
#   define FO_PARSE_API
#endif

#include "../universe/Tech.h"

#include <boost/filesystem/path.hpp>
#include <boost/uuid/uuid.hpp>

class BuildingType;
class FieldType;
class FleetPlan;
class HullType;
class MonsterFleetPlan;
class PartType;
class ShipDesign;
class Special;
class Species;
struct Encyclopedia;
class GameRules;

namespace parse {
    FO_PARSE_API void init();

    FO_PARSE_API bool buildings(std::map<std::string,
                                std::unique_ptr<BuildingType>>& building_types);

    FO_PARSE_API bool fields(std::map<std::string, FieldType*>& field_types);

    FO_PARSE_API bool specials(std::map<std::string, std::unique_ptr<Special>>& specials_);

    FO_PARSE_API bool species(std::map<std::string, Species*>& species_);

    FO_PARSE_API bool techs(TechManager::TechContainer& techs_,
                            std::map<std::string, TechCategory*>& tech_categories,
                            std::set<std::string>& categories_seen);

    FO_PARSE_API bool items(std::vector<ItemSpec>& items_);

    FO_PARSE_API bool starting_buildings(std::vector<ItemSpec>& starting_buildings_);

    FO_PARSE_API bool ship_parts(std::map<std::string, PartType*>& parts);

    FO_PARSE_API bool ship_hulls(std::map<std::string, HullType*>& hulls);

    /** Parse all ship designs in directory \p path, store them with their filename in \p
        design_and_path. If a file exists called ShipDesignOrdering.focs.txt, parse it and
        store the order in \p ordering. */
    FO_PARSE_API bool ship_designs(const boost::filesystem::path& path,
                                   std::vector<std::pair<std::unique_ptr<ShipDesign>,
                                                         boost::filesystem::path>>& designs_and_paths,
                                   std::vector<boost::uuids::uuid>& ordering);

    FO_PARSE_API bool ship_designs(std::vector<std::unique_ptr<ShipDesign>>& designs,
                                   std::vector<boost::uuids::uuid>& ordering);

    FO_PARSE_API bool monster_designs(std::vector<std::unique_ptr<ShipDesign>>& designs,
                                      std::vector<boost::uuids::uuid>& ordering);

    FO_PARSE_API bool fleet_plans(std::vector<FleetPlan*>& fleet_plans_);

    FO_PARSE_API bool monster_fleet_plans(std::vector<MonsterFleetPlan*>& monster_fleet_plans_);

    FO_PARSE_API bool statistics(std::map<std::string, ValueRef::ValueRefBase<double>*>& stats_);

    FO_PARSE_API bool encyclopedia_articles(Encyclopedia& enc);

    FO_PARSE_API bool keymaps(std::map<std::string, std::map<int, int>>& nkm);

    FO_PARSE_API bool game_rules(GameRules& rules);

    FO_PARSE_API bool read_file(const boost::filesystem::path& path, std::string& file_contents);

    /** Find all FOCS scripts (files with .focs.txt suffix) in \p path.  If \p allow_permissive =
        true then if \p path is not empty and there are no .focs.txt files allow all files to qualify.*/
    FO_PARSE_API std::vector< boost::filesystem::path > ListScripts(const boost::filesystem::path& path, bool permissive = false);

    FO_PARSE_API void file_substitution(std::string& text, const boost::filesystem::path& file_search_path);

    FO_PARSE_API void process_include_substitutions(std::string& text, 
                                                    const boost::filesystem::path& file_search_path, 
                                                    std::set<boost::filesystem::path>& files_included);
}

#endif
