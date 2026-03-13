#include "Serialize.h"

#include "AppInterface.h"
#include "SitRepEntry.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Supply.h"
#include "../Empire/Diplomacy.h"

#include "GameRules.h"

#include "Serialize.ipp"
#include <boost/serialization/array.hpp>
#include <boost/serialization/version.hpp>


template <typename Archive>
void Empire::PolicyAdoptionInfo::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(adoption_turn)
        & BOOST_SERIALIZATION_NVP(category)
        & BOOST_SERIALIZATION_NVP(slot_in_category);
}

template void Empire::PolicyAdoptionInfo::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Empire::PolicyAdoptionInfo::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Empire::PolicyAdoptionInfo::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Empire::PolicyAdoptionInfo::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

namespace {
#if defined(__cpp_lib_to_chars) && defined(__cpp_lib_constexpr_charconv)
    constexpr
#endif
    std::size_t ToChars(uint16_t num, char* buffer, char* buffer_end) {
#if defined(__cpp_lib_to_chars)
        const auto result_ptr = std::to_chars(buffer, buffer_end, num).ptr;
        return static_cast<std::size_t>(std::distance(buffer, result_ptr));
#else
        std::size_t buffer_sz = std::distance(buffer, buffer_end);
        auto temp = std::to_string(num);
        auto out_sz = std::min(buffer_sz, temp.size());
        std::copy_n(temp.begin(), out_sz, buffer);
        return out_sz;
#endif
    }

    consteval std::size_t Pow(std::size_t base, std::size_t exp) noexcept {
        std::size_t retval = 1;
        while (exp--)
            retval *= base;
        return retval;
    }

    constexpr uint16_t uint16_t_max = std::numeric_limits<uint16_t>::max();
    constexpr uint8_t uint16_t_digits = 5; // digits in base 10 of 65536 = 2^16
    static_assert(Pow(10, uint16_t_digits + 1) > uint16_t_max);

    constexpr uint8_t data_arr_sz = 8;
    using DataArrT = std::array<std::pair<uint16_t, uint16_t>, data_arr_sz>;

    constexpr std::size_t buffer_size = data_arr_sz*2*(uint16_t_digits + 1); // space for "65535 12345 " repeated data_arr_sz times

    // returns { next unconsumed char*, true/false did the parse succeed }
    // parsed value returned in \a val_out
    template <typename ValT>
    inline auto FromChars(const char* start, const char* end, ValT& val_out) -> std::pair<const char*, bool>
    {
        static_assert(std::is_unsigned_v<ValT> && sizeof(ValT) <= sizeof(unsigned int));
        unsigned int scratch = 0;
        static constexpr unsigned int val_t_max{std::numeric_limits<ValT>::max()};

#if defined(__cpp_lib_to_chars)
        const auto result = std::from_chars(start, end, scratch);
        val_out = static_cast<ValT>(std::min(val_t_max, scratch));
        return {result.ptr, result.ec == std::errc()};

#else
        int chars_consumed = 0;
        using val_out_t = std::decay_t<decltype(val_out)>;
        constexpr auto format_str = "%u%n";
        const auto matched = sscanf(start, format_str, &scratch, &chars_consumed);
        val_out = static_cast<ValT>(std::min(val_t_max, scratch));
        return {start + chars_consumed, matched >= 1};
#endif
    }


    template <typename Archive, std::size_t N>
    void Serialize(Archive& ar, uint8_t& count, std::array<std::pair<uint16_t, uint16_t>, N>& data)
    {
        static_assert(N < std::numeric_limits<std::decay_t<decltype(count)>>::max());

        if constexpr (Archive::is_loading::value) {
            ar >> boost::serialization::make_nvp("m_strings_count", count);

            std::vector<std::pair<uint16_t, uint16_t>> data_vec;
            ar >> boost::serialization::make_nvp("m_string_offsets_sizes", data_vec);
            const std::size_t clamped_count = std::min(data_vec.size(), std::min(static_cast<std::size_t>(count), N));
            std::copy_n(data_vec.begin(), clamped_count, data.begin());

        } else {
            uint8_t clamped_count = static_cast<uint8_t>(std::min(static_cast<std::size_t>(count), N));
            ar << boost::serialization::make_nvp("m_strings_count", clamped_count);
            std::vector<std::pair<uint16_t, uint16_t>> data_vec(data.begin(), data.begin() + clamped_count);
            ar << boost::serialization::make_nvp("m_string_offsets_sizes", data_vec);
        }
    }

