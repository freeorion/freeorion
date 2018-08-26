#include "IDAllocator.h"

#include "../util/Logger.h"
#include "../util/LoggerWithOptionsDB.h"
#include "../util/Random.h"
#include "../util/Serialize.h"
#include "../util/Serialize.ipp"
#include "../util/AppInterface.h"


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
    m_exhausted_threshold(std::numeric_limits<int>::max() - 10 * m_stride),
    m_random_generator()
{
    TraceLogger(IDallocator) << "IDAllocator() server id = " << server_id << " invalid id = " << invalid_id
                             << " zero = " << m_zero
                             << " warn threshold =  " << m_warn_threshold << " num clients = " << client_ids.size();

    // Assign the server and each client a unique initial offset modulo m_stride.
    auto ii = m_zero;

    // Assign the server to the first offset
    m_offset_to_empire_id[(ii - m_zero) % m_stride] = m_server_id;
    m_empire_id_to_next_assigned_object_id.insert({m_server_id, ii});
    ++ii;

    for (const auto empire_id : client_ids) {
        if (empire_id == m_server_id)
            continue;
         AssigningEmpireForID(ii) = empire_id;
         m_empire_id_to_next_assigned_object_id.insert({empire_id, ii});
        ++ii;
    }
}

int IDAllocator::NewID() {
    // increment next id for this client until next id is not an already-used id
    IncrementNextAssignedId(m_empire_id, Objects().HighestObjectID());
    IncrementNextAssignedId(m_empire_id, GetUniverse().HighestDestroyedObjectID());

    // Find the next id for this client in the table.
    auto&& it = m_empire_id_to_next_assigned_object_id.find(m_empire_id);
    if (it == m_empire_id_to_next_assigned_object_id.end()) {
        ErrorLogger() << "m_empire_id " << m_empire_id << " not in id manager table.";
        return m_invalid_id;
    }

    // Copy the id and then update the id for the next time NewID() is called.
    const auto retval = it->second;

    auto apparent_assigning_empire = AssigningEmpireForID(retval);
    if (apparent_assigning_empire != m_empire_id)
        ErrorLogger() << "m_empire_id " << m_empire_id << " does not match apparent assigning id "
                      << apparent_assigning_empire << " for id = " << retval << " m_zero = " << m_zero
                      << " stride = " << m_stride;

    // Increment the next id if not exhausted
    if (it->second >= m_exhausted_threshold) {
        it->second = m_invalid_id;
    } else if (it->second != m_invalid_id) {
        it->second += m_stride;
    }

    if (retval == m_invalid_id)
        ErrorLogger() << "Object IDs are exhausted.  No objects can be added to the Universe.";

    if (retval >= m_warn_threshold)
        WarnLogger() << "Object IDs are almost exhausted. Currently assigning id, " << retval;

    TraceLogger(IDallocator) << "Allocating id = " << retval << " for empire = " << it->first;
    return retval;
}

std::pair<bool, bool> IDAllocator::IsIDValidAndUnused(const ID_t checked_id, const int checked_empire_id) {
    const std::pair<bool, bool> hard_fail = {false, false};
    const std::pair<bool, bool> complete_success = {true, true};
    // allow legacy loading and order processing
    const std::pair<bool, bool> legacy_success = {true, false};

    if (checked_id == m_invalid_id) {
        ErrorLogger() << m_invalid_id << " is an invalid id.";
        return hard_fail;
    }

    if (checked_id == m_temp_id)
        return complete_success;

    if (checked_id >= m_exhausted_threshold) {
        ErrorLogger() << " invalid id = " << checked_id << " is greater then the maximum id " << m_exhausted_threshold;
        return hard_fail;
    }

    if (checked_id < m_zero) {
        ErrorLogger() << " invalid id = " << checked_id
                      << " is lower than the expected minimum new id this turn " << m_zero;
        return hard_fail;
    }

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
    bool is_correct_modulus = (AssigningEmpireForID(checked_id) == checked_empire_id);
    if (!is_correct_modulus)
        return legacy_success;

    if (checked_empire_id != m_server_id)
        TraceLogger(IDallocator) << "Allocated object id = " << checked_id
                                 << " is valid for empire = " << checked_empire_id;
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

    // On the server
    // Update the assigning empire's next assigned id.
    auto assigning_empire = AssigningEmpireForID(checked_id);
    IncrementNextAssignedId(assigning_empire, checked_id);

    return true;;
}

