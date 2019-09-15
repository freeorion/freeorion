#ifndef _Enums_h_
#define _Enums_h_


#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "../util/Enum.h"
#include "../util/Export.h"


/** Types for Meters
  * Only active paired meters should lie between MeterType::METER_POPULATION and MeterType::METER_TROOPS
  * (See: UniverseObject::ResetPairedActiveMeters())
  */
FO_ENUM(
    (MeterType),
    ((INVALID_METER_TYPE, -1))
    ((METER_TARGET_POPULATION))
    ((METER_TARGET_INDUSTRY))
    ((METER_TARGET_RESEARCH))
    ((METER_TARGET_INFLUENCE))
    ((METER_TARGET_CONSTRUCTION))
    ((METER_TARGET_HAPPINESS))

    ((METER_MAX_CAPACITY))
    ((METER_MAX_SECONDARY_STAT))

    ((METER_MAX_FUEL))
    ((METER_MAX_SHIELD))
    ((METER_MAX_STRUCTURE))
    ((METER_MAX_DEFENSE))
    ((METER_MAX_SUPPLY))
    ((METER_MAX_STOCKPILE))
    ((METER_MAX_TROOPS))

    ((METER_POPULATION))
    ((METER_INDUSTRY))
    ((METER_RESEARCH))
    ((METER_INFLUENCE))
    ((METER_CONSTRUCTION))
    ((METER_HAPPINESS))

    ((METER_CAPACITY))
    ((METER_SECONDARY_STAT))

    ((METER_FUEL))
    ((METER_SHIELD))
    ((METER_STRUCTURE))
    ((METER_DEFENSE))
    ((METER_SUPPLY))
    ((METER_STOCKPILE))
    ((METER_TROOPS))

    ((METER_REBEL_TROOPS))
    ((METER_SIZE))
    ((METER_STEALTH))
    ((METER_DETECTION))
    ((METER_SPEED))

    ((NUM_METER_TYPES))
)

/** types of empire affiliations to another empire*/
FO_ENUM(
    (EmpireAffiliationType),
    ((INVALID_EMPIRE_AFFIL_TYPE, -1))
    ((AFFIL_SELF))      ///< the given empire iteslf
    ((AFFIL_ENEMY))     ///< enemies of the given empire
    ((AFFIL_PEACE))     ///< empires at peace with the given empire
    ((AFFIL_ALLY))      ///< allies of the given empire
    ((AFFIL_ANY))       ///< any empire
    ((AFFIL_NONE))      ///< no empire
    ((AFFIL_CAN_SEE))   ///< special case enum used to specify empires that can detect particular objects, for use in effects or conditions
    ((AFFIL_HUMAN))     ///< empire controlled by a human player

    ((NUM_AFFIL_TYPES)) ///< keep last, the number of affiliation types

/** diplomatic statuses between empires*/
FO_ENUM(
    (DiplomaticStatus),
    ((INVALID_DIPLOMATIC_STATUS, -1))
    ((DIPLO_WAR))
    ((DIPLO_PEACE))
    ((DIPLO_ALLIED))
    ((DISP_SHARED_SUPPLY))
    ((NUM_DIPLO_STATUSES))
)


/** Returns mapping from active to target or max meter types that correspond.
  * eg. MeterType::METER_RESEARCH -> MeterType::METER_TARGET_RESEARCH */
FO_COMMON_API const std::map<MeterType, MeterType>& AssociatedMeterTypes();

/** Returns the target or max meter type that is associated with the given
  * active meter type.  If no associated meter type exists, MeterType::INVALID_METER_TYPE
  * is returned. */
FO_COMMON_API MeterType AssociatedMeterType(MeterType meter_type);


#endif
