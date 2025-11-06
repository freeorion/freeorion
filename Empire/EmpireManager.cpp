#include "EmpireManager.h"

#include "Empire.h"
#include "../util/Directories.h"
#include "../util/GameRules.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/XMLDoc.h"

#include <fstream>

namespace {
    // sorted pair, so order of empire IDs specified doesn't matter
    [[nodiscard]] inline constexpr auto DiploKey(int id1, int ind2) noexcept
    { return std::pair(std::max(id1, ind2), std::min(id1, ind2)); }
}

EmpireManager& EmpireManager::operator=(EmpireManager&& other) noexcept {
    if (this != std::addressof(other)) {
        m_empire_ids = std::move(other.m_empire_ids);
        m_capital_ids = std::move(other.m_capital_ids);
        m_empire_map = std::move(other.m_empire_map);
        m_const_empire_map = std::move(other.m_const_empire_map);
        m_empire_diplomatic_statuses = std::move(other.m_empire_diplomatic_statuses);
        m_diplomatic_messages = std::move(other.m_diplomatic_messages);
    }
    return *this;
}

std::shared_ptr<const Empire> EmpireManager::GetEmpire(int id) const {
    auto it = m_const_empire_map.find(id);
    return it == m_const_empire_map.end() ? nullptr : it->second;
}

std::shared_ptr<const UniverseObject> EmpireManager::GetSource(int id, const ObjectMap& objects) const {
    auto it = m_const_empire_map.find(id);
    return it != m_const_empire_map.end() ? it->second->Source(objects) : nullptr;
}

int EmpireManager::NumEliminatedEmpires() const {
    int eliminated_count = 0;

    for (const auto& empire : m_const_empire_map)
        if (empire.second->Eliminated())
            eliminated_count++;

    return eliminated_count;
}

std::string EmpireManager::Dump() const {
    std::string retval = "Empires:\n";
    for (const auto& entry : m_empire_map)
        retval += entry.second->Dump();
    retval += DumpDiplomacy();
    return retval;
}

std::string EmpireManager::DumpDiplomacy() const {
    std::string retval = "Diplomatic Statuses:\n";
    for (auto& [ids, diplo_status] : m_empire_diplomatic_statuses) {
        auto it = m_const_empire_map.find(ids.first);
        if (it == m_const_empire_map.end())
            continue;
        const auto& empire1 = it->second;
        it = m_const_empire_map.find(ids.second);
        if (it == m_const_empire_map.end())
            continue;
        const auto& empire2 = it->second;
        if (!empire1 || !empire2)
            continue;
        retval += " * " + empire1->Name() + " / " + empire2->Name() + " : ";
        switch (diplo_status) {
        case DiplomaticStatus::DIPLO_WAR:    retval += "War";    break;
        case DiplomaticStatus::DIPLO_PEACE:  retval += "Peace";  break;
        case DiplomaticStatus::DIPLO_ALLIED: retval += "Allied";  break;
        default:                             retval += "?";      break;
        }
        retval += "\n";
    }
    retval += "Diplomatic Messages:\n";
    for (auto& [ids, message] : m_diplomatic_messages) {
        if (message.GetType() == DiplomaticMessage::Type::INVALID)
            continue;   // don't print non-messages and pollute the log files...
        retval += "From: " + std::to_string(ids.first)
               + " to: " + std::to_string(ids.second)
               + " message: " + message.Dump() + "\n";
    }

    return retval;
}

std::shared_ptr<Empire> EmpireManager::GetEmpire(int id) {
    iterator it = m_empire_map.find(id);
    return it == end() ? nullptr : it->second;
}

std::size_t EmpireManager::SizeInMemory() const {
    std::size_t retval = sizeof(EmpireManager);

    retval += sizeof(decltype(m_empire_ids)::value_type)*m_empire_ids.capacity();
    retval += sizeof(decltype(m_capital_ids)::value_type)*m_capital_ids.capacity();

    retval += (sizeof(decltype(m_empire_map)::value_type) + sizeof(void*))*m_empire_map.size();
    for (const auto& id_e : m_empire_map) {
        if (id_e.second)
            retval += id_e.second->SizeInMemory();
    }

    retval += (sizeof(decltype(m_const_empire_map)::value_type) + sizeof(void*))*m_const_empire_map.size();
    retval += sizeof(decltype(m_empire_diplomatic_statuses)::value_type)*m_empire_diplomatic_statuses.capacity();
    retval += (sizeof(decltype(m_diplomatic_messages)::value_type) + sizeof(void*))*m_diplomatic_messages.size();

    return retval;
}


void EmpireManager::BackPropagateMeters() noexcept {
    for (auto& entry : m_empire_map)
        entry.second->BackPropagateMeters();
}

