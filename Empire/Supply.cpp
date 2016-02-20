#include "Supply.h"

#include "EmpireManager.h"
#include "Empire.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../util/AppInterface.h"

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

void SupplyManager::Update() {
    m_supply_starlane_traversals.clear();
    m_supply_starlane_obstructed_traversals.clear();
    m_fleet_supplyable_system_ids.clear();
    m_resource_supply_groups.clear();

    // for each empire, need to get a set of sets of systems that can exchange
    // resources.  some sets may be just one system, in which resources can be
    // exchanged between UniverseObjects producing or consuming them, but which
    // can't exchange with any other systems.

    // which systems can share resources depends on system supply ranges, which
    // systems have obstructions to supply propegation for reach empire, and
    // the ranges and obstructions of other empires' supply, as only one empire
    // can supply each system or popegate along each starlane. one empire's
    // propegating supply can push back another's, if the pusher's range is
    // larger.

    // map from empire id to map from system id to set of systems that are
    // supply-connected to it directly (which may involve multiple starlane jumps
    std::map<int, std::map<int, float> > empire_system_supply_ranges;
    // map from empire id to which systems are obstructed for it for supply
    // propegation
    std::map<int, std::set<int> > empire_supply_unobstructed_systems;

    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        const Empire* empire = it->second;
        empire_system_supply_ranges[it->first] = empire->SystemSupplyRanges();
        empire_supply_unobstructed_systems[it->first] = empire->SupplyUnobstructedSystems();
    }

    // system connections each empire can see / use for supply propegation
    std::map<int, std::map<int, std::set<int> > > empire_visible_starlanes;
    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        const Empire* empire = it->second;
        empire_visible_starlanes[it->first] = empire->KnownStarlanes();//  VisibleStarlanes();
    }

    // store supply range in jumps of all unobstructed systems before
    // propegation, and add to list of systems to propegate from.
    std::map<int, std::map<int, float> > empire_propegating_supply_ranges;
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
                empire_propegating_supply_ranges[empire_id][system_id] = it->second;
                if (it->second > max_range)
                    max_range = it->second;
            }
        }
    }

    // spread supply out from sources by "diffusion" like process, along unobjstructed
    // starlanes, until the range is exhausted or spreading supply bumps into other
    // empires' spreading supply. when multiple empires' supply spreads meet up, the
    // higher-ranged supply takes precendence for a system. if two empires are evenly
    // matched in a system, then neither supplies (to) it.
    std::set<int> systems_with_supply_in_them;
    for (int iteration = 0; iteration <= static_cast<int>(max_range); ++iteration) {
        // initialize next iteration with current supply distribution
        std::map<int, std::map<int, float> > empire_propegating_supply_ranges_next = empire_propegating_supply_ranges;

        // for all sources of supply in current map, give adjacent systems one less
        // supply in the next iteration (unless more is already there)
        for (std::map<int, std::map<int, float> >::const_iterator prev_empire_it = empire_propegating_supply_ranges.begin();
             prev_empire_it != empire_propegating_supply_ranges.end(); ++prev_empire_it)
        {
            int empire_id = prev_empire_it->first;
            const std::map<int, float>& prev_sys_ranges = prev_empire_it->second;
            const std::set<int>& unobstructed_systems = empire_supply_unobstructed_systems[empire_id];

            for (std::map<int, float>::const_iterator prev_sys_it = prev_sys_ranges.begin();
                 prev_sys_it != prev_sys_ranges.end(); ++prev_sys_it)
            {
                // record that each system has some supply in it for later use...
                systems_with_supply_in_them.insert(prev_sys_it->first);

                // does the source system has enough supply range to propegate outwards at all?
                float range = prev_sys_it->second;
                if (range < 1.0f)
                    continue;   // need at least 1.0 range to propegate further.
                float range_after_one_more_jump = range - 1.0f; // what to set adjacent systems' ranges to (at least)

                // what starlanes can be used to propegate supply?
                int system_id = prev_sys_it->first;
                const std::set<int>& adjacent_systems = empire_visible_starlanes[empire_id][system_id];

                // attempt to propegate to all adjacent systems...
                for (std::set<int>::const_iterator lane_it = adjacent_systems.begin();
                     lane_it != adjacent_systems.end(); ++lane_it)
                {
                    int lane_end_sys_id = *lane_it;
                    // is propegation to the adjacent system obstructed?
                    if (unobstructed_systems.find(lane_end_sys_id) == unobstructed_systems.end()) {
                        // propegation obstructed!
                        m_supply_starlane_obstructed_traversals[empire_id].insert(std::make_pair(system_id, lane_end_sys_id));
                        continue;
                    }
                    // propegation can occur, get adjacent system's current range
                    float adj_sys_existing_range = empire_propegating_supply_ranges_next[empire_id][lane_end_sys_id];

                    // is the supply at the adjacent system bigger than (the source after propegating)
                    if (range_after_one_more_jump > adj_sys_existing_range) {
                        empire_propegating_supply_ranges_next[empire_id][lane_end_sys_id] = range_after_one_more_jump;
                        m_supply_starlane_traversals[empire_id].insert(std::make_pair(system_id, lane_end_sys_id));
                        // done later: remove traversals blocked by opposing supply
                    }
                }
            }
        }

        // save propegated results for next iteration
        empire_propegating_supply_ranges = empire_propegating_supply_ranges_next;
    }

    // pass over all empire-supplied systems, removing supply and traversals for all
    // but the empire with the highest supply range in each system
    for (std::set<int>::const_iterator sys_it = systems_with_supply_in_them.begin();
         sys_it != systems_with_supply_in_them.end(); ++sys_it)
    {
        int sys_id = *sys_it;
        std::map<float, std::set<int> > empire_ranges_here;

        // sort empires by range in this system
        for (std::map<int, std::map<int, float> >::const_iterator empire_it = empire_propegating_supply_ranges.begin();
             empire_it != empire_propegating_supply_ranges.end(); ++empire_it)
        {
            int empire_id = empire_it->first;
            std::map<int, float>::const_iterator empire_supply_it = empire_it->second.find(sys_id);
            // does this empire have any range in this system? if so, store it
            if (empire_supply_it != empire_it->second.end()) {
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
        }

        if (empire_ranges_here.empty())
            continue;   // no empire has supply here?
        if (empire_ranges_here.size() == 1 && empire_ranges_here.begin()->second.size() < 2)
            continue;   // only one empire has supply here

        // set to zero the range for all empires except the top-ranged empire here
        // if there is a tie, set all to zero
        std::map<float, std::set<int> >::reverse_iterator range_empire_it = empire_ranges_here.rbegin();

        int top_range_empire_id = ALL_EMPIRES;
        if (range_empire_it->second.size() == 1)
            top_range_empire_id = *(range_empire_it->second.begin());

        // remove range entries and traversals for all but top empire
        // (or all empires if there is no single top empire)
        for (std::map<int, std::map<int, float> >::iterator empire_it = empire_propegating_supply_ranges.begin();
             empire_it != empire_propegating_supply_ranges.end(); ++empire_it)
        {
            int empire_id = empire_it->first;
            if (empire_id == top_range_empire_id)
                continue;   // this is the top empire, so leave as the sole empire supplying here

            // remove from range entry...
            std::map<int, float>& empire_ranges = empire_it->second;
            empire_ranges.erase(empire_id);

            // remove from traversals involving this system for this empire
            std::set<std::pair<int, int> >& lane_traversals = m_supply_starlane_traversals[empire_id];
            std::set<std::pair<int, int> > lane_traversals_initial = lane_traversals;

            std::set<std::pair<int, int> >& obstructed_traversals = m_supply_starlane_obstructed_traversals[empire_id];
            std::set<std::pair<int, int> > obstrcuted_traversals_initial = obstructed_traversals;
            for (std::set<std::pair<int, int> >::iterator traversals_it = lane_traversals_initial.begin();
                 traversals_it != lane_traversals_initial.end(); ++traversals_it)
            {
                if (traversals_it->first == sys_id || traversals_it->second == sys_id) {
                    lane_traversals.erase(std::make_pair(traversals_it->first, traversals_it->second));
                    obstructed_traversals.insert(std::make_pair(traversals_it->first, traversals_it->second));
                }
            }

            // remove from obstructed traverals involving this system for this empire
            for (std::set<std::pair<int, int> >::iterator traversals_it = obstrcuted_traversals_initial.begin();
                 traversals_it != obstrcuted_traversals_initial.end(); ++traversals_it)
            {
                if (traversals_it->first == sys_id /*|| traversals_it->second == sys_id*/)
                    obstructed_traversals.erase(std::make_pair(traversals_it->first, traversals_it->second));
            }
        }
    }

    // record which systems are fleet supplyable by each empire (after resolving conflicts in each system)
    for (std::map<int, std::map<int, float> >::const_iterator empire_it = empire_propegating_supply_ranges.begin();
         empire_it != empire_propegating_supply_ranges.end(); ++empire_it)
    {
        int empire_id = empire_it->first;
        const std::map<int, float>& system_ranges = empire_it->second;
        for (std::map<int, float>::const_iterator sys_it = system_ranges.begin();
             sys_it != system_ranges.end(); ++sys_it)
        {
            if (sys_it->second < 0.0f)
                continue;   // negative supply doesn't count... zero does (it just reaches)
            m_fleet_supplyable_system_ids[empire_id].insert(sys_it->first);
        }
    }

    // determine supply-connected groups of systems for each empire.
    // need to merge interconnected supply groups into as few sets of mutually-
    // supply-exchanging systems as possible.  This requires finding the
    // connected components of an undirected graph, where the node
    // adjacency are the directly-connected systems determined above.
    for (std::map<int, std::map<int, float> >::const_iterator empire_it = empire_propegating_supply_ranges.begin();
         empire_it != empire_propegating_supply_ranges.end(); ++empire_it)
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
