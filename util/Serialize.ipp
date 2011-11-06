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

// some endianness and size checks to ensure portability of binary save files;
// of one or more of these fails, it means that FreeOrion is not supported on
// your platform/compiler pair, and must be modified to provide data of the
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

// This is commented out, but left here by way of explanation.  This assert is
// the only one that seems to fail on 64-bit systems.  It would seem that
// short of writing some Boost.Serialization archive that handles longs
// portably, we cannot transmit longs across machines with different bit-size
// architectures.  So, don't use longs -- use long longs instead if you need
// something bigger than an int for some reason.
//BOOST_STATIC_ASSERT(sizeof(long) == 4);

#include <GG/Clr.h>

namespace boost { namespace serialization {

    template <class Archive>
    void serialize(Archive& ar, GG::Clr& clr, const unsigned int version)
    {
        ar  & BOOST_SERIALIZATION_NVP(clr.r)
            & BOOST_SERIALIZATION_NVP(clr.g)
            & BOOST_SERIALIZATION_NVP(clr.b)
            & BOOST_SERIALIZATION_NVP(clr.a);
    }

} }
