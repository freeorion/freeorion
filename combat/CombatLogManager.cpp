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

        } else if (object.ObjectType() == UniverseObjectType::OBJ_PLANET) {
            float ret = 0.0f;
            if (const Meter* defense = object.GetMeter(MeterType::METER_MAX_DEFENSE))
                ret += defense->Current();
            if (const Meter* shield = object.GetMeter(MeterType::METER_MAX_SHIELD))
                ret += shield->Current();
            if (const Meter* construction = object.GetMeter(MeterType::METER_TARGET_CONSTRUCTION))
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
    damaged_object_ids(combat_info.damaged_object_ids.begin(), combat_info.damaged_object_ids.end()),
    destroyed_object_ids(combat_info.destroyed_object_ids.begin(), combat_info.destroyed_object_ids.end()),
    combat_events(combat_info.combat_events)
{
    // compile all remaining and destroyed objects' ids
    object_ids.insert(combat_info.destroyed_object_ids.begin(), combat_info.destroyed_object_ids.end());
    for (const auto* obj : combat_info.objects.allRaw()) {
        object_ids.insert(obj->ID());
        participant_states[obj->ID()] = CombatParticipantState(*obj);
    }
}


////////////////////////////////////////////////
// CombatLogManager
////////////////////////////////////////////////
CombatLogManager& CombatLogManager::operator=(CombatLogManager&& rhs) noexcept {
    m_latest_log_id.store(rhs.m_latest_log_id.load());
    m_logs = std::move(rhs.m_logs);
    m_incomplete_logs = std::move(rhs.m_incomplete_logs);
    return *this;
}

boost::optional<const CombatLog&> CombatLogManager::GetLog(int log_id) const {
    auto it = m_logs.find(log_id);
    if (it != m_logs.end())
        return it->second;
    return boost::none;
}

int CombatLogManager::AddNewLog(CombatLog log) {
    int new_log_id = ++m_latest_log_id;
    m_logs.emplace(new_log_id, std::move(log));
    return new_log_id;
}

void CombatLogManager::CompleteLog(int id, CombatLog log) {
    auto incomplete_it = m_incomplete_logs.find(id);
    if (incomplete_it == m_incomplete_logs.end()) {
        DebugLogger(combat_log) << "CombatLogManager::CompleteLog id = " << id << " is not a known incomplete log";
        auto complete_it = m_logs.find(id);
        if (complete_it != m_logs.end())
            DebugLogger(combat_log) << "... rather, it is already a complete log";
    } else {
        m_incomplete_logs.erase(incomplete_it);
    }
    m_logs.insert_or_assign(id, std::move(log));

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



///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
CombatLogManager& GetCombatLogManager() {
    static CombatLogManager manager;
    return manager;
}

boost::optional<const CombatLog&> GetCombatLog(int log_id)
{ return GetCombatLogManager().GetLog(log_id); }
