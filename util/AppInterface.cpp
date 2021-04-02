#include "AppInterface.h"

#include "../parse/Parse.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Government.h"
#include "../universe/BuildingType.h"
#include "../universe/Encyclopedia.h"
#include "../universe/FieldType.h"
#include "../universe/NamedValueRefManager.h"
#include "../universe/Special.h"
#include "../universe/Species.h"
#include "../universe/ShipDesign.h"
#include "../universe/ShipPart.h"
#include "../universe/ShipHull.h"
#include "../universe/Tech.h"
#include "Directories.h"
#include "GameRules.h"
#include "Pending.h"

#include <boost/filesystem.hpp>

#include <future>

extern template TechManager::TechParseTuple parse::techs<TechManager::TechParseTuple>(const boost::filesystem::path& path);

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
    constexpr int max_number_AIs = 40;
    return max_number_AIs;
}

void IApp::StartBackgroundParsing(std::promise<void>&& barrier) {
    namespace fs = boost::filesystem;
    barrier.set_value();

    const auto& rdir = GetResourceDir();
    if (!IsExistingDir(rdir)) {
        ErrorLogger() << "Background parse given non-existant resources directory: " << rdir.string() ;
        return;
    }

    // named value ref parsing can be done in parallel as the referencing happens after parsing
    if (IsExistingDir(rdir / "scripting/common"))
        GetNamedValueRefManager().SetNamedValueRefParse(Pending::StartParsing(parse::named_value_refs, rdir / "scripting/common"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/common").string();

    if (IsExistingDir(rdir / "scripting/buildings"))
        GetBuildingTypeManager().SetBuildingTypes(Pending::StartParsing(parse::buildings, rdir / "scripting/buildings"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/buildings").string();

    if (IsExistingDir(rdir / "scripting/policies"))
        GetPolicyManager().SetPolicies(Pending::StartParsing(parse::policies, rdir / "scripting/policies"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/policies").string();

    if (IsExistingDir(rdir / "scripting/encyclopedia"))
        GetEncyclopedia().SetArticles(Pending::StartParsing(parse::encyclopedia_articles, rdir / "scripting/encyclopedia"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/encyclopedia").string();

    if (IsExistingDir(rdir / "scripting/fields"))
        GetFieldTypeManager().SetFieldTypes(Pending::StartParsing(parse::fields, rdir / "scripting/fields"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/fields").string();

    if (IsExistingDir(rdir / "scripting/specials"))
        GetSpecialsManager().SetSpecialsTypes(Pending::StartParsing(parse::specials, rdir / "scripting/specials"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/specials").string();

    if (IsExistingDir(rdir / "scripting/species"))
        GetSpeciesManager().SetSpeciesTypes(Pending::StartParsing(parse::species, rdir / "scripting/species"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/species").string();

    if (IsExistingDir(rdir / "scripting/ship_parts"))
        GetShipPartManager().SetShipParts(Pending::StartParsing(parse::ship_parts, rdir / "scripting/ship_parts"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/ship_parts").string();

    if (IsExistingDir(rdir / "scripting/ship_hulls"))
        GetShipHullManager().SetShipHulls(Pending::StartParsing(parse::ship_hulls, rdir / "scripting/ship_hulls"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/ship_hulls").string();

    if (IsExistingDir(rdir / "scripting/ship_designs"))
        GetPredefinedShipDesignManager().SetShipDesignTypes(Pending::StartParsing(parse::ship_designs, rdir / "scripting/ship_designs"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/ship_designs").string();

    if (IsExistingDir(rdir / "scripting/monster_designs"))
        GetPredefinedShipDesignManager().SetMonsterDesignTypes(Pending::StartParsing(parse::ship_designs, rdir / "scripting/monster_designs"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/monster_designs").string();

    if (IsExistingFile(rdir / "scripting/game_rules.focs.txt"))
        GetGameRules().Add(Pending::StartParsing(parse::game_rules, rdir / "scripting/game_rules.focs.txt"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/game_rules.focs.txt").string();

    if (IsExistingDir(rdir / "scripting/techs"))
        GetTechManager().SetTechs(Pending::StartParsing(parse::techs<TechManager::TechParseTuple>, rdir / "scripting/techs"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/techs").string();

    if (IsExistingFile(rdir / "scripting/empire_colors.xml"))
        InitEmpireColors(rdir / "scripting/empire_colors.xml");
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/empire_colors.xml").string();
}
