#include "Message.h"

#include "../combat/CombatLogManager.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Supply.h"
#include "../Empire/Diplomacy.h"
#include "../util/Logger.h"
#include "../util/ModeratorAction.h"
#include "../util/SaveGamePreviewUtils.h"
#include "../universe/Species.h"
#include "../universe/Universe.h"
#include "../util/OptionsDB.h"
#include "../util/OrderSet.h"
#include "../util/Serialize.h"
#include "../util/ScopedTimer.h"
#include "../util/i18n.h"
#include "../util/Version.h"
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <zlib.h>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <map>


namespace {
    const std::string DUMMY_EMPTY_MESSAGE = "Lathanda";
}

////////////////////////////////////////////////
// Free Functions
////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const Message& msg) {
    os << "Message: "
       << msg.Type();

    os << " \"" << msg.Text() << "\"\n";

    return os;
}


////////////////////////////////////////////////
// Message
////////////////////////////////////////////////
void Message::Resize(std::size_t size) {
    m_message_size = size;
    m_message_text.clear();
    m_message_text.resize(size);
}

void Message::Swap(Message& rhs) noexcept {
    std::swap(m_type, rhs.m_type);
    std::swap(m_message_size, rhs.m_message_size);
    std::swap(m_message_text, rhs.m_message_text);
}

void Message::Reset() noexcept {
    m_type = MessageType::UNDEFINED;
    m_message_size = 0;
    m_message_text.clear();
}

bool operator==(const Message& lhs, const Message& rhs) noexcept
{ return lhs.Type() == rhs.Type() && lhs.Text() == rhs.Text(); }

void swap(Message& lhs, Message& rhs) noexcept
{ lhs.Swap(rhs); }

void BufferToHeader(const Message::HeaderBuffer& buffer, Message& message) {
    message.m_type = static_cast<Message::MessageType>(buffer[Message::Parts::TYPE]);
    message.m_message_size = buffer[Message::Parts::SIZE];
}

void HeaderToBuffer(const Message& message, Message::HeaderBuffer& buffer) {
    buffer[Message::Parts::TYPE] = int(message.Type());
    buffer[Message::Parts::SIZE] = int(message.Size());
}

////////////////////////////////////////////////
// Message named ctors
////////////////////////////////////////////////
Message ErrorMessage(const std::string& problem_stringtable_key, bool fatal, int player_id) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << boost::serialization::make_nvp("problem", problem_stringtable_key)
           << BOOST_SERIALIZATION_NVP(fatal)
           << BOOST_SERIALIZATION_NVP(player_id);
    }
    return Message{Message::MessageType::ERROR_MSG, std::move(os).str()};
}

Message ErrorMessage(const std::string& problem_stringtable_key, const std::string& unlocalized_info, bool fatal, int player_id) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << boost::serialization::make_nvp("problem", problem_stringtable_key)
           << BOOST_SERIALIZATION_NVP(fatal)
           << BOOST_SERIALIZATION_NVP(player_id);
        oa << BOOST_SERIALIZATION_NVP(unlocalized_info);
    }
    return Message{Message::MessageType::ERROR_MSG, std::move(os).str()};
}

Message HostSPGameMessage(const SinglePlayerSetupData& setup_data, const std::map<std::string, std::string>& dependencies) {
    std::ostringstream os;
    {
        std::string client_version_string = FreeOrionVersionString();
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(setup_data)
           << BOOST_SERIALIZATION_NVP(client_version_string)
           << BOOST_SERIALIZATION_NVP(dependencies);
    }
    return Message{Message::MessageType::HOST_SP_GAME, std::move(os).str()};
}

Message HostMPGameMessage(const std::string& host_player_name, const std::map<std::string, std::string>& dependencies)
{
    std::ostringstream os;
    {
        std::string client_version_string = FreeOrionVersionString();
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(host_player_name)
           << BOOST_SERIALIZATION_NVP(client_version_string)
           << BOOST_SERIALIZATION_NVP(dependencies);
    }
    return Message{Message::MessageType::HOST_MP_GAME, std::move(os).str()};
}

Message JoinGameMessage(const std::string& player_name,
                        Networking::ClientType client_type,
                        const std::map<std::string, std::string>& dependencies,
                        boost::uuids::uuid cookie) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        std::string client_version_string = FreeOrionVersionString();
        oa << BOOST_SERIALIZATION_NVP(player_name)
           << BOOST_SERIALIZATION_NVP(client_type)
           << BOOST_SERIALIZATION_NVP(client_version_string)
           << BOOST_SERIALIZATION_NVP(cookie)
           << BOOST_SERIALIZATION_NVP(dependencies);
    }
    return Message{Message::MessageType::JOIN_GAME, std::move(os).str()};
}

Message HostIDMessage(int host_player_id)
{ return Message{Message::MessageType::HOST_ID, std::to_string(host_player_id)}; }

