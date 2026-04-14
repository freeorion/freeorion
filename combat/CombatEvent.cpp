#include "CombatEvent.h"

#include "../util/Logger.h"

std::optional<int> CombatEvent::PrincipalFaction(int viewing_empire_id) const {
    ErrorLogger() << "A combat logger expected this event to be associated with a faction";
    return std::nullopt;
}
