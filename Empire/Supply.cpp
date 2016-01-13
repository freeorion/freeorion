#include "Supply.h"

#include "EmpireManager.h"

#include <boost/graph/connected_components.hpp>
//
//void Supply::UpdateSupply()
//{ UpdateSupply(this->KnownStarlanes()); }
//
//void Supply::UpdateSupply(const std::map<int, std::set<int> >& starlanes) {
//    //std::cout << "Supply::UpdateSupply for empire " << this->Name() << std::endl;
//
//    // Please also update PythonEmpireWrapper.cpp:CalculateSupplyUpdate if there is a change to the supply propagation rules: 
//    // (i) there is a set of supply sources in systems, (ii) propagating supply drops one per starlane jump, (iii) propagation is blocked
//    // into and out of any systems not in SupplyUnobstructedSystems, and (iv) a system gets the highest supply thus available to it.
//
//    m_supply_starlane_traversals.clear();
//    m_supply_starlane_obstructed_traversals.clear();
//    m_fleet_supplyable_system_ids.clear();
//    m_resource_supply_groups.clear();
//
//    // need to get a set of sets of systems that can exchange resources.  some
//    // sets may be just one system, in which resources can be exchanged between
//    // UniverseObjects producing or consuming them, but which can't exchange
//    // with any other systems.
//
//    // map from system id to set of systems that are supply-connected to it
//    // directly (which may involve multiple starlane jumps
//    std::map<int, std::set<int> > supply_groups_map;
//
//
//    // store supply range in jumps of all unobstructed systems before
//    // propegation, and add to list of systems to propegate from.
//    std::map<int, int> propegating_supply_ranges;
//    std::list<int> propegating_systems_list;
//    for (std::map<int, int>::const_iterator it = m_supply_system_ranges.begin();
//         it != m_supply_system_ranges.end(); ++it)
//    {
//        if (m_supply_unobstructed_systems.find(it->first) != m_supply_unobstructed_systems.end())
//            propegating_supply_ranges.insert(*it);
//
//        // system can supply itself, so store this fact
//        supply_groups_map[it->first].insert(it->first);
//
//        // add system to list of systems to popegate supply from
//        propegating_systems_list.push_back(it->first);
//
//        // Currently, only supply sources whose meter is above zero provide any fleet supply, and this
//        // is handled by the propegating_systems_list loop below. If it becomes intended that a supply 
//        // source with supply meter at zero could still provide fleet supply at its own system, 
//        // then uncomment the following line
//        //m_fleet_supplyable_system_ids.insert(it->first);
//    }
//
//
//    // iterate through list of accessible systems, processing each in order it
//    // was added (like breadth first search) until no systems are left able to
//    // further propregate
//    std::list<int>::iterator sys_list_it = propegating_systems_list.begin();
//    std::list<int>::iterator sys_list_end = propegating_systems_list.end();
//    while (sys_list_it != sys_list_end) {
//        int cur_sys_id = *sys_list_it;
//        int cur_sys_range = propegating_supply_ranges[cur_sys_id];    // range away from this system that supplies can be transported
//
//        // any unobstructed system can share resources within itself
//        supply_groups_map[cur_sys_id].insert(cur_sys_id);
//
//        if (cur_sys_range <= 0) {
//            // can't propegate supply out a system that has no range
//            ++sys_list_it;
//            continue;
//        }
//
//        // any system with nonzero fleet supply range can provide fleet supply
//        m_fleet_supplyable_system_ids.insert(cur_sys_id);
//
//        // can propegate further, if adjacent systems have smaller supply range
//        // than one less than this system's range
//        std::map<int, std::set<int> >::const_iterator system_it = starlanes.find(cur_sys_id);
//        if (system_it == starlanes.end()) {
//            // no starlanes out of this system
//            ++sys_list_it;
//            continue;
//        }
//
//
//        const std::set<int>& starlane_ends = system_it->second;
//        for (std::set<int>::const_iterator lane_it = starlane_ends.begin();
//             lane_it != starlane_ends.end(); ++lane_it)
//        {
//            int lane_end_sys_id = *lane_it;
//
//            if (m_supply_unobstructed_systems.find(lane_end_sys_id) == m_supply_unobstructed_systems.end()) {
//                // can't propegate here
//                m_supply_starlane_obstructed_traversals.insert(std::make_pair(cur_sys_id, lane_end_sys_id));
//                continue;
//            }
//
//            // can supply fleets here
//            m_fleet_supplyable_system_ids.insert(lane_end_sys_id);
//
//            // compare next system's supply range to this system's supply range.  propegate if necessary.
//            std::map<int, int>::const_iterator lane_end_sys_it = propegating_supply_ranges.find(lane_end_sys_id);
//            if (lane_end_sys_it == propegating_supply_ranges.end() || lane_end_sys_it->second <= cur_sys_range) {
//                // next system has no supply yet, or its range equal to or smaller than this system's
//
//                // update next system's range, if propegating from this system would make it larger
//                if (lane_end_sys_it == propegating_supply_ranges.end() || lane_end_sys_it->second < cur_sys_range - 1) {
//                    // update with new range
//                    propegating_supply_ranges[lane_end_sys_id] = cur_sys_range - 1;
//                    // add next system to list of systems to propegate further
//                    propegating_systems_list.push_back(lane_end_sys_id);
//                }
//
//                // regardless of whether propegating from current to next system
//                // increased its range, add the traversed lane to show
//                // redundancies in supply network to player
//                m_supply_starlane_traversals.insert(std::make_pair(cur_sys_id, lane_end_sys_id));
//
//                // current system can share resources with next system
//                supply_groups_map[cur_sys_id].insert(lane_end_sys_id);
//                supply_groups_map[lane_end_sys_id].insert(cur_sys_id);
//                //DebugLogger() << "added sys(" << lane_end_sys_id << ") to supply_groups_map[ " << cur_sys_id<<"]";
//                //DebugLogger() << "added sys(" << cur_sys_id << ") to supply_groups_map[ " << lane_end_sys_id <<"]";
//            }
//        }
//        ++sys_list_it;
//        sys_list_end = propegating_systems_list.end();
//    }
//
//    // Need to merge interconnected supply groups into as few sets of mutually-
//    // supply-exchanging systems as possible.  This requires finding the
//    // connected components of an undirected graph, where the node
//    // adjacency are the directly-connected systems determined above.
//
//    // create graph
//    boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> graph;
//
//    // boost expects vertex labels to range from 0 to num vertices - 1, so need
//    // to map from system id to graph id and back when accessing vertices
//    std::vector<int> graph_id_to_sys_id;
//    graph_id_to_sys_id.reserve(supply_groups_map.size());
//
//    std::map<int, int> sys_id_to_graph_id;
//    int graph_id = 0;
//    for (std::map<int, std::set<int> >::const_iterator sys_it = supply_groups_map.begin();
//         sys_it != supply_groups_map.end(); ++sys_it, ++graph_id)
//    {
//        int sys_id = sys_it->first;
//        //TemporaryPtr<const System> sys = GetSystem(sys_id);
//        //std::string name = sys->Name();
//        //DebugLogger() << "supply-exchanging system: " << name << " ID (" << sys_id <<")";
//
//        boost::add_vertex(graph);   // should add with index = graph_id
//
//        graph_id_to_sys_id.push_back(sys_id);
//        sys_id_to_graph_id[sys_id] = graph_id;
//    }
//
//    // add edges for all direct connections between systems
//    for (std::map<int, std::set<int> >::const_iterator maps_it = supply_groups_map.begin();
//         maps_it != supply_groups_map.end(); ++maps_it)
//    {
//        int start_graph_id = sys_id_to_graph_id[maps_it->first];
//        const std::set<int>& set = maps_it->second;
//        for(std::set<int>::const_iterator set_it = set.begin(); set_it != set.end(); ++set_it) {
//            int end_graph_id = sys_id_to_graph_id[*set_it];
//            boost::add_edge(start_graph_id, end_graph_id, graph);
//
//            //int sys_id1 = graph_id_to_sys_id[start_graph_id];
//            //TemporaryPtr<const System> sys1 = GetSystem(sys_id1);
//            //std::string name1 = sys1->Name();
//            //int sys_id2 = graph_id_to_sys_id[end_graph_id];
//            //TemporaryPtr<const System> sys2 = GetSystem(sys_id2);
//            //std::string name2 = sys2->Name();
//            //DebugLogger() << "added edge to graph: " << name1 << " and " << name2;
//        }
//    }
//
//    // declare storage and fill with the component id (group id of connected systems) for each graph vertex
//    std::vector<int> components(boost::num_vertices(graph));
//    boost::connected_components(graph, &components[0]);
//
//    //for (std::vector<int>::size_type i = 0; i != components.size(); ++i) {
//    //    int sys_id = graph_id_to_sys_id[i];
//    //    TemporaryPtr<const System> sys = GetSystem(sys_id);
//    //    std::string name = sys->Name();
//    //    DebugLogger() << "system " << name <<" is in component " << components[i];
//    //}
//    //std::cout << std::endl;
//
//    // convert results back from graph id to system id, and into desired output format
//    // output: std::set<std::set<int> >& m_resource_supply_groups
//
//    // first, sort into a map from component id to set of system ids in component
//    std::map<int, std::set<int> > component_sets_map;
//    for (std::size_t graph_id = 0; graph_id != components.size(); ++graph_id) {
//        int label = components[graph_id];
//        int sys_id = graph_id_to_sys_id[graph_id];
//        component_sets_map[label].insert(sys_id);
//    }
//
//    // copy sets in map into set of sets
//    for (std::map<int, std::set<int> >::const_iterator map_it = component_sets_map.begin(); map_it != component_sets_map.end(); ++map_it) {
//        m_resource_supply_groups.insert(map_it->second);
//
//        //// DEBUG!
//        //DebugLogger() << "Set: ";
//        //for (std::set<int>::const_iterator set_it = map_it->second.begin(); set_it != map_it->second.end(); ++set_it) {
//        //    TemporaryPtr<const UniverseObject> obj = GetUniverse().Object(*set_it);
//        //    if (!obj) {
//        //        DebugLogger() << " ... missing object!";
//        //        continue;
//        //    }
//        //    DebugLogger() << " ... " << obj->Name();
//        //}
//    }
//}
//
//const std::set<std::pair<int, int> >& Supply::SupplyStarlaneTraversals() const
//{ return m_supply_starlane_traversals; }
//
//const std::set<std::pair<int, int> >& Supply::SupplyObstructedStarlaneTraversals() const
//{ return m_supply_starlane_obstructed_traversals; }
//
//const std::set<int>& Supply::FleetSupplyableSystemIDs() const
//{ return m_fleet_supplyable_system_ids; }
//
//bool Supply::SystemHasFleetSupply(int system_id) const {
//    if (system_id == INVALID_OBJECT_ID)
//        return false;
//    if (m_fleet_supplyable_system_ids.find(system_id) != m_fleet_supplyable_system_ids.end())
//        return true;
//    return false;
//}
//
//const std::set<std::set<int> >& Supply::ResourceSupplyGroups() const
//{ return m_resource_supply_groups; }
