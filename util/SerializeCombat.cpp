#include "Serialize.h"

#include "Serialize.ipp"
#include "../combat/CombatLogManager.h"

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