    template <std::size_t N> requires (N <= data_arr_sz)
    void Serialize(boost::archive::xml_oarchive& ar, uint8_t& count, std::array<std::pair<uint16_t, uint16_t>, N>& data)
    {
        std::array<std::string::value_type, buffer_size> buffer{};
        auto* buffer_next = buffer.data();
        auto* buffer_end = buffer.data() + buffer.size();

        // store number of value pairs
        buffer_next += ToChars(static_cast<int16_t>(count), buffer_next, buffer_end);

        uint8_t tally = 0;
        for (const auto& [offset, sz] : data) {
            if (tally >= count)
                break;
            ++tally;
            *buffer_next++ = ' ';
            buffer_next += ToChars(offset, buffer_next, buffer_end);
            *buffer_next++ = ' ';
            buffer_next += ToChars(sz, buffer_next, buffer_end);
        }

        const auto buffer_next_dist = static_cast<std::size_t>(std::distance(buffer.data(), buffer_next));
        const std::size_t string_size = std::min(buffer_size, buffer_next_dist);
        std::string s(buffer.data(), string_size);
        ar << boost::serialization::make_nvp("ct_ofst_szs", s);
    }

    template <std::size_t N>
    void Serialize(boost::archive::xml_iarchive& ar, uint8_t& count, std::array<std::pair<uint16_t, uint16_t>, N>& data)
    {
        std::string buffer;
        buffer.reserve(buffer_size);
        ar >> boost::serialization::make_nvp("ct_ofst_szs", buffer); // ct_ofst_szs = abbreviation for "count & (offsets & sizes)

        unsigned int count_scratch = 0U;
        const char* const buffer_end = buffer.c_str() + buffer.size();

        auto [next, success] = FromChars(buffer.c_str(), buffer_end, count_scratch);
        if (!success) {
            count = 0;
            return;
        }
        count = static_cast<uint8_t>(std::min<unsigned int>(count_scratch, N));

        for (uint8_t idx = 0; idx < static_cast<uint8_t>(count) && next != buffer_end; ++idx) {
            while (std::distance(next, buffer_end) > 0 && *next == ' ')
                ++next;

            if (std::distance(next, buffer_end) < 3) // 3 is enough for "1 1" or similar
                return;

            unsigned int offset = 0;
            static_assert(sizeof(decltype(offset)) >= sizeof(uint16_t));
            std::tie(next, success) = FromChars(next, buffer_end, offset);
            if (!success || offset >= std::numeric_limits<uint16_t>::max())
                return;
            data[idx].first = offset;

            while (std::distance(next, buffer_end) > 0 && *next == ' ')
                ++next;


            if (std::distance(next, buffer_end) < 1) // 1 is enough for "1" or similar
                return;

            unsigned int size = 0;
            std::tie(next, success) = FromChars(next, buffer_end, size);
            if (!success || offset >= std::numeric_limits<uint16_t>::max())
                return;
            data[idx].second = size;
        }
    }
}

template <typename Archive>
void Empire::ChunkedStringAndViews::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_str);
    Serialize(ar, m_strings_count, m_string_offsets_sizes);
}

template void Empire::ChunkedStringAndViews::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Empire::ChunkedStringAndViews::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Empire::ChunkedStringAndViews::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Empire::ChunkedStringAndViews::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);


