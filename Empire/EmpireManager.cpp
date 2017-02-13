#include "EmpireManager.h"

#include "Empire.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"
#include "../util/XMLDoc.h"
#include "../universe/Enums.h"

#include <boost/filesystem/fstream.hpp>

namespace {
    std::pair<int, int> DiploKey(int id1, int ind2)
    { return std::make_pair(std::max(id1, ind2), std::min(id1, ind2)); }

    const std::string EMPTY_STRING;
}

DiplomaticStatusUpdateInfo::DiplomaticStatusUpdateInfo() :
    empire1_id(ALL_EMPIRES),
    empire2_id(ALL_EMPIRES),
    diplo_status(INVALID_DIPLOMATIC_STATUS)
{}

DiplomaticStatusUpdateInfo::DiplomaticStatusUpdateInfo(int empire1_id_, int empire2_id_, DiplomaticStatus status) :
    empire1_id(empire1_id_),
    empire2_id(empire2_id_),
    diplo_status(status)
{}

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
    const_iterator it = m_empire_map.find(id);
    return it == m_empire_map.end() ? nullptr : it->second;
}

std::shared_ptr<const UniverseObject> EmpireManager::GetSource(int id) const {
    auto it = m_empire_map.find(id);
    return it != m_empire_map.end() ? it->second->Source() : nullptr;
}

const std::string& EmpireManager::GetEmpireName(int id) const {
    const_iterator it = m_empire_map.find(id);
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
    for (const std::map<int, Empire*>::value_type& entry : m_empire_map)
        retval += entry.second->Dump();
    retval += DumpDiplomacy();
    return retval;
}

std::string EmpireManager::DumpDiplomacy() const {
    std::string retval = "Diplomatic Statuses:\n";
    for (const std::map<std::pair<int, int>, DiplomaticStatus>::value_type&
         entry : m_empire_diplomatic_statuses)
    {
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
    for (auto message : m_diplomatic_messages) {
        retval += "From: " + std::to_string(message.first.first)
               + " to: " + std::to_string(message.first.second)
               + " message:" + message.second.Dump();
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
    for (std::map<int, Empire*>::value_type& entry : m_empire_map)
        entry.second->BackPropagateMeters();
}

Empire* EmpireManager::CreateEmpire(int empire_id, const std::string& name,
                                    const std::string& player_name,
                                    const GG::Clr& color)
{
    Empire* empire = new Empire(name, player_name, empire_id, color);
    InsertEmpire(empire);
    return empire;
}

void EmpireManager::InsertEmpire(Empire* empire) {
    if (!empire) {
        ErrorLogger() << "EmpireManager::InsertEmpire passed null empire";
        return;
    }

    int empire_id = empire->EmpireID();

    if (m_empire_map.find(empire_id) != m_empire_map.end()) {
        ErrorLogger() << "EmpireManager::InsertEmpire passed empire with id (" << empire_id << ") for which there already is an empire.";
        return;
    }

    m_empire_map[empire_id] = empire;
}

void EmpireManager::Clear() {
    for (std::map<int, Empire*>::value_type& entry : m_empire_map)
        delete entry.second;
    m_empire_map.clear();
    m_empire_diplomatic_statuses.clear();
}

DiplomaticStatus EmpireManager::GetDiplomaticStatus(int empire1, int empire2) const {
    if (empire1 == ALL_EMPIRES || empire2 == ALL_EMPIRES)
        return INVALID_DIPLOMATIC_STATUS;

    auto it = m_empire_diplomatic_statuses.find(DiploKey(empire1, empire2));
    if (it != m_empire_diplomatic_statuses.end())
        return it->second;
    ErrorLogger() << "Couldn't find diplomatic status between empires " << empire1 << " and " << empire2;
    return INVALID_DIPLOMATIC_STATUS;
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
    ErrorLogger() << "Couldn't find diplomatic message between empires " << sender_id << " and " << recipient_id;
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
        m_diplomatic_messages[DiploKey(empire1, empire2)] = message;
        DiplomaticMessageChangedSignal(empire1, empire2);
    }
}

void EmpireManager::RemoveDiplomaticMessage(int sender_id, int recipient_id) {
    auto it = m_diplomatic_messages.find({sender_id, recipient_id});
    if (it != m_diplomatic_messages.end()) {
        m_diplomatic_messages[{sender_id, recipient_id}] =
            DiplomaticMessage(sender_id, recipient_id, DiplomaticMessage::INVALID_DIPLOMATIC_MESSAGE_TYPE);
        DiplomaticMessageChangedSignal(sender_id, recipient_id);
        return;
    }
    ErrorLogger() << "Was no diplomatic message entry between empires " << sender_id << " and " << recipient_id;
    m_diplomatic_messages[{sender_id, recipient_id}] =
        DiplomaticMessage(sender_id, recipient_id, DiplomaticMessage::INVALID_DIPLOMATIC_MESSAGE_TYPE);
}

void EmpireManager::HandleDiplomaticMessage(const DiplomaticMessage& message) {
    DebugLogger() << "HandleDiplomaticMessage start message: " << message.Dump();
    DebugLogger() << this->DumpDiplomacy();

    int sender_empire_id = message.SenderEmpireID();
    int recipient_empire_id = message.RecipientEmpireID();

    DiplomaticStatus diplo_status = GetDiplomaticStatus(sender_empire_id, recipient_empire_id);
    bool message_already_available = DiplomaticMessageAvailable(sender_empire_id, recipient_empire_id);
    const DiplomaticMessage& existing_message = GetDiplomaticMessage(sender_empire_id, recipient_empire_id);

    switch (message.GetType()) {
    case DiplomaticMessage::WAR_DECLARATION: {
        if (diplo_status == DIPLO_PEACE) {
            RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
            SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DIPLO_WAR);
        }
        break;
    }
    case DiplomaticMessage::PEACE_PROPOSAL: {
        if (diplo_status == DIPLO_WAR && !message_already_available) {
            SetDiplomaticMessage(message);
        } else if (diplo_status == DIPLO_WAR &&
                   message_already_available &&
                   existing_message.SenderEmpireID() == message.RecipientEmpireID())
        {
            if (existing_message.GetType() == DiplomaticMessage::PEACE_PROPOSAL) {
                RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
                SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DIPLO_PEACE);
            }
        }
        break;
    }
    case DiplomaticMessage::ACCEPT_PEACE_PROPOSAL: {
        if (message_already_available &&
            existing_message.SenderEmpireID() == message.RecipientEmpireID())
        {
            if (existing_message.GetType() == DiplomaticMessage::PEACE_PROPOSAL) {
                RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
                SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DIPLO_PEACE);
            }
        }
        break;
    }
    case DiplomaticMessage::ALLIES_PROPOSAL: {
        if (diplo_status == DIPLO_PEACE && !message_already_available) {
            SetDiplomaticMessage(message);
        }
        else if (diplo_status == DIPLO_PEACE &&
            message_already_available &&
            existing_message.SenderEmpireID() == message.RecipientEmpireID())
        {
            if (existing_message.GetType() == DiplomaticMessage::ALLIES_PROPOSAL) {
                RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
                SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DIPLO_ALLIED);
            }
        }
        break;
    }
    case DiplomaticMessage::ACCEPT_ALLIES_PROPOSAL: {
        if (message_already_available &&
            existing_message.SenderEmpireID() == message.RecipientEmpireID())
        {
            if (existing_message.GetType() == DiplomaticMessage::ALLIES_PROPOSAL) {
                RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
                SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DIPLO_ALLIED);
            }
        }
        break;
    }
    case DiplomaticMessage::END_ALLIANCE_DECLARATION: {
        if (diplo_status == DIPLO_ALLIED) {
            RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
            SetDiplomaticStatus(sender_empire_id, recipient_empire_id, DIPLO_PEACE);
        }
        break;
    }
    case DiplomaticMessage::CANCEL_PROPOSAL: {
        if (message_already_available &&
            existing_message.SenderEmpireID() == message.SenderEmpireID())
        {
            if (existing_message.GetType() == DiplomaticMessage::PEACE_PROPOSAL ||
                existing_message.GetType() == DiplomaticMessage::ALLIES_PROPOSAL)
            {
                RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
            }
        }
        break;
    }
    case DiplomaticMessage::REJECT_PROPOSAL: {
        if (message_already_available &&
            existing_message.SenderEmpireID() == message.SenderEmpireID())
        {
            if (existing_message.GetType() == DiplomaticMessage::PEACE_PROPOSAL ||
                existing_message.GetType() == DiplomaticMessage::ALLIES_PROPOSAL)
            {
                RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
            }
        }
        break;
    }
    default:
        break;
    }
    DebugLogger() << "HandleDiplomaticMessage end";
    DebugLogger() << this->DumpDiplomacy();

}

