// -*- C++ -*-
#ifndef _Enums_h_
#define _Enums_h_

#include "GGEnum.h"

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
    ENUM_MAP_BEGIN(UniverseObjectType)
    ENUM_MAP_INSERT(INVALID_UNIVERSE_OBJECT_TYPE)
    ENUM_MAP_INSERT(OBJ_BUILDING)
    ENUM_MAP_INSERT(OBJ_SHIP)
    ENUM_MAP_INSERT(OBJ_FLEET)
    ENUM_MAP_INSERT(OBJ_PLANET)
    ENUM_MAP_INSERT(OBJ_POP_CENTER)
    ENUM_MAP_INSERT(OBJ_PROD_CENTER)
    ENUM_MAP_INSERT(OBJ_SYSTEM)
    ENUM_MAP_END
}
ENUM_STREAM_IN(UniverseObjectType)
ENUM_STREAM_OUT(UniverseObjectType)

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
    ENUM_MAP_BEGIN(StarType)
    ENUM_MAP_INSERT(INVALID_STAR_TYPE)
    ENUM_MAP_INSERT(STAR_BLUE)
    ENUM_MAP_INSERT(STAR_WHITE)
    ENUM_MAP_INSERT(STAR_YELLOW)
    ENUM_MAP_INSERT(STAR_ORANGE)
    ENUM_MAP_INSERT(STAR_RED)
    ENUM_MAP_INSERT(STAR_NEUTRON)
    ENUM_MAP_INSERT(STAR_BLACK)
    ENUM_MAP_END
}
ENUM_STREAM_IN(StarType)
ENUM_STREAM_OUT(StarType)

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
    ENUM_MAP_BEGIN(PlanetType)
    ENUM_MAP_INSERT(INVALID_PLANET_TYPE)
    ENUM_MAP_INSERT(PT_SWAMP)
    ENUM_MAP_INSERT(PT_TOXIC)
    ENUM_MAP_INSERT(PT_INFERNO)
    ENUM_MAP_INSERT(PT_RADIATED)
    ENUM_MAP_INSERT(PT_BARREN)
    ENUM_MAP_INSERT(PT_TUNDRA)
    ENUM_MAP_INSERT(PT_DESERT)
    ENUM_MAP_INSERT(PT_TERRAN)
    ENUM_MAP_INSERT(PT_OCEAN)
    ENUM_MAP_INSERT(PT_GAIA)
    ENUM_MAP_INSERT(PT_ASTEROIDS)
    ENUM_MAP_INSERT(PT_GASGIANT)
    ENUM_MAP_END
}
ENUM_STREAM_IN(PlanetType)
ENUM_STREAM_OUT(PlanetType)


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
    ENUM_MAP_BEGIN(PlanetSize)
    ENUM_MAP_INSERT(INVALID_PLANET_SIZE)
    ENUM_MAP_INSERT(SZ_NOWORLD)
    ENUM_MAP_INSERT(SZ_TINY)
    ENUM_MAP_INSERT(SZ_SMALL)
    ENUM_MAP_INSERT(SZ_MEDIUM)
    ENUM_MAP_INSERT(SZ_LARGE)
    ENUM_MAP_INSERT(SZ_HUGE)
    ENUM_MAP_INSERT(SZ_ASTEROIDS)
    ENUM_MAP_INSERT(SZ_GASGIANT)
    ENUM_MAP_END
}
ENUM_STREAM_IN(PlanetSize)
ENUM_STREAM_OUT(PlanetSize)


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
    ENUM_MAP_BEGIN(PlanetEnvironment)
    ENUM_MAP_INSERT(INVALID_PLANET_ENVIRONMENT)
    ENUM_MAP_INSERT(PE_UNINHABITABLE)
    ENUM_MAP_INSERT(PE_TERRIBLE)
    ENUM_MAP_INSERT(PE_ADEQUATE)
    ENUM_MAP_INSERT(PE_SUPERB)
    ENUM_MAP_INSERT(PE_OPTIMAL)
    ENUM_MAP_END
}
ENUM_STREAM_IN(PlanetEnvironment)
ENUM_STREAM_OUT(PlanetEnvironment)


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
    ENUM_MAP_BEGIN(FocusType)
    ENUM_MAP_INSERT(INVALID_FOCUS_TYPE)
    ENUM_MAP_INSERT(FOCUS_UNKNOWN)
    ENUM_MAP_INSERT(FOCUS_BALANCED)
    ENUM_MAP_INSERT(FOCUS_FARMING)
    ENUM_MAP_INSERT(FOCUS_INDUSTRY)
    ENUM_MAP_INSERT(FOCUS_MINING)
    ENUM_MAP_INSERT(FOCUS_RESEARCH)
    ENUM_MAP_END
}
ENUM_STREAM_IN(FocusType)
ENUM_STREAM_OUT(FocusType)


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
    ENUM_MAP_BEGIN(MeterType)
    ENUM_MAP_INSERT(INVALID_METER_TYPE)
    ENUM_MAP_INSERT(METER_FARMING)
    ENUM_MAP_INSERT(METER_POPULATION)
    ENUM_MAP_INSERT(METER_INDUSTRY)
    ENUM_MAP_INSERT(METER_RESEARCH)
    ENUM_MAP_INSERT(METER_TRADE)
    ENUM_MAP_INSERT(METER_MINING)
    ENUM_MAP_INSERT(METER_CONSTRUCTION)
    ENUM_MAP_INSERT(METER_HEALTH)
    ENUM_MAP_END
}
ENUM_STREAM_IN(MeterType)
ENUM_STREAM_OUT(MeterType)

