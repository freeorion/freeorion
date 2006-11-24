// -*- C++ -*-
#ifndef _Serialize_h_
#define _Serialize_h_

// Set this to true to do all serialization using binary archives.  Otherwise, XML archives will be used.
#define FREEORION_BINARY_SERIALIZATION 0

#if FREEORION_BINARY_SERIALIZATION
#  include <boost/archive/binary_iarchive.hpp>
#  include <boost/archive/binary_oarchive.hpp>
#  define FREEORION_IARCHIVE_TYPE boost::archive::binary_iarchive
#  define FREEORION_OARCHIVE_TYPE boost::archive::binary_oarchive
#else
#  include <boost/archive/xml_iarchive.hpp>
#  include <boost/archive/xml_oarchive.hpp>
#  define FREEORION_IARCHIVE_TYPE boost::archive::xml_iarchive
#  define FREEORION_OARCHIVE_TYPE boost::archive::xml_oarchive
#endif

#include <vector>


class Empire;
class EmpireManager;
class OrderSet;
class Universe;

/** Serializes the single empire \a empire to output archive \a oa. */
void Serialize(FREEORION_OARCHIVE_TYPE& oa, const Empire& empire);

/** Serializes \a empire_empire, including all its Empires, to output archive \a oa. */
void Serialize(FREEORION_OARCHIVE_TYPE& oa, const EmpireManager& empire_manager);

/** Serializes \a universe to output archive \a oa. */
void Serialize(FREEORION_OARCHIVE_TYPE& oa, const Universe& universe);

/** Serializes \a order_set to output archive \a oa. */
void Serialize(FREEORION_OARCHIVE_TYPE& oa, const OrderSet& order_set);

/** Deserializes the single empire \a empire from input archive \a ia. */
void Deserialize(FREEORION_IARCHIVE_TYPE& ia, Empire& empire);

/** Deserializes \a empire_empire, including all its Empires, from input archive \a ia. */
void Deserialize(FREEORION_IARCHIVE_TYPE& ia, EmpireManager& empire_manager);

/** Deserializes \a universe from input archive \a ia. */
void Deserialize(FREEORION_IARCHIVE_TYPE& ia, Universe& universe);

/** Deserializes \a order_set from input archive \a ia. */
void Deserialize(FREEORION_IARCHIVE_TYPE& ia, OrderSet& order_set);

#endif // _Serialize_h_
