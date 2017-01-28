#ifndef _Supply_h_
#define _Supply_h_

#include "../util/Export.h"

#include <boost/serialization/access.hpp>

#include <map>
#include <set>
#include <string>
#include <memory>

extern const int ALL_EMPIRES;

/** Used to calcuate all empires' supply distributions. */
class FO_COMMON_API SupplyManager {
public:
    /** \name Structors */ //@{
    SupplyManager();
    ~SupplyManager();
    SupplyManager& operator=(const SupplyManager& rhs);
    //@}

    /** \name Accessors */ //@{
    /** Returns set of directed starlane traversals along which supply can flow.
      * Results are pairs of system ids of start and end system of traversal. */
    const std::map<int, std::set<std::pair<int, int>>>&     SupplyStarlaneTraversals() const;
    const std::set<std::pair<int, int>>&                    SupplyStarlaneTraversals(int empire_id) const;

    /** Returns set of directed starlane traversals along which supply could
      * flow for this empire, but which can't due to some obstruction in one
      * of the systems. */
    const std::map<int, std::set<std::pair<int, int>>>&     SupplyObstructedStarlaneTraversals() const;
    const std::set<std::pair<int, int>>&                    SupplyObstructedStarlaneTraversals(int empire_id) const;

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
