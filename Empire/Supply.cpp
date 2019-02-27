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
#include "../util/Logger.h"

#include <boost/graph/connected_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

SupplyManager::SupplyManager() :
    m_supply_starlane_traversals(),
    m_supply_starlane_obstructed_traversals(),
    m_fleet_supplyable_system_ids(),
    m_resource_supply_groups()
{}

SupplyManager& SupplyManager::operator=(const SupplyManager& rhs) {
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
}

const std::map<int, std::set<std::pair<int, int>>>& SupplyManager::SupplyStarlaneTraversals() const
{ return m_supply_starlane_traversals; }

const std::set<std::pair<int, int>>& SupplyManager::SupplyStarlaneTraversals(int empire_id) const {
    auto it = m_supply_starlane_traversals.find(empire_id);
    if (it != m_supply_starlane_traversals.end())
        return it->second;
    return EMPTY_INT_PAIR_SET;
}

const std::map<int, std::set<std::pair<int, int>>>& SupplyManager::SupplyObstructedStarlaneTraversals() const
{ return m_supply_starlane_obstructed_traversals; }

const std::set<std::pair<int, int>>& SupplyManager::SupplyObstructedStarlaneTraversals(int empire_id) const {
    auto it = m_supply_starlane_obstructed_traversals.find(empire_id);
    if (it != m_supply_starlane_obstructed_traversals.end())
        return it->second;
    return EMPTY_INT_PAIR_SET;
}

const std::map<int, std::set<int>>& SupplyManager::FleetSupplyableSystemIDs() const
{ return m_fleet_supplyable_system_ids; }

const std::set<int>& SupplyManager::FleetSupplyableSystemIDs(int empire_id) const {
    auto it = m_fleet_supplyable_system_ids.find(empire_id);
    if (it != m_fleet_supplyable_system_ids.end())
        return it->second;
    return EMPTY_INT_SET;
}

std::set<int> SupplyManager::FleetSupplyableSystemIDs(int empire_id, bool include_allies) const {
    std::set<int> retval = FleetSupplyableSystemIDs(empire_id);
    if (!include_allies)
        return retval;

    // add supplyable systems of all allies
    for (auto empire_id_sys_id_set : m_fleet_supplyable_system_ids) {
        int other_empire_id = empire_id_sys_id_set.first;
        const std::set<int>& systems = empire_id_sys_id_set.second;
        if (systems.empty() || Empires().GetDiplomaticStatus(empire_id, other_empire_id) != DIPLO_ALLIED)
            continue;
        retval.insert(systems.begin(), systems.end());
    }
    return retval;
}

int SupplyManager::EmpireThatCanSupplyAt(int system_id) const {
    for (const auto& entry : m_fleet_supplyable_system_ids) {
        if (entry.second.count(system_id))
            return entry.first;
    }
    return ALL_EMPIRES;
}

const std::map<int, std::set<std::set<int>>>& SupplyManager::ResourceSupplyGroups() const
{ return m_resource_supply_groups; }

const std::set<std::set<int>>& SupplyManager::ResourceSupplyGroups(int empire_id) const {
    auto it = m_resource_supply_groups.find(empire_id);
    if (it != m_resource_supply_groups.end())
        return it->second;
    return EMPTY_INT_SET_SET;
}

const std::map<int, float>& SupplyManager::PropagatedSupplyRanges() const {
    std::cout << "BLAAAAH" << std::endl;
    return m_propagated_supply_ranges;
}

const std::map<int, float>& SupplyManager::PropagatedSupplyRanges(int empire_id) const
{
    auto emp_it = m_empire_propagated_supply_ranges.find(empire_id);
    if (emp_it == m_empire_propagated_supply_ranges.end())
        return EMPTY_INT_FLOAT_MAP;
    return emp_it->second;
}

const std::map<int, float>& SupplyManager::PropagatedSupplyDistances() const {
    std::cout << "GLAARB" << std::endl;
    return m_propagated_supply_distances;
}