IDAllocator::ID_t& IDAllocator::AssigningEmpireForID(ID_t id)
{ return m_offset_to_empire_id[(id - m_zero) % m_stride]; }

void IDAllocator::IncrementNextAssignedId(const int assigning_empire, const int checked_id) {
    auto&& empire_and_next_id = m_empire_id_to_next_assigned_object_id.find(assigning_empire);
    if (empire_and_next_id == m_empire_id_to_next_assigned_object_id.end())
        return;

    auto& next_id = empire_and_next_id->second;
    auto init_next_id = next_id;

    while (next_id <= checked_id && next_id != m_invalid_id) {
        next_id += m_stride;

        // Don't increment to the next_id if ids are exhausted.
        if (next_id >= m_exhausted_threshold)
            next_id = m_invalid_id;
    }

    if (init_next_id != next_id)
        TraceLogger(IDallocator) << "next id for empire " << assigning_empire << " updated from "
                                 << init_next_id << " to " << next_id;
};

void IDAllocator::ObfuscateBeforeSerialization() {
    /** Do three things to obfuscate the number of ids allocated by each client:
           1. Randomize the association of modulus and client id.
           2. Advance all clients to the same offset plus their new modulus.
           3. Add a random amount to all clients to conceal the largest number of ids allocated by
           a client. */

    // Ignore on clients.
    if (m_empire_id != m_server_id)
        return;;

    TraceLogger(IDallocator) << "Before obfuscation " << StateString();

    // Randomize the moduli
    std::shuffle(m_offset_to_empire_id.begin(), m_offset_to_empire_id.end(), m_random_generator);

    // Move the zero offset to the highest next assigned id.
    auto max_next_assigned = m_empire_id_to_next_assigned_object_id.begin()->second;
    for (const auto& empire_and_id : m_empire_id_to_next_assigned_object_id) {
        max_next_assigned = std::max(max_next_assigned, empire_and_id.second);
    }

    // The /2 factor makes the random offset decrease over time to near the typical amount of ids
    // assigned per turn, instead of growing without bound.
    auto max_random_offset = std::max(1, (max_next_assigned - m_zero) / 2);
    m_zero = max_next_assigned;

    // Check that this does not exhaust the ids
    auto new_max_next_id = max_next_assigned + max_random_offset + m_stride;
    if (new_max_next_id > m_warn_threshold)
        WarnLogger() << "Object IDs are almost exhausted. Currently assigning id, " << new_max_next_id;

    if (new_max_next_id > m_exhausted_threshold) {
        ErrorLogger() << "Object IDs are exhausted.  No objects can be added to the Universe.";
        for (auto& empire_and_id : m_empire_id_to_next_assigned_object_id)
            empire_and_id.second = m_invalid_id;
        return;
    }

    // Advance each client to its own random offset.
    ID_t assigning_empire_offset_modulus = 0;
    for (const auto assigning_empire : m_offset_to_empire_id) {
        auto empire_random_offset =  SmallIntDist(0, max_random_offset)();
        auto new_next_id = empire_random_offset + m_zero;

        // Increment until it is at the correct offset
        ID_t ii_dont_check_more_than_m_stride_ids = 0;
        while (AssigningEmpireForID(new_next_id) != assigning_empire && ii_dont_check_more_than_m_stride_ids <= m_stride) {
            ++new_next_id;
            ++ii_dont_check_more_than_m_stride_ids;
        }

        // If m_stride consecutive ids have been checked, then
        // assigning_empire is not in m_offset_to_empire_id, which is an error.
        if (ii_dont_check_more_than_m_stride_ids > m_stride) {
            ErrorLogger()
                << "While obfuscating id allocation empire " << assigning_empire
                << "is missing from the table m_offset_to_empire_id: "
                << "[(offset, empire id), " << [this]() {
                std::stringstream ss;
                std::size_t offset = 0;
                for (auto& empire_id : m_offset_to_empire_id) {
                    ss << " (" << offset++ << ", " << empire_id << "), ";
                }
                return ss.str();
            }() << "]"
                << " Empire " << assigning_empire
                << " may not be able to create new designs or objects.";
        }

        m_empire_id_to_next_assigned_object_id[assigning_empire] = new_next_id;

        ++assigning_empire_offset_modulus;
    }

    TraceLogger(IDallocator) << "After obfuscation " << StateString();
}

