#include "CombatTargetting.h"

#include "../universe/Condition.h"
#include "../universe/Enums.h"
#include "../universe/Planet.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/Fighter.h"
#include "../universe/ValueRef.h"

#include "../util/Logger.h"
#include "../util/Random.h"

bool Targetting::isPreferredTarget(Targetting::TriggerConditions conditions,
                                   Targetting::Target target) {
    const Precision highestWeight = Targetting::findPrecision(conditions);
    for (long unsigned int i = 0; i < conditions.conditions.size(); i++) {
        const auto& condition = conditions.conditions[i];
        if (!condition) {
            DebugLogger() << "No preferences. Target is perfect." ;
            return true;
        }

        if (Targetting::isPreferredTarget(condition, target)) {
            const int upper = std::round(highestWeight * 1.5f); // HEURISTIC
            const int improbability = RandInt(0, upper);
            const int luck = conditions.weights[i];
            DebugLogger() << "Prioritized locking attempt" <<  [&improbability, &luck, &upper](){
                                                                   std::stringstream ss;
                                                                   ss << "( " << luck << "/" << upper << " ?>? " << improbability << "/" << upper << ")";
                                                                   return ss.str();
                                                               }();
            if (luck > improbability) {
                DebugLogger() << "Locked in target.";
                return true;
            } else {
                DebugLogger() << "Couldnt acquire lock." ;
            }
        }
    }
    return false;
}

bool Targetting::isPreferredTarget(Targetting::TriggerCondition condition,
                                   Targetting::Target target) {
    if (!condition) {
            DebugLogger() << "No preferences. Target is perfect." ;
            return true;
    }

    DebugLogger() << "Evaluate preferred prey condition" << condition->Description();
    bool is_preferred = condition->Eval(target);
    auto obj_type = target->ObjectType();
    if ( is_preferred ) {
        DebugLogger() << "This " << obj_type << " ship should die! " ;
    } else {
        DebugLogger() << "Don't care about this " << obj_type << " - reroll if possible. " ;
    }
    return is_preferred;
}
Targetting::TriggerCondition Targetting::preyAsTriggerCondition(const Targetting::Prey prey) {
        switch( prey ) {
        case Targetting::NoPreference:
                return std::make_shared<Condition::All>();
                break;
        default:
                // error
                return nullptr;
        }
}
Targetting::TriggerConditions Targetting::preyAsTriggerConditions(const Targetting::Prey prey) {
    return Targetting::TriggerConditions(Targetting::preyAsTriggerCondition(prey), 1);
}
Targetting::Prey Targetting::findTargetTypes(const std::string& part_name) {
        if (part_name.rfind("FT_", 0) == 0) {
            return BoatPrey;
        }
        if (part_name.rfind("SR_", 0) == 0) {
            return ShipPrey;
        }
        // shouldnt happen
        return AllPrey;
}

Targetting::TriggerCondition Targetting::findPreferredTargets(const std::string& part_name) {
    const PartType* part = GetPartType(part_name);
    return part->PreferredPrey();
}

Targetting::Precision Targetting::findPrecision(const Targetting::TriggerConditions& conditions) {
    //    std::vector<Precision>::iterator
        auto result = std::max_element(conditions.weights.begin(), conditions.weights.end());
    return *result;
}

Targetting::Precision Targetting::findPrecision(const Targetting::HunterType hunting_, const std::string& part_name) {
    Targetting::HunterType hunting(hunting_);
    if ( Targetting::HunterType::Unknown == hunting_ ) {
        if ( part_name.rfind("FT_",0) == 0 ) {
            hunting = Targetting::HunterType::Boat;
        } else if ( part_name.rfind("SR_",0) == 0 ) {
            hunting = Targetting::HunterType::Ship;
        }
    }
    switch ( hunting ) {
    case Targetting::HunterType::Boat:
            return 2;
    case Targetting::HunterType::Ship:
            if ( (part_name == "SR_WEAPON_0_1" ) || ( part_name =="SR_WEAPON_1_1") ) {
                    const PartType* part = GetPartType(part_name);
                    if (part) {
                            return part->Precision();
                    }
                    return 8;
            }
            return 2;
    case Targetting::HunterType::Planet:
    case Targetting::HunterType::Unknown:
            return 1;
    default:
            ErrorLogger() << "Got a lazy hunter weapon: " << part_name;
            return 1;
    }

}

Targetting::TriggerConditions Targetting::combine(const Targetting::TriggerConditions& condition, const Targetting::TriggerConditions& another_condition) {

    Targetting::TriggerConditions combined;
    combined.conditions.reserve( condition.conditions.size() + another_condition.conditions.size() ); // preallocate memory
    combined.conditions.insert( combined.conditions.end(), condition.conditions.begin(), condition.conditions.end() );
    combined.conditions.insert( combined.conditions.end(), another_condition.conditions.begin(), another_condition.conditions.end() );

    combined.weights.reserve( condition.weights.size() + another_condition.weights.size() ); // preallocate memory
    combined.weights.insert( combined.weights.end(), condition.weights.begin(), condition.weights.end() );
    combined.weights.insert( combined.weights.end(), another_condition.weights.begin(), another_condition.weights.end() );
    return combined;

}

Targetting::TriggerConditions Targetting::combine(      Targetting::TriggerConditions& conditions,       Targetting::TriggerCondition condition, int weight) {
    Targetting::TriggerConditions combined;
    combined.conditions.reserve( conditions.conditions.size() + 1 );
    combined.weights.reserve( conditions.weights.size() + 1 );

    combined = conditions;
    combined.conditions.push_back(condition);
    combined.weights.push_back(weight);
    return combined;
}
