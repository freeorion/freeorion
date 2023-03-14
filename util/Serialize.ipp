#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif

#include <boost/version.hpp>

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

/** \brief Structure to have compatibility with saves made with GG::Clr.
 *  \todo Remove it on compatibility breakage. */
struct CompatColor {
    unsigned char r;   ///< the red channel
    unsigned char g;   ///< the green channel
    unsigned char b;   ///< the blue channel
    unsigned char a;   ///< the alpha channel
};

namespace boost::container {
    template <class Key, class Compare, class AllocatorOrContainer> class flat_set;
    template <class Key, class Value, class Compare, class AllocatorOrContainer> class flat_map;
}

namespace boost::serialization {
    template <typename Archive>
    void serialize(Archive& ar, CompatColor& clr, const unsigned int version)
    {
        ar  & BOOST_SERIALIZATION_NVP(clr.r)
            & BOOST_SERIALIZATION_NVP(clr.g)
            & BOOST_SERIALIZATION_NVP(clr.b)
            & BOOST_SERIALIZATION_NVP(clr.a);
    }


    using namespace boost::container;

    template<class Archive, class Key, class Value, class Compare, class AllocatorOrContainer>
    void save(Archive& ar, const flat_map<Key, Value, Compare, AllocatorOrContainer>& m, const unsigned int)
    { stl::save_collection(ar, m); }

    template<class Archive, class Key, class Value, class Compare, class AllocatorOrContainer>
    void load(Archive& ar, flat_map<Key, Value, Compare, AllocatorOrContainer>& m, const unsigned int)
    { load_map_collection(ar, m); }

    template<class Archive, class Key, class Value, class Compare, class AllocatorOrContainer>
    void serialize(Archive& ar, flat_map<Key, Value, Compare, AllocatorOrContainer>& m, const unsigned int file_version)
    { split_free(ar, m, file_version); }

    // Note: I tried loading the internal vector of a flat_map instead of
    //       loading it as a map and constructing elements on the stack.
    //       The result was similar or slightly slower than the stack loader.

    template<class Archive, class Value, class Compare, class AllocatorOrContainer>
    void save(Archive& ar, const flat_set<Value, Compare, AllocatorOrContainer>& s, const unsigned int)
    { stl::save_collection(ar, s); }

    template<class Archive, class Value, class Compare, class AllocatorOrContainer>
    void load(Archive& ar, flat_set<Value, Compare, AllocatorOrContainer>& s, const unsigned int)
    { load_set_collection(ar, s); }

    template<class Archive, class Value, class Compare, class AllocatorOrContainer>
    void serialize(Archive& ar, flat_set<Value, Compare, AllocatorOrContainer>& s, const unsigned int file_version)
    { split_free(ar, s, file_version); }
}

