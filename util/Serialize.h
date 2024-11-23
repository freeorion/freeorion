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

//! This must be set to the encoding empire's id when serializing various
//! gamestate information, so that only the relevant info is serialized for the
//! intended recieipient. This is implemented this way so that we don't need to
//! write custom boost::serialization classes that implement empire-dependent
//! visibility.
FO_COMMON_API int& GlobalSerializationEncodingForEmpire();

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


struct ChatHistoryEntity;

BOOST_CLASS_VERSION(ChatHistoryEntity, 1);

template <typename Archive>
void serialize(Archive&, ChatHistoryEntity&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, ChatHistoryEntity&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, ChatHistoryEntity&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, ChatHistoryEntity&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, ChatHistoryEntity&, unsigned int const);

struct CombatLog;

BOOST_CLASS_VERSION(CombatLog, 1);

template <typename Archive>
void serialize(Archive&, CombatLog&, const unsigned int);

extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, CombatLog&, const unsigned int);
extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, CombatLog&, const unsigned int);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, CombatLog&, const unsigned int);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, CombatLog&, const unsigned int);


class CombatLogManager;

template <typename Archive>
void serialize(Archive&, CombatLogManager&, const unsigned int);

extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, CombatLogManager&, const unsigned int);
extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, CombatLogManager&, const unsigned int);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, CombatLogManager&, const unsigned int);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, CombatLogManager&, const unsigned int);

template <typename Archive>
void SerializeIncompleteLogs(Archive&, CombatLogManager&, const unsigned int);

extern template FO_COMMON_API void SerializeIncompleteLogs<freeorion_bin_iarchive>(freeorion_bin_iarchive&, CombatLogManager&, const unsigned int);
extern template FO_COMMON_API void SerializeIncompleteLogs<freeorion_bin_oarchive>(freeorion_bin_oarchive&, CombatLogManager&, const unsigned int);
extern template FO_COMMON_API void SerializeIncompleteLogs<freeorion_xml_iarchive>(freeorion_xml_iarchive&, CombatLogManager&, const unsigned int);
extern template FO_COMMON_API void SerializeIncompleteLogs<freeorion_xml_oarchive>(freeorion_xml_oarchive&, CombatLogManager&, const unsigned int);


class EmpireManager;

BOOST_CLASS_VERSION(EmpireManager, 3);

template <typename Archive>
void serialize(Archive&, EmpireManager&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, EmpireManager&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, EmpireManager&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, EmpireManager&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, EmpireManager&, unsigned int const);


struct GalaxySetupData;

BOOST_CLASS_VERSION(GalaxySetupData, 3);

template <typename Archive>
void serialize(Archive&, GalaxySetupData&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, GalaxySetupData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, GalaxySetupData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, GalaxySetupData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, GalaxySetupData&, unsigned int const);


struct MultiplayerLobbyData;

BOOST_CLASS_VERSION(MultiplayerLobbyData, 2);

template <typename Archive>
void serialize(Archive&, MultiplayerLobbyData&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, MultiplayerLobbyData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, MultiplayerLobbyData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, MultiplayerLobbyData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, MultiplayerLobbyData&, unsigned int const);


struct PlayerInfo;

template <typename Archive>
void serialize(Archive&, PlayerInfo&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, PlayerInfo&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, PlayerInfo&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, PlayerInfo&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, PlayerInfo&, unsigned int const);


struct PlayerSaveGameData;

BOOST_CLASS_VERSION(PlayerSaveGameData, 3);

template <typename Archive>
void serialize(Archive&, PlayerSaveGameData&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, PlayerSaveGameData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, PlayerSaveGameData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, PlayerSaveGameData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, PlayerSaveGameData&, unsigned int const);


struct PlayerSaveHeaderData;

template <typename Archive>
void serialize(Archive&, PlayerSaveHeaderData&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, PlayerSaveHeaderData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, PlayerSaveHeaderData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, PlayerSaveHeaderData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, PlayerSaveHeaderData&, unsigned int const);


struct SaveGamePreviewData;

template<typename Archive>
void serialize(Archive&, SaveGamePreviewData&, unsigned int const);

BOOST_CLASS_VERSION(SaveGamePreviewData, 5);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SaveGamePreviewData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SaveGamePreviewData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SaveGamePreviewData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SaveGamePreviewData&, unsigned int const);


struct PlayerSetupData;

BOOST_CLASS_VERSION(PlayerSetupData, 2);

template <typename Archive>
void serialize(Archive&, PlayerSetupData&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, PlayerSetupData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, PlayerSetupData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, PlayerSetupData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, PlayerSetupData&, unsigned int const);


struct PreviewInformation;

template<typename Archive>
void serialize(Archive&, PreviewInformation&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, PreviewInformation&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, PreviewInformation&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, PreviewInformation&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, PreviewInformation&, unsigned int const);


struct SaveGameEmpireData;

BOOST_CLASS_VERSION(SaveGameEmpireData, 3);

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


struct ServerSaveGameData;

template <typename Archive>
void serialize(Archive&, ServerSaveGameData&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, ServerSaveGameData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, ServerSaveGameData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, ServerSaveGameData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, ServerSaveGameData&, unsigned int const);


struct SinglePlayerSetupData;

template <typename Archive>
void serialize(Archive&, SinglePlayerSetupData&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SinglePlayerSetupData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SinglePlayerSetupData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SinglePlayerSetupData&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SinglePlayerSetupData&, unsigned int const);


class SpeciesManager;

BOOST_CLASS_VERSION(SpeciesManager, 1)

template <typename Archive>
void serialize(Archive&, SpeciesManager&, unsigned int const);

extern template FO_COMMON_API void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SpeciesManager&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SpeciesManager&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SpeciesManager&, unsigned int const);
extern template FO_COMMON_API void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SpeciesManager&, unsigned int const);


#endif
