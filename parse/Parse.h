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
struct EncyclopediaArticle;
class GameRules;

namespace parse {
    FO_PARSE_API void init();

    FO_PARSE_API std::map<std::string, std::unique_ptr<BuildingType>> buildings();

    FO_PARSE_API std::map<std::string, std::unique_ptr<FieldType>> fields();

    FO_PARSE_API std::map<std::string, std::unique_ptr<Special>> specials();

    FO_PARSE_API std::map<std::string, std::unique_ptr<Species>> species();

    FO_PARSE_API std::tuple<
        TechManager::TechContainer, // techs_
        std::map<std::string, std::unique_ptr<TechCategory>>, // tech_categories,
        std::set<std::string> // categories_seen
        > techs();

    FO_PARSE_API std::vector<ItemSpec> items();

    FO_PARSE_API std::vector<ItemSpec> starting_buildings();

    FO_PARSE_API std::map<std::string, std::unique_ptr<PartType>> ship_parts();

    FO_PARSE_API std::map<std::string, std::unique_ptr<HullType>> ship_hulls();

    /** Parse all ship designs in directory \p path, store them with their filename in \p
        design_and_path. If a file exists called ShipDesignOrdering.focs.txt, parse it and
        store the order in \p ordering. */
    FO_PARSE_API std::pair<
        std::vector<std::pair<std::unique_ptr<ShipDesign>, boost::filesystem::path>>, // designs_and_paths,
        std::vector<boost::uuids::uuid> // ordering
        > ship_designs(const boost::filesystem::path& path);

    FO_PARSE_API std::pair<
        std::vector<std::unique_ptr<ShipDesign>>, // designs
        std::vector<boost::uuids::uuid> // ordering
        > ship_designs();

    FO_PARSE_API std::pair<
        std::vector<std::unique_ptr<ShipDesign>>, // designs
        std::vector<boost::uuids::uuid> // ordering
        > monster_designs();

    FO_PARSE_API std::vector<FleetPlan*> fleet_plans();

    FO_PARSE_API std::vector<MonsterFleetPlan*> monster_fleet_plans();

    FO_PARSE_API std::map<std::string, ValueRef::ValueRefBase<double>*> statistics();

    FO_PARSE_API std::map<std::string, std::vector<EncyclopediaArticle>> encyclopedia_articles();

    FO_PARSE_API std::map<std::string, std::map<int, int>> keymaps();

    FO_PARSE_API bool game_rules(GameRules& game_rules);

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
