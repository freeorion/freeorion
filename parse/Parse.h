#ifndef _Parse_h_
#define _Parse_h_

#if FREEORION_BUILD_PARSE && __GNUC__
#   define FO_PARSE_API __attribute__((__visibility__("default")))
#else
#   define FO_PARSE_API
#endif

#include "../universe/Tech.h"

#include <boost/filesystem/path.hpp>


class Alignment;
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

namespace parse {
    FO_PARSE_API void init();

    FO_PARSE_API bool buildings(std::map<std::string,
                                std::unique_ptr<BuildingType>>& building_types);

    FO_PARSE_API bool fields(std::map<std::string, FieldType*>& field_types);

    FO_PARSE_API bool specials(std::map<std::string, Special*>& specials_);

    FO_PARSE_API bool species(std::map<std::string, Species*>& species_);

    FO_PARSE_API bool techs(TechManager::TechContainer& techs_,
                            std::map<std::string, TechCategory*>& tech_categories,
                            std::set<std::string>& categories_seen);

    FO_PARSE_API bool items(std::vector<ItemSpec>& items_);

    FO_PARSE_API bool starting_buildings(std::vector<ItemSpec>& starting_buildings_);

    FO_PARSE_API bool ship_parts(std::map<std::string, PartType*>& parts);

    FO_PARSE_API bool ship_hulls(std::map<std::string, HullType*>& hulls);

    FO_PARSE_API bool ship_designs(const boost::filesystem::path& path,
                                   std::map<std::string, ShipDesign*>& designs);

    FO_PARSE_API bool ship_designs(std::map<std::string, ShipDesign*>& designs);

    FO_PARSE_API bool monster_designs(std::map<std::string, ShipDesign*>& designs);

    FO_PARSE_API bool fleet_plans(std::vector<FleetPlan*>& fleet_plans_);

    FO_PARSE_API bool monster_fleet_plans(std::vector<MonsterFleetPlan*>& monster_fleet_plans_);

    FO_PARSE_API bool alignments(std::vector<Alignment>& alignments_,
                                 std::vector<std::shared_ptr<Effect::EffectsGroup>>& effects_groups);

    FO_PARSE_API bool statistics(std::map<std::string, ValueRef::ValueRefBase<double>*>& stats_);

    FO_PARSE_API bool encyclopedia_articles(Encyclopedia& enc);

    FO_PARSE_API bool keymaps(std::map<std::string, std::map<int, int>>& nkm);

    FO_PARSE_API bool read_file(const boost::filesystem::path& path, std::string& file_contents);

    FO_PARSE_API std::vector< boost::filesystem::path > ListScripts(const boost::filesystem::path& path);

    FO_PARSE_API void file_substitution(std::string& text, const boost::filesystem::path& file_search_path);

    FO_PARSE_API void process_include_substitutions(std::string& text, 
                                                    const boost::filesystem::path& file_search_path, 
                                                    std::set<boost::filesystem::path>& files_included);
}

#endif
