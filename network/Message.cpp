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
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include <boost/timer.hpp>
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
Message::Message() :
    m_type(UNDEFINED),
    m_message_size(0),
    m_message_text()
{}

Message::Message(MessageType type,
                 const std::string& text) :
    m_type(type),
    m_message_size(text.size()),
    m_message_text(new char[text.size()])
{ std::copy(text.begin(), text.end(), m_message_text.get()); }

Message::MessageType Message::Type() const
{ return m_type; }

std::size_t Message::Size() const
{ return m_message_size; }

const char* Message::Data() const
{ return m_message_text.get(); }

std::string Message::Text() const
{ return std::string(m_message_text.get(), m_message_size); }

void Message::Resize(std::size_t size) {
    m_message_size = size;
    m_message_text.reset(new char[m_message_size]);
}

char* Message::Data()
{ return m_message_text.get(); }

void Message::Swap(Message& rhs) {
    std::swap(m_type, rhs.m_type);
    std::swap(m_message_size, rhs.m_message_size);
    std::swap(m_message_text, rhs.m_message_text);
}

bool operator==(const Message& lhs, const Message& rhs) {
    return
        lhs.Type() == rhs.Type() &&
        lhs.Text() == rhs.Text();
}

bool operator!=(const Message& lhs, const Message& rhs)
{ return !(lhs == rhs); }

void swap(Message& lhs, Message& rhs)
{ lhs.Swap(rhs); }

void BufferToHeader(const Message::HeaderBuffer& buffer, Message& message) {
    message.m_type = static_cast<Message::MessageType>(buffer[Message::Parts::TYPE]);
    message.m_message_size = buffer[Message::Parts::SIZE];
}

void HeaderToBuffer(const Message& message, Message::HeaderBuffer& buffer) {
    buffer[Message::Parts::TYPE] = message.Type();
    buffer[Message::Parts::SIZE] = message.Size();
}

////////////////////////////////////////////////
// Message named ctors
////////////////////////////////////////////////
Message ErrorMessage(const std::string& problem, bool fatal/* = true*/,
                     int player_id/* = Networking::INVALID_PLAYER_ID*/) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(problem)
           << BOOST_SERIALIZATION_NVP(fatal)
           << BOOST_SERIALIZATION_NVP(player_id);
    }
    return Message(Message::ERROR_MSG, os.str());
}

Message HostSPGameMessage(const SinglePlayerSetupData& setup_data) {
    std::ostringstream os;
    {
        std::string client_version_string = FreeOrionVersionString();
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(setup_data)
           << BOOST_SERIALIZATION_NVP(client_version_string);
    }
    return Message(Message::HOST_SP_GAME, os.str());
}

Message HostMPGameMessage(const std::string& host_player_name)
{
    std::ostringstream os;
    {
        std::string client_version_string = FreeOrionVersionString();
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(host_player_name)
           << BOOST_SERIALIZATION_NVP(client_version_string);
    }
    return Message(Message::HOST_MP_GAME, os.str());
}

Message JoinGameMessage(const std::string& player_name,
                        Networking::ClientType client_type,
                        boost::uuids::uuid cookie) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        std::string client_version_string = FreeOrionVersionString();
        oa << BOOST_SERIALIZATION_NVP(player_name)
           << BOOST_SERIALIZATION_NVP(client_type)
           << BOOST_SERIALIZATION_NVP(client_version_string)
           << BOOST_SERIALIZATION_NVP(cookie);
    }
    return Message(Message::JOIN_GAME, os.str());
}

Message HostIDMessage(int host_player_id) {
    return Message(Message::HOST_ID,
                   std::to_string(host_player_id));
}

Message GameStartMessage(bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const SpeciesManager& species,
                         CombatLogManager& combat_logs, const SupplyManager& supply,
                         const std::map<int, PlayerInfo>& players,
                         const GalaxySetupData& galaxy_setup_data,
                         bool use_binary_serialization)
{
    std::ostringstream os;
    {
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = false;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = false;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
    }
    return Message(Message::GAME_START, os.str());
}

