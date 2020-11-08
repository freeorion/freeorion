#include "Serialize.h"

#include "Serialize.ipp"
#include "../combat/CombatEvents.h"
#include "../combat/CombatLogManager.h"


template<typename Archive>
void serialize(Archive&, CombatEvent&, unsigned int const)
{}

BOOST_CLASS_EXPORT(CombatEvent)

template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, CombatEvent&, const unsigned int);
template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, CombatEvent&, const unsigned int);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, CombatEvent&, const unsigned int);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, CombatEvent&, const unsigned int);


template <typename Archive>
void serialize(Archive& ar, BoutBeginEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));
    ar & make_nvp("bout", obj.bout);
}

BOOST_CLASS_EXPORT(BoutBeginEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, BoutBeginEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, BoutBeginEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, BoutBeginEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, BoutBeginEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, BoutEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));
    ar & make_nvp("bout", obj.bout)
       & make_nvp("events", obj.events);
}

BOOST_CLASS_VERSION(BoutEvent, 4)
BOOST_CLASS_EXPORT(BoutEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, BoutEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, BoutEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, BoutEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, BoutEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, SimultaneousEvents& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));
    ar & make_nvp("events", obj.events);
}

BOOST_CLASS_VERSION(SimultaneousEvents, 4)
BOOST_CLASS_EXPORT(SimultaneousEvents)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SimultaneousEvents&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SimultaneousEvents&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SimultaneousEvents&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SimultaneousEvents&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, InitialStealthEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));
    ar & make_nvp("empire_to_object_visibility", obj.empire_to_object_visibility);
}

BOOST_CLASS_VERSION(InitialStealthEvent, 4)
BOOST_CLASS_EXPORT(InitialStealthEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, InitialStealthEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, InitialStealthEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, InitialStealthEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, InitialStealthEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, StealthChangeEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));
    ar & make_nvp("bout", obj.bout)
       & make_nvp("events", obj.events);
}

BOOST_CLASS_VERSION(StealthChangeEvent, 4)
BOOST_CLASS_EXPORT(StealthChangeEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, StealthChangeEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, StealthChangeEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, StealthChangeEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, StealthChangeEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, StealthChangeEvent::StealthChangeEventDetail& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("attacker_id", obj.attacker_id)
        & make_nvp("target_id", obj.target_id)
        & make_nvp("attacker_empire_id", obj.attacker_empire_id)
        & make_nvp("target_empire_id", obj.target_empire_id)
        & make_nvp("visibility", obj.visibility);
}

BOOST_CLASS_VERSION(StealthChangeEvent::StealthChangeEventDetail, 4)
BOOST_CLASS_EXPORT(StealthChangeEvent::StealthChangeEventDetail)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, StealthChangeEvent::StealthChangeEventDetail&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, StealthChangeEvent::StealthChangeEventDetail&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, StealthChangeEvent::StealthChangeEventDetail&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, StealthChangeEvent::StealthChangeEventDetail&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, WeaponFireEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    if (version < 5) {
        ar & make_nvp("bout", obj.bout)
           & make_nvp("round", obj.round)
           & make_nvp("attacker_id", obj.attacker_id)
           & make_nvp("target_id", obj.target_id)
           & make_nvp("weapon_name", obj.weapon_name)
           & make_nvp("power", obj.power)
           & make_nvp("shield", obj.shield)
           & make_nvp("damage", obj.damage)
           & make_nvp("target_owner_id", obj.target_owner_id)
           & make_nvp("attacker_owner_id", obj.attacker_owner_id);
    } else {
        ar & make_nvp("b", obj.bout)
           & make_nvp("r", obj.round)
           & make_nvp("a", obj.attacker_id)
           & make_nvp("t", obj.target_id)
           & make_nvp("w", obj.weapon_name)
           & make_nvp("p", obj.power)
           & make_nvp("s", obj.shield)
           & make_nvp("d", obj.damage)
           & make_nvp("to", obj.target_owner_id)
           & make_nvp("ao", obj.attacker_owner_id);
    }
}

BOOST_CLASS_VERSION(WeaponFireEvent, 5)
BOOST_CLASS_EXPORT(WeaponFireEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, WeaponFireEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, WeaponFireEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, WeaponFireEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, WeaponFireEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, IncapacitationEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    if (version < 2) {
        ar & make_nvp("bout", obj.bout)
           & make_nvp("object_id", obj.object_id)
           & make_nvp("object_owner_id", obj.object_owner_id);
    } else {
        ar & make_nvp("b", obj.bout)
           & make_nvp("i", obj.object_id)
           & make_nvp("o", obj.object_owner_id);
    }
}

BOOST_CLASS_VERSION(IncapacitationEvent, 2)
BOOST_CLASS_EXPORT(IncapacitationEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, IncapacitationEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, IncapacitationEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, IncapacitationEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, IncapacitationEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, FightersAttackFightersEvent& obj, unsigned int const)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    ar & make_nvp("bout", obj.bout)
       & make_nvp("events", obj.events);
}

