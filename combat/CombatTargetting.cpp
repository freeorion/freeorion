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

bool Targetting::isPreferredTarget(Targetting::TriggerCondition condition,
                                   Targetting::Target target) {
    if (!condition) {
            DebugLogger() << "No preferences. Target is perfect." ;
            return true;
    }

    DebugLogger() << "Use enemy object as context source for PreferredPrey condition";
    // ScriptingContext context(target);
    // ScriptingContext context();
    DebugLogger() << "Evaluate preferred prey condition";
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
    //const Condition::ConditionBase* condition = part->PreferredPrey();
    return part->PreferredPrey();
}

Targetting::Precision Targetting::findPrecision(Targetting::HunterType hunting, const std::string& part_name) {
    if ( Targetting::HunterType::Unknown == hunting ) {
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
