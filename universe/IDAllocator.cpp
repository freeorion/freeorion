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
                         const ID_t temp_id) :
    m_invalid_id(invalid_id),
    m_temp_id(temp_id),
    m_stride(client_ids.size() + 1),
    m_server_id(server_id),
    m_empire_id(server_id),
    m_warn_threshold(std::numeric_limits<int>::max() - 1000 * m_stride),
    m_exhausted_threshold(std::numeric_limits<int>::max() - 10 * m_stride)
{
    TraceLogger(IDallocator) << "IDAllocator() server id = " << server_id << " invalid id = " << invalid_id
                             << " warn threshold =  " << m_warn_threshold << " num clients = " << client_ids.size();
    int ii = std::max(m_invalid_id + 1, m_temp_id + 1);
    // Assign the server and each client a unique initial offset modulo m_stride.

    // Assign the server to the first offset
    m_empire_id_to_next_assigned_object_id.insert(
        std::make_pair(m_server_id, ii++ /*intentional post increment*/ ));

    for (const auto empire_id : client_ids) {
        if (empire_id == m_server_id)
            continue;
        m_empire_id_to_next_assigned_object_id.insert(std::make_pair(empire_id, ii++));
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

    DebugLogger(IDallocator) << "Allocating id = " << retval << " for empire = " << it->first;
    return retval;
}

bool IDAllocator::UpdateIDAndCheckIfOwned(const ID_t id) {
    if (id == m_invalid_id)
        return false;

    if (id == m_temp_id)
        return true;

    // Make sure this empire exists.
    auto&& it = m_empire_id_to_next_assigned_object_id.find(m_empire_id);
    if (it == m_empire_id_to_next_assigned_object_id.end()) {
        ErrorLogger() << "m_empire_id " << m_empire_id << " not in id manager table.";
        return false;
    }

    // If ids are exhausted return false.
    if (it->second == m_invalid_id)
        return false;

    // If not on the server then check that id is modulo the same as m_empire_id's output.
    if (m_empire_id != m_server_id) {
        auto valid =   (id % m_stride == it->second % m_stride);
        DebugLogger(IDallocator) << "Valid = " << valid << " client id = " << id << " for empire = " << m_empire_id;
        return valid;
    }

    // On the server figure out which empire assigned this id and update their next assigned id
    for (auto& empire_and_next_id : m_empire_id_to_next_assigned_object_id) {
        if (id % m_stride == empire_and_next_id.second % m_stride) {
            const auto next_id = id + m_stride;
            if (next_id > empire_and_next_id.second)
                empire_and_next_id.second = next_id;
            DebugLogger(IDallocator) << "Valid server id = " << id << " for empire = " << empire_and_next_id.first;
            return true;;
        }
    }

    ErrorLogger() << "id = " << id << " is invalid and was allocated by none of the empires known to the id allocator.";
    return false;
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
           & BOOST_SERIALIZATION_NVP(m_empire_id_to_next_assigned_object_id);

    } else {

        // If the target empire is the server, provide the full map.
        if (empire_id == m_server_id) {
            if (m_empire_id != m_server_id)
                ErrorLogger() << "An empire with id = " << m_empire_id << " which is not the server "
                              << "is attempting to serialize the IDAllocator for the server.";
            ar & BOOST_SERIALIZATION_NVP(m_empire_id)
               & BOOST_SERIALIZATION_NVP(m_empire_id_to_next_assigned_object_id);
        } else {
            auto temp_empire_id = empire_id;
            ar  & BOOST_SERIALIZATION_NVP(temp_empire_id);

            // Filter the map for empires so they only have their own actual next id and no
            // information about other clients.
            std::unordered_map<int, int> temp_empire_id_to_object_id{};
            for (const auto& empire_and_next_id : m_empire_id_to_next_assigned_object_id) {
                if (empire_and_next_id.first == empire_id)
                    temp_empire_id_to_object_id.insert(empire_and_next_id);
                else
                    temp_empire_id_to_object_id.insert(
                        std::make_pair(empire_and_next_id.first, empire_and_next_id.second % m_stride));
                DebugLogger(IDallocator) << "output allocator[" <<empire_and_next_id.first << "] = "
                                         << temp_empire_id_to_object_id[empire_and_next_id.first]<<"";
            }
            ar & BOOST_SERIALIZATION_NVP(temp_empire_id_to_object_id);
        }
    }
}

template void IDAllocator::SerializeForEmpire<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version, const int empire_id);
template void IDAllocator::SerializeForEmpire<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version, const int empire_id);
template void IDAllocator::SerializeForEmpire<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version, const int empire_id);
template void IDAllocator::SerializeForEmpire<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version, const int empire_id);

