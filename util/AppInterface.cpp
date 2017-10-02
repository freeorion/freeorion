#include "AppInterface.h"

#include "../parse/Parse.h"
#include "../universe/Building.h"
#include "../universe/Encyclopedia.h"
#include "../universe/Field.h"
#include "../universe/Special.h"
#include "../universe/Species.h"
#include "../universe/Tech.h"
#include "../util/Directories.h"
#include "../util/MultiplayerCommon.h"

#include <boost/filesystem.hpp>

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

namespace {
    template <typename ParserFunc>
    auto StartParsing(const ParserFunc& parser, const boost::filesystem::path& subdir)
        -> std::future<decltype(parser(subdir))>
    {
        return std::async(std::launch::async, parser, subdir);
    }
}

void IApp::ParseUniverseObjectTypes() {
    const auto& rdir = GetResourceDir();
    GetBuildingTypeManager().SetBuildingTypes(StartParsing(parse::buildings, rdir / "scripting/buildings"));
    GetEncyclopedia().SetArticles(StartParsing(parse::encyclopedia_articles, rdir / "scripting/encyclopedia"));
    GetFieldTypeManager().SetFieldTypes(StartParsing(parse::fields, rdir / "scripting/fields"));
    GetSpecialsManager().SetSpecialsTypes(StartParsing(parse::specials, rdir / "scripting/specials"));
    GetSpeciesManager().SetSpeciesTypes(StartParsing(parse::species, rdir / "scripting/species"));
    GetPartTypeManager().SetPartTypes(StartParsing(parse::ship_parts, rdir / "scripting/ship_parts"));
    GetHullTypeManager().SetHullTypes(StartParsing(parse::ship_hulls, rdir / "scripting/ship_hulls"));
    GetPredefinedShipDesignManager().SetShipDesignTypes(StartParsing(parse::ship_designs, rdir / "scripting/ship_designs"));
    GetPredefinedShipDesignManager().SetMonsterDesignTypes(StartParsing(parse::ship_designs, rdir / "scripting/monster_designs"));
    GetGameRules().Add(StartParsing(parse::game_rules, rdir / "scripting/game_rules.focs.txt"));
    GetTechManager().SetTechs(StartParsing(parse::techs<TechManager::TechParseTuple>, rdir / "scripting/techs"));
}
