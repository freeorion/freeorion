// -*- C++ -*-
#ifndef _Enums_h_
#define _Enums_h_

#include <GG/Enum.h>

#include <iostream>


/* the various major subclasses of UniverseObject */
enum UniverseObjectType {
    INVALID_UNIVERSE_OBJECT_TYPE = -1,
    OBJ_BUILDING,
    OBJ_SHIP,
    OBJ_FLEET, 
    OBJ_PLANET,
    OBJ_POP_CENTER,
    OBJ_PROD_CENTER,
    OBJ_SYSTEM,
    NUM_OBJ_TYPES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(UniverseObjectType)
    GG_ENUM_MAP_INSERT(INVALID_UNIVERSE_OBJECT_TYPE)
    GG_ENUM_MAP_INSERT(OBJ_BUILDING)
    GG_ENUM_MAP_INSERT(OBJ_SHIP)
    GG_ENUM_MAP_INSERT(OBJ_FLEET)
    GG_ENUM_MAP_INSERT(OBJ_PLANET)
    GG_ENUM_MAP_INSERT(OBJ_POP_CENTER)
    GG_ENUM_MAP_INSERT(OBJ_PROD_CENTER)
    GG_ENUM_MAP_INSERT(OBJ_SYSTEM)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(UniverseObjectType)
GG_ENUM_STREAM_OUT(UniverseObjectType)

/** types of stars */
enum StarType {
    INVALID_STAR_TYPE = -1,
    STAR_BLUE,
    STAR_WHITE,
    STAR_YELLOW,
    STAR_ORANGE,
    STAR_RED,
    STAR_NEUTRON,
    STAR_BLACK,
    STAR_NONE,
    NUM_STAR_TYPES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(StarType)
    GG_ENUM_MAP_INSERT(INVALID_STAR_TYPE)
    GG_ENUM_MAP_INSERT(STAR_BLUE)
    GG_ENUM_MAP_INSERT(STAR_WHITE)
    GG_ENUM_MAP_INSERT(STAR_YELLOW)
    GG_ENUM_MAP_INSERT(STAR_ORANGE)
    GG_ENUM_MAP_INSERT(STAR_RED)
    GG_ENUM_MAP_INSERT(STAR_NEUTRON)
    GG_ENUM_MAP_INSERT(STAR_BLACK)
    GG_ENUM_MAP_INSERT(STAR_NONE)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(StarType)
GG_ENUM_STREAM_OUT(StarType)

/** types of planets */
enum PlanetType {
    INVALID_PLANET_TYPE = -1,
    PT_SWAMP,
    PT_TOXIC,
    PT_INFERNO,
    PT_RADIATED,
    PT_BARREN,
    PT_TUNDRA,
    PT_DESERT,
    PT_TERRAN,
    PT_OCEAN,
    PT_ASTEROIDS,
    PT_GASGIANT,
    NUM_PLANET_TYPES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(PlanetType)
    GG_ENUM_MAP_INSERT(INVALID_PLANET_TYPE)
    GG_ENUM_MAP_INSERT(PT_SWAMP)
    GG_ENUM_MAP_INSERT(PT_TOXIC)
    GG_ENUM_MAP_INSERT(PT_INFERNO)
    GG_ENUM_MAP_INSERT(PT_RADIATED)
    GG_ENUM_MAP_INSERT(PT_BARREN)
    GG_ENUM_MAP_INSERT(PT_TUNDRA)
    GG_ENUM_MAP_INSERT(PT_DESERT)
    GG_ENUM_MAP_INSERT(PT_TERRAN)
    GG_ENUM_MAP_INSERT(PT_OCEAN)
    GG_ENUM_MAP_INSERT(PT_ASTEROIDS)
    GG_ENUM_MAP_INSERT(PT_GASGIANT)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(PlanetType)
GG_ENUM_STREAM_OUT(PlanetType)

/** sizes of planets */
enum PlanetSize {
    INVALID_PLANET_SIZE = -1,
    SZ_NOWORLD,         ///< used to designate an empty planet slot
    SZ_TINY,
    SZ_SMALL,
    SZ_MEDIUM,
    SZ_LARGE,
    SZ_HUGE,
    SZ_ASTEROIDS,
    SZ_GASGIANT,
    NUM_PLANET_SIZES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(PlanetSize)
    GG_ENUM_MAP_INSERT(INVALID_PLANET_SIZE)
    GG_ENUM_MAP_INSERT(SZ_NOWORLD)
    GG_ENUM_MAP_INSERT(SZ_TINY)
    GG_ENUM_MAP_INSERT(SZ_SMALL)
    GG_ENUM_MAP_INSERT(SZ_MEDIUM)
    GG_ENUM_MAP_INSERT(SZ_LARGE)
    GG_ENUM_MAP_INSERT(SZ_HUGE)
    GG_ENUM_MAP_INSERT(SZ_ASTEROIDS)
    GG_ENUM_MAP_INSERT(SZ_GASGIANT)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(PlanetSize)
GG_ENUM_STREAM_OUT(PlanetSize)


/** environmental suitability of planets for a particular race */
enum PlanetEnvironment {
    INVALID_PLANET_ENVIRONMENT = -1,
    PE_UNINHABITABLE,
    PE_HOSTILE,
    PE_POOR,
    PE_ADEQUATE,
    PE_GOOD,
    NUM_PLANET_ENVIRONMENTS
};

namespace GG {
    GG_ENUM_MAP_BEGIN(PlanetEnvironment)
    GG_ENUM_MAP_INSERT(INVALID_PLANET_ENVIRONMENT)
    GG_ENUM_MAP_INSERT(PE_UNINHABITABLE)
    GG_ENUM_MAP_INSERT(PE_HOSTILE)
    GG_ENUM_MAP_INSERT(PE_POOR)
    GG_ENUM_MAP_INSERT(PE_ADEQUATE)
    GG_ENUM_MAP_INSERT(PE_GOOD)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(PlanetEnvironment)
GG_ENUM_STREAM_OUT(PlanetEnvironment)

/** types for Meters */
enum MeterType {
    INVALID_METER_TYPE = -1,
    METER_TARGET_POPULATION,
    METER_TARGET_INDUSTRY,
    METER_TARGET_RESEARCH,
    METER_TARGET_TRADE,
    METER_TARGET_MINING,
    METER_TARGET_CONSTRUCTION,

