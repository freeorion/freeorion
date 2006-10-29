#include "Serialize.h"

#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"

#include "SDL_byteorder.h"

#if defined(_MSC_VER)
  // HACK! this keeps VC 7.x from barfing when it sees "typedef __int64 int64_t;"
  // in boost/cstdint.h when compiling under windows
#  if defined(int64_t)
#    undef int64_t
#  endif
#elif defined(WIN32)
  // HACK! this keeps gcc 3.x from barfing when it sees "typedef long long uint64_t;"
  // in boost/cstdint.h when compiling under windows
#  define BOOST_MSVC -1
#endif

#include <boost/serialization/export.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/static_assert.hpp>
#include <boost/variant/get.hpp>


// exports for boost serialization of polymorphic UniverseObject hierarchy
BOOST_CLASS_EXPORT(System)
BOOST_CLASS_EXPORT(Planet)
BOOST_CLASS_EXPORT(Building)
BOOST_CLASS_EXPORT(Fleet)
BOOST_CLASS_EXPORT(Ship)

// some endianness and size checks to ensure portability of binary save files; of one or more of these fails, it means
// that FreeOrion is not supported on your platform/compiler pair, and must be modified to provide data of the
// appropriate size(s).
BOOST_STATIC_ASSERT(SDL_BYTEORDER == SDL_LIL_ENDIAN);
BOOST_STATIC_ASSERT(sizeof(char) == 1);
BOOST_STATIC_ASSERT(sizeof(short) == 2);
BOOST_STATIC_ASSERT(sizeof(int) == 4);
BOOST_STATIC_ASSERT(sizeof(Uint32) == 4);
BOOST_STATIC_ASSERT(sizeof(long) == 4);
BOOST_STATIC_ASSERT(sizeof(long long) == 8);
BOOST_STATIC_ASSERT(sizeof(float) == 4);
BOOST_STATIC_ASSERT(sizeof(double) == 8);


void Serialize(OArchivePtr oa, const Empire& empire)
{
    if (oa.which())
        *boost::get<boost::archive::xml_oarchive*>(oa) << BOOST_SERIALIZATION_NVP(empire);
    else
        *boost::get<boost::archive::binary_oarchive*>(oa) << BOOST_SERIALIZATION_NVP(empire);
}

void Serialize(OArchivePtr oa, const EmpireManager& empire_manager)
{
    if (oa.which())
        *boost::get<boost::archive::xml_oarchive*>(oa) << BOOST_SERIALIZATION_NVP(empire_manager);
    else
        *boost::get<boost::archive::binary_oarchive*>(oa) << BOOST_SERIALIZATION_NVP(empire_manager);
}

void Serialize(OArchivePtr oa, const Universe& universe)
{
    if (oa.which())
        *boost::get<boost::archive::xml_oarchive*>(oa) << BOOST_SERIALIZATION_NVP(universe);
    else
        *boost::get<boost::archive::binary_oarchive*>(oa) << BOOST_SERIALIZATION_NVP(universe);
}

void Deserialize(IArchivePtr ia, Empire& empire)
{
    if (ia.which())
        *boost::get<boost::archive::xml_iarchive*>(ia) >> BOOST_SERIALIZATION_NVP(empire);
    else
        *boost::get<boost::archive::binary_iarchive*>(ia) >> BOOST_SERIALIZATION_NVP(empire);
}

void Deserialize(IArchivePtr ia, EmpireManager& empire_manager)
{
    if (ia.which())
        *boost::get<boost::archive::xml_iarchive*>(ia) >> BOOST_SERIALIZATION_NVP(empire_manager);
    else
        *boost::get<boost::archive::binary_iarchive*>(ia) >> BOOST_SERIALIZATION_NVP(empire_manager);
}

void Deserialize(IArchivePtr ia, Universe& universe)
{
    if (ia.which())
        *boost::get<boost::archive::xml_iarchive*>(ia) >> BOOST_SERIALIZATION_NVP(universe);
    else
        *boost::get<boost::archive::binary_iarchive*>(ia) >> BOOST_SERIALIZATION_NVP(universe);
}
