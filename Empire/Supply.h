#ifndef _Supply_h_
#define _Supply_h_

#include "../util/Export.h"

#include <boost/serialization/access.hpp>
#include <boost/functional/hash.hpp>

#include <map>
#include <set>
#include <string>
#include <memory>
#include <unordered_set>
#include <unordered_map>

extern const int ALL_EMPIRES;

// Inject a ordered pair hash into std namespace so that it is the default specialization of
// std::hash(pair).  If this is a problem with other uses of hashed pairs, then
// directly use OrderedPairHash in the unordered_xs.

namespace std {
    template <typename T1, typename T2> struct hash<std::pair<T1, T2>> {
        std::size_t operator()(pair<T1, T2> const& x) const {
            std::size_t seed{0};
            // Use boost::hash_combine vs XOR so that the hash is order dependent.
            boost::hash_combine(seed, x.first);
            boost::hash_combine(seed, x.second);
            return seed;
        }
    };
}


/** Used to calcuate all empires' supply distributions. */
class FO_COMMON_API SupplyManager {
public:
    /** \name Structors */ //@{
    SupplyManager();
    ~SupplyManager();
    SupplyManager& operator=(const SupplyManager& rhs);
    //@}

    /** \name Accessors */ //@{
    /** Returns set of directed starlane traversals along which supply can flow,
      * along with their stealth.  Results are pairs of system ids of start and
      * end system of traversal. */
    const std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>&   SupplyStarlaneTraversals() const;
    const std::unordered_map<std::pair<int, int>, float>&                            SupplyStarlaneTraversals(int empire_id) const;

    /** Returns set of directed starlane traversals along which supply could
      * flow for this empire along with its stealth, but which can't due to some
      * obstruction in one of the systems. */
    const std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>&   SupplyObstructedStarlaneTraversals() const;
    const std::unordered_map<std::pair<int, int> ,float>&                            SupplyObstructedStarlaneTraversals(int empire_id) const;

    /** Returns set of system ids where fleets can be supplied by this empire
      * (as determined by object supply meters and rules of supply propagation
      * and blockade). */
    const std::map<int, std::set<int>>&                     FleetSupplyableSystemIDs() const;
    const std::set<int>&                                    FleetSupplyableSystemIDs(int empire_id) const;
    std::set<int>                                           FleetSupplyableSystemIDs(int empire_id, bool include_allies) const;
    int                                                     EmpireThatCanSupplyAt(int system_id) const;

    /** Returns set of sets of systems that can share industry (systems in
      * separate groups are blockaded or otherwise separated). */
    const std::map<int, std::set<std::set<int>>>&           ResourceSupplyGroups() const;
    const std::set<std::set<int>>&                          ResourceSupplyGroups(int empire_id) const;

    /** Returns the range from each system some empire can propagate supply.*/
    const std::map<int, float>&                             PropagatedSupplyRanges() const;
    /** Returns the range from each system that the empire with id \a empire_id
      * can propagate supply.*/
    const std::map<int, float>&                             PropagatedSupplyRanges(int empire_id) const;

    /** Returns the distance from each system some empire is away
      * from its closest source of supply without passing
      * through obstructed systems for that empire. */
    const std::map<int, float>&                             PropagatedSupplyDistances() const;
    /** Returns the distance from each system for the empire with id
      * \a empire_id to the closest source of supply without passing
      * through obstructed systems for that empire. */
    const std::map<int, float>&                             PropagatedSupplyDistances(int empire_id) const;

    /** Returns true if system with id \a system_id is fleet supplyable or in
      * one of the resource supply groups for empire with id \a empire_id */
    bool        SystemHasFleetSupply(int system_id, int empire_id) const;
    bool        SystemHasFleetSupply(int system_id, int empire_id, bool include_allies) const;

    std::string Dump(int empire_id = ALL_EMPIRES) const;
    //@}

    /** \name Mutators */ //@{
    /** Calculates systems at which fleets of empires can be supplied, and
      * groups of systems that can exchange resources, and the starlane
      * traversals used to do so. */
    void    Update();
    //@}

private:
    class SupplyManagerImpl;
    std::unique_ptr<SupplyManagerImpl> const pimpl;


    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif // _Supply_h_