void EmpireManager::CreateEmpire(int empire_id, std::string name, std::string player_name,
                                 EmpireColor color, bool authenticated)
{
    auto empire = std::make_shared<Empire>(std::move(name), std::move(player_name),
                                           empire_id, color, authenticated);
    InsertEmpire(std::move(empire));
}

void EmpireManager::InsertEmpire(std::shared_ptr<Empire>&& empire) {
    if (!empire) {
        ErrorLogger() << "EmpireManager::InsertEmpire passed null empire";
        return;
    }

    int empire_id = empire->EmpireID();

    if (m_empire_map.contains(empire_id)) {
        ErrorLogger() << "EmpireManager::InsertEmpire passed empire with id (" << empire_id << ") for which there already is an empire.";
        return;
    }

    m_empire_ids.insert(empire_id);
    m_const_empire_map[empire_id] = empire;
    m_empire_map[empire_id] = std::move(empire);
}

void EmpireManager::Clear() noexcept {
    m_empire_ids.clear();
    m_const_empire_map.clear();
    m_empire_map.clear();
    m_empire_diplomatic_statuses.clear();
}

DiplomaticStatus EmpireManager::GetDiplomaticStatus(int empire1, int empire2) const {
    if (empire1 == ALL_EMPIRES || empire2 == ALL_EMPIRES || empire1 == empire2)
        return DiplomaticStatus::INVALID_DIPLOMATIC_STATUS;

    auto it = m_empire_diplomatic_statuses.find(DiploKey(empire1, empire2));
    if (it != m_empire_diplomatic_statuses.end())
        return it->second;
    ErrorLogger() << "Couldn't find diplomatic status between empires " << empire1 << " and " << empire2;
    return DiplomaticStatus::INVALID_DIPLOMATIC_STATUS;
}

boost::container::flat_set<int> EmpireManager::GetEmpireIDsWithDiplomaticStatusWithEmpire(
    int empire_id, DiplomaticStatus diplo_status, const DiploStatusMap& statuses)
{
    if (empire_id == ALL_EMPIRES || diplo_status == DiplomaticStatus::INVALID_DIPLOMATIC_STATUS)
        return {};

    // find ids of empires with the specified diplomatic status with the specified empire
    const auto is_status = [diplo_status](const auto& ids_status) noexcept
    { return ids_status.second == diplo_status; };

    const auto has_empire_id = [empire_id](const auto& e1e2) noexcept
    { return e1e2.first == empire_id || e1e2.second == empire_id; };

    const auto to_other_id = [empire_id](const auto& e1e2) noexcept
    { return e1e2.first == empire_id ? e1e2.second : e1e2.first; };

    return statuses | range_filter(is_status) | range_keys | range_filter(has_empire_id)
        | range_transform(to_other_id) | range_to<boost::container::flat_set<int>>();
}

bool EmpireManager::DiplomaticMessageAvailable(int sender_id, int recipient_id) const {
    auto it = m_diplomatic_messages.find({sender_id, recipient_id});
    return it != m_diplomatic_messages.end() &&
           it->second.GetType() != DiplomaticMessage::Type::INVALID;
}

const DiplomaticMessage& EmpireManager::GetDiplomaticMessage(int sender_id, int recipient_id) const {
    const auto it = m_diplomatic_messages.find({sender_id, recipient_id});
    if (it != m_diplomatic_messages.end())
        return it->second;
    static constexpr DiplomaticMessage DEFAULT_DIPLOMATIC_MESSAGE;
    //WarnLogger() << "Couldn't find requested diplomatic message between empires " << sender_id << " and " << recipient_id;
    return DEFAULT_DIPLOMATIC_MESSAGE;
}

void EmpireManager::SetDiplomaticStatus(int empire1, int empire2, DiplomaticStatus status) {
    const DiplomaticStatus initial_status = GetDiplomaticStatus(empire1, empire2);
    if (status != initial_status) {
        m_empire_diplomatic_statuses[DiploKey(empire1, empire2)] = status;
        DiplomaticStatusChangedSignal(empire1, empire2);
    }
}

void EmpireManager::SetDiplomaticMessage(const DiplomaticMessage& message) {
    const int empire1 = message.SenderEmpireID();
    const int empire2 = message.RecipientEmpireID();
    const DiplomaticMessage& initial_message = GetDiplomaticMessage(empire1, empire2);
    if (message != initial_message) {
        m_diplomatic_messages[{empire1, empire2}] = message;
        DiplomaticMessageChangedSignal(empire1, empire2);
    }
}

