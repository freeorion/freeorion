#ifndef COMBATEVENT_H
#define COMBATEVENT_H


#include <boost/optional/optional.hpp>

#include "../util/Export.h"

#include <memory>
#include <string>
#include <vector>


// Should be unique_ptr, but we don't have c++11
struct CombatEvent;
typedef std::shared_ptr<CombatEvent> CombatEventPtr;
typedef std::shared_ptr<const CombatEvent> ConstCombatEventPtr;
struct ScriptingContext;

/// An abstract base class for combat events
/**
Combat events are created during combat processing to act as a log of
combat events.

Many combat events are created, but few are examined by players.
The constructors must be fast.  They should not do any string processing
in the constructor. The descriptions can be expanded on request.

*/
struct FO_COMMON_API CombatEvent {
    constexpr CombatEvent() = default;
    virtual ~CombatEvent() = default;

    [[nodiscard]] virtual std::string DebugString(const ScriptingContext& context) const = 0;

    /** Generate the combat log description.
        Describe the result of a combat event (i.e. what happened). */
    [[nodiscard]] virtual std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const = 0;

    /** Generate the combat log details.
        Describe how it happened in enough detail to avoid a trip to the Pedia. */
    [[nodiscard]] virtual std::string CombatLogDetails(int viewing_empire_id) const
    { return {}; }

    /** If the combat event is composed of smaller events then return a vector of the sub events,
        otherwise returns an empty vector. */
    [[nodiscard]] virtual std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const
    { return std::vector<ConstCombatEventPtr>(); }

    /** Return true if there are no details; */
    [[nodiscard]] virtual bool AreDetailsEmpty(int viewing_empire_id) const noexcept
    { return true; }

    /** Return true if there are no sub events; */
    [[nodiscard]] virtual bool AreSubEventsEmpty(int viewing_empire_id) const noexcept
    { return true; }

    /** Return true if sub events are to be flattened on display; */
    virtual bool FlattenSubEvents() const
    { return false; }

    /** Return principal faction.

        PrincipalFaction is used by UnorderedEvents to sort the display
        of events by Facton.  The principal faction should be the
        faction most active in the event (i.e. the attacker in a WeaponEvent).
        It is from the perspective of the \p viewing_empire_id. Some events
        like BoutBegin are not associated with any faction.*/
    [[nodiscard]] virtual boost::optional<int> PrincipalFaction(int viewing_empire_id) const;
};


#endif
