#include "../universe/Enums.h"
#include "../Empire/Diplomacy.h"

#include <boost/python.hpp>

namespace FreeOrionPython {
    using boost::python::enum_;
    void WrapGameStateEnums() {
        enum_<StarType>("starType")
            .value("blue",      STAR_BLUE)
            .value("white",     STAR_WHITE)
            .value("yellow",    STAR_YELLOW)
            .value("orange",    STAR_ORANGE)
            .value("red",       STAR_RED)
            .value("neutron",   STAR_NEUTRON)
            .value("blackHole", STAR_BLACK)
            .value("noStar",    STAR_NONE)
            .value("unknown",   INVALID_STAR_TYPE)
        ;
        enum_<PlanetSize>("planetSize")
            .value("tiny",      SZ_TINY)
            .value("small",     SZ_SMALL)
            .value("medium",    SZ_MEDIUM)
            .value("large",     SZ_LARGE)
            .value("huge",      SZ_HUGE)
            .value("asteroids", SZ_ASTEROIDS)
            .value("gasGiant",  SZ_GASGIANT)
        ;
        enum_<PlanetType>("planetType")
            .value("swamp",     PT_SWAMP)
            .value("radiated",  PT_RADIATED)
            .value("toxic",     PT_TOXIC)
            .value("inferno",   PT_INFERNO)
            .value("barren",    PT_BARREN)
            .value("tundra",    PT_TUNDRA)
            .value("desert",    PT_DESERT)
            .value("terran",    PT_TERRAN)
            .value("ocean",     PT_OCEAN)
            .value("asteroids", PT_ASTEROIDS)
            .value("gasGiant",  PT_GASGIANT)
        ;
        enum_<PlanetEnvironment>("planetEnvironment")
            .value("uninhabitable", PE_UNINHABITABLE)
            .value("hostile",       PE_HOSTILE)
            .value("poor",          PE_POOR)
            .value("adequate",      PE_ADEQUATE)
            .value("good",          PE_GOOD)
        ;
        enum_<TechType>("techType")
            .value("theory",        TT_THEORY)
            .value("application",   TT_APPLICATION)
            .value("refinement",    TT_REFINEMENT)
        ;
        enum_<TechStatus>("techStatus")
            .value("unresearchable",    TS_UNRESEARCHABLE)
            .value("researchable",      TS_RESEARCHABLE)
            .value("complete",          TS_COMPLETE)
        ;
        enum_<BuildType>("buildType")
            .value("building",          BT_BUILDING)
            .value("ship",              BT_SHIP)
        ;
        enum_<ResourceType>("resourceType")
            .value("industry",      RE_INDUSTRY)
            .value("trade",         RE_TRADE)
            .value("research",      RE_RESEARCH)
        ;
        enum_<MeterType>("meterType")
            .value("targetPopulation",  METER_TARGET_POPULATION)
            .value("targetIndustry",    METER_TARGET_INDUSTRY)
            .value("targetResearch",    METER_TARGET_RESEARCH)
            .value("targetTrade",       METER_TARGET_TRADE)
            .value("targetConstruction",METER_TARGET_CONSTRUCTION)

            .value("maxFuel",           METER_MAX_FUEL)
            .value("maxShield",         METER_MAX_SHIELD)
            .value("maxStructure",      METER_MAX_STRUCTURE)
            .value("maxDefense",        METER_MAX_DEFENSE)

            .value("population",        METER_POPULATION)
            .value("industry",          METER_INDUSTRY)
            .value("research",          METER_RESEARCH)
            .value("trade",             METER_TRADE)
            .value("construction",      METER_CONSTRUCTION)

            .value("fuel",              METER_FUEL)
            .value("shield",            METER_SHIELD)
            .value("structure",         METER_STRUCTURE)
            .value("defense",           METER_DEFENSE)

            .value("supply",            METER_SUPPLY)
            .value("stealth",           METER_STEALTH)
            .value("detection",         METER_DETECTION)
            .value("battleSpeed",       METER_BATTLE_SPEED)
            .value("starlaneSpeed",     METER_STARLANE_SPEED)

            .value("damage",            METER_DAMAGE)
            .value("rof",               METER_ROF)
            .value("range",             METER_RANGE)
            .value("speed",             METER_SPEED)
            .value("capacity",          METER_CAPACITY)
            .value("antiShipDamage",    METER_ANTI_SHIP_DAMAGE)
            .value("antiFighterDamage", METER_ANTI_FIGHTER_DAMAGE)
            .value("launchRate",        METER_LAUNCH_RATE)
            .value("fighterWeaponRange",METER_FIGHTER_WEAPON_RANGE)
        ;
        enum_<DiplomaticStatus>("diplomaticStatus")
            .value("war",               DIPLO_WAR)
            .value("peace",             DIPLO_PEACE)
        ;
        enum_<DiplomaticMessage::DiplomaticMessageType>("diplomaticMessageType")
            .value("noMessage",         DiplomaticMessage::INVALID_DIPLOMATIC_MESSAGE_TYPE)
            .value("warDeclaration",    DiplomaticMessage::WAR_DECLARATION)
            .value("peaceProposal",     DiplomaticMessage::PEACE_PROPOSAL)
            .value("acceptProposal",    DiplomaticMessage::ACCEPT_PROPOSAL)
            .value("cancelProposal",    DiplomaticMessage::CANCEL_PROPOSAL)
        ;
        enum_<CaptureResult>("captureResult")
            .value("capture",       CR_CAPTURE)
            .value("destroy",       CR_DESTROY)
            .value("retain",        CR_RETAIN)
        ;
        enum_<ShipSlotType>("shipSlotType")
            .value("external",      SL_EXTERNAL)
            .value("internal",      SL_INTERNAL)
        ;
        enum_<ShipPartClass>("shipPartClass")
            .value("shortRange",    PC_SHORT_RANGE)
            .value("missiles",      PC_MISSILES)
            .value("fighters",      PC_FIGHTERS)
            .value("pointDefense",  PC_POINT_DEFENSE)
            .value("shields",       PC_SHIELD)
            .value("armour",        PC_ARMOUR)
            .value("detection",     PC_DETECTION)
            .value("stealth",       PC_STEALTH)
            .value("fuel",          PC_FUEL)
            .value("colony",        PC_COLONY)
            .value("battleSpeed",   PC_BATTLE_SPEED)
            .value("starlaneSpeed", PC_STARLANE_SPEED)
        ;
    }
}