    METER_MAX_FUEL,
    METER_MAX_SHIELD,
    METER_MAX_STRUCTURE,
    METER_MAX_DEFENSE,
    METER_MAX_TROOPS,

    METER_POPULATION,
    METER_INDUSTRY,
    METER_RESEARCH,
    METER_TRADE,
    METER_MINING,
    METER_CONSTRUCTION,

    METER_FUEL,
    METER_SHIELD,
    METER_STRUCTURE,
    METER_DEFENSE,
    METER_TROOPS,

    METER_SUPPLY,
    METER_STEALTH,
    METER_DETECTION,
    METER_BATTLE_SPEED,
    METER_STARLANE_SPEED,

    // These meter enumerators only apply to ship part meters.

    METER_DAMAGE,
    METER_ROF,
    METER_RANGE,
    METER_SPEED,
    METER_CAPACITY,
    METER_ANTI_SHIP_DAMAGE,
    METER_ANTI_FIGHTER_DAMAGE,
    METER_LAUNCH_RATE,
    METER_FIGHTER_WEAPON_RANGE,

    NUM_METER_TYPES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(MeterType)
    GG_ENUM_MAP_INSERT(INVALID_METER_TYPE)
    GG_ENUM_MAP_INSERT(METER_TARGET_POPULATION)
    GG_ENUM_MAP_INSERT(METER_TARGET_INDUSTRY)
    GG_ENUM_MAP_INSERT(METER_TARGET_RESEARCH)
    GG_ENUM_MAP_INSERT(METER_TARGET_TRADE)
    GG_ENUM_MAP_INSERT(METER_TARGET_MINING)
    GG_ENUM_MAP_INSERT(METER_TARGET_CONSTRUCTION)

    GG_ENUM_MAP_INSERT(METER_MAX_FUEL)
    GG_ENUM_MAP_INSERT(METER_MAX_SHIELD)
    GG_ENUM_MAP_INSERT(METER_MAX_STRUCTURE)
    GG_ENUM_MAP_INSERT(METER_MAX_DEFENSE)
    GG_ENUM_MAP_INSERT(METER_MAX_TROOPS)

