#include "Serialize.h"

#include "../combat/OpenSteer/AsteroidBeltObstacle.h"
#include "../combat/OpenSteer/CombatShip.h"
#include "../combat/OpenSteer/CombatFighter.h"
#include "../combat/OpenSteer/Missile.h"
#include "../combat/OpenSteer/Obstacle.h"
#include "../combat/OpenSteer/PathingEngine.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../util/OrderSet.h"


#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif

#include <boost/static_assert.hpp>
#include <boost/detail/endian.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include <boost/ptr_container/serialize_ptr_vector.hpp>


// exports for boost serialization of polymorphic UniverseObject hierarchy
BOOST_CLASS_EXPORT(System)
BOOST_CLASS_EXPORT(Planet)
BOOST_CLASS_EXPORT(Building)
BOOST_CLASS_EXPORT(Fleet)
BOOST_CLASS_EXPORT(Ship)

// exports for boost serialization of polymorphic Order hierarchy
BOOST_CLASS_EXPORT(RenameOrder)
BOOST_CLASS_EXPORT(NewFleetOrder)
BOOST_CLASS_EXPORT(FleetMoveOrder)
BOOST_CLASS_EXPORT(FleetTransferOrder)
BOOST_CLASS_EXPORT(FleetColonizeOrder)
BOOST_CLASS_EXPORT(DeleteFleetOrder)
BOOST_CLASS_EXPORT(ChangeFocusOrder)
BOOST_CLASS_EXPORT(ResearchQueueOrder)
BOOST_CLASS_EXPORT(ProductionQueueOrder)
BOOST_CLASS_EXPORT(ShipDesignOrder)
BOOST_CLASS_EXPORT(ScrapOrder)

// exports for boost serialization of PathingEngine-related classes
BOOST_CLASS_EXPORT(CombatShip)
BOOST_CLASS_EXPORT(CombatFighter)
BOOST_CLASS_EXPORT(Missile)
BOOST_CLASS_EXPORT(OpenSteer::SphereObstacle)
BOOST_CLASS_EXPORT(OpenSteer::BoxObstacle)
BOOST_CLASS_EXPORT(OpenSteer::PlaneObstacle)
BOOST_CLASS_EXPORT(OpenSteer::RectangleObstacle)
BOOST_CLASS_EXPORT(AsteroidBeltObstacle)

// some endianness and size checks to ensure portability of binary save files; of one or more of these fails, it means
// that FreeOrion is not supported on your platform/compiler pair, and must be modified to provide data of the
// appropriate size(s).
#ifndef BOOST_LITTLE_ENDIAN
#  error "Incompatible endianness for binary serialization."
#endif
BOOST_STATIC_ASSERT(sizeof(char) == 1);
BOOST_STATIC_ASSERT(sizeof(short) == 2);
BOOST_STATIC_ASSERT(sizeof(int) == 4);
BOOST_STATIC_ASSERT(sizeof(long long) == 8);
BOOST_STATIC_ASSERT(sizeof(float) == 4);
BOOST_STATIC_ASSERT(sizeof(double) == 8);

// This is commented out, but left here by way of explanation.  This assert is the only one that seems to fail on 64-bit
// systems.  It would seem that short of writing some Boost.Serialization archive that handles longs portably, we cannot
// transmit longs across machines with different bit-size architectures.  So, don't use longs -- use long longs instead
// if you need something bigger than an int for some reason.
//BOOST_STATIC_ASSERT(sizeof(long) == 4);


void Serialize(FREEORION_OARCHIVE_TYPE& oa, const Empire& empire)
{ oa << BOOST_SERIALIZATION_NVP(empire); }

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const EmpireManager& empire_manager)
{ oa << BOOST_SERIALIZATION_NVP(empire_manager); }

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const Universe& universe)
{ oa << BOOST_SERIALIZATION_NVP(universe); }

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const std::map<int, UniverseObject*>& objects)
{ oa << BOOST_SERIALIZATION_NVP(objects); }

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const OrderSet& order_set)
{ oa << BOOST_SERIALIZATION_NVP(order_set); }

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const PathingEngine& pathing_engine)
{ oa << BOOST_SERIALIZATION_NVP(pathing_engine); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, Empire& empire)
{ ia >> BOOST_SERIALIZATION_NVP(empire); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, EmpireManager& empire_manager)
{ ia >> BOOST_SERIALIZATION_NVP(empire_manager); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, Universe& universe)
{ ia >> BOOST_SERIALIZATION_NVP(universe); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, std::map<int, UniverseObject*>& objects)
{ ia >> BOOST_SERIALIZATION_NVP(objects); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, OrderSet& order_set)
{ ia >> BOOST_SERIALIZATION_NVP(order_set); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, PathingEngine& pathing_engine)
{ ia >> BOOST_SERIALIZATION_NVP(pathing_engine); }
