#include "EmpireManager.h"

#include "Empire.h"
#include "../universe/Planet.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"

namespace {
    std::pair<int, int> DiploKey(int id1, int ind2)
    { return std::make_pair(std::max(id1, ind2), std::min(id1, ind2)); }
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

EmpireManager::~EmpireManager()
{ Clear(); }

const EmpireManager& EmpireManager::operator=(EmpireManager& rhs) {
    Clear();
    m_empire_map = rhs.m_empire_map;
    rhs.m_empire_map.clear();
    return *this;
}

const Empire* EmpireManager::Lookup(int id) const {
    const_iterator it = m_empire_map.find(id);
    return it == m_empire_map.end() ? 0 : it->second;
}

EmpireManager::const_iterator EmpireManager::begin() const
{ return m_empire_map.begin(); }

EmpireManager::const_iterator EmpireManager::end() const
{ return m_empire_map.end(); }

bool EmpireManager::Eliminated(int id) const
{ return m_eliminated_empires.find(id) != m_eliminated_empires.end(); }

std::string EmpireManager::Dump() const {
    std::string retval = "Empires:\n";
    for (const_iterator it = begin(); it != end(); ++it)
        retval += it->second->Dump();
    retval += "Diplomatic Statuses:\n";
    for (std::map<std::pair<int, int>, DiplomaticStatus>::const_iterator it = m_empire_diplomatic_statuses.begin();
         it != m_empire_diplomatic_statuses.end(); ++it)
    {
        const Empire* empire1 = Lookup(it->first.first);
        const Empire* empire2 = Lookup(it->first.second);
        if (!empire1 || !empire2)
            continue;
        retval += " * " + empire1->Name() + " / " + empire2->Name() + " : ";
        switch (it->second) {
        case DIPLO_WAR:     retval += "War";    break;
        case DIPLO_PEACE:   retval += "Peace";  break;
        default:            retval += "?";      break;
        }
        retval += "\n";
    }
    return retval;
}

Empire* EmpireManager::Lookup(int id) {
    iterator it = m_empire_map.find(id);
    return it == end() ? 0 : it->second;
}

EmpireManager::iterator EmpireManager::begin()
{ return m_empire_map.begin(); }

EmpireManager::iterator EmpireManager::end()
{ return m_empire_map.end(); }

void EmpireManager::BackPropegateMeters() {
    for (iterator it = m_empire_map.begin(); it != m_empire_map.end(); ++it)
        it->second->BackPropegateMeters();
}

void EmpireManager::EliminateEmpire(int id) {
    if (Empire* emp = Lookup(id)) {
        emp->EliminationCleanup();
        m_eliminated_empires.insert(id);
    } else {
        Logger().errorStream() << "Tried to eliminate nonexistant empire with ID " << id;
    }
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
        Logger().errorStream() << "EmpireManager::InsertEmpire passed null empire";
        return;
    }

    int empire_id = empire->EmpireID();

    if (m_empire_map.find(empire_id) != m_empire_map.end()) {
        Logger().errorStream() << "EmpireManager::InsertEmpire passed empire with id (" << empire_id << ") for which there already is an empire.";
        return;
    }

    m_empire_map[empire_id] = empire;
}

void EmpireManager::Clear() {
    for (EmpireManager::iterator it = begin(); it != end(); ++it)
        delete it->second;
    m_empire_map.clear();
    m_eliminated_empires.clear();
    m_empire_diplomatic_statuses.clear();
}

DiplomaticStatus EmpireManager::GetDiplomaticStatus(int empire1, int empire2) const {
    if (empire1 == ALL_EMPIRES || empire2 == ALL_EMPIRES) {
        Logger().errorStream() << "EmpireManager::GetDiplomaticStatus passed invalid empire id";
        return INVALID_DIPLOMATIC_STATUS;
    }

    std::map<std::pair<int, int>, DiplomaticStatus>::const_iterator it =
        m_empire_diplomatic_statuses.find(DiploKey(empire1, empire2));
    if (it != m_empire_diplomatic_statuses.end())
        return it->second;
    Logger().errorStream() << "Couldn't find diplomatic status between empires " << empire1 << " and " << empire2;
    return INVALID_DIPLOMATIC_STATUS;
}

