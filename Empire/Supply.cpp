#include "Supply.h"

#include "EmpireManager.h"
#include "Empire.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Fleet.h"
#include "../universe/Enums.h"
#include "../util/AppInterface.h"
#include "../util/Serialize.h"
#include "../util/Logger.h"

#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>


class SupplyManager::SupplyManagerImpl {
public:
    /** \name Structors */ //@{
    SupplyManagerImpl();
    SupplyManagerImpl& operator=(const SupplyManagerImpl& rhs);
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
    const std::map<int, std::set<int> >&                    FleetSupplyableSystemIDs() const;
    const std::set<int>&                                    FleetSupplyableSystemIDs(int empire_id) const;
    std::set<int>                                           FleetSupplyableSystemIDs(int empire_id, bool include_allies) const;
    int                                                     EmpireThatCanSupplyAt(int system_id) const;

    /** Returns set of sets of systems that can share industry (systems in
      * separate groups are blockaded or otherwise separated). */
    const std::map<int, std::set<std::set<int> > >&         ResourceSupplyGroups() const;
    const std::set<std::set<int> >&                         ResourceSupplyGroups(int empire_id) const;

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

    /** Part of the Update function.*/
    std::map<int, std::map<int, std::pair<float, float>>> PropagateSupplyAlongUnobstructedStarlanes(
        const std::map<int, std::map<int, float>>& empire_system_supply_ranges,
        std::map<int, std::set<int>>& empire_supply_unobstructed_systems);

    void DetermineSupplyConnectedSystemGroups(
        const std::map<int, std::map<int, std::pair<float, float>>>& empire_propagating_supply_ranges) ;
    //@}

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);

private:
    /** ordered pairs of system ids between which a starlane runs that can be
        used to convey resources between systems. indexed first by empire id. */
    std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>  m_supply_starlane_traversals;

    /** ordered pairs of system ids between which a starlane could be used to
        convey resources between system, but is not because something is
        obstructing the resource flow.  That is, the resource flow isn't limited
        by range, but by something blocking its flow. */
    std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>  m_supply_starlane_obstructed_traversals;

    /** ids of systems where fleets can be resupplied. indexed by empire id. */
    std::map<int, std::set<int> >                   m_fleet_supplyable_system_ids;

    /** sets of system ids that are connected by supply lines and are able to
        share resources between systems or between objects in systems. indexed
        by empire id. */
    std::map<int, std::set<std::set<int> > >        m_resource_supply_groups;

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

};


SupplyManager::SupplyManagerImpl::SupplyManagerImpl() :
    m_supply_starlane_traversals(),
    m_supply_starlane_obstructed_traversals(),
    m_fleet_supplyable_system_ids(),
    m_resource_supply_groups()
{}

SupplyManager::SupplyManagerImpl& SupplyManager::SupplyManagerImpl::operator=(const SupplyManagerImpl& rhs) {
    m_supply_starlane_traversals =              rhs.m_supply_starlane_traversals;
    m_supply_starlane_obstructed_traversals =   rhs.m_supply_starlane_obstructed_traversals;
    m_fleet_supplyable_system_ids =             rhs.m_fleet_supplyable_system_ids;
    m_resource_supply_groups =                  rhs.m_resource_supply_groups;
    return *this;
}

namespace {
    static const std::set<int> EMPTY_INT_SET;
    static const std::set<std::set<int>> EMPTY_INT_SET_SET;
    static const std::set<std::pair<int, int>> EMPTY_INT_PAIR_SET;
    static const std::map<int, float> EMPTY_INT_FLOAT_MAP;
    static const std::unordered_map<std::pair<int, int>, float> EMPTY_MAP_PAIR_INT_TO_FLOAT;
}

const std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>& SupplyManager::SupplyManagerImpl::SupplyStarlaneTraversals() const
{ return m_supply_starlane_traversals; }

const std::unordered_map<std::pair<int, int>, float>& SupplyManager::SupplyManagerImpl::SupplyStarlaneTraversals(int empire_id) const {
    const auto it = m_supply_starlane_traversals.find(empire_id);
    if (it != m_supply_starlane_traversals.end())
        return it->second;
    return EMPTY_MAP_PAIR_INT_TO_FLOAT;
}

const std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>& SupplyManager::SupplyManagerImpl::SupplyObstructedStarlaneTraversals() const
{ return m_supply_starlane_obstructed_traversals; }

const std::unordered_map<std::pair<int, int>, float>& SupplyManager::SupplyManagerImpl::SupplyObstructedStarlaneTraversals(int empire_id) const {
    const auto it = m_supply_starlane_obstructed_traversals.find(empire_id);
    if (it != m_supply_starlane_obstructed_traversals.end())
        return it->second;
    return EMPTY_MAP_PAIR_INT_TO_FLOAT;
}

const std::map<int, std::set<int>>& SupplyManager::SupplyManagerImpl::FleetSupplyableSystemIDs() const
{ return m_fleet_supplyable_system_ids; }

const std::set<int>& SupplyManager::SupplyManagerImpl::FleetSupplyableSystemIDs(int empire_id) const {
    std::map<int, std::set<int>>::const_iterator it = m_fleet_supplyable_system_ids.find(empire_id);
    if (it != m_fleet_supplyable_system_ids.end())
        return it->second;
    return EMPTY_INT_SET;
}

std::set<int> SupplyManager::SupplyManagerImpl::FleetSupplyableSystemIDs(int empire_id, bool include_allies) const {
    std::set<int> retval = FleetSupplyableSystemIDs(empire_id);
    if (!include_allies)
        return retval;

    // add supplyable systems of all allies
    for (auto empire_id_sys_id_set : m_fleet_supplyable_system_ids) {
        int other_empire_id = empire_id_sys_id_set.first;
        const std::set<int>& systems = empire_id_sys_id_set.second;
        if (systems.empty() || Empires().GetDiplomaticStatus(empire_id, other_empire_id) != DIPLO_PEACE)
            continue;
        retval.insert(systems.begin(), systems.end());
    }
    return retval;
}