Message GameStartMessage(bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const SpeciesManager& species,
                         CombatLogManager& combat_logs, const SupplyManager& supply,
                         const std::map<int, PlayerInfo>& players,
                         GalaxySetupData galaxy_setup_data,
                         bool use_binary_serialization, bool use_compression)
{
    std::ostringstream os;
    {
        using namespace boost::iostreams;
        zlib_params params{use_compression ? zlib::default_compression : zlib::no_compression};
        filtering_ostream zos;
        zos.push(zlib_compressor(params));
        zos.push(os);
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(zos);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GlobalSerializationEncodingForEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            SerializeIncompleteLogs(oa, combat_logs, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = false;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            galaxy_setup_data.encoding_empire = empire_id;
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        } else {
            freeorion_xml_oarchive oa(zos);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GlobalSerializationEncodingForEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            SerializeIncompleteLogs(oa, combat_logs, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = false;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            galaxy_setup_data.encoding_empire = empire_id;
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
        if (!zos.strict_sync())
            zos.reset();
    }
    return Message{Message::MessageType::GAME_START, std::move(os).str()};
}

Message GameStartMessage(bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const SpeciesManager& species,
                         CombatLogManager& combat_logs, const SupplyManager& supply,
                         const std::map<int, PlayerInfo>& players,
                         const OrderSet& orders, const SaveGameUIData& ui_data,
                         GalaxySetupData galaxy_setup_data,
                         bool use_binary_serialization, bool use_compression)
{
    std::ostringstream os;
    {
        using namespace boost::iostreams;
        zlib_params params{use_compression ? zlib::default_compression : zlib::no_compression};
        filtering_ostream zos;
        zos.push(zlib_compressor(params));
        zos.push(os);
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(zos);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GlobalSerializationEncodingForEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            SerializeIncompleteLogs(oa, combat_logs, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = true;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            oa << boost::serialization::make_nvp("ui_data", ui_data);
            bool save_state_string_available = false;
            oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
            galaxy_setup_data.encoding_empire = empire_id;
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        } else {
            freeorion_xml_oarchive oa(zos);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GlobalSerializationEncodingForEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            SerializeIncompleteLogs(oa, combat_logs, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = true;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            oa << boost::serialization::make_nvp("ui_data", ui_data);
            bool save_state_string_available = false;
            oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
            galaxy_setup_data.encoding_empire = empire_id;
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
        if (!zos.strict_sync())
            zos.reset();
    }
    return Message{Message::MessageType::GAME_START, std::move(os).str()};
}

Message GameStartMessage(bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const SpeciesManager& species,
                         CombatLogManager& combat_logs, const SupplyManager& supply,
                         const std::map<int, PlayerInfo>& players,
                         const OrderSet& orders, const std::string* save_state_string,
                         GalaxySetupData galaxy_setup_data,
                         bool use_binary_serialization, bool use_compression)

{
    std::ostringstream os;
    {
        using namespace boost::iostreams;
        zlib_params params{use_compression ? zlib::default_compression : zlib::no_compression};
        filtering_ostream zos;
        zos.push(zlib_compressor(params));
        zos.push(os);
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(zos);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GlobalSerializationEncodingForEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            SerializeIncompleteLogs(oa, combat_logs, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = false;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            bool save_state_string_available = (save_state_string != nullptr);
            oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
            if (save_state_string)
                oa << boost::serialization::make_nvp("save_state_string", *save_state_string);
            galaxy_setup_data.encoding_empire = empire_id;
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        } else {
            freeorion_xml_oarchive oa(zos);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GlobalSerializationEncodingForEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            SerializeIncompleteLogs(oa, combat_logs, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = false;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            bool save_state_string_available = (save_state_string != nullptr);
            oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
            if (save_state_string)
                oa << boost::serialization::make_nvp("save_state_string", *save_state_string);
            galaxy_setup_data.encoding_empire = empire_id;
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
        if (!zos.strict_sync())
            zos.reset();
    }
    return Message{Message::MessageType::GAME_START, std::move(os).str()};
}

Message HostSPAckMessage(int player_id)
{ return Message{Message::MessageType::HOST_SP_GAME, std::to_string(player_id)}; }

Message HostMPAckMessage(int player_id)
{ return Message{Message::MessageType::HOST_MP_GAME, std::to_string(player_id)}; }

Message JoinAckMessage(int player_id, boost::uuids::uuid cookie)
{
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(player_id)
           << BOOST_SERIALIZATION_NVP(cookie);
    }
    return Message{Message::MessageType::JOIN_GAME, std::move(os).str()};
}

Message TurnOrdersMessage(const OrderSet& orders, const SaveGameUIData& ui_data) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        Serialize(oa, orders);
        bool ui_data_available = true;
        bool save_state_string_available = false;
        oa << BOOST_SERIALIZATION_NVP(ui_data_available)
           << BOOST_SERIALIZATION_NVP(ui_data)
           << BOOST_SERIALIZATION_NVP(save_state_string_available);
    }
    return Message{Message::MessageType::TURN_ORDERS, std::move(os).str()};
}

Message TurnOrdersMessage(const OrderSet& orders, const std::string& save_state_string) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        Serialize(oa, orders);
        bool ui_data_available = false;
        bool save_state_string_available = true;
        oa << BOOST_SERIALIZATION_NVP(ui_data_available)
           << BOOST_SERIALIZATION_NVP(save_state_string_available)
           << BOOST_SERIALIZATION_NVP(save_state_string);
    }
    return Message{Message::MessageType::TURN_ORDERS, std::move(os).str()};
}

Message TurnPartialOrdersMessage(const std::pair<OrderSet, std::set<int>>& orders_updates) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        Serialize(oa, orders_updates.first);
        oa << boost::serialization::make_nvp("deleted", orders_updates.second);
    }
    return Message{Message::MessageType::TURN_PARTIAL_ORDERS, std::move(os).str()};
}

Message TurnTimeoutMessage(int timeout_remaining)
{ return Message{Message::MessageType::TURN_TIMEOUT, std::to_string(timeout_remaining)}; }

Message TurnProgressMessage(Message::TurnProgressPhase phase_id) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(phase_id);
    }
    return Message{Message::MessageType::TURN_PROGRESS, std::move(os).str()};
}

Message PlayerStatusMessage(Message::PlayerStatus player_status,
                            int about_empire_id)
{
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(player_status)
           << BOOST_SERIALIZATION_NVP(about_empire_id);
    }
    return Message{Message::MessageType::PLAYER_STATUS, std::move(os).str()};
}

