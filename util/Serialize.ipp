#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif

#include <boost/static_assert.hpp>
#include <boost/detail/endian.hpp>
#include <boost/version.hpp>

#if BOOST_VERSION == 105800
// HACK: The following two includes work around a bug in boost 1.58
#include <boost/archive/basic_archive.hpp>
#endif

#include <boost/serialization/export.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/serialization/optional.hpp>

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
