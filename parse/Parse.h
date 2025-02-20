#ifndef _Parse_h_
#define _Parse_h_

#if FREEORION_BUILD_PARSE && __GNUC__
#   define FO_PARSE_API __attribute__((__visibility__("default")))
#else
#   define FO_PARSE_API
#endif

#include "../util/boost_fix.h"
#include <boost/filesystem/path.hpp>
#include <boost/uuid/uuid.hpp>

#include <array>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include <unordered_map>

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
struct GameRule;
struct UnlockableItem;
class Policy;

class PythonParser;

namespace ValueRef {
    struct ValueRefBase;
    template <typename T>
    struct ValueRef;
}

namespace parse {
    FO_PARSE_API std::map<std::string, std::unique_ptr<BuildingType>, std::less<>> buildings(const PythonParser& parser, const boost::filesystem::path& path, bool& success);
    FO_PARSE_API std::map<std::string, std::unique_ptr<FieldType>, std::less<>> fields(const boost::filesystem::path& path);
    FO_PARSE_API std::map<std::string, std::unique_ptr<ValueRef::ValueRefBase>, std::less<>> named_value_refs(const boost::filesystem::path& path);
    FO_PARSE_API std::map<std::string, std::unique_ptr<Special>, std::less<>> specials(const boost::filesystem::path& path);

    /* P in policies<P> should be std::vector<Policy>. This avoids having to include
       the government header here. */
    template <typename P>
    FO_PARSE_API P policies(const boost::filesystem::path& path);

    /** Parse all species in directory \p path, store them with their name in \p
        species_by_name. If a file exists called SpeciesCensusOrdering.focs.txt, parse it and
        store the census order in \p ordering. */
    using species_type = std::pair<
        std::map<std::string, Species>, // species_by_name,
        std::vector<std::string> // ordering
    >;
    FO_PARSE_API species_type species(const PythonParser& parser, const boost::filesystem::path& path, bool& success);

    /* T in techs<T> can only be TechManager::TechParseTuple.  This decouples
       Parse.h from Tech.h so that all parsers are not recompiled when Tech.h changes.*/
    template <typename T>
    FO_PARSE_API T techs(const PythonParser& parser, const boost::filesystem::path& path, bool& success);

    FO_PARSE_API std::vector<UnlockableItem> items(const boost::filesystem::path& path);
    FO_PARSE_API std::vector<UnlockableItem> starting_buildings(const boost::filesystem::path& path);
    FO_PARSE_API std::map<std::string, std::unique_ptr<ShipPart>, std::less<>> ship_parts(const boost::filesystem::path& path);
    FO_PARSE_API std::map<std::string, std::unique_ptr<ShipHull>, std::less<>> ship_hulls(const boost::filesystem::path& path);

    /** Parse all ship designs in directory \p path, store them with their filename in \p
        design_and_path. If a file exists called ShipDesignOrdering.focs.txt, parse it and
        store the order in \p ordering. */
    using ship_designs_type = std::pair<
        std::vector<std::pair<std::unique_ptr<ParsedShipDesign>, boost::filesystem::path>>, // designs_and_paths,
        std::vector<boost::uuids::uuid> // ordering
        >;
    FO_PARSE_API ship_designs_type ship_designs(const boost::filesystem::path& path);

    FO_PARSE_API std::vector<std::unique_ptr<FleetPlan>> fleet_plans(const boost::filesystem::path& path);
    FO_PARSE_API std::vector<std::unique_ptr<MonsterFleetPlan>> monster_fleet_plans(const boost::filesystem::path& path);
    FO_PARSE_API std::map<std::string, std::unique_ptr<ValueRef::ValueRef<double>>> statistics(const PythonParser& parser, const boost::filesystem::path& path, bool& success);
    FO_PARSE_API std::map<std::string, std::vector<EncyclopediaArticle>, std::less<>> encyclopedia_articles(const boost::filesystem::path& path);
    FO_PARSE_API std::unordered_map<std::string, GameRule> game_rules(const PythonParser& parser, const boost::filesystem::path& path, bool& success);

    FO_PARSE_API void file_substitution(std::string& text, const boost::filesystem::path& file_search_path, const std::string& filename);
    FO_PARSE_API void process_include_substitutions(std::string& text,
                                                    const boost::filesystem::path& file_search_path,
                                                    std::set<boost::filesystem::path>& files_included);

    FO_PARSE_API bool int_free_variable(std::string& text);
    FO_PARSE_API bool double_free_variable(std::string& text);
    FO_PARSE_API bool string_free_variable(std::string& text);
}

#endif