Message TurnUpdateMessage(int empire_id, int current_turn,
                          const EmpireManager& empires, const Universe& universe,
                          const SpeciesManager& species, CombatLogManager& combat_logs,
                          const SupplyManager& supply,
                          const std::map<int, PlayerInfo>& players,
                          bool use_binary_serialization, bool use_compression)
{
    std::ostringstream os;
    {
        using namespace boost::iostreams;
        zlib_params params{use_compression ? zlib::default_compression : zlib::no_compression};
        filtering_ostream zos;
        zos.push(zlib_compressor(params));
        zos.push(os);
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(zos);
            GlobalSerializationEncodingForEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(current_turn);
            oa << BOOST_SERIALIZATION_NVP(empires);
            oa << BOOST_SERIALIZATION_NVP(species);
            SerializeIncompleteLogs(oa, combat_logs, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            oa << BOOST_SERIALIZATION_NVP(players);
        } else {
            freeorion_xml_oarchive oa(zos);
            GlobalSerializationEncodingForEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(current_turn)
               << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            SerializeIncompleteLogs(oa, combat_logs, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            oa << BOOST_SERIALIZATION_NVP(players);
        }
        if (!zos.strict_sync())
            zos.reset();
    }
    return Message{Message::MessageType::TURN_UPDATE, std::move(os).str()};
}

Message TurnPartialUpdateMessage(int empire_id, const Universe& universe,
                                 bool use_binary_serialization, bool use_compression) {
    std::ostringstream os;
    {
        using namespace boost::iostreams;
        zlib_params params{use_compression ? zlib::default_compression : zlib::no_compression};
        filtering_ostream zos;
        zos.push(zlib_compressor(params));
        zos.push(os);
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(zos);
            GlobalSerializationEncodingForEmpire() = empire_id;
            Serialize(oa, universe);
        } else {
            freeorion_xml_oarchive oa(zos);
            GlobalSerializationEncodingForEmpire() = empire_id;
            Serialize(oa, universe);
        }
        if (!zos.strict_sync())
            zos.reset();
    }
    return Message{Message::MessageType::TURN_PARTIAL_UPDATE, std::move(os).str()};
}

Message HostSaveGameInitiateMessage(std::string filename)
{ return Message{Message::MessageType::SAVE_GAME_INITIATE, std::move(filename)}; }

Message ServerSaveGameCompleteMessage(const std::string& save_filename, int bytes_written) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(save_filename)
           << BOOST_SERIALIZATION_NVP(bytes_written);
    }
    return Message{Message::MessageType::SAVE_GAME_COMPLETE, std::move(os).str()};
}

Message DiplomacyMessage(const DiplomaticMessage& diplo_message) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(diplo_message);
    }
    return Message{Message::MessageType::DIPLOMACY, std::move(os).str()};
}

Message DiplomaticStatusMessage(const DiplomaticStatusUpdateInfo& diplo_update) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(diplo_update.empire1_id)
           << BOOST_SERIALIZATION_NVP(diplo_update.empire2_id)
           << BOOST_SERIALIZATION_NVP(diplo_update.diplo_status);
    }
    return Message{Message::MessageType::DIPLOMATIC_STATUS, std::move(os).str()};
}

Message EndGameMessage(Message::EndGameReason reason,
                       const std::string& reason_player_name)
{
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(reason)
           << BOOST_SERIALIZATION_NVP(reason_player_name);
    }
    return Message{Message::MessageType::END_GAME, std::move(os).str()};
}

Message AIEndGameAcknowledgeMessage()
{ return Message{Message::MessageType::AI_END_GAME_ACK, DUMMY_EMPTY_MESSAGE}; }

Message ModeratorActionMessage(const Moderator::ModeratorAction& action) {
    std::ostringstream os;
    {
        const Moderator::ModeratorAction* mod_action = &action;
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(mod_action);
    }
    return Message{Message::MessageType::MODERATOR_ACTION, std::move(os).str()};
}

Message ShutdownServerMessage()
{ return Message{Message::MessageType::SHUT_DOWN_SERVER, DUMMY_EMPTY_MESSAGE}; }

/** requests previews of savefiles from server synchronously */
Message RequestSavePreviewsMessage(std::string relative_directory)
{ return Message{Message::MessageType::REQUEST_SAVE_PREVIEWS, std::move(relative_directory)}; }

/** returns the savegame previews to the client */
Message DispatchSavePreviewsMessage(const PreviewInformation& previews) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(previews);
    }
    return Message{Message::MessageType::DISPATCH_SAVE_PREVIEWS, std::move(os).str()};
}

Message RequestCombatLogsMessage(const std::vector<int>& ids) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(ids);
    }
    return Message{Message::MessageType::REQUEST_COMBAT_LOGS, std::move(os).str()};
}

Message DispatchCombatLogsMessage(const std::vector<std::pair<int, const CombatLog>>& logs,
                                  bool use_binary_serialization, bool use_compression)
{
    std::ostringstream os;
    {
        try {
            using namespace boost::iostreams;
            zlib_params params{use_compression ? zlib::default_compression : zlib::no_compression};
            filtering_ostream zos;
            zos.push(zlib_compressor(params));
            zos.push(os);
            if (use_binary_serialization) {
                freeorion_bin_oarchive oa(zos);
                oa << BOOST_SERIALIZATION_NVP(logs);
            } else {
                freeorion_xml_oarchive oa(zos);
                oa << BOOST_SERIALIZATION_NVP(logs);
            }
            if (!zos.strict_sync())
                zos.reset();
        } catch (const std::exception& e) {
            ErrorLogger() << "Caught exception serializing combat logs: " << e.what();
        }
    }

    return Message{Message::MessageType::DISPATCH_COMBAT_LOGS, std::move(os).str()};
}

