/// \file CombatEvent.h declares the base CombatEvent.
#ifndef COMBATEVENT_H
#define COMBATEVENT_H

#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>

#include "../util/Export.h"

/// An abstract base class for combat events
struct FO_COMMON_API CombatEvent {
    public:
    CombatEvent();

    virtual ~CombatEvent() {}
    virtual std::string DebugString() const = 0;

    private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize ( Archive& ar, const unsigned int version );
};

BOOST_CLASS_EXPORT_KEY ( CombatEvent )

// Should be unique_ptr, but we don't have c++11
typedef boost::shared_ptr<CombatEvent> CombatEventPtr;

#endif // COMBATEVENT_H
