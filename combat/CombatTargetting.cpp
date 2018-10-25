#include "CombatTargetting.h"

#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include "../universe/Enums.h"

//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

#include "../util/Logger.h"

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