Message LoggerConfigMessage(int sender, const std::vector<std::tuple<std::string, std::string, LogLevel>>& options) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        std::size_t size = options.size();
        oa << BOOST_SERIALIZATION_NVP(size);
        for (const auto& option_tuple : options) {
            const auto& [option, name, value] = option_tuple;
            oa << BOOST_SERIALIZATION_NVP(option);
            oa << BOOST_SERIALIZATION_NVP(name);
            oa << BOOST_SERIALIZATION_NVP(value);
        }
    }
    return Message{Message::MessageType::LOGGER_CONFIG, std::move(os).str()};
}

////////////////////////////////////////////////
// Multiplayer Lobby Message named ctors
////////////////////////////////////////////////
Message LobbyUpdateMessage(const MultiplayerLobbyData& lobby_data) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(lobby_data);
    }
    return Message{Message::MessageType::LOBBY_UPDATE, std::move(os).str()};
}

Message ServerLobbyUpdateMessage(const MultiplayerLobbyData& lobby_data) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(lobby_data);
    }
    return Message{Message::MessageType::LOBBY_UPDATE, std::move(os).str()};
}

Message ChatHistoryMessage(const std::vector<std::reference_wrapper<const ChatHistoryEntity>>& chat_history,
                           bool use_compression) {
    std::ostringstream os;
    {
        using namespace boost::iostreams;
        zlib_params params{use_compression ? zlib::default_compression : zlib::no_compression};
        filtering_ostream zos;
        zos.push(zlib_compressor(params));
        zos.push(os);
        {
            // Nested block ensures archive is closed and completes writing before
            // the filtering_ostream is rendered unusable by reset(). 
            freeorion_xml_oarchive oa(zos);
            std::size_t size = chat_history.size();
            oa << BOOST_SERIALIZATION_NVP(size);
            for (const auto &elem : chat_history) {
                oa << boost::serialization::make_nvp(BOOST_PP_STRINGIZE(elem), elem.get());
            }
        }
        if (!zos.strict_sync())
            zos.reset();
    }
    return Message{Message::MessageType::CHAT_HISTORY, std::move(os).str()};
}

Message PlayerChatMessage(const std::string& data, std::set<int> recipients, bool pm) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(recipients)
           << BOOST_SERIALIZATION_NVP(data)
           << BOOST_SERIALIZATION_NVP(pm);
    }
    return Message{Message::MessageType::PLAYER_CHAT, std::move(os).str()};
}

Message ServerPlayerChatMessage(int sender, const boost::posix_time::ptime& timestamp,
                                const std::string& data, bool pm)
{
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(sender)
           << BOOST_SERIALIZATION_NVP(timestamp)
           << BOOST_SERIALIZATION_NVP(data)
           << BOOST_SERIALIZATION_NVP(pm);
    }
    return Message{Message::MessageType::PLAYER_CHAT, std::move(os).str()};
}

Message StartMPGameMessage()
{ return Message{Message::MessageType::START_MP_GAME, DUMMY_EMPTY_MESSAGE}; }

Message ContentCheckSumMessage(const SpeciesManager& species) {
    auto checksums = CheckSumContent(species);

    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(checksums);
    }
    return Message{Message::MessageType::CHECKSUM, std::move(os).str()};
}

Message AuthRequestMessage(const std::string& player_name, const std::string& auth) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(player_name)
           << BOOST_SERIALIZATION_NVP(auth);
    }
    return Message{Message::MessageType::AUTH_REQUEST, std::move(os).str()};
}

Message AuthResponseMessage(const std::string& player_name, const std::string& auth) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(player_name)
           << BOOST_SERIALIZATION_NVP(auth);
    }
    return Message{Message::MessageType::AUTH_RESPONSE, std::move(os).str()};
}

Message SetAuthorizationRolesMessage(Networking::AuthRoles roles)
{ return Message{Message::MessageType::SET_AUTH_ROLES, roles.Text()}; }

Message EliminateSelfMessage()
{ return Message{Message::MessageType::ELIMINATE_SELF, DUMMY_EMPTY_MESSAGE}; }

Message UnreadyMessage()
{ return Message{Message::MessageType::UNREADY, DUMMY_EMPTY_MESSAGE}; }

Message PlayerInfoMessage(const std::map<int, PlayerInfo>& players) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(players);
    }
    return Message{Message::MessageType::PLAYER_INFO, std::move(os).str()};
}

Message AutoTurnMessage(int turns_count)
{ return Message{Message::MessageType::AUTO_TURN, std::to_string(turns_count)}; }

Message RevertOrdersMessage()
{ return Message{Message::MessageType::REVERT_ORDERS, DUMMY_EMPTY_MESSAGE}; }


