#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"

// exports for boost serialization of polymorphic UniverseObject hierarchy

#ifdef FREEORION_WIN32

BOOST_CLASS_EXPORT(System)

#else

// HACK!  For some odd reason, defining guid() below with inline, and only
// when specialized to System, fails to define a reachable specialization when
// overload resolution looks for it at call time later.  GCC.  Go figure.

namespace boost { namespace serialization {
    template<>
    struct guid_defined<System> : boost::mpl::true_ {};

    template<>
    const char * guid<System>()
    { return "System"; }
} }

BOOST_CLASS_EXPORT_IMPLEMENT(System)

#endif

BOOST_CLASS_EXPORT(Planet)
BOOST_CLASS_EXPORT(Building)
BOOST_CLASS_EXPORT(Fleet)
BOOST_CLASS_EXPORT(Ship)