void EmpireManager::ResetDiplomacy() {
    m_diplomatic_messages.clear();
    m_empire_diplomatic_statuses.clear();

    for (std::map<int, Empire*>::const_iterator emp1_it = m_empire_map.begin(); emp1_it != m_empire_map.end(); ++emp1_it) {
        std::map<int, Empire*>::const_iterator emp2_it = emp1_it;
        ++emp2_it;
        for (; emp2_it != m_empire_map.end(); ++emp2_it) {
            const std::pair<int, int> diplo_key = DiploKey(emp1_it->first, emp2_it->first);
            m_empire_diplomatic_statuses[diplo_key] = DIPLO_PEACE;
            m_diplomatic_messages[diplo_key] = DiplomaticMessage(diplo_key.first, diplo_key.second, 
                                                                 DiplomaticMessage::INVALID_DIPLOMATIC_MESSAGE_TYPE);
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
    for (const std::map<std::pair<int, int>, DiplomaticMessage>::value_type& entry : m_diplomatic_messages) {
        if (entry.first.first == encoding_empire || entry.first.second == encoding_empire)
            messages.insert(entry);
    }
}

const std::vector<GG::Clr>& EmpireColors() {
    static std::vector<GG::Clr> colors;
    if (colors.empty()) {
        XMLDoc doc;

        std::string file_name = "empire_colors.xml";

        boost::filesystem::ifstream ifs(GetResourceDir() / file_name);
        if (ifs) {
            doc.ReadDoc(ifs);
            ifs.close();
        } else {
            ErrorLogger() << "Unable to open data file " << file_name;
            return colors;
        }

        for (const XMLElement& elem : doc.root_node.children) {
            try {
                std::string hex_colour("#");
                hex_colour.append(elem.attributes.at("hex"));
                colors.push_back(GG::HexClr(hex_colour));
            } catch(const std::exception& e) {
                std::cerr << "empire_colors.xml: " << e.what() << std::endl;
            }
        }
    }
    if (colors.empty()) {
        colors = {  GG::Clr(  0, 255,   0, 255),    GG::Clr(  0,   0, 255, 255),    GG::Clr(255,   0,   0, 255),
                    GG::Clr(  0, 255, 255, 255),    GG::Clr(255, 255,   0, 255),    GG::Clr(255,   0, 255, 255)};
    }
    return colors;
}