////////////////////////////////////////////////
// Message data extractors
////////////////////////////////////////////////
void ExtractErrorMessageData(const Message& msg, int& player_id, std::string& problem_key,
                             std::string& unlocalized_info, bool& fatal)
{
    bool got_stuff_before_unlocalized = false;
    try {
        std::istringstream is(msg.Text() /*+ "</boost_serialization>"*/); // additional closing tag needed to prevent crash in freeorion_xml_iarchive destructor for incomplete input
        freeorion_xml_iarchive ia(is);

        ia >> boost::serialization::make_nvp("problem", problem_key);
        ia >> BOOST_SERIALIZATION_NVP(fatal);
        ia >> BOOST_SERIALIZATION_NVP(player_id);

        got_stuff_before_unlocalized = true;
        // may fail if message didn't have any unlocalized info specified
        ia >> BOOST_SERIALIZATION_NVP(unlocalized_info);

        // this scope must exactly match scope of the archive see: https://github.com/boostorg/serialization/issues/220#issuecomment-690786015
    } catch (const std::exception& err) {
        if (!got_stuff_before_unlocalized) {
            ErrorLogger() << "ExtractErrorMessageData(const Message& msg, std::string& problem_key, std::string& unlocalized_info, bool& fatal) failed!  Message:\n"
                          << msg.Text() << "\n"
                          << "Error: " << err.what();
            problem_key = UserStringNop("SERVER_MESSAGE_NOT_UNDERSTOOD");
        } // else, not a problem; old format error message without the unlocalized info
    } catch (...) {
        ErrorLogger() << "ExtractErrorMessageData(const Message& msg, std::string& problem_key, std::string& unlocalized_info, bool& fatal) failed!  Message:\n"
                      << msg.Text() << "\nError unknown.";
        problem_key = UserStringNop("SERVER_MESSAGE_NOT_UNDERSTOOD");
    }
}