    GG_ENUM_MAP_INSERT(METER_POPULATION)
    GG_ENUM_MAP_INSERT(METER_INDUSTRY)
    GG_ENUM_MAP_INSERT(METER_RESEARCH)
    GG_ENUM_MAP_INSERT(METER_TRADE)
    GG_ENUM_MAP_INSERT(METER_MINING)
    GG_ENUM_MAP_INSERT(METER_CONSTRUCTION)

    GG_ENUM_MAP_INSERT(METER_FUEL)
    GG_ENUM_MAP_INSERT(METER_SHIELD)
    GG_ENUM_MAP_INSERT(METER_STRUCTURE)
    GG_ENUM_MAP_INSERT(METER_DEFENSE)
    GG_ENUM_MAP_INSERT(METER_TROOPS)

    GG_ENUM_MAP_INSERT(METER_SUPPLY)
    GG_ENUM_MAP_INSERT(METER_STEALTH)
    GG_ENUM_MAP_INSERT(METER_DETECTION)
    GG_ENUM_MAP_INSERT(METER_BATTLE_SPEED)
    GG_ENUM_MAP_INSERT(METER_STARLANE_SPEED)

    GG_ENUM_MAP_INSERT(METER_DAMAGE)
    GG_ENUM_MAP_INSERT(METER_ROF)
    GG_ENUM_MAP_INSERT(METER_RANGE)
    GG_ENUM_MAP_INSERT(METER_SPEED)
    GG_ENUM_MAP_INSERT(METER_CAPACITY)
    GG_ENUM_MAP_INSERT(METER_ANTI_SHIP_DAMAGE)
    GG_ENUM_MAP_INSERT(METER_ANTI_FIGHTER_DAMAGE)
    GG_ENUM_MAP_INSERT(METER_LAUNCH_RATE)
    GG_ENUM_MAP_INSERT(METER_FIGHTER_WEAPON_RANGE)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(MeterType)
GG_ENUM_STREAM_OUT(MeterType)

/** types of universe shapes during galaxy generation */
enum Shape {
    INVALID_SHAPE = -1,
    SPIRAL_2,       ///< a two-armed spiral galaxy
    SPIRAL_3,       ///< a three-armed spiral galaxy
    SPIRAL_4,       ///< a four-armed spiral galaxy
    CLUSTER,        ///< a cluster galaxy
    ELLIPTICAL,     ///< an elliptical galaxy
    IRREGULAR,      ///< an irregular galaxy
    RING,           ///< a ring galaxy
    GALAXY_SHAPES   ///< the number of shapes in this enum (leave this last)
};

namespace GG {
    GG_ENUM_MAP_BEGIN(Shape)
    GG_ENUM_MAP_INSERT(INVALID_SHAPE)
    GG_ENUM_MAP_INSERT(SPIRAL_2)
    GG_ENUM_MAP_INSERT(SPIRAL_3)
    GG_ENUM_MAP_INSERT(SPIRAL_4)
    GG_ENUM_MAP_INSERT(CLUSTER)
    GG_ENUM_MAP_INSERT(ELLIPTICAL)
    GG_ENUM_MAP_INSERT(IRREGULAR)
    GG_ENUM_MAP_INSERT(RING)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(Shape)
GG_ENUM_STREAM_OUT(Shape)

/** General-use option for galaxy setup picks with "more" or "less" options. */
enum GalaxySetupOption {
    INVALID_GALAXY_SETUP_OPTION = -1,
    GALAXY_SETUP_NONE,
    GALAXY_SETUP_LOW,
    GALAXY_SETUP_MEDIUM,
    GALAXY_SETUP_HIGH,
    NUM_GALAXY_SETUP_OPTIONS
};

namespace GG {
    GG_ENUM_MAP_BEGIN(GalaxySetupOption)
    GG_ENUM_MAP_INSERT(INVALID_GALAXY_SETUP_OPTION)
    GG_ENUM_MAP_INSERT(GALAXY_SETUP_NONE)
    GG_ENUM_MAP_INSERT(GALAXY_SETUP_LOW)
    GG_ENUM_MAP_INSERT(GALAXY_SETUP_MEDIUM)
    GG_ENUM_MAP_INSERT(GALAXY_SETUP_HIGH)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(GalaxySetupOption)
GG_ENUM_STREAM_OUT(GalaxySetupOption)

/** types of diplomatic empire affiliations to another empire*/
enum EmpireAffiliationType {
    INVALID_EMPIRE_AFFIL_TYPE = -1,
    AFFIL_SELF,     ///< not an affiliation as such; this indicates that the given empire, rather than its affiliates
    AFFIL_ENEMY,    ///< enemies of the given empire
    AFFIL_ALLY,     ///< allies of the given empire
    AFFIL_ANY,      ///< any empire
    NUM_AFFIL_TYPES ///< keep last, the number of affiliation types
};

namespace GG {
    GG_ENUM_MAP_BEGIN(EmpireAffiliationType)
    GG_ENUM_MAP_INSERT(INVALID_EMPIRE_AFFIL_TYPE)
    GG_ENUM_MAP_INSERT(AFFIL_ENEMY)
    GG_ENUM_MAP_INSERT(AFFIL_SELF)
    GG_ENUM_MAP_INSERT(AFFIL_ALLY)
    GG_ENUM_MAP_INSERT(AFFIL_ANY)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(EmpireAffiliationType)
GG_ENUM_STREAM_OUT(EmpireAffiliationType)

/** types of items that can be unlocked for empires */
enum UnlockableItemType {
    INVALID_UNLOCKABLE_ITEM_TYPE = -1,
    UIT_BUILDING,               ///< a kind of Building
    UIT_SHIP_PART,              ///< a kind of ship part (which are placed into hulls to make designs)
    UIT_SHIP_HULL,              ///< a ship hull (into which parts are placed)
    UIT_SHIP_DESIGN,            ///< a complete ship design
    UIT_TECH,                   ///< a technology
    NUM_UNLOCKABLE_ITEM_TYPES   ///< keep last, the number of types of unlockable item
};

namespace GG {
    GG_ENUM_MAP_BEGIN(UnlockableItemType)
    GG_ENUM_MAP_INSERT(INVALID_UNLOCKABLE_ITEM_TYPE)
    GG_ENUM_MAP_INSERT(UIT_BUILDING)
    GG_ENUM_MAP_INSERT(UIT_SHIP_PART)
    GG_ENUM_MAP_INSERT(UIT_SHIP_HULL)
    GG_ENUM_MAP_INSERT(UIT_SHIP_DESIGN)
    GG_ENUM_MAP_INSERT(UIT_TECH)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(UnlockableItemType)
GG_ENUM_STREAM_OUT(UnlockableItemType)

/** General classification for purpose and function of techs, and allowed place in tech prerequisite tree */
enum TechType {
    INVALID_TECH_TYPE = -1,
    TT_THEORY,      ///< Theory: does nothing itself, but is prerequisite for applications and refinements
    TT_APPLICATION, ///< Application: has effects that do things, or may unlock something such as a building that does things
    TT_REFINEMENT,  ///< Refinement: does nothing itself, but if researched, may alter the effects of something else
    NUM_TECH_TYPES  ///< keep last, the number of types of tech
};

namespace GG {
    GG_ENUM_MAP_BEGIN(TechType)
    GG_ENUM_MAP_INSERT(INVALID_TECH_TYPE)
    GG_ENUM_MAP_INSERT(TT_THEORY)
    GG_ENUM_MAP_INSERT(TT_APPLICATION)
    GG_ENUM_MAP_INSERT(TT_REFINEMENT)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(TechType)
GG_ENUM_STREAM_OUT(TechType)

/** Research status of techs, relating to whether they have been or can be researched */
enum TechStatus {
    INVALID_TECH_STATUS = -1,
    TS_UNRESEARCHABLE,
    TS_RESEARCHABLE,
    TS_COMPLETE,
    NUM_TECH_STATUSES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(TechStatus)
    GG_ENUM_MAP_INSERT(INVALID_TECH_STATUS)
    GG_ENUM_MAP_INSERT(TS_UNRESEARCHABLE)
    GG_ENUM_MAP_INSERT(TS_RESEARCHABLE)
    GG_ENUM_MAP_INSERT(TS_COMPLETE)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(TechStatus)
GG_ENUM_STREAM_OUT(TechStatus)

/** The general type of construction being done at a ProdCenter.  Within each valid type, a specific kind 
    of item is being built, e.g. under BUILDING a kind of building called "SuperFarm" might be built. */
enum BuildType {
    INVALID_BUILD_TYPE = -1,
    BT_NOT_BUILDING,         ///< no building is taking place
    BT_BUILDING,             ///< a Building object is being built
    BT_SHIP,                 ///< a Ship object is being built
    NUM_BUILD_TYPES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(BuildType)
    GG_ENUM_MAP_INSERT(INVALID_BUILD_TYPE)
    GG_ENUM_MAP_INSERT(BT_NOT_BUILDING)
    GG_ENUM_MAP_INSERT(BT_BUILDING)
    GG_ENUM_MAP_INSERT(BT_SHIP)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(BuildType)
GG_ENUM_STREAM_OUT(BuildType)

/** Types of resources that planets can produce */
enum ResourceType {
    INVALID_RESOURCE_TYPE = -1,
    RE_MINERALS,
    RE_INDUSTRY,
    RE_TRADE,
    RE_RESEARCH,
    NUM_RESOURCE_TYPES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(ResourceType)
    GG_ENUM_MAP_INSERT(INVALID_RESOURCE_TYPE)
    GG_ENUM_MAP_INSERT(RE_MINERALS)
    GG_ENUM_MAP_INSERT(RE_INDUSTRY)
    GG_ENUM_MAP_INSERT(RE_TRADE)
    GG_ENUM_MAP_INSERT(RE_RESEARCH)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(ResourceType)
GG_ENUM_STREAM_OUT(ResourceType)

/** Types of fighters. */
enum CombatFighterType {
    INVALID_COMBAT_FIGHTER_TYPE,

