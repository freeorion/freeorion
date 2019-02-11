#include "EmpireManager.h"

#include "Empire.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/XMLDoc.h"
#include "../universe/Enums.h"

#include <boost/filesystem/fstream.hpp>

namespace {
    // sorted pair, so order of empire IDs specified doesn't matter
    std::pair<int, int> DiploKey(int id1, int ind2)
    { return std::make_pair(std::max(id1, ind2), std::min(id1, ind2)); }

    const std::string EMPTY_STRING;
}

EmpireManager::EmpireManager()
{}

EmpireManager::~EmpireManager()
{ Clear(); }

const EmpireManager& EmpireManager::operator=(EmpireManager& rhs) {
    Clear();
    m_empire_map = rhs.m_empire_map;
    rhs.m_empire_map.clear();
    return *this;
}

const Empire* EmpireManager::GetEmpire(int id) const {
    auto it = m_empire_map.find(id);
    return it == m_empire_map.end() ? nullptr : it->second;
}

std::shared_ptr<const UniverseObject> EmpireManager::GetSource(int id) const {
    auto it = m_empire_map.find(id);
    return it != m_empire_map.end() ? it->second->Source() : nullptr;
}

const std::string& EmpireManager::GetEmpireName(int id) const {
    auto it = m_empire_map.find(id);
    return it == m_empire_map.end() ? EMPTY_STRING : it->second->Name();
}

EmpireManager::const_iterator EmpireManager::begin() const
{ return m_empire_map.begin(); }

EmpireManager::const_iterator EmpireManager::end() const
{ return m_empire_map.end(); }

int EmpireManager::NumEmpires() const
{ return m_empire_map.size(); }

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
        const Empire* empire1 = GetEmpire(entry.first.first);
        const Empire* empire2 = GetEmpire(entry.first.second);
        if (!empire1 || !empire2)
            continue;
        retval += " * " + empire1->Name() + " / " + empire2->Name() + " : ";
        switch (entry.second) {
        case DIPLO_WAR:     retval += "War";    break;
        case DIPLO_PEACE:   retval += "Peace";  break;
        case DIPLO_ALLIED:  retval += "Allied";  break;
        default:            retval += "?";      break;
        }
        retval += "\n";
    }
    retval += "Diplomatic Messages:\n";
    for (const auto& message : m_diplomatic_messages) {
        if (message.second.GetType() == DiplomaticMessage::INVALID_DIPLOMATIC_MESSAGE_TYPE)
            continue;   // don't print non-messages and pollute the log files...
        retval += "From: " + std::to_string(message.first.first)
               + " to: " + std::to_string(message.first.second)
               + " message: " + message.second.Dump() + "\n";
    }

    return retval;
}

Empire* EmpireManager::GetEmpire(int id) {
    iterator it = m_empire_map.find(id);
    return it == end() ? nullptr : it->second;
}

EmpireManager::iterator EmpireManager::begin()
{ return m_empire_map.begin(); }

EmpireManager::iterator EmpireManager::end()
{ return m_empire_map.end(); }

void EmpireManager::BackPropagateMeters() {
    for (auto& entry : m_empire_map)
        entry.second->BackPropagateMeters();
}

Empire* EmpireManager::CreateEmpire(int empire_id, const std::string& name,
                                    const std::string& player_name,
                                    const GG::Clr& color, bool authenticated)
{
    Empire* empire = new Empire(name, player_name, empire_id, color, authenticated);
    InsertEmpire(empire);
    return empire;
}

void EmpireManager::InsertEmpire(Empire* empire) {
    if (!empire) {
        ErrorLogger() << "EmpireManager::InsertEmpire passed null empire";
        return;
    }

    int empire_id = empire->EmpireID();

    if (m_empire_map.count(empire_id)) {
        ErrorLogger() << "EmpireManager::InsertEmpire passed empire with id (" << empire_id << ") for which there already is an empire.";
        return;
    }

    m_empire_map[empire_id] = empire;
}

void EmpireManager::Clear() {
    for (auto& entry : m_empire_map)
        delete entry.second;
    m_empire_map.clear();
    m_empire_diplomatic_statuses.clear();
}

DiplomaticStatus EmpireManager::GetDiplomaticStatus(int empire1, int empire2) const {
    if (empire1 == ALL_EMPIRES || empire2 == ALL_EMPIRES || empire1 == empire2)
        return INVALID_DIPLOMATIC_STATUS;

    auto it = m_empire_diplomatic_statuses.find(DiploKey(empire1, empire2));
    if (it != m_empire_diplomatic_statuses.end())
        return it->second;
    ErrorLogger() << "Couldn't find diplomatic status between empires " << empire1 << " and " << empire2;
    return INVALID_DIPLOMATIC_STATUS;
}

std::set<int> EmpireManager::GetEmpireIDsWithDiplomaticStatusWithEmpire(
    int empire_id, DiplomaticStatus diplo_status) const
{
    std::set<int> retval;
    if (empire_id == ALL_EMPIRES || diplo_status == INVALID_DIPLOMATIC_STATUS)
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
           it->second.GetType() != DiplomaticMessage::INVALID_DIPLOMATIC_MESSAGE_TYPE;
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
                   (it->second.GetType() != DiplomaticMessage::INVALID_DIPLOMATIC_MESSAGE_TYPE);

    m_diplomatic_messages[{sender_id, recipient_id}] =
        DiplomaticMessage(sender_id, recipient_id, DiplomaticMessage::INVALID_DIPLOMATIC_MESSAGE_TYPE);

    // if there already was a message, and it wasn't already a non-message, notify about change
    if (changed)
        DiplomaticMessageChangedSignal(sender_id, recipient_id);
}

