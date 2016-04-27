#ifndef COMBATEVENT_H
#define COMBATEVENT_H

#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <vector>

#include "../util/Export.h"

// Should be unique_ptr, but we don't have c++11
struct CombatEvent;
typedef boost::shared_ptr<CombatEvent> CombatEventPtr;
typedef boost::shared_ptr<const CombatEvent> ConstCombatEventPtr;

/// An abstract base class for combat events
struct FO_COMMON_API CombatEvent {
    CombatEvent();

    virtual ~CombatEvent() {}
    virtual std::string DebugString() const = 0;

    /** Generate the combat log description.
        Describe the result of a combat event (i.e. what happened). */
    virtual std::string CombatLogDescription(int viewing_empire_id) const = 0;

    /** Generate the combat log details.
        Describe how it happened in enough detail to avoid a trip to the Pedia. */
    virtual std::string CombatLogDetails(int viewing_empire_id) const
    { return std::string(""); }

    /** If the combat event is composed of smaller events then return a vector of the sub events,
        otherwise returns an empty vector.
    */
    virtual std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const
    { return std::vector<ConstCombatEventPtr>(); }

    /** Return true if there are no sub events;
    */
    virtual bool AreSubEventsEmpty(int viewing_empire_id) const
    { return true; }

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

BOOST_CLASS_EXPORT_KEY(CombatEvent)

#endif // COMBATEVENT_H
