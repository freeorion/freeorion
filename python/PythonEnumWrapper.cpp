#include "../universe/Enums.h"

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
            .value("unresearchable",    BT_BUILDING)
            .value("researchable",      BT_SHIP)
        ;
        enum_<ResourceType>("resourceType")
            .value("food",          RE_FOOD)
            .value("minerals",      RE_MINERALS)
            .value("industry",      RE_INDUSTRY)
            .value("trade",         RE_TRADE)
            .value("research",      RE_RESEARCH)
        ;
        enum_<MeterType>("meterType")
            .value("population",    METER_POPULATION)
            .value("farming",       METER_FARMING)
            .value("industry",      METER_INDUSTRY)
            .value("research",      METER_RESEARCH)
            .value("trade",         METER_TRADE)
            .value("mining",        METER_MINING)
            .value("construction",  METER_CONSTRUCTION)
            .value("health",        METER_HEALTH)
            .value("fuel",          METER_FUEL)
            .value("supply",        METER_SUPPLY)
            .value("stealth",       METER_STEALTH)
            .value("detection",     METER_DETECTION)
            .value("shield",        METER_SHIELD)
            .value("defense",       METER_DEFENSE)
        ;
        enum_<FocusType>("focusType")
            .value("balanced",      FOCUS_BALANCED)
            .value("farming",       FOCUS_FARMING)
            .value("industry",      FOCUS_INDUSTRY)
            .value("mining",        FOCUS_MINING)
            .value("research",      FOCUS_RESEARCH)
            .value("trade",         FOCUS_TRADE)
        ;
        enum_<CaptureResult>("captureResult")
            .value("capture",       CR_CAPTURE)
            .value("destroy",       CR_DESTROY)
            .value("retain",        CR_RETAIN)
            .value("share",         CR_SHARE)
        ;
        enum_<ShipSlotType>("shipSlotType")
            .value("external",      SL_EXTERNAL)
            .value("internal",      SL_INTERNAL)
        ;
        enum_<ShipPartClass>("shipPartClass")
            .value("shortrange",    PC_SHORT_RANGE)
            .value("missiles",      PC_MISSILES)
            .value("fighters",      PC_FIGHTERS)
            .value("pointdefense",  PC_POINT_DEFENSE)
            .value("shields",       PC_SHIELD)
            .value("armour",        PC_ARMOUR)
            .value("detection",     PC_DETECTION)
            .value("stealth",       PC_STEALTH)
            .value("fuel",          PC_FUEL)
        ;
    }
}