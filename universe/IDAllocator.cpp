#include "IDAllocator.h"

#include "../util/Logger.h"
#include "../util/LoggerWithOptionsDB.h"
#include "../util/Serialize.h"
#include "../util/Serialize.ipp"


#include <limits>

namespace {
    DeclareThreadSafeLogger(IDallocator);
}

IDAllocator::IDAllocator(const int server_id,
                         const std::vector<int>& client_ids,
                         const ID_t invalid_id,
                         const ID_t temp_id,
                         const ID_t highest_pre_allocated_id) :
    m_invalid_id(invalid_id),
    m_temp_id(temp_id),
    m_stride(client_ids.size() + 1),
    m_zero(std::max({m_invalid_id + 1, m_temp_id + 1, highest_pre_allocated_id + 1})),
    m_server_id(server_id),
    m_empire_id(server_id),
    m_offset_to_empire_id(client_ids.size() + 1, server_id),
    m_warn_threshold(std::numeric_limits<int>::max() - 1000 * m_stride),
    m_exhausted_threshold(std::numeric_limits<int>::max() - 10 * m_stride)
{
    TraceLogger(IDallocator) << "IDAllocator() server id = " << server_id << " invalid id = " << invalid_id
                             << " zero = " << m_zero
                             << " warn threshold =  " << m_warn_threshold << " num clients = " << client_ids.size();

    auto ii = m_zero;

    // Assign the server and each client a unique initial offset modulo m_stride.

    // Assign the server to the first offset
    m_offset_to_empire_id[ii % m_stride] = m_server_id;
    m_empire_id_to_next_assigned_object_id.insert(
        std::make_pair(m_server_id, ii++ /*intentional post increment*/ ));

    for (const auto empire_id : client_ids) {
        if (empire_id == m_server_id)
            continue;
        m_offset_to_empire_id[ii % m_stride] = empire_id;
        m_empire_id_to_next_assigned_object_id.insert(
            std::make_pair(empire_id, ii++ /*intentional post increment*/));
    }
}

int IDAllocator::NewID() {
    // Find the next id for this client in the table.
    auto&& it = m_empire_id_to_next_assigned_object_id.find(m_empire_id);
    if (it == m_empire_id_to_next_assigned_object_id.end()) {
        ErrorLogger() << "m_empire_id " << m_empire_id << " not in id manager table.";
        return m_invalid_id;
    }

    // Copy the id and then update the id for the next time NewID() is called.
    const auto retval = it->second;

    // Increment the next id if not exhausted
    if (it->second >= m_exhausted_threshold) {
        it->second = m_invalid_id;
    } else if (it->second != m_invalid_id) {
        it->second += m_stride;
    }

    if (retval == m_invalid_id)
        ErrorLogger() << "Object IDs are exhausted.  No objects can be added to the Universe.";

    if (retval >= m_warn_threshold)
        WarnLogger() << "Object IDs are almost exhausted. Currently assiging id, " << retval;

    TraceLogger(IDallocator) << "Allocating id = " << retval << " for empire = " << it->first;
    return retval;
}

std::pair<bool, bool> IDAllocator::IsIDValidAndUnused(const ID_t checked_id, const int checked_empire_id) {
    const std::pair<bool, bool> hard_fail = {false, false};
    const std::pair<bool, bool> complete_success = {true, true};
    // allow legacy loading and order processing
    const std::pair<bool, bool> legacy_success = {true, false};

    if (checked_id == m_invalid_id)
        return hard_fail;

    if (checked_id == m_temp_id)
        return complete_success;

    if (checked_id >= m_exhausted_threshold || checked_id < m_zero)
        return hard_fail;

    // On the server all ids are valid. On the client only the client id is valid.
    bool is_valid_id = (m_empire_id == m_server_id) || (m_empire_id == checked_empire_id);
    if (!is_valid_id)
        return hard_fail;

    // Make sure this empire exists.
    const auto& check_it = m_empire_id_to_next_assigned_object_id.find(checked_empire_id);
    if (check_it == m_empire_id_to_next_assigned_object_id.end()) {
        ErrorLogger() << "empire_id " << checked_empire_id << " not in id manager table.";
        return hard_fail;
    }

    // If ids are exhausted then fail.
    if (check_it->second == m_invalid_id)
        return hard_fail;

    // Check that the checked_id has the correct modulus again allowing for legacy games with
    // incorrect id.
    bool is_correct_modulus = (m_offset_to_empire_id[(checked_id - m_zero) % m_stride] == checked_empire_id);
    if (!is_correct_modulus)
        return legacy_success;

    if (checked_empire_id != m_server_id)
        TraceLogger(IDallocator) << "Allocated object id = " << checked_id << " is "
                                 << " valid for empire = " << checked_empire_id;
    return complete_success;
}