Message GameStartMessage(bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const SpeciesManager& species,
                         CombatLogManager& combat_logs, const SupplyManager& supply,
                         const std::map<int, PlayerInfo>& players,
                         const OrderSet& orders, const SaveGameUIData* ui_data,
                         const GalaxySetupData& galaxy_setup_data,
                         bool use_binary_serialization)
{
    std::ostringstream os;
    {
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = (ui_data != nullptr);
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            if (ui_data_available)
                oa << boost::serialization::make_nvp("ui_data", *ui_data);
            bool save_state_string_available = false;
            oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = (ui_data != nullptr);
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            if (ui_data_available)
                oa << boost::serialization::make_nvp("ui_data", *ui_data);
            bool save_state_string_available = false;
            oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
    }
    return Message(Message::GAME_START, os.str());
}

Message GameStartMessage(bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const SpeciesManager& species,
                         CombatLogManager& combat_logs, const SupplyManager& supply,
                         const std::map<int, PlayerInfo>& players,
                         const OrderSet& orders, const std::string* save_state_string,
                         const GalaxySetupData& galaxy_setup_data,
                         bool use_binary_serialization)
{
    std::ostringstream os;
    {
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
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
            if (save_state_string_available)
                oa << boost::serialization::make_nvp("save_state_string", *save_state_string);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
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
            if (save_state_string_available)
                oa << boost::serialization::make_nvp("save_state_string", *save_state_string);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
    }
    return Message(Message::GAME_START, os.str());
}

Message HostSPAckMessage(int player_id)
{ return Message(Message::HOST_SP_GAME, std::to_string(player_id)); }

Message HostMPAckMessage(int player_id)
{ return Message(Message::HOST_MP_GAME, std::to_string(player_id)); }

Message JoinAckMessage(int player_id, boost::uuids::uuid cookie)
{
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(player_id)
           << BOOST_SERIALIZATION_NVP(cookie);
    }
    return Message(Message::JOIN_GAME, os.str());
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
    return Message(Message::TURN_ORDERS, os.str());
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
    return Message(Message::TURN_ORDERS, os.str());
}

Message TurnPartialOrdersMessage(const std::pair<OrderSet, std::set<int>>& orders_updates) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        Serialize(oa, orders_updates.first);
        oa << boost::serialization::make_nvp("deleted", orders_updates.second);
    }
    return Message(Message::TURN_PARTIAL_ORDERS, os.str());
}

Message TurnProgressMessage(Message::TurnProgressPhase phase_id) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(phase_id);
    }
    return Message(Message::TURN_PROGRESS, os.str());
}

Message PlayerStatusMessage(int about_player_id,
                            Message::PlayerStatus player_status,
                            int about_empire_id)
{
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(about_player_id)
           << BOOST_SERIALIZATION_NVP(player_status)
           << BOOST_SERIALIZATION_NVP(about_empire_id);
    }
    return Message(Message::PLAYER_STATUS, os.str());
}

Message TurnUpdateMessage(int empire_id, int current_turn,
                          const EmpireManager& empires, const Universe& universe,
                          const SpeciesManager& species, CombatLogManager& combat_logs,
                          const SupplyManager& supply,
                          const std::map<int, PlayerInfo>& players,
                          bool use_binary_serialization)
{
    std::ostringstream os;
    {
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(os);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(current_turn);
            oa << BOOST_SERIALIZATION_NVP(empires);
            oa << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            oa << BOOST_SERIALIZATION_NVP(players);
        } else {
            freeorion_xml_oarchive oa(os);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(current_turn)
               << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            oa << BOOST_SERIALIZATION_NVP(players);
        }
    }
    return Message(Message::TURN_UPDATE, os.str());
}