void EmpireManager::RemoveDiplomaticMessage(int sender_id, int recipient_id) {
    const auto it = m_diplomatic_messages.find({sender_id, recipient_id});
    const bool changed = (it != m_diplomatic_messages.end()) &&
                         (it->second.GetType() != DiplomaticMessage::Type::INVALID);

    m_diplomatic_messages[{sender_id, recipient_id}] =
        DiplomaticMessage(sender_id, recipient_id, DiplomaticMessage::Type::INVALID);

    // if there already was a message, and it wasn't already a non-message, notify about change
    if (changed)
        DiplomaticMessageChangedSignal(sender_id, recipient_id);
}

void EmpireManager::HandleDiplomaticMessage(const DiplomaticMessage& message) {
    const int sender_empire_id = message.SenderEmpireID();
    const int recipient_empire_id = message.RecipientEmpireID();

    if (!message.IsAllowed())
        return;

    const DiplomaticStatus diplo_status = GetDiplomaticStatus(sender_empire_id, recipient_empire_id);
    const bool message_from_recipient_to_sender_available = DiplomaticMessageAvailable(recipient_empire_id, sender_empire_id);
    const auto& existing_message_from_recipient_to_sender = GetDiplomaticMessage(recipient_empire_id, sender_empire_id);
    //bool message_from_sender_to_recipient_available = DiplomaticMessageAvailable(sender_empire_id, recipient_empire_id);

    switch (message.GetType()) {
    case DiplomaticMessage::Type::WAR_DECLARATION: {
        if (diplo_status == DiplomaticStatus::DIPLO_PEACE) {
            // cancels any previous messages, sets empires at war
            RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
            RemoveDiplomaticMessage(recipient_empire_id, sender_empire_id);
            SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DiplomaticStatus::DIPLO_WAR);
        }
        break;
    }

    case DiplomaticMessage::Type::PEACE_PROPOSAL: {
        if (diplo_status == DiplomaticStatus::DIPLO_WAR && !message_from_recipient_to_sender_available) {
            SetDiplomaticMessage(message);

        } else if (diplo_status == DiplomaticStatus::DIPLO_WAR && message_from_recipient_to_sender_available) {
            if (existing_message_from_recipient_to_sender.GetType() ==
                DiplomaticMessage::Type::PEACE_PROPOSAL)
            {
                // somehow multiple peace proposals sent by players to eachother
                // cancel and remove
                RemoveDiplomaticMessage(recipient_empire_id, sender_empire_id);
                RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
                SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DiplomaticStatus::DIPLO_PEACE);
            }
        }
        break;
    }

    case DiplomaticMessage::Type::ACCEPT_PEACE_PROPOSAL: {
        if (message_from_recipient_to_sender_available &&
            existing_message_from_recipient_to_sender.GetType() == DiplomaticMessage::Type::PEACE_PROPOSAL)
        {
            // one player proposed peace and the other accepted
            RemoveDiplomaticMessage(recipient_empire_id, sender_empire_id);
            RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
            SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DiplomaticStatus::DIPLO_PEACE);
        }
        break;
    }

    case DiplomaticMessage::Type::ALLIES_PROPOSAL: {
        if (diplo_status == DiplomaticStatus::DIPLO_PEACE && !message_from_recipient_to_sender_available) {
            SetDiplomaticMessage(message);

        } else if (diplo_status == DiplomaticStatus::DIPLO_PEACE && message_from_recipient_to_sender_available) {
            if (existing_message_from_recipient_to_sender.GetType() ==
                DiplomaticMessage::Type::ALLIES_PROPOSAL)
            {
                // somehow multiple allies proposals sent by players to eachother
                // cancel and remove
                RemoveDiplomaticMessage(recipient_empire_id, sender_empire_id);
                RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
                SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DiplomaticStatus::DIPLO_ALLIED);
            }
        }
        break;
    }

    case DiplomaticMessage::Type::ACCEPT_ALLIES_PROPOSAL: {
        if (message_from_recipient_to_sender_available &&
            existing_message_from_recipient_to_sender.GetType() == DiplomaticMessage::Type::ALLIES_PROPOSAL)
        {
            // one player proposed alliance and the other accepted
            RemoveDiplomaticMessage(recipient_empire_id, sender_empire_id);
            RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
            SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DiplomaticStatus::DIPLO_ALLIED);
        }
        break;
    }

    case DiplomaticMessage::Type::END_ALLIANCE_DECLARATION: {
        if (diplo_status == DiplomaticStatus::DIPLO_ALLIED) {
            // cancels any previous messages, sets empires to peace
            RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
            RemoveDiplomaticMessage(recipient_empire_id, sender_empire_id);
            SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DiplomaticStatus::DIPLO_PEACE);
        }
        break;
    }

    case DiplomaticMessage::Type::CANCEL_PROPOSAL: {
        RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
        break;
    }

    case DiplomaticMessage::Type::REJECT_PROPOSAL: {
        RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
        RemoveDiplomaticMessage(recipient_empire_id, sender_empire_id);
        break;
    }

    default:
        break;
    }
}

