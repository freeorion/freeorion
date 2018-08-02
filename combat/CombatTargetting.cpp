#include "CombatTargetting.h"

#include "../universe/Enums.h"
#include "../universe/Planet.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Fighter.h"

#include "../util/Logger.h"

bool Targetting::isPreferredTarget(Targetting::TriggerCondition condition,
                                   Targetting::Target target) {
    if (condition == 0) {
            DebugLogger() << "No preferences. Target is perfect." ;
            return true;
    }
    DebugLogger() << "Looking for a (typemask "<< condition << ") target";
    Fighter* boat = dynamic_cast<Fighter*>(target.get());
    auto obj_type = target->ObjectType();
    if ( obj_type == OBJ_FIGHTER ) {
        if ( condition & (boat->TargetTypes()) ) {
            DebugLogger() << "This space boat (type " << (boat->TargetTypes()) << ") should die!";
            return true;
        } else {
            DebugLogger() << "Don't care about this space boat (type " << (boat->TargetTypes()) << ") - reroll if possible";
            return false;
        }
    } else if ( obj_type == OBJ_SHIP ) {
        if ( condition & ShipPrey ) {
            DebugLogger() << "This space ship (type " << ShipPrey << ") should die! " ;
            return true;
        } else {
            DebugLogger() << "Don't care about this space ship (type " << ShipPrey << ") - reroll if possible. " ;
            return false;
        }
    }
    ErrorLogger() << "Nobody likes to shoot at such " << obj_type;
    return false;
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
    if ("FT_HANGAR_3" == part_name) {
        return ShipPrey;
    }
    if ("FT_HANGAR_1" == part_name || "SR_WEAPON_0_1" == part_name) {
        return BoatPrey;
    }
    return NoPreference;
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
        if (part_name == "SR_WEAPON_0_1") {
            return 3;
        }
    case Targetting::HunterType::Planet:
    case Targetting::HunterType::Unknown:
            return 1;
    default:
            ErrorLogger() << "Got a lazy hunter weapon: " << part_name;
            return 1;
    }

}
