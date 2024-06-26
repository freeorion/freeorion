#ifndef _Enums_h_
#define _Enums_h_


#include "../util/Enum.h"
#include "../util/Export.h"

#include <array>
#include <utility>

/** types of diplomatic empire affiliations to another empire*/
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
)


#endif
