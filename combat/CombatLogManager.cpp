#include "CombatLogManager.h"
#include "../universe/Meter.h"
#include "../universe/UniverseObject.h"
#include "../universe/Enums.h"
#include "../util/Serialize.h"
#include "../util/Serialize.ipp"
#include "../util/Logger.h"
#include "CombatEvents.h"


namespace {
    static float MaxHealth(const UniverseObject& object) {
        if (object.ObjectType() == OBJ_SHIP) {
            return object.GetMeter(METER_MAX_STRUCTURE)->Current();

        } else if ( object.ObjectType() == OBJ_PLANET ) {
            const Meter* defense = object.GetMeter(METER_MAX_DEFENSE);
            const Meter* shield = object.GetMeter(METER_MAX_SHIELD);
            const Meter* construction = object.GetMeter(METER_TARGET_CONSTRUCTION);

            float ret = 0.0f;
            if (defense)
                ret += defense->Current();
            if (shield)
                ret += shield->Current();
            if (construction)
                ret += construction->Current();
            return ret;
        }

        return 0.0f;
    }

    static float CurrentHealth(const UniverseObject& object) {
        if (object.ObjectType() == OBJ_SHIP) {
            return object.GetMeter(METER_STRUCTURE)->Current();

        } else if (object.ObjectType() == OBJ_PLANET) {
            const Meter* defense = object.GetMeter(METER_DEFENSE);
            const Meter* shield = object.GetMeter(METER_SHIELD);
            const Meter* construction = object.GetMeter(METER_CONSTRUCTION);

            float ret = 0.0f;
            if (defense)
                ret += defense->Current();
            if (shield)
                ret += shield->Current();
            if (construction)
                ret += construction->Current();
            return ret;
        }

        return 0.0f;
    }

    static void FillState(CombatParticipantState& state, const UniverseObject& object) {
        state.current_health = CurrentHealth(object);
        state.max_health = MaxHealth(object);
    };
}

CombatParticipantState::CombatParticipantState() {}

CombatParticipantState::CombatParticipantState(const UniverseObject& object)
{ FillState(*this, object); }

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
    for (const auto& obj : combat_info.objects.all()) {
        object_ids.insert(obj->ID());
        participant_states[obj->ID()] = CombatParticipantState(*obj);
    }
}

template <typename Archive>
void CombatParticipantState::serialize(Archive& ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(current_health)
       & BOOST_SERIALIZATION_NVP(max_health);
}

template <typename Archive>
void CombatLog::serialize(Archive& ar, const unsigned int version)
{
    // CombatEvents are serialized only through
    // pointers to their base class.
    // Therefore we need to manually register their types
    // in the archive.
    ar.template register_type<WeaponFireEvent>();
    ar.template register_type<IncapacitationEvent>();
    ar.template register_type<BoutBeginEvent>();
    ar.template register_type<InitialStealthEvent>();
    ar.template register_type<StealthChangeEvent>();
    ar.template register_type<WeaponsPlatformEvent>();

    ar  & BOOST_SERIALIZATION_NVP(turn)
        & BOOST_SERIALIZATION_NVP(system_id)
        & BOOST_SERIALIZATION_NVP(empire_ids)
        & BOOST_SERIALIZATION_NVP(object_ids)
        & BOOST_SERIALIZATION_NVP(damaged_object_ids)
        & BOOST_SERIALIZATION_NVP(destroyed_object_ids);

    if (combat_events.size() > 1)
        TraceLogger() << "CombatLog::serialize turn " << turn << "  combat at " << system_id << "  combat events size: " << combat_events.size();
    try {
        ar  & BOOST_SERIALIZATION_NVP(combat_events);
    } catch (const std::exception& e) {
        ErrorLogger() << "combat events serializing failed!: caught exception: " << e.what();
    }

    ar & BOOST_SERIALIZATION_NVP(participant_states);
}

template
void CombatLog::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);
template
void CombatLog::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);
template
void CombatLog::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);
template
void CombatLog::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);



