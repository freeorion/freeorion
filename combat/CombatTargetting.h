#ifndef _CombatTargetting_h_
#define _CombatTargetting_h_

#include "../universe/UniverseObject.h"

// possible target, prey
// hit a mark
// wanted, sight, prey, target, preferredX, sought, aim, triggerCondition
namespace Targetting {
    enum PreyType { NoPreference = 7, AllPrey = 7, PlanetPrey = 1, ShipPrey = 2, BoatPrey = 4 };
    enum class HunterType { Unknown, Planet, Boat, Ship }; 
    typedef PreyType                           Prey;
    typedef std::shared_ptr<UniverseObject>  Target;
    typedef PreyType               TriggerCondition;
    typedef int                           Precision;

    bool isPreferredTarget(TriggerCondition condition, Target target);

    Precision findPrecision(HunterType hunting, const std::string& part_name);
    Prey findTargetTypes(const std::string& part_name);
    TriggerCondition findPreferredTargets(const std::string& part_name);
}

#endif // _CombatTargetting_h_
