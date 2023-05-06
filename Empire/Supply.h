#ifndef _Supply_h_
#define _Supply_h_

#include <map>
#include <set>
#include <string>
#include <vector>
#include <boost/container/flat_map.hpp>
#include <boost/serialization/access.hpp>
#include "../universe/ConstantsFwd.h"
#include "../universe/EnumsFwd.h"
#include "../util/Export.h"

class Universe;
struct ScriptingContext;
using DiploStatusMap = boost::container::flat_map<std::pair<int, int>, DiplomaticStatus>;

/** Used to calcuate all empires' supply distributions. */
class FO_COMMON_API SupplyManager {
public:
    /** Returns set of directed starlane traversals along which supply can flow.
      * Results are pairs of system ids of start and end system of traversal. */
    [[nodiscard]] auto& SupplyStarlaneTraversals() const noexcept { return m_supply_starlane_traversals; }
    [[nodiscard]] const std::set<std::pair<int, int>>& SupplyStarlaneTraversals(int empire_id) const;

    /** Returns set of directed starlane traversals along which supply could
      * flow for this empire, but which can't due to some obstruction in one
      * of the systems. */
    [[nodiscard]] auto& SupplyObstructedStarlaneTraversals() const noexcept { return m_supply_starlane_obstructed_traversals; }
    [[nodiscard]] const std::set<std::pair<int, int>>& SupplyObstructedStarlaneTraversals(int empire_id) const;

    /** Returns set of system ids where fleets can be supplied by this empire
      * (as determined by object supply meters and rules of supply propagation
      * and blockade). */
    [[nodiscard]] auto&                FleetSupplyableSystemIDs() const noexcept { return m_fleet_supplyable_system_ids; }
    [[nodiscard]] const std::set<int>& FleetSupplyableSystemIDs(int empire_id) const;
    [[nodiscard]] std::vector<int>     FleetSupplyableSystemIDs(int empire_id, bool include_allies, const ScriptingContext& context) const;
    [[nodiscard]] int                  EmpireThatCanSupplyAt(int system_id) const;

    /** Returns set of sets of systems that can share industry (systems in
      * separate groups are blockaded or otherwise separated). */
    [[nodiscard]] auto&                          ResourceSupplyGroups() const noexcept { return m_resource_supply_groups; }
    [[nodiscard]] const std::set<std::set<int>>& ResourceSupplyGroups(int empire_id) const;

    /** Returns the range from each system some empire can propagate supply.*/
    [[nodiscard]] auto&                       PropagatedSupplyRanges() const noexcept { return m_propagated_supply_ranges; }
    /** Returns the range from each system that the empire with id \a empire_id
      * can propagate supply.*/
    [[nodiscard]] const std::map<int, float>& PropagatedSupplyRanges(int empire_id) const;

    /** Returns the distance from each system some empire is away
      * from its closest source of supply without passing
      * through obstructed systems for that empire. */
    [[nodiscard]] auto& PropagatedSupplyDistances() const noexcept { return m_propagated_supply_distances; }
    /** Returns the distance from each system for the empire with id
      * \a empire_id to the closest source of supply without passing
      * through obstructed systems for that empire. */
    [[nodiscard]] const std::map<int, float>& PropagatedSupplyDistances(int empire_id) const;

    /** Returns true if system with id \a system_id is fleet supplyable or in
      * one of the resource supply groups for empire with id \a empire_id */
    [[nodiscard]] bool SystemHasFleetSupply(int system_id, int empire_id) const;
    [[nodiscard]] bool SystemHasFleetSupply(int system_id, int empire_id, bool include_allies,
                                            const DiploStatusMap& diplo_statuses) const;

    [[nodiscard]] std::string Dump(const Universe& u, int empire_id = ALL_EMPIRES) const;

    /** Calculates systems at which fleets of empires can be supplied, and
      * groups of systems that can exchange resources, and the starlane
      * traversals used to do so. */
    void Update(const ScriptingContext& context);

private:
    /** ordered pairs of system ids between which a starlane runs that can be
        used to convey resources between systems. indexed first by empire id. */
    std::map<int, std::set<std::pair<int, int>>>    m_supply_starlane_traversals;

    /** ordered pairs of system ids between which a starlane could be used to
        convey resources between system, but is not because something is
        obstructing the resource flow.  That is, the resource flow isn't limited
        by range, but by something blocking its flow. */
    std::map<int, std::set<std::pair<int, int>>>    m_supply_starlane_obstructed_traversals;

    /** ids of systems where fleets can be resupplied. indexed by empire id. */
    std::map<int, std::set<int>>                    m_fleet_supplyable_system_ids;

    /** sets of system ids that are connected by supply lines and are able to
        share resources between systems or between objects in systems. indexed
        by empire id. */
    std::map<int, std::set<std::set<int>>>          m_resource_supply_groups;

    /** for whichever empire can propagate supply into this system, what is the
        additional range from this system that empire can propagate supply */
    std::map<int, float>                            m_propagated_supply_ranges;

    /** for each empire, what systems it can propagate supply into, and how many
      * further supply jumps it could propagate past this system, if not blocked
      * from doing so by supply obstructions. */
    std::map<int, std::map<int, float>>             m_empire_propagated_supply_ranges;

    /** for whichever empire can propagate supply into this system, how far
      * that system is from the closest source of supply for that empire, along
      * possible supply propgation connections (ie. not though a supply
      * obstructed system for that empire) */
    std::map<int, float>                            m_propagated_supply_distances;

    /** for each empire, what systems it can propagate supply into, and how far
      * that system is from the closest source of supply for that empire, along
      * possible supply propgation connections (ie. not though a supply
      * obstructed system) */
    std::map<int, std::map<int, float>>             m_empire_propagated_supply_distances;


    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


#endif
