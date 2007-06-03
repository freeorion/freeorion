// -*- C++ -*-
#ifndef _Enums_h_
#define _Enums_h_

#include <GG/Enum.h>

/* the various major subclasses of UniverseObject */
enum UniverseObjectType {
    INVALID_UNIVERSE_OBJECT_TYPE,
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

/** types of stars in FreeOrion v0.2 */
enum StarType {
    INVALID_STAR_TYPE = -1,  ///< the highest illegal negative StarType value
    STAR_BLUE,
    STAR_WHITE,
    STAR_YELLOW,
    STAR_ORANGE,
    STAR_RED,
    STAR_NEUTRON,
    STAR_BLACK,
    NUM_STAR_TYPES           ///< the lowest illegal positive StarType value
}; // others TBD

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
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(StarType)
GG_ENUM_STREAM_OUT(StarType)

/** the types of planets in FreeOrion*/
enum PlanetType {
    INVALID_PLANET_TYPE = -1,
    PT_SWAMP,
    PT_TOXIC,
    PT_INFERNO,
    PT_RADIATED,
    PT_BARREN,
    PT_TUNDRA,
    PT_DESERT,
    PT_TERRAN,                   //changed the order to be clockwise around the 
    PT_OCEAN,                    // wheel of EP, added Inferno and Swamp types
    PT_GAIA,
    PT_ASTEROIDS,                //these need to be types also so they can have an environment
    PT_GASGIANT,                     
    NUM_PLANET_TYPES   //keep this last
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
    GG_ENUM_MAP_INSERT(PT_GAIA)
    GG_ENUM_MAP_INSERT(PT_ASTEROIDS)
    GG_ENUM_MAP_INSERT(PT_GASGIANT)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(PlanetType)
GG_ENUM_STREAM_OUT(PlanetType)


/** the sizes of planets in FreeOrion*/
enum PlanetSize {
    INVALID_PLANET_SIZE = -1,
    SZ_NOWORLD,   // used to designate an empty planet slot
    SZ_TINY,
    SZ_SMALL,
    SZ_MEDIUM,
    SZ_LARGE,
    SZ_HUGE,
    SZ_ASTEROIDS,
    SZ_GASGIANT,
    NUM_PLANET_SIZES   //keep this last
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


/** the environmental conditions of planets in FreeOrion*/
enum PlanetEnvironment {
    INVALID_PLANET_ENVIRONMENT = -1,
    PE_UNINHABITABLE,   //for gas giants and asteroids
    PE_TERRIBLE,
    PE_ADEQUATE,
    PE_SUPERB,
    PE_OPTIMAL,
    NUM_PLANET_ENVIRONMENTS   //keep this last
};

namespace GG {
    GG_ENUM_MAP_BEGIN(PlanetEnvironment)
    GG_ENUM_MAP_INSERT(INVALID_PLANET_ENVIRONMENT)
    GG_ENUM_MAP_INSERT(PE_UNINHABITABLE)
    GG_ENUM_MAP_INSERT(PE_TERRIBLE)
    GG_ENUM_MAP_INSERT(PE_ADEQUATE)
    GG_ENUM_MAP_INSERT(PE_SUPERB)
    GG_ENUM_MAP_INSERT(PE_OPTIMAL)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(PlanetEnvironment)
GG_ENUM_STREAM_OUT(PlanetEnvironment)


/** the types of production focus*/
enum FocusType {
    INVALID_FOCUS_TYPE = -1,
    FOCUS_UNKNOWN,
    FOCUS_BALANCED,
    FOCUS_FARMING,
    FOCUS_INDUSTRY,
    FOCUS_MINING,
    FOCUS_RESEARCH,
    FOCUS_TRADE,
    NUM_FOCI
}; // others TBD
                      
namespace GG {
    GG_ENUM_MAP_BEGIN(FocusType)
    GG_ENUM_MAP_INSERT(INVALID_FOCUS_TYPE)
    GG_ENUM_MAP_INSERT(FOCUS_UNKNOWN)
    GG_ENUM_MAP_INSERT(FOCUS_BALANCED)
    GG_ENUM_MAP_INSERT(FOCUS_FARMING)
    GG_ENUM_MAP_INSERT(FOCUS_INDUSTRY)
    GG_ENUM_MAP_INSERT(FOCUS_MINING)
    GG_ENUM_MAP_INSERT(FOCUS_RESEARCH)
    GG_ENUM_MAP_INSERT(FOCUS_TRADE)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(FocusType)
GG_ENUM_STREAM_OUT(FocusType)


/** the possible types for Meters in FreeOrion. */
enum MeterType {
    INVALID_METER_TYPE = -1,
    METER_POPULATION,
    METER_FARMING,
    METER_INDUSTRY,
    METER_RESEARCH,
    METER_TRADE,
    METER_MINING,
    METER_CONSTRUCTION,
    METER_HEALTH,
    /* Future meter types (more TBD)
    METER_HAPPINESS,
    METER_SECURITY
    */
    NUM_METER_TYPES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(MeterType)
    GG_ENUM_MAP_INSERT(INVALID_METER_TYPE)
    GG_ENUM_MAP_INSERT(METER_FARMING)
    GG_ENUM_MAP_INSERT(METER_POPULATION)
    GG_ENUM_MAP_INSERT(METER_INDUSTRY)
    GG_ENUM_MAP_INSERT(METER_RESEARCH)
    GG_ENUM_MAP_INSERT(METER_TRADE)
    GG_ENUM_MAP_INSERT(METER_MINING)
    GG_ENUM_MAP_INSERT(METER_CONSTRUCTION)
    GG_ENUM_MAP_INSERT(METER_HEALTH)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(MeterType)
GG_ENUM_STREAM_OUT(MeterType)

/** the types of universe shapes available in FreeOrion*/
enum Shape {
    INVALID_SHAPE = -1,
    SPIRAL_2,      ///< a two-armed spiral galaxy
    SPIRAL_3,      ///< a three-armed spiral galaxy
    SPIRAL_4,      ///< a four-armed spiral galaxy
    CLUSTER,       ///< a cluster galaxy
    ELLIPTICAL,    ///< an elliptical galaxy
    IRREGULAR,     ///< an irregular galaxy
    RING,          ///< a ring galaxy
    GALAXY_SHAPES  ///< the number of shapes in this enum (leave this last)
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


/** types of Univervse ages*/
enum Age {
    INVALID_AGE = -1,
    AGE_YOUNG,
    AGE_MATURE,
    AGE_ANCIENT,
    NUM_UNIVERSE_AGES    // keep this last, the number of universe age options
};

namespace GG {
    GG_ENUM_MAP_BEGIN(Age)
    GG_ENUM_MAP_INSERT(INVALID_AGE)
    GG_ENUM_MAP_INSERT(AGE_YOUNG)
    GG_ENUM_MAP_INSERT(AGE_MATURE)
    GG_ENUM_MAP_INSERT(AGE_ANCIENT)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(Age)
GG_ENUM_STREAM_OUT(Age)


/** types of Planet Density */
enum PlanetDensity {
    INVALID_PLANET_DENSITY = -1,
    PD_LOW,
    PD_AVERAGE,
    PD_HIGH,
    NUM_UNIVERSE_PLANET_DENSITIES        //keep this last, the number of planet density options
};

namespace GG {
    GG_ENUM_MAP_BEGIN(PlanetDensity)
    GG_ENUM_MAP_INSERT(INVALID_PLANET_DENSITY)
    GG_ENUM_MAP_INSERT(PD_LOW)
    GG_ENUM_MAP_INSERT(PD_AVERAGE)
    GG_ENUM_MAP_INSERT(PD_HIGH)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(PlanetDensity)
GG_ENUM_STREAM_OUT(PlanetDensity)


/** types of starlane frequencies */
enum StarlaneFrequency {
    INVALID_STARLANE_FREQENCY = -1,
    LANES_NONE, 
    LANES_FEW, 
    LANES_SOME, 
    LANES_SEVERAL, 
    LANES_MANY, 
    LANES_VERY_MANY,
    NUM_STARLANE_FREQENCIES    // keep this last, the number of starlane frequency options
};

namespace GG {
    GG_ENUM_MAP_BEGIN(StarlaneFrequency)
    GG_ENUM_MAP_INSERT(INVALID_STARLANE_FREQENCY)
    GG_ENUM_MAP_INSERT(LANES_NONE)
    GG_ENUM_MAP_INSERT(LANES_FEW)
    GG_ENUM_MAP_INSERT(LANES_SOME)
    GG_ENUM_MAP_INSERT(LANES_SEVERAL)
    GG_ENUM_MAP_INSERT(LANES_MANY)
    GG_ENUM_MAP_INSERT(LANES_VERY_MANY)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(StarlaneFrequency)
GG_ENUM_STREAM_OUT(StarlaneFrequency)


/** types of starlane frequencies */
enum SpecialsFrequency {
    INVALID_SPECIALS_FREQENCY = -1,
    SPECIALS_NONE, 
    SPECIALS_RARE, 
    SPECIALS_UNCOMMON, 
    SPECIALS_COMMON, 
    NUM_SPECIALS_FREQENCIES    // keep this last, the number of specials frequency options
};

namespace GG {
    GG_ENUM_MAP_BEGIN(SpecialsFrequency)
    GG_ENUM_MAP_INSERT(INVALID_SPECIALS_FREQENCY)
    GG_ENUM_MAP_INSERT(SPECIALS_NONE)
    GG_ENUM_MAP_INSERT(SPECIALS_RARE)
    GG_ENUM_MAP_INSERT(SPECIALS_UNCOMMON)
    GG_ENUM_MAP_INSERT(SPECIALS_COMMON)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(SpecialsFrequency)
GG_ENUM_STREAM_OUT(SpecialsFrequency)


/** the types of diplomatic empire affiliations to a given empire*/
enum EmpireAffiliationType {
    INVALID_EMPIRE_AFFIL_TYPE = -1,
    AFFIL_SELF,     ///< not an affiliation as such; this indicates that the given empire, rather than its affiliates
    AFFIL_ENEMY,    ///< enemies of the given empire
    AFFIL_ALLY,     ///< allies of the given empire
    NUM_AFFIL_TYPES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(EmpireAffiliationType)
    GG_ENUM_MAP_INSERT(INVALID_EMPIRE_AFFIL_TYPE)
    GG_ENUM_MAP_INSERT(AFFIL_ENEMY)
    GG_ENUM_MAP_INSERT(AFFIL_SELF)
    GG_ENUM_MAP_INSERT(AFFIL_ALLY)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(EmpireAffiliationType)
GG_ENUM_STREAM_OUT(EmpireAffiliationType)

/** the types of items that can be unlocked for production by Effects */
enum UnlockableItemType {
    INVALID_UNLOCKABLE_ITEM_TYPE = -1,
    UIT_BUILDING,             ///< a kind of Building
    UIT_SHIP_COMPONENT,       ///< a kind of ShipComponent
    NUM_UNLOCKABLE_ITEM_TYPES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(UnlockableItemType)
    GG_ENUM_MAP_INSERT(INVALID_UNLOCKABLE_ITEM_TYPE)
    GG_ENUM_MAP_INSERT(UIT_BUILDING)
    GG_ENUM_MAP_INSERT(UIT_SHIP_COMPONENT)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(UnlockableItemType)
GG_ENUM_STREAM_OUT(UnlockableItemType)

/** General classification for purpose and function of techs, and allowed place in tech prerequisite tree */
enum TechType {
    INVALID_TECH_TYPE = -1,
    TT_THEORY,      // Theory: does nothing itself, but is prerequisite for applications and refinements
    TT_APPLICATION, // Application: has effects that do things, or may unlock something such as a building that does things
    TT_REFINEMENT,  // Refinement: does nothing itself, but if researched, may alter the effects of something else
    NUM_TECH_TYPES
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
    BT_ORBITAL,              ///< an Orbital object is being built
    NUM_BUILD_TYPES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(BuildType)
    GG_ENUM_MAP_INSERT(INVALID_BUILD_TYPE)
    GG_ENUM_MAP_INSERT(BT_NOT_BUILDING)
    GG_ENUM_MAP_INSERT(BT_BUILDING)
    GG_ENUM_MAP_INSERT(BT_SHIP)
    GG_ENUM_MAP_INSERT(BT_ORBITAL)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(BuildType)
GG_ENUM_STREAM_OUT(BuildType)

enum ResourceType {
    INVALID_RESOURCE_TYPE = -1,
    RE_FOOD,
    RE_MINERALS,
    RE_INDUSTRY,
    RE_TRADE,
    RE_RESEARCH,
    NUM_RESOURCE_TYPES
};

namespace GG {
    GG_ENUM_MAP_BEGIN(ResourceType)
    GG_ENUM_MAP_INSERT(INVALID_RESOURCE_TYPE)
    GG_ENUM_MAP_INSERT(RE_FOOD)
    GG_ENUM_MAP_INSERT(RE_MINERALS)
    GG_ENUM_MAP_INSERT(RE_INDUSTRY)
    GG_ENUM_MAP_INSERT(RE_TRADE)
    GG_ENUM_MAP_INSERT(RE_RESEARCH)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(ResourceType)
GG_ENUM_STREAM_OUT(ResourceType)

/** Returns the equivalent focus type for the given meter; if no such focus exists, returns INVALID_METER_TYPE. */
MeterType FocusToMeter(FocusType type);

/** Returns the equivalent meter type for the given focus; if no such focus exists, returns INVALID_FOCUS_TYPE. */
FocusType MeterToFocus(MeterType type);

/** the value used for the encoding empire when all data are desired, not just those visible to one empire */
enum EncodingVisbility {
    ALL_EMPIRES = -1
};

/** Possible results of an UniverseObject being captured by other empires, or an object's containing
    UniverseObject being captured, or the location of a Production Queue Build Item being conquered, 
    or the result of other future events, such as spy activity... */
enum CaptureResult {
    INVALID_CAPTURE_RESULT = -1,
    CR_CAPTURE,    // object has ownership by original empire(s) removed, and conquering empire added
    CR_DESTROY,    // object is destroyed
    CR_RETAIN,     // object ownership unchanged: original empire(s) still own object
    CR_SHARE       // object has ownership by conquering empire added, while still retaining ownership by original empire(s)
};

#endif // _Enums_h_