////////////////////////////////////////////////
// CombatLogManager
////////////////////////////////////////////////
CombatLogManager::CombatLogManager() = default;

// Require here because CombatLogManagerImpl is defined in this file.
CombatLogManager::~CombatLogManager() {}

boost::optional<const CombatLog&> CombatLogManager::GetLog(int log_id) const
{
    auto it = m_logs.find(log_id);
    if (it != m_logs.end())
        return it->second;
    return boost::none;
}

int CombatLogManager::AddNewLog(const CombatLog& log)
{
    int new_log_id = ++m_latest_log_id;
    m_logs[new_log_id] = log;
    return new_log_id;
}

void CombatLogManager::CompleteLog(int id, const CombatLog& log)
{

    auto incomplete_it = m_incomplete_logs.find(id);
    if (incomplete_it == m_incomplete_logs.end()) {
        ErrorLogger() << "CombatLogManager::CompleteLog id = " << id << " is not an incomplete log, so log is being discarded.";
        return;
    }
    m_incomplete_logs.erase(incomplete_it);
    m_logs[id] = log;

    if (id > m_latest_log_id) {
        for (++m_latest_log_id; m_latest_log_id <= id; ++m_latest_log_id) {
            m_incomplete_logs.insert(m_latest_log_id);
        }
        ErrorLogger() << "CombatLogManager::CompleteLog id = " << id << " is greater than m_latest_log_id, m_latest_log_id was increased and intervening logs will be requested.";
    }
}

void CombatLogManager::Clear()
{
    m_logs.clear();
    m_incomplete_logs.clear();
    m_latest_log_id = -1;
}

boost::optional<std::vector<int>> CombatLogManager::IncompleteLogIDs() const
{
    if (m_incomplete_logs.empty())
        return boost::none;

    // Set the log ids in reverse order so that if the server only has time to
    // send one log it is the most recent combat log, which is the one most
    // likely of interest to the player.
    std::vector<int> ids;
    for (auto rit = m_incomplete_logs.rbegin(); rit != m_incomplete_logs.rend(); ++rit)
        ids.push_back(*rit);

    return ids;
}

CombatLogManager& CombatLogManager::GetCombatLogManager() {
    static CombatLogManager manager;
    return manager;
}

template <typename Archive>
void CombatLogManager::SerializeIncompleteLogs(Archive& ar, const unsigned int version)
{
    int old_latest_log_id = m_latest_log_id;
    ar & BOOST_SERIALIZATION_NVP(m_latest_log_id);

    // If the new m_latest_log_id is greater than the old one then add all
    // of the new ids to the incomplete log set.
    if (Archive::is_loading::value && m_latest_log_id > old_latest_log_id)
        for (++old_latest_log_id; old_latest_log_id <= m_latest_log_id; ++old_latest_log_id)
            m_incomplete_logs.insert(old_latest_log_id);
}

template <typename Archive>
void CombatLogManager::serialize(Archive& ar, const unsigned int version)
{
    std::map<int, CombatLog> logs;

    if (Archive::is_saving::value) {
        // TODO: filter logs by who should have access to them
        for (auto it = m_logs.begin(); it != m_logs.end(); ++it)
            logs.insert({it->first, it->second});
    }

    ar  & BOOST_SERIALIZATION_NVP(logs)
        & BOOST_SERIALIZATION_NVP(m_latest_log_id);

    if (Archive::is_loading::value) {
        // copy new logs, but don't erase old ones
        for (auto& log : logs)
            m_logs[log.first] = log.second;
    }
}

template
void CombatLogManager::SerializeIncompleteLogs<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void CombatLogManager::SerializeIncompleteLogs<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void CombatLogManager::SerializeIncompleteLogs<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void CombatLogManager::SerializeIncompleteLogs<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);

template
void CombatLogManager::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void CombatLogManager::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void CombatLogManager::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void CombatLogManager::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);

///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
CombatLogManager& GetCombatLogManager()
{ return CombatLogManager::GetCombatLogManager(); }

boost::optional<const CombatLog&> GetCombatLog(int log_id)
{ return GetCombatLogManager().GetLog(log_id); }
