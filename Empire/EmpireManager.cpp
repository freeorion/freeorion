#include "EmpireManager.h"

#include "Empire.h"
#include "../util/Directories.h"
#include "../util/GameRules.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/XMLDoc.h"

#include <boost/filesystem/fstream.hpp>

namespace {
    // sorted pair, so order of empire IDs specified doesn't matter
    std::pair<int, int> DiploKey(int id1, int ind2)
    { return std::make_pair(std::max(id1, ind2), std::min(id1, ind2)); }

    const std::string EMPTY_STRING;
}

EmpireManager& EmpireManager::operator=(EmpireManager&& other) noexcept {
    if (this != &other) {
        m_empire_map = std::move(other.m_empire_map);
        m_const_empire_map = std::move(other.m_const_empire_map);
        m_empire_diplomatic_statuses = std::move(other.m_empire_diplomatic_statuses);
        m_diplomatic_messages = std::move(other.m_diplomatic_messages);
    }
    return *this;
}

EmpireManager::~EmpireManager()
{}

std::shared_ptr<const Empire> EmpireManager::GetEmpire(int id) const {
    auto it = m_const_empire_map.find(id);
    return it == m_const_empire_map.end() ? nullptr : it->second;
}

std::shared_ptr<const UniverseObject> EmpireManager::GetSource(int id) const {
    auto it = m_const_empire_map.find(id);
    return it != m_const_empire_map.end() ? it->second->Source() : nullptr;
}

const std::string& EmpireManager::GetEmpireName(int id) const {
    auto it = m_const_empire_map.find(id);
    return it == m_const_empire_map.end() ? EMPTY_STRING : it->second->Name();
}

int EmpireManager::NumEmpires() const
{ return m_const_empire_map.size(); }

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
    for (const auto& entry : m_empire_diplomatic_statuses) {
        auto empire1 = GetEmpire(entry.first.first).get();
        auto empire2 = GetEmpire(entry.first.second).get();
        if (!empire1 || !empire2)
            continue;
        retval += " * " + empire1->Name() + " / " + empire2->Name() + " : ";
        switch (entry.second) {
        case DiplomaticStatus::DIPLO_WAR:     retval += "War";    break;
        case DiplomaticStatus::DIPLO_PEACE:   retval += "Peace";  break;
        case DiplomaticStatus::DIPLO_ALLIED:  retval += "Allied";  break;
        default:            retval += "?";      break;
        }
        retval += "\n";
    }
    retval += "Diplomatic Messages:\n";
    for (const auto& message : m_diplomatic_messages) {
        if (message.second.GetType() == DiplomaticMessage::Type::INVALID)
            continue;   // don't print non-messages and pollute the log files...
        retval += "From: " + std::to_string(message.first.first)
               + " to: " + std::to_string(message.first.second)
               + " message: " + message.second.Dump() + "\n";
    }

    return retval;
}

std::shared_ptr<Empire> EmpireManager::GetEmpire(int id) {
    iterator it = m_empire_map.find(id);
    return it == end() ? nullptr : it->second;
}

EmpireManager::const_iterator EmpireManager::begin() const
{ return m_const_empire_map.begin(); }

EmpireManager::const_iterator EmpireManager::end() const
{ return m_const_empire_map.end(); }

EmpireManager::iterator EmpireManager::begin()
{ return m_empire_map.begin(); }

EmpireManager::iterator EmpireManager::end()
{ return m_empire_map.end(); }

void EmpireManager::BackPropagateMeters() {
    for (auto& entry : m_empire_map)
        entry.second->BackPropagateMeters();
}

void EmpireManager::CreateEmpire(int empire_id, std::string name,
                                 std::string player_name,
                                 const EmpireColor& color, bool authenticated)
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

    if (m_empire_map.count(empire_id)) {
        ErrorLogger() << "EmpireManager::InsertEmpire passed empire with id (" << empire_id << ") for which there already is an empire.";
        return;
    }

    m_const_empire_map[empire_id] = empire;
    m_empire_map[empire_id] = std::move(empire);
}

