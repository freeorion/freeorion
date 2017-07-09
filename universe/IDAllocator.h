#ifndef _IDAllocator_h_
#define _IDAllocator_h_

#include "../util/Logger.h"
#include <unordered_map>
#include <vector>

#include <boost/serialization/access.hpp>

/** The IDAllocator coordinates the allocation of new IDs between the server
    and a number of empires, id consumers.

    IDs are ints.  The id consumers have disjoint id spaces.  All consumers have
    a common m_stride and a unique initial offset modulo m_stride. Each id
    consumer only receives new ids from the set number of numbers x, where
    (x % m_stride == consumer's initial offset).

    The IDAllocator is associated with a single empire id.  The constructor
    creates the server association. Once serialized for another empire_id then
    it is associated with that new (lesser) empire id, and will no longer have
    the complete table.
*/

class IDAllocator {
public:
    using ID_t = int;

    /// \p client_ids are all the player ids in the game.
    IDAllocator(const int server_id,
                const std::vector<int>& client_ids,
                const ID_t invalid_id,
                const ID_t temp_id);

    /// Return a valid new id.  This is used by both clients and servers.
    ID_t NewID();

    /** Return true if \p id is unused and in the id space of \p empire_id. */
    bool IsIDValidAndUnused(const ID_t id, const int empire_id);

    /** UpdateIDAndCheckIfOwned behaves differently on the server and clients.

        On the server, it determines which client allocated \p id and updates
        the next allocated id of that client.  It then returns true.  This means
        that the server has a master list of the maximum id used by each client
        and can use all ids as valid ids when executing orders.

        On the client it returns true iff this client allocated \p id and does
        nothing else. That means that id modulo m_stride == client's offset.*/
    bool UpdateIDAndCheckIfOwned(const ID_t id);

    /** Serialize while stripping out information not known to \p empire_id. */
    template <class Archive>
        void SerializeForEmpire(Archive& ar, const unsigned int version, const int empire_id);

private:
    ID_t m_invalid_id;
    ID_t m_temp_id;

    /// m_stride is used to partition the id space into modulo m_stride sections.
    ID_t m_stride;

    // The server id and the empire id are equal on construction.  Serialization
    // and deserialization may downgrade empire id to one of the lesser (not
    // server) ids.
    int m_server_id;
    int m_empire_id;

    // A map from empire id to next used object id;
    std::unordered_map<int, ID_t> m_empire_id_to_next_assigned_object_id;

    // An index from id % m_stride to empire ids
    std::vector<int> m_offset_to_empire_id;

    /// if less than m_next_assigned_id warn about id exhaustion.
    ID_t m_warn_threshold;
    /// Stop assigning ids and start generating errors.
    ID_t m_exhausted_threshold;

};

#endif // _IDAllocator_h_
