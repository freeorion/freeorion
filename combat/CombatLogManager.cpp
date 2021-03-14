#include "CombatLogManager.h"
#include "../universe/Meter.h"
#include "../universe/UniverseObject.h"
#include "../universe/Enums.h"
#include "../util/Logger.h"
#include "CombatEvents.h"


namespace {
    DeclareThreadSafeLogger(combat_log);

    float MaxHealth(const UniverseObject& object) {
        if (object.ObjectType() == UniverseObjectType::OBJ_SHIP) {
            return object.GetMeter(MeterType::METER_MAX_STRUCTURE)->Current();

        } else if ( object.ObjectType() == UniverseObjectType::OBJ_PLANET ) {
            const Meter* defense = object.GetMeter(MeterType::METER_MAX_DEFENSE);
            const Meter* shield = object.GetMeter(MeterType::METER_MAX_SHIELD);
            const Meter* construction = object.GetMeter(MeterType::METER_TARGET_CONSTRUCTION);

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

    float CurrentHealth(const UniverseObject& object) {
        if (object.ObjectType() == UniverseObjectType::OBJ_SHIP) {
            return object.GetMeter(MeterType::METER_STRUCTURE)->Current();

        } else if (object.ObjectType() == UniverseObjectType::OBJ_PLANET) {
            const Meter* defense = object.GetMeter(MeterType::METER_DEFENSE);
            const Meter* shield = object.GetMeter(MeterType::METER_SHIELD);
            const Meter* construction = object.GetMeter(MeterType::METER_CONSTRUCTION);

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

    void FillState(CombatParticipantState& state, const UniverseObject& object) {
        state.current_health = CurrentHealth(object);
        state.max_health = MaxHealth(object);
    };
}

CombatParticipantState::CombatParticipantState(const UniverseObject& object)
{ FillState(*this, object); }

////////////////////////////////////////////////
// CombatLog
////////////////////////////////////////////////
CombatLog::CombatLog(const CombatInfo& combat_info) :
    turn(combat_info.turn),
    system_id(combat_info.system_id),
    empire_ids(combat_info.empire_ids),
    damaged_object_ids(combat_info.damaged_object_ids),
    destroyed_object_ids(combat_info.destroyed_object_ids),
    combat_events(combat_info.combat_events)
{
    // compile all remaining and destroyed objects' ids
    object_ids = combat_info.destroyed_object_ids;
    for (const auto& obj : combat_info.objects->all()) {
        object_ids.insert(obj->ID());
        participant_states[obj->ID()] = CombatParticipantState(*obj);
    }
}


////////////////////////////////////////////////
// CombatLogManager
////////////////////////////////////////////////
boost::optional<const CombatLog&> CombatLogManager::GetLog(int log_id) const {
    auto it = m_logs.find(log_id);
    if (it != m_logs.end())
        return it->second;
    return boost::none;
}

int CombatLogManager::AddNewLog(const CombatLog& log) {
    int new_log_id = ++m_latest_log_id;
    m_logs[new_log_id] = log;
    return new_log_id;
}

void CombatLogManager::CompleteLog(int id, const CombatLog& log) {
    auto incomplete_it = m_incomplete_logs.find(id);
    if (incomplete_it == m_incomplete_logs.end()) {
        DebugLogger(combat_log) << "CombatLogManager::CompleteLog id = " << id << " is not a known incomplete log";
        auto complete_it = m_logs.find(id);
        if (complete_it != m_logs.end())
            DebugLogger(combat_log) << "... rather, it is already a complete log";
    } else {
        m_incomplete_logs.erase(incomplete_it);
    }
    m_logs[id] = log;

    if (id > m_latest_log_id) {
        for (++m_latest_log_id; m_latest_log_id <= id; ++m_latest_log_id)
            m_incomplete_logs.insert(m_latest_log_id);
        ErrorLogger(combat_log) << "CombatLogManager::CompleteLog id = " << id
                                << " is greater than m_latest_log_id, m_latest_log_id was increased and intervening logs will be requested.";
    }
}

void CombatLogManager::Clear() {
    m_logs.clear();
    m_incomplete_logs.clear();
    m_latest_log_id = -1;
}

boost::optional<std::vector<int>> CombatLogManager::IncompleteLogIDs() const {
    if (m_incomplete_logs.empty())
        return boost::none;

    // Set the log ids in reverse order so that if the server only has time to
    // send one log it is the most recent combat log, which is the one most
    // likely of interest to the player.
    return std::vector<int>{m_incomplete_logs.begin(), m_incomplete_logs.end()};
}


///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
CombatLogManager& GetCombatLogManager() {
    static CombatLogManager manager;
    return manager;
}

boost::optional<const CombatLog&> GetCombatLog(int log_id)
{ return GetCombatLogManager().GetLog(log_id); }