const std::map<int, float>& SupplyManager::PropagatedSupplyDistances(int empire_id) const {
    auto emp_it = m_empire_propagated_supply_distances.find(empire_id);
    if (emp_it == m_empire_propagated_supply_distances.end())
        return EMPTY_INT_FLOAT_MAP;
    return emp_it->second;
}

bool SupplyManager::SystemHasFleetSupply(int system_id, int empire_id) const {
    if (system_id == INVALID_OBJECT_ID)
        return false;
    if (empire_id == ALL_EMPIRES)
        return false;
    auto it = m_fleet_supplyable_system_ids.find(empire_id);
    if (it == m_fleet_supplyable_system_ids.end())
        return false;
    const std::set<int>& sys_set = it->second;
    if (sys_set.count(system_id))
        return true;
    return false;
}

bool SupplyManager::SystemHasFleetSupply(int system_id, int empire_id, bool include_allies) const {
    if (!include_allies)
        return SystemHasFleetSupply(system_id, empire_id);
    if (system_id == INVALID_OBJECT_ID)
        return false;
    if (empire_id == ALL_EMPIRES)
        return false;

    std::set<int> empire_ids = Empires().GetEmpireIDsWithDiplomaticStatusWithEmpire(empire_id, DIPLO_ALLIED);
    empire_ids.insert(empire_id);

    for (auto id : empire_ids) {
        auto sys_set_it = m_fleet_supplyable_system_ids.find(id);
        if (sys_set_it == m_fleet_supplyable_system_ids.end())
            continue;
        auto sys_set = sys_set_it->second;
        if (sys_set.count(system_id))
            return true;
    }

    return false;
}