BOOST_CLASS_VERSION(FightersAttackFightersEvent, 4)
BOOST_CLASS_EXPORT(FightersAttackFightersEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, FightersAttackFightersEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, FightersAttackFightersEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, FightersAttackFightersEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, FightersAttackFightersEvent&, unsigned int const);


template <typename Archive>
void serialize (Archive& ar, FighterLaunchEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    ar & make_nvp("bout", obj.bout)
       & make_nvp("fighter_owner_empire_id", obj.fighter_owner_empire_id)
       & make_nvp("launched_from_id", obj.launched_from_id)
       & make_nvp("number_launched", obj.number_launched);
}

BOOST_CLASS_EXPORT(FighterLaunchEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, FighterLaunchEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, FighterLaunchEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, FighterLaunchEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, FighterLaunchEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, FightersDestroyedEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    ar & make_nvp("bout", obj.bout)
       & make_nvp("events", obj.events);
}

BOOST_CLASS_VERSION(FightersDestroyedEvent, 4)
BOOST_CLASS_EXPORT(FightersDestroyedEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, FightersDestroyedEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, FightersDestroyedEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, FightersDestroyedEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, FightersDestroyedEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, WeaponsPlatformEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    ar & make_nvp("bout", obj.bout)
       & make_nvp("attacker_id", obj.attacker_id)
       & make_nvp("attacker_owner_id", obj.attacker_owner_id)
       & make_nvp("events", obj.events);
}

BOOST_CLASS_VERSION(WeaponsPlatformEvent, 4)
BOOST_CLASS_EXPORT(WeaponsPlatformEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, WeaponsPlatformEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, WeaponsPlatformEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, WeaponsPlatformEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, WeaponsPlatformEvent&, unsigned int const);


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
void SerializeIncompleteLogs(Archive& ar, CombatLogManager& obj, const unsigned int version)
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

template void SerializeIncompleteLogs<freeorion_bin_oarchive>(freeorion_bin_oarchive&, CombatLogManager&, unsigned int const);
template void SerializeIncompleteLogs<freeorion_bin_iarchive>(freeorion_bin_iarchive&, CombatLogManager&, unsigned int const);
template void SerializeIncompleteLogs<freeorion_xml_oarchive>(freeorion_xml_oarchive&, CombatLogManager&, unsigned int const);
template void SerializeIncompleteLogs<freeorion_xml_iarchive>(freeorion_xml_iarchive&, CombatLogManager&, unsigned int const);


template <typename Archive>
void serialize(Archive & ar, CombatInfo& obj, const unsigned int version)
{
    using namespace boost::serialization;

    std::set<int>                       filtered_empire_ids;
    ObjectMap                           filtered_objects;
    std::set<int>                       filtered_damaged_object_ids;
    std::set<int>                       filtered_destroyed_object_ids;
    std::map<int, std::set<int>>        filtered_destroyed_object_knowers;
    Universe::EmpireObjectVisibilityMap filtered_empire_object_visibility;
    std::vector<CombatEventPtr>         filtered_combat_events;

    if (Archive::is_saving::value) {
        obj.GetEmpireIdsToSerialize(             filtered_empire_ids,               GlobalSerializationEncodingForEmpire());
        obj.GetObjectsToSerialize(               filtered_objects,                  GlobalSerializationEncodingForEmpire());
        obj.GetDamagedObjectsToSerialize(        filtered_damaged_object_ids,       GlobalSerializationEncodingForEmpire());
        obj.GetDestroyedObjectsToSerialize(      filtered_destroyed_object_ids,     GlobalSerializationEncodingForEmpire());
        obj.GetDestroyedObjectKnowersToSerialize(filtered_destroyed_object_knowers, GlobalSerializationEncodingForEmpire());
        obj.GetEmpireObjectVisibilityToSerialize(filtered_empire_object_visibility, GlobalSerializationEncodingForEmpire());
        obj.GetCombatEventsToSerialize(          filtered_combat_events,            GlobalSerializationEncodingForEmpire());
    }

    ar  & make_nvp("turn", obj.turn)
        & make_nvp("system_id", obj.system_id)
        & BOOST_SERIALIZATION_NVP(filtered_empire_ids)
        & BOOST_SERIALIZATION_NVP(filtered_objects)
        & BOOST_SERIALIZATION_NVP(filtered_damaged_object_ids)
        & BOOST_SERIALIZATION_NVP(filtered_destroyed_object_ids)
        & BOOST_SERIALIZATION_NVP(filtered_destroyed_object_knowers)
        & BOOST_SERIALIZATION_NVP(filtered_empire_object_visibility)
        & BOOST_SERIALIZATION_NVP(filtered_combat_events);

    if (Archive::is_loading::value) {
        obj.empire_ids.swap(              filtered_empire_ids);
        obj.objects.swap(                 filtered_objects);
        obj.damaged_object_ids.swap(      filtered_damaged_object_ids);
        obj.destroyed_object_ids.swap(    filtered_destroyed_object_ids);
        obj.destroyed_object_knowers.swap(filtered_destroyed_object_knowers);
        obj.empire_object_visibility.swap(filtered_empire_object_visibility);
        obj.combat_events.swap(           filtered_combat_events);
    }
}
