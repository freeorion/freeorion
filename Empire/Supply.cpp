#include "Supply.h"

#include "EmpireManager.h"
#include "Empire.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Fleet.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"

#include <boost/graph/connected_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/lexical_cast.hpp>

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
    static const std::set<std::set<int> > EMPTY_INT_SET_SET;
    static const std::set<std::pair<int, int> > EMPTY_INT_PAIR_SET;
}

const std::map<int, std::set<std::pair<int, int> > >& SupplyManager::SupplyStarlaneTraversals() const
{ return m_supply_starlane_traversals; }

const std::set<std::pair<int, int> >& SupplyManager::SupplyStarlaneTraversals(int empire_id) const {
    std::map<int, std::set<std::pair<int, int> > >::const_iterator it = m_supply_starlane_traversals.find(empire_id);
    if (it != m_supply_starlane_traversals.end())
        return it->second;
    return EMPTY_INT_PAIR_SET;
}

const std::map<int, std::set<std::pair<int, int> > >& SupplyManager::SupplyObstructedStarlaneTraversals() const
{ return m_supply_starlane_obstructed_traversals; }

const std::set<std::pair<int, int> >& SupplyManager::SupplyObstructedStarlaneTraversals(int empire_id) const {
    std::map<int, std::set<std::pair<int, int> > >::const_iterator it = m_supply_starlane_obstructed_traversals.find(empire_id);
    if (it != m_supply_starlane_obstructed_traversals.end())
        return it->second;
    return EMPTY_INT_PAIR_SET;
}

const std::map<int, std::set<int> >& SupplyManager::FleetSupplyableSystemIDs() const
{ return m_fleet_supplyable_system_ids; }

const std::set<int>& SupplyManager::FleetSupplyableSystemIDs(int empire_id) const {
    std::map<int, std::set<int> >::const_iterator it = m_fleet_supplyable_system_ids.find(empire_id);
    if (it != m_fleet_supplyable_system_ids.end())
        return it->second;
    return EMPTY_INT_SET;
}

int SupplyManager::EmpireThatCanSupplyAt(int system_id) const {
    for (std::map<int, std::set<int> >::const_iterator it = m_fleet_supplyable_system_ids.begin();
         it != m_fleet_supplyable_system_ids.end(); ++it)
    {
        if (it->second.find(system_id) != it->second.end())
            return it->first;
    }
    return ALL_EMPIRES;
}

const std::map<int, std::set<std::set<int> > >& SupplyManager::ResourceSupplyGroups() const
{ return m_resource_supply_groups; }

const std::set<std::set<int> >& SupplyManager::ResourceSupplyGroups(int empire_id) const {
    std::map<int, std::set<std::set<int> > >::const_iterator it = m_resource_supply_groups.find(empire_id);
    if (it != m_resource_supply_groups.end())
        return it->second;
    return EMPTY_INT_SET_SET;
}

const std::map<int, float>& SupplyManager::PropagatedSupplyRanges() const
{ return m_propagated_supply_ranges; }

bool SupplyManager::SystemHasFleetSupply(int system_id, int empire_id) const {
    if (system_id == INVALID_OBJECT_ID)
        return false;
    if (empire_id == ALL_EMPIRES)
        return false;
    std::map<int, std::set<int> >::const_iterator it = m_fleet_supplyable_system_ids.find(empire_id);
    if (it == m_fleet_supplyable_system_ids.end())
        return false;
    const std::set<int>& sys_set = it->second;
    if (sys_set.find(system_id) != sys_set.end())
        return true;
    return false;
}

