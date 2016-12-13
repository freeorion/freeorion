#include "CombatLogManager.h"
#include "../universe/Meter.h"
#include "../universe/UniverseObject.h"
#include "../util/Serialize.h"
#include "../util/Serialize.ipp"
#include "CombatEvents.h"


namespace {
    static float MaxHealth(const UniverseObject& object) {
        if ( object.ObjectType() == OBJ_SHIP ) {
            return object.CurrentMeterValue(METER_MAX_STRUCTURE);
        } else if ( object.ObjectType() == OBJ_PLANET ) {
            const Meter* defense = object.GetMeter(METER_MAX_DEFENSE);
            const Meter* shield = object.GetMeter(METER_MAX_SHIELD);
            const Meter* construction = object.UniverseObject::GetMeter(METER_TARGET_CONSTRUCTION);

            float ret = 0.0;
            if(defense) {
                ret += defense->Current();
            }
            if(shield) {
                ret += shield->Current();
            }
            if(construction) {
                ret += construction->Current();
            }
            return ret;
        } else {
            return 0.0;
        }
    }

    static float CurrentHealth(const UniverseObject& object) {
        if ( object.ObjectType() == OBJ_SHIP ) {
            return object.CurrentMeterValue(METER_STRUCTURE);
        } else if ( object.ObjectType() == OBJ_PLANET ) {
            const Meter* defense = object.GetMeter(METER_DEFENSE);
            const Meter* shield = object.GetMeter(METER_SHIELD);
            const Meter* construction = object.UniverseObject::GetMeter(METER_CONSTRUCTION);

            float ret = 0.0;
            if(defense) {
                ret += defense->Current();
            }
            if(shield) {
                ret += shield->Current();
            }
            if(construction) {
                ret += construction->Current();
            }
            return ret;
        } else {
            return 0.0;
        }
    }

    static void FillState(CombatParticipantState& state, const UniverseObject& object) {
        state.current_health = CurrentHealth(object);
        state.max_health = MaxHealth(object);
    };
}

CombatParticipantState::CombatParticipantState():
current_health(0.0f),
max_health(0.0f)
{

}


CombatParticipantState::CombatParticipantState(const UniverseObject& object):
current_health(0.0f),
max_health(0.0f)
{
    FillState(*this, object);
}

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
    {
        object_ids.insert(it->ID());
        participant_states[it->ID()] = CombatParticipantState(**it);
    }
}

template <class Archive>
void CombatParticipantState::serialize(Archive& ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(current_health)
       & BOOST_SERIALIZATION_NVP(max_health);
}

template <class Archive>
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
    & BOOST_SERIALIZATION_NVP(destroyed_object_ids)
    & BOOST_SERIALIZATION_NVP(combat_events);

    // Store state of fleet at this battle.
    // Used to show summaries of past battles.
    if(version >= 1) {
        ar & BOOST_SERIALIZATION_NVP(participant_states);
    }
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
// CombatLogManagerImpl
////////////////////////////////////////////////

class CombatLogManager::CombatLogManagerImpl
{
    public:
    CombatLogManagerImpl();

      /** \name Accessors */ //@{
    /** Return the requested combat log or boost::none.*/
    boost::optional<const CombatLog&>  GetLog(int log_id) const;

    /** Return true if there are partial logs.*/
    bool HasIncompleteLogs() const;

    /** Return the ids of all incomplete logs.*/
    std::vector<int> IncompleteLogIDs() const;
    //@}

    /** \name Mutators */ //@{
    int     AddLog(const CombatLog& log);   // adds log, returns unique log id
    void    Clear();
    //@}

    void GetLogsToSerialize(std::map<int, CombatLog>& logs, int encoding_empire) const;
    void SetLog(int log_id, const CombatLog& log);

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);

    private:
    std::map<int, CombatLog>    m_logs;
    int                         m_latest_log_id;
};

CombatLogManager::CombatLogManagerImpl::CombatLogManagerImpl() :
    m_logs(),
    m_latest_log_id(-1)
{}

boost::optional<const CombatLog&> CombatLogManager::CombatLogManagerImpl::GetLog(int log_id) const {
    std::map<int, CombatLog>::const_iterator it = m_logs.find(log_id);
    if (it != m_logs.end())
        return it->second;
    return boost::none;
}

int CombatLogManager::CombatLogManagerImpl::AddLog(const CombatLog& log) {
    int new_log_id = ++m_latest_log_id;
    m_logs[new_log_id] = log;
    return new_log_id;
}

void CombatLogManager::CombatLogManagerImpl::Clear()
{ m_logs.clear(); }

void CombatLogManager::CombatLogManagerImpl::GetLogsToSerialize(std::map<int, CombatLog>& logs, int encoding_empire) const {
    if (&logs == &m_logs)
        return;
    // TODO: filter logs by who should have access to them
    logs = m_logs;
}

void CombatLogManager::CombatLogManagerImpl::SetLog(int log_id, const CombatLog& log)
{ m_logs[log_id] = log; }


bool CombatLogManager::CombatLogManagerImpl::HasIncompleteLogs() const
{
    return true;
}

std::vector<int> CombatLogManager::CombatLogManagerImpl::IncompleteLogIDs() const
{
    return std::vector<int>(1,19);
}

template <class Archive>
void CombatLogManager::CombatLogManagerImpl::serialize(Archive& ar, const unsigned int version)
{
    std::map<int, CombatLog> logs;

    if (Archive::is_saving::value) {
        GetLogsToSerialize(logs, GetUniverse().EncodingEmpire());
    }

    ar  & BOOST_SERIALIZATION_NVP(logs)
        & BOOST_SERIALIZATION_NVP(m_latest_log_id);

    if (Archive::is_loading::value) {
        // copy new logs, but don't erase old ones
        for (auto& log : logs)
           SetLog(log.first, log.second);
    }
}


////////////////////////////////////////////////
// CombatLogManager
////////////////////////////////////////////////
CombatLogManager::CombatLogManager() :
    pimpl(new CombatLogManagerImpl)
{}

// Require here because CombatLogManagerImpl is defined in this file.
CombatLogManager::~CombatLogManager() {}

boost::optional<const CombatLog&> CombatLogManager::GetLog(int log_id) const
{ return pimpl->GetLog(log_id); }

int CombatLogManager::AddLog(const CombatLog& log)
{ return pimpl->AddLog(log); }

void CombatLogManager::Clear()
{ return pimpl->Clear(); }

bool CombatLogManager::HasIncompleteLogs() const
{ return pimpl->HasIncompleteLogs(); }

std::vector<int> CombatLogManager::IncompleteLogIDs() const
{ return pimpl->IncompleteLogIDs(); }

CombatLogManager& CombatLogManager::GetCombatLogManager() {
    static CombatLogManager manager;
    return manager;
}

template <class Archive>
void CombatLogManager::serialize(Archive& ar, const unsigned int version)
{ pimpl->serialize(ar, version); }

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
