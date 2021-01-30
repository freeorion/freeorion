#include "CombatEvent.h"

#include "../util/Logger.h"

#include <sstream>

boost::optional<int> CombatEvent::PrincipalFaction(int viewing_empire_id) const {
    ErrorLogger() << "A combat logger expected this event to be associated with a faction";
    return boost::optional<int>();
}