template <typename Archive>
void Empire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_player_name);
    ar  & BOOST_SERIALIZATION_NVP(m_color);
    ar  & BOOST_SERIALIZATION_NVP(m_capital_id)
        & BOOST_SERIALIZATION_NVP(m_source_id)
        & BOOST_SERIALIZATION_NVP(m_eliminated);


    if (Archive::is_loading::value && version < 13) {
        std::set<std::string> victories;
        ar  & boost::serialization::make_nvp("m_victories", victories);
        m_victories.clear();
        m_victories.insert(boost::container::ordered_unique_range, victories.begin(), victories.end());
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_victories);
    }


    const auto encoding_empire = GlobalSerializationEncodingForEmpire();
    TraceLogger() << "serializing empire " << m_id << ": " << m_name
                  << " for encoding empire: " << encoding_empire;

    if (Archive::is_loading::value && version < 11) {
        std::map<std::string, int> techs;
        ar  & boost::serialization::make_nvp("m_techs", techs);
        m_techs.insert(boost::container::ordered_unique_range, techs.begin(), techs.end());

    } else if (Archive::is_loading::value && version < 13) {
        std::map<std::string, int, std::less<>> techs;
        ar  & boost::serialization::make_nvp("m_techs", techs);
        m_techs.insert(boost::container::ordered_unique_range, techs.begin(), techs.end());

    } else if constexpr (Archive::is_loading::value) {
        ar  & boost::serialization::make_nvp("m_techs", m_techs);

    } else {
        const auto& techs_to_serialize = GetTechsToSerialize(encoding_empire);
        ar  & boost::serialization::make_nvp("m_techs", techs_to_serialize);
    }

    if (Archive::is_loading::value && version < 10) {
        std::map<std::string, PolicyAdoptionInfo> adopted_policies;
        std::map<std::string, PolicyAdoptionInfo> initial_adopted_policies;
        std::set<std::string>                     available_policies;

        ar  & boost::serialization::make_nvp("m_adopted_policies", adopted_policies);
        m_adopted_policies.clear();
        m_adopted_policies.insert(adopted_policies.begin(), adopted_policies.end());

        ar  & boost::serialization::make_nvp("m_initial_adopted_policies", initial_adopted_policies);
        m_initial_adopted_policies.clear();
        m_initial_adopted_policies.insert(initial_adopted_policies.begin(), initial_adopted_policies.end());

        ar  & boost::serialization::make_nvp("m_available_policies", available_policies);
        m_available_policies.clear();
        m_available_policies.insert(available_policies.begin(), available_policies.end());

    } else if constexpr (Archive::is_loading::value) {
        ar  & BOOST_SERIALIZATION_NVP(m_adopted_policies)
            & BOOST_SERIALIZATION_NVP(m_initial_adopted_policies)
            & BOOST_SERIALIZATION_NVP(m_available_policies);
    } else {
        const auto& adopted_to_serialize = GetAdoptedPoliciesToSerialize(encoding_empire);
        const auto& initial_to_serialize = GetInitialPoliciesToSerialize(encoding_empire);
        const auto& available_to_serialize = GetAvailablePoliciesToSerialize(encoding_empire);
        ar  & boost::serialization::make_nvp("m_adopted_policies", adopted_to_serialize)
            & boost::serialization::make_nvp("m_initial_adopted_policies", initial_to_serialize)
            & boost::serialization::make_nvp("m_available_policies", available_to_serialize);
    }

    if constexpr (Archive::is_loading::value) {
        ar  & BOOST_SERIALIZATION_NVP(m_policy_adoption_total_duration);
    } else {
        const auto& adopted_durations_to_serialize = GetAdoptionTotalDurationsToSerialize(encoding_empire);
        ar  & boost::serialization::make_nvp("m_policy_adoption_total_duration", adopted_durations_to_serialize);
    }

    if (Archive::is_loading::value && version < 7) {
        const auto* app = IApp::GetApp();
        const auto current_turn = app ? app->CurrentTurn() : INVALID_GAME_TURN;

        m_policy_adoption_current_duration.clear();
        for (const auto& [policy_name, adoption_info] : m_adopted_policies) {
            m_policy_adoption_current_duration.emplace(
                policy_name, app ? (current_turn - adoption_info.adoption_turn) : 0);
        }

    } else if constexpr (Archive::is_loading::value) {
        ar  & BOOST_SERIALIZATION_NVP(m_policy_adoption_current_duration);

    } else {
        const auto& current_durations_to_serialize = GetAdoptionCurrentDurationsToSerialize(encoding_empire);
        ar  & boost::serialization::make_nvp("m_policy_adoption_current_duration", current_durations_to_serialize);
    }

    if (Archive::is_loading::value && version < 14) {
        // initialize to current turn for all currently adopted policies.
        // others ignored, since there is no previous record of if or when they were adopted
        const auto* app = IApp::GetApp();
        const auto current_turn = app ? app->CurrentTurn() : 1;

        m_policy_latest_turn_adopted.clear();
        for (const auto& policy_name : m_adopted_policies | range_keys)
            m_policy_latest_turn_adopted.emplace(policy_name, current_turn);

    } else if constexpr (Archive::is_loading::value) {
        ar  & BOOST_SERIALIZATION_NVP(m_policy_latest_turn_adopted);

    } else {
        const auto& adopted_last_turns_to_serialize = GetAdoptionLatestTurnsToSerialize(encoding_empire);
        ar  & boost::serialization::make_nvp("m_policy_latest_turn_adopted", adopted_last_turns_to_serialize);
    }


    if (Archive::is_loading::value && version < 11) {
        std::map<std::string, Meter> meters;
        ar  & boost::serialization::make_nvp("m_meters", meters);
        m_meters.insert(boost::container::ordered_unique_range, meters.begin(), meters.end());

    } else if (Archive::is_loading::value && version < 13) {
        std::vector<std::pair<std::string, Meter>> meters;
        ar  & boost::serialization::make_nvp("m_meters", meters);
        std::sort(meters.begin(), meters.end());
        m_meters.insert(std::make_move_iterator(meters.begin()), std::make_move_iterator(meters.end()));

    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_meters);
    }

    if constexpr (Archive::is_loading::value) {
        // loading
        ar  & BOOST_SERIALIZATION_NVP(m_research_queue)
            & BOOST_SERIALIZATION_NVP(m_research_progress)
            & BOOST_SERIALIZATION_NVP(m_production_queue)
            & BOOST_SERIALIZATION_NVP(m_influence_queue);

        if (version < 13) {
            std::set<std::string> buf;
            ar  & boost::serialization::make_nvp("m_available_building_types", buf);
            m_available_building_types.insert(boost::container::ordered_unique_range, buf.begin(), buf.end());
            buf.clear();
            ar  & boost::serialization::make_nvp("m_available_part_types", buf);
            m_available_ship_parts.insert(boost::container::ordered_unique_range, buf.begin(), buf.end());
            buf.clear();
            ar  & boost::serialization::make_nvp("m_available_hull_types", buf);
            m_available_ship_hulls.insert(boost::container::ordered_unique_range, buf.begin(), buf.end());

        } else {
            ar  & boost::serialization::make_nvp("m_available_building_types", m_available_building_types)
                & boost::serialization::make_nvp("m_available_part_types", m_available_ship_parts)
                & boost::serialization::make_nvp("m_available_hull_types", m_available_ship_hulls);
        }

    } else {
        const auto& research_queue = GetResearchQueueToSerialize(encoding_empire);
        const auto& research_progress = GetResearchProgressToSerialize(encoding_empire);
        const auto& prod_queue = GetProductionQueueToSerialize(encoding_empire);
        const auto& influence_queue = GetInfluenceQueueToSerialize(encoding_empire);
        const auto& buildings = GetAvailableBuildingsToSerialize(encoding_empire);
        const auto& parts = GetAvailablePartsToSerialize(encoding_empire);
        const auto& hulls = GetAvailableHullsToSerialize(encoding_empire);

        ar  & boost::serialization::make_nvp("m_research_queue", research_queue)
            & boost::serialization::make_nvp("m_research_progress", research_progress)
            & boost::serialization::make_nvp("m_production_queue", prod_queue)
            & boost::serialization::make_nvp("m_influence_queue", influence_queue)
            & boost::serialization::make_nvp("m_available_building_types", buildings)
            & boost::serialization::make_nvp("m_available_part_types", parts)
            & boost::serialization::make_nvp("m_available_hull_types", hulls);
    }

    ar  & BOOST_SERIALIZATION_NVP(m_supply_system_ranges);
    ar  & BOOST_SERIALIZATION_NVP(m_supply_unobstructed_systems);
    ar  & BOOST_SERIALIZATION_NVP(m_preserved_system_exit_lanes);


    const bool visible = (ALL_EMPIRES == encoding_empire) || (m_id == encoding_empire);

    if (visible) {
        try {
            ar  & boost::serialization::make_nvp("m_ship_designs", m_known_ship_designs);

            if (Archive::is_loading::value && version < 15) {
                DebugLogger() << "fallback sitrep load";
                std::vector<SitRepEntry> sitreps;
                ar  & boost::serialization::make_nvp("m_sitrep_entries", sitreps);
                MoveSitrepsToBlob(std::move(sitreps));
            } else {
                ar  & boost::serialization::make_nvp("m_sitrep_blob", m_blobbed_sitreps)
                    & BOOST_SERIALIZATION_NVP(m_blobbed_sitrep_fixed_infos);
            }

            if (Archive::is_loading::value && version < 12) {
                std::map<ResourceType, std::shared_ptr<ResourcePool>> scratch;
                ar  & boost::serialization::make_nvp("m_resource_pools", scratch);
                auto it = scratch.find(ResourceType::RE_INDUSTRY);
                if (it != scratch.end())
                    m_industry_pool = std::move(*it->second);
                it = scratch.find(ResourceType::RE_RESEARCH);
                if (it != scratch.end())
                    m_research_pool = std::move(*it->second);
                it = scratch.find(ResourceType::RE_INFLUENCE);
                if (it != scratch.end())
                    m_influence_pool = std::move(*it->second);

            } else {
                ar  & BOOST_SERIALIZATION_NVP(m_industry_pool)
                    & BOOST_SERIALIZATION_NVP(m_research_pool)
                    & BOOST_SERIALIZATION_NVP(m_influence_pool);
            }
            ar  & BOOST_SERIALIZATION_NVP(m_population_pool);

            if (Archive::is_loading::value && version < 8) {
                std::set<int> explored_system_ids;
                ar  & boost::serialization::make_nvp("m_explored_systems", explored_system_ids);
                m_explored_systems.clear();
                for (auto id : explored_system_ids)
                    m_explored_systems.emplace(id, 0);
            } else {
                ar  & BOOST_SERIALIZATION_NVP(m_explored_systems);
            }

            ar  & BOOST_SERIALIZATION_NVP(m_ship_names_used)
                & BOOST_SERIALIZATION_NVP(m_species_ships_owned)
                & BOOST_SERIALIZATION_NVP(m_ship_designs_owned)
                & boost::serialization::make_nvp("m_ship_part_types_owned", m_ship_parts_owned)
                & BOOST_SERIALIZATION_NVP(m_ship_part_class_owned)
                & BOOST_SERIALIZATION_NVP(m_species_colonies_owned)
                & BOOST_SERIALIZATION_NVP(m_outposts_owned)
                & BOOST_SERIALIZATION_NVP(m_building_types_owned);

            if (Archive::is_loading::value && version < 9) {
                m_ships_destroyed.clear();
            } else {
                ar  & BOOST_SERIALIZATION_NVP(m_ships_destroyed);
            }

            ar  & BOOST_SERIALIZATION_NVP(m_empire_ships_destroyed)
                & BOOST_SERIALIZATION_NVP(m_ship_designs_destroyed)
                & BOOST_SERIALIZATION_NVP(m_species_ships_destroyed)
                & BOOST_SERIALIZATION_NVP(m_species_planets_invaded)

                & BOOST_SERIALIZATION_NVP(m_ship_designs_in_production)
                & BOOST_SERIALIZATION_NVP(m_species_ships_produced)
                & BOOST_SERIALIZATION_NVP(m_ship_designs_produced)
                & BOOST_SERIALIZATION_NVP(m_species_ships_lost)
                & BOOST_SERIALIZATION_NVP(m_ship_designs_lost)
                & BOOST_SERIALIZATION_NVP(m_species_ships_scrapped)
                & BOOST_SERIALIZATION_NVP(m_ship_designs_scrapped)

                & BOOST_SERIALIZATION_NVP(m_species_planets_depoped)
                & BOOST_SERIALIZATION_NVP(m_species_planets_bombed)

                & BOOST_SERIALIZATION_NVP(m_building_types_produced)
                & BOOST_SERIALIZATION_NVP(m_building_types_scrapped);
        } catch (const std::exception& e) {
            ErrorLogger() << "Empire::serialize failed to (de)serialize stuff for visible empire: " << e.what();
        }
    }


    ar  & BOOST_SERIALIZATION_NVP(m_authenticated);
    ar  & BOOST_SERIALIZATION_NVP(m_ready);
    ar  & BOOST_SERIALIZATION_NVP(m_auto_turn_count);

    if (Archive::is_loading::value && version < 13) {
        m_last_turn_received = INVALID_GAME_TURN;
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_last_turn_received);
    }
}