/** the types of diplomatic empire affiliations to a given empire*/
enum EmpireAffiliationType {
    INVALID_EMPIRE_AFFIL_TYPE = -1,
    AFFIL_SELF,     ///< not an affiliation as such; this indicates that the given empire, rather than its affiliates
    AFFIL_ENEMY,    ///< allies of the given empire
    AFFIL_ALLY,     ///< enamies of the given empire
    NUM_AFFIL_TYPES
};

namespace GG {
    ENUM_MAP_BEGIN(EmpireAffiliationType)
    ENUM_MAP_INSERT(INVALID_EMPIRE_AFFIL_TYPE)
    ENUM_MAP_INSERT(AFFIL_ENEMY)
    ENUM_MAP_INSERT(AFFIL_SELF)
    ENUM_MAP_INSERT(AFFIL_ALLY)
    ENUM_MAP_END
}
ENUM_STREAM_IN(EmpireAffiliationType)
ENUM_STREAM_OUT(EmpireAffiliationType)

/** the types of items that can be unlocked for production by Effects */
enum UnlockableItemType {
    INVALID_UNLOCKABLE_ITEM_TYPE = -1,
    UIT_BUILDING,             ///< a kind of Building
    UIT_SHIP_COMPONENT,       ///< a kind of ShipComponent
    NUM_UNLOCKABLE_ITEM_TYPES
};

namespace GG {
    ENUM_MAP_BEGIN(UnlockableItemType)
    ENUM_MAP_INSERT(INVALID_UNLOCKABLE_ITEM_TYPE)
    ENUM_MAP_INSERT(UIT_BUILDING)
    ENUM_MAP_INSERT(UIT_SHIP_COMPONENT)
    ENUM_MAP_END
}
ENUM_STREAM_IN(UnlockableItemType)
ENUM_STREAM_OUT(UnlockableItemType)

/** the types of Techs in FreeOrion */
enum TechType {
    INVALID_TECH_TYPE = -1,
    TT_THEORY,           ///< a Theory Tech
    TT_APPLICATION,      ///< an Application Tech
    TT_REFINEMENT,       ///< a Refinment Tech
    NUM_TECH_TYPES
};

namespace GG {
    ENUM_MAP_BEGIN(TechType)
    ENUM_MAP_INSERT(INVALID_TECH_TYPE)
    ENUM_MAP_INSERT(TT_THEORY)
    ENUM_MAP_INSERT(TT_APPLICATION)
    ENUM_MAP_INSERT(TT_REFINEMENT)
    ENUM_MAP_END
}
ENUM_STREAM_IN(TechType)
ENUM_STREAM_OUT(TechType)

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
    ENUM_MAP_BEGIN(BuildType)
    ENUM_MAP_INSERT(INVALID_BUILD_TYPE)
    ENUM_MAP_INSERT(BT_NOT_BUILDING)
    ENUM_MAP_INSERT(BT_BUILDING)
    ENUM_MAP_INSERT(BT_SHIP)
    ENUM_MAP_INSERT(BT_ORBITAL)
    ENUM_MAP_END
}
ENUM_STREAM_IN(BuildType)
ENUM_STREAM_OUT(BuildType)

/** The types of empire stockpiles. */
enum StockpileType {
    INVALID_STOCKPILE_TYPE = -1,
    ST_FOOD,         ///< food stockpile
    ST_MINERAL,      ///< mineral stockpile
    ST_TRADE,        ///< money stockpile
    NUM_STOCKPILE_TYPES
};

namespace GG {
    ENUM_MAP_BEGIN(StockpileType)
    ENUM_MAP_INSERT(INVALID_STOCKPILE_TYPE)
    ENUM_MAP_INSERT(ST_FOOD)
    ENUM_MAP_INSERT(ST_MINERAL)
    ENUM_MAP_INSERT(ST_TRADE)
    ENUM_MAP_END
}
ENUM_STREAM_IN(StockpileType)
ENUM_STREAM_OUT(StockpileType)

inline std::pair<std::string, std::string> EnumsRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _Enums_h_
