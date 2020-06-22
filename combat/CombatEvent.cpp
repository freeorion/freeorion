#include "CombatEvent.h"

#include "../util/Logger.h"

#include <sstream>

CombatEvent::CombatEvent() {}

boost::optional<int> CombatEvent::PrincipalFaction(int viewing_empire_id) const {
    ErrorLogger() << "A combat logger expected this event to be associated with a faction: "
                  << this->DebugString();
    return boost::optional<int>();
}
