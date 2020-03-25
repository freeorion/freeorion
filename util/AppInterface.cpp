#include "AppInterface.h"

#include "../parse/Parse.h"
#include "../Empire/EmpireManager.h"
#include "../universe/Building.h"
#include "../universe/Encyclopedia.h"
#include "../universe/Field.h"
#include "../universe/Special.h"
#include "../universe/Species.h"
#include "../universe/Tech.h"
#include "../util/Directories.h"
#include "../util/GameRules.h"
#include "../util/Pending.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

#include <future>

extern template TechManager::TechParseTuple parse::techs<TechManager::TechParseTuple>(const boost::filesystem::path& path);

const int INVALID_GAME_TURN = -(2 << 15) + 1;
const int BEFORE_FIRST_TURN = -(2 << 14);
const int IMPOSSIBLY_LARGE_TURN = 2 << 15;

IApp*  IApp::s_app = nullptr;

IApp::IApp() {
    if (s_app)
        throw std::runtime_error("Attempted to construct a second instance of Application");

    s_app = this;
}

IApp::~IApp()
{ s_app = nullptr; }

IApp* IApp::GetApp()
{ return s_app; }


int IApp::MAX_AI_PLAYERS() {
    // This is not just a constant to avoid the static initialization
    // order fiasco, because it is used in more than one compilation
    // unit during static initialization, albeit a the moment in two
    // different threads.
    static const int max_number_AIs = 40;
    return max_number_AIs;
}

void IApp::StartBackgroundParsing() {
    namespace fs = boost::filesystem;

    const auto& rdir = GetResourceDir();
    if (!fs::exists(rdir) || !fs::is_directory(rdir)) {
        ErrorLogger() << "Background parse given non-existant resources directory!";
        return;
    }

    auto parse_path = rdir / "scripting/buildings";
    if (fs::exists(parse_path))
        GetBuildingTypeManager().SetBuildingTypes(Pending::StartParsing(parse::buildings, parse_path));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << parse_path.string();

    parse_path = rdir / "scripting/buildings";
    if (fs::exists(parse_path))
        GetEncyclopedia().SetArticles(Pending::StartParsing(parse::encyclopedia_articles, parse_path));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << parse_path.string();

    parse_path = rdir / "scripting/fields";
    if (fs::exists(parse_path))
        GetFieldTypeManager().SetFieldTypes(Pending::StartParsing(parse::fields, parse_path));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << parse_path.string();

    parse_path = rdir / "scripting/specials";
    if (fs::exists(parse_path))
        GetSpecialsManager().SetSpecialsTypes(Pending::StartParsing(parse::specials, parse_path));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << parse_path.string();

    parse_path = rdir / "scripting/species";
    if (fs::exists(parse_path))
        GetSpeciesManager().SetSpeciesTypes(Pending::StartParsing(parse::species, parse_path));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << parse_path.string();

    parse_path = rdir / "scripting/ship_parts";
    if (fs::exists(parse_path))
        GetPartTypeManager().SetPartTypes(Pending::StartParsing(parse::ship_parts, parse_path));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << parse_path.string();

    parse_path = rdir / "scripting/ship_hulls";
    if (fs::exists(parse_path))
        GetHullTypeManager().SetHullTypes(Pending::StartParsing(parse::ship_hulls, parse_path));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << parse_path.string();

    parse_path = rdir / "scripting/ship_designs";
    if (fs::exists(parse_path))
        GetPredefinedShipDesignManager().SetShipDesignTypes(Pending::StartParsing(parse::ship_designs, parse_path));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << parse_path.string();

    parse_path = rdir / "scripting/monster_designs";
    if (fs::exists(parse_path))
        GetPredefinedShipDesignManager().SetMonsterDesignTypes(Pending::StartParsing(parse::ship_designs, parse_path));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << parse_path.string();

    parse_path = rdir / "scripting/game_rules.focs.txt";
    if (fs::exists(parse_path))
        GetGameRules().Add(Pending::StartParsing(parse::game_rules, parse_path));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << parse_path.string();

    parse_path = rdir / "scripting/techs";
    if (fs::exists(parse_path))
        GetTechManager().SetTechs(Pending::StartParsing(parse::techs<TechManager::TechParseTuple>, parse_path));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << parse_path.string();

    InitEmpireColors(rdir / "empire_colors.xml");
}