void EmpireManager::HandleDiplomaticMessage(const DiplomaticMessage& message) {
    int sender_empire_id = message.SenderEmpireID();
    int recipient_empire_id = message.RecipientEmpireID();

    DiplomaticStatus diplo_status = GetDiplomaticStatus(sender_empire_id, recipient_empire_id);
    bool message_from_recipient_to_sender_available = DiplomaticMessageAvailable(recipient_empire_id, sender_empire_id);
    const DiplomaticMessage& existing_message_from_recipient_to_sender = GetDiplomaticMessage(recipient_empire_id, sender_empire_id);
    //bool message_from_sender_to_recipient_available = DiplomaticMessageAvailable(sender_empire_id, recipient_empire_id);

    switch (message.GetType()) {
    case DiplomaticMessage::WAR_DECLARATION: {
        if (diplo_status == DIPLO_PEACE) {
            // cancels any previous messages, sets empires at war
            RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
            RemoveDiplomaticMessage(recipient_empire_id, sender_empire_id);
            SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DIPLO_WAR);
        }
        break;
    }

    case DiplomaticMessage::PEACE_PROPOSAL: {
        if (diplo_status == DIPLO_WAR && !message_from_recipient_to_sender_available) {
            SetDiplomaticMessage(message);

        } else if (diplo_status == DIPLO_WAR && message_from_recipient_to_sender_available) {
            if (existing_message_from_recipient_to_sender.GetType() ==
                DiplomaticMessage::PEACE_PROPOSAL)
            {
                // somehow multiple peace proposals sent by players to eachother
                // cancel and remove
                RemoveDiplomaticMessage(recipient_empire_id, sender_empire_id);
                RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
                SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DIPLO_PEACE);
            }
        }
        break;
    }

    case DiplomaticMessage::ACCEPT_PEACE_PROPOSAL: {
        if (message_from_recipient_to_sender_available &&
            existing_message_from_recipient_to_sender.GetType() == DiplomaticMessage::PEACE_PROPOSAL)
        {
            // one player proposed peace and the other accepted
            RemoveDiplomaticMessage(recipient_empire_id, sender_empire_id);
            RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
            SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DIPLO_PEACE);
        }
        break;
    }

    case DiplomaticMessage::ALLIES_PROPOSAL: {
        if (diplo_status == DIPLO_PEACE && !message_from_recipient_to_sender_available) {
            SetDiplomaticMessage(message);

        } else if (diplo_status == DIPLO_PEACE && message_from_recipient_to_sender_available) {
            if (existing_message_from_recipient_to_sender.GetType() ==
                DiplomaticMessage::ALLIES_PROPOSAL)
            {
                // somehow multiple allies proposals sent by players to eachother
                // cancel and remove
                RemoveDiplomaticMessage(recipient_empire_id, sender_empire_id);
                RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
                SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DIPLO_ALLIED);
            }
        }
        break;
    }

    case DiplomaticMessage::ACCEPT_ALLIES_PROPOSAL: {
        if (message_from_recipient_to_sender_available &&
            existing_message_from_recipient_to_sender.GetType() == DiplomaticMessage::ALLIES_PROPOSAL)
        {
            // one player proposed alliance and the other accepted
            RemoveDiplomaticMessage(recipient_empire_id, sender_empire_id);
            RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
            SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DIPLO_ALLIED);
        }
        break;
    }

    case DiplomaticMessage::END_ALLIANCE_DECLARATION: {
        if (diplo_status == DIPLO_ALLIED) {
            // cancels any previous messages, sets empires to peace
            RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
            RemoveDiplomaticMessage(recipient_empire_id, sender_empire_id);
            SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DIPLO_PEACE);
        }
        break;
    }

    case DiplomaticMessage::CANCEL_PROPOSAL: {
        RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
        break;
    }

    case DiplomaticMessage::REJECT_PROPOSAL: {
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
    for (auto id_empire_1 : m_empire_map) {
        for (auto id_empire_2 : m_empire_map) {
            if (id_empire_1.first == id_empire_2.first)
                continue;
            const std::pair<int, int> diplo_key = DiploKey(id_empire_1.first, id_empire_2.first);
            m_empire_diplomatic_statuses[diplo_key] = DIPLO_WAR;
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

std::vector<GG::Clr>& EmpireColorsNonConst() {
    static std::vector<GG::Clr> colors;
    return colors;
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
            colors.push_back(GG::HexClr(hex_colour));
        } catch(const std::exception& e) {
            ErrorLogger() << "empire_colors.xml: " << e.what() << std::endl;
        }
    }
}
const std::vector<GG::Clr>& EmpireColors() {
    auto& colors = EmpireColorsNonConst();
    if (colors.empty()) {
        colors = {  GG::Clr(  0, 255,   0, 255),    GG::Clr(  0,   0, 255, 255),    GG::Clr(255,   0,   0, 255),
                    GG::Clr(  0, 255, 255, 255),    GG::Clr(255, 255,   0, 255),    GG::Clr(255,   0, 255, 255)};
    }
    return colors;
}
