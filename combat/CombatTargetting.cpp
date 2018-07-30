#include "CombatTargetting.h"

bool Targetting::isPreferredTarget(Targetting::TriggerCondition condition,
				   Targetting::Target target) {
    if (condition == 0) {
	    ErrorLogger() << "No preferences. Target is perfect." ;
	    return true;
    }
    ErrorLogger() << "Looking for a (targetting "<< condition << ") target";
    if (Fighter* boat = dynamic_cast<Fighter*>(target.get())) {
        if ( condition & (boat->TargetTypes()) ) {
	    ErrorLogger() << "This space boat (targetting " << target << ") should die! " ;
	    return true;
	} else {
	    ErrorLogger() << "Don't care about this space boat - reroll if possible";
	    return false;
	}
    } else if (Ship* ship = dynamic_cast<Ship*>(target.get()) && (condition & ShipPrey) ) {
	    ErrorLogger() << "This space ship (prey " << ShipPrey << ") should die! " ;
	return true;
    }
    return false;
}

Targetting::Prey Targetting::findTargetTypes(const std::string& part_name_) {
	if (part_name.rfind("FT_", 0) == 0) {
		return BoatPrey;
	}
	if (part_name.rfind("SR_", 0) == 0) {
		return ShipPrey;


	} 
	// shouldnt happen
	return AllPrey;
}

Targetting::TriggerCondition Targetting::findPreferredTargets(const std::string& part_name_) {
    if ("FT_HANGAR_3" == part_name) {
	return ShipPrey;
    }
    if ("FT_HANGAR_1" == part_name || "SR_WEAPON_0_1" == part_name) {
        return BoatPrey;
    }
    return NoPreference;
}
