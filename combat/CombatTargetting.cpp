#include "Targetting.h"

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
