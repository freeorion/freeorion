#include "CombatLogManager.h"
#include "../universe/UniverseObject.h"
#include "../util/Serialize.h"
#include "../util/Serialize.ipp"
#include "CombatEvents.h"

////////////////////////////////////////////////
// CombatLog
////////////////////////////////////////////////
CombatLog::CombatLog() :
    turn(INVALID_GAME_TURN),
    system_id(INVALID_OBJECT_ID)
{}

CombatLog::CombatLog(const CombatInfo& combat_info) :
    turn(combat_info.turn),
    system_id(combat_info.system_id),
    empire_ids(combat_info.empire_ids),
    object_ids(),
    damaged_object_ids(combat_info.damaged_object_ids),
    destroyed_object_ids(combat_info.destroyed_object_ids),
    combat_events(combat_info.combat_events)
{
    // compile all remaining and destroyed objects' ids
    object_ids = combat_info.destroyed_object_ids;
    for (ObjectMap::const_iterator<> it = combat_info.objects.const_begin();
         it != combat_info.objects.const_end(); ++it)
    { object_ids.insert(it->ID()); }
}


template <class Archive>
void CombatLog::serialize(Archive& ar, const unsigned int version)
{
    // CombatEvents are serialized only through
    // pointers to their base class.
    // Therefore we need to manually register their types
    // in the archive.
    ar.template register_type<AttackEvent>();
    ar.template register_type<IncapacitationEvent>();
    ar.template register_type<BoutBeginEvent>();
    
    ar  & BOOST_SERIALIZATION_NVP(turn)
    & BOOST_SERIALIZATION_NVP(system_id)
    & BOOST_SERIALIZATION_NVP(empire_ids)
    & BOOST_SERIALIZATION_NVP(object_ids)
    & BOOST_SERIALIZATION_NVP(damaged_object_ids)
    & BOOST_SERIALIZATION_NVP(destroyed_object_ids)
    & BOOST_SERIALIZATION_NVP(combat_events);
}

template
void CombatLog::serialize<freeorion_iarchive>(freeorion_iarchive& ar, const unsigned int version);
template
void CombatLog::serialize<freeorion_oarchive>(freeorion_oarchive& ar, const unsigned int version);


////////////////////////////////////////////////
// CombatLogManager
////////////////////////////////////////////////
CombatLogManager::CombatLogManager() :
    m_logs(),
    m_latest_log_id(-1)
{}

std::map<int, CombatLog>::const_iterator CombatLogManager::begin() const
{ return m_logs.begin(); }

std::map<int, CombatLog>::const_iterator CombatLogManager::end() const
{ return m_logs.end(); }

std::map<int, CombatLog>::const_iterator CombatLogManager::find(int log_id) const
{ return m_logs.find(log_id); }

bool CombatLogManager::LogAvailable(int log_id) const
{ return m_logs.begin() != m_logs.end(); }

const CombatLog& CombatLogManager::GetLog(int log_id) const {
    std::map<int, CombatLog>::const_iterator it = m_logs.find(log_id);
    if (it != m_logs.end())
        return it->second;
    static CombatLog EMPTY_LOG;
    return EMPTY_LOG;
}

int CombatLogManager::AddLog(const CombatLog& log) {
    int new_log_id = ++m_latest_log_id;
    m_logs[new_log_id] = log;
    return new_log_id;
}

void CombatLogManager::RemoveLog(int log_id)
{ m_logs.erase(log_id); }

void CombatLogManager::Clear()
{ m_logs.clear(); }

void CombatLogManager::GetLogsToSerialize(std::map<int, CombatLog>& logs, int encoding_empire) const {
    if (&logs == &m_logs)
        return;
    // TODO: filter logs by who should have access to them
    logs = m_logs;
}

void CombatLogManager::SetLog(int log_id, const CombatLog& log)
{ m_logs[log_id] = log; }

CombatLogManager& CombatLogManager::GetCombatLogManager() {
    static CombatLogManager manager;
    return manager;
}

///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
CombatLogManager& GetCombatLogManager()
{ return CombatLogManager::GetCombatLogManager(); }

const CombatLog& GetCombatLog(int log_id)
{ return GetCombatLogManager().GetLog(log_id); }

bool CombatLogAvailable(int log_id)
{ return GetCombatLogManager().LogAvailable(log_id); }
