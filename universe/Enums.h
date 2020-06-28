#ifndef _Enums_h_
#define _Enums_h_


#include <iostream>
#include <string>
#include <vector>
#include <GG/Enum.h>
#include "../util/Export.h"


/* the various major subclasses of UniverseObject */
GG_ENUM(UniverseObjectType,
    INVALID_UNIVERSE_OBJECT_TYPE = -1,
    OBJ_BUILDING,
    OBJ_SHIP,
    OBJ_FLEET,
    OBJ_PLANET,
    OBJ_POP_CENTER,
    OBJ_PROD_CENTER,
    OBJ_SYSTEM,
    OBJ_FIELD,
    OBJ_FIGHTER,
    NUM_OBJ_TYPES
)

/** types of stars */
GG_ENUM(StarType,
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
)

/** types of planets */
GG_ENUM(PlanetType,
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
)

/** sizes of planets */
GG_ENUM(PlanetSize,
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
)

/** environmental suitability of planets for a particular race */
GG_ENUM(PlanetEnvironment,
    INVALID_PLANET_ENVIRONMENT = -1,
    PE_UNINHABITABLE,
    PE_HOSTILE,
    PE_POOR,
    PE_ADEQUATE,
    PE_GOOD,
    NUM_PLANET_ENVIRONMENTS
)

/** Types for Meters
  * Only active paired meters should lie between METER_POPULATION and METER_TROOPS
  * (See: UniverseObject::ResetPairedActiveMeters())
  */
GG_ENUM(MeterType,
    INVALID_METER_TYPE = -1,
    METER_TARGET_POPULATION,
    METER_TARGET_INDUSTRY,
    METER_TARGET_RESEARCH,
    METER_TARGET_INFLUENCE,
    METER_TARGET_CONSTRUCTION,
    METER_TARGET_HAPPINESS,

    METER_MAX_CAPACITY,
    METER_MAX_SECONDARY_STAT,

    METER_MAX_FUEL,
    METER_MAX_SHIELD,
    METER_MAX_STRUCTURE,
    METER_MAX_DEFENSE,
    METER_MAX_SUPPLY,
    METER_MAX_STOCKPILE,
    METER_MAX_TROOPS,

    METER_POPULATION,
    METER_INDUSTRY,
    METER_RESEARCH,
    METER_INFLUENCE,
    METER_CONSTRUCTION,
    METER_HAPPINESS,

    METER_CAPACITY,
    METER_SECONDARY_STAT,

    METER_FUEL,
    METER_SHIELD,
    METER_STRUCTURE,
    METER_DEFENSE,
    METER_SUPPLY,
    METER_STOCKPILE,
    METER_TROOPS,

    METER_REBEL_TROOPS,
    METER_SIZE,
    METER_STEALTH,
    METER_DETECTION,
    METER_SPEED,

    NUM_METER_TYPES
)

/** types of diplomatic empire affiliations to another empire*/
GG_ENUM(EmpireAffiliationType,
    INVALID_EMPIRE_AFFIL_TYPE = -1,
    AFFIL_SELF,     ///< the given empire iteslf
    AFFIL_ENEMY,    ///< enemies of the given empire
    AFFIL_PEACE,    ///< empires at peace with the given empire
    AFFIL_ALLY,     ///< allies of the given empire
    AFFIL_ANY,      ///< any empire
    AFFIL_NONE,     ///< no empire
    AFFIL_CAN_SEE,  ///< special case enum used to specify empires that can detect particular objects, for use in effects or conditions
    AFFIL_HUMAN,    ///< empire controlled by a human player
    NUM_AFFIL_TYPES ///< keep last, the number of affiliation types
)

/** diplomatic statuses */
GG_ENUM(DiplomaticStatus,
    INVALID_DIPLOMATIC_STATUS = -1,
    DIPLO_WAR,
    DIPLO_PEACE,
    DIPLO_ALLIED,
    NUM_DIPLO_STATUSES
)

/** Research status of techs, relating to whether they have been or can be researched */
GG_ENUM(TechStatus,
    INVALID_TECH_STATUS = -1,
    TS_UNRESEARCHABLE,          ///< never researchable, or has no researched prerequisites
    TS_HAS_RESEARCHED_PREREQ,   ///< has at least one researched, and at least one unreserached, prerequisite
    TS_RESEARCHABLE,            ///< all prerequisites researched
    TS_COMPLETE,                ///< has been researched
    NUM_TECH_STATUSES
)

