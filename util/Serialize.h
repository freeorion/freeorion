#ifndef _Serialize_h_
#define _Serialize_h_

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/version.hpp>

#include <map>

#include "Export.h"

class OrderSet;
class Universe;
class UniverseObject;

typedef boost::archive::binary_iarchive freeorion_bin_iarchive;
typedef boost::archive::binary_oarchive freeorion_bin_oarchive;
typedef boost::archive::xml_iarchive freeorion_xml_iarchive;
typedef boost::archive::xml_oarchive freeorion_xml_oarchive;

//! @warning
//!     Do not try to serialize types that contain longs, since longs are
//!     different sizes on 32- and 64-bit architectures.  Replace your longs
//!     with long longs for portability.  It would seem that short of writing
//!     some Boost.Serialization archive that handles longs portably, we cannot
//!     transmit longs across machines with different bit-size architectures.

//! Serialize @p universe to output archive @p oa.
template <typename Archive>
FO_COMMON_API void Serialize(Archive& oa, const Universe& universe);

//! Serialize @p object_map to output archive @p oa.
template <typename Archive>
void Serialize(Archive& oa, const std::map<int, std::shared_ptr<UniverseObject>>& objects);

//! Serialize @p order_set to output archive @p oa.
template <typename Archive>
void Serialize(Archive& oa, const OrderSet& order_set);

//! Deserialize @p universe from input archive @p ia.
template <typename Archive>
FO_COMMON_API void Deserialize(Archive& ia, Universe& universe);

//! Deserialize @p object_map from input archive @p ia.
template <typename Archive>
void Deserialize(Archive& ia, std::map<int, std::shared_ptr<UniverseObject>>& objects);

//! Deserialize @p order_set from input archive @p ia.
template <typename Archive>
void Deserialize(Archive& ia, OrderSet& order_set);


struct GalaxySetupData;

BOOST_CLASS_VERSION(GalaxySetupData, 3);

template <typename Archive>
void serialize(Archive&, GalaxySetupData&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, GalaxySetupData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, GalaxySetupData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, GalaxySetupData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, GalaxySetupData&, unsigned int const);


struct PlayerSaveHeaderData;

template <typename Archive>
void serialize(Archive&, PlayerSaveHeaderData&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, PlayerSaveHeaderData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, PlayerSaveHeaderData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, PlayerSaveHeaderData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, PlayerSaveHeaderData&, unsigned int const);


struct SaveGameEmpireData;

BOOST_CLASS_VERSION(SaveGameEmpireData, 2);

template <typename Archive>
void serialize(Archive&, SaveGameEmpireData&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SaveGameEmpireData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SaveGameEmpireData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SaveGameEmpireData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SaveGameEmpireData&, unsigned int const);


struct SaveGameUIData;

BOOST_CLASS_VERSION(SaveGameUIData, 4);

template <typename Archive>
void serialize(Archive&, SaveGameUIData&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SaveGameUIData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SaveGameUIData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SaveGameUIData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SaveGameUIData&, unsigned int const);


#endif // _Serialize_h_