    /** A fighter that is better at attacking other fighters than at
        attacking ships. */
    INTERCEPTOR,

    /** A fighter that is better at attacking ships than at attacking
        other fighters. */
    BOMBER
};

namespace GG {
    GG_ENUM_MAP_BEGIN(CombatFighterType)
    GG_ENUM_MAP_INSERT(INVALID_COMBAT_FIGHTER_TYPE)
    GG_ENUM_MAP_INSERT(INTERCEPTOR)
    GG_ENUM_MAP_INSERT(BOMBER)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(CombatFighterType)
GG_ENUM_STREAM_OUT(CombatFighterType)

/** Types "classes" of ship parts */
enum ShipPartClass {
    INVALID_SHIP_PART_CLASS = -1,
    PC_SHORT_RANGE,         ///< short range direct weapons, good against ships, bad against fighters
    PC_MISSILES,            ///< long range indirect weapons, good against ships, bad against fighters
    PC_FIGHTERS,            ///< self-propelled long-range weapon platforms, good against fighters and/or ships
    PC_POINT_DEFENSE,       ///< short range direct weapons, good against fighters or incoming missiles, bad against ships
    PC_SHIELD,              ///< energy-based defense
    PC_ARMOUR,              ///< defensive material on hull of ship
    PC_TROOPS,              ///< ground troops, used to conquer planets
    PC_DETECTION,           ///< range of vision and seeing through stealth
    PC_STEALTH,             ///< hiding from enemies
    PC_FUEL,                ///< distance that can be traveled away from resupply
    PC_COLONY,              ///< transports colonists and allows ships to make new colonies
    PC_BATTLE_SPEED,        ///< affects ship speed in battle
    PC_STARLANE_SPEED,      ///< affects ship speed on starlanes
    NUM_SHIP_PART_CLASSES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(ShipPartClass)
    GG_ENUM_MAP_INSERT(INVALID_SHIP_PART_CLASS)
    GG_ENUM_MAP_INSERT(PC_SHORT_RANGE)
    GG_ENUM_MAP_INSERT(PC_MISSILES)
    GG_ENUM_MAP_INSERT(PC_FIGHTERS)
    GG_ENUM_MAP_INSERT(PC_POINT_DEFENSE)
    GG_ENUM_MAP_INSERT(PC_SHIELD)
    GG_ENUM_MAP_INSERT(PC_ARMOUR)
    GG_ENUM_MAP_INSERT(PC_TROOPS)
    GG_ENUM_MAP_INSERT(PC_DETECTION)
    GG_ENUM_MAP_INSERT(PC_STEALTH)
    GG_ENUM_MAP_INSERT(PC_FUEL)
    GG_ENUM_MAP_INSERT(PC_COLONY)
    GG_ENUM_MAP_INSERT(PC_BATTLE_SPEED)
    GG_ENUM_MAP_INSERT(PC_STARLANE_SPEED)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(ShipPartClass)
GG_ENUM_STREAM_OUT(ShipPartClass)

/* Types of slots in hulls.  Parts may be restricted to only certain slot types */
enum ShipSlotType {
    INVALID_SHIP_SLOT_TYPE = -1,
    SL_EXTERNAL,            ///< external slots.  more easily damaged
    SL_INTERNAL,            ///< internal slots.  more protected, fewer in number
    NUM_SHIP_SLOT_TYPES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(ShipSlotType)
    GG_ENUM_MAP_INSERT(INVALID_SHIP_SLOT_TYPE)
    GG_ENUM_MAP_INSERT(SL_EXTERNAL)
    GG_ENUM_MAP_INSERT(SL_INTERNAL)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(ShipSlotType)
GG_ENUM_STREAM_OUT(ShipSlotType)


/** Returns the equivalent meter type for the given resource type; if no such
  * meter type exists, returns INVALID_METER_TYPE. */
MeterType ResourceToMeter(ResourceType type);

/** Returns the equivalent resource type for the given meter type; if no such
  * resource type exists, returns INVALID_RESOURCE_TYPE. */
ResourceType MeterToResource(MeterType type);

/** Returns the target or max meter type that is associated with the given
  * active meter type.  If no associated meter type exists, INVALID_METER_TYPE
  * is returned. */
MeterType AssociatedMeterType(MeterType meter_type);


extern const int ALL_EMPIRES;


/** degrees of visibility an Empire or UniverseObject can have for an
  * UniverseObject.  determines how much information the empire
  * gets about the (non)visible object. */
enum Visibility {
    INVALID_VISIBILITY = -1,
    VIS_NO_VISIBILITY,
    VIS_BASIC_VISIBILITY,
    VIS_PARTIAL_VISIBILITY,
    VIS_FULL_VISIBILITY,
    NUM_VISIBILITIES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(Visibility)
    GG_ENUM_MAP_INSERT(INVALID_VISIBILITY)
    GG_ENUM_MAP_INSERT(VIS_NO_VISIBILITY)
    GG_ENUM_MAP_INSERT(VIS_BASIC_VISIBILITY)
    GG_ENUM_MAP_INSERT(VIS_PARTIAL_VISIBILITY)
    GG_ENUM_MAP_INSERT(VIS_FULL_VISIBILITY)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(Visibility)
GG_ENUM_STREAM_OUT(Visibility)


/** Possible results of an UniverseObject being captured by other empires, or an
  * object's containing UniverseObject being captured, or the location of a
  * Production Queue Build Item being conquered, or the result of other future
  * events, such as spy activity... */
enum CaptureResult {
    INVALID_CAPTURE_RESULT = -1,
    CR_CAPTURE,    // object has ownership by original empire(s) removed, and conquering empire added
    CR_DESTROY,    // object is destroyed
    CR_RETAIN      // object ownership unchanged: original empire(s) still own object
};

/** Types of in-game things that might contain an EffectsGroup, or "cause" effects to occur */
enum EffectsCauseType {
    INVALID_EFFECTS_GROUP_CAUSE_TYPE = -1,
    ECT_UNKNOWN_CAUSE,
    ECT_INHERENT,
    ECT_TECH,
    ECT_BUILDING,
    ECT_SPECIAL,
    ECT_SPECIES,
    ECT_SHIP_PART,
    ECT_SHIP_HULL
};


#endif // _Enums_h_