void EmpireManager::ResetDiplomacy() {
    // remove messages
    m_diplomatic_messages.clear();

    // set all empires at war with each other (but not themselves)
    m_empire_diplomatic_statuses.clear();
    for (auto id_empire_1 : m_const_empire_map) {
        for (auto id_empire_2 : m_const_empire_map) {
            if (id_empire_1.first == id_empire_2.first)
                continue;
            std::pair<int, int> diplo_key = DiploKey(id_empire_1.first, id_empire_2.first);
            m_empire_diplomatic_statuses[diplo_key] = DiplomaticStatus::DIPLO_WAR;
        }
    }
}

void EmpireManager::RefreshCapitalIDs() {
    static constexpr auto to_cap_id = [](const auto& e) noexcept { return e.second->CapitalID(); };
    m_capital_ids = m_const_empire_map | range_transform(to_cap_id) | range_to<decltype(m_capital_ids)>();
}

void EmpireManager::GetDiplomaticMessagesToSerialize(std::map<std::pair<int, int>, DiplomaticMessage>& messages,
                                                     int encoding_empire) const
{
    messages.clear();

    // return all messages for general case
    if (encoding_empire == ALL_EMPIRES) {
        messages = m_diplomatic_messages;
        return;
    }

    // find all messages involving encoding empire
    for (const auto& entry : m_diplomatic_messages) {
        if (entry.first.first == encoding_empire || entry.first.second == encoding_empire)
            messages.insert(entry);
    }
}

namespace {
    const std::vector<EmpireColor> backup_empire_colors = {
        {{ 0, 255,   0, 255}}, {{  0,   0, 255, 255}}, {{255,   0,   0, 255}},
        {{ 0, 255, 255, 255}}, {{255, 255,   0, 255}}, {{255,   0, 255, 255}}
    };
    std::vector<EmpireColor> empire_colors;

    constexpr uint8_t HexCharToUint8(std::string_view::value_type digit) noexcept
    { return (digit >= 'A' ? (digit - 'A' + 10) : (digit - '0')); }

    constexpr uint8_t HexCharsToUInt8(std::string_view chars) noexcept {
        if (chars.empty())
            return 0;
        const uint8_t val0 = HexCharToUint8(chars[0]);
        if (chars.size() == 1) [[unlikely]]
            return val0;
        const uint8_t val1 = HexCharToUint8(chars[1]);
        return 16*val0 + val1;
    };

    static_assert(HexCharToUint8('0') == 0u);
    static_assert(HexCharToUint8('9') == 9u);
    static_assert(HexCharToUint8('A') == 10u);
    static_assert(HexCharToUint8('F') == 15u);
    static_assert(HexCharToUint8('G') == 16u);

    static_assert(HexCharsToUInt8("") == 0u);
    static_assert(HexCharsToUInt8("A") == 10u);
    static_assert(HexCharsToUInt8("01") == 1u);
    static_assert(HexCharsToUInt8("FF") == 255u);
    static_assert(HexCharsToUInt8("A0") == 160u);
    static_assert(HexCharsToUInt8("!.") == 14u);

    constexpr EmpireColor HexStringToEmpireColor(std::string_view hex_colour) noexcept {
        const auto sz = hex_colour.size();
        return {{
            (sz >= 2) ? HexCharsToUInt8(hex_colour.substr(0, 2)) : uint8_t{0u},
            (sz >= 4) ? HexCharsToUInt8(hex_colour.substr(2, 2)) : uint8_t{0u},
            (sz >= 6) ? HexCharsToUInt8(hex_colour.substr(4, 2)) : uint8_t{0u},
            (sz >= 8) ? HexCharsToUInt8(hex_colour.substr(6, 2)) : uint8_t{255u}
        }};
    }
}

const std::vector<EmpireColor>& EmpireColors() {
    if (empire_colors.empty())
        return backup_empire_colors;
    return empire_colors;
}

void InitEmpireColors(const std::filesystem::path& path) {
    XMLDoc doc;

    std::string empire_colors_content;
    if (ReadFile(path, empire_colors_content)) {
        doc.ReadDoc(empire_colors_content);
    } else {
        ErrorLogger() << "InitEmpireColors: Unable to open data file " << path.filename();
        return;
    }

    for (const XMLElement& elem : doc.root_node.Children())
        empire_colors.push_back(HexStringToEmpireColor(elem.Attribute("hex")));
}