int SupplyManager::SupplyManagerImpl::EmpireThatCanSupplyAt(int system_id) const {
    for (const std::map<int, std::set<int>>::value_type& entry : m_fleet_supplyable_system_ids) {
        if (entry.second.find(system_id) != entry.second.end())
            return entry.first;
    }
    return ALL_EMPIRES;
}

const std::map<int, std::set<std::set<int>>>& SupplyManager::SupplyManagerImpl::ResourceSupplyGroups() const
{ return m_resource_supply_groups; }

const std::set<std::set<int>>& SupplyManager::SupplyManagerImpl::ResourceSupplyGroups(int empire_id) const {
    std::map<int, std::set<std::set<int>>>::const_iterator it = m_resource_supply_groups.find(empire_id);
    if (it != m_resource_supply_groups.end())
        return it->second;
    return EMPTY_INT_SET_SET;
}

const std::map<int, float>& SupplyManager::SupplyManagerImpl::PropagatedSupplyRanges() const {
    std::cout << "BLAAAAH" << std::endl;
    return m_propagated_supply_ranges;
}

const std::map<int, float>& SupplyManager::SupplyManagerImpl::PropagatedSupplyRanges(int empire_id) const
{
    auto emp_it = m_empire_propagated_supply_ranges.find(empire_id);
    if (emp_it == m_empire_propagated_supply_ranges.end())
        return EMPTY_INT_FLOAT_MAP;
    return emp_it->second;
}

const std::map<int, float>& SupplyManager::SupplyManagerImpl::PropagatedSupplyDistances() const {
    std::cout << "GLAARB" << std::endl;
    return m_propagated_supply_distances;
}

const std::map<int, float>& SupplyManager::SupplyManagerImpl::PropagatedSupplyDistances(int empire_id) const {
    auto emp_it = m_empire_propagated_supply_distances.find(empire_id);
    if (emp_it == m_empire_propagated_supply_distances.end())
        return EMPTY_INT_FLOAT_MAP;
    return emp_it->second;
}

bool SupplyManager::SupplyManagerImpl::SystemHasFleetSupply(int system_id, int empire_id) const {
    if (system_id == INVALID_OBJECT_ID)
        return false;
    if (empire_id == ALL_EMPIRES)
        return false;
    std::map<int, std::set<int>>::const_iterator it = m_fleet_supplyable_system_ids.find(empire_id);
    if (it == m_fleet_supplyable_system_ids.end())
        return false;
    const std::set<int>& sys_set = it->second;
    if (sys_set.find(system_id) != sys_set.end())
        return true;
    return false;
}

bool SupplyManager::SupplyManagerImpl::SystemHasFleetSupply(int system_id, int empire_id, bool include_allies) const {
    if (!include_allies)
        return SystemHasFleetSupply(system_id, empire_id);
    if (system_id == INVALID_OBJECT_ID)
        return false;
    if (empire_id == ALL_EMPIRES)
        return false;

    std::set<int> empire_ids{empire_id};
    for (auto e_pair : Empires()) {
        if (Empires().GetDiplomaticStatus(empire_id, e_pair.first) == DIPLO_PEACE)
            empire_ids.insert(e_pair.first);
    }

    for (auto id : empire_ids) {
        auto sys_set_it = m_fleet_supplyable_system_ids.find(id);
        if (sys_set_it == m_fleet_supplyable_system_ids.end())
            continue;
        auto sys_set = sys_set_it->second;
        if (sys_set.find(system_id) != sys_set.end())
            return true;
    }

    return false;
}