std::string SupplyManager::Dump(int empire_id) const {
    std::string retval;

    try {
        for (std::map<int, std::set<int> >::const_iterator empire_it = m_fleet_supplyable_system_ids.begin();
             empire_it != m_fleet_supplyable_system_ids.end(); ++empire_it)
        {
            if (empire_id != ALL_EMPIRES && empire_it->first != empire_id)
                continue;
            retval += "Supplyable systems for empire " + boost::lexical_cast<std::string>(empire_it->first) + "\n";
            for (std::set<int>::const_iterator set_it = empire_it->second.begin();
                 set_it != empire_it->second.end(); ++set_it)
            {
                TemporaryPtr<const System> sys = GetSystem(*set_it);
                if (!sys)
                    continue;
                retval += "\n" + sys->PublicName(empire_id) + " (" + boost::lexical_cast<std::string>(sys->ID()) + ") ";

                retval += "\nTraversals from here to: ";
                const std::set<std::pair<int, int> >& traversals = m_supply_starlane_traversals.at(empire_it->first);
                for (std::set<std::pair<int, int> >::const_iterator trav_it = traversals.begin();
                     trav_it != traversals.end(); ++trav_it)
                {
                    if (trav_it->first == sys->ID()) {
                        TemporaryPtr<const UniverseObject> obj = GetUniverseObject(trav_it->second);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + boost::lexical_cast<std::string>(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Traversals to here from: ";
                for (std::set<std::pair<int, int> >::const_iterator trav_it = traversals.begin();
                     trav_it != traversals.end(); ++trav_it)
                {
                    if (trav_it->second == sys->ID()) {
                        TemporaryPtr<const UniverseObject> obj = GetUniverseObject(trav_it->first);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + boost::lexical_cast<std::string>(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Blocked Traversals from here to: ";
                const std::set<std::pair<int, int> >& otraversals = m_supply_starlane_obstructed_traversals.at(empire_it->first);
                for (std::set<std::pair<int, int> >::const_iterator trav_it = otraversals.begin();
                     trav_it != otraversals.end(); ++trav_it)
                {
                    if (trav_it->first == sys->ID()) {
                        TemporaryPtr<const UniverseObject> obj = GetUniverseObject(trav_it->second);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + boost::lexical_cast<std::string>(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

                retval += "Blocked Traversals to here from: ";
                for (std::set<std::pair<int, int> >::const_iterator trav_it = otraversals.begin();
                     trav_it != otraversals.end(); ++trav_it)
                {
                    if (trav_it->second == sys->ID()) {
                        TemporaryPtr<const UniverseObject> obj = GetUniverseObject(trav_it->first);
                        if (obj)
                            retval += obj->PublicName(empire_id) + " (" + boost::lexical_cast<std::string>(obj->ID()) + ")  ";
                    }
                }
                retval += "\n";

            }
            retval += "\n\n";
        }

        for (std::map<int, std::set<std::set<int> > >::const_iterator empire_it = m_resource_supply_groups.begin();
             empire_it != m_resource_supply_groups.end(); ++empire_it)
        {
            retval += "Supply groups for empire " + boost::lexical_cast<std::string>(empire_it->first) + "\n";
            for (std::set<std::set<int> >::const_iterator set_set_it = empire_it->second.begin();
                 set_set_it != empire_it->second.end(); ++set_set_it)
            {
                retval += "group: ";
                for (std::set<int>::const_iterator set_it = set_set_it->begin();
                     set_it != set_set_it->end(); ++set_it)
                {
                    TemporaryPtr<const System> sys = GetSystem(*set_it);
                    if (!sys)
                        continue;
                    retval += "\n" + sys->PublicName(empire_id) + " (" + boost::lexical_cast<std::string>(sys->ID()) + ") ";
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
    std::map<int, std::map<int, float> > empire_system_supply_ranges;
    // map from empire id to which systems are obstructed for it for supply
    // propagation
    std::map<int, std::set<int> > empire_supply_unobstructed_systems;

    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        const Empire* empire = it->second;
        empire_system_supply_ranges[it->first] = empire->SystemSupplyRanges();
        empire_supply_unobstructed_systems[it->first] = empire->SupplyUnobstructedSystems();

        //std::stringstream ss;
        //for (std::set<int>::const_iterator sys_it = empire_supply_unobstructed_systems[it->first].begin();
        //     sys_it != empire_supply_unobstructed_systems[it->first].end(); ++sys_it)
        //{ ss << *sys_it << ", "; }
        //DebugLogger() << "Empire " << empire->EmpireID() << " unobstructed systems: " << ss.str();
    }


    /////
    // probably temporary: additional restriction here for supply propagation
    // but not for general system obstruction as determind within Empire::UpdateSupplyUnobstructedSystems
    /////
    const std::vector<TemporaryPtr<Fleet> > fleets = GetUniverse().Objects().FindObjects<Fleet>();

    for (EmpireManager::const_iterator empire_it = Empires().begin(); empire_it != Empires().end(); ++empire_it) {
        int empire_id = empire_it->first;
        const std::set<int>& known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(empire_id);
        std::set<int> systems_containing_friendly_fleets;

        for (std::vector<TemporaryPtr<Fleet> >::const_iterator it = fleets.begin(); it != fleets.end(); ++it) {
            TemporaryPtr<const Fleet> fleet = *it;
            int system_id = fleet->SystemID();
            if (system_id == INVALID_OBJECT_ID) {
                continue;   // not in a system, so can't affect system obstruction
            } else if (known_destroyed_objects.find(fleet->ID()) != known_destroyed_objects.end()) {
                continue; //known to be destroyed so can't affect supply, important just in case being updated on client side
            }

            if (fleet->HasArmedShips() && fleet->Aggressive()) {
                if (fleet->OwnedBy(empire_id)) {
                    if (fleet->NextSystemID() == INVALID_OBJECT_ID || fleet->NextSystemID() == fleet->SystemID()) {
                        systems_containing_friendly_fleets.insert(system_id);
                    }
                }
            }
        }

        std::set<int> systems_where_others_have_supply_sources_and_current_empire_doesnt;
        // add all systems where others have supply
        for (std::map<int, std::map<int, float> >::const_iterator it = empire_system_supply_ranges.begin();
             it != empire_system_supply_ranges.end(); ++it)
        {
            if (it->first == empire_id || it->first == ALL_EMPIRES)
                continue;

            const std::map<int, float>& system_supply_ranges = it->second;
            for (std::map<int, float>::const_iterator sys_it = system_supply_ranges.begin();
                 sys_it != system_supply_ranges.end(); ++sys_it)
            {
                if (sys_it->second <= 0.0f)
                    continue;
                systems_where_others_have_supply_sources_and_current_empire_doesnt.insert(sys_it->first);
            }
        }
        // remove systems were this empire has supply
        std::map<int, std::map<int, float> >::const_iterator it = empire_system_supply_ranges.find(empire_id);
        if (it != empire_system_supply_ranges.end()) {
            const std::map<int, float>& system_supply_ranges = it->second;
            for (std::map<int, float>::const_iterator sys_it = system_supply_ranges.begin();
                 sys_it != system_supply_ranges.end(); ++sys_it)
            {
                if (sys_it->second <= 0.0f)
                    continue;
                systems_where_others_have_supply_sources_and_current_empire_doesnt.erase(sys_it->first);
            }
        }

        // for systems where others have supply sources and this empire doesn't
        // and where this empire has no fleets...
        // supply is obstructed
        for (std::set<int>::const_iterator sys_it = systems_where_others_have_supply_sources_and_current_empire_doesnt.begin();
             sys_it != systems_where_others_have_supply_sources_and_current_empire_doesnt.end(); ++sys_it)
        {
            if (systems_containing_friendly_fleets.find(*sys_it) == systems_containing_friendly_fleets.end())
                empire_supply_unobstructed_systems[empire_id].erase(*sys_it);
        }
    }
    /////
    // end probably temporary...
    /////


    // system connections each empire can see / use for supply propagation
    std::map<int, std::map<int, std::set<int> > > empire_visible_starlanes;
    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        const Empire* empire = it->second;
        empire_visible_starlanes[it->first] = empire->KnownStarlanes();//  VisibleStarlanes();
    }

    std::set<int> systems_with_supply_in_them;

    // store supply range in jumps of all unobstructed systems before
    // propagation, and add to list of systems to propagate from.
    std::map<int, std::map<int, float> > empire_propagating_supply_ranges;
    float max_range = 0.0f;
    for (std::map<int, std::map<int, float> >::const_iterator empire_it = empire_system_supply_ranges.begin();
         empire_it != empire_system_supply_ranges.end(); ++empire_it)
    {
        int empire_id = empire_it->first;
        const std::map<int, float>& system_supply_ranges = empire_it->second;
        const std::set<int>& unobstructed_systems = empire_supply_unobstructed_systems[empire_id];

        for (std::map<int, float>::const_iterator it = system_supply_ranges.begin();
             it != system_supply_ranges.end(); ++it)
        {
            int system_id = it->first;
            if (unobstructed_systems.find(system_id) != unobstructed_systems.end()) {
                empire_propagating_supply_ranges[empire_id][system_id] = it->second;
                if (it->second > max_range)
                    max_range = it->second;
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
        for (std::map<int, std::map<int, float> >::const_iterator empire_it = empire_propagating_supply_ranges.begin();
             empire_it != empire_propagating_supply_ranges.end(); ++empire_it)
        {
            for (std::map<int, float>::const_iterator sys_it = empire_it->second.begin();
                 sys_it != empire_it->second.end(); ++sys_it)
            { systems_with_supply_in_them.insert(sys_it->first); }
        }


        // resolve supply fights between multiple empires in one system.
        // pass over all empire-supplied systems, removing supply for all
        // but the empire with the highest supply range in each system
        for (std::set<int>::const_iterator sys_it = systems_with_supply_in_them.begin();
             sys_it != systems_with_supply_in_them.end(); ++sys_it)
        {
            int sys_id = *sys_it;

            // sort empires by range in this system
            std::map<float, std::set<int> > empire_ranges_here;
            for (std::map<int, std::map<int, float> >::const_iterator empire_it = empire_propagating_supply_ranges.begin();
                 empire_it != empire_propagating_supply_ranges.end(); ++empire_it)
            {
                int empire_id = empire_it->first;
                std::map<int, float>::const_iterator empire_supply_it = empire_it->second.find(sys_id);
                // does this empire have any range in this system? if so, store it
                if (empire_supply_it == empire_it->second.end())
                    continue;

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
                if (TemporaryPtr<const System> sys = GetSystem(sys_id)) {
                    std::vector<int> obj_ids;
                    std::copy(sys->ContainedObjectIDs().begin(), sys->ContainedObjectIDs().end(), std::back_inserter(obj_ids));
                    std::vector<TemporaryPtr<UniverseObject> > sys_objs = Objects().FindObjects(obj_ids);
                    for (std::vector<TemporaryPtr<UniverseObject> >::iterator obj_it = sys_objs.begin();
                         obj_it != sys_objs.end(); ++obj_it)
                    {
                        TemporaryPtr<UniverseObject> obj = *obj_it;
                        if (!obj)
                            continue;
                        if (!obj->OwnedBy(empire_id))
                            continue;
                        if (obj->ObjectType() == OBJ_SHIP) {
                            has_ship = true;
                            continue;
                        }
                        if (obj->ObjectType() == OBJ_PLANET) {
                            if (TemporaryPtr<Planet> planet = boost::dynamic_pointer_cast<Planet>(obj)) {
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

                // todo: other bonuses?

                empire_ranges_here[empire_supply_it->second].insert(empire_it->first + bonus);
            }

            if (empire_ranges_here.empty())
                continue;   // no empire has supply here?
            if (empire_ranges_here.size() == 1 && empire_ranges_here.begin()->second.size() < 2)
                continue;   // only one empire has supply here

            // remove supply for all empires except the top-ranged empire here
            // if there is a tie for top-ranged, remove all
            std::map<float, std::set<int> >::reverse_iterator range_empire_it = empire_ranges_here.rbegin();
            int top_range_empire_id = ALL_EMPIRES;
            if (range_empire_it->second.size() == 1)
                top_range_empire_id = *(range_empire_it->second.begin());
            //DebugLogger() << "top ranged empire here: " << top_range_empire_id;

            // remove range entries and traversals for all but the top empire
            // (or all empires if there is no single top empire)
            for (std::map<int, std::map<int, float> >::iterator empire_it = empire_propagating_supply_ranges.begin();
                 empire_it != empire_propagating_supply_ranges.end(); ++empire_it)
            {
                int empire_id = empire_it->first;
                if (empire_id == top_range_empire_id)
                    continue;   // this is the top empire, so leave as the sole empire supplying here

                // remove from range entry...
                std::map<int, float>& empire_ranges = empire_it->second;
                empire_ranges.erase(sys_id);

                //DebugLogger() << "... removed empire " << empire_id << " system " << sys_id << " supply.";

                // Remove from unobstructed systems
                empire_supply_unobstructed_systems[empire_id].erase(sys_id);

                std::set<std::pair<int, int> >& lane_traversals = m_supply_starlane_traversals[empire_id];
                std::set<std::pair<int, int> > lane_traversals_initial = lane_traversals;
                std::set<std::pair<int, int> >& obstructed_traversals = m_supply_starlane_obstructed_traversals[empire_id];
                std::set<std::pair<int, int> > obstrcuted_traversals_initial = obstructed_traversals;

                // remove from traversals departing from or going to this system for this empire,
                // and set any traversals going to this system as obstructed
                for (std::set<std::pair<int, int> >::iterator traversals_it = lane_traversals_initial.begin();
                     traversals_it != lane_traversals_initial.end(); ++traversals_it)
                {
                    if (traversals_it->first == sys_id) {
                        lane_traversals.erase(std::make_pair(sys_id, traversals_it->second));
                    }
                    if (traversals_it->second == sys_id) {
                        lane_traversals.erase(std::make_pair(traversals_it->first, sys_id));
                        obstructed_traversals.insert(std::make_pair(traversals_it->first, sys_id));
                    }
                }

                // remove obstructed traverals departing from this system
                for (std::set<std::pair<int, int> >::iterator traversals_it = obstrcuted_traversals_initial.begin();
                     traversals_it != obstrcuted_traversals_initial.end(); ++traversals_it)
                {
                    if (traversals_it->first == sys_id)
                        obstructed_traversals.erase(std::make_pair(traversals_it->first, traversals_it->second));
                }
            }

            //// DEBUG
            //DebugLogger() << "after culling empires ranges at system " << sys_id << ":";
            //for (std::map<int, std::map<int, float> >::iterator empire_it = empire_propagating_supply_ranges.begin();
            //     empire_it != empire_propagating_supply_ranges.end(); ++empire_it)
            //{
            //    std::map<int, float>& system_ranges = empire_it->second;
            //    std::map<int, float>::iterator range_it = system_ranges.find(sys_id);
            //    if (range_it != system_ranges.end())
            //        DebugLogger() << empire_it->first << " : " << range_it->second;
            //}
            //// END DEBUG
        }

        if (range_to_spread <= 0)
            break;

        // initialize next iteration with current supply distribution
        std::map<int, std::map<int, float> > empire_propagating_supply_ranges_next = empire_propagating_supply_ranges;


        // for sources of supply of at least the minimum range for this
        // iteration that are in the current map, give adjacent systems one
        // less supply in the next iteration (unless as much or more is already
        // there)
        for (std::map<int, std::map<int, float> >::const_iterator prev_empire_it = empire_propagating_supply_ranges.begin();
             prev_empire_it != empire_propagating_supply_ranges.end(); ++prev_empire_it)
        {
            int empire_id = prev_empire_it->first;
            //DebugLogger() << ">-< Doing supply propagation for empire " << empire_id << " >-<";
            const std::map<int, float>& prev_sys_ranges = prev_empire_it->second;
            const std::set<int>& unobstructed_systems = empire_supply_unobstructed_systems[empire_id];

            for (std::map<int, float>::const_iterator prev_sys_it = prev_sys_ranges.begin();
                 prev_sys_it != prev_sys_ranges.end(); ++prev_sys_it)
            {
                // does the source system has enough supply range to propagate outwards?
                float range = prev_sys_it->second;
                if (range != range_to_spread)
                    continue;
                float range_after_one_more_jump = range - 1.0f; // what to set adjacent systems' ranges to (at least)

                // what starlanes can be used to propagate supply?
                int system_id = prev_sys_it->first;
                const std::set<int>& adjacent_systems = empire_visible_starlanes[empire_id][system_id];

                //DebugLogger() << "propagating from system " << system_id << " which has range: " << range;

                // attempt to propagate to all adjacent systems...
                for (std::set<int>::const_iterator lane_it = adjacent_systems.begin();
                     lane_it != adjacent_systems.end(); ++lane_it)
                {
                    int lane_end_sys_id = *lane_it;
                    // is propagation to the adjacent system obstructed?
                    if (unobstructed_systems.find(lane_end_sys_id) == unobstructed_systems.end()) {
                        // propagation obstructed!
                        //DebugLogger() << "Added obstructed traversal from " << system_id << " to " << lane_end_sys_id << " due to not being on unobstructed systems";
                        m_supply_starlane_obstructed_traversals[empire_id].insert(std::make_pair(system_id, lane_end_sys_id));
                        continue;
                    }
                    // propagation not obstructed.


                    // does another empire already have as much or more supply here from a previous iteration?
                    float other_empire_biggest_range = -10000.0f;   // arbitrary big numbeer
                    for (std::map<int, std::map<int, float> >::const_iterator prev_other_empire_it = empire_propagating_supply_ranges.begin();
                         prev_other_empire_it != empire_propagating_supply_ranges.end(); ++prev_other_empire_it)
                    {
                        int other_empire_id = prev_other_empire_it->first;
                        if (other_empire_id == empire_id)
                            continue;
                        const std::map<int, float>& prev_other_empire_sys_ranges = prev_other_empire_it->second;
                        std::map<int, float>::const_iterator prev_other_empire_range_it = prev_other_empire_sys_ranges.find(lane_end_sys_id);
                        if (prev_other_empire_range_it == prev_other_empire_sys_ranges.end())
                            continue;
                        if (prev_other_empire_range_it->second > other_empire_biggest_range)
                            other_empire_biggest_range = prev_other_empire_range_it->second;
                    }

                    // if so, add a blocked traversal and continue
                    if (range_after_one_more_jump <= other_empire_biggest_range) {
                        m_supply_starlane_obstructed_traversals[empire_id].insert(std::make_pair(system_id, lane_end_sys_id));
                        //DebugLogger() << "Added obstructed traversal from " << system_id << " to " << lane_end_sys_id << " due to other empire biggest range being " << other_empire_biggest_range;
                        continue;
                    }

                    // otherwise, propagate into system...

                    // if propagating supply would increase the range of the adjacent system, do so.
                    std::map<int, float>::const_iterator prev_range_it = prev_sys_ranges.find(lane_end_sys_id);
                    if (prev_range_it == prev_sys_ranges.end() || range_after_one_more_jump > prev_range_it->second) {
                        empire_propagating_supply_ranges_next[empire_id][lane_end_sys_id] = range_after_one_more_jump;
                        //DebugLogger() << "Set system " << lane_end_sys_id << " range to: " << range_after_one_more_jump;
                    }
                    // always record a traversal, so connectivity is calculated properly
                    m_supply_starlane_traversals[empire_id].insert(std::make_pair(system_id, lane_end_sys_id));
                    //DebugLogger() << "Added traversal from " << system_id << " to " << lane_end_sys_id;

                    // erase any previous obstructed traversal that just succeeded
                    if (m_supply_starlane_obstructed_traversals[empire_id].find(std::make_pair(system_id, lane_end_sys_id)) !=
                        m_supply_starlane_obstructed_traversals[empire_id].end())
                    {
                        //DebugLogger() << "Removed obstructed traversal from " << system_id << " to " << lane_end_sys_id;
                        m_supply_starlane_obstructed_traversals[empire_id].erase(std::make_pair(system_id, lane_end_sys_id));
                    }
                    if (m_supply_starlane_obstructed_traversals[empire_id].find(std::make_pair(lane_end_sys_id, system_id)) !=
                        m_supply_starlane_obstructed_traversals[empire_id].end())
                    {
                        //DebugLogger() << "Removed obstructed traversal from " << lane_end_sys_id << " to " << system_id;
                        m_supply_starlane_obstructed_traversals[empire_id].erase(std::make_pair(lane_end_sys_id, system_id));
                    }
                }
            }
        }

        // save propagated results for next iteration
        empire_propagating_supply_ranges = empire_propagating_supply_ranges_next;
    }

    //// DEBUG
    //DebugLogger() << "SuppolyManager::Update: after removing conflicts, empires can provide supply to the following system ids:";
    //for (std::map<int, std::map<int, float> >::iterator empire_it = empire_propagating_supply_ranges.begin();
    //     empire_it != empire_propagating_supply_ranges.end(); ++empire_it)
    //{
    //    int empire_id = empire_it->first;
    //    std::stringstream ss;
    //    for (std::map<int, float>::iterator sys_it = empire_it->second.begin();
    //         sys_it != empire_it->second.end(); ++sys_it)
    //    {
    //        ss << sys_it->first << " (" << sys_it->second << "),  ";
    //    }
    //    DebugLogger() << "empire: " << empire_id << ":  " << ss.str();
    //}
    //// END DEBUG

    // record which systems are fleet supplyable by each empire (after resolving conflicts in each system)
    for (std::map<int, std::map<int, float> >::const_iterator empire_it = empire_propagating_supply_ranges.begin();
         empire_it != empire_propagating_supply_ranges.end(); ++empire_it)
    {
        int empire_id = empire_it->first;
        const std::map<int, float>& system_ranges = empire_it->second;
        for (std::map<int, float>::const_iterator sys_it = system_ranges.begin();
             sys_it != system_ranges.end(); ++sys_it)
        {
            if (sys_it->second < 0.0f)
                continue;   // negative supply doesn't count... zero does (it just reaches)
            m_fleet_supplyable_system_ids[empire_id].insert(sys_it->first);
            // should be only one empire per system at this point, but use max just to be safe...
            m_propagated_supply_ranges[sys_it->first] = std::max(sys_it->second, m_propagated_supply_ranges[sys_it->first]);
        }
    }

    // determine supply-connected groups of systems for each empire.
    // need to merge interconnected supply groups into as few sets of mutually-
    // supply-exchanging systems as possible.  This requires finding the
    // connected components of an undirected graph, where the node
    // adjacency are the directly-connected systems determined above.
    for (std::map<int, std::map<int, float> >::const_iterator empire_it = empire_propagating_supply_ranges.begin();
         empire_it != empire_propagating_supply_ranges.end(); ++empire_it)
    {
        int empire_id = empire_it->first;

        // assemble all direct connections between systems from remaining traversals
        std::map<int, std::set<int> > supply_groups_map;
        const std::set<std::pair<int, int> >& traversals = m_supply_starlane_traversals[empire_id];
        for (std::set<std::pair<int, int> >::const_iterator pair_it = traversals.begin();
             pair_it != traversals.end(); ++pair_it)
        {
            supply_groups_map[pair_it->first].insert(pair_it->second);
            supply_groups_map[pair_it->second].insert(pair_it->first);
        }

        // also add connections from all fleet-supplyable systems to themselves, so that
        // any fleet supply able system with no connection to another system can still
        // have resource sharing within tiself
        const std::set<int>& fleet_supplyable = m_fleet_supplyable_system_ids[empire_id];
        for (std::set<int>::const_iterator sys_it = fleet_supplyable.begin();
             sys_it != fleet_supplyable.end(); ++sys_it)
        {
            supply_groups_map[*sys_it].insert(*sys_it);
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
        for (std::map<int, std::set<int> >::const_iterator sys_it = supply_groups_map.begin();
             sys_it != supply_groups_map.end(); ++sys_it, ++graph_id)
        {
            int sys_id = sys_it->first;
            boost::add_vertex(graph);   // should add with index = graph_id

            graph_id_to_sys_id.push_back(sys_id);
            sys_id_to_graph_id[sys_id] = graph_id;
        }

        // add edges for all direct connections between systems
        // and add edges from fleet supplyable systems to themselves
        for (std::map<int, std::set<int> >::const_iterator maps_it = supply_groups_map.begin();
             maps_it != supply_groups_map.end(); ++maps_it)
        {
            int start_graph_id = sys_id_to_graph_id[maps_it->first];
            const std::set<int>& set = maps_it->second;
            for (std::set<int>::const_iterator set_it = set.begin(); set_it != set.end(); ++set_it) {
                int end_graph_id = sys_id_to_graph_id[*set_it];
                boost::add_edge(start_graph_id, end_graph_id, graph);
            }
        }

        // declare storage and fill with the component id (group id of connected systems)
        // for each graph vertex
        std::vector<int> components(boost::num_vertices(graph));
        boost::connected_components(graph, &components[0]);

        // convert results back from graph id to system id, and into desired output format
        // output: std::map<int, std::set<std::set<int> > >& m_resource_supply_groups

        // first, sort into a map from component id to set of system ids in component
        std::map<int, std::set<int> > component_sets_map;
        for (std::size_t graph_id = 0; graph_id != components.size(); ++graph_id) {
            int label = components[graph_id];
            int sys_id = graph_id_to_sys_id[graph_id];
            component_sets_map[label].insert(sys_id);
        }

        // copy sets in map into set of sets
        for (std::map<int, std::set<int> >::const_iterator map_it = component_sets_map.begin();
             map_it != component_sets_map.end(); ++map_it)
        {
            m_resource_supply_groups[empire_id].insert(map_it->second);
        }
    }
}
