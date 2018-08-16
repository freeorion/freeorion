#ifndef _CombatTargetting_h_
#define _CombatTargetting_h_

#include "../universe/UniverseObject.h"
#include "../universe/Condition.h"

//  Big Picture:
//    Hunters have a precision and preferred types of prey. This means that
//    hunters with a higher precisions are more likely to hit the preferred
//    prey in combat.
//
//  Design Choices:
//    Combat Interpretation - Precision means number of rolls to find a preferred target
//       Precision 1 or NoPreference means the old behavior (doesnt change chances to hit certain prey)
//
//    Content Setup - in CombatTargetting.cpp
//       Flak (SR_WEAPON_0_1) - Precision 3 hunting space boats
//       Bombers (FT_HANGAR_3) - Precision 2 hunting space ship
//       Interceptors (FT_HANGAR_1) - Precision 2 hunting space boats
//       Everything else - no preferences
//    Attacking planets should be reconsidered (E.g. let ships prefer ships with precision 2); also glass cannon
//       
//    Content Setup Language
//       Hardcoded TriggerConditions and PreyTypes in CombatTargetting.cpp
//       Alternatives
//         - specify TriggerCondition as FOCS condition on weapon system / hangar   
//         - specify TriggerCondition and PreyType as ~tags in FOCS on weapons/hangar and resp. ShipDesigns
//    The content may actually not be hardcoded, so this needs to change before merge.

namespace Targetting {
    enum PreyType { NoPreference = 7, AllPrey = 7, PlanetPrey = 1, ShipPrey = 2, BoatPrey = 4 };
    enum class HunterType { Unknown, Planet, Boat, Ship }; 
    typedef PreyType                           Prey;
    typedef std::shared_ptr<UniverseObject>  Target;
    typedef Condition::ConditionBase* TriggerCondition;
    typedef int                           Precision;

    bool isPreferredTarget(TriggerCondition condition, Target target);

    Precision findPrecision(HunterType hunting, const std::string& part_name);
    Prey findTargetTypes(const std::string& part_name);
    TriggerCondition findPreferredTargets(const std::string& part_name);
    TriggerCondition preyAsTriggerCondition(Prey prey);
}

#endif // _CombatTargetting_h_
