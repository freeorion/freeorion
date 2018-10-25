#ifndef _CombatTargetting_h_
#define _CombatTargetting_h_

#include "../universe/Condition.h"

namespace Targetting {
    enum PreyType {
        NoPreference = 7,
        AllPrey = 7,
        PlanetPrey = 1,
        ShipPrey = 2,
        BoatPrey = 4
    };

    /* gives access to static conditions */
    const ::Condition::ConditionBase* PreyAsTriggerCondition(PreyType prey);
}

#endif // _CombatTargetting_h_
