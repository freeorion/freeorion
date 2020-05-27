#include "Serialize.h"

#include "Serialize.ipp"
#include "../combat/CombatEvents.h"
#include "../combat/CombatLogManager.h"


template <typename Archive>
void serialize(Archive& ar, CombatParticipantState& obj, const unsigned int version)
{
    using namespace boost::serialization;

    ar & make_nvp("current_health", obj.current_health)
       & make_nvp("max_health", obj.max_health);
}


template <typename Archive>
void serialize(Archive& ar, CombatLog& obj, const unsigned int version)
{
    using namespace boost::serialization;

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

    ar  & make_nvp("turn", obj.turn)
        & make_nvp("system_id", obj.system_id)
        & make_nvp("empire_ids", obj.empire_ids)
        & make_nvp("object_ids", obj.object_ids)
        & make_nvp("damaged_object_ids", obj.damaged_object_ids)
        & make_nvp("destroyed_object_ids", obj.destroyed_object_ids);

    if (obj.combat_events.size() > 1)
        TraceLogger() << "CombatLog::serialize turn " << obj.turn << "  combat at " << obj.system_id << "  combat events size: " << obj.combat_events.size();
    try {
        ar  & make_nvp("combat_events", obj.combat_events);
    } catch (const std::exception& e) {
        ErrorLogger() << "combat events serializing failed!: caught exception: " << e.what();
    }

    ar & make_nvp("participant_states", obj.participant_states);
}


template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, CombatLog&, const unsigned int);
template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, CombatLog&, const unsigned int);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, CombatLog&, const unsigned int);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, CombatLog&, const unsigned int);

template <typename Archive>
void serialize(Archive& ar, CombatLogManager& obj, const unsigned int version)
{
    using namespace boost::serialization;

    std::map<int, CombatLog> logs;

    if (Archive::is_saving::value) {
        // TODO: filter logs by who should have access to them
        for (auto it = obj.m_logs.begin(); it != obj.m_logs.end(); ++it)
            logs.insert({it->first, it->second});
    }

    ar  & make_nvp("logs", logs)
        & make_nvp("m_latest_log_id", obj.m_latest_log_id);

    if (Archive::is_loading::value) {
        // copy new logs, but don't erase old ones
        for (auto& log : logs)
            obj.m_logs[log.first] = log.second;
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, CombatLogManager&, const unsigned int);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, CombatLogManager&, const unsigned int);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, CombatLogManager&, const unsigned int);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, CombatLogManager&, const unsigned int);


template <typename Archive>
void serializeIncompleteLogs(Archive& ar, CombatLogManager& obj, const unsigned int version)
{
    using namespace boost::serialization;

    int old_latest_log_id = obj.m_latest_log_id;
    ar & make_nvp("m_latest_log_id", obj.m_latest_log_id);

    // If the new m_latest_log_id is greater than the old one then add all
    // of the new ids to the incomplete log set.
    if (Archive::is_loading::value && obj.m_latest_log_id > old_latest_log_id)
        for (++old_latest_log_id; old_latest_log_id <= obj.m_latest_log_id; ++old_latest_log_id)
            obj.m_incomplete_logs.insert(old_latest_log_id);
}

template void serializeIncompleteLogs<freeorion_bin_oarchive>(freeorion_bin_oarchive&, CombatLogManager&, unsigned int const);
template void serializeIncompleteLogs<freeorion_bin_iarchive>(freeorion_bin_iarchive&, CombatLogManager&, unsigned int const);
template void serializeIncompleteLogs<freeorion_xml_oarchive>(freeorion_xml_oarchive&, CombatLogManager&, unsigned int const);
template void serializeIncompleteLogs<freeorion_xml_iarchive>(freeorion_xml_iarchive&, CombatLogManager&, unsigned int const);