/** The general type of production being done at a ProdCenter.  Within each
  * valid type, a specific kind of item is being built, e.g. under BT_BUILDING
  * a kind of building called "SuperFarm" might be built. */
GG_ENUM(BuildType,
    INVALID_BUILD_TYPE = -1,
    BT_NOT_BUILDING,        ///< no building is taking place
    BT_BUILDING,            ///< a Building object is being produced
    BT_SHIP,                ///< a Ship object is being produced
    BT_PROJECT,             ///< a project may generate effects while on the queue, may or may not ever complete, and does not result in a ship or building being produced
    BT_STOCKPILE,
    NUM_BUILD_TYPES
)

/** Types of resources that planets can produce */
GG_ENUM(ResourceType,
    INVALID_RESOURCE_TYPE = -1,
    RE_INDUSTRY,
    RE_INFLUENCE,
    RE_RESEARCH,
    RE_STOCKPILE,
    NUM_RESOURCE_TYPES
)

/* Types of slots in hulls.  Parts may be restricted to only certain slot types */
GG_ENUM(ShipSlotType,
    INVALID_SHIP_SLOT_TYPE = -1,
    SL_EXTERNAL,            ///< external slots.  more easily damaged
    SL_INTERNAL,            ///< internal slots.  more protected, fewer in number
    SL_CORE,
    NUM_SHIP_SLOT_TYPES
)


/** Returns the equivalent meter type for the given resource type; if no such
  * meter type exists, returns INVALID_METER_TYPE. */
FO_COMMON_API MeterType ResourceToMeter(ResourceType type);
FO_COMMON_API MeterType ResourceToTargetMeter(ResourceType type);

/** Returns the equivalent resource type for the given meter type; if no such
  * resource type exists, returns INVALID_RESOURCE_TYPE. */
FO_COMMON_API ResourceType MeterToResource(MeterType type);

/** Returns mapping from active to target or max meter types that correspond.
  * eg. METER_RESEARCH -> METER_TARGET_RESEARCH */
FO_COMMON_API const std::map<MeterType, MeterType>& AssociatedMeterTypes();

/** Returns the target or max meter type that is associated with the given
  * active meter type.  If no associated meter type exists, INVALID_METER_TYPE
  * is returned. */
FO_COMMON_API MeterType AssociatedMeterType(MeterType meter_type);


/** degrees of visibility an Empire or UniverseObject can have for an
  * UniverseObject.  determines how much information the empire
  * gets about the (non)visible object. */
GG_ENUM(Visibility,
    INVALID_VISIBILITY = -1,
    VIS_NO_VISIBILITY,
    VIS_BASIC_VISIBILITY,
    VIS_PARTIAL_VISIBILITY,
    VIS_FULL_VISIBILITY,
    NUM_VISIBILITIES
)


/** Possible results of an UniverseObject being captured by other empires, or an
  * object's containing UniverseObject being captured, or the location of a
  * Production Queue Build Item being conquered, or the result of other future
  * events, such as spy activity... */
GG_ENUM(CaptureResult,
    INVALID_CAPTURE_RESULT = -1,
    CR_CAPTURE,    // object has ownership by original empire(s) removed, and conquering empire added
    CR_DESTROY,    // object is destroyed
    CR_RETAIN      // object ownership unchanged: original empire(s) still own object
)

/** Types of in-game things that might contain an EffectsGroup, or "cause" effects to occur */
GG_ENUM(EffectsCauseType,
    INVALID_EFFECTS_GROUP_CAUSE_TYPE = -1,
    ECT_UNKNOWN_CAUSE,
    ECT_INHERENT,
    ECT_TECH,
    ECT_BUILDING,
    ECT_FIELD,
    ECT_SPECIAL,
    ECT_SPECIES,
    ECT_SHIP_PART,
    ECT_SHIP_HULL,
    ECT_POLICY
)

/** Used for tracking what moderator action is set */
GG_ENUM(ModeratorActionSetting,
    MAS_NoAction,
    MAS_Destroy,
    MAS_SetOwner,
    MAS_AddStarlane,
    MAS_RemoveStarlane,
    MAS_CreateSystem,
    MAS_CreatePlanet
)


#endif // _Enums_h_