bool EmpireManager::DiplomaticMessageAvailable(int empire1, int empire2) const {
    std::map<std::pair<int, int>, DiplomaticMessage>::const_iterator it =
        m_diplomatic_messages.find(DiploKey(empire1, empire2));
    return it != m_diplomatic_messages.end() &&
           it->second.GetType() != DiplomaticMessage::INVALID_DIPLOMATIC_MESSAGE_TYPE;
}

const DiplomaticMessage& EmpireManager::GetDiplomaticMessage(int empire1, int empire2) const {
    std::map<std::pair<int, int>, DiplomaticMessage>::const_iterator it =
        m_diplomatic_messages.find(DiploKey(empire1, empire2));
    if (it != m_diplomatic_messages.end())
        return it->second;
    static DiplomaticMessage DEFAULT_DIPLOMATIC_MESSAGE;
    Logger().errorStream() << "Couldn't find diplomatic message between empires " << empire1 << " and " << empire2;
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

void EmpireManager::RemoveDiplomaticMessage(int empire1, int empire2) {
    std::pair<int, int> key = DiploKey(empire1, empire2);
    std::map<std::pair<int, int>, DiplomaticMessage>::iterator it = m_diplomatic_messages.find(key);
    if (it != m_diplomatic_messages.end()) {
        m_diplomatic_messages[key] = DiplomaticMessage(empire1, empire2, DiplomaticMessage::INVALID_DIPLOMATIC_MESSAGE_TYPE);
        DiplomaticMessageChangedSignal(empire1, empire2);
        return;
    }
    Logger().errorStream() << "Was no diplomatic message entry between empires " << empire1 << " and " << empire2;
    m_diplomatic_messages[key] = DiplomaticMessage(empire1, empire2, DiplomaticMessage::INVALID_DIPLOMATIC_MESSAGE_TYPE);
}

void EmpireManager::HandleDiplomaticMessage(const DiplomaticMessage& message) {
    int sender_empire_id = message.SenderEmpireID();
    int recipient_empire_id = message.RecipientEmpireID();
    std::pair<int, int> key = DiploKey(sender_empire_id, recipient_empire_id);
    DiplomaticStatus diplo_status = GetDiplomaticStatus(sender_empire_id, recipient_empire_id);
    bool message_already_available = DiplomaticMessageAvailable(sender_empire_id, recipient_empire_id);
    const DiplomaticMessage& existing_message = GetDiplomaticMessage(sender_empire_id, recipient_empire_id);

    switch (message.GetType()) {
    case DiplomaticMessage::WAR_DECLARATION: {
        if (diplo_status == DIPLO_PEACE) {
            RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id); // todo leave some kinds of messages?
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
    case DiplomaticMessage::ACCEPT_PROPOSAL: {
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
    case DiplomaticMessage::CANCEL_PROPOSAL: {
        if (message_already_available &&
            existing_message.SenderEmpireID() == message.SenderEmpireID())
        {
            if (existing_message.GetType() == DiplomaticMessage::PEACE_PROPOSAL) {
                RemoveDiplomaticMessage(sender_empire_id, recipient_empire_id);
            }
        }
        break;
    }
    default:
        break;
    }
}

void EmpireManager::ResetDiplomacy() {
    m_diplomatic_messages.clear();
    m_empire_diplomatic_statuses.clear();

    for (std::map<int, Empire*>::const_iterator emp1_it = m_empire_map.begin(); emp1_it != m_empire_map.end(); ++emp1_it) {
        std::map<int, Empire*>::const_iterator emp2_it = emp1_it;
        emp2_it++;
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
    std::map<std::pair<int, int>, DiplomaticMessage>::const_iterator it;
    for (it = m_diplomatic_messages.begin();
         it != m_diplomatic_messages.end(); ++it)
    {
        if (it->first.first == encoding_empire || it->first.second == encoding_empire)
            messages.insert(*it);
    }
}
