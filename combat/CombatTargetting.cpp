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

bool Targetting::IsPriorityTarget(const Targetting::TriggerConditions& conditions,
                                   Targetting::Target target)
{
    const Precision highest_weight = Targetting::FindPrecision(conditions);
    for (long unsigned int i = 0; i < conditions.conditions.size(); i++) {
        const auto& condition = conditions.conditions[i];
        if (!condition) {
            DebugLogger() << "No preferences. Target is perfect." ;
            return true;
        }

        if (Targetting::IsPriorityTarget(*condition, target)) {
            const int scale = 10; // scale all compared values to remove artifacts from rounding
            const int universe = std::round(highest_weight *  1.5f * scale); // HEURISTIC scaling the chance of highest weighted conditions
            const int max_locked = conditions.weights[i] * scale;
            // HEURISTIC expanding the universe to lower chance of lower weights
            // shift by one, as the real precision difference should be the number of rerolls, not the number of rolls
            int light_universe_weight = conditions.weights[i]-1;
            int heavy_universe_weight = highest_weight-1;
            if (light_universe_weight < 1) {
                // make a precision of 1 safe again for multiplication
                heavy_universe_weight += (1 - light_universe_weight);
                light_universe_weight = 1;
            }
            const int weighted_universe = (float)(universe * heavy_universe_weight) / (float)(light_universe_weight);
            const int roll = RandInt(1, weighted_universe);
            if (conditions.weights[i] == highest_weight) {
                DebugLogger() << "Prioritized locking attempt. Lock if h <= " << max_locked << "/" << weighted_universe
                              << ". Got h = " << roll << "; " << roll << "/" << weighted_universe << " (highest precision condition)";
            } else {
                DebugLogger() << "Prioritized locking attempt. Lock if w <= " << max_locked << "/" << weighted_universe
                              << ". Got w = " << roll << "; " << roll << "/" << weighted_universe << " (weighted precision condition)";
            }
            if (max_locked >= roll) {
                DebugLogger() << "Locked in target.";
                return true;
            } else {
                DebugLogger() << "Couldnt acquire lock." ;
            }
        }
    }
    return false;
}

bool Targetting::IsPriorityTarget(const ::Condition::ConditionBase& condition,
                                   Targetting::Target target)
{
    DebugLogger() << "Evaluate preferred prey condition" << condition.Description();
    bool is_preferred = condition.Eval(target);
    auto obj_type = target->ObjectType();
    if (is_preferred) {
        DebugLogger() << "This " << obj_type << " ship should die! " ;
    } else {
        DebugLogger() << "Don't care about this " << obj_type << " - reroll if possible. " ;
    }
    return is_preferred;
}

const ::Condition::ConditionBase* Targetting::PreyAsTriggerCondition(const Targetting::PreyType prey)
{
    static std::unique_ptr<::Condition::ConditionBase> is_boat =
        boost::make_unique<::Condition::Type>(std::unique_ptr<ValueRef::ValueRefBase<UniverseObjectType>>(new ValueRef::Constant<UniverseObjectType>(OBJ_FIGHTER)));
    static std::unique_ptr<::Condition::ConditionBase> is_planet =
        boost::make_unique<::Condition::Type>(std::unique_ptr<ValueRef::ValueRefBase<UniverseObjectType>>(new ValueRef::Constant<UniverseObjectType>(OBJ_PLANET)));
    static std::unique_ptr<::Condition::ConditionBase> is_ship =
        boost::make_unique<::Condition::Type>(std::unique_ptr<ValueRef::ValueRefBase<UniverseObjectType>>(new ValueRef::Constant<UniverseObjectType>(OBJ_SHIP)));
    static std::unique_ptr<::Condition::ConditionBase> is_any = boost::make_unique<Condition::All>();
    // One could support mixed values by allowing bit patterns; e.g. returning static is_boat_or_ship Condition::And({is_boat,is_ship})
    switch (prey) {
    case Targetting::PreyType::PlanetPrey:
        return is_planet.get();
    case Targetting::PreyType::BoatPrey:
        return is_boat.get();
    case Targetting::ShipPrey:
        return is_ship.get();
    case Targetting::AllPrey:
    //case Targetting::PreyType::NoPreference:
        return is_any.get();
    default:
        // error
        ErrorLogger() << "PreyAsTriggerCondition encountered unsupported PreyType " << prey << ".";
        return nullptr;
    }
}

const ::Condition::ConditionBase* Targetting::FindPriorityTargets(const std::string& part_name)
{
    const PartType* part = GetPartType(part_name);
    return part->PriorityTargets();
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

const Targetting::TriggerConditions Targetting::Combine(const Targetting::TriggerConditions& conditions,
                                                        const ::Condition::ConditionBase* condition, int weight)
{
    if (!condition) {
        return conditions;
    }
    Targetting::TriggerConditions combined;
    combined.conditions.reserve( conditions.conditions.size() + 1 );
    combined.weights.reserve( conditions.weights.size() + 1 );
    combined = conditions;
    combined.conditions.push_back(std::move(condition));
    combined.weights.push_back(weight);
    return combined;
}

