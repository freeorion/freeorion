// -*- C++ -*-
#ifndef _Serialize_h_
#define _Serialize_h_

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/variant/variant.hpp>

class Empire;
class EmpireManager;
class Universe;

/** The type used to refer to an input archive, whether binary or XML. */
typedef boost::variant<boost::archive::binary_iarchive*,
                       boost::archive::xml_iarchive*> IArchivePtr;

/** The type used to refer to an output archive, whether binary or XML. */
typedef boost::variant<boost::archive::binary_oarchive*,
                       boost::archive::xml_oarchive*> OArchivePtr;

/** Serializes the single empire \a empire to output archive \a oa. */
void Serialize(OArchivePtr oa, const Empire& empire);

/** Serializes \a empire_empire, including all its Empires, to output archive \a oa. */
void Serialize(OArchivePtr oa, const EmpireManager& empire_manager);

/** Serializes \a universe to output archive \a oa. */
void Serialize(OArchivePtr oa, const Universe& universe);

/** Deserializes the single empire \a empire from input archive \a ia. */
void Deserialize(IArchivePtr ia, Empire& empire);

/** Deserializes \a empire_empire, including all its Empires, from input archive \a ia. */
void Deserialize(IArchivePtr ia, EmpireManager& empire_manager);

/** Deserializes \a universe from input archive \a ia. */
void Deserialize(IArchivePtr ia, Universe& universe);

#endif // _Serialize_h_
