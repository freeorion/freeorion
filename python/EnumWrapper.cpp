#include "../universe/BuildingType.h"
#include "../universe/Condition.h"
#include "../universe/Effect.h"
#include "../universe/Enums.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/ShipHull.h"
#include "../universe/ShipPart.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/ValueRef.h"
#include "../universe/UniverseObject.h"
#include "../universe/UnlockableItem.h"
#include "../Empire/Diplomacy.h"
#include "../Empire/Empire.h"
#include "../util/MultiplayerCommon.h"
#include "../util/GameRules.h"

#include <boost/python.hpp>

namespace FreeOrionPython {
    void WrapGameStateEnums() {
        namespace py = boost::python;

        py::enum_<StarType>("starType")
            .value("blue",      StarType::STAR_BLUE)
            .value("white",     StarType::STAR_WHITE)
            .value("yellow",    StarType::STAR_YELLOW)
            .value("orange",    StarType::STAR_ORANGE)
            .value("red",       StarType::STAR_RED)
            .value("neutron",   StarType::STAR_NEUTRON)
            .value("blackHole", StarType::STAR_BLACK)
            .value("noStar",    StarType::STAR_NONE)
            .value("unknown",   StarType::INVALID_STAR_TYPE)
        ;
        py::enum_<Visibility>("visibility")
            .value("invalid",   Visibility::INVALID_VISIBILITY)
            .value("none",      Visibility::VIS_NO_VISIBILITY)
            .value("basic",     Visibility::VIS_BASIC_VISIBILITY)
            .value("partial",   Visibility::VIS_PARTIAL_VISIBILITY)
            .value("full",      Visibility::VIS_FULL_VISIBILITY)
        ;
        py::enum_<PlanetSize>("planetSize")
            .value("tiny",      PlanetSize::SZ_TINY)
            .value("small",     PlanetSize::SZ_SMALL)
            .value("medium",    PlanetSize::SZ_MEDIUM)
            .value("large",     PlanetSize::SZ_LARGE)
            .value("huge",      PlanetSize::SZ_HUGE)
            .value("asteroids", PlanetSize::SZ_ASTEROIDS)
            .value("gasGiant",  PlanetSize::SZ_GASGIANT)
            .value("noWorld",   PlanetSize::SZ_NOWORLD)
            .value("unknown",   PlanetSize::INVALID_PLANET_SIZE)
        ;
        py::enum_<PlanetType>("planetType")
            .value("swamp",     PlanetType::PT_SWAMP)
            .value("radiated",  PlanetType::PT_RADIATED)
            .value("toxic",     PlanetType::PT_TOXIC)
            .value("inferno",   PlanetType::PT_INFERNO)
            .value("barren",    PlanetType::PT_BARREN)
            .value("tundra",    PlanetType::PT_TUNDRA)
            .value("desert",    PlanetType::PT_DESERT)
            .value("terran",    PlanetType::PT_TERRAN)
            .value("ocean",     PlanetType::PT_OCEAN)
            .value("asteroids", PlanetType::PT_ASTEROIDS)
            .value("gasGiant",  PlanetType::PT_GASGIANT)
            .value("unknown",   PlanetType::INVALID_PLANET_TYPE)
        ;
        py::enum_<PlanetEnvironment>("planetEnvironment")
            .value("uninhabitable", PlanetEnvironment::PE_UNINHABITABLE)
            .value("hostile",       PlanetEnvironment::PE_HOSTILE)
            .value("poor",          PlanetEnvironment::PE_POOR)
            .value("adequate",      PlanetEnvironment::PE_ADEQUATE)
            .value("good",          PlanetEnvironment::PE_GOOD)
        ;
        py::enum_<TechStatus>("techStatus")
            .value("unresearchable",    TechStatus::TS_UNRESEARCHABLE)
            .value("partiallyUnlocked", TechStatus::TS_HAS_RESEARCHED_PREREQ)
            .value("researchable",      TechStatus::TS_RESEARCHABLE)
            .value("complete",          TechStatus::TS_COMPLETE)
        ;
        TraceLogger() << "WrapGameStateEnums: Wrap BuildType enum";
        auto buildType = py::enum_<BuildType>("buildType");
        for (const auto& [bt, sv] : BuildTypeValues()) {
            TraceLogger() << "WrapGameStateEnums: Wrap BuildType enum " << sv;
            buildType.value(sv.data(), bt);
        }
        TraceLogger() << "WrapGameStateEnums: BuildType enum wrapped";
        py::enum_<ResourceType>("resourceType")
            .value("industry",          ResourceType::RE_INDUSTRY)
            .value("influence",         ResourceType::RE_INFLUENCE)
            .value("research",          ResourceType::RE_RESEARCH)
            .value("stockpile",         ResourceType::RE_STOCKPILE)
        ;
        py::enum_<MeterType>("meterType")
            .value("targetPopulation",  MeterType::METER_TARGET_POPULATION)
            .value("targetIndustry",    MeterType::METER_TARGET_INDUSTRY)
            .value("targetResearch",    MeterType::METER_TARGET_RESEARCH)
            .value("targetInfluence",   MeterType::METER_TARGET_INFLUENCE)
            .value("targetConstruction",MeterType::METER_TARGET_CONSTRUCTION)
            .value("targetHappiness",   MeterType::METER_TARGET_HAPPINESS)

            .value("maxCapacity",       MeterType::METER_MAX_CAPACITY)
            .value("maxSecondaryStat",  MeterType::METER_MAX_SECONDARY_STAT)

            .value("maxFuel",           MeterType::METER_MAX_FUEL)
            .value("maxShield",         MeterType::METER_MAX_SHIELD)
            .value("maxStructure",      MeterType::METER_MAX_STRUCTURE)
            .value("maxDefense",        MeterType::METER_MAX_DEFENSE)
            .value("maxSupply",         MeterType::METER_MAX_SUPPLY)
            .value("maxStockpile",      MeterType::METER_MAX_STOCKPILE)
            .value("maxTroops",         MeterType::METER_MAX_TROOPS)

            .value("population",        MeterType::METER_POPULATION)
            .value("industry",          MeterType::METER_INDUSTRY)
            .value("research",          MeterType::METER_RESEARCH)
            .value("influence",         MeterType::METER_INFLUENCE)
            .value("construction",      MeterType::METER_CONSTRUCTION)
            .value("happiness",         MeterType::METER_HAPPINESS)

            .value("capacity",          MeterType::METER_CAPACITY)
            .value("secondaryStat",     MeterType::METER_SECONDARY_STAT)

            .value("fuel",              MeterType::METER_FUEL)
            .value("shield",            MeterType::METER_SHIELD)
            .value("structure",         MeterType::METER_STRUCTURE)
            .value("defense",           MeterType::METER_DEFENSE)
            .value("supply",            MeterType::METER_SUPPLY)
            .value("stockpile",         MeterType::METER_STOCKPILE)
            .value("troops",            MeterType::METER_TROOPS)

            .value("rebels",            MeterType::METER_REBEL_TROOPS)
            .value("size",              MeterType::METER_SIZE)
            .value("stealth",           MeterType::METER_STEALTH)
            .value("detection",         MeterType::METER_DETECTION)
            .value("speed",             MeterType::METER_SPEED)
        ;
        py::enum_<FleetAggression>("fleetAggression")
            .value("passive",           FleetAggression::FLEET_PASSIVE)
            .value("defensive",         FleetAggression::FLEET_DEFENSIVE)
            .value("obstructive",       FleetAggression::FLEET_OBSTRUCTIVE)
            .value("aggressive",        FleetAggression::FLEET_AGGRESSIVE)
        ;
        py::enum_<DiplomaticStatus>("diplomaticStatus")
            .value("war",               DiplomaticStatus::DIPLO_WAR)
            .value("peace",             DiplomaticStatus::DIPLO_PEACE)
            .value("allied",            DiplomaticStatus::DIPLO_ALLIED)
        ;
        py::enum_<DiplomaticMessage::Type>("diplomaticMessageType")
            .value("noMessage",             DiplomaticMessage::Type::INVALID)
            .value("warDeclaration",        DiplomaticMessage::Type::WAR_DECLARATION)
            .value("peaceProposal",         DiplomaticMessage::Type::PEACE_PROPOSAL)
            .value("acceptPeaceProposal",   DiplomaticMessage::Type::ACCEPT_PEACE_PROPOSAL)
            .value("alliesProposal",        DiplomaticMessage::Type::ALLIES_PROPOSAL)
            .value("acceptAlliesProposal",  DiplomaticMessage::Type::ACCEPT_ALLIES_PROPOSAL)
            .value("endAllies",             DiplomaticMessage::Type::END_ALLIANCE_DECLARATION)
            .value("cancelProposal",        DiplomaticMessage::Type::CANCEL_PROPOSAL)
            .value("rejectProposal",        DiplomaticMessage::Type::REJECT_PROPOSAL)
        ;
        py::enum_<CaptureResult>("captureResult")
            .value("capture",       CaptureResult::CR_CAPTURE)
            .value("destroy",       CaptureResult::CR_DESTROY)
            .value("retain",        CaptureResult::CR_RETAIN)
        ;
        py::enum_<EffectsCauseType>("effectsCauseType")
            .value("invalid",       EffectsCauseType::INVALID_EFFECTS_GROUP_CAUSE_TYPE)
            .value("unknown",       EffectsCauseType::ECT_UNKNOWN_CAUSE)
            .value("inherent",      EffectsCauseType::ECT_INHERENT)
            .value("tech",          EffectsCauseType::ECT_TECH)
            .value("building",      EffectsCauseType::ECT_BUILDING)
            .value("field",         EffectsCauseType::ECT_FIELD)
            .value("special",       EffectsCauseType::ECT_SPECIAL)
            .value("species",       EffectsCauseType::ECT_SPECIES)
            .value("shipPart",      EffectsCauseType::ECT_SHIP_PART)
            .value("shipHull",      EffectsCauseType::ECT_SHIP_HULL)
            .value("policy",        EffectsCauseType::ECT_POLICY)
        ;
        py::enum_<ShipSlotType>("shipSlotType")
            .value("external",      ShipSlotType::SL_EXTERNAL)
            .value("internal",      ShipSlotType::SL_INTERNAL)
            .value("core",          ShipSlotType::SL_CORE)
        ;
        py::enum_<ShipPartClass>("shipPartClass")
            .value("shortRange",        ShipPartClass::PC_DIRECT_WEAPON)
            .value("fighterBay",        ShipPartClass::PC_FIGHTER_BAY)
            .value("fighterHangar",     ShipPartClass::PC_FIGHTER_HANGAR)
            .value("shields",           ShipPartClass::PC_SHIELD)
            .value("armour",            ShipPartClass::PC_ARMOUR)
            .value("troops",            ShipPartClass::PC_TROOPS)
            .value("detection",         ShipPartClass::PC_DETECTION)
            .value("stealth",           ShipPartClass::PC_STEALTH)
            .value("fuel",              ShipPartClass::PC_FUEL)
            .value("colony",            ShipPartClass::PC_COLONY)
            .value("speed",             ShipPartClass::PC_SPEED)
            .value("general",           ShipPartClass::PC_GENERAL)
            .value("bombard",           ShipPartClass::PC_BOMBARD)
            .value("industry",          ShipPartClass::PC_INDUSTRY)
            .value("research",          ShipPartClass::PC_RESEARCH)
            .value("influence",         ShipPartClass::PC_INFLUENCE)
            .value("productionLocation",ShipPartClass::PC_PRODUCTION_LOCATION)
        ;
        py::enum_<UnlockableItemType>("unlockableItemType")
            .value("invalid",       UnlockableItemType::INVALID_UNLOCKABLE_ITEM_TYPE)
            .value("building",      UnlockableItemType::UIT_BUILDING)
            .value("shipPart",      UnlockableItemType::UIT_SHIP_PART)
            .value("shipHull",      UnlockableItemType::UIT_SHIP_HULL)
            .value("shipDesign",    UnlockableItemType::UIT_SHIP_DESIGN)
            .value("tech",          UnlockableItemType::UIT_TECH)
            .value("policy",        UnlockableItemType::UIT_POLICY)
        ;
        py::enum_<Aggression>("aggression")
            .value("invalid",       Aggression::INVALID_AGGRESSION)
            .value("beginner",      Aggression::BEGINNER)
            .value("turtle",        Aggression::TURTLE)
            .value("cautious",      Aggression::CAUTIOUS)
            .value("typical",       Aggression::TYPICAL)
            .value("aggressive",    Aggression::AGGRESSIVE)
            .value("maniacal",      Aggression::MANIACAL)
        ;
        py::enum_<GalaxySetupOptionGeneric>("galaxySetupOptionGeneric")
            .value("invalid",       GalaxySetupOptionGeneric::INVALID_GALAXY_SETUP_OPTION)
            .value("none",          GalaxySetupOptionGeneric::GALAXY_SETUP_NONE)
            .value("low",           GalaxySetupOptionGeneric::GALAXY_SETUP_LOW)
            .value("medium",        GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM)
            .value("high",          GalaxySetupOptionGeneric::GALAXY_SETUP_HIGH)
            .value("random",        GalaxySetupOptionGeneric::GALAXY_SETUP_RANDOM)
        ;
        py::enum_<GalaxySetupOptionMonsterFreq>("galaxySetupOptionMonsterFreq")
            .value("invalid",       GalaxySetupOptionMonsterFreq::INVALID_MONSTER_SETUP_OPTION)
            .value("none",          GalaxySetupOptionMonsterFreq::MONSTER_SETUP_NONE)
            .value("extremelyLow",  GalaxySetupOptionMonsterFreq::MONSTER_SETUP_EXTREMELY_LOW)
            .value("veryLow",       GalaxySetupOptionMonsterFreq::MONSTER_SETUP_VERY_LOW)
            .value("low",           GalaxySetupOptionMonsterFreq::MONSTER_SETUP_LOW)
            .value("medium",        GalaxySetupOptionMonsterFreq::MONSTER_SETUP_MEDIUM)
            .value("high",          GalaxySetupOptionMonsterFreq::MONSTER_SETUP_HIGH)
            .value("veryHigh",      GalaxySetupOptionMonsterFreq::MONSTER_SETUP_VERY_HIGH)
            .value("extremelyHigh", GalaxySetupOptionMonsterFreq::MONSTER_SETUP_EXTREMELY_HIGH)
            .value("random",        GalaxySetupOptionMonsterFreq::MONSTER_SETUP_RANDOM)
        ;
        py::enum_<Shape>("galaxyShape")
            .value("invalid",       Shape::INVALID_SHAPE)
            .value("spiral2",       Shape::SPIRAL_2)
            .value("spiral3",       Shape::SPIRAL_3)
            .value("spiral4",       Shape::SPIRAL_4)
            .value("cluster",       Shape::CLUSTER)
            .value("elliptical",    Shape::ELLIPTICAL)
            .value("disc",          Shape::DISC)
            .value("box",           Shape::BOX)
            .value("irregular",     Shape::IRREGULAR)
            .value("ring",          Shape::RING)
            .value("random",        Shape::RANDOM)
        ;
        py::enum_<GameRule::Type>("ruleType")
            .value("invalid",       GameRule::Type::INVALID)
            .value("toggle",        GameRule::Type::TOGGLE)
            .value("int",           GameRule::Type::INT)
            .value("double",        GameRule::Type::DOUBLE)
            .value("string",        GameRule::Type::STRING)
        ;
        py::enum_<Networking::RoleType>("roleType")
            .value("host",                Networking::RoleType::ROLE_HOST)
            .value("clientTypeModerator", Networking::RoleType::ROLE_CLIENT_TYPE_MODERATOR)
            .value("clientTypePlayer",    Networking::RoleType::ROLE_CLIENT_TYPE_PLAYER)
            .value("clientTypeObserver",  Networking::RoleType::ROLE_CLIENT_TYPE_OBSERVER)
            .value("galaxySetup",         Networking::RoleType::ROLE_GALAXY_SETUP)
        ;
    }
}