BOOST_CLASS_VERSION(Empire, 15)

template void Empire::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Empire::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Empire::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Empire::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);


namespace {
    constexpr auto DiploKey(int id1, int ind2) noexcept
    { return std::pair(std::max(id1, ind2), std::min(id1, ind2)); }
}

template <typename Archive>
void serialize(Archive& ar, EmpireManager& em, unsigned int const version)
{
    using boost::serialization::make_nvp;

    TraceLogger() << "Serializing EmpireManager encoding empire: " << GlobalSerializationEncodingForEmpire();

    if constexpr (Archive::is_loading::value)
        em.Clear();    // clean up any existing dynamically allocated contents before replacing containers with deserialized data

    std::map<std::pair<int, int>, DiplomaticMessage> messages;
    if constexpr (Archive::is_saving::value)
        em.GetDiplomaticMessagesToSerialize(messages, GlobalSerializationEncodingForEmpire());

    //TraceLogger() << "EmpireManager version : " << version;
    if (Archive::is_loading::value && version < 1) {
        std::map<int, Empire*> empire_raw_ptr_map;
        ar  & make_nvp("m_empire_map", empire_raw_ptr_map);
        for (const auto& [empire_id, empire_ptr] : empire_raw_ptr_map)
            em.m_empire_map[empire_id] = std::shared_ptr<Empire>(empire_ptr);

    } else if (Archive::is_loading::value && version < 2) {
        ar  & make_nvp("m_empire_map", em.m_empire_map);
        TraceLogger() << "EmpireManager serialized " << em.m_empire_map.size() << " empires";

        std::map<std::pair<int, int>, DiplomaticStatus> diplo_statuses;
        ar  & make_nvp("m_empire_diplomatic_statuses", diplo_statuses);
        em.m_empire_diplomatic_statuses.clear();
        for (const auto& ids_status : diplo_statuses)
            em.m_empire_diplomatic_statuses.emplace(ids_status);

    } else if (Archive::is_loading::value && version < 3) {
        std::map<std::pair<int, int>, DiplomaticStatus> diplo_statuses;
        ar  & make_nvp("m_empire_diplomatic_statuses", diplo_statuses);
        em.m_empire_diplomatic_statuses.clear();
        for (const auto& ids_status : diplo_statuses)
            em.m_empire_diplomatic_statuses.emplace(ids_status);

        ar  & make_nvp("m_empire_map", em.m_empire_map);
        TraceLogger() << "EmpireManager serialized " << em.m_empire_map.size() << " empires";

    } else {
        ar  & make_nvp("m_empire_diplomatic_statuses", em.m_empire_diplomatic_statuses);
        ar  & make_nvp("m_empire_map", em.m_empire_map);
        TraceLogger() << "EmpireManager serialized " << em.m_empire_map.size() << " empires";
    }
    ar  & BOOST_SERIALIZATION_NVP(messages);

    if constexpr (Archive::is_loading::value) {
        for (const auto& [empire_id, empire_ptr] : em.m_empire_map) {
            em.m_const_empire_map.emplace(empire_id, empire_ptr);
            em.m_empire_ids.insert(empire_id);
        }

        em.RefreshCapitalIDs();

        em.m_diplomatic_messages = std::move(messages);

        // erase invalid empire diplomatic statuses
        std::vector<std::pair<int, int>> to_erase;
        to_erase.reserve(em.m_empire_diplomatic_statuses.size());
        for (const auto &[e1, e2] : em.m_empire_diplomatic_statuses | range_keys) {
            if (!em.m_empire_map.contains(e1) || !em.m_empire_map.contains(e2)) {
                to_erase.emplace_back(e1, e2);
                ErrorLogger() << "Erased invalid diplomatic status between empires " << e1 << " and " << e2;
            }
        }
        for (const auto& p : to_erase)
            em.m_empire_diplomatic_statuses.erase(p);

        // add missing empire diplomatic statuses
        for (const auto e1_id : em.m_empire_map | range_keys) {
            const auto e1_id_lower = [e1_id](auto e2_id) { return e1_id < e2_id; };
            for (const auto e2_id : em.m_empire_map | range_keys | range_filter(e1_id_lower)) {
                auto dk = DiploKey(e1_id, e2_id);
                const auto inserted_missing_status = em.m_empire_diplomatic_statuses.try_emplace(
                    dk, DiplomaticStatus::DIPLO_WAR).second;
                if (inserted_missing_status)
                    ErrorLogger() << "Added missing diplomatic status (default WAR) between empires "
                                  << e1_id << " and " << e2_id;
            }
        }
    }

    DebugLogger() << "EmpireManager takes at least: " << em.SizeInMemory()/1024 << " kB";
    DebugLogger() << "SitReps:";
    for (const auto& [eid, e] : em.m_empire_map) {
        const auto template_mems = e->SitRepsSizeInMemory();
        const auto sum = std::transform_reduce(template_mems.begin(), template_mems.end(), std::size_t{0}, std::plus<>{},
                                               [](const auto& p) noexcept { return p.second; });
        DebugLogger() << "  empire " << eid << "  " << sum/1024 << " kB:";

        for (const auto& [templ, mems] : template_mems) {
            if (mems > 20000)
                DebugLogger() << "    " << templ << ": " << mems/1024 << " kB"; 
        }
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, EmpireManager&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, EmpireManager&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, EmpireManager&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, EmpireManager&, unsigned int const);


template <typename Archive>
void DiplomaticMessage::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_sender_empire)
        & BOOST_SERIALIZATION_NVP(m_recipient_empire)
        & BOOST_SERIALIZATION_NVP(m_type);
}

template void DiplomaticMessage::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void DiplomaticMessage::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void DiplomaticMessage::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void DiplomaticMessage::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <typename Archive>
void SupplyManager::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_supply_starlane_traversals)
        & BOOST_SERIALIZATION_NVP(m_supply_starlane_obstructed_traversals)
        & BOOST_SERIALIZATION_NVP(m_fleet_supplyable_system_ids)
        & BOOST_SERIALIZATION_NVP(m_resource_supply_groups)
        & BOOST_SERIALIZATION_NVP(m_propagated_supply_ranges)
        & BOOST_SERIALIZATION_NVP(m_empire_propagated_supply_ranges)
        & BOOST_SERIALIZATION_NVP(m_propagated_supply_distances)
        & BOOST_SERIALIZATION_NVP(m_empire_propagated_supply_distances);
}

template void SupplyManager::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void SupplyManager::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void SupplyManager::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void SupplyManager::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);
