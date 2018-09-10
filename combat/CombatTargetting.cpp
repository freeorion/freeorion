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

bool Targetting::IsPreferredTarget(Targetting::TriggerConditions conditions,
                                   Targetting::Target target)
{
    const Precision highest_weight = Targetting::FindPrecision(conditions);
    for (long unsigned int i = 0; i < conditions.conditions.size(); i++) {
        const auto& condition = conditions.conditions[i];
        if (!condition) {
            DebugLogger() << "No preferences. Target is perfect." ;
            return true;
        }

        if (Targetting::IsPreferredTarget(condition, target)) {
            const int scale = 10; // scale all compared values to remove artifacts from rounding
            const int upper = std::round(highest_weight * 1.5f * scale); // HEURISTIC
            const int improbability = RandInt(0, upper);
            const int luck = conditions.weights[i] * scale;
            DebugLogger() << "Prioritized locking attempt (? " << luck << "/" << upper << " >? " << improbability << "/" << upper << " ?)";
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

bool Targetting::IsPreferredTarget(Targetting::TriggerCondition condition,
                                   Targetting::Target target)
{
    if (!condition) {
        DebugLogger() << "No preferences. Target is perfect." ;
        return true;
    }

    DebugLogger() << "Evaluate preferred prey condition" << condition->Description();
    bool is_preferred = condition->Eval(target);
    auto obj_type = target->ObjectType();
    if (is_preferred) {
        DebugLogger() << "This " << obj_type << " ship should die! " ;
    } else {
        DebugLogger() << "Don't care about this " << obj_type << " - reroll if possible. " ;
    }
    return is_preferred;
}

Targetting::TriggerCondition Targetting::PreyAsTriggerCondition(const Targetting::PreyType prey)
{
    std::unique_ptr<ValueRef::ValueRefBase<UniverseObjectType>> p_obj;
    switch (prey) {
    case Targetting::PreyType::PlanetPrey:
        p_obj = std::unique_ptr<ValueRef::ValueRefBase<UniverseObjectType>>(new ValueRef::Constant<UniverseObjectType>(OBJ_PLANET));
        break;
    case Targetting::PreyType::BoatPrey:
        p_obj = std::unique_ptr<ValueRef::ValueRefBase<UniverseObjectType>>(new ValueRef::Constant<UniverseObjectType>(OBJ_FIGHTER));
        break;
    case Targetting::ShipPrey:
        p_obj = std::unique_ptr<ValueRef::ValueRefBase<UniverseObjectType>>(new ValueRef::Constant<UniverseObjectType>(OBJ_SHIP));
        break;
    case Targetting::AllPrey:
        //case Targetting::PreyType::NoPreference:
        return std::make_shared<Condition::All>();
        break;
    default:
        // error
        ErrorLogger() << "PreyAsTriggerCondition encountered unsupported PreyType " << prey << ".";
        return nullptr;
    }
    return std::make_shared<Condition::Type>(move(p_obj));
}

Targetting::TriggerConditions Targetting::PreyAsTriggerConditions(const Targetting::PreyType prey)
{ return Targetting::TriggerConditions(Targetting::PreyAsTriggerCondition(prey), 1); }

Targetting::TriggerCondition Targetting::FindPreferredTargets(const std::string& part_name)
{
    const PartType* part = GetPartType(part_name);
    return part->PreferredPrey();
}

Targetting::Precision Targetting::FindPrecision(const Targetting::TriggerConditions& conditions)
{
    auto result = std::max_element(conditions.weights.begin(), conditions.weights.end());
    return *result;
}

Targetting::TriggerConditions Targetting::Combine(const Targetting::TriggerConditions& condition,
                                                  const Targetting::TriggerConditions& another_condition)
{

    Targetting::TriggerConditions combined;
    combined.conditions.reserve( condition.conditions.size() + another_condition.conditions.size() ); // preallocate memory
    combined.conditions.insert( combined.conditions.end(), condition.conditions.begin(), condition.conditions.end() );
    combined.conditions.insert( combined.conditions.end(), another_condition.conditions.begin(), another_condition.conditions.end() );
    combined.weights.reserve( condition.weights.size() + another_condition.weights.size() ); // preallocate memory
    combined.weights.insert( combined.weights.end(), condition.weights.begin(), condition.weights.end() );
    combined.weights.insert( combined.weights.end(), another_condition.weights.begin(), another_condition.weights.end() );
    return combined;
}

Targetting::TriggerConditions Targetting::Combine(Targetting::TriggerConditions& conditions,
                                                  Targetting::TriggerCondition condition, int weight)
{
    Targetting::TriggerConditions combined;
    combined.conditions.reserve( conditions.conditions.size() + 1 );
    combined.weights.reserve( conditions.weights.size() + 1 );
    combined = conditions;
    combined.conditions.push_back(condition);
    combined.weights.push_back(weight);
    return combined;
}