std::string SupplyManager::SupplyManagerImpl::Dump(int empire_id) const {
    std::string retval;

    try {
        for (const std::map<int, std::set<int>>::value_type& empire_supply : m_fleet_supplyable_system_ids) {
            if (empire_id != ALL_EMPIRES && empire_supply.first != empire_id)
                continue;
            retval += "Supplyable systems for empire " + std::to_string(empire_supply.first) + "\n";
            for (int system_id : empire_supply.second) {
                std::shared_ptr<const System> sys = GetSystem(system_id);
                if (!sys)
                    continue;
                retval += "\n" + sys->PublicName(empire_id) + " (" + std::to_string(sys->ID()) + ") ";

                retval += "\nTraversals from here to: ";

                for (const auto& trav : m_supply_starlane_traversals.at(empire_supply.first)) {
                    if (trav.first.first == sys->ID()) {
                        std::shared_ptr<const UniverseObject> obj = GetUniverseObject(trav.first.second);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Traversals to here from: ";
                for (const auto& trav : m_supply_starlane_traversals.at(empire_supply.first)) {
                    if (trav.first.second == sys->ID()) {
                        std::shared_ptr<const UniverseObject> obj = GetUniverseObject(trav.first.first);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Blocked Traversals from here to: ";
                for (const auto& trav : m_supply_starlane_obstructed_traversals.at(empire_supply.first)) {
                    if (trav.first.first == sys->ID()) {
                        std::shared_ptr<const UniverseObject> obj = GetUniverseObject(trav.first.second);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Blocked Traversals to here from: ";
                for (const auto& trav : m_supply_starlane_obstructed_traversals.at(empire_supply.first)) {
                    if (trav.first.second == sys->ID()) {
                        std::shared_ptr<const UniverseObject> obj = GetUniverseObject(trav.first.first);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

            }
            retval += "\n\n";
        }

        for (const std::map<int, std::set<std::set<int>>>::value_type& empire_supply : m_resource_supply_groups) {
            retval += "Supply groups for empire " + std::to_string(empire_supply.first) + "\n";
            for (const std::set<std::set<int>>::value_type& system_group : empire_supply.second) {
                retval += "group: ";
                for (int system_id : system_group) {
                    std::shared_ptr<const System> sys = GetSystem(system_id);
                    if (!sys)
                        continue;
                    retval += "\n" + sys->PublicName(empire_id) + " (" + std::to_string(sys->ID()) + ") ";
                }
                retval += "\n";
            }
            retval += "\n\n";
        }
    } catch (const std::exception& e) {
        ErrorLogger() << "SupplyManager::Dump caught exception: " << e.what();
    }

    return retval;
}

namespace {
    float EmpireTotalSupplyRangeSumInSystem(int empire_id, int system_id) {
        return 0.0f;
    }

    float DistanceBetweenObjects(int obj1_id, int obj2_id) {
        std::shared_ptr<const System> obj1 = GetSystem(obj1_id);
        if (!obj1)
            return 0.0f;
        std::shared_ptr<const System> obj2 = GetSystem(obj2_id);
        if (!obj2)
            return 0.0f;
        double dx = obj2->X() - obj1->X();
        double dy = obj2->Y() - obj1->Y();
        return static_cast<float>(std::sqrt(dx*dx + dy*dy));
    }
}

namespace {
    /** Parts of the Update function that don't depend on SupplyManager.*/

std::unordered_map<int, std::vector<int>> CalculateColonyDisruptedSupply(
    const std::map<int, std::map<int, float>>& empire_system_supply_ranges)
{
    // Map from Empire to Systems where only other empires have supply sources and there is no
    // friendly fleet.  This is used to help the AI until it can full manage supply mechanics
    std::unordered_map<int, std::vector<int>> empire_to_colony_disrupted_systems;

    const std::vector<std::shared_ptr<Fleet>> fleets = GetUniverse().Objects().FindObjects<Fleet>();

    for (const std::map<int, Empire*>::value_type& entry : Empires()) {
        int empire_id = entry.first;

        auto& self_protected_supply = empire_to_colony_disrupted_systems[empire_id];

        // Find systems containing armed aggressive fleets from own empire.
        std::set<int> systems_containing_friendly_fleets;

        const std::set<int>& known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(empire_id);
        for (std::shared_ptr<const Fleet> fleet : fleets) {
            int system_id = fleet->SystemID();
            if (system_id == INVALID_OBJECT_ID) {
                continue;   // not in a system, so can't affect system obstruction
            } else if (known_destroyed_objects.find(fleet->ID()) != known_destroyed_objects.end()) {
                continue; //known to be destroyed so can't affect supply, important just in case being updated on client side
            }

            if ((fleet->HasArmedShips() || fleet->HasFighterShips()) && fleet->Aggressive()) {
                if (fleet->OwnedBy(empire_id)) {
                    if (fleet->NextSystemID() == INVALID_OBJECT_ID || fleet->NextSystemID() == fleet->SystemID()) {
                        systems_containing_friendly_fleets.insert(system_id);
                    }
                }
            }
        }

        std::set<int> systems_where_hostile_others_have_supply_sources_and_current_empire_doesnt;
        // add all systems where hostile_others have supply
        for (const std::map<int, std::map<int, float>>::value_type& empire_supply : empire_system_supply_ranges) {
            if (empire_supply.first == empire_id || empire_supply.first == ALL_EMPIRES)
                continue;

            if (Empires().GetDiplomaticStatus(empire_id, empire_supply.first) != DIPLO_WAR)
                continue;

            for (const std::map<int, float>::value_type& supply_range : empire_supply.second) {
                if (supply_range.second <= 0.0f)
                    continue;
                systems_where_hostile_others_have_supply_sources_and_current_empire_doesnt.insert(supply_range.first);
            }
        }
        // remove systems were this empire has supply
        std::map<int, std::map<int, float>>::const_iterator it = empire_system_supply_ranges.find(empire_id);
        if (it != empire_system_supply_ranges.end()) {
            for (const std::map<int, float>::value_type& supply_range : it->second) {
                if (supply_range.second <= 0.0f)
                    continue;
                systems_where_hostile_others_have_supply_sources_and_current_empire_doesnt.erase(supply_range.first);
            }
        }

        // for systems where others have supply sources and this empire doesn't
        // and where this empire has no fleets...
        // supply is obstructed
        for (int system_id : systems_where_hostile_others_have_supply_sources_and_current_empire_doesnt) {
            if (systems_containing_friendly_fleets.find(system_id) == systems_containing_friendly_fleets.end())
                self_protected_supply.push_back(system_id);
        }
    }
    return empire_to_colony_disrupted_systems;
}

void ObstructSupplyIfUncontestedHostileSupplySource(
    const std::map<int, std::map<int, float>>& empire_system_supply_ranges,
    std::map<int, std::set<int>>& empire_supply_unobstructed_systems)
{
    for (auto& empire_to_colony_disrupted_systems : CalculateColonyDisruptedSupply(empire_system_supply_ranges)) {
        for (auto system_id: empire_to_colony_disrupted_systems.second)
            empire_supply_unobstructed_systems.erase(system_id);
    }
}

/** Return a bonus value applied to supply to reduce likelyhood of ties. */
// TODO consider either exposing this in the GUI or removing it to improve the transparency of the
// supply mechanic.
float ComputeSupplyBonuses(const int empire_id, const int sys_id, std::pair<float, float> range_and_distance,
                           std::map<int, std::map<int, float>>& empire_system_supply_range_sums)
{
    // stuff to break ties...
    float bonus = 0.0f;

    // empires with visibility into system
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(sys_id, empire_id);
    if (vis >= VIS_PARTIAL_VISIBILITY)
        bonus += 0.02f;
    else if (vis == VIS_BASIC_VISIBILITY)
        bonus += 0.01f;

    // empires with ships / planets in system (that haven't already made it obstructed for another empire)
    bool has_ship = false, has_outpost = false, has_colony = false;
    if (std::shared_ptr<const System> sys = GetSystem(sys_id)) {
        std::vector<int> obj_ids;
        std::copy(sys->ContainedObjectIDs().begin(), sys->ContainedObjectIDs().end(), std::back_inserter(obj_ids));
        for (std::shared_ptr<UniverseObject> obj : Objects().FindObjects(obj_ids)) {
            if (!obj)
                continue;
            if (!obj->OwnedBy(empire_id))
                continue;
            if (obj->ObjectType() == OBJ_SHIP) {
                has_ship = true;
                continue;
            }
            if (obj->ObjectType() == OBJ_PLANET) {
                if (std::shared_ptr<Planet> planet = std::dynamic_pointer_cast<Planet>(obj)) {
                    if (!planet->SpeciesName().empty())
                        has_colony = true;
                    else
                        has_outpost = true;
                    continue;
                }
            }
        }
    }
    if (has_ship)
        bonus += 0.1f;
    if (has_colony)
        bonus += 0.5f;
    else if (has_outpost)
        bonus += 0.3f;

    // sum of all supply sources in this system
    bonus += empire_system_supply_range_sums[empire_id][sys_id] / 1000.0f;

    // distance to supply source from here
    float propagated_distance_to_supply_source = std::max(1.0f, range_and_distance.second);
    bonus += 0.0001f / propagated_distance_to_supply_source;

    // todo: other bonuses?
    return bonus;
}

/** Remove \p sys_id from unobstructed_systems, remove all startlanes that arrive or depart sys_id
    from lane_traversals and remove obstructed traversals that depart this system.*/
void RemoveSystemFromTraversals(
    const int sys_id,
    std::set<int>& supply_unobstructed_systems,
    std::unordered_map<std::pair<int, int>, float>& lane_traversals,
    std::unordered_map<std::pair<int, int>, float>& obstructed_traversals)
{
    // Remove from unobstructed systems
    supply_unobstructed_systems.erase(sys_id);

    auto lane_traversals_initial = lane_traversals;
    auto obstrcuted_traversals_initial = obstructed_traversals;

    // remove from traversals departing from or going to this system for this empire,
    // and set any traversals going to this system as obstructed
    for (const auto& lane : lane_traversals_initial) {
        if (lane.first.first == sys_id) {
            lane_traversals.erase(std::make_pair(sys_id, lane.first.second));
        }
        if (lane.first.second == sys_id) {
            lane_traversals.erase(std::make_pair(lane.first.first, sys_id));
            obstructed_traversals.insert(std::make_pair(std::make_pair(lane.first.first, sys_id), 0.0f));
        }
    }

    // remove obstructed traverals departing from this system
    for (const auto& lane : obstrcuted_traversals_initial) {
        if (lane.first.first == sys_id)
            obstructed_traversals.erase(std::make_pair(lane.first.first, lane.first.second));
    }
}

/** Add an obstructed traversal.*/
void AddObstructedTraversal(
    const int sys_id, const int end_sys_id,
    const std::set<int>& supply_unobstructed_systems,
    const std::unordered_map<std::pair<int, int>, float>& lane_traversals,
    std::unordered_map<std::pair<int, int>, float>& obstructed_traversals)
{
    obstructed_traversals.insert(std::make_pair(std::make_pair(sys_id, end_sys_id), 0.0f));
}

/** Add a traversal */
void AddTraversal(
    const int sys_id, const int end_sys_id,
    const std::set<int>& supply_unobstructed_systems,
    std::unordered_map<std::pair<int, int>, float>& lane_traversals,
    std::unordered_map<std::pair<int, int>, float>& obstructed_traversals)
{

    //DebugLogger() << "Added traversal from " << sys_id << " to " << end_sys_id;
    // always record a traversal, so connectivity is calculated properly
    lane_traversals.insert(std::make_pair(std::make_pair(sys_id, end_sys_id), 0.0f));

    // erase any previous obstructed traversal that just succeeded
    if (obstructed_traversals.find(std::make_pair(sys_id, end_sys_id)) !=
        obstructed_traversals.end())
    {
        //DebugLogger() << "Removed obstructed traversal from " << sys_id << " to " << end_sys_id;
        obstructed_traversals.erase(std::make_pair(sys_id, end_sys_id));
    }
    if (obstructed_traversals.find(std::make_pair(end_sys_id, sys_id)) !=
        obstructed_traversals.end())
    {
        //DebugLogger() << "Removed obstructed traversal from " << end_sys_id << " to " << sys_id;
        obstructed_traversals.erase(std::make_pair(end_sys_id, sys_id));
    }
}
}

std::map<int, std::map<int, std::pair<float, float>>> SupplyManager::SupplyManagerImpl::PropagateSupplyAlongUnobstructedStarlanes(
    const std::map<int, std::map<int, float>>& empire_system_supply_ranges,
    std::map<int, std::set<int>>& empire_supply_unobstructed_systems
)
{
    // map from empire id to map from system id to sum of supply source ranges
    // owned by empire in that in system
    std::map<int, std::map<int, float>> empire_system_supply_range_sums;
    for (auto empire_id_pair : empire_system_supply_ranges) {
        for (auto sys_id_pair : empire_id_pair.second) {
            empire_system_supply_range_sums[empire_id_pair.first][sys_id_pair.first] =
                EmpireTotalSupplyRangeSumInSystem(empire_id_pair.first, sys_id_pair.first);
        }
    }

    // system connections each empire can see / use for supply propagation
    std::map<int, std::map<int, std::set<int>>> empire_visible_starlanes;
    for (std::map<int, Empire*>::value_type& entry : Empires()) {
        const Empire* empire = entry.second;
        empire_visible_starlanes[entry.first] = empire->KnownStarlanes();//  VisibleStarlanes();
    }

    std::set<int> systems_with_supply_in_them;

    // store (supply range in jumps, and distance to supply source) of all
    // unobstructed systems before propagation, and add to list of systems
    // to propagate from.
    std::map<int, std::map<int, std::pair<float, float>>> empire_propagating_supply_ranges;
    float max_range = 0.0f;

    for (const std::map<int, std::map<int, float>>::value_type& empire_supply : empire_system_supply_ranges) {
        int empire_id = empire_supply.first;
        const std::set<int>& unobstructed_systems = empire_supply_unobstructed_systems[empire_id];

        for (const std::map<int, float>::value_type& supply_range : empire_supply.second) {
            int system_id = supply_range.first;
            if (unobstructed_systems.find(system_id) != unobstructed_systems.end()) {
                // stored: first -> source supply range.  second -> distance to source (0 for the source itself)
                empire_propagating_supply_ranges[empire_id][system_id] = {supply_range.second, 0.0f};
                if (supply_range.second > max_range)
                    max_range = supply_range.second;
                systems_with_supply_in_them.insert(system_id);
            }
        }
    }


    // spread supply out from sources by "diffusion" like process, along unobstructed
    // starlanes, until the range is exhausted.
    for (float range_to_spread = max_range; range_to_spread >= 0;
         range_to_spread -= 1.0f)
    {
        //DebugLogger() << "!!!! Reduced spreading range to " << range_to_spread;

        // update systems that have supply in them
        for (const std::map<int, std::map<int, std::pair<float, float>>>::value_type& empire_supply : empire_propagating_supply_ranges) {
            for (const std::map<int, std::pair<float, float>>::value_type& supply_range : empire_supply.second)
            { systems_with_supply_in_them.insert(supply_range.first); }
        }


        // resolve supply fights between multiple empires in one system.
        // pass over all empire-supplied systems, removing supply for all
        // but the empire with the highest supply range in each system
        for (int sys_id : systems_with_supply_in_them) {
            // sort empires by range in this system
            std::map<float, std::set<int>> empire_ranges_here;
            for (std::map<int, std::map<int, std::pair<float, float>>>::value_type& empire_supply : empire_propagating_supply_ranges) {
                int empire_id = empire_supply.first;
                std::map<int, std::pair<float, float>>::const_iterator empire_supply_it = empire_supply.second.find(sys_id);
                // does this empire have any range in this system? if so, store it
                if (empire_supply_it == empire_supply.second.end())
                    continue;

                // store ids of empires indexed by adjusted propgated range, in order to sort by range
                float propagated_range = empire_supply_it->second.first;
                empire_ranges_here[
                    propagated_range
                    + ComputeSupplyBonuses(empire_id, sys_id,
                                           empire_supply_it->second, empire_system_supply_range_sums)
                ].insert(empire_supply.first);
            }

            if (empire_ranges_here.empty())
                continue;   // no empire has supply here?
            if (empire_ranges_here.size() == 1 && empire_ranges_here.begin()->second.size() < 2)
                continue;   // only one empire has supply here

            // remove range entries and traversals for all empires hostile to any of the top empires
            const auto & top_empires = empire_ranges_here.rbegin()->second;

            for (std::map<int, std::map<int, std::pair<float, float>>>::value_type& empire_supply : empire_propagating_supply_ranges) {
                int empire_id = empire_supply.first;

                // Dont remove if none of the top empires are hostile to this empire.
                if (std::none_of(top_empires.begin(), top_empires.end(),
                                 [empire_id](int a_top_empire_id) {
                                     return (empire_id == a_top_empire_id) ? false :
                                         Empires().GetDiplomaticStatus(empire_id, a_top_empire_id) == DIPLO_WAR;
                                 }))
                { continue; }

                // remove from range entry...
                std::map<int, std::pair<float, float>>& empire_ranges = empire_supply.second;
                empire_ranges.erase(sys_id);

                RemoveSystemFromTraversals(sys_id, empire_supply_unobstructed_systems[empire_id],
                                           m_supply_starlane_traversals[empire_id],
                                           m_supply_starlane_obstructed_traversals[empire_id]);
                //DebugLogger() << "... removed empire " << empire_id << " system " << sys_id << " supply.";
            }

            //// DEBUG
            // DebugLogger() << "after culling empires ranges at system " << sys_id << ":";
            // for (std::map<int, std::map<int, float>>::value_type& empire_supply : empire_propagating_supply_ranges) {
            //    std::map<int, float>& system_ranges = empire_supply.second;
            //    std::map<int, float>::iterator range_it = system_ranges.find(sys_id);
            //    if (range_it != system_ranges.end())
            //        DebugLogger() << empire_supply.first << " : " << range_it->second;
            // }
            //// END DEBUG
        }

        if (range_to_spread <= 0)
            break;

        // initialize next iteration with current supply distribution
        std::map<int, std::map<int, std::pair<float, float>>> empire_propagating_supply_ranges_next = empire_propagating_supply_ranges;


        // for sources of supply of at least the minimum range for this
        // iteration that are in the current map, give adjacent systems one
        // less supply in the next iteration (unless as much or more is already
        // there for a hostile empire)
            for (const std::map<int, std::map<int, std::pair<float, float>>>::value_type& empire_supply : empire_propagating_supply_ranges) {
            int empire_id = empire_supply.first;
            //DebugLogger() << ">-< Doing supply propagation for empire " << empire_id << " >-<";
            const std::map<int, std::pair<float, float>>& prev_sys_ranges = empire_supply.second;
            const std::set<int>& unobstructed_systems = empire_supply_unobstructed_systems[empire_id];

            for (const std::map<int, std::pair<float, float>>::value_type& supply_range : empire_supply.second) {
                // does the source system have enough supply range to propagate outwards?
                float range = supply_range.second.first;
                if (range != range_to_spread)
                    continue;
                float range_after_one_more_jump = range - 1.0f; // what to set adjacent systems' ranges to (at least)

                // how far is this system from a source of supply for this empire?
                float distance_to_supply_source = supply_range.second.second;

                // what starlanes can be used to propagate supply?
                int system_id = supply_range.first;

                //DebugLogger() << "propagating from system " << system_id << " which has range: " << range << " and distance: " << distance_to_supply_source;

                // attempt to propagate to all adjacent systems...
                for (int lane_end_sys_id : empire_visible_starlanes[empire_id][system_id]) {
                    // is propagation to the adjacent system obstructed?
                    if (unobstructed_systems.find(lane_end_sys_id) == unobstructed_systems.end()) {
                        // propagation obstructed!
                        //DebugLogger() << "Added obstructed traversal from " << system_id << " to " << lane_end_sys_id << " due to not being on unobstructed systems";
                        AddObstructedTraversal(system_id, lane_end_sys_id,
                                               empire_supply_unobstructed_systems[empire_id],
                                               m_supply_starlane_traversals[empire_id],
                                               m_supply_starlane_obstructed_traversals[empire_id]);
                        continue;
                    }
                    // propagation not obstructed.


                    // does another hostile empire already have as much or more supply here from a previous iteration?
                    float other_empire_biggest_range = -10000.0f;   // arbitrary big numbeer
                    for (const std::map<int, std::map<int, std::pair<float, float>>>::value_type& other_empire_supply : empire_propagating_supply_ranges) {
                        int other_empire_id = other_empire_supply.first;
                        if ((other_empire_id == empire_id)
                            || Empires().GetDiplomaticStatus(empire_id, other_empire_id) != DIPLO_WAR)
                        { continue; }

                        const std::map<int, std::pair<float, float>>& prev_other_empire_sys_ranges = other_empire_supply.second;
                        std::map<int, std::pair<float, float>>::const_iterator prev_other_empire_range_it = prev_other_empire_sys_ranges.find(lane_end_sys_id);
                        if (prev_other_empire_range_it == prev_other_empire_sys_ranges.end())
                            continue;
                        if (prev_other_empire_range_it->second.first > other_empire_biggest_range)
                            other_empire_biggest_range = prev_other_empire_range_it->second.first;
                    }

                    // if so, add a blocked traversal and continue
                    if (range_after_one_more_jump <= other_empire_biggest_range) {
                        AddObstructedTraversal(system_id, lane_end_sys_id,
                                               empire_supply_unobstructed_systems[empire_id],
                                               m_supply_starlane_traversals[empire_id],
                                               m_supply_starlane_obstructed_traversals[empire_id]);
                        //DebugLogger() << "Added obstructed traversal from " << system_id << " to " << lane_end_sys_id << " due to other empire biggest range being " << other_empire_biggest_range;
                        continue;
                    }

                    // otherwise, propagate into system...
                    float lane_length = DistanceBetweenObjects(system_id, lane_end_sys_id);
                    float distance_to_supply_source_after_next_lane = lane_length + distance_to_supply_source;

                    //DebugLogger() << "Attempting to propagate into system: " << lane_end_sys_id << " the new range: " << range_after_one_more_jump << " and distance: " << distance_to_supply_source_after_next_lane;

                    // if propagating supply would increase the range of the adjacent system,
                    // or decrease the distance to the adjacent system from a supply source...
                    std::map<int, std::pair<float, float>>::const_iterator prev_range_it = prev_sys_ranges.find(lane_end_sys_id);
                    if (prev_range_it == prev_sys_ranges.end()) {
                        empire_propagating_supply_ranges_next[empire_id][lane_end_sys_id] =
                            {range_after_one_more_jump, distance_to_supply_source_after_next_lane};
                        //DebugLogger() << " ... default case: no previous entry.";

                    } else {
                        //DebugLogger() << " ... previous entry values: " << prev_range_it->second.first << " and " << prev_range_it->second.second;

                        if (range_after_one_more_jump > prev_range_it->second.first) {
                            empire_propagating_supply_ranges_next[empire_id][lane_end_sys_id].first =
                                range_after_one_more_jump;
                            //DebugLogger() << " ... range increased!";
                        }
                        if (distance_to_supply_source_after_next_lane < prev_range_it->second.second) {
                            empire_propagating_supply_ranges_next[empire_id][lane_end_sys_id].second =
                                distance_to_supply_source_after_next_lane;
                            //DebugLogger() << " ... distance decreased!";
                        }
                    }

                    AddTraversal(system_id, lane_end_sys_id,
                                 empire_supply_unobstructed_systems[empire_id],
                                 m_supply_starlane_traversals[empire_id],
                                 m_supply_starlane_obstructed_traversals[empire_id]);
                }
            }
        }

        // save propagated results for next iteration
        empire_propagating_supply_ranges = empire_propagating_supply_ranges_next;
    }

    return empire_propagating_supply_ranges;
}

void SupplyManager::SupplyManagerImpl::DetermineSupplyConnectedSystemGroups(
    const std::map<int, std::map<int, std::pair<float, float>>>& empire_propagating_supply_ranges)
{
    // determine supply-connected groups of systems for each empire.
    // need to merge interconnected supply groups into as few sets of mutually-
    // supply-exchanging systems as possible.  This requires finding the
    // connected components of an undirected graph, where the node
    // adjacency are the directly-connected systems determined above.
    for (const std::map<int, std::map<int, std::pair<float, float>>>::value_type& empire_supply : empire_propagating_supply_ranges) {
        int empire_id = empire_supply.first;

        // assemble all direct connections between systems from remaining traversals
        std::map<int, std::set<int> > supply_groups_map;
        for (const auto& lane : m_supply_starlane_traversals[empire_id]) {
            supply_groups_map[lane.first.first].insert(lane.first.second);
            supply_groups_map[lane.first.second].insert(lane.first.first);
        }

        // also add connections from all fleet-supplyable systems to themselves, so that
        // any fleet supply able system with no connection to another system can still
        // have resource sharing within tiself
        for (int system_id : m_fleet_supplyable_system_ids[empire_id]) {
            supply_groups_map[system_id].insert(system_id);
        }

        if (supply_groups_map.empty())
            continue;

        // create graph
        boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> graph;

        // boost expects vertex labels to range from 0 to num vertices - 1, so need
        // to map from system id to graph id and back when accessing vertices
        std::vector<int> graph_id_to_sys_id;
        graph_id_to_sys_id.reserve(supply_groups_map.size());

        std::map<int, int> sys_id_to_graph_id;
        int graph_id = 0;
        for (std::map<int, std::set<int>>::value_type& supply_group : supply_groups_map) {
            int sys_id = supply_group.first;
            boost::add_vertex(graph);   // should add with index = graph_id

            graph_id_to_sys_id.push_back(sys_id);
            sys_id_to_graph_id[sys_id] = graph_id;
            ++graph_id;
        }

        // add edges for all direct connections between systems
        // and add edges from fleet supplyable systems to themselves
        for (std::map<int, std::set<int>>::value_type& supply_group : supply_groups_map) {
            int start_graph_id = sys_id_to_graph_id[supply_group.first];
            for (int system_id : supply_group.second) {
                int end_graph_id = sys_id_to_graph_id[system_id];
                boost::add_edge(start_graph_id, end_graph_id, graph);
            }
        }

        // declare storage and fill with the component id (group id of connected systems)
        // for each graph vertex
        std::vector<int> components(boost::num_vertices(graph));
        boost::connected_components(graph, &components[0]);

        // convert results back from graph id to system id, and into desired output format
        // output: std::map<int, std::set<std::set<int>>>& m_resource_supply_groups

        // first, sort into a map from component id to set of system ids in component
        std::map<int, std::set<int>> component_sets_map;
        for (std::size_t comp_graph_id = 0; comp_graph_id != components.size(); ++comp_graph_id) {
            int label = components[comp_graph_id];
            int sys_id = graph_id_to_sys_id[comp_graph_id];
            component_sets_map[label].insert(sys_id);
        }

        // copy sets in map into set of sets
        for (std::map<int, std::set<int>>::value_type& component_set : component_sets_map) {
            m_resource_supply_groups[empire_id].insert(component_set.second);
        }
    }
}

namespace {
    /** Return a map from empire id to detection strength.*/
    std::unordered_map<int, float> EmpireToDetectionStrengths() {
        std::unordered_map<int, float> retval;
        for (const auto& entry : Empires()) {
            const auto  empire_id = entry.first;
            const auto& empire = entry.second;
            const auto  meter = empire->GetMeter("METER_DETECTION_STRENGTH");
            retval[empire_id] = meter ? meter->Current() : 0.0f;
        }
        return retval;
    }
}

void SupplyManager::SupplyManagerImpl::Update() {
    m_supply_starlane_traversals.clear();
    m_supply_starlane_obstructed_traversals.clear();
    m_fleet_supplyable_system_ids.clear();
    m_resource_supply_groups.clear();
    m_propagated_supply_ranges.clear();

    // for each empire, need to get a set of sets of systems that can exchange
    // resources.  some sets may be just one system, in which resources can be
    // exchanged between UniverseObjects producing or consuming them, but which
    // can't exchange with any other systems.

    // which systems can share resources depends on system supply ranges, which
    // systems have obstructions to supply propagation for each empire, and
    // the ranges and obstructions of other empires' supply, as only one empire
    // can supply each system or propagate along each starlane. one empire's
    // propagating supply can push back another's, if the pusher's range is
    // larger.


    // Map from empire id to a map from system id to the stealth and supply ranges of empire
    // objects in that system.
    std::unordered_map<int, std::unordered_map<int, std::set<std::pair<float, float>>>> empire_to_system_to_stealth_supply;

    // map from empire id to map from system id to range (in starlane jumps)
    // that supply can be propagated out of that system by that empire.
    std::map<int, std::map<int, float> > empire_system_supply_ranges;
    // map from empire id to which systems are obstructed for it for supply
    // propagation
    std::map<int, std::set<int> > empire_supply_unobstructed_systems;

    /** Map from empire id to detection strength.*/
    auto empire_to_detection = EmpireToDetectionStrengths();

    for (const auto& entry : Empires()) {
        const auto empire_id = entry.first;
        const auto& empire = entry.second;
        empire_to_system_to_stealth_supply[empire_id] = empire->SystemToStealthAndSupplyRange();
        empire_system_supply_ranges[empire_id] = empire->SystemSupplyRanges();
        empire_supply_unobstructed_systems[empire_id] = empire->SupplyUnobstructedSystems();

        //std::stringstream ss;
        //for (int system_id : empire_supply_unobstructed_systems[entry.first])
        //{ ss << system_id << ", "; }
        //DebugLogger() << "Empire " << empire->EmpireID() << " unobstructed systems: " << ss.str();
    }

    /////
    // probably temporary: additional restriction here for supply propagation
    // but not for general system obstruction as determind within Empire::UpdateSupplyUnobstructedSystems
    /////
    ObstructSupplyIfUncontestedHostileSupplySource(
        empire_system_supply_ranges,
        empire_supply_unobstructed_systems);
    /////
    // end probably temporary...
    /////


    // Calculate for each empire the supply available at each system
    std::map<int, std::map<int, std::pair<float, float>>>&& empire_propagating_supply_ranges =
        PropagateSupplyAlongUnobstructedStarlanes(
            empire_system_supply_ranges,
            empire_supply_unobstructed_systems);


    //// DEBUG
    //DebugLogger() << "SuppolyManager::Update: after removing conflicts, empires can provide supply to the following system ids:";
    //for (std::map<int, std::map<int, float>>::value_type& empire_supply : empire_propagating_supply_ranges) {
    //    int empire_id = empire_supply.first;
    //    std::stringstream ss;
    //    for (std::map<int, float>::value_type& supply_range : empire_supply.second) {
    //        ss << supply_range.first << " (" << supply_range.second << "),  ";
    //    }
    //    DebugLogger() << "empire: " << empire_id << ":  " << ss.str();
    //}
    //// END DEBUG

    // record which systems are fleet supplyable by each empire (after resolving conflicts in each system)
    for (const std::map<int, std::map<int, std::pair<float, float>>>::value_type& empire_supply : empire_propagating_supply_ranges) {
        int empire_id = empire_supply.first;
        for (const std::map<int, std::pair<float, float>>::value_type& supply_range : empire_supply.second) {
            if (supply_range.second.first < 0.0f)
                continue;   // negative supply doesn't count... zero does (it just reaches)
            m_fleet_supplyable_system_ids[empire_id].insert(supply_range.first);

            // should be only one empire per system at this point, but use max just to be safe...
            m_propagated_supply_ranges[supply_range.first] =
                std::max(supply_range.second.first, m_propagated_supply_ranges[supply_range.first]);
            m_empire_propagated_supply_ranges[empire_id][supply_range.first] =
                m_propagated_supply_ranges[supply_range.first];

            // should be only one empire per system at this point, but use max just to be safe...
            m_propagated_supply_distances[supply_range.first] =
                std::max(supply_range.second.second, m_propagated_supply_distances[supply_range.first]);
            m_empire_propagated_supply_distances[empire_id][supply_range.first] =
                m_propagated_supply_distances[supply_range.first];
        }

        //DebugLogger() << "For empire: " << empire_id << " system supply distances: ";
        //for (auto entry : m_empire_propagated_supply_distances[empire_id]) {
        //    DebugLogger() << entry.first << " : " << entry.second;
        //}
    }

    DetermineSupplyConnectedSystemGroups(empire_propagating_supply_ranges);
}

template <class Archive>
void SupplyManager::SupplyManagerImpl::serialize(Archive& ar, const unsigned int version)
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

template void SupplyManager::SupplyManagerImpl::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void SupplyManager::SupplyManagerImpl::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void SupplyManager::SupplyManagerImpl::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void SupplyManager::SupplyManagerImpl::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);


////////////////////////////////////////
// Supply Manager
////////////////////////////////////////

SupplyManager::SupplyManager() :
    pimpl(new SupplyManagerImpl)
{}

// Required to call pimpl destructor.
SupplyManager::~SupplyManager() {}

SupplyManager& SupplyManager::operator=(const SupplyManager& rhs) {
    pimpl->operator=(*rhs.pimpl);
    return *this;
}

const std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>&   SupplyManager::SupplyStarlaneTraversals() const
{ return pimpl->SupplyStarlaneTraversals(); }

const std::unordered_map<std::pair<int, int>, float>&                   SupplyManager::SupplyStarlaneTraversals(int empire_id) const
{ return pimpl->SupplyStarlaneTraversals(empire_id); }

const std::unordered_map<int, std::unordered_map<std::pair<int, int>, float>>&   SupplyManager::SupplyObstructedStarlaneTraversals() const
{ return pimpl->SupplyObstructedStarlaneTraversals(); }

const std::unordered_map<std::pair<int, int>, float>&                   SupplyManager::SupplyObstructedStarlaneTraversals(int empire_id) const
{ return pimpl->SupplyObstructedStarlaneTraversals(empire_id); }

const std::map<int, std::set<int> >&                    SupplyManager::FleetSupplyableSystemIDs() const
{ return pimpl->FleetSupplyableSystemIDs(); }

const std::set<int>&                                    SupplyManager::FleetSupplyableSystemIDs(int empire_id) const
{ return pimpl->FleetSupplyableSystemIDs(empire_id); }

std::set<int>                                           SupplyManager::FleetSupplyableSystemIDs(int empire_id, bool include_allies) const
{ return pimpl->FleetSupplyableSystemIDs(empire_id, include_allies); }

int                                                     SupplyManager::EmpireThatCanSupplyAt(int system_id) const
{ return pimpl->EmpireThatCanSupplyAt(system_id); }

const std::map<int, std::set<std::set<int> > >&         SupplyManager::ResourceSupplyGroups() const
{ return pimpl->ResourceSupplyGroups(); }

const std::set<std::set<int> >&                         SupplyManager::ResourceSupplyGroups(int empire_id) const
{ return pimpl->ResourceSupplyGroups(empire_id); }

const std::map<int, float>&                             SupplyManager::PropagatedSupplyRanges() const
{ return pimpl->PropagatedSupplyRanges(); }

const std::map<int, float>&                             SupplyManager::PropagatedSupplyRanges(int empire_id) const
{ return pimpl->PropagatedSupplyRanges(empire_id); }

const std::map<int, float>&                             SupplyManager::PropagatedSupplyDistances() const
{ return pimpl->PropagatedSupplyDistances(); }

const std::map<int, float>&                             SupplyManager::PropagatedSupplyDistances(int empire_id) const
{ return pimpl->PropagatedSupplyDistances(empire_id); }

bool                                                    SupplyManager::SystemHasFleetSupply(int system_id, int empire_id) const
{ return pimpl->SystemHasFleetSupply(system_id, empire_id); }

bool                                                    SupplyManager::SystemHasFleetSupply(int system_id, int empire_id, bool include_allies) const
{ return pimpl->SystemHasFleetSupply(system_id, empire_id, include_allies); }

std::string                                             SupplyManager::Dump(int empire_id /*= ALL_EMPIRES*/) const
{ return pimpl->Dump(empire_id); }

void                                                    SupplyManager::Update()
{ return pimpl->Update(); }

template <class Archive>
void SupplyManager::serialize(Archive& ar, const unsigned int version)
{ pimpl->serialize(ar, version); }

template void SupplyManager::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void SupplyManager::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void SupplyManager::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void SupplyManager::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);
