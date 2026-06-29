#ifndef _IDAllocator_h_
#define _IDAllocator_h_


#include <random>
#include <unordered_map>
#include <vector>
#include "../util/Logger.h"
#include "../universe/ConstantsFwd.h"

class Universe;

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

    TODO:  Replace IDAllocator with using UUID as the id type ID_t instead of int.  This does not
    require coordination between clients and servers and does not leak any information to clients
    about other client's allocations.
*/
class IDAllocator {
public:
    using ID_t = int;

    /** \p client_ids are all the player ids in the game. \p highest_pre_allocated_id is the used for
      * legacy loads to offset the newly allocated id to after the ones from the save game. */
    IDAllocator(const int server_id,
                const std::vector<int>& client_ids,
                const ID_t invalid_id,
                const ID_t temp_id,
                const ID_t highest_pre_allocated_id);


    IDAllocator(const int server_id,
                const std::vector<int>& client_ids,
                const UniverseObjectID invalid_id,
                const UniverseObjectID temp_id,
                const UniverseObjectID highest_pre_allocated_id) :
        IDAllocator(server_id, client_ids, static_cast<ID_t>(Value(invalid_id)),
                    static_cast<ID_t>(Value(temp_id)), static_cast<ID_t>(Value(highest_pre_allocated_id)))
    {}

    IDAllocator& operator=(IDAllocator&& other) noexcept = default;

    /// Return a valid new id.  This is used by both clients and servers.
    ID_t NewID(const Universe& universe);

    /** Return {hard_success, soft_success} where \p hard_success determines if
        \p id is unused and valid and \p soft_success determines if \p id is in
        the id space of \p empire_id.  This allows errors that are probably due
        to legacy loading and order processing to be ignored for now. */
    std::pair<bool, bool> IsIDValidAndUnused(ID_t id, EmpireID empire_id);
    std::pair<bool, bool> IsIDValidAndUnused(UniverseObjectID id, EmpireID empire_id) { return IsIDValidAndUnused(Value(id), empire_id); }

    /** UpdateIDAndCheckIfOwned behaves differently on the server and clients.

        On the server, it determines which client allocated \p id and updates
        the next allocated id of that client.  It then returns true.  This means
        that the server has a master list of the maximum id used by each client
        and can use all ids as valid ids when executing orders.

        On the client it returns true iff this client allocated \p id and does
        nothing else. That means that id modulo m_stride == client's offset.*/
    bool UpdateIDAndCheckIfOwned(ID_t id);

    bool UpdateIDAndCheckIfOwned(UniverseObjectID id)
    { return UpdateIDAndCheckIfOwned(static_cast<ID_t>(Value(id))); }

    /** ObfuscateBeforeSerialization randomizes which client is using which modulus each turn
        before IDAllocator is serialized and sent to the clients. */
    void ObfuscateBeforeSerialization();

    /** Serialize while stripping out information not known to \p empire_id. */
    template <typename Archive>
        void SerializeForEmpire(Archive& ar, const unsigned int version, EmpireID empire_id);

private:
    /** Return the empire that should have assigned \p id. */
    EmpireID& AssigningEmpireForID(ID_t id);

    /// Increment the next assigned id for an empire until it is past checked_id.
    void IncrementNextAssignedId(const EmpireID assigning_empire, const int checked_id);

    /// Return a string representing the state.
    std::string StateString() const;

    ID_t m_invalid_id;
    ID_t m_temp_id;

    /// m_stride is used to partition the id space into modulo m_stride sections.
    ID_t m_stride;
    /// The zero point or first allocated id.
    ID_t m_zero;

    // The server id and the empire id are equal on construction.  Serialization
    // and deserialization may downgrade empire id to one of the lesser (not
    // server) ids.
    int m_server_id;
    EmpireID m_empire_id;

    // A map from empire id to next used object id;
    std::unordered_map<EmpireID, ID_t> m_empire_id_to_next_assigned_object_id;

    // An index from id % m_stride to empire ids
    std::vector<EmpireID> m_offset_to_empire_id;

    /// if less than m_next_assigned_id warn about id exhaustion.
    ID_t m_warn_threshold;
    /// Stop assigning ids and start generating errors.
    ID_t m_exhausted_threshold;

    /// Random number generator
    std::mt19937 m_random_generator;
};


#endif
