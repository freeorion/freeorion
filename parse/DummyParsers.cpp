#include "Parse.h"

#include "../util/GameRules.h"
#include "../universe/BuildingType.h"
#include "../universe/Encyclopedia.h"
#include "../universe/FieldType.h"
#include "../universe/FleetPlan.h"
#include "../Empire/Government.h"
#include "../universe/ShipDesign.h"
#include "../universe/ShipHull.h"
#include "../universe/ShipPart.h"
#include "../universe/Special.h"
#include "../universe/Species.h"
#include "../universe/Tech.h"
#include "../universe/UnlockableItem.h"
#include "../universe/ValueRef.h"
#include "PythonParser.h"

namespace parse {
    std::map<std::string, std::unique_ptr<BuildingType>, std::less<>> buildings(const PythonParser& parser, const std::filesystem::path& path, bool& success) {
        success = true;
        return {};
    }

    std::map<std::string, std::unique_ptr<FieldType>, std::less<>> fields(const std::filesystem::path& path)
    { return {}; }

    std::map<std::string, std::unique_ptr<ValueRef::ValueRefBase>, std::less<>> named_value_refs(const std::filesystem::path& path)
    { return {}; }

    std::map<std::string, std::unique_ptr<Special>, std::less<>> specials(const std::filesystem::path& path)
    { return {}; }

    template <>
    std::vector<Policy> policies(const std::filesystem::path& path)
    { return {}; }

    species_type species(const PythonParser& parser, const std::filesystem::path& path, bool& success) {
        success = true;
        return {};
    }

    template <>
    TechManager::TechParseTuple techs(const PythonParser& parser, const std::filesystem::path& path, bool& success) {
        success = true;
        return TechManager::TechParseTuple{};
    }

    std::vector<UnlockableItem> items(const std::filesystem::path& path)
    { return {}; }

    std::vector<UnlockableItem> starting_buildings(const std::filesystem::path& path)
    { return {}; }

    std::map<std::string, std::unique_ptr<ShipPart>, std::less<>> ship_parts(const std::filesystem::path& path)
    { return {}; }

    std::map<std::string, std::unique_ptr<ShipHull>, std::less<>> ship_hulls(const std::filesystem::path& path)
    { return {}; }

    ship_designs_type ship_designs(const std::filesystem::path& path)
    { return {}; }

    std::vector<std::unique_ptr<FleetPlan>> fleet_plans(const std::filesystem::path& path)
    { return {}; }

    std::vector<std::unique_ptr<MonsterFleetPlan>> monster_fleet_plans(const std::filesystem::path& path)
    { return {}; }

    std::map<std::string, std::unique_ptr<ValueRef::ValueRef<double>>> statistics(const PythonParser& parser, const std::filesystem::path& path, bool& success) {
        success = true;
        return {};
    }

    std::map<std::string, std::vector<EncyclopediaArticle>, std::less<>> encyclopedia_articles(const std::filesystem::path& path)
    { return {}; }

    GameRulesTypeMap game_rules(const PythonParser& parser, const std::filesystem::path& path, bool& success) {
        success = true;
        return {};
    }

    void file_substitution(std::string& text, const std::filesystem::path& file_search_path, const std::string& filename)
    {}

    void process_include_substitutions(std::string& text,
                                       const std::filesystem::path& file_search_path,
                                       std::set<std::filesystem::path>& files_included)
    {}

    bool int_free_variable(std::string& text) { return false; }
    bool double_free_variable(std::string& text) { return false; }
    bool string_free_variable(std::string& text) { return false; }
}

template FO_PARSE_API TechManager::TechParseTuple parse::techs<TechManager::TechParseTuple>(const PythonParser& parser, const std::filesystem::path& path, bool& success);

template FO_PARSE_API std::vector<Policy> parse::policies<std::vector<Policy>>(const std::filesystem::path& path);

PythonParser::PythonParser(PythonCommon& _python, const std::filesystem::path& scripting_dir) :
    m_python(_python),
    m_scripting_dir(scripting_dir)
{ }

PythonParser::~PythonParser() = default;
