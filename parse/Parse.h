#ifndef _Parse_h_
#define _Parse_h_

#include "../util/Export.h"

#include <boost/filesystem/path.hpp>
#include <boost/uuid/uuid.hpp>

#include <map>
#include <set>
#include <vector>

class BuildingType;
class FieldType;
class FleetPlan;
class ShipHull;
class MonsterFleetPlan;
class ShipPart;
struct ParsedShipDesign;
class Special;
class Species;
struct EncyclopediaArticle;
class GameRules;
struct UnlockableItem;
class Policy;

namespace ValueRef {
    template <typename T>
    struct ValueRef;
}

namespace parse {
    FO_COMMON_API std::map<std::string, std::unique_ptr<BuildingType>> buildings(const boost::filesystem::path& path);
    FO_COMMON_API std::map<std::string, std::unique_ptr<FieldType>> fields(const boost::filesystem::path& path);
    FO_COMMON_API std::map<std::string, std::unique_ptr<Special>> specials(const boost::filesystem::path& path);

    FO_COMMON_API std::map<std::string, std::unique_ptr<Policy>> policies(const boost::filesystem::path& path);

    /** Parse all species in directory \p path, store them with their name in \p
        species_by_name. If a file exists called SpeciesCensusOrdering.focs.txt, parse it and
        store the census order in \p ordering. */
    using species_type = std::pair<
        std::map<std::string, std::unique_ptr<Species>>, // species_by_name,
        std::vector<std::string> // ordering
        >;
    FO_COMMON_API species_type species(const boost::filesystem::path& path);

    /* T in techs<T> can only be TechManager::TechParseTuple.  This decouples
       Parse.h from Tech.h so that all parsers are not recompiled when Tech.h changes.*/
    template <typename T>
    FO_COMMON_API T techs(const boost::filesystem::path& path);

    FO_COMMON_API std::vector<UnlockableItem> items(const boost::filesystem::path& path);
    FO_COMMON_API std::vector<UnlockableItem> starting_buildings(const boost::filesystem::path& path);
    FO_COMMON_API std::map<std::string, std::unique_ptr<ShipPart>> ship_parts(const boost::filesystem::path& path);
    FO_COMMON_API std::map<std::string, std::unique_ptr<ShipHull>> ship_hulls(const boost::filesystem::path& path);

    /** Parse all ship designs in directory \p path, store them with their filename in \p
        design_and_path. If a file exists called ShipDesignOrdering.focs.txt, parse it and
        store the order in \p ordering. */
    using ship_designs_type = std::pair<
        std::vector<std::pair<std::unique_ptr<ParsedShipDesign>, boost::filesystem::path>>, // designs_and_paths,
        std::vector<boost::uuids::uuid> // ordering
        >;
    FO_COMMON_API ship_designs_type ship_designs(const boost::filesystem::path& path);

    FO_COMMON_API std::vector<std::unique_ptr<FleetPlan>> fleet_plans(const boost::filesystem::path& path);
    FO_COMMON_API std::vector<std::unique_ptr<MonsterFleetPlan>> monster_fleet_plans(const boost::filesystem::path& path);
    FO_COMMON_API std::map<std::string, std::unique_ptr<ValueRef::ValueRef<double>>> statistics(const boost::filesystem::path& path);
    FO_COMMON_API std::map<std::string, std::vector<EncyclopediaArticle>> encyclopedia_articles(const boost::filesystem::path& path);
    FO_COMMON_API GameRules game_rules(const boost::filesystem::path& path);
    FO_COMMON_API bool read_file(const boost::filesystem::path& path, std::string& file_contents);

    FO_COMMON_API void file_substitution(std::string& text, const boost::filesystem::path& file_search_path);
    FO_COMMON_API void process_include_substitutions(std::string& text,
                                                    const boost::filesystem::path& file_search_path,
                                                    std::set<boost::filesystem::path>& files_included);

    FO_COMMON_API bool int_free_variable(std::string& text);
    FO_COMMON_API bool double_free_variable(std::string& text);
    FO_COMMON_API bool string_free_variable(std::string& text);
}

#endif