std::string SupplyManager::Dump(int empire_id) const {
    std::string retval;

    try {
        for (const auto& empire_supply : m_fleet_supplyable_system_ids) {
            if (empire_id != ALL_EMPIRES && empire_supply.first != empire_id)
                continue;
            retval += "Supplyable systems for empire " + std::to_string(empire_supply.first) + "\n";
            for (int system_id : empire_supply.second) {
                auto sys = GetSystem(system_id);
                if (!sys)
                    continue;
                retval += "\n" + sys->PublicName(empire_id) + " (" + std::to_string(sys->ID()) + ") ";

                retval += "\nTraversals from here to: ";

                for (const auto& trav : m_supply_starlane_traversals.at(empire_supply.first)) {
                    if (trav.first == sys->ID()) {
                        auto obj = GetUniverseObject(trav.second);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Traversals to here from: ";
                for (const auto& trav : m_supply_starlane_traversals.at(empire_supply.first)) {
                    if (trav.second == sys->ID()) {
                        auto obj = GetUniverseObject(trav.first);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Blocked Traversals from here to: ";
                for (const auto& trav : m_supply_starlane_obstructed_traversals.at(empire_supply.first)) {
                    if (trav.first == sys->ID()) {
                        auto obj = GetUniverseObject(trav.second);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Blocked Traversals to here from: ";
                for (const auto& trav : m_supply_starlane_obstructed_traversals.at(empire_supply.first)) {
                    if (trav.second == sys->ID()) {
                        auto obj = GetUniverseObject(trav.first);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + std::to_string(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

            }
            retval += "\n\n";
        }

        for (const auto& empire_supply : m_resource_supply_groups) {
            retval += "Supply groups for empire " + std::to_string(empire_supply.first) + "\n";
            for (const auto& system_group : empire_supply.second) {
                retval += "group: ";
                for (int system_id : system_group) {
                    auto sys = GetSystem(system_id);
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
        auto obj1 = GetSystem(obj1_id);
        if (!obj1)
            return 0.0f;
        auto obj2 = GetSystem(obj2_id);
        if (!obj2)
            return 0.0f;
        double dx = obj2->X() - obj1->X();
        double dy = obj2->Y() - obj1->Y();
        return static_cast<float>(std::sqrt(dx*dx + dy*dy));
    }
}

void SupplyManager::Update() {
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
    // systems have obstructions to supply propagation for reach empire, and
    // the ranges and obstructions of other empires' supply, as only one empire
    // can supply each system or propagate along each starlane. one empire's
    // propagating supply can push back another's, if the pusher's range is
    // larger.

    // map from empire id to map from system id to range (in starlane jumps)
    // that supply can be propagated out of that system by that empire.
    std::map<int, std::map<int, float>> empire_system_supply_ranges;
    // map from empire id to which systems are obstructed for it for supply
    // propagation
    std::map<int, std::set<int>> empire_supply_unobstructed_systems;
    // map from empire id to map from system id to sum of supply source ranges
    // owned by empire in that in system
    std::map<int, std::map<int, float>> empire_system_supply_range_sums;

    for (const auto& entry : Empires()) {
        const Empire* empire = entry.second;
        empire_system_supply_ranges[entry.first] = empire->SystemSupplyRanges();
        empire_supply_unobstructed_systems[entry.first] = empire->SupplyUnobstructedSystems();

        //std::stringstream ss;
        //for (int system_id : empire_supply_unobstructed_systems[entry.first])
        //{ ss << system_id << ", "; }
        //DebugLogger() << "Empire " << empire->EmpireID() << " unobstructed systems: " << ss.str();
    }
    for (auto empire_id_pair : empire_system_supply_ranges) {
        for (auto sys_id_pair : empire_id_pair.second) {
            empire_system_supply_range_sums[empire_id_pair.first][sys_id_pair.first] =
                EmpireTotalSupplyRangeSumInSystem(empire_id_pair.first, sys_id_pair.first);
        }
    }


    /////
    // probably temporary: additional restriction here for supply propagation
    // but not for general system obstruction as determind within Empire::UpdateSupplyUnobstructedSystems
    /////
    const auto fleets = GetUniverse().Objects().FindObjects<Fleet>();

    for (const auto& entry : Empires()) {
        int empire_id = entry.first;
        const auto& known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(empire_id);
        std::set<int> systems_containing_friendly_fleets;

        for (auto& fleet : fleets) {
            int system_id = fleet->SystemID();
            if (system_id == INVALID_OBJECT_ID || known_destroyed_objects.count(fleet->ID()))
                continue;

            if ((fleet->HasArmedShips() || fleet->HasFighterShips()) && fleet->Aggressive()) {
                if (fleet->OwnedBy(empire_id)) {
                    if (fleet->NextSystemID() == INVALID_OBJECT_ID || fleet->NextSystemID() == fleet->SystemID()) {
                        systems_containing_friendly_fleets.insert(system_id);
                    }
                }
            }
        }

        std::set<int> systems_where_others_have_supply_sources_and_current_empire_doesnt;
        // add all systems where others have supply
        for (auto& empire_supply : empire_system_supply_ranges) {
            if (empire_supply.first == empire_id || empire_supply.first == ALL_EMPIRES)
                continue;

            for (const auto& supply_range : empire_supply.second) {
                if (supply_range.second <= 0.0f)
                    continue;
                systems_where_others_have_supply_sources_and_current_empire_doesnt.insert(supply_range.first);
            }
        }
        // remove systems were this empire has supply
        auto it = empire_system_supply_ranges.find(empire_id);
        if (it != empire_system_supply_ranges.end()) {
            for (const auto& supply_range : it->second) {
                if (supply_range.second <= 0.0f)
                    continue;
                systems_where_others_have_supply_sources_and_current_empire_doesnt.erase(supply_range.first);
            }
        }

        // for systems where others have supply sources and this empire doesn't
        // and where this empire has no fleets...
        // supply is obstructed
        for (int system_id : systems_where_others_have_supply_sources_and_current_empire_doesnt) {
            if (!systems_containing_friendly_fleets.count(system_id))
                empire_supply_unobstructed_systems[empire_id].erase(system_id);
        }
    }
    /////
    // end probably temporary...
    /////


    // system connections each empire can see / use for supply propagation
    std::map<int, std::map<int, std::set<int>>> empire_visible_starlanes;
    for (auto& entry : Empires()) {
        const Empire* empire = entry.second;
        empire_visible_starlanes[entry.first] = empire->KnownStarlanes();//  VisibleStarlanes();
    }

    std::set<int> systems_with_supply_in_them;

    // store (supply range in jumps, and distance to supply source) of all
    // unobstructed systems before propagation, and add to list of systems
    // to propagate from.
    std::map<int, std::map<int, std::pair<float, float>>> empire_propagating_supply_ranges;
    float max_range = 0.0f;

    for (const auto& empire_supply : empire_system_supply_ranges) {
        int empire_id = empire_supply.first;
        const std::set<int>& unobstructed_systems = empire_supply_unobstructed_systems[empire_id];

        for (const auto& supply_range : empire_supply.second) {
            int system_id = supply_range.first;
            if (unobstructed_systems.count(system_id)) {
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
        for (const auto& empire_supply : empire_propagating_supply_ranges) {
            for (const auto& supply_range : empire_supply.second)
            { systems_with_supply_in_them.insert(supply_range.first); }
        }


        // resolve supply fights between multiple empires in one system.
        // pass over all empire-supplied systems, removing supply for all
        // but the empire with the highest supply range in each system
        for (int sys_id : systems_with_supply_in_them) {
            // sort empires by range in this system
            std::map<float, std::set<int>> empire_ranges_here;
            for (auto& empire_supply : empire_propagating_supply_ranges) {
                int empire_id = empire_supply.first;
                auto empire_supply_it = empire_supply.second.find(sys_id);
                // does this empire have any range in this system? if so, store it
                if (empire_supply_it == empire_supply.second.end())
                    continue;

                // stuff to break ties...
                float bonus = 0.0f;

                // empires with planets in system
                bool has_outpost = false, has_colony = false;
                if (auto sys = GetSystem(sys_id)) {
                    std::vector<int> obj_ids;
                    std::copy(sys->ContainedObjectIDs().begin(), sys->ContainedObjectIDs().end(), std::back_inserter(obj_ids));
                    for (auto& obj : Objects().FindObjects(obj_ids)) {
                        if (!obj)
                            continue;
                        if (!obj->OwnedBy(empire_id))
                            continue;
                        if (obj->ObjectType() == OBJ_PLANET) {
                            if (auto planet = std::dynamic_pointer_cast<Planet>(obj)) {
                                if (!planet->SpeciesName().empty())
                                    has_colony = true;
                                else
                                    has_outpost = true;
                                continue;
                            }
                        }
                    }
                }
                if (has_colony)
                    bonus += 0.5f;
                else if (has_outpost)
                    bonus += 0.3f;

                // sum of all supply sources in this system
                bonus += empire_system_supply_range_sums[empire_id][sys_id] / 1000.0f;

                // distance to supply source from here
                float propagated_distance_to_supply_source = std::max(1.0f, empire_supply_it->second.second);
                bonus += 0.0001f / propagated_distance_to_supply_source;

                // store ids of empires indexed by adjusted propgated range, in order to sort by range
                float propagated_range = empire_supply_it->second.first;
                empire_ranges_here[propagated_range + bonus].insert(empire_id);
            }

            if (empire_ranges_here.empty())
                continue;   // no empire has supply here?
            if (empire_ranges_here.size() == 1 && empire_ranges_here.begin()->second.size() < 2)
                continue;   // only one empire has supply here

            // at least two empires have supply sources here...
            // check if one is stronger

            // remove supply for all empires except the top-ranged empire here
            // if there is a tie for top-ranged, remove all
            auto range_empire_it = empire_ranges_here.rbegin();
            int top_range_empire_id = ALL_EMPIRES;
            if (range_empire_it->second.size() == 1) {
                // if just one empire has the most range, it is the top empire
                top_range_empire_id = *(range_empire_it->second.begin());
            }
            //DebugLogger() << "top ranged empire here: " << top_range_empire_id;

            // remove range entries and traversals for all but the top empire
            // (or all empires if there is no single top empire)
            for (auto& empire_supply : empire_propagating_supply_ranges) {
                int empire_id = empire_supply.first;
                if (empire_id == top_range_empire_id)
                    continue;   // this is the top empire, so leave as the sole empire supplying here

                // remove from range entry...
                auto& empire_ranges = empire_supply.second;
                empire_ranges.erase(sys_id);

                //DebugLogger() << "... removed empire " << empire_id << " system " << sys_id << " supply.";

                // Remove from unobstructed systems
                empire_supply_unobstructed_systems[empire_id].erase(sys_id);

                auto& lane_traversals = m_supply_starlane_traversals[empire_id];
                auto lane_traversals_initial = lane_traversals;
                auto& obstructed_traversals = m_supply_starlane_obstructed_traversals[empire_id];
                auto obstrcuted_traversals_initial = obstructed_traversals;

                // remove from traversals departing from or going to this system for this empire,
                // and set any traversals going to this system as obstructed
                for (const auto& lane : lane_traversals_initial) {
                    if (lane.first == sys_id) {
                        lane_traversals.erase({sys_id, lane.second});
                    }
                    if (lane.second == sys_id) {
                        lane_traversals.erase({lane.first, sys_id});
                        obstructed_traversals.insert({lane.first, sys_id});
                    }
                }

                // remove obstructed traverals departing from this system
                for (const auto& lane : obstrcuted_traversals_initial) {
                    if (lane.first == sys_id)
                        obstructed_traversals.erase({lane.first, lane.second});
                }
            }

            //// DEBUG
            //DebugLogger() << "after culling empires ranges at system " << sys_id << ":";
            //for (auto& empire_supply : empire_propagating_supply_ranges) {
            //    auto& system_ranges = empire_supply.second;
            //    auto range_it = system_ranges.find(sys_id);
            //    if (range_it != system_ranges.end())
            //        DebugLogger() << empire_supply.first << " : " << range_it->second;
            //}
            //// END DEBUG
        }

        if (range_to_spread <= 0)
            break;

        // initialize next iteration with current supply distribution
        auto empire_propagating_supply_ranges_next = empire_propagating_supply_ranges;


        // for sources of supply of at least the minimum range for this
        // iteration that are in the current map, give adjacent systems one
        // less supply in the next iteration (unless as much or more is already
        // there)
        for (const auto& empire_supply : empire_propagating_supply_ranges) {
            int empire_id = empire_supply.first;
            //DebugLogger() << ">-< Doing supply propagation for empire " << empire_id << " >-<";
            const auto& prev_sys_ranges = empire_supply.second;
            const auto& unobstructed_systems = empire_supply_unobstructed_systems[empire_id];

            for (const auto& supply_range : empire_supply.second) {
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
                    if (!unobstructed_systems.count(lane_end_sys_id)) {
                        // propagation obstructed!
                        //DebugLogger() << "Added obstructed traversal from " << system_id << " to " << lane_end_sys_id << " due to not being on unobstructed systems";
                        m_supply_starlane_obstructed_traversals[empire_id].insert({system_id, lane_end_sys_id});
                        continue;
                    }
                    // propagation not obstructed.


                    // does another empire already have as much or more supply here from a previous iteration?
                    float other_empire_biggest_range = -10000.0f;   // arbitrary big numbeer
                    for (const auto& other_empire_supply : empire_propagating_supply_ranges) {
                        int other_empire_id = other_empire_supply.first;
                        if (other_empire_id == empire_id)
                            continue;
                        const auto& prev_other_empire_sys_ranges = other_empire_supply.second;
                        auto prev_other_empire_range_it = prev_other_empire_sys_ranges.find(lane_end_sys_id);
                        if (prev_other_empire_range_it == prev_other_empire_sys_ranges.end())
                            continue;
                        if (prev_other_empire_range_it->second.first > other_empire_biggest_range)
                            other_empire_biggest_range = prev_other_empire_range_it->second.first;
                    }

                    // if so, add a blocked traversal and continue
                    if (range_after_one_more_jump <= other_empire_biggest_range) {
                        m_supply_starlane_obstructed_traversals[empire_id].insert({system_id, lane_end_sys_id});
                        //DebugLogger() << "Added obstructed traversal from " << system_id << " to " << lane_end_sys_id << " due to other empire biggest range being " << other_empire_biggest_range;
                        continue;
                    }

                    // otherwise, propagate into system...
                    float lane_length = DistanceBetweenObjects(system_id, lane_end_sys_id);
                    float distance_to_supply_source_after_next_lane = lane_length + distance_to_supply_source;

                    //DebugLogger() << "Attempting to propagate into system: " << lane_end_sys_id << " the new range: " << range_after_one_more_jump << " and distance: " << distance_to_supply_source_after_next_lane;

                    // if propagating supply would increase the range of the adjacent system,
                    // or decrease the distance to the adjacent system from a supply source...
                    auto prev_range_it = prev_sys_ranges.find(lane_end_sys_id);
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
                    // always record a traversal, so connectivity is calculated properly
                    m_supply_starlane_traversals[empire_id].insert({system_id, lane_end_sys_id});
                    //DebugLogger() << "Added traversal from " << system_id << " to " << lane_end_sys_id;

                    // erase any previous obstructed traversal that just succeeded
                    if (m_supply_starlane_obstructed_traversals[empire_id].count({system_id, lane_end_sys_id}))
                    {
                        //DebugLogger() << "Removed obstructed traversal from " << system_id << " to " << lane_end_sys_id;
                        m_supply_starlane_obstructed_traversals[empire_id].erase({system_id, lane_end_sys_id});
                    }
                    if (m_supply_starlane_obstructed_traversals[empire_id].count({lane_end_sys_id, system_id}))
                    {
                        //DebugLogger() << "Removed obstructed traversal from " << lane_end_sys_id << " to " << system_id;
                        m_supply_starlane_obstructed_traversals[empire_id].erase({lane_end_sys_id, system_id});
                    }
                }
            }
        }

        // save propagated results for next iteration
        empire_propagating_supply_ranges = std::move(empire_propagating_supply_ranges_next);
    }

    //// DEBUG
    //DebugLogger() << "SuppolyManager::Update: after removing conflicts, empires can provide supply to the following system ids:";
    //for (auto& empire_supply : empire_propagating_supply_ranges) {
    //    int empire_id = empire_supply.first;
    //    std::stringstream ss;
    //    for (auto& supply_range : empire_supply.second) {
    //        ss << supply_range.first << " (" << supply_range.second << "),  ";
    //    }
    //    DebugLogger() << "empire: " << empire_id << ":  " << ss.str();
    //}
    //// END DEBUG

    // record which systems are fleet supplyable by each empire (after resolving conflicts in each system)
    for (const auto& empire_supply : empire_propagating_supply_ranges) {
        int empire_id = empire_supply.first;
        for (const auto& supply_range : empire_supply.second) {
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



    // TEST STUFF FOR INTER-EMPIRE-MERGING
    auto ally_merged_supply_starlane_traversals = m_supply_starlane_traversals;

    // add connections into allied empire systems when their obstructed lane
    // traversals originate on either end of a starlane
    for (auto& empire_set : m_supply_starlane_obstructed_traversals) {
        // input:
        const auto& empire_obstructed_traversals = empire_set.second;
        // output:
        auto& empire_supply_traversals = ally_merged_supply_starlane_traversals[empire_set.first];


        std::set<int> allies_of_empire = Empires().GetEmpireIDsWithDiplomaticStatusWithEmpire(empire_set.first, DIPLO_ALLIED);
        for (int ally_id : allies_of_empire) {
            // input:
            auto const& ally_obstructed_traversals = m_supply_starlane_obstructed_traversals[ally_id];

            // find cases where outer loop empire has an obstructed traversal from A to B
            // and inner loop empire has an obstructed traversal from B to A
            for (auto const& empire_trav : empire_obstructed_traversals) {
                for (auto const& ally_trav : ally_obstructed_traversals) {
                    if (empire_trav.first == ally_trav.second && empire_trav.second == ally_trav.first) {
                        empire_supply_traversals.insert({empire_trav.first, empire_trav.second});
                        empire_supply_traversals.insert({empire_trav.second, empire_trav.first});
                    }
                }
            }
        }
    }

    // add allied supply starlane traversals to empires' traversals, so that
    // allies can use eachothers' supply networks
    for (auto& empire_set : ally_merged_supply_starlane_traversals) {
        auto& output_empire_traversals = empire_set.second;
        for (int ally_id : Empires().GetEmpireIDsWithDiplomaticStatusWithEmpire(empire_set.first, DIPLO_ALLIED)) {
            // copy ally traversals into the output empire traversals set
            for (const auto& traversal_pair : m_supply_starlane_traversals[ally_id])
                output_empire_traversals.insert(traversal_pair);
        }
    }

    // same for fleet supplyable system ids, as these are added to supplyable
    // groups in following code
    auto ally_merged_fleet_supplyable_system_ids = m_fleet_supplyable_system_ids;
    for (auto& empire_set : ally_merged_fleet_supplyable_system_ids) {
        std::set<int>& output_empire_ids = empire_set.second;
        for (int ally_id : Empires().GetEmpireIDsWithDiplomaticStatusWithEmpire(empire_set.first, DIPLO_ALLIED)) {
            // copy ally traversals into the output empire traversals set
            for (int sys_id : m_fleet_supplyable_system_ids[ally_id])
                output_empire_ids.insert(sys_id);
        }
    }

    // END TEST STUFF FOR INTER-EMPIRE-MERGING



    // determine supply-connected groups of systems for each empire.
    // need to merge interconnected supply groups into as few sets of mutually-
    // supply-exchanging systems as possible.  This requires finding the
    // connected components of an undirected graph, where the node
    // adjacency are the directly-connected systems determined above.
    for (const auto& empire_supply : empire_propagating_supply_ranges) {
        int empire_id = empire_supply.first;

        // assemble all direct connections between systems from remaining traversals
        std::map<int, std::set<int>> supply_groups_map;
        for (auto const& lane : ally_merged_supply_starlane_traversals[empire_id]) {
            supply_groups_map[lane.first].insert(lane.second);
            supply_groups_map[lane.second].insert(lane.first);
        }

        // also add connections from all fleet-supplyable systems to themselves, so that
        // any fleet supply able system with no connection to another system can still
        // have resource sharing within tiself
        for (int system_id : ally_merged_fleet_supplyable_system_ids[empire_id])
            supply_groups_map[system_id].insert(system_id);


        if (supply_groups_map.empty())
            continue;

        //DebugLogger() << "Empire " << empire_id << " supply groups map:";
        //for (auto const& q : supply_groups_map) {
        //    int source_id = q.first;

        //    std::stringstream other_ids;
        //    for (auto const& r : q.second)
        //    { other_ids << r << ", "; }

        //    DebugLogger() << " ... src: " << source_id << " to: " << other_ids.str();
        //}


        // create graph
        boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> graph;

        // boost expects vertex labels to range from 0 to num vertices - 1, so need
        // to map from system id to graph id and back when accessing vertices
        std::vector<int> graph_id_to_sys_id;
        graph_id_to_sys_id.reserve(supply_groups_map.size());

        std::map<int, int> sys_id_to_graph_id;
        int graph_id = 0;
        for (auto& supply_group : supply_groups_map) {
            int sys_id = supply_group.first;
            boost::add_vertex(graph);   // should add with index = graph_id

            graph_id_to_sys_id.push_back(sys_id);
            sys_id_to_graph_id[sys_id] = graph_id;
            ++graph_id;
        }

        // add edges for all direct connections between systems
        // and add edges from fleet supplyable systems to themselves
        for (auto& supply_group : supply_groups_map) {
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
        for (auto& component_set : component_sets_map)
            m_resource_supply_groups[empire_id].insert(component_set.second);
    }
}