Message TurnPartialUpdateMessage(int empire_id, const Universe& universe,
                                 bool use_binary_serialization) {
    std::ostringstream os;
    {
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(os);
            GetUniverse().EncodingEmpire() = empire_id;
            Serialize(oa, universe);
        } else {
            freeorion_xml_oarchive oa(os);
            GetUniverse().EncodingEmpire() = empire_id;
            Serialize(oa, universe);
        }
    }
    return Message(Message::TURN_PARTIAL_UPDATE, os.str());
}

Message HostSaveGameInitiateMessage(const std::string& filename)
{ return Message(Message::SAVE_GAME_INITIATE, filename); }

Message ServerSaveGameCompleteMessage(const std::string& save_filename, int bytes_written) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(save_filename)
           << BOOST_SERIALIZATION_NVP(bytes_written);
    }
    return Message(Message::SAVE_GAME_COMPLETE, os.str());
}

Message DiplomacyMessage(const DiplomaticMessage& diplo_message) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(diplo_message);
    }
    return Message(Message::DIPLOMACY, os.str());
}

Message DiplomaticStatusMessage(const DiplomaticStatusUpdateInfo& diplo_update) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(diplo_update.empire1_id)
           << BOOST_SERIALIZATION_NVP(diplo_update.empire2_id)
           << BOOST_SERIALIZATION_NVP(diplo_update.diplo_status);
    }
    return Message(Message::DIPLOMATIC_STATUS, os.str());
}

Message EndGameMessage(Message::EndGameReason reason,
                       const std::string& reason_player_name/* = ""*/)
{
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(reason)
           << BOOST_SERIALIZATION_NVP(reason_player_name);
    }
    return Message(Message::END_GAME, os.str());
}

Message AIEndGameAcknowledgeMessage()
{ return Message(Message::AI_END_GAME_ACK, DUMMY_EMPTY_MESSAGE); }

Message ModeratorActionMessage(const Moderator::ModeratorAction& action) {
    std::ostringstream os;
    {
        const Moderator::ModeratorAction* mod_action = &action;
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(mod_action);
    }
    return Message(Message::MODERATOR_ACTION, os.str());
}

Message ShutdownServerMessage()
{ return Message(Message::SHUT_DOWN_SERVER, DUMMY_EMPTY_MESSAGE); }

/** requests previews of savefiles from server synchronously */
Message RequestSavePreviewsMessage(std::string relative_directory)
{ return Message(Message::REQUEST_SAVE_PREVIEWS, relative_directory); }

/** returns the savegame previews to the client */
Message DispatchSavePreviewsMessage(const PreviewInformation& previews) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(previews);
    }
    return Message(Message::DISPATCH_SAVE_PREVIEWS, os.str());
}

Message RequestCombatLogsMessage(const std::vector<int>& ids) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(ids);
    }
    return Message(Message::REQUEST_COMBAT_LOGS, os.str());
}

Message DispatchCombatLogsMessage(const std::vector<std::pair<int, const CombatLog>>& logs) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(logs);
    }
    return Message(Message::DISPATCH_COMBAT_LOGS, os.str());
}

Message LoggerConfigMessage(int sender, const std::set<std::tuple<std::string, std::string, LogLevel>>& options) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        std::size_t size = options.size();
        oa << BOOST_SERIALIZATION_NVP(size);
        for (const auto& option_tuple : options) {
            auto option = std::get<0>(option_tuple);
            auto name = std::get<1>(option_tuple);
            auto value = std::get<2>(option_tuple);
            oa << BOOST_SERIALIZATION_NVP(option);
            oa << BOOST_SERIALIZATION_NVP(name);
            oa << BOOST_SERIALIZATION_NVP(value);
        }
    }
    return Message(Message::LOGGER_CONFIG, os.str());
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
    return Message(Message::LOBBY_UPDATE, os.str());
}

Message ServerLobbyUpdateMessage(const MultiplayerLobbyData& lobby_data) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(lobby_data);
    }
    return Message(Message::LOBBY_UPDATE, os.str());
}