void EmpireManager::Clear() {
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

std::set<int> EmpireManager::GetEmpireIDsWithDiplomaticStatusWithEmpire(
    int empire_id, DiplomaticStatus diplo_status) const
{
    std::set<int> retval;
    if (empire_id == ALL_EMPIRES || diplo_status == DiplomaticStatus::INVALID_DIPLOMATIC_STATUS)
        return retval;
    // find ids of empires with the specified diplomatic status with the specified empire
    for (auto const& id_pair_status : m_empire_diplomatic_statuses) {
        if (id_pair_status.second != diplo_status)
            continue;
        if (id_pair_status.first.first == empire_id)
            retval.insert(id_pair_status.first.second);
        else if (id_pair_status.first.second == empire_id)
            retval.insert(id_pair_status.first.first);
    }
    return retval;
}

bool EmpireManager::DiplomaticMessageAvailable(int sender_id, int recipient_id) const {
    auto it = m_diplomatic_messages.find({sender_id, recipient_id});
    return it != m_diplomatic_messages.end() &&
           it->second.GetType() != DiplomaticMessage::Type::INVALID;
}

const DiplomaticMessage& EmpireManager::GetDiplomaticMessage(int sender_id, int recipient_id) const {
    auto it = m_diplomatic_messages.find({sender_id, recipient_id});
    if (it != m_diplomatic_messages.end())
        return it->second;
    static DiplomaticMessage DEFAULT_DIPLOMATIC_MESSAGE;
    //WarnLogger() << "Couldn't find requested diplomatic message between empires " << sender_id << " and " << recipient_id;
    return DEFAULT_DIPLOMATIC_MESSAGE;
}

void EmpireManager::SetDiplomaticStatus(int empire1, int empire2, DiplomaticStatus status) {
    DiplomaticStatus initial_status = GetDiplomaticStatus(empire1, empire2);
    if (status != initial_status) {
        m_empire_diplomatic_statuses[DiploKey(empire1, empire2)] = status;
        DiplomaticStatusChangedSignal(empire1, empire2);
    }
}

void EmpireManager::SetDiplomaticMessage(const DiplomaticMessage& message) {
    int empire1 = message.SenderEmpireID();
    int empire2 = message.RecipientEmpireID();
    const DiplomaticMessage& initial_message = GetDiplomaticMessage(empire1, empire2);
    if (message != initial_message) {
        m_diplomatic_messages[{empire1, empire2}] = message;
        DiplomaticMessageChangedSignal(empire1, empire2);
    }
}

void EmpireManager::RemoveDiplomaticMessage(int sender_id, int recipient_id) {
    auto it = m_diplomatic_messages.find({sender_id, recipient_id});
    bool changed = (it != m_diplomatic_messages.end()) &&
                   (it->second.GetType() != DiplomaticMessage::Type::INVALID);

    m_diplomatic_messages[{sender_id, recipient_id}] =
        DiplomaticMessage(sender_id, recipient_id, DiplomaticMessage::Type::INVALID);

    // if there already was a message, and it wasn't already a non-message, notify about change
    if (changed)
        DiplomaticMessageChangedSignal(sender_id, recipient_id);
}

void EmpireManager::HandleDiplomaticMessage(const DiplomaticMessage& message) {
    int sender_empire_id = message.SenderEmpireID();
    int recipient_empire_id = message.RecipientEmpireID();

    if (!message.IsAllowed())
        return;

    DiplomaticStatus diplo_status = GetDiplomaticStatus(sender_empire_id, recipient_empire_id);
    bool message_from_recipient_to_sender_available = DiplomaticMessageAvailable(recipient_empire_id, sender_empire_id);
    const DiplomaticMessage& existing_message_from_recipient_to_sender = GetDiplomaticMessage(recipient_empire_id, sender_empire_id);
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

std::vector<EmpireColor>& EmpireColorsNonConst() {
    static std::vector<EmpireColor> colors;
    return colors;
}

/** Named ctor that constructs a EmpireColor from a string that represents the color
    channels in the format '#RRGGBB', '#RRGGBBAA' where each channel value
    ranges from 0 to FF.  When the alpha component is left out the alpha
    value FF is assumed.
    @throws std::invalid_argument if the hex_colour string is not well formed
    */
inline EmpireColor HexClr(const std::string& hex_colour)
{
    std::istringstream iss(hex_colour);

    unsigned long rgba = 0;
    if ((hex_colour.size() == 7 || hex_colour.size() == 9) &&
            '#' == iss.get() && !(iss >> std::hex >> rgba).fail())
    {
        EmpireColor retval = EmpireColor{0, 0, 0, 255};

        if (hex_colour.size() == 7) {
            std::get<0>(retval) = (rgba >> 16) & 0xFF;
            std::get<1>(retval) = (rgba >> 8)  & 0xFF;
            std::get<2>(retval) = rgba         & 0xFF;
            std::get<3>(retval) = 255;
        } else {
            std::get<0>(retval) = (rgba >> 24) & 0xFF;
            std::get<1>(retval) = (rgba >> 16) & 0xFF;
            std::get<2>(retval) = (rgba >> 8)  & 0xFF;
            std::get<3>(retval) = rgba         & 0xFF;
        }

        return retval;
    }

    throw std::invalid_argument("EmpireColor could not interpret hex colour string");
}

void InitEmpireColors(const boost::filesystem::path& path) {
    auto& colors = EmpireColorsNonConst();

    XMLDoc doc;

    boost::filesystem::ifstream ifs(path);
    if (ifs) {
        doc.ReadDoc(ifs);
        ifs.close();
    } else {
        ErrorLogger() << "Unable to open data file " << path.filename();
        return;
    }

    for (const XMLElement& elem : doc.root_node.children) {
        try {
            std::string hex_colour("#");
            hex_colour.append(elem.attributes.at("hex"));
            colors.push_back(HexClr(hex_colour));
        } catch(const std::exception& e) {
            ErrorLogger() << "empire_colors.xml: " << e.what() << "\n";
        }
    }
}

const std::vector<EmpireColor>& EmpireColors() {
    auto& colors = EmpireColorsNonConst();
    if (colors.empty()) {
        colors = {  {  0, 255,   0, 255},    {  0,   0, 255, 255},    {255,   0,   0, 255},
                    {  0, 255, 255, 255},    {255, 255,   0, 255},    {255,   0, 255, 255}};
    }
    return colors;
}
