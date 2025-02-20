#include "AppInterface.h"

#include "../parse/Parse.h"
#include "../parse/PythonParser.h"
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

#include <exception>
#include <future>
#include <stdexcept>

extern template TechManager::TechParseTuple parse::techs<TechManager::TechParseTuple>(const PythonParser& parser, const boost::filesystem::path& path, bool& success);

IApp* IApp::s_app = nullptr;

IApp::IApp() {
    if (s_app)
        throw std::runtime_error("Attempted to construct a second instance of Application");

    s_app = this;
}

IApp::~IApp()
{ s_app = nullptr; }


int IApp::MAX_AI_PLAYERS() noexcept {
    // This is not just a constant to avoid the static initialization
    // order fiasco, because it is used in more than one compilation
    // unit during static initialization, albeit a the moment in two
    // different threads.
    static constexpr int max_number_AIs = 40;
    return max_number_AIs;
}

void IApp::StartBackgroundParsing(const PythonParser& python, std::promise<void>&& barrier) {
    namespace fs = boost::filesystem;

    const auto& rdir = GetResourceDir();
    if (!IsExistingDir(rdir)) {
        ErrorLogger() << "Background parse given non-existant resources directory: " << rdir.string() ;
        barrier.set_exception(std::make_exception_ptr(std::runtime_error("non-existant resources directory")));
        return;
    }

    DebugLogger() << "Start background parsing...";

    // named value ref parsing can be done in parallel as the referencing happens after parsing
    if (IsExistingDir(rdir / "scripting/macros"))
        GetNamedValueRefManager().SetNamedValueRefParse(Pending::ParseSynchronously(parse::named_value_refs, rdir / "scripting/macros"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/macros").string();

    if (IsExistingDir(rdir / "scripting/buildings"))
        GetBuildingTypeManager().SetBuildingTypes(Pending::ParseSynchronously(parse::buildings, python, rdir / "scripting/buildings"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/buildings").string();

    if (IsExistingDir(rdir / "scripting/policies"))
        GetPolicyManager().SetPolicies(Pending::StartAsyncParsing(parse::policies<std::vector<Policy>>, rdir / "scripting/policies"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/policies").string();

    if (IsExistingDir(rdir / "scripting/encyclopedia"))
        GetEncyclopedia().SetArticles(Pending::StartAsyncParsing(parse::encyclopedia_articles, rdir / "scripting/encyclopedia"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/encyclopedia").string();

    if (IsExistingDir(rdir / "scripting/fields"))
        GetFieldTypeManager().SetFieldTypes(Pending::StartAsyncParsing(parse::fields, rdir / "scripting/fields"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/fields").string();

    if (IsExistingDir(rdir / "scripting/specials"))
        GetSpecialsManager().SetSpecialsTypes(Pending::StartAsyncParsing(parse::specials, rdir / "scripting/specials"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/specials").string();

    if (IsExistingDir(rdir / "scripting/species"))
        GetSpeciesManager().SetSpeciesTypes(Pending::ParseSynchronously(parse::species, python, rdir / "scripting/species"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/species").string();

    if (IsExistingDir(rdir / "scripting/ship_parts"))
        GetShipPartManager().SetShipParts(Pending::StartAsyncParsing(parse::ship_parts, rdir / "scripting/ship_parts"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/ship_parts").string();

    if (IsExistingDir(rdir / "scripting/ship_hulls"))
        GetShipHullManager().SetShipHulls(Pending::StartAsyncParsing(parse::ship_hulls, rdir / "scripting/ship_hulls"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/ship_hulls").string();

    if (IsExistingDir(rdir / "scripting/ship_designs"))
        GetPredefinedShipDesignManager().SetShipDesignTypes(Pending::StartAsyncParsing(parse::ship_designs, rdir / "scripting/ship_designs"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/ship_designs").string();

    if (IsExistingDir(rdir / "scripting/monster_designs"))
        GetPredefinedShipDesignManager().SetMonsterDesignTypes(Pending::StartAsyncParsing(parse::ship_designs, rdir / "scripting/monster_designs"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/monster_designs").string();

    if (IsExistingFile(rdir / "scripting/game_rules.focs.py"))
        GetGameRules().Add(Pending::ParseSynchronously(parse::game_rules, python, rdir / "scripting/game_rules.focs.py"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/game_rules.focs.py").string();

    if (IsExistingDir(rdir / "scripting/techs"))
        GetTechManager().SetTechs(Pending::ParseSynchronously(parse::techs<TechManager::TechParseTuple>, python, rdir / "scripting/techs", std::move(barrier)));
    else {
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/techs").string();
        barrier.set_value();
    }

    if (IsExistingFile(rdir / "scripting/empire_colors.xml"))
        InitEmpireColors(rdir / "scripting/empire_colors.xml");
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/empire_colors.xml").string();
}
