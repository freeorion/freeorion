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
//       Also if there are multiple sources defining preferred prey, it is used as a weight when randomly choosing a target.
//       TBD: consider sources besides the curren weapon/hangar part
//
//    Content Setup (examples) - in FOCS scripts
//       Flak (SR_WEAPON_0_1)  - hunting space boats
//       Bombers (FT_HANGAR_3) - mostly hunting space ships
//       Interceptors (FT_HANGAR_1) - mostly hunting space boats
//    Attacking planets should be reconsidered (E.g. let ships prefer ships with precision 2); also glass cannon
//       
//    Content Setup Language
//       Precion          'precision'     value in FOCS script for ship_parts
//       TriggerCondition 'preferredPrey' condition in FOCS script for ship_parts
//    The content may actually not be hardcoded, so this needs to change before merge.

namespace Targetting {
    enum PreyType {
        NoPreference = 7,
        AllPrey = 7,
        PlanetPrey = 1,
        ShipPrey = 2,
        BoatPrey = 4
    };

    typedef std::shared_ptr<UniverseObject>                   Target;
    typedef const ::Condition::ConditionBase                  TriggerCondition;
    typedef std::shared_ptr<const ::Condition::ConditionBase> TriggerConditionP;
    typedef int                                               Precision;
    struct TriggerConditions {
        // Currently ShipPart takes ownership of TriggerCondition using a unique_ptr
        // TriggerConditions takes ownership of conditions so it could become part of ShipDesign, Parts etc..
        // If ownership is not necessary we still not some kind of reference type, where shared_ptr seems to be a lesser evil
        std::vector<std::shared_ptr<TriggerCondition>> conditions;
        std::vector<int> weights;

        TriggerConditions()
        {}

        TriggerConditions(const std::vector<TriggerConditionP>&& conditions_, const std::vector<int>&& weights_) :
            conditions(std::move(conditions_)),
            weights(std::move(weights_))
        {}
        /*
        TriggerConditions(const Targetting::TriggerConditionP&& condition_, const int weight_) :
            conditions({condition_}),
            weights({weight_})
        {}
        */
        TriggerConditions(const TriggerConditionP& condition_, const int weight_) :
            conditions({condition_}),
            weights({weight_})
        {}


        TriggerConditions(const TriggerCondition* condition_, const int weight_) :
            conditions({std::shared_ptr<TriggerCondition>(condition_)}),
            weights({weight_})
        {}

    };

    bool IsPreferredTarget(const TriggerCondition& condition, Target target);
    bool IsPreferredTarget(const TriggerConditions& condition, Target target);

    Precision FindPrecision(const TriggerConditions& conditions);

    /* returns nullptr if no preference */
    TriggerCondition* FindPreferredTargets(const std::string& part_name);
    std::unique_ptr<TriggerCondition> PreyAsTriggerCondition(PreyType prey);
    TriggerConditions PreyAsTriggerConditions(PreyType prey);
    TriggerConditions Combine(const TriggerConditions& one, const TriggerConditions& another);
    /* if the given condition exists, a copy with the appended condition gets returned. */
    const TriggerConditionP WrapCondition(TriggerCondition* condition);
    const TriggerConditions Combine(const TriggerConditions& one, const TriggerConditionP&& another, int weight);
    //     const TriggerConditions Combine(const TriggerConditions& one, std::shared_ptr<const TriggerCondition>&& another, int weight);
}

#endif // _CombatTargetting_h_
