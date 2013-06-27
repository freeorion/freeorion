// -*- C++ -*-
#ifndef _Serialize_h_
#define _Serialize_h_

// Set this to true to do all serialization using binary archives.  Otherwise, XML archives will be used.
#define FREEORION_BINARY_SERIALIZATION 0

#if FREEORION_BINARY_SERIALIZATION
#  include <boost/archive/binary_iarchive.hpp>
#  include <boost/archive/binary_oarchive.hpp>
typedef boost::archive::binary_iarchive freeorion_iarchive;
typedef boost::archive::binary_oarchive freeorion_oarchive;
#else
#  include <boost/archive/xml_iarchive.hpp>
#  include <boost/archive/xml_oarchive.hpp>
typedef boost::archive::xml_iarchive freeorion_iarchive;
typedef boost::archive::xml_oarchive freeorion_oarchive;
#endif

#include <map>

#include "Export.h"

class OrderSet;
class PathingEngine;
class Universe;
class UniverseObject;

// NB: Do not try to serialize types that contain longs, since longs are different sizes on 32- and 64-bit
// architectures.  Replace your longs with long longs for portability.  See longer note in Serialize.cpp for more info.

/** Serializes \a universe to output archive \a oa. */
FO_COMMON_API void Serialize(freeorion_oarchive& oa, const Universe& universe);

/** Serializes \a object_map to output archive \a oa. */
void Serialize(freeorion_oarchive& oa, const std::map<int, UniverseObject*>& objects);

/** Serializes \a order_set to output archive \a oa. */
void Serialize(freeorion_oarchive& oa, const OrderSet& order_set);

/** Serializes \a pathing_engine to output archive \a oa. */
void Serialize(freeorion_oarchive& oa, const PathingEngine& pathing_engine);

/** Deserializes \a universe from input archive \a ia. */
FO_COMMON_API void Deserialize(freeorion_iarchive& ia, Universe& universe);

/** Serializes \a object_map from input archive \a ia. */
void Deserialize(freeorion_iarchive& ia, std::map<int, UniverseObject*>& objects);

/** Deserializes \a order_set from input archive \a ia. */
void Deserialize(freeorion_iarchive& ia, OrderSet& order_set);

/** Deserializes \a pathing_engine from input archive \a ia. */
void Deserialize(freeorion_iarchive& ia, PathingEngine& pathing_engine);

#endif // _Serialize_h_