void ExtractHostMPGameMessageData(const Message& msg, std::string& host_player_name, std::string& client_version_string, std::map<std::string, std::string>& dependencies) {
    dependencies.clear();
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(host_player_name)
           >> BOOST_SERIALIZATION_NVP(client_version_string)
           >> BOOST_SERIALIZATION_NVP(dependencies);
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractHostMPGameMessageData(const Message& msg, std::string& host_player_name, std::string& client_version_string) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractLobbyUpdateMessageData(const Message& msg, MultiplayerLobbyData& lobby_data) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(lobby_data);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractLobbyUpdateMessageData(const Message& msg, MultiplayerLobbyData& lobby_data) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractChatHistoryMessage(const Message& msg, std::vector<ChatHistoryEntity>& chat_history) {
    try {
        std::istringstream is(msg.Text());
        boost::iostreams::filtering_istream zis;
        zis.push(boost::iostreams::zlib_decompressor());
        zis.push(is);
        freeorion_xml_iarchive ia(zis);
        std::size_t size;
        ia >> BOOST_SERIALIZATION_NVP(size);
        chat_history.clear();
        chat_history.reserve(size);
        for (std::size_t ii = 0; ii < size; ++ii) {
            ChatHistoryEntity elem;
            ia >> BOOST_SERIALIZATION_NVP(elem);
            chat_history.push_back(elem);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractChatHistoryMessage(const Message& msg, std::vector<ChatHistoryEntity>& chat_history) failed!  Message:]n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractPlayerChatMessageData(const Message& msg, std::set<int>& recipients, std::string& data, bool& pm) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(recipients)
           >> BOOST_SERIALIZATION_NVP(data)
           >> BOOST_SERIALIZATION_NVP(pm);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractPlayerChatMessageData(const Message& msg, int& receiver, std::string& data) failed! Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractServerPlayerChatMessageData(const Message& msg,
                                        int& sender, boost::posix_time::ptime& timestamp,
                                        std::string& data, bool& pm)
{
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(sender)
           >> BOOST_SERIALIZATION_NVP(timestamp)
           >> BOOST_SERIALIZATION_NVP(data)
           >> BOOST_SERIALIZATION_NVP(pm);
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractServerPlayerChatMessageData(const Message& msg, int& sender, std::string& data) failed! Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractGameStartMessageData(const Message& msg, bool& single_player_game, int& empire_id, int& current_turn,
                                 EmpireManager& empires, Universe& universe, SpeciesManager& species,
                                 CombatLogManager& combat_logs, SupplyManager& supply,
                                 std::map<int, PlayerInfo>& players, OrderSet& orders, bool& loaded_game_data,
                                 bool& ui_data_available, SaveGameUIData& ui_data, bool& save_state_string_available,
                                 std::string& save_state_string, GalaxySetupData& galaxy_setup_data)
{
    ExtractGameStartMessageData(msg.Text(), single_player_game, empire_id, current_turn, empires, universe,
                                species, combat_logs, supply, players, orders, loaded_game_data, ui_data_available,
                                ui_data, save_state_string_available, save_state_string, galaxy_setup_data);
}

void ExtractGameStartMessageData(std::string text, bool& single_player_game, int& empire_id, int& current_turn,
                                 EmpireManager& empires, Universe& universe, SpeciesManager& species,
                                 CombatLogManager& combat_logs, SupplyManager& supply,
                                 std::map<int, PlayerInfo>& players, OrderSet& orders, bool& loaded_game_data,
                                 bool& ui_data_available, SaveGameUIData& ui_data, bool& save_state_string_available,
                                 std::string& save_state_string, GalaxySetupData& galaxy_setup_data)
{
    try {
        bool try_xml = false;
        bool did_some_binary_deserialization = false;
        try {
            // first attempt binary deserialziation
            std::istringstream is(text);
            boost::iostreams::filtering_istream zis;
            zis.push(boost::iostreams::zlib_decompressor());
            zis.push(is);

            freeorion_bin_iarchive ia(zis);
            ia >> BOOST_SERIALIZATION_NVP(single_player_game)
               >> BOOST_SERIALIZATION_NVP(empire_id)
               >> BOOST_SERIALIZATION_NVP(current_turn);
            GlobalSerializationEncodingForEmpire() = empire_id;

            did_some_binary_deserialization = true; // got some binary data, so don't retry as XML even if following deserialization fails

            ScopedTimer deserialize_timer;
            ia >> BOOST_SERIALIZATION_NVP(empires);
            DebugLogger() << "ExtractGameStartMessage empire deserialization time " << deserialize_timer.DurationString();

            ia >> BOOST_SERIALIZATION_NVP(species);
            combat_logs.Clear();    // only needed when loading new game, not when incrementally serializing logs on turn update
            SerializeIncompleteLogs(ia, combat_logs, 1);
            ia >> BOOST_SERIALIZATION_NVP(supply);

            deserialize_timer.restart();
            Deserialize(ia, universe);
            DebugLogger() << "ExtractGameStartMessage universe deserialization time " << deserialize_timer.DurationString();


            ia >> BOOST_SERIALIZATION_NVP(players)
               >> BOOST_SERIALIZATION_NVP(loaded_game_data);
            if (loaded_game_data) {
                Deserialize(ia, orders);
                DebugLogger() << "deserialized orders: " << orders.size();
                ia >> BOOST_SERIALIZATION_NVP(ui_data_available);
                DebugLogger() << (ui_data_available ? "have UI data" : "do not have UI data");
                if (ui_data_available)
                    ia >> BOOST_SERIALIZATION_NVP(ui_data);
                ia >> BOOST_SERIALIZATION_NVP(save_state_string_available);
                DebugLogger() << (save_state_string_available ? "have save state string" : "do not have save state string");
                if (save_state_string_available) {
                    ia >> BOOST_SERIALIZATION_NVP(save_state_string);
                    DebugLogger() << "save state string size:" << save_state_string.size();
                }
            } else {
                ui_data_available = false;
                save_state_string_available = false;
            }
            ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        } catch (...) {
            if (did_some_binary_deserialization) {
                ErrorLogger() << "Deserialization error after partially-done binary deserialization";
                throw;
            }
            try_xml = true; // try XML deserialization if no binary data was deserialized
        }
        if (try_xml) {
            // if binary deserialization failed, try more-portable XML deserialization
            std::istringstream is(text);
            boost::iostreams::filtering_istream zis;
            zis.push(boost::iostreams::zlib_decompressor());
            zis.push(is);

            freeorion_xml_iarchive ia(zis);
            ia >> BOOST_SERIALIZATION_NVP(single_player_game)
               >> BOOST_SERIALIZATION_NVP(empire_id)
               >> BOOST_SERIALIZATION_NVP(current_turn);
            GlobalSerializationEncodingForEmpire() = empire_id;

            ScopedTimer deserialize_timer;
            ia >> BOOST_SERIALIZATION_NVP(empires);

            DebugLogger() << "ExtractGameStartMessage empire deserialization time " << deserialize_timer.DurationString();

            ia >> BOOST_SERIALIZATION_NVP(species);
            combat_logs.Clear();    // only needed when loading new game, not when incrementally serializing logs on turn update
            SerializeIncompleteLogs(ia, combat_logs, 1);
            ia >> BOOST_SERIALIZATION_NVP(supply);

            deserialize_timer.restart();
            Deserialize(ia, universe);
            DebugLogger() << "ExtractGameStartMessage universe deserialization time " << deserialize_timer.DurationString();


            ia >> BOOST_SERIALIZATION_NVP(players)
               >> BOOST_SERIALIZATION_NVP(loaded_game_data);
            TraceLogger() << "ExtractGameStartMessage players and loaded_game_data=" << loaded_game_data << " deserialization time " << deserialize_timer.DurationString();
            if (loaded_game_data) {
                Deserialize(ia, orders);
                TraceLogger() << "ExtractGameStartMessage orders deserialization time " << deserialize_timer.DurationString();
                ia >> BOOST_SERIALIZATION_NVP(ui_data_available);
                if (ui_data_available)
                    ia >> BOOST_SERIALIZATION_NVP(ui_data);
                TraceLogger() << "ExtractGameStartMessage UI data " << ui_data_available << " deserialization time " << deserialize_timer.DurationString();
                ia >> BOOST_SERIALIZATION_NVP(save_state_string_available);
                if (save_state_string_available)
                    ia >> BOOST_SERIALIZATION_NVP(save_state_string);
                TraceLogger() << "ExtractGameStartMessage save state " << save_state_string_available << " deserialization time " << deserialize_timer.DurationString();
            } else {
                ui_data_available = false;
                save_state_string_available = false;
            }
            ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);
            TraceLogger() << "ExtractGameStartMessage galaxy setup data deserialization time " << deserialize_timer.DurationString();
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractGameStartMessageData(...) failed!  Message text length: " << text.size()
                      << "\nError: " << err.what();
        throw err;
    }
}

void ExtractJoinGameMessageData(const Message& msg, std::string& player_name,
                                Networking::ClientType& client_type,
                                std::string& version_string,
                                std::map<std::string, std::string>& dependencies,
                                boost::uuids::uuid& cookie)
{
    DebugLogger() << "ExtractJoinGameMessageData() from " << player_name
                  << " client type " << client_type;
    dependencies.clear();
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(player_name)
           >> BOOST_SERIALIZATION_NVP(client_type)
           >> BOOST_SERIALIZATION_NVP(version_string)
           >> BOOST_SERIALIZATION_NVP(cookie)
           >> BOOST_SERIALIZATION_NVP(dependencies);
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractJoinGameMessageData(const Message& msg, std::string& player_name, "
                      << "Networking::ClientType client_type, std::string& version_string) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractJoinAckMessageData(const Message& msg, int& player_id,
                               boost::uuids::uuid& cookie)
{
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(player_id)
           >> BOOST_SERIALIZATION_NVP(cookie);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractJoinAckMessageData(const Message& msg, int& player_id, "
                      << "boost::uuids::uuid& cookie) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }

}

void ExtractTurnOrdersMessageData(const Message& msg, OrderSet& orders, bool& ui_data_available,
                                  SaveGameUIData& ui_data, bool& save_state_string_available,
                                  std::string& save_state_string)
{
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        DebugLogger() << "deserializing orders";
        Deserialize(ia, orders);
        DebugLogger() << "checking for ui data";
        ia >> BOOST_SERIALIZATION_NVP(ui_data_available);
        if (ui_data_available) {
            DebugLogger() << "deserializing UI data";
            ia >> BOOST_SERIALIZATION_NVP(ui_data);
        }
        DebugLogger() << "checking for save state string";
        ia >> BOOST_SERIALIZATION_NVP(save_state_string_available);
        if (save_state_string_available) {
            DebugLogger() << "deserializing save state string";
            ia >> BOOST_SERIALIZATION_NVP(save_state_string);
        }

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractTurnOrdersMessageData(const Message& msg, ...) failed! Message probably long, so not outputting to log.\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractTurnPartialOrdersMessageData(const Message& msg, OrderSet& added, std::set<int>& deleted) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        DebugLogger() << "deserializing partial orders";
        Deserialize(ia, added);
        ia >> BOOST_SERIALIZATION_NVP(deleted);
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractTurnPartialOrdersMessageData(const Message& msg) failed! Message probably long, so not outputting to log.\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractTurnUpdateMessageData(const Message& msg, int empire_id, int& current_turn, EmpireManager& empires,
                                  Universe& universe, SpeciesManager& species, CombatLogManager& combat_logs,
                                  SupplyManager& supply, std::map<int, PlayerInfo>& players)
{
    ExtractTurnUpdateMessageData(msg.Text(), empire_id, current_turn, empires,
                                 universe, species, combat_logs, supply, players);
}

void ExtractTurnUpdateMessageData(std::string text, int empire_id, int& current_turn, EmpireManager& empires,
                                  Universe& universe, SpeciesManager& species, CombatLogManager& combat_logs,
                                  SupplyManager& supply, std::map<int, PlayerInfo>& players)
{
    try {
        ScopedTimer timer("Turn Update Unpacking");

        try {
            // first attempt binary deserialization
            std::istringstream is(text);
            boost::iostreams::filtering_istream zis;
            zis.push(boost::iostreams::zlib_decompressor());
            zis.push(is);

            freeorion_bin_iarchive ia(zis);
            GlobalSerializationEncodingForEmpire() = empire_id;
            ia >> BOOST_SERIALIZATION_NVP(current_turn)
               >> BOOST_SERIALIZATION_NVP(empires)
               >> BOOST_SERIALIZATION_NVP(species);
            SerializeIncompleteLogs(ia, combat_logs, 1);
            ia >> BOOST_SERIALIZATION_NVP(supply);
            Deserialize(ia, universe);
            ia >> BOOST_SERIALIZATION_NVP(players);
        } catch (...) {
            // try again with more-portable XML deserialization
            std::istringstream is(text);
            boost::iostreams::filtering_istream zis;
            zis.push(boost::iostreams::zlib_decompressor());
            zis.push(is);

            freeorion_xml_iarchive ia(zis);
            GlobalSerializationEncodingForEmpire() = empire_id;
            ia >> BOOST_SERIALIZATION_NVP(current_turn)
               >> BOOST_SERIALIZATION_NVP(empires)
               >> BOOST_SERIALIZATION_NVP(species);
            SerializeIncompleteLogs(ia, combat_logs, 1);
            ia >> BOOST_SERIALIZATION_NVP(supply);
            Deserialize(ia, universe);
            ia >> BOOST_SERIALIZATION_NVP(players);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractTurnUpdateMessageData(...) failed!  Message probably long, so not outputting to log.\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractTurnPartialUpdateMessageData(const Message& msg, int empire_id, Universe& universe) {
    try {
        ScopedTimer timer("Mid Turn Update Unpacking");

        bool try_xml = false;
        try {
            // first attempt binary deserialization
            std::istringstream is(msg.Text());
            boost::iostreams::filtering_istream zis;
            zis.push(boost::iostreams::zlib_decompressor());
            zis.push(is);

            freeorion_bin_iarchive ia(zis);
            GlobalSerializationEncodingForEmpire() = empire_id;
            Deserialize(ia, universe);
        } catch (...) {
            try_xml = true;
        }
        if (try_xml) {
            // try again with more-portable XML deserialization
            std::istringstream is(msg.Text());
            boost::iostreams::filtering_istream zis;
            zis.push(boost::iostreams::zlib_decompressor());
            zis.push(is);

            freeorion_xml_iarchive ia(zis);
            GlobalSerializationEncodingForEmpire() = empire_id;
            Deserialize(ia, universe);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtracturnPartialUpdateMessageData(...) failed!  Message probably long, so not outputting to log.\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractTurnProgressMessageData(const Message& msg, Message::TurnProgressPhase& phase_id) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(phase_id);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractTurnProgressMessageData(const Message& msg, Message::TurnProgressPhase& phase_id) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractPlayerStatusMessageData(const Message& msg, Message::PlayerStatus& status, int& about_empire_id) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(status)
           >> BOOST_SERIALIZATION_NVP(about_empire_id);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractPlayerStatusMessageData(const Message& msg, Message::PlayerStatus&, int& about_empire_id) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractHostSPGameMessageData(const Message& msg, SinglePlayerSetupData& setup_data, std::string& client_version_string, std::map<std::string, std::string>& dependencies) {
    dependencies.clear();
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(setup_data)
           >> BOOST_SERIALIZATION_NVP(client_version_string)
           >> BOOST_SERIALIZATION_NVP(dependencies);
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractHostSPGameMessageData(const Message& msg, SinglePlayerSetupData& setup_data, std::string& client_version_string) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractEndGameMessageData(const Message& msg, Message::EndGameReason& reason, std::string& reason_player_name) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(reason)
           >> BOOST_SERIALIZATION_NVP(reason_player_name);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractEndGameMessageData(const Message& msg, Message::EndGameReason& reason, "
                      << "std::string& reason_player_name) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractModeratorActionMessageData(const Message& msg,
                                       std::unique_ptr<Moderator::ModeratorAction>& mod_action)
{
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        Moderator::ModeratorAction* action = nullptr;
        ia >> BOOST_SERIALIZATION_NVP(action);
        mod_action.reset(action);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractModeratorActionMessageData(const Message& msg, Moderator::ModeratorAction& mod_act) "
                      << "failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
    }
}

void ExtractDiplomacyMessageData(const Message& msg, DiplomaticMessage& diplo_message) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(diplo_message);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractDiplomacyMessageData(const Message& msg, DiplomaticMessage& "
                      << "diplo_message) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractDiplomaticStatusMessageData(const Message& msg, DiplomaticStatusUpdateInfo& diplo_update) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(diplo_update.empire1_id)
           >> BOOST_SERIALIZATION_NVP(diplo_update.empire2_id)
           >> BOOST_SERIALIZATION_NVP(diplo_update.diplo_status);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractDiplomaticStatusMessageData(const Message& msg, DiplomaticStatusUpdate& diplo_update) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractRequestSavePreviewsMessageData(const Message& msg, std::string& directory)
{ directory = msg.Text(); }

void ExtractDispatchSavePreviewsMessageData(const Message& msg, PreviewInformation& previews) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(previews);

    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractDispatchSavePreviewsMessageData(const Message& msg, PreviewInformation& previews) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

FO_COMMON_API void ExtractServerSaveGameCompleteMessageData(const Message& msg, std::string& save_filename, int& bytes_written) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(save_filename)
           >> BOOST_SERIALIZATION_NVP(bytes_written);

    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractServerSaveGameCompleteServerSaveGameCompleteMessageData(const Message& msg, std::string& save_filename, int& bytes_written) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

FO_COMMON_API void ExtractRequestCombatLogsMessageData(const Message& msg, std::vector<int>& ids) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(ids);
    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractRequestCombatLogMessageData(const Message& msg, std::vector<int>& ids) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

FO_COMMON_API void ExtractDispatchCombatLogsMessageData(
    const Message& msg, std::vector<std::pair<int, CombatLog>>& logs)
{
    try {
        bool try_xml = false;
        try {
            // first attempt binary deserialization
            std::istringstream is(msg.Text());
            boost::iostreams::filtering_istream zis;
            zis.push(boost::iostreams::zlib_decompressor());
            zis.push(is);

            freeorion_bin_iarchive ia(zis);
            ia >> BOOST_SERIALIZATION_NVP(logs);
        } catch (...) {
            try_xml = true;
        }
        if (try_xml) {
            // try again with more-portable XML deserialization
            std::istringstream is(msg.Text());
            boost::iostreams::filtering_istream zis;
            zis.push(boost::iostreams::zlib_decompressor());
            zis.push(is);

            freeorion_xml_iarchive ia(zis);
            ia >> BOOST_SERIALIZATION_NVP(logs);
        }
    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractDispatchCombatLogMessageData(const Message& msg, std::vector<std::pair<int, const CombatLog&>>& logs) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

FO_COMMON_API std::vector<std::tuple<std::string, std::string, LogLevel>>
    ExtractLoggerConfigMessageData(const Message& msg)
{
    std::vector<std::tuple<std::string, std::string, LogLevel>> options;

    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        std::size_t size;
        ia >> BOOST_SERIALIZATION_NVP(size);
        options.reserve(size);
        for (std::size_t ii = 0; ii < size; ++ii) {
            std::string option;
            std::string name;
            LogLevel level;
            ia >> BOOST_SERIALIZATION_NVP(option);
            ia >> BOOST_SERIALIZATION_NVP(name);
            ia >> BOOST_SERIALIZATION_NVP(level);
            options.emplace_back(std::move(option), std::move(name), level);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractDispatchCombatLogMessageData(const Message& msg, std::vector<std::pair<int, const CombatLog&> >& logs) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
    return options;
}

void ExtractContentCheckSumMessageData(const Message& msg, std::map<std::string, unsigned int>& checksums) {
    checksums.clear();
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(checksums);

    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractContentCheckSumMessageData(const Message& msg, std::string& save_filename, std::map<std::string, unsigned int>) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractAuthRequestMessageData(const Message& msg, std::string& player_name, std::string& auth) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(player_name)
           >> BOOST_SERIALIZATION_NVP(auth);

    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractAuthRequestMessageData(const Message& msg, std::string& player_name, std::string& auth) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractAuthResponseMessageData(const Message& msg, std::string& player_name, std::string& auth) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(player_name)
           >> BOOST_SERIALIZATION_NVP(auth);

    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractAuthResponeMessageData(const Message& msg, std::string& player_name, std::string& auth) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractSetAuthorizationRolesMessage(const Message &msg, Networking::AuthRoles& roles)
{ roles.SetText(msg.Text()); }

void ExtractPlayerInfoMessageData(const Message &msg, std::map<int, PlayerInfo>& players) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(players);
    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractPlayerInfo(const Message &msg, std::map<int, PlayerInfo>& players) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
    }
}
