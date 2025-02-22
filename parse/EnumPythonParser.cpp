#include "EnumPythonParser.h"

#include <boost/python/dict.hpp>

#include "../universe/BuildingType.h"
#include "../universe/Enums.h"
#include "../universe/Planet.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/ValueRef.h"
#include "../Empire/ResourcePool.h"
#include "../Empire/ProductionQueue.h"

void RegisterGlobalsEnums(boost::python::dict& globals) {
    for (const auto& op : std::initializer_list<std::pair<const char*, ResourceType>>{
        {"ResourceInfluence", ResourceType::RE_INFLUENCE},
        {"ResourceIndustry",  ResourceType::RE_INDUSTRY}})
    {
        globals[op.first] = enum_wrapper<ResourceType>(op.second);
    }

    for (const auto& op : std::initializer_list<std::pair<const char*, EmpireAffiliationType>>{
            {"TheEmpire", EmpireAffiliationType::AFFIL_SELF},
            {"EnemyOf",   EmpireAffiliationType::AFFIL_ENEMY},
            {"PeaceWith", EmpireAffiliationType::AFFIL_PEACE},
            {"AllyOf",    EmpireAffiliationType::AFFIL_ALLY},
            {"AnyEmpire", EmpireAffiliationType::AFFIL_ANY},
            {"None",      EmpireAffiliationType::AFFIL_NONE},
            {"CanSee",    EmpireAffiliationType::AFFIL_CAN_SEE},
            {"Human",     EmpireAffiliationType::AFFIL_HUMAN}})
    {
        globals[op.first] = enum_wrapper<EmpireAffiliationType>(op.second);
    }

// TODO more meter types
    for (const auto& op : std::initializer_list<std::pair<const char*, MeterType>>{
            {"Capacity",         MeterType::METER_CAPACITY},
            {"MaxCapacity",      MeterType::METER_MAX_CAPACITY},
            {"SecondaryStat",    MeterType::METER_SECONDARY_STAT},
            {"MaxSecondaryStat", MeterType::METER_MAX_SECONDARY_STAT}})
    {
        globals[op.first] = enum_wrapper<MeterType>(op.second);
    }

    for (const auto& op : std::initializer_list<std::pair<const char*, ::PlanetEnvironment>>{
            {"Uninhabitable", PlanetEnvironment::PE_UNINHABITABLE},
            {"Hostile",       PlanetEnvironment::PE_HOSTILE},
            {"Poor",          PlanetEnvironment::PE_POOR},
            {"Adequate",      PlanetEnvironment::PE_ADEQUATE},
            {"Good",          PlanetEnvironment::PE_GOOD}})
    {
        globals[op.first] = enum_wrapper< ::PlanetEnvironment>(op.second);
    }

    for (const auto& op : std::initializer_list<std::pair<const char*, ValueRef::StatisticType>>{
            {"CountUnique",     ValueRef::StatisticType::UNIQUE_COUNT},
            {"If",              ValueRef::StatisticType::IF},
            {"Count",           ValueRef::StatisticType::COUNT},
            {"HistogramMax",    ValueRef::StatisticType::HISTO_MAX},
            {"HistogramMin",    ValueRef::StatisticType::HISTO_MIN},
            {"HistogramSpread", ValueRef::StatisticType::HISTO_SPREAD},
            {"Sum",             ValueRef::StatisticType::SUM},
            {"Mean",            ValueRef::StatisticType::MEAN},
            {"RMS",             ValueRef::StatisticType::RMS},
            {"Mode",            ValueRef::StatisticType::MODE},
            {"Max",             ValueRef::StatisticType::MAX},
            {"Min",             ValueRef::StatisticType::MIN},
            {"Spread",          ValueRef::StatisticType::SPREAD},
            {"StDev",           ValueRef::StatisticType::STDEV},
            {"Product",         ValueRef::StatisticType::PRODUCT}})
    {
        globals[op.first] = enum_wrapper<ValueRef::StatisticType>(op.second);
    }

    for (const auto& op : std::initializer_list<std::pair<const char*, ::StarType>>{
            {"Blue",      StarType::STAR_BLUE},
            {"White",     StarType::STAR_WHITE},
            {"Yellow",    StarType::STAR_YELLOW},
            {"Orange",    StarType::STAR_ORANGE},
            {"Red",       StarType::STAR_RED},
            {"Neutron",   StarType::STAR_NEUTRON},
            {"BlackHole", StarType::STAR_BLACK},
            {"NoStar",    StarType::STAR_NONE}})
    {
        globals[op.first] = enum_wrapper< ::StarType>(op.second);
    }

    for (const auto& op : std::initializer_list<std::pair<const char*, PlanetSize>>{
            {"Tiny",      PlanetSize::SZ_TINY},
            {"Small",     PlanetSize::SZ_SMALL},
            {"Medium",    PlanetSize::SZ_MEDIUM},
            {"Large",     PlanetSize::SZ_LARGE},
            {"Huge",      PlanetSize::SZ_HUGE},
            {"Asteroids", PlanetSize::SZ_ASTEROIDS},
            {"GasGiant",  PlanetSize::SZ_GASGIANT}})
    {
        globals[op.first] = enum_wrapper<PlanetSize>(op.second);
    }

    for (const auto& op : std::initializer_list<std::pair<const char*, PlanetType>>{
            {"Swamp",         PlanetType::PT_SWAMP},
            {"Toxic",         PlanetType::PT_TOXIC},
            {"Inferno",       PlanetType::PT_INFERNO},
            {"Radiated",      PlanetType::PT_RADIATED},
            {"Barren",        PlanetType::PT_BARREN},
            {"Tundra",        PlanetType::PT_TUNDRA},
            {"Desert",        PlanetType::PT_DESERT},
            {"Terran",        PlanetType::PT_TERRAN},
            {"Ocean",         PlanetType::PT_OCEAN},
            {"AsteroidsType", PlanetType::PT_ASTEROIDS},
            {"GasGiantType",  PlanetType::PT_GASGIANT}})
    {
        globals[op.first] = enum_wrapper<PlanetType>(op.second);
    }

    for (const auto& op : std::initializer_list<std::pair<const char*, UnlockableItemType>>{
            {"UnlockBuilding",   UnlockableItemType::UIT_BUILDING},
            {"UnlockShipPart",   UnlockableItemType::UIT_SHIP_PART},
            {"UnlockShipHull",   UnlockableItemType::UIT_SHIP_HULL},
            {"UnlockShipDesign", UnlockableItemType::UIT_SHIP_DESIGN},
            {"UnlockTech",       UnlockableItemType::UIT_TECH},
            {"UnlockPolicy",     UnlockableItemType::UIT_POLICY}})
    {
        globals[op.first] = enum_wrapper<UnlockableItemType>(op.second);
    }

    for (const auto& op : std::initializer_list<std::pair<const char*, BuildType>>{
            {"BuildBuilding", BuildType::BT_BUILDING},
            {"BuildShip", BuildType::BT_SHIP}})
    {
        globals[op.first] = enum_wrapper<BuildType>(op.second);
    }

    // visibility_parser_rules
    for (const auto& vis : std::initializer_list<std::pair<const char*, Visibility>>{
            {"Invisible", Visibility::VIS_NO_VISIBILITY},
            {"Basic",     Visibility::VIS_BASIC_VISIBILITY},
            {"Partial",   Visibility::VIS_PARTIAL_VISIBILITY},
            {"Full",      Visibility::VIS_FULL_VISIBILITY}})
    {
        globals[vis.first] = enum_wrapper<Visibility>(vis.second);
    }

    // capture_result_enum_grammar
    for (const auto& capt : std::initializer_list<std::pair<const char*, CaptureResult>>{
            {"Capture",          CaptureResult::CR_CAPTURE},
            {"Retain",           CaptureResult::CR_RETAIN},
            {"DestroyOnCapture", CaptureResult::CR_DESTROY}})
    {
        globals[capt.first] = enum_wrapper<CaptureResult>(capt.second);
    }
}

