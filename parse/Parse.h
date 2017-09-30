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
struct ParsedShipDesign;
class Special;
class Species;
struct EncyclopediaArticle;
class GameRules;

namespace parse {
    FO_PARSE_API std::map<std::string, std::unique_ptr<BuildingType>> buildings(const boost::filesystem::path& path);

    FO_PARSE_API std::map<std::string, std::unique_ptr<FieldType>> fields(const boost::filesystem::path& path);

    FO_PARSE_API std::map<std::string, std::unique_ptr<Special>> specials(const boost::filesystem::path& path);

    FO_PARSE_API std::map<std::string, std::unique_ptr<Species>> species(const boost::filesystem::path& path);

    FO_PARSE_API TechManager::TechParseTuple techs(const boost::filesystem::path& path);

    FO_PARSE_API std::vector<ItemSpec> items(const boost::filesystem::path& path);

    FO_PARSE_API std::vector<ItemSpec> starting_buildings(const boost::filesystem::path& path);

    FO_PARSE_API std::map<std::string, std::unique_ptr<PartType>> ship_parts(const boost::filesystem::path& path);

    FO_PARSE_API std::map<std::string, std::unique_ptr<HullType>> ship_hulls(const boost::filesystem::path& path);

    /** Parse all ship designs in directory \p path, store them with their filename in \p
        design_and_path. If a file exists called ShipDesignOrdering.focs.txt, parse it and
        store the order in \p ordering. */
    using ship_designs_type = std::pair<
        std::vector<std::pair<std::unique_ptr<ParsedShipDesign>, boost::filesystem::path>>, // designs_and_paths,
        std::vector<boost::uuids::uuid> // ordering
        >;
    FO_PARSE_API ship_designs_type ship_designs(const boost::filesystem::path& path);

    FO_PARSE_API std::vector<FleetPlan*> fleet_plans(const boost::filesystem::path& path);

    FO_PARSE_API std::vector<MonsterFleetPlan*> monster_fleet_plans(const boost::filesystem::path& path);

    FO_PARSE_API std::map<std::string, ValueRef::ValueRefBase<double>*> statistics(const boost::filesystem::path& path);

    FO_PARSE_API std::map<std::string, std::vector<EncyclopediaArticle>> encyclopedia_articles(const boost::filesystem::path& path);

    FO_PARSE_API std::map<std::string, std::map<int, int>> keymaps(const boost::filesystem::path& path);

    FO_PARSE_API GameRules game_rules(const boost::filesystem::path& path);

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