Message ChatHistoryMessage(const std::vector<std::reference_wrapper<const ChatHistoryEntity>>& chat_history) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        std::size_t size = chat_history.size();
        oa << BOOST_SERIALIZATION_NVP(size);
        for (const auto& elem : chat_history) {
            oa << boost::serialization::make_nvp(BOOST_PP_STRINGIZE(elem), elem.get());
        }
    }
    return Message(Message::CHAT_HISTORY, os.str());
}

Message PlayerChatMessage(const std::string& data, int receiver) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(receiver)
           << BOOST_SERIALIZATION_NVP(data);
    }
    return Message(Message::PLAYER_CHAT, os.str());
}

Message ServerPlayerChatMessage(int sender,
                                const boost::posix_time::ptime& timestamp,
                                const std::string& data)
{
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(sender)
           << BOOST_SERIALIZATION_NVP(timestamp)
           << BOOST_SERIALIZATION_NVP(data);
    }
    return Message(Message::PLAYER_CHAT, os.str());
}

Message StartMPGameMessage()
{ return Message(Message::START_MP_GAME, DUMMY_EMPTY_MESSAGE); }

Message ContentCheckSumMessage() {
    auto checksums = CheckSumContent();

    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(checksums);
    }
    return Message(Message::CHECKSUM, os.str());
}

Message AuthRequestMessage(const std::string& player_name, const std::string& auth) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(player_name)
           << BOOST_SERIALIZATION_NVP(auth);
    }
    return Message(Message::AUTH_REQUEST, os.str());
}

Message AuthResponseMessage(const std::string& player_name, const std::string& auth) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(player_name)
           << BOOST_SERIALIZATION_NVP(auth);
    }
    return Message(Message::AUTH_RESPONSE, os.str());
}

Message SetAuthorizationRolesMessage(const Networking::AuthRoles& roles)
{ return Message(Message::SET_AUTH_ROLES, roles.Text()); }

Message EliminateSelfMessage()
{ return Message(Message::ELIMINATE_SELF, DUMMY_EMPTY_MESSAGE); }

Message UnreadyMessage()
{ return Message(Message::UNREADY, DUMMY_EMPTY_MESSAGE); }

////////////////////////////////////////////////
// Message data extractors
////////////////////////////////////////////////
void ExtractErrorMessageData(const Message& msg, int& player_id, std::string& problem, bool& fatal) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(problem)
           >> BOOST_SERIALIZATION_NVP(fatal)
           >> BOOST_SERIALIZATION_NVP(player_id);
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractErrorMessageData(const Message& msg, std::string& problem, bool& fatal) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        problem = UserStringNop("SERVER_MESSAGE_NOT_UNDERSTOOD");
        fatal = false;
    }
}