std::string IDAllocator::StateString() const {
    std::stringstream ss;
    ss << "IDAllocator m_zero = " << m_zero << " (Empire, offset, next_id) = [" ;

    ID_t offset = 0;
    for (const auto empire_id : m_offset_to_empire_id) {
        auto next_id_it = m_empire_id_to_next_assigned_object_id.find(empire_id);
        if (next_id_it == m_empire_id_to_next_assigned_object_id.end()) {
            ErrorLogger(IDallocator) << "missing empire_id = " << empire_id;
            continue;
        }

        ss << "(" << empire_id << ", " << offset << ", " << next_id_it->second << ") ";
        ++offset;
    }

    ss << "]";
    return ss.str();
}

template <class Archive>
void IDAllocator::SerializeForEmpire(Archive& ar, const unsigned int version, int empire_id) {
    DebugLogger(IDallocator) << (Archive::is_loading::value ? "Deserialize " : "Serialize ")
                             << "IDAllocator()  server id = "
                             << m_server_id << " empire id = " << empire_id;

    ar  & BOOST_SERIALIZATION_NVP(m_invalid_id)
        & BOOST_SERIALIZATION_NVP(m_temp_id)
        & BOOST_SERIALIZATION_NVP(m_stride);
    if (version > 0)
        ar & BOOST_SERIALIZATION_NVP(m_zero);
    ar  & BOOST_SERIALIZATION_NVP(m_server_id)
        & BOOST_SERIALIZATION_NVP(m_warn_threshold)
        & BOOST_SERIALIZATION_NVP(m_exhausted_threshold);

    if (Archive::is_loading::value) {
        // Always load whatever is there.
        ar  & BOOST_SERIALIZATION_NVP(m_empire_id)
            & BOOST_SERIALIZATION_NVP(m_empire_id_to_next_assigned_object_id)
            & BOOST_SERIALIZATION_NVP(m_offset_to_empire_id);

        DebugLogger(IDallocator) << "Deserialized [" << [this]() {
            std::stringstream ss;
            for (auto& empire_and_next_id : m_empire_id_to_next_assigned_object_id) {
                ss << "empire = " << empire_and_next_id.first << " next id = " << empire_and_next_id.second << ", ";
            }
            return ss.str();
        }() << "]";

    } else {

        if (m_empire_id != empire_id && m_empire_id != m_server_id)
            ErrorLogger() << "An empire with id = " << m_empire_id << " which is not the server "
                          << "is attempting to serialize the IDAllocator for a different empire " << empire_id;

        // If the target empire is the server, provide the full map.
        if (empire_id == m_server_id) {
            ar  & BOOST_SERIALIZATION_NVP(m_empire_id)
                & BOOST_SERIALIZATION_NVP(m_empire_id_to_next_assigned_object_id)
                & BOOST_SERIALIZATION_NVP(m_offset_to_empire_id);
        } else {
            ar  & boost::serialization::make_nvp(BOOST_PP_STRINGIZE(m_empire_id), empire_id);

            // Filter the map for empires so they only have their own actual next id and no
            // information about other clients.
            std::unordered_map<int, ID_t> temp_empire_id_to_object_id{};
            auto temp_offset_to_empire_id = std::vector<int>(m_offset_to_empire_id.size(), m_server_id);

            auto&& it = m_empire_id_to_next_assigned_object_id.find(empire_id);
            if (it == m_empire_id_to_next_assigned_object_id.end()) {
                ErrorLogger() << "Attempt to serialize allocator for an empire_id "
                              << empire_id << " not in id manager table.";
            } else {
                temp_empire_id_to_object_id.insert(*it);
                temp_offset_to_empire_id[(it->second - m_zero) % m_stride] = empire_id;
            }

            ar & boost::serialization::make_nvp(BOOST_PP_STRINGIZE(m_empire_id_to_next_assigned_object_id), temp_empire_id_to_object_id);
            ar & boost::serialization::make_nvp(BOOST_PP_STRINGIZE(m_offset_to_empire_id), temp_offset_to_empire_id);

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