bool IDAllocator::UpdateIDAndCheckIfOwned(const ID_t checked_id) {
    auto valid = IsIDValidAndUnused(checked_id, m_empire_id);

    // Hard failure
    if (!valid.first)
        return false;

    // If not on the server then ignore any legacy failures and return the check.
    if (m_empire_id != m_server_id)
        return valid.first;

    // a function to increment the next assigned id for an empire until it is past checked_id
    auto increment_next_assigned_id = [this, &checked_id](const int assigning_empire) {
        auto&& empire_and_next_id = m_empire_id_to_next_assigned_object_id.find(assigning_empire);
        if (empire_and_next_id == m_empire_id_to_next_assigned_object_id.end())
            return;

        auto& next_id = empire_and_next_id->second;
        auto init_next_id = next_id;

        while (next_id <= checked_id) {
            // Don't increment to the next_id if ids are exhausted.
            if (next_id == m_invalid_id || next_id >= m_exhausted_threshold) {
                next_id = m_invalid_id;
                return;
            }

            next_id += m_stride;
        }

        TraceLogger(IDallocator) << "next id for empire " << assigning_empire << " updated from "
                                 << init_next_id << " to " << next_id;
    };

    // On the server
    if (valid.second) {
        // Update the assigning empire's next assigned id.
        auto assigning_empire = m_offset_to_empire_id[checked_id % m_stride];
        increment_next_assigned_id(assigning_empire);

    } else {
        // For legacy saved games update all empire's next assigned ids.
        // This should stop happening after all of a saved games unprocessed orders are processed.
        for (const auto assigning_empire : m_offset_to_empire_id)
            increment_next_assigned_id(assigning_empire);
    }

    return true;;
}

template <class Archive>
void IDAllocator::SerializeForEmpire(Archive& ar, const unsigned int version, const int empire_id) {
    DebugLogger(IDallocator) << (Archive::is_loading::value ? "Deserialize " : "Serialize ")
                             << "IDAllocator()  server id = "
                             << m_server_id << " empire id = " << empire_id;

    ar & BOOST_SERIALIZATION_NVP(m_invalid_id)
        & BOOST_SERIALIZATION_NVP(m_temp_id)
        & BOOST_SERIALIZATION_NVP(m_stride)
        & BOOST_SERIALIZATION_NVP(m_server_id)
        & BOOST_SERIALIZATION_NVP(m_warn_threshold)
        & BOOST_SERIALIZATION_NVP(m_exhausted_threshold);

    if (Archive::is_loading::value) {
        // Always load whatever is there.
        ar & BOOST_SERIALIZATION_NVP(m_empire_id)
            & BOOST_SERIALIZATION_NVP(m_empire_id_to_next_assigned_object_id)
            & BOOST_SERIALIZATION_NVP(m_offset_to_empire_id);

        DebugLogger(IDallocator) << "Deserialized [" << [this](){
            std::stringstream ss;
            for (auto& empire_and_next_id : m_empire_id_to_next_assigned_object_id) {
                ss << "empire = " << empire_and_next_id.first << " next id = " << empire_and_next_id.second << ", ";
            }
            return ss.str();
        }() << "]";

    } else {

        // If the target empire is the server, provide the full map.
        if (empire_id == m_server_id) {
            if (m_empire_id != m_server_id)
                ErrorLogger() << "An empire with id = " << m_empire_id << " which is not the server "
                              << "is attempting to serialize the IDAllocator for the server.";
            ar & BOOST_SERIALIZATION_NVP(m_empire_id)
                & BOOST_SERIALIZATION_NVP(m_empire_id_to_next_assigned_object_id)
                & BOOST_SERIALIZATION_NVP(m_offset_to_empire_id);
        } else {
            auto temp_empire_id = empire_id;
            ar  & BOOST_SERIALIZATION_NVP(temp_empire_id);

            // Filter the map for empires so they only have their own actual next id and no
            // information about other clients.
            std::unordered_map<int, int> temp_empire_id_to_object_id{};
            auto temp_offset_to_empire_id = std::vector<int>(m_offset_to_empire_id.size(), m_server_id);

            auto&& it = m_empire_id_to_next_assigned_object_id.find(empire_id);
            if (it == m_empire_id_to_next_assigned_object_id.end()) {
                ErrorLogger() << "Attempt to serialize allocator for an empire_id "
                              << empire_id << " not in id manager table.";
            } else {
                temp_empire_id_to_object_id.insert(*it);
                temp_offset_to_empire_id[it->second % m_stride] = empire_id;
            }

            ar & BOOST_SERIALIZATION_NVP(temp_empire_id_to_object_id);
            ar & BOOST_SERIALIZATION_NVP(temp_offset_to_empire_id);

            DebugLogger(IDallocator) << "Serialized [" << [this, &temp_empire_id_to_object_id](){
                std::stringstream ss;
                for (auto& empire_and_next_id : temp_empire_id_to_object_id) {
                    ss << "empire = " << empire_and_next_id.first << " next id = " << empire_and_next_id.second << ", ";
                }
                return ss.str();
            }() << "]";
        }
    }
}

template void IDAllocator::SerializeForEmpire<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version, const int empire_id);
template void IDAllocator::SerializeForEmpire<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version, const int empire_id);
template void IDAllocator::SerializeForEmpire<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version, const int empire_id);
template void IDAllocator::SerializeForEmpire<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version, const int empire_id);