void ExtractHostMPGameMessageData(const Message& msg, std::string& host_player_name, std::string& client_version_string) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(host_player_name)
           >> BOOST_SERIALIZATION_NVP(client_version_string);

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
        freeorion_xml_iarchive ia(is);
        std::size_t size;
        ia >> BOOST_SERIALIZATION_NVP(size);
        chat_history.clear();
        chat_history.reserve(size);
        for (size_t ii = 0; ii < size; ++ii) {
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

void ExtractPlayerChatMessageData(const Message& msg, int& receiver, std::string& data) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(receiver)
           >> BOOST_SERIALIZATION_NVP(data);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractPlayerChatMessageData(const Message& msg, int& receiver, std::string& data) failed! Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractServerPlayerChatMessageData(const Message& msg,
                                        int& sender,
                                        boost::posix_time::ptime& timestamp,
                                        std::string& data)
{
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(sender)
           >> BOOST_SERIALIZATION_NVP(timestamp)
           >> BOOST_SERIALIZATION_NVP(data);
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
    try {
        bool try_xml = false;
        if (strncmp(msg.Data(), "<?xml", 5)) {
            try {
                // first attempt binary deserialziation
                std::istringstream is(msg.Text());

                freeorion_bin_iarchive ia(is);
                ia >> BOOST_SERIALIZATION_NVP(single_player_game)
                   >> BOOST_SERIALIZATION_NVP(empire_id)
                   >> BOOST_SERIALIZATION_NVP(current_turn);
                GetUniverse().EncodingEmpire() = empire_id;

                boost::timer deserialize_timer;
                ia >> BOOST_SERIALIZATION_NVP(empires);
                DebugLogger() << "ExtractGameStartMessage empire deserialization time " << (deserialize_timer.elapsed() * 1000.0);

                ia >> BOOST_SERIALIZATION_NVP(species);
                combat_logs.Clear();    // only needed when loading new game, not when incrementally serializing logs on turn update
                combat_logs.SerializeIncompleteLogs(ia, 1);
                ia >> BOOST_SERIALIZATION_NVP(supply);

                deserialize_timer.restart();
                Deserialize(ia, universe);
                DebugLogger() << "ExtractGameStartMessage universe deserialization time " << (deserialize_timer.elapsed() * 1000.0);


                ia >> BOOST_SERIALIZATION_NVP(players)
                   >> BOOST_SERIALIZATION_NVP(loaded_game_data);
                if (loaded_game_data) {
                    Deserialize(ia, orders);
                    ia >> BOOST_SERIALIZATION_NVP(ui_data_available);
                    if (ui_data_available)
                        ia >> BOOST_SERIALIZATION_NVP(ui_data);
                    ia >> BOOST_SERIALIZATION_NVP(save_state_string_available);
                    if (save_state_string_available)
                        ia >> BOOST_SERIALIZATION_NVP(save_state_string);
                } else {
                    ui_data_available = false;
                    save_state_string_available = false;
                }
                ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);
            } catch (...) {
                try_xml = true;
            }
        } else {
            try_xml = true;
        }
        if (try_xml) {
            // if binary deserialization failed, try more-portable XML deserialization
            std::istringstream is(msg.Text());

            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(single_player_game)
               >> BOOST_SERIALIZATION_NVP(empire_id)
               >> BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;

            boost::timer deserialize_timer;
            ia >> BOOST_SERIALIZATION_NVP(empires);
            DebugLogger() << "ExtractGameStartMessage empire deserialization time " << (deserialize_timer.elapsed() * 1000.0);

            ia >> BOOST_SERIALIZATION_NVP(species);
            combat_logs.Clear();    // only needed when loading new game, not when incrementally serializing logs on turn update
            combat_logs.SerializeIncompleteLogs(ia, 1);
            ia >> BOOST_SERIALIZATION_NVP(supply);

            deserialize_timer.restart();
            Deserialize(ia, universe);
            DebugLogger() << "ExtractGameStartMessage universe deserialization time " << (deserialize_timer.elapsed() * 1000.0);


            ia >> BOOST_SERIALIZATION_NVP(players)
               >> BOOST_SERIALIZATION_NVP(loaded_game_data);
            if (loaded_game_data) {
                Deserialize(ia, orders);
                ia >> BOOST_SERIALIZATION_NVP(ui_data_available);
                if (ui_data_available)
                    ia >> BOOST_SERIALIZATION_NVP(ui_data);
                ia >> BOOST_SERIALIZATION_NVP(save_state_string_available);
                if (save_state_string_available)
                    ia >> BOOST_SERIALIZATION_NVP(save_state_string);
            } else {
                ui_data_available = false;
                save_state_string_available = false;
            }
            ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractGameStartMessageData(...) failed!  Message probably long, so not outputting to log.\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractJoinGameMessageData(const Message& msg, std::string& player_name,
                                Networking::ClientType& client_type,
                                std::string& version_string,
                                boost::uuids::uuid& cookie)
{
    DebugLogger() << "ExtractJoinGameMessageData() from " << player_name << " client type " << client_type;
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(player_name)
           >> BOOST_SERIALIZATION_NVP(client_type)
           >> BOOST_SERIALIZATION_NVP(version_string)
           >> BOOST_SERIALIZATION_NVP(cookie);

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

void ExtractTurnOrdersMessageData(const Message& msg,
                                  OrderSet& orders,
                                  bool& ui_data_available,
                                  SaveGameUIData& ui_data,
                                  bool& save_state_string_available,
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
    try {
        ScopedTimer timer("Turn Update Unpacking", true);

        bool try_xml = false;
        if (std::strncmp(msg.Data(), "<?xml", 5)) {
            try {
                // first attempt binary deserialization
                std::istringstream is(msg.Text());
                freeorion_bin_iarchive ia(is);
                GetUniverse().EncodingEmpire() = empire_id;
                ia >> BOOST_SERIALIZATION_NVP(current_turn)
                   >> BOOST_SERIALIZATION_NVP(empires)
                   >> BOOST_SERIALIZATION_NVP(species);
                combat_logs.SerializeIncompleteLogs(ia, 1);
                ia >> BOOST_SERIALIZATION_NVP(supply);
                Deserialize(ia, universe);
                ia >> BOOST_SERIALIZATION_NVP(players);
            } catch (...) {
                try_xml = true;
            }
        } else {
            try_xml = true;
        }
        if (try_xml) {
            // try again with more-portable XML deserialization
            std::istringstream is(msg.Text());
            freeorion_xml_iarchive ia(is);
            GetUniverse().EncodingEmpire() = empire_id;
            ia >> BOOST_SERIALIZATION_NVP(current_turn)
               >> BOOST_SERIALIZATION_NVP(empires)
               >> BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(ia, 1);
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
        ScopedTimer timer("Mid Turn Update Unpacking", true);

        bool try_xml = false;
        if (std::strncmp(msg.Data(), "<?xml", 5)) {
            try {
                // first attempt binary deserialization
                std::istringstream is(msg.Text());
                freeorion_bin_iarchive ia(is);
                GetUniverse().EncodingEmpire() = empire_id;
                Deserialize(ia, universe);
            } catch (...) {
                try_xml = true;
            }
        } else {
            try_xml = true;
        }
        if (try_xml) {
            // try again with more-portable XML deserialization
            std::istringstream is(msg.Text());
            freeorion_xml_iarchive ia(is);
            GetUniverse().EncodingEmpire() = empire_id;
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

void ExtractPlayerStatusMessageData(const Message& msg,
                                    int& about_player_id,
                                    Message::PlayerStatus& status,
                                    int& about_empire_id) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(about_player_id)
           >> BOOST_SERIALIZATION_NVP(status)
           >> BOOST_SERIALIZATION_NVP(about_empire_id);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractPlayerStatusMessageData(const Message& msg, int& about_player_id, Message::PlayerStatus&) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractHostSPGameMessageData(const Message& msg, SinglePlayerSetupData& setup_data, std::string& client_version_string) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(setup_data)
           >> BOOST_SERIALIZATION_NVP(client_version_string);

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

void ExtractModeratorActionMessageData(const Message& msg, Moderator::ModeratorAction*& mod_action) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(mod_action);

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
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(logs);
    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractDispatchCombatLogMessageData(const Message& msg, std::vector<std::pair<int, const CombatLog&>>& logs) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }

}

FO_COMMON_API void ExtractLoggerConfigMessageData(
    const Message& msg, std::set<std::tuple<std::string, std::string, LogLevel>>& options)
{
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        std::size_t size;
        ia >> BOOST_SERIALIZATION_NVP(size);
        for (size_t ii = 0; ii < size; ++ii) {
            std::string option;
            std::string name;
            LogLevel level;
            ia >> BOOST_SERIALIZATION_NVP(option);
            ia >> BOOST_SERIALIZATION_NVP(name);
            ia >> BOOST_SERIALIZATION_NVP(level);
            options.insert(std::make_tuple(option, name, level));
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractDispatchCombatLogMessageData(const Message& msg, std::vector<std::pair<int, const CombatLog&> >& logs) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }

}

void ExtractContentCheckSumMessageData(
    const Message& msg, std::map<std::string, unsigned int>& checksums)
{
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
