#ifndef _CombatTargetting_h_
#define _CombatTargetting_h_

#include "../universe/UniverseObject.h"
#include "../universe/Condition.h"
#include <vector>

//  Big Picture:
//    Hunters have a precision and preferred types of prey. This means that
//    hunters with a higher precisions are more likely to hit the preferred
//    prey in combat.
//
//  Design Choices:
//    Combat Interpretation - Precision means number of rolls to find a preferred target
//       Precision 1 or NoPreference means the old behavior (doesnt change chances to hit certain prey)
//       Also if there are multiple sources defining preferred prey, precision is used as a weight when randomly choosing a target.
//    ShipParts are responsible for condition ownership. For other conditions (i.e. planets), CombatTargetting is responsible.
//
//  Content Wiring:
//    For planets: these shoot with a precision of 3 at space ships
//    For space ships, the respective weapon part contributes the TriggerConditions to the shot
//                     also detection parts work as "battle scanners" and contribute their TriggerCondition to the shot
//    For space boats, the hangars part contributes the TriggerConditions to the shot
//
//    Content Setup Language (FOCS)
//       Precision                        'precision'     value in FOCS script for ship_parts
//       const ::Condition::ConditionBase 'priorityTargets' condition in FOCS script for ship_parts
//
//    Content Setup (examples) - in FOCS scripts
//       Flak (SR_WEAPON_0_1)  - hunting space boats
//       Bombers (FT_HANGAR_3) - mostly hunting space ships
//       Fighters (FT_HANGAR_2) - slightly preferring to shoot at ships containing anti-boat guns (SR_WEAPON_0_1)
//       Interceptors (FT_HANGAR_1) - mostly hunting space boats
//    Attacking planets have their precision and preference hardwired (look for DEF_DEFENSE in CombatSystem.cpp)



namespace Targetting {
    enum PreyType {
        NoPreference = 7,
        AllPrey = 7,
        PlanetPrey = 1,
        ShipPrey = 2,
        BoatPrey = 4
    };

    typedef std::shared_ptr<UniverseObject>                   Target;
    typedef int                                               Precision;
    struct TriggerConditions {
        // Currently ShipPart takes ownership of trigger condition using a unique_ptr - should be moved to some ConditionManager
        std::vector<const ::Condition::ConditionBase *> conditions;
        std::vector<int> weights;

        TriggerConditions()
        {}

        TriggerConditions(const std::vector<const ::Condition::ConditionBase *>&& conditions_, const std::vector<int>&& weights_) :
            conditions(std::move(conditions_)),
            weights(std::move(weights_))
        {}
        TriggerConditions(const ::Condition::ConditionBase* condition_, const int weight_) :
            conditions({condition_}),
            weights({weight_})
        {}

    };

    bool IsPriorityTarget(const ::Condition::ConditionBase& condition, Target target);
    bool IsPriorityTarget(const TriggerConditions& condition, Target target);

    Precision FindPrecision(const TriggerConditions& conditions);

    /* returns nullptr if no preference */
    const ::Condition::ConditionBase* FindPriorityTargets(const std::string& part_name);

    /* gives access to static conditions */
    const ::Condition::ConditionBase* PreyAsTriggerCondition(PreyType prey);

    /* returns a new TriggerCondition with the priorities and preferences of both given TriggerConditions */ 
    TriggerConditions Combine(const TriggerConditions& one, const TriggerConditions& another);

    /* if the given condition exists, a copy with the appended condition gets returned. */
    const TriggerConditions Combine(const TriggerConditions& one, const ::Condition::ConditionBase* another, int weight);
}

#endif // _CombatTargetting_h_
