#include "Universe.h"

#include "../util/AppInterface.h"
#include "../util/DataTable.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/Random.h"
#include "../parse/Parse.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "Building.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "System.h"
#include "Field.h"
#include "UniverseObject.h"
#include "Effect.h"
#include "Predicates.h"
#include "Special.h"
#include "Species.h"
#include "Condition.h"
#include "ValueRef.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>
#include <boost/timer.hpp>

#include <cmath>
#include <stdexcept>


namespace {
    const bool ENABLE_VISIBILITY_EMPIRE_MEMORY = true;      // toggles using memory with visibility, so that empires retain knowledge of objects viewed on previous turns

    void AddOptions(OptionsDB& db) {
        db.Add("verbose-logging", UserStringNop("OPTIONS_DB_VERBOSE_LOGGING_DESC"),  false,  Validator<bool>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    const double  OFFROAD_SLOWDOWN_FACTOR = 1000000000.0;   // the factor by which non-starlane travel is slower than starlane travel

    template <class Key, class Value> struct constant_property
    { Value m_value; };
}

namespace boost {
    template <class Key, class Value> struct property_traits<constant_property<Key, Value> > {
        typedef Value value_type;
        typedef Key key_type;
        typedef readable_property_map_tag category;
    };
    template <class Key, class Value> const Value& get(const constant_property<Key, Value>& pmap, const Key&) { return pmap.m_value; }
}

namespace SystemPathing {
    /** Used to short-circuit the use of BFS (breadth-first search) or
      * Dijkstra's algorithm for pathfinding when it finds the desired
      * destination system. */
    struct PathFindingShortCircuitingVisitor : public boost::base_visitor<PathFindingShortCircuitingVisitor>
    {
        typedef boost::on_finish_vertex event_filter;

        struct FoundDestination {}; // exception type thrown when destination is found

        PathFindingShortCircuitingVisitor(int dest_system) : destination_system(dest_system) {}
        template <class Vertex, class Graph>
        void operator()(Vertex u, Graph& g)
        {
            if (static_cast<int>(u) == destination_system)
                throw FoundDestination();
        }
        const int destination_system;
    };

    /** Complete BFS visitor implementing:
      *  - predecessor recording
      *  - short-circuit exit on found match
      *  - maximum search depth 
      */
    template <class Graph, class Edge, class Vertex> class BFSVisitorImpl
    {
    public:
        class FoundDestination {}; 
        class ReachedDepthLimit {};

    private:
        Vertex m_marker;
        Vertex m_stop;
        Vertex m_source;
        Vertex * m_predecessors;
        int m_levels_remaining;
        bool m_level_complete;

    public:
        BFSVisitorImpl(const Vertex& start, const Vertex& stop, Vertex predecessors[], int max_depth)
            : m_marker(start),
              m_stop(stop),
              m_source(start),
              m_predecessors(predecessors),
              m_levels_remaining(max_depth),
              m_level_complete(false)
        {}

        void initialize_vertex(const Vertex& v, const Graph& g)
        {}

        void discover_vertex(const Vertex& v, const Graph& g) {
            m_predecessors[static_cast<int>(v)] = m_source;

            if (v == m_stop)
                throw FoundDestination();

            if (m_level_complete) {
                m_marker = v;
                m_level_complete = false;
            }
        }

        void examine_vertex(const Vertex& v, const Graph& g) {
            if (v == m_marker) {
                if (!m_levels_remaining)
                    throw ReachedDepthLimit();
                m_levels_remaining--;
                m_level_complete = true;
            }
            
            m_source = v; // avoid re-calculating source from edge
        }

        void examine_edge(const Edge& e, const Graph& g) {}
        void tree_edge(const Edge& e, const Graph& g) {}    // wait till target is calculated


        void non_tree_edge(const Edge& e, const Graph& g) {}
        void gray_target(const Edge& e, const Graph& g) {}
        void black_target(const Edge& e, const Graph& g) {}
        void finish_vertex(const Vertex& e, const Graph& g) {}
    };

    ////////////////////////////////////////////////////////////////
    // templated implementations of Universe graph search methods //
    ////////////////////////////////////////////////////////////////
    struct vertex_system_id_t {typedef boost::vertex_property_tag kind;}; ///< a system graph property map type

    /** Returns the path between vertices \a system1_id and \a system2_id of
      * \a graph that travels the shorest distance on starlanes, and the path
      * length.  If system1_id is the same vertex as system2_id, the path has
      * just that system in it, and the path lenth is 0.  If there is no path
      * between the two vertices, then the list is empty and the path length
      * is -1.0 */
    template <class Graph>
    std::pair<std::list<int>, double> ShortestPathImpl(const Graph& graph, int system1_id, int system2_id,
                                                       double linear_distance, const boost::unordered_map<int, int>& id_to_graph_index)
    {
        typedef typename boost::property_map<Graph, vertex_system_id_t>::const_type     ConstSystemIDPropertyMap;
        typedef typename boost::property_map<Graph, boost::vertex_index_t>::const_type  ConstIndexPropertyMap;
        typedef typename boost::property_map<Graph, boost::edge_weight_t>::const_type   ConstEdgeWeightPropertyMap;

        std::pair<std::list<int>, double> retval(std::list<int>(), -1.0);

        ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), graph);

        // convert system IDs to graph indices.  try/catch for invalid input system ids.
        int system1_index, system2_index;
        try {
            system1_index = id_to_graph_index.at(system1_id);
            system2_index = id_to_graph_index.at(system2_id);
        } catch (...) {
            return retval;
        }

        // early exit if systems are the same
        if (system1_id == system2_id) {
            retval.first.push_back(system2_id);
            retval.second = 0.0;    // no jumps needed -> 0 distance
            return retval;
        }

        /* initializing all vertices' predecessors to themselves prevents endless loops when back traversing the tree in the case where
           one of the end systems is system 0, because systems that are not connected to the root system (system2) are not visited
           by the search, and so their predecessors are left unchanged.  Default initialization of the vector may be 0 or undefined
           which could lead to out of bounds errors, or endless loops if a system's default predecessor is 0 (debug mode), and 0's
           predecessor is that system */
        std::vector<int> predecessors(boost::num_vertices(graph));
        std::vector<double> distances(boost::num_vertices(graph));
        for (unsigned int i = 0; i < boost::num_vertices(graph); ++i) {
            predecessors[i] = i;
            distances[i] = -1.0;
        }


        ConstIndexPropertyMap index_map = boost::get(boost::vertex_index, graph);
        ConstEdgeWeightPropertyMap edge_weight_map = boost::get(boost::edge_weight, graph);


        // do the actual path finding using verbose boost magic...
        try {
            boost::dijkstra_shortest_paths(graph, system1_index, &predecessors[0], &distances[0], edge_weight_map, index_map, 
                                           std::less<double>(), std::plus<double>(), std::numeric_limits<int>::max(), 0, 
                                           boost::make_dijkstra_visitor(PathFindingShortCircuitingVisitor(system2_index)));
        } catch (const PathFindingShortCircuitingVisitor::FoundDestination&) {
            // catching this just means that the destination was found, and so the algorithm was exited early, via exception
        }


        int current_system = system2_index;
        while (predecessors[current_system] != current_system) {
            retval.first.push_front(sys_id_property_map[current_system]);
            current_system = predecessors[current_system];
        }
        retval.second = distances[system2_index];

        if (retval.first.empty()) {
            // there is no path between the specified nodes
            retval.second = -1.0;
            return retval;
        } else {
            // add start system to path, as it wasn't added by traversing predecessors array
            retval.first.push_front(sys_id_property_map[system1_index]);
        }

        return retval;
    }

    /** Returns the path between vertices \a system1_id and \a system2_id of
      * \a graph that takes the fewest number of jumps (edge traversals), and
      * the number of jumps this path takes.  If system1_id is the same vertex
      * as system2_id, the path has just that system in it, and the path lenth
      * is 0.  If there is no path between the two vertices, then the list is
      * empty and the path length is -1 */
    template <class Graph>
    std::pair<std::list<int>, int> LeastJumpsPathImpl(const Graph& graph, int system1_id, int system2_id,
                                                      const boost::unordered_map<int, int>& id_to_graph_index,
                                                      int max_jumps = INT_MAX)
    {
        typedef typename boost::property_map<Graph, vertex_system_id_t>::const_type ConstSystemIDPropertyMap;

        ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), graph);
        std::pair<std::list<int>, int> retval;

        int system1_index = id_to_graph_index.at(system1_id);
        int system2_index = id_to_graph_index.at(system2_id);

        // early exit if systems are the same
        if (system1_id == system2_id) {
            retval.first.push_back(system2_id);
            retval.second = 0;  // no jumps needed
            return retval;
        }

        /* initializing all vertices' predecessors to themselves prevents endless loops when back traversing the tree in the case where
           one of the end systems is system 0, because systems that are not connected to the root system (system2) are not visited
           by the search, and so their predecessors are left unchanged.  Default initialization of the vector may be 0 or undefined
           which could lead to out of bounds errors, or endless loops if a system's default predecessor is 0, (debug mode) and 0's
           predecessor is that system */
        std::vector<int> predecessors(boost::num_vertices(graph));
        for (unsigned int i = 0; i < boost::num_vertices(graph); ++i)
            predecessors[i] = i;


        // do the actual path finding using verbose boost magic...
        typedef BFSVisitorImpl<Graph, typename boost::graph_traits<Graph>::edge_descriptor, int> BFSVisitor;
        try {
            boost::queue<int> buf;
            std::vector<int> colors(boost::num_vertices(graph));

            BFSVisitor bfsVisitor(system1_index, system2_index, &predecessors[0], max_jumps);
            boost::breadth_first_search(graph, system1_index, buf, bfsVisitor, &colors[0]);
        } catch (const typename BFSVisitor::ReachedDepthLimit&) {
            // catching this means the algorithm explored the neighborhood until max_jumps and didn't find anything
            return std::make_pair(std::list<int>(), -1);
        } catch (const typename BFSVisitor::FoundDestination&) {
            // catching this just means that the destination was found, and so the algorithm was exited early, via exception
        }


        int current_system = system2_index;
        while (predecessors[current_system] != current_system) {
            retval.first.push_front(sys_id_property_map[current_system]);
            current_system = predecessors[current_system];
        }
        retval.second = retval.first.size() - 1;    // number of jumps is number of systems in path minus one for the starting system

        if (retval.first.empty()) {
            // there is no path between the specified nodes
            retval.second = -1;
        } else {
            // add start system to path, as it wasn't added by traversing predecessors array
            retval.first.push_front(sys_id_property_map[system1_index]);
        }

        return retval;
    }

    template <class Graph>
    std::multimap<double, int> ImmediateNeighborsImpl(const Graph& graph, int system_id,
                                                      const boost::unordered_map<int, int>& id_to_graph_index)
    {
        typedef typename Graph::out_edge_iterator OutEdgeIterator;
        typedef typename boost::property_map<Graph, vertex_system_id_t>::const_type ConstSystemIDPropertyMap;
        typedef typename boost::property_map<Graph, boost::edge_weight_t>::const_type ConstEdgeWeightPropertyMap;

        std::multimap<double, int> retval;
        ConstEdgeWeightPropertyMap edge_weight_map = boost::get(boost::edge_weight, graph);
        ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), graph);
        std::pair<OutEdgeIterator, OutEdgeIterator> edges = boost::out_edges(id_to_graph_index.at(system_id), graph);
        for (OutEdgeIterator it = edges.first; it != edges.second; ++it)
        { retval.insert(std::make_pair(edge_weight_map[*it], sys_id_property_map[boost::target(*it, graph)])); }
        return retval;
    }
}
using namespace SystemPathing;  // to keep GCC 4.2 on OSX happy

extern const int ALL_EMPIRES            = -1;
extern const int INVALID_OBJECT_ID      = -1;   //the ID number assigned to a UniverseObject upon construction; it is assigned an ID later when it is placed in the universe
extern const int TEMPORARY_OBJECT_ID    = -2; // the ID number assigned to temporary objects
// TODO: implement a robust, thread-safe solution for creating multiple client-local temporary objects with unique IDs that will never conflict with each other or the server.
extern const int MAX_ID                 = 2000000000;


/////////////////////////////////////////////
// struct Universe::GraphImpl
/////////////////////////////////////////////
struct Universe::GraphImpl {
    typedef boost::property<vertex_system_id_t, int,
                            boost::property<boost::vertex_index_t, int> >   vertex_property_t;  ///< a system graph property map type
    typedef boost::property<boost::edge_weight_t, double>                   edge_property_t;    ///< a system graph property map type

    // declare main graph types, including properties declared above
    // could add boost::disallow_parallel_edge_tag GraphProperty but it doesn't
    // work for vecS vector-based lists and parallel edges can be avoided while
    // creating the graph by filtering the edges to be added
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                                  vertex_property_t, edge_property_t> SystemGraph;

    struct EdgeVisibilityFilter     {
        EdgeVisibilityFilter() :
            m_graph(0),
            m_empire_id(ALL_EMPIRES)
        {}

        EdgeVisibilityFilter(const SystemGraph* graph, int empire_id) :
            m_graph(graph),
            m_empire_id(empire_id)
        {
            if (!graph)
                Logger().errorStream() << "EdgeVisibilityFilter passed null graph pointer";
        }

        template <typename EdgeDescriptor>
        bool operator()(const EdgeDescriptor& edge) const
        {
            if (!m_graph)
                return false;

            // get system ids from graph indices
            ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), *m_graph); // for reverse-lookup System universe ID from graph index
            int sys_graph_index_1 = boost::source(edge, *m_graph);
            int sys_id_1 = sys_id_property_map[sys_graph_index_1];
            int sys_graph_index_2 = boost::target(edge, *m_graph);
            int sys_id_2 = sys_id_property_map[sys_graph_index_2];

            // look up lane between systems
            const System* system1 = GetEmpireKnownSystem(sys_id_1, m_empire_id);
            if (!system1) {
                Logger().errorStream() << "EdgeDescriptor::operator() couldn't find system with id " << sys_id_1;
                return false;
            }
            if (system1->HasStarlaneTo(sys_id_2))
                return true;

            // lane not found
            return false;
        }

    private:
        const SystemGraph*      m_graph;
        int                     m_empire_id;
    };
    typedef boost::filtered_graph<SystemGraph, EdgeVisibilityFilter> EmpireViewSystemGraph;
    typedef std::map<int, boost::shared_ptr<EmpireViewSystemGraph> > EmpireViewSystemGraphMap;

    // declare property map types for properties declared above
    typedef boost::property_map<SystemGraph, vertex_system_id_t>::const_type        ConstSystemIDPropertyMap;
    typedef boost::property_map<SystemGraph, vertex_system_id_t>::type              SystemIDPropertyMap;
    typedef boost::property_map<SystemGraph, boost::vertex_index_t>::const_type     ConstIndexPropertyMap;
    typedef boost::property_map<SystemGraph, boost::vertex_index_t>::type           IndexPropertyMap;
    typedef boost::property_map<SystemGraph, boost::edge_weight_t>::const_type      ConstEdgeWeightPropertyMap;
    typedef boost::property_map<SystemGraph, boost::edge_weight_t>::type            EdgeWeightPropertyMap;

    SystemGraph                 system_graph;                 ///< a graph in which the systems are vertices and the starlanes are edges
    EmpireViewSystemGraphMap    empire_system_graph_views;    ///< a map of empire IDs to the views of the system graph by those empires
};

/////////////////////////////////////////////
// class Universe
/////////////////////////////////////////////
Universe::Universe() :
    m_graph_impl(new GraphImpl),
    m_last_allocated_object_id(-1), // this is conicidentally equal to INVALID_OBJECT_ID as of this writing, but the reason for this to be -1 is so that the first object has id 0, and all object ids are non-negative
    m_last_allocated_design_id(-1), // same, but for ShipDesign::INVALID_DESIGN_ID
    m_universe_width(1000.0),
    m_inhibit_universe_object_signals(false),
    m_encoding_empire(ALL_EMPIRES),
    m_all_objects_visible(false)
{}

Universe::~Universe() {
    Clear();
    delete m_graph_impl;
}

void Universe::Clear() {
    // empty object maps
    m_objects.Clear();
    for (EmpireObjectMap::iterator it = m_empire_latest_known_objects.begin(); it != m_empire_latest_known_objects.end(); ++it)
        it->second.Clear();
    m_empire_latest_known_objects.clear();

    // clean up ship designs
    for (ShipDesignMap::iterator it = m_ship_designs.begin(); it != m_ship_designs.end(); ++it)
        delete it->second;
    m_ship_designs.clear();

    m_destroyed_object_ids.clear();

    m_empire_object_visibility.clear();
    m_empire_object_visibility_turns.clear();

    m_empire_object_visible_specials.clear();

    m_system_id_to_graph_index.clear();
    m_effect_accounting_map.clear();
    m_effect_discrepancy_map.clear();

    m_last_allocated_object_id = -1;
    m_last_allocated_design_id = -1;

    m_empire_known_destroyed_object_ids.clear();
    m_empire_stale_knowledge_object_ids.clear();

    m_empire_known_ship_design_ids.clear();

    m_marked_destroyed.clear();
    m_marked_for_victory.clear();
}

const ObjectMap& Universe::EmpireKnownObjects(int empire_id) const {
    if (empire_id == ALL_EMPIRES)
        return m_objects;

    EmpireObjectMap::const_iterator it = m_empire_latest_known_objects.find(empire_id);
    if (it != m_empire_latest_known_objects.end())
        return it->second;

    static const ObjectMap const_empty_map;
    return const_empty_map;
}

ObjectMap& Universe::EmpireKnownObjects(int empire_id) {
    if (empire_id == ALL_EMPIRES)
        return m_objects;

    EmpireObjectMap::iterator it = m_empire_latest_known_objects.find(empire_id);
    if (it != m_empire_latest_known_objects.end()) {
        return it->second;
    }

    static ObjectMap empty_map;
    empty_map.Clear();
    return empty_map;
}

std::set<int> Universe::EmpireVisibleObjectIDs(int empire_id/* = ALL_EMPIRES*/) const {
    std::set<int> retval;

    // get id(s) of all empires to consider visibility of...
    std::set<int> empire_ids;
    if (empire_id != ALL_EMPIRES)
        empire_ids.insert(empire_id);
    else
        for (EmpireManager::const_iterator empire_it = Empires().begin(); empire_it != Empires().end(); ++empire_it)
            empire_ids.insert(empire_it->first);

    // check each object's visibility against all empires, including the object
    // if an empire has visibility of it
    for (ObjectMap::const_iterator<> obj_it = m_objects.const_begin(); obj_it != m_objects.const_end(); ++obj_it) {
        int id = obj_it->ID();
        for (std::set<int>::const_iterator empire_it = empire_ids.begin(); empire_it != empire_ids.end(); ++empire_it) {
            Visibility vis = GetObjectVisibilityByEmpire(id, empire_id);
            if (vis >= VIS_BASIC_VISIBILITY) {
                retval.insert(id);
                break;
            }
        }
    }

    return retval;
}

const std::set<int>& Universe::DestroyedObjectIds() const
{ return m_destroyed_object_ids; }

const std::set<int>& Universe::EmpireKnownDestroyedObjectIDs(int empire_id) const {
    ObjectKnowledgeMap::const_iterator it = m_empire_known_destroyed_object_ids.find(empire_id);
    if (it != m_empire_known_destroyed_object_ids.end())
        return it->second;
    return m_destroyed_object_ids;
}

const std::set<int>& Universe::EmpireStaleKnowledgeObjectIDs(int empire_id) const {
    ObjectKnowledgeMap::const_iterator it = m_empire_stale_knowledge_object_ids.find(empire_id);
    if (it != m_empire_stale_knowledge_object_ids.end())
        return it->second;
    static const std::set<int> empty_set;
    return empty_set;
}

const ShipDesign* Universe::GetShipDesign(int ship_design_id) const {
    if (ship_design_id == ShipDesign::INVALID_DESIGN_ID)
        return 0;
    ship_design_iterator it = m_ship_designs.find(ship_design_id);
    return (it != m_ship_designs.end() ? it->second : 0);
}

const ShipDesign* Universe::GetGenericShipDesign(const std::string& name) const {
    if (name.empty())
        return 0;
    for (ship_design_iterator it = m_ship_designs.begin(); it != m_ship_designs.end(); ++it) {
        const ShipDesign* design = it->second;
        const std::string& design_name = design->Name(false);
        if (name == design_name)
            return design;
    }
    return 0;
}

const std::set<int>& Universe::EmpireKnownShipDesignIDs(int empire_id) const {
    std::map<int, std::set<int> >::const_iterator it = m_empire_known_ship_design_ids.find(empire_id);
    if (it != m_empire_known_ship_design_ids.end())
        return it->second;
    static const std::set<int> empty_set;
    return empty_set;
}

Visibility Universe::GetObjectVisibilityByEmpire(int object_id, int empire_id) const {
    if (empire_id == ALL_EMPIRES || GetUniverse().AllObjectsVisible())
        return VIS_FULL_VISIBILITY;

    EmpireObjectVisibilityMap::const_iterator empire_it = m_empire_object_visibility.find(empire_id);
    if (empire_it == m_empire_object_visibility.end())
        return VIS_NO_VISIBILITY;

    const ObjectVisibilityMap& vis_map = empire_it->second;

    ObjectVisibilityMap::const_iterator vis_map_it = vis_map.find(object_id);
    if (vis_map_it == vis_map.end())
        return VIS_NO_VISIBILITY;

    return vis_map_it->second;
}

const Universe::VisibilityTurnMap& Universe::GetObjectVisibilityTurnMapByEmpire(int object_id, int empire_id) const {
    static const std::map<Visibility, int> empty_map;

    EmpireObjectVisibilityTurnMap::const_iterator empire_it = m_empire_object_visibility_turns.find(empire_id);
    if (empire_it == m_empire_object_visibility_turns.end())
        return empty_map;

    const ObjectVisibilityTurnMap& obj_vis_turn_map = empire_it->second;
    ObjectVisibilityTurnMap::const_iterator object_it = obj_vis_turn_map.find(object_id);
    if (object_it == obj_vis_turn_map.end())
        return empty_map;

    return object_it->second;
}

std::set<std::string> Universe::GetObjectVisibleSpecialsByEmpire(int object_id, int empire_id) const {
    if (empire_id != ALL_EMPIRES) {
        EmpireObjectSpecialsMap::const_iterator empire_it = m_empire_object_visible_specials.find(empire_id);
        if (empire_it == m_empire_object_visible_specials.end())
            return std::set<std::string>();
        const ObjectSpecialsMap& object_specials_map = empire_it->second;
        ObjectSpecialsMap::const_iterator object_it = object_specials_map.find(object_id);
        if (object_it == object_specials_map.end())
            return std::set<std::string>();
        return object_it->second;
    } else {
        const UniverseObject* obj = m_objects.Object(object_id);
        if (!obj)
            return std::set<std::string>();
        // all specials visible
        std::set<std::string> retval;
        const std::map<std::string, int>& specials = obj->Specials();
        for (std::map<std::string, int>::const_iterator it = specials.begin();
             it != specials.end(); ++it)
        {
            retval.insert(it->first);
        }
        return retval;
    }
}

double Universe::LinearDistance(int system1_id, int system2_id) const {
    try {
        int system1_index = m_system_id_to_graph_index.at(system1_id);
        int system2_index = m_system_id_to_graph_index.at(system2_id);
        return m_system_distances[system1_index][system2_index];
    } catch (const std::out_of_range&) {
        Logger().errorStream() << "Universe::LinearDistance passed invalid system id(s): "
                               << system1_id << " & " << system2_id;
        throw;
    }
}

short Universe::JumpDistance(int system1_id, int system2_id) const {
    try {
        int system1_index = m_system_id_to_graph_index.at(system1_id);
        int system2_index = m_system_id_to_graph_index.at(system2_id);
        short jumps = m_system_jumps[system1_index][system2_index];
        if (jumps == SHRT_MAX)  // value returned for no valid path
            return -1;
        return jumps;
    } catch (const std::out_of_range&) {
        Logger().errorStream() << "Universe::JumpDistance passed invalid system id(s): "
                               << system1_id << " & " << system2_id;
        throw;
    }
}

std::pair<std::list<int>, double> Universe::ShortestPath(int system1_id, int system2_id, int empire_id/* = ALL_EMPIRES*/) const {
    if (empire_id == ALL_EMPIRES) {
        // find path on full / complete system graph
        try {
            double linear_distance = LinearDistance(system1_id, system2_id);
            return ShortestPathImpl(m_graph_impl->system_graph, system1_id, system2_id,
                                    linear_distance, m_system_id_to_graph_index);
        } catch (const std::out_of_range&) {
            Logger().errorStream() << "Universe::ShortestPath passed invalid system id(s): "
                                   << system1_id << " & " << system2_id;
            throw;
        }
    }

    // find path on single empire's view of system graph
    GraphImpl::EmpireViewSystemGraphMap::const_iterator graph_it =
        m_graph_impl->empire_system_graph_views.find(empire_id);
    if (graph_it == m_graph_impl->empire_system_graph_views.end()) {
        Logger().errorStream() << "Universe::ShortestPath passed unknown empire id: " << empire_id;
        throw std::out_of_range("Universe::ShortestPath passed unknown empire id");
    }
    try {
        double linear_distance = LinearDistance(system1_id, system2_id);
        return ShortestPathImpl(*graph_it->second, system1_id, system2_id,
                                linear_distance, m_system_id_to_graph_index);
    } catch (const std::out_of_range&) {
        Logger().errorStream() << "Universe::ShortestPath passed invalid system id(s): "
                               << system1_id << " & " << system2_id;
        throw;
    }
}

std::pair<std::list<int>, int> Universe::LeastJumpsPath(int system1_id, int system2_id, int empire_id/* = ALL_EMPIRES*/,
                                                        int max_jumps/* = INT_MAX*/) const
{
    if (empire_id == ALL_EMPIRES) {
        // find path on full / complete system graph
        try {
            return LeastJumpsPathImpl(m_graph_impl->system_graph, system1_id, system2_id,
                                      m_system_id_to_graph_index, max_jumps);
        } catch (const std::out_of_range&) {
            Logger().errorStream() << "Universe::LeastJumpsPath passed invalid system id(s): "
                                   << system1_id << " & " << system2_id;
            throw;
        }
    }

    // find path on single empire's view of system graph
    GraphImpl::EmpireViewSystemGraphMap::const_iterator graph_it =
        m_graph_impl->empire_system_graph_views.find(empire_id);
    if (graph_it == m_graph_impl->empire_system_graph_views.end()) {
        Logger().errorStream() << "Universe::LeastJumpsPath passed unknown empire id: " << empire_id;
        throw std::out_of_range("Universe::LeastJumpsPath passed unknown empire id");
    }
    try {
        return LeastJumpsPathImpl(*graph_it->second, system1_id, system2_id,
                                  m_system_id_to_graph_index, max_jumps);
    } catch (const std::out_of_range&) {
        Logger().errorStream() << "Universe::LeastJumpsPath passed invalid system id(s): "
                               << system1_id << " & " << system2_id;
        throw;
    }
}

bool Universe::SystemsConnected(int system1_id, int system2_id, int empire_id) const {
    //Logger().debugStream() << "SystemsConnected(" << system1_id << ", " << system2_id << ", " << empire_id << ")";
    std::pair<std::list<int>, int> path = LeastJumpsPath(system1_id, system2_id, empire_id);
    //Logger().debugStream() << "SystemsConnected returned path of size: " << path.first.size();
    bool retval = !path.first.empty();
    //Logger().debugStream() << "SystemsConnected retval: " << retval;
    return retval;
}

bool Universe::SystemHasVisibleStarlanes(int system_id, int empire_id) const {
    if (const System* system = GetEmpireKnownSystem(system_id, empire_id))
        if (!system->StarlanesWormholes().empty())
            return true;
    return false;
}

std::multimap<double, int> Universe::ImmediateNeighbors(int system_id, int empire_id/* = ALL_EMPIRES*/) const {
    if (empire_id == ALL_EMPIRES) {
        return ImmediateNeighborsImpl(m_graph_impl->system_graph, system_id, m_system_id_to_graph_index);
    } else {
        GraphImpl::EmpireViewSystemGraphMap::const_iterator graph_it = m_graph_impl->empire_system_graph_views.find(empire_id);
        if (graph_it != m_graph_impl->empire_system_graph_views.end())
            return ImmediateNeighborsImpl(*graph_it->second, system_id, m_system_id_to_graph_index);
    }
    return std::multimap<double, int>();
}

int Universe::Insert(UniverseObject* obj) {
    if (!obj)
        return INVALID_OBJECT_ID;

    if (m_last_allocated_object_id + 1 < MAX_ID) {
        int id = ++m_last_allocated_object_id;
        obj->SetID(id);
        m_objects.Insert(obj);
        return id;
    }

    // we'll probably never execute this branch, considering how many IDs are available
    // find a hole in the assigned IDs in which to place the object
    int last_id_seen = INVALID_OBJECT_ID;
    for (ObjectMap::iterator<> it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (1 < it->ID() - last_id_seen) {
            int id = last_id_seen + 1;
            obj->SetID(id);
            m_objects.Insert(obj);
            return id;
        }
    }

    return INVALID_OBJECT_ID;
}

bool Universe::InsertID(UniverseObject* obj, int id) {
    if (!obj || id == INVALID_OBJECT_ID || id >= MAX_ID)
        return false;

    obj->SetID(id);
    m_objects.Insert(obj);
    if ( id > m_last_allocated_object_id )
        m_last_allocated_object_id=id;
    return true;
}

int Universe::InsertShipDesign(ShipDesign* ship_design) {
    int retval = ShipDesign::INVALID_DESIGN_ID;
    if (ship_design) {
        if (m_last_allocated_design_id + 1 < MAX_ID) {
            m_ship_designs[++m_last_allocated_design_id] = ship_design;
            retval = m_last_allocated_design_id;
        } else { // we'll probably never execute this branch, considering how many IDs are available
            // find a hole in the assigned IDs in which to place the object
            int last_id_seen = ShipDesign::INVALID_DESIGN_ID;
            for (ShipDesignMap::iterator it = m_ship_designs.begin(); it != m_ship_designs.end(); ++it) {
                if (1 < it->first - last_id_seen) {
                    m_ship_designs[last_id_seen + 1] = ship_design;
                    retval = last_id_seen + 1;
                    break;
                }
            }
        }
    }
    return retval;
}

bool Universe::InsertShipDesignID(ShipDesign* ship_design, int id) {
    bool retval = false;

    if (ship_design  &&  id != ShipDesign::INVALID_DESIGN_ID  &&  id < ShipDesign::MAX_ID) {
        ship_design->SetID(id);
        m_ship_designs[id] = ship_design;
        retval = true;
    }
    return retval;
}

bool Universe::DeleteShipDesign(int design_id) {
    ShipDesignMap::iterator it = m_ship_designs.find(design_id);
    if (it != m_ship_designs.end()) {
        m_ship_designs.erase(it);
        return true;
    } else { return false; }
}

void Universe::ApplyAllEffectsAndUpdateMeters() {
    ScopedTimer timer("Universe::ApplyAllEffectsAndUpdateMeters");
    std::vector<int> object_ids = m_objects.FindObjectIDs();

    // cache all activation and scoping condition results before applying
    // Effects, since the application of these Effects may affect the activation
    // and scoping evaluations
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes, object_ids);

    // revert all current meter values (which are modified by effects) to
    // their initial state for this turn, so that max/target/unpaired meter
    // value can be calculated (by accumulating all effects' modifications this
    // turn) and active meters have the proper baseline from which to
    // accumulate changes from effects
    for (ObjectMap::iterator<> it = m_objects.begin(); it != m_objects.end(); ++it) {
        it->ResetTargetMaxUnpairedMeters();
        it->ResetPairedActiveMeters();
    }
    for (EmpireManager::iterator it = Empires().begin(); it != Empires().end(); ++it)
        it->second->ResetMeters();

    ExecuteEffects(targets_causes, true, false, false, true);

    // clamp max meters to [DEFAULT_VALUE, LARGE_VALUE] and current meters to [DEFAULT_VALUE, max]
    // clamp max and target meters to [DEFAULT_VALUE, LARGE_VALUE] and current meters to [DEFAULT_VALUE, max]
    for (ObjectMap::iterator<> it = m_objects.begin(); it != m_objects.end(); ++it)
        it->ClampMeters();
}

void Universe::ApplyMeterEffectsAndUpdateMeters(const std::vector<int>& object_ids) {
    ScopedTimer timer("Universe::ApplyMeterEffectsAndUpdateMeters on " + boost::lexical_cast<std::string>(object_ids.size()) + " objects");
    // cache all activation and scoping condition results before applying Effects, since the application of
    // these Effects may affect the activation and scoping evaluations
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes, object_ids);

    std::vector<UniverseObject*> objects = m_objects.FindObjects(object_ids);

    // revert all current meter values (which are modified by effects) to
    // their initial state for this turn, so meter
    // value can be calculated (by accumulating all effects' modifications this
    // turn) and active meters have the proper baseline from which to
    // accumulate changes from effects
    for (std::vector<UniverseObject*>::iterator it = objects.begin(); it != objects.end(); ++it) {
        (*it)->ResetTargetMaxUnpairedMeters();
        (*it)->ResetPairedActiveMeters();
    }
    // could also reset empire meters here, but unless all objects have meters
    // recalculated, some targets that lead to empire meters being modified may
    // be missed, and estimated empire meters would be inaccurate

    ExecuteEffects(targets_causes, true, true);

    for (std::vector<UniverseObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
        (*it)->ClampMeters();  // clamp max, target and unpaired meters to [DEFAULT_VALUE, LARGE_VALUE] and active meters with max meters to [DEFAULT_VALUE, max]
}

void Universe::ApplyMeterEffectsAndUpdateMeters() {
    ScopedTimer timer("Universe::ApplyMeterEffectsAndUpdateMeters on all objects");

    std::vector<int> object_ids = m_objects.FindObjectIDs();
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes, object_ids);

    std::vector<UniverseObject*> objects = m_objects.FindObjects(object_ids);

    for (std::vector<UniverseObject*>::iterator it = objects.begin(); it != objects.end(); ++it) {
        (*it)->ResetTargetMaxUnpairedMeters();
        (*it)->ResetPairedActiveMeters();
    }
    for (EmpireManager::iterator it = Empires().begin(); it != Empires().end(); ++it)
        it->second->ResetMeters();
    ExecuteEffects(targets_causes, true, true, false, true);

    for (std::vector<UniverseObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
        (*it)->ClampMeters();  // clamp max, target and unpaired meters to [DEFAULT_VALUE, LARGE_VALUE] and active meters with max meters to [DEFAULT_VALUE, max]
}

void Universe::ApplyAppearanceEffects(const std::vector<int>& object_ids) {
    ScopedTimer timer("Universe::ApplyAppearanceEffects on " + boost::lexical_cast<std::string>(object_ids.size()) + " objects");

    // cache all activation and scoping condition results before applying
    // Effects, since the application of these Effects may affect the
    // activation and scoping evaluations
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes, object_ids);
    ExecuteEffects(targets_causes, false, false, true);
}

void Universe::ApplyAppearanceEffects()
{ ApplyAppearanceEffects(m_objects.FindObjectIDs()); }

void Universe::InitMeterEstimatesAndDiscrepancies() {
    Logger().debugStream() << "Universe::InitMeterEstimatesAndDiscrepancies";
    ScopedTimer timer("Universe::InitMeterEstimatesAndDiscrepancies");

    // clear old discrepancies and accounting
    m_effect_discrepancy_map.clear();
    m_effect_accounting_map.clear();

    //Logger().debugStream() << "Universe::InitMeterEstimatesAndDiscrepancies";

    // generate new estimates (normally uses discrepancies, but in this case will find none)
    UpdateMeterEstimates();

    // determine meter max discrepancies
    for (Effect::AccountingMap::iterator obj_it = m_effect_accounting_map.begin();
         obj_it != m_effect_accounting_map.end(); ++obj_it)
    {
        int object_id = obj_it->first;
        // skip destroyed objects
        if (m_destroyed_object_ids.find(object_id) != m_destroyed_object_ids.end())
            continue;
        // get object
        UniverseObject* obj = m_objects.Object(object_id);
        if (!obj) {
            Logger().errorStream() << "Universe::InitMeterEstimatesAndDiscrepancies couldn't find an object that was in the effect accounting map...?";
            continue;
        }

        // every meter has a value at the start of the turn, and a value after updating with known effects
        for (std::map<MeterType, Meter>::iterator meter_it = obj->Meters().begin();
             meter_it != obj->Meters().end(); ++meter_it)
        {
            MeterType type = meter_it->first;
            Meter& meter = meter_it->second;

            // discrepancy is the difference between expected and actual meter values at start of turn
            double discrepancy = meter.Initial() - meter.Current();

            if (discrepancy == 0.0) continue;   // no discrepancy for this meter

            // add to discrepancy map
            m_effect_discrepancy_map[object_id][type] = discrepancy;

            // correct current max meter estimate for discrepancy
            meter.AddToCurrent(discrepancy);

            // add discrepancy adjustment to meter accounting
            Effect::AccountingInfo info;
            info.cause_type = ECT_UNKNOWN_CAUSE;
            info.meter_change = discrepancy;
            info.running_meter_total = meter.Current();

            m_effect_accounting_map[object_id][type].push_back(info);
        }
    }
}

void Universe::UpdateMeterEstimates()
{ UpdateMeterEstimates(INVALID_OBJECT_ID, false); }

void Universe::UpdateMeterEstimates(int object_id, bool update_contained_objects) {
    if (object_id == INVALID_OBJECT_ID) {
        // update meters for all objects.  Value of updated_contained_objects is irrelivant and is ignored in this case.
        UpdateMeterEstimates(m_objects.FindObjectIDs());
        return;
    }

    // collect objects to update meter for.  this may be a single object, a group of related objects, or all objects
    // in the (known) universe.  also clear effect accounting for meters that are to be updated.
    std::set<int> objects_set;
    std::list<int> objects_list;
    objects_list.push_back(object_id);

    for (std::list<int>::iterator list_it = objects_list.begin(); list_it !=  objects_list.end(); ++list_it) {
        // get next object id on list
        int cur_object_id = *list_it;
        // get object
        UniverseObject* cur_object = m_objects.Object(cur_object_id);
        if (!cur_object) {
            Logger().errorStream() << "Universe::UpdateMeterEstimates tried to get an invalid object...";
            return;
        }

        // add object and clear effect accounting for all its meters
        objects_set.insert(cur_object_id);
        m_effect_accounting_map[cur_object_id].clear();

        // add contained objects to list of objects to process, if requested.  assumes no objects contain themselves (which could cause infinite loops)
        if (update_contained_objects) {
            std::vector<int> contained_objects = cur_object->FindObjectIDs(); // get all contained objects
            std::copy(contained_objects.begin(), contained_objects.end(), std::back_inserter(objects_list));
        }
    }
    std::vector<int> objects_vec;
    std::copy(objects_set.begin(), objects_set.end(), std::back_inserter(objects_vec));
    UpdateMeterEstimatesImpl(objects_vec);
}

void Universe::UpdateMeterEstimates(const std::vector<int>& objects_vec) {
    std::set<int> objects_set;  // ensures no duplicates

    for (std::vector<int>::const_iterator obj_it = objects_vec.begin(); obj_it != objects_vec.end(); ++obj_it) {
        int object_id = *obj_it;
        // skip destroyed objects
        if (m_destroyed_object_ids.find(object_id) != m_destroyed_object_ids.end())
            continue;
        m_effect_accounting_map[object_id].clear();
        objects_set.insert(object_id);
    }
    std::vector<int> final_objects_vec;
    std::copy(objects_set.begin(), objects_set.end(), std::back_inserter(final_objects_vec));
    UpdateMeterEstimatesImpl(final_objects_vec);
}

void Universe::UpdateMeterEstimatesImpl(const std::vector<int>& objects_vec) {
    ScopedTimer timer("Universe::UpdateMeterEstimatesImpl on " + boost::lexical_cast<std::string>(objects_vec.size()) + " objects", true);
    if (objects_vec.empty())
        return;

    for (std::vector<int>::const_iterator obj_it = objects_vec.begin(); obj_it != objects_vec.end(); ++obj_it) {
        int obj_id = *obj_it;
        UniverseObject* obj = m_objects.Object(obj_id);

        // Reset max meters to DEFAULT_VALUE and current meters to initial value at start of this turn
        obj->ResetTargetMaxUnpairedMeters();
        obj->ResetPairedActiveMeters();

        // record current value(s) of meters after resetting
        for (MeterType type = MeterType(0); type != NUM_METER_TYPES; type = MeterType(type + 1)) {
            if (Meter* meter = obj->GetMeter(type)) {
                Effect::AccountingInfo info;
                info.source_id = INVALID_OBJECT_ID;
                info.cause_type = ECT_INHERENT;
                info.meter_change = meter->Current() - Meter::DEFAULT_VALUE;
                info.running_meter_total = meter->Current();

                if (info.meter_change > 0.0)
                    m_effect_accounting_map[obj_id][type].push_back(info);
            }
        }
    }

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        Logger().debugStream() << "UpdateMeterEstimatesImpl after resetting meters objects:";
        for (std::vector<int>::const_iterator it = objects_vec.begin(); it != objects_vec.end(); ++it) {
            if (const UniverseObject* obj = GetUniverseObject(*it))
                Logger().debugStream() << obj->Dump();
        }
    }

    // cache all activation and scoping condition results before applying Effects, since the application of
    // these Effects may affect the activation and scoping evaluations
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes, objects_vec);

    // Apply and record effect meter adjustments
    ExecuteEffects(targets_causes, true, true, false, false);

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        Logger().debugStream() << "UpdateMeterEstimatesImpl after executing effects objects:";
        for (std::vector<int>::const_iterator it = objects_vec.begin(); it != objects_vec.end(); ++it) {
            if (const UniverseObject* obj = GetUniverseObject(*it))
                Logger().debugStream() << obj->Dump();
        }
    }

    // Apply known discrepancies between expected and calculated meter maxes at start of turn.  This
    // accounts for the unknown effects on the meter, and brings the estimate in line with the actual
    // max at the start of the turn
    if (!m_effect_discrepancy_map.empty()) {
        for (std::vector<int>::const_iterator obj_it = objects_vec.begin(); obj_it != objects_vec.end(); ++obj_it) {
            int obj_id = *obj_it;
            UniverseObject* obj = m_objects.Object(obj_id);

            // check if this object has any discrepancies
            Effect::DiscrepancyMap::iterator dis_it = m_effect_discrepancy_map.find(obj_id);
            if (dis_it == m_effect_discrepancy_map.end())
                continue;   // no discrepancy, so skip to next object

            // apply all meters' discrepancies
            std::map<MeterType, double>& meter_map = dis_it->second;
            for(std::map<MeterType, double>::iterator meter_it = meter_map.begin(); meter_it != meter_map.end(); ++meter_it) {
                MeterType type = meter_it->first;
                double discrepancy = meter_it->second;

                //if (discrepancy == 0.0) continue;

                Meter* meter = obj->GetMeter(type);

                if (meter) {
                    if (GetOptionsDB().Get<bool>("verbose-logging"))
                        Logger().debugStream() << "object " << obj_id << " has meter " << type << ": discrepancy: " << discrepancy << " and : " << meter->Dump();

                    meter->AddToCurrent(discrepancy);

                    Effect::AccountingInfo info;
                    info.cause_type = ECT_UNKNOWN_CAUSE;
                    info.meter_change = discrepancy;
                    info.running_meter_total = meter->Current();

                    m_effect_accounting_map[obj_id][type].push_back(info);
                }
            }
        }
    }

    // clamp meters to valid range of max values, and so current is less than max
    for (std::vector<int>::const_iterator obj_it = objects_vec.begin(); obj_it != objects_vec.end(); ++obj_it) {
        // currently this clamps all meters, even if not all meters are being processed by this function...
        // but that shouldn't be a problem, as clamping meters that haven't changed since they were last
        // updated should have no effect
        m_objects.Object(*obj_it)->ClampMeters();
    }

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        Logger().debugStream() << "UpdateMeterEstimatesImpl after discrepancies and clamping objects:";
        for (std::vector<int>::const_iterator it = objects_vec.begin(); it != objects_vec.end(); ++it) {
            if (const UniverseObject* obj = GetUniverseObject(*it))
                Logger().debugStream() << obj->Dump();
        }
    }
}

void Universe::BackPropegateObjectMeters(const std::vector<int>& object_ids) {
    std::vector<UniverseObject*> objects = m_objects.FindObjects(object_ids);

    // copy current meter values to initial values
    for (std::vector<UniverseObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
        (*it)->BackPropegateMeters();
}

void Universe::BackPropegateObjectMeters()
{ BackPropegateObjectMeters(m_objects.FindObjectIDs()); }

void Universe::GetEffectsAndTargets(Effect::TargetsCauses& targets_causes) {
    targets_causes.clear();

    std::vector<int> all_objects = m_objects.FindObjectIDs();
    GetEffectsAndTargets(targets_causes, all_objects);
}

void Universe::GetEffectsAndTargets(Effect::TargetsCauses& targets_causes,
                                    const std::vector<int>& target_objects)
{
    ScopedTimer timer("Universe::GetEffectsAndTargets");
    if (target_objects.empty())
        return;

    // transfer target objects from input vector to a set
    Effect::TargetSet all_potential_targets;
    all_potential_targets.reserve(target_objects.size());
    for (std::vector<int>::const_iterator it = target_objects.begin(); it != target_objects.end(); ++it)
        all_potential_targets.push_back(m_objects.Object(*it));

    Logger().debugStream() << "Universe::GetEffectsAndTargets";
    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        Logger().debugStream() << "target objects:";
        for (std::vector<int>::const_iterator it = target_objects.begin(); it != target_objects.end(); ++it) {
            if (const UniverseObject* obj = GetUniverseObject(*it))
                Logger().debugStream() << obj->Dump();
        }
    }

    boost::timer type_timer;


    // caching space for each source object's results of finding matches for
    // scope conditions. Index INVALID_OBJECT_ID stores results for
    // source-invariant conditions
    std::map<int, std::map<const Condition::ConditionBase*, Effect::TargetSet> >
        cached_source_condition_matches;
    std::map<const Condition::ConditionBase*, Effect::TargetSet>& invariant_condition_matches =
        cached_source_condition_matches[INVALID_OBJECT_ID];


    // 1) EffectsGroups from Species
    if (GetOptionsDB().Get<bool>("verbose-logging"))
        Logger().debugStream() << "Universe::GetEffectsAndTargets for SPECIES";
    type_timer.restart();
    std::vector<Planet*> planets = m_objects.FindObjects<Planet>();
    for (std::vector<Planet*>::const_iterator planet_it = planets.begin();
         planet_it != planets.end(); ++planet_it)
    {
        const Planet* planet = *planet_it;
        if (m_destroyed_object_ids.find(planet->ID()) != m_destroyed_object_ids.end())
            continue;
        const std::string& species_name = planet->SpeciesName();
        if (species_name.empty())
            continue;
        const Species* species = GetSpecies(species_name);
        if (!species) {
            Logger().errorStream() << "GetEffectsAndTargets couldn't get Species " << species_name;
            continue;
        }
        StoreTargetsAndCausesOfEffectsGroups(species->Effects(), planet, ECT_SPECIES, species_name,
                                             all_potential_targets, targets_causes,
                                             cached_source_condition_matches[planet->ID()],
                                             invariant_condition_matches);
    }
    double planet_species_time = type_timer.elapsed();
    type_timer.restart();

    std::vector<Ship*> ships = m_objects.FindObjects<Ship>();
    for (std::vector<Ship*>::const_iterator ship_it = ships.begin();
         ship_it != ships.end(); ++ship_it)
    {
        const Ship* ship = *ship_it;
        if (m_destroyed_object_ids.find(ship->ID()) != m_destroyed_object_ids.end())
            continue;
        const std::string& species_name = ship->SpeciesName();
        if (species_name.empty())
            continue;
        const Species* species = GetSpecies(species_name);
        if (!species) {
            Logger().errorStream() << "GetEffectsAndTargets couldn't get Species " << species_name;
            continue;
        }
        StoreTargetsAndCausesOfEffectsGroups(species->Effects(), ship, ECT_SPECIES, species_name,
                                             all_potential_targets, targets_causes,
                                             cached_source_condition_matches[ship->ID()],
                                             invariant_condition_matches);
    }
    double ship_species_time = type_timer.elapsed();
    type_timer.restart();

    // 2) EffectsGroups from Specials
    if (GetOptionsDB().Get<bool>("verbose-logging"))
        Logger().debugStream() << "Universe::GetEffectsAndTargets for SPECIALS";
    type_timer.restart();
    for (ObjectMap::const_iterator<> it = m_objects.const_begin(); it != m_objects.const_end(); ++it) {
        const UniverseObject* obj = *it;
        int source_object_id = obj->ID();
        if (m_destroyed_object_ids.find(source_object_id) != m_destroyed_object_ids.end())
            continue;
        const std::map<std::string, int>& specials = it->Specials();
        for (std::map<std::string, int>::const_iterator special_it = specials.begin(); special_it != specials.end(); ++special_it) {
            const Special* special = GetSpecial(special_it->first);
            if (!special) {
                Logger().errorStream() << "GetEffectsAndTargets couldn't get Special " << special_it->first;
                continue;
            }

            StoreTargetsAndCausesOfEffectsGroups(special->Effects(), obj, ECT_SPECIAL, special->Name(),
                                                 all_potential_targets, targets_causes,
                                                 cached_source_condition_matches[source_object_id],
                                                 invariant_condition_matches);
        }
    }
    double special_time = type_timer.elapsed();

    // 3) EffectsGroups from Techs
    if (GetOptionsDB().Get<bool>("verbose-logging"))
        Logger().debugStream() << "Universe::GetEffectsAndTargets for TECHS";
    type_timer.restart();
    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        const Empire* empire = it->second;
        int source_id = empire->CapitalID();
        const UniverseObject* source = m_objects.Object(source_id);
        if (source_id == INVALID_OBJECT_ID ||
            !source ||
            !source->Unowned() ||
            !source->OwnedBy(empire->EmpireID()))
        {
            // find alternate object owned by this empire to act as source
            // first try to get a planet
            std::vector<UniverseObject*> empire_planets = m_objects.FindObjects(OwnedVisitor<Planet>(empire->EmpireID()));
            if (!empire_planets.empty()) {
                source = *empire_planets.begin();
                source_id = source->ID();
            } else {
                // if no planet, use any owned object
                std::vector<UniverseObject*> empire_objects = m_objects.FindObjects(OwnedVisitor<UniverseObject>(empire->EmpireID()));
                if (!empire_objects.empty()) {
                    source = *empire_objects.begin();
                    source_id = source->ID();
                } else {
                    continue;   // can't do techs for this empire
                }
            }
        }
        for (Empire::TechItr tech_it = empire->TechBegin(); tech_it != empire->TechEnd(); ++tech_it) {
            const Tech* tech = GetTech(*tech_it);
            if (!tech) continue;
            if (!source) {
                Logger().errorStream() << "somehow to to storing targets and causes of tech effectsgroup without a source...?";
                break;
            }
            StoreTargetsAndCausesOfEffectsGroups(tech->Effects(), source, ECT_TECH, tech->Name(),
                                                 all_potential_targets, targets_causes,
                                                 cached_source_condition_matches[source_id],
                                                 invariant_condition_matches);
        }
    }
    double tech_time = type_timer.elapsed();

    // 4) EffectsGroups from Buildings
    if (GetOptionsDB().Get<bool>("verbose-logging"))
        Logger().debugStream() << "Universe::GetEffectsAndTargets for BUILDINGS";
    type_timer.restart();
    std::vector<Building*> buildings = m_objects.FindObjects<Building>();
    for (std::vector<Building*>::const_iterator building_it = buildings.begin();
         building_it != buildings.end(); ++building_it)
    {
        const Building* building = *building_it;
        if (m_destroyed_object_ids.find(building->ID()) != m_destroyed_object_ids.end())
            continue;
        const BuildingType* building_type = GetBuildingType(building->BuildingTypeName());
        if (!building_type) {
            Logger().errorStream() << "GetEffectsAndTargets couldn't get BuildingType " << building->BuildingTypeName();
            continue;
        }

        StoreTargetsAndCausesOfEffectsGroups(building_type->Effects(), building,
                                             ECT_BUILDING, building_type->Name(),
                                             all_potential_targets, targets_causes,
                                             cached_source_condition_matches[building->ID()],
                                             invariant_condition_matches);
    }
    double building_time = type_timer.elapsed();

    // 5) EffectsGroups from Ship Hull and Ship Parts
    if (GetOptionsDB().Get<bool>("verbose-logging"))
        Logger().debugStream() << "Universe::GetEffectsAndTargets for SHIPS hulls and parts";
    type_timer.restart();
    ships = m_objects.FindObjects<Ship>();
    for (std::vector<Ship*>::const_iterator ship_it = ships.begin(); ship_it != ships.end(); ++ship_it) {
        const Ship* ship = *ship_it;
        if (m_destroyed_object_ids.find(ship->ID()) != m_destroyed_object_ids.end())
            continue;

        const ShipDesign* ship_design = ship->Design();
        if (!ship_design) {
            Logger().errorStream() << "GetEffectsAndTargets couldn't get ShipDesign";
            continue;
        }
        const HullType* hull_type = ship_design->GetHull();
        if (!hull_type) {
            Logger().errorStream() << "GetEffectsAndTargets couldn't get HullType";
            continue;
        }

        StoreTargetsAndCausesOfEffectsGroups(hull_type->Effects(), ship, ECT_SHIP_HULL,
                                             hull_type->Name(),
                                             all_potential_targets, targets_causes,
                                             cached_source_condition_matches[ship->ID()],
                                             invariant_condition_matches);

        const std::vector<std::string>& parts = ship_design->Parts();
        for (std::vector<std::string>::const_iterator part_it = parts.begin(); part_it != parts.end(); ++part_it) {
            const std::string& part = *part_it;
            if (part.empty())
                continue;
            const PartType* part_type = GetPartType(*part_it);
            if (!part_type) {
                Logger().errorStream() << "GetEffectsAndTargets couldn't get PartType";
                continue;
            }
            StoreTargetsAndCausesOfEffectsGroups(part_type->Effects(), ship, ECT_SHIP_PART,
                                                 part_type->Name(),
                                                 all_potential_targets, targets_causes,
                                                 cached_source_condition_matches[ship->ID()],
                                                 invariant_condition_matches);
        }
    }
    double ships_time = type_timer.elapsed();

    // 6) EffectsGroups from Fields
    if (GetOptionsDB().Get<bool>("verbose-logging"))
        Logger().debugStream() << "Universe::GetEffectsAndTargets for FIELDS";
    type_timer.restart();
    std::vector<Field*> fields = m_objects.FindObjects<Field>();
    for (std::vector<Field*>::const_iterator field_it = fields.begin(); field_it != fields.end(); ++field_it) {
        const Field* field = *field_it;
        if (m_destroyed_object_ids.find(field->ID()) != m_destroyed_object_ids.end())
            continue;

        const FieldType* field_type = GetFieldType(field->FieldTypeName());
        if (!field_type) {
            Logger().errorStream() << "GetEffectsAndTargets couldn't get FieldType " << field->FieldTypeName();
            continue;
        }

        StoreTargetsAndCausesOfEffectsGroups(field_type->Effects(), field, ECT_FIELD,
                                             field_type->Name(),
                                             all_potential_targets, targets_causes,
                                             cached_source_condition_matches[field->ID()],
                                             invariant_condition_matches);
    }
    double fields_time = type_timer.elapsed();

    Logger().debugStream() << "Type times: planet species: " << planet_species_time*1000
                           << " ship species: " << ship_species_time*1000
                           << " specials: " << special_time*1000
                           << " techs: " << tech_time*1000
                           << " buildings: " << building_time*1000
                           << " hulls/parts: " << ships_time*1000
                           << " fields: " << fields_time*1000;
}

namespace {
    Effect::TargetSet& GetConditionMatches(const Condition::ConditionBase* cond,
                                           std::map<const Condition::ConditionBase*, Effect::TargetSet>&
                                               cached_condition_matches,
                                           const UniverseObject* source,
                                           const ScriptingContext& source_context,
                                           Effect::TargetSet& target_objects)
    {
        if (!cond)
            return cached_condition_matches[0]; // empty TargetSet

        for (std::map<const Condition::ConditionBase*, Effect::TargetSet>::iterator
             it = cached_condition_matches.begin(); it != cached_condition_matches.end(); ++it)
        {
            if (*cond == *(it->first)) {
                //Logger().debugStream() << "Reused target set!";
                return it->second;
            }
        }

        // no cached result. calculate it...

        // set up storage in cache and to return
        Effect::TargetSet& target_set = cached_condition_matches[cond];
        Condition::ObjectSet& matched_target_objects =
            *static_cast<Condition::ObjectSet *>(static_cast<void *>(&target_set));
        Condition::ObjectSet& potential_target_objects =
            *static_cast<Condition::ObjectSet *>(static_cast<void *>(&target_objects));

        // move matches from candidates in target_objects into target_set
        cond->Eval(source_context, matched_target_objects, potential_target_objects);
        // restore target_objects by copying objects back from targets to target_objects
        // this should be cheaper than doing a full copy because target_set is usually small
        target_objects.insert(target_objects.end(), target_set.begin(), target_set.end());

        //Logger().debugStream() << "Generated new target set!";

        return target_set;
    }
}

void Universe::StoreTargetsAndCausesOfEffectsGroups(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups,
                                                    const UniverseObject* source,
                                                    EffectsCauseType effect_cause_type,
                                                    const std::string& specific_cause_name,
                                                    Effect::TargetSet& target_objects,
                                                    Effect::TargetsCauses& targets_causes,
                                                    std::map<const Condition::ConditionBase*, Effect::TargetSet>& source_cached_condition_matches,
                                                    std::map<const Condition::ConditionBase*, Effect::TargetSet>& invariant_cached_condition_matches)
{
    ScopedTimer timer("Universe::StoreTargetsAndCausesOfEffectsGroups");

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        int source_id = (source ? source->ID() : INVALID_OBJECT_ID);
        Logger().debugStream() << "Universe::StoreTargetsAndCausesOfEffectsGroups( , source id: " << source_id << ", , specific cause: " << specific_cause_name << ", , )";
    }


    ScriptingContext source_context(source);
    int source_object_id = (source ? source->ID() : INVALID_OBJECT_ID);


    // process all effects groups in set provided
    int eg_count = 1;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >::const_iterator effects_it;
    for (effects_it = effects_groups.begin(); effects_it != effects_groups.end(); ++effects_it) {
        ScopedTimer update_timer("... Universe::StoreTargetsAndCausesOfEffectsGroups done processing source " +
                                 boost::lexical_cast<std::string>(source_object_id) +
                                 " cause: " + specific_cause_name +
                                 " effects group " + boost::lexical_cast<std::string>(eg_count++));

        // get effects group to process for this iteration
        boost::shared_ptr<const Effect::EffectsGroup> effects_group = *effects_it;

        // skip inactive effects groups
        const Condition::ConditionBase* activation = effects_group->Activation();
        if (activation && !activation->Eval(source_context, source))
            continue;

        // get objects matched by scope
        const Condition::ConditionBase* scope = effects_group->Scope();
        if (!scope)
            continue;
        bool source_invariant = !source || scope->SourceInvariant();
        Effect::TargetSet& target_set = GetConditionMatches(scope,
                                                            source_invariant ?
                                                                invariant_cached_condition_matches :
                                                                source_cached_condition_matches,
                                                            source,
                                                            source_context,
                                                            target_objects);
        if (target_set.empty())
            continue;

        // combine effects group and source object id into a sourced effects group
        Effect::SourcedEffectsGroup sourced_effects_group(source_object_id, effects_group);

        // combine cause type and specific cause into effect cause
        Effect::EffectCause effect_cause(effect_cause_type, specific_cause_name,
                                         effects_group->AccountingLabel());

        // combine target set and effect cause
        Effect::TargetsAndCause target_and_cause(target_set, effect_cause);

        // store effect cause and targets info in map, indexed by sourced effects group
        targets_causes.push_back(std::make_pair(sourced_effects_group, target_and_cause));
    }
}

void Universe::ExecuteEffects(const Effect::TargetsCauses& targets_causes,
                              bool update_effect_accounting,
                              bool only_meter_effects/* = false*/,
                              bool only_appearance_effects/* = false*/,
                              bool include_empire_meter_effects/* = false*/)
{
    ScopedTimer timer("Universe::ExecuteEffects", true);

    m_marked_destroyed.clear();
    m_marked_for_victory.clear();
    std::map<std::string, Effect::TargetSet> executed_nonstacking_effects;

    for (Effect::TargetsCauses::const_iterator targets_it = targets_causes.begin(); targets_it != targets_causes.end(); ++targets_it) {
        const UniverseObject* source = GetUniverseObject(targets_it->first.source_object_id);
        ScopedTimer update_timer("Universe::ExecuteEffects effgrp (source " +
                                 (source ? source->Name() : "No Source!") +
                                 ") on " + boost::lexical_cast<std::string>(targets_it->second.target_set.size()) + " objects");

        // if other EffectsGroups with the same stacking group have affected some of the targets in
        // the scope of the current EffectsGroup, skip them
        const Effect::SourcedEffectsGroup& sourced_effects_group = targets_it->first;
        const boost::shared_ptr<const Effect::EffectsGroup> effects_group = sourced_effects_group.effects_group;
        const Effect::TargetsAndCause& targets_and_cause = targets_it->second;
        Effect::TargetSet targets = targets_and_cause.target_set;
        if (targets.empty())
            continue;

        std::map<std::string, Effect::TargetSet>::iterator non_stacking_it = executed_nonstacking_effects.find(effects_group->StackingGroup());
        if (non_stacking_it != executed_nonstacking_effects.end()) {
            for (Effect::TargetSet::const_iterator object_it = non_stacking_it->second.begin();
                 object_it != non_stacking_it->second.end(); ++object_it)
            {
                Effect::TargetSet::iterator it = std::find(targets.begin(), targets.end(), *object_it);
                if (it != targets.end()) {
                    *it = targets.back();
                    targets.pop_back();
                }
            }
        }
        if (targets.empty())
            continue;
        Effect::TargetsAndCause filtered_targets_and_cause(targets, targets_and_cause.effect_cause);

        if (GetOptionsDB().Get<bool>("verbose-logging")) {
            Logger().debugStream() << "ExecuteEffects effectsgroup: \n" << effects_group->Dump();
            Logger().debugStream() << "ExecuteEffects Targets before: ";
            for (Effect::TargetSet::const_iterator t_it = targets.begin(); t_it != targets.end(); ++t_it)
                Logger().debugStream() << " ... " << (*t_it)->Dump();
        }

        // execute Effects in the EffectsGroup
        if (only_appearance_effects) {
            effects_group->ExecuteAppearanceModifications(  sourced_effects_group.source_object_id, filtered_targets_and_cause.target_set);
        } else if (update_effect_accounting && only_meter_effects) {
            effects_group->ExecuteSetMeter(                 sourced_effects_group.source_object_id, filtered_targets_and_cause,     m_effect_accounting_map);
            if (include_empire_meter_effects)
                effects_group->ExecuteSetEmpireMeter(       sourced_effects_group.source_object_id, filtered_targets_and_cause.target_set);
        } else if (only_meter_effects) {
            effects_group->ExecuteSetMeter(                 sourced_effects_group.source_object_id, filtered_targets_and_cause.target_set);
            if (include_empire_meter_effects)
                effects_group->ExecuteSetEmpireMeter(       sourced_effects_group.source_object_id, filtered_targets_and_cause.target_set);
        } else if (update_effect_accounting) {
            effects_group->Execute(                         sourced_effects_group.source_object_id, filtered_targets_and_cause,     m_effect_accounting_map);
        } else {
            effects_group->Execute(                         sourced_effects_group.source_object_id, filtered_targets_and_cause.target_set);
        }

        if (GetOptionsDB().Get<bool>("verbose-logging")) {
            Logger().debugStream() << "ExecuteEffects Targets after: ";
            for (Effect::TargetSet::const_iterator t_it = targets.begin(); t_it != targets.end(); ++t_it)
                Logger().debugStream() << " ... " << (*t_it)->Dump();
            Logger().debugStream() << "         * * * * * *";
        }

        // if this EffectsGroup belongs to a stacking group, add the objects just affected by it to executed_nonstacking_effects
        if (!effects_group->StackingGroup().empty()) {
            Effect::TargetSet& affected_targets = executed_nonstacking_effects[effects_group->StackingGroup()];
            for (Effect::TargetSet::const_iterator object_it = targets.begin(); object_it != targets.end(); ++object_it) {
                affected_targets.push_back(*object_it);
            }
        }
    }

    // actually do destroy effect action.  Executing the effect just marks
    // objects to be destroyed, but doesn't actually do so in order to ensure
    // no interaction in order of effects and source or target objects being
    // destroyed / deleted between determining target sets and executing effects
    for (std::set<int>::iterator it = m_marked_destroyed.begin(); it != m_marked_destroyed.end(); ++it)
        RecursiveDestroy(*it);
}

void Universe::SetEmpireObjectVisibility(int empire_id, int object_id, Visibility vis) {
    if (empire_id == ALL_EMPIRES || object_id == INVALID_OBJECT_ID)
        return;

    // get visibility map for empire and find object in it
    Universe::ObjectVisibilityMap& vis_map = m_empire_object_visibility[empire_id];
    Universe::ObjectVisibilityMap::iterator vis_map_it = vis_map.find(object_id);

    // if object not already present, store default value (which may be replaced)
    if (vis_map_it == vis_map.end()) {
        vis_map[object_id] = VIS_NO_VISIBILITY;

        // get iterator pointing at newly-created entry
        vis_map_it = vis_map.find(object_id);
    }

    // increase stored value if new visibility is higher than last recorded
    if (vis > vis_map_it->second)
        vis_map_it->second = vis;

    // if object is a ship, empire also gets knowledge of its design
    if (vis >= VIS_PARTIAL_VISIBILITY) {
        if (const Ship* ship = GetShip(object_id)) {
            int design_id = ship->DesignID();
            if (design_id == ShipDesign::INVALID_DESIGN_ID) {
                Logger().errorStream() << "SetEmpireObjectVisibility got invalid design id for ship with id " << object_id;
            } else {
                m_empire_known_ship_design_ids[empire_id].insert(design_id);
            }
        }
    }
}

void Universe::SetEmpireSpecialVisibility(int empire_id, int object_id,
                                          const std::string& special_name,
                                          bool visible/* = true*/)
{
    if (empire_id == ALL_EMPIRES || special_name.empty() || object_id == INVALID_OBJECT_ID)
        return;
    //const UniverseObject* obj = GetUniverseObject(object_id);
    //if (!obj)
    //    return;
    //if (!obj->HasSpecial(special_name))
    //    return;
    if (visible)
        m_empire_object_visible_specials[empire_id][object_id].insert(special_name);
    else
        m_empire_object_visible_specials[empire_id][object_id].erase(special_name);
}

namespace {
    /** for each empire: for each position where the empire has detector objects,
      * what is the empire's detection range at that location?  (this is the
      * largest of the detection ranges of objects the empire has at that spot) */
    std::map<int, std::map<std::pair<double, double>, float> >
        GetEmpiresPositionDetectionRanges()
    {
        std::map<int, std::map<std::pair<double, double>, float> > retval;

        for (ObjectMap::const_iterator<> object_it = Objects().const_begin();
                object_it != Objects().const_end(); ++object_it)
        {
            // skip unowned objects, which can't provide detection to any empire
            const UniverseObject* obj = *object_it;
            if (obj->Unowned())
                continue;

            // skip objects with no detection range
            const Meter* detection_meter = obj->GetMeter(METER_DETECTION);
            if (!detection_meter)
                continue;
            float object_detection_range = detection_meter->Current();
            if (object_detection_range <= 0.0f)
                continue;

            // don't allow moving ships / fleets to give detection
            const Fleet* fleet = 0;
            if (obj->ObjectType() == OBJ_FLEET) {
                fleet = universe_object_cast<const Fleet*>(obj);
            } else if (obj->ObjectType() == OBJ_SHIP) {
                const Ship* ship = universe_object_cast<const Ship*>(obj);
                if (ship)
                    fleet = Objects().Object<Fleet>(ship->FleetID());
            }
            if (fleet) {
                int cur_id = fleet->SystemID();
                if (cur_id == INVALID_OBJECT_ID) // fleets do not grant detection when in a starlane
                    continue;
            }

            // record object's detection range for owner
            int object_owner_empire_id = obj->Owner();
            std::pair<double, double> object_pos(obj->X(), obj->Y());
            // store range in output map (if new for location or larger than any
            // previously-found range at this location)
            std::map<std::pair<double, double>, float>& retval_empire_pos_range = retval[object_owner_empire_id];
            std::map<std::pair<double, double>, float>::iterator retval_pos_it = retval_empire_pos_range.find(object_pos);
            if (retval_pos_it == retval_empire_pos_range.end())
                retval_empire_pos_range[object_pos] = object_detection_range;
            else
                retval_pos_it->second = std::max(retval_pos_it->second, object_detection_range);
        }
        return retval;
    }

    std::map<int, float> GetEmpiresDetectionStrengths(int empire_id = ALL_EMPIRES) {
        std::map<int, float> retval;
        if (empire_id == ALL_EMPIRES) {
            for (EmpireManager::iterator empire_it = Empires().begin();
                 empire_it != Empires().end(); ++empire_it)
            {
                const Meter* meter = empire_it->second->GetMeter("METER_DETECTION_STRENGTH");
                float strength = meter ? meter->Current() : 0.0f;
                retval[empire_it->first] = strength;
            }
        } else {
            if (const Empire* empire = Empires().Lookup(empire_id))
                if (const Meter* meter = empire->GetMeter("METER_DETECTION_STRENGTH"))
                    retval[empire_id] = meter->Current();
        }
        return retval;
    }

    /** for each empire: for each position, what objects have low enough stealth
      * that the empire could detect them if an object owned by the empire is in
      * range? */
    std::map<int, std::map<std::pair<double, double>, std::vector<int> > >
        GetEmpiresPositionsPotentiallyDetectableObjects(const ObjectMap& objects,
                                                        int empire_id = ALL_EMPIRES)
    {
        std::map<int, std::map<std::pair<double, double>, std::vector<int> > > retval;

        std::map<int, float> empire_detection_strengths = GetEmpiresDetectionStrengths(empire_id);

        // filter objects as detectors for this empire or detectable objects
        for (ObjectMap::const_iterator<> object_it = objects.const_begin();
             object_it != objects.const_end(); ++object_it)
        {
            const UniverseObject* obj = *object_it;
            int object_id = object_it->ID();
            const Meter* stealth_meter = obj->GetMeter(METER_STEALTH);
            if (!stealth_meter)
                continue;
            float object_stealth = stealth_meter->Current();
            std::pair<double, double> object_pos(obj->X(), obj->Y());
            // check if each empire might be able to detect object, and if so,
            // add it to the returned list
            for (std::map<int, float>::const_iterator empire_it = empire_detection_strengths.begin();
                 empire_it != empire_detection_strengths.end(); ++empire_it)
            {
                int empire_id = empire_it->first;
                if (object_stealth <= empire_it->second || object_stealth == 0.0f || obj->OwnedBy(empire_id))
                    retval[empire_id][object_pos].push_back(object_id);
            }
        }
        return retval;
    }

    /** filters set of objects at locations by which of those locations are
      * within range of a set of detectors and ranges */
    std::vector<int> FilterObjectPositionsByDetectorPositionsAndRanges(
        const std::map<std::pair<double, double>, std::vector<int> >& object_positions,
        const std::map<std::pair<double, double>, float>& detector_position_ranges)
    {
        std::vector<int> retval;
        // check each detector position and range against each object position
        for (std::map<std::pair<double, double>, std::vector<int> >::const_iterator
             object_position_it = object_positions.begin();
             object_position_it != object_positions.end();
             ++object_position_it)
        {
            const std::pair<double, double>& object_pos = object_position_it->first;
            const std::vector<int>& objects = object_position_it->second;
            // search through detector positions until one is found in range
            for (std::map<std::pair<double, double>, float>::const_iterator
                 detector_position_it = detector_position_ranges.begin();
                 detector_position_it != detector_position_ranges.end();
                 ++detector_position_it)
            {
                // check range for this detector location for this detectables location
                float detector_range2 = detector_position_it->second * detector_position_it->second;
                const std::pair<double, double>& detector_pos = detector_position_it->first;
                double x_dist = detector_pos.first - object_pos.first;
                double y_dist = detector_pos.second - object_pos.second;
                double dist2 = x_dist*x_dist + y_dist*y_dist;
                if (dist2 > detector_range2)
                    continue;   // object out of range
                // add objects at position to return value
                std::copy(objects.begin(), objects.end(), std::back_inserter(retval));
                break;  // done searching for a detector position in range
            }
        }
        return retval;
    }

    /** removes ids of objects that the indicated empire knows have been
      * destroyed */
    void FilterObjectIDsByKnownDestruction(std::vector<int>& object_ids, int empire_id,
                                           const std::map<int, std::set<int> >&
                                               empire_known_destroyed_object_ids) {
        if (empire_id == ALL_EMPIRES)
            return;
        for (std::vector<int>::iterator it = object_ids.begin(); it != object_ids.end();) {
            int object_id = *it;
            std::map<int, std::set<int> >::const_iterator obj_it =
                empire_known_destroyed_object_ids.find(object_id);
            if (obj_it == empire_known_destroyed_object_ids.end()) {
                ++it;
                continue;
            }
            const std::set<int>& empires_that_know = obj_it->second;
            if (empires_that_know.find(empire_id) == empires_that_know.end()) {
                ++it;
                continue;
            }
            // remove object from vector...
            *it = object_ids.back();
            object_ids.pop_back();
        }
    }

    /** sets visibility of field objects for empires based on input locations
      * and stealth of fields in supplied ObjectMap and input empire detection
      * ranges at locations. the rules for detection of fields are more
      * permissive than other object types, so a special function for them is
      * needed in addition to SetEmpireObjectVisibilitiesFromRanges(...) */
    void SetEmpireFieldVisibilitiesFromRanges(
        const std::map<int, std::map<std::pair<double, double>, float> >&
            empire_location_detection_ranges,
        const ObjectMap& objects)
    {
        std::vector<const Field*> fields = objects.FindObjects<Field>();
        Universe& universe = GetUniverse();

        for (std::map<int, std::map<std::pair<double, double>, float> >::const_iterator
             detecting_empire_it = empire_location_detection_ranges.begin();
             detecting_empire_it != empire_location_detection_ranges.end();
             ++detecting_empire_it)
        {
            int detecting_empire_id = detecting_empire_it->first;
            double detection_strength = 0.0;
            const Empire* empire = Empires().Lookup(detecting_empire_id);
            if (!empire)
                continue;
            const Meter* meter = empire->GetMeter("METER_DETECTION_STRENGTH");
            if (!meter)
                continue;
            detection_strength = meter->Current();

            // get empire's locations of detection ranges
            const std::map<std::pair<double, double>, float>& detector_position_ranges =
                detecting_empire_it->second;

            // for each field, try to find a detector position in range for this empire
            for (ObjectMap::const_iterator<Field> field_it = objects.const_begin<Field>();
                 field_it != objects.const_end<Field>(); ++field_it)
            {
                const Field* field = *field_it;
                if (field->GetMeter(METER_STEALTH)->Current() > detection_strength)
                    continue;
                double field_size = field->GetMeter(METER_SIZE)->Current();
                const std::pair<double, double> object_pos(field->X(), field->Y());

                // search through detector positions until one is found in range
                for (std::map<std::pair<double, double>, float>::const_iterator
                     detector_position_it = detector_position_ranges.begin();
                     detector_position_it != detector_position_ranges.end();
                     ++detector_position_it)
                {
                    // check range for this detector location, for field of this
                    // size, against distance between field and detector
                    float detector_range = detector_position_it->second;
                    const std::pair<double, double>& detector_pos = detector_position_it->first;
                    double x_dist = detector_pos.first - object_pos.first;
                    double y_dist = detector_pos.second - object_pos.second;
                    double dist = std::sqrt(x_dist*x_dist + y_dist*y_dist);
                    double effective_dist = dist - field_size;
                    if (effective_dist > detector_range)
                        continue;   // object out of range

                    universe.SetEmpireObjectVisibility(detecting_empire_id, field->ID(),
                                                       VIS_PARTIAL_VISIBILITY);
                }
            }
        }
    }

    /** sets visibility of objects for empires based on input locations of
      * potentially detectable objects (if in range) and and input empire
      * detection ranges at locations. */
    void SetEmpireObjectVisibilitiesFromRanges(
        const std::map<int, std::map<std::pair<double, double>, float> >&
            empire_location_detection_ranges,
        const std::map<int, std::map<std::pair<double, double>, std::vector<int> > >&
            empire_location_potentially_detectable_objects)
    {
        Universe& universe = GetUniverse();

        for (std::map<int, std::map<std::pair<double, double>, float> >::const_iterator
             detecting_empire_it = empire_location_detection_ranges.begin();
             detecting_empire_it != empire_location_detection_ranges.end();
             ++detecting_empire_it)
        {
            int detecting_empire_id = detecting_empire_it->first;
            // get empire's locations of detection ability
            const std::map<std::pair<double, double>, float>& detector_position_ranges =
                detecting_empire_it->second;
            // for this empire, get objects it could potentially detect
            const std::map<int, std::map<std::pair<double, double>, std::vector<int> > >::const_iterator
                empire_detectable_objects_it = empire_location_potentially_detectable_objects.find(detecting_empire_id);
            if (empire_detectable_objects_it == empire_location_potentially_detectable_objects.end())
                continue;   // empire can't detect anything!
            const std::map<std::pair<double, double>, std::vector<int> >& detectable_position_objects =
                empire_detectable_objects_it->second;
            if (detectable_position_objects.empty())
                continue;

            // filter potentially detectable objects by which are within range
            // of a detector
            std::vector<int> in_range_detectable_objects =
                FilterObjectPositionsByDetectorPositionsAndRanges(detectable_position_objects,
                                                                  detector_position_ranges);
            if (in_range_detectable_objects.empty())
                continue;

            // set all in-range detectable objects as partially visible (unless
            // any are already full vis, in which case do nothing)
            for (std::vector<int>::const_iterator detected_object_it = in_range_detectable_objects.begin();
                 detected_object_it != in_range_detectable_objects.end(); ++detected_object_it)
            {
                universe.SetEmpireObjectVisibility(detecting_empire_id, *detected_object_it,
                                                   VIS_PARTIAL_VISIBILITY);
            }
        }
    }

    /** sets visibility of objects that empires own for those objects */
    void SetEmpireOwnedObjectVisibilities() {
        Universe& universe = GetUniverse();
        for (ObjectMap::const_iterator<> object_it = Objects().const_begin();
                object_it != Objects().const_end(); ++object_it)
        {
            const UniverseObject* obj = *object_it;
            if (obj->Unowned())
                continue;
            universe.SetEmpireObjectVisibility(obj->Owner(), object_it->ID(), VIS_FULL_VISIBILITY);
        }
    }

    /** sets all objects visible to all empires */
    void SetAllObjectsVisibleToAllEmpires() {
        Universe& universe = GetUniverse();
        // set every object visible to all empires
        for (ObjectMap::const_iterator<> obj_it = Objects().const_begin();
             obj_it != Objects().const_end(); ++obj_it)
        {
            for (EmpireManager::iterator empire_it = Empires().begin();
                 empire_it != Empires().end(); ++empire_it)
            {
                // objects
                universe.SetEmpireObjectVisibility(empire_it->first, obj_it->ID(), VIS_FULL_VISIBILITY);
                // specials on objects
                const std::map<std::string, int>& specials = obj_it->Specials();
                for (std::map<std::string, int>::const_iterator special_it = specials.begin();
                     special_it != specials.end(); ++special_it)
                {
                    universe.SetEmpireSpecialVisibility(empire_it->first, obj_it->ID(), special_it->first);
                }
            }
        }
    }

    /** sets planets in system where an empire owns an object to be basically
      * visible, and those systems to be partially visible */
    void SetSameSystemPlanetsVisible(const ObjectMap& objects) {
        Universe& universe = GetUniverse();
        // map from empire ID to ID of systems where those empires own at least one object
        std::map<int, std::set<int> > empires_systems_with_owned_objects;
        // get systems where empires have owned objects
        for (ObjectMap::const_iterator<> it = objects.const_begin(); it != objects.const_end(); ++it) {
            const UniverseObject* obj = *it;
            if (obj->Unowned() || obj->SystemID() == INVALID_OBJECT_ID)
                continue;
            empires_systems_with_owned_objects[obj->Owner()].insert(obj->SystemID());
        }

        // set system visibility
        for (std::map<int, std::set<int> >::const_iterator it = empires_systems_with_owned_objects.begin();
             it != empires_systems_with_owned_objects.end(); ++it)
        {
            int empire_id = it->first;
            const std::set<int>& system_ids = it->second;
            for (std::set<int>::const_iterator sys_it = system_ids.begin();
                 sys_it != system_ids.end(); ++sys_it)
            { universe.SetEmpireObjectVisibility(empire_id, *sys_it, VIS_PARTIAL_VISIBILITY); }
        }

        // get planets, check their locations...
        std::vector<const Planet*> planets = objects.FindObjects<Planet>();
        for (std::vector<const Planet*>::iterator it = planets.begin(); it != planets.end(); ++it) {
            const Planet* planet = *it;
            int system_id = planet->SystemID();
            if (system_id == INVALID_OBJECT_ID)
                continue;

            int planet_id = planet->ID();
            for (std::map<int, std::set<int> >::const_iterator
                 emp_it = empires_systems_with_owned_objects.begin();
                 emp_it != empires_systems_with_owned_objects.end();
                 ++emp_it)
            {
                int empire_id = emp_it->first;
                const std::set<int>& empire_systems = emp_it->second;
                if (empire_systems.find(system_id) == empire_systems.end())
                    continue;
                // ensure planets are at least basicaly visible.  does not
                // overwrite higher visibility levels
                universe.SetEmpireObjectVisibility(empire_id, planet_id, VIS_BASIC_VISIBILITY);
            }
        }
    }

    void PropegateVisibilityToContainerObjects(const ObjectMap& objects,
                                               Universe::EmpireObjectVisibilityMap& empire_object_visibility)
    {
        // propegate visibility from contained to container objects
        for (ObjectMap::const_iterator<> container_object_it = objects.const_begin();
             container_object_it != objects.const_end(); ++container_object_it)
        {
            int container_obj_id = container_object_it->ID();

            // get container object
            const UniverseObject* container_obj = *container_object_it;
            if (!container_obj)
                continue;   // shouldn't be necessary, but I like to be safe...

            // does object actually contain any other objects?
            std::vector<int> contained_objects = container_obj->FindObjectIDs();
            if (contained_objects.empty())
                continue;   // nothing to propegate if no objects contained

            // check if container object is a fleet, for special case later...
            bool container_fleet = container_obj->ObjectType() == OBJ_FLEET;


            //Logger().debugStream() << "Container object " << container_obj->Name() << " (" << container_obj->ID() << ")";

            // for each contained object within container
            for (std::vector<int>::iterator contained_obj_it = contained_objects.begin();
                 contained_obj_it != contained_objects.end(); ++contained_obj_it)
            {
                int contained_obj_id = *contained_obj_it;

                //Logger().debugStream() << " ... contained object (" << contained_obj_id << ")";

                // for each empire with a visibility map
                for (Universe::EmpireObjectVisibilityMap::iterator empire_it = empire_object_visibility.begin();
                     empire_it != empire_object_visibility.end(); ++empire_it)
                {
                    Universe::ObjectVisibilityMap& vis_map = empire_it->second;

                    //Logger().debugStream() << " ... ... empire id " << empire_it->first;

                    // find current empire's visibility entry for current container object
                    Universe::ObjectVisibilityMap::iterator container_vis_it = vis_map.find(container_obj_id);
                    // if no entry yet stored for this object, default to not visible
                    if (container_vis_it == vis_map.end()) {
                        vis_map[container_obj_id] = VIS_NO_VISIBILITY;

                        // get iterator pointing at newly-created entry
                        container_vis_it = vis_map.find(container_obj_id);
                    } else {
                        // check whether having a contained object would change container's visibility
                        if (container_fleet) {
                            // special case for fleets: grant partial visibility if
                            // a contained ship is seen with partial visibility or
                            // higher visibilitly
                            if (container_vis_it->second >= VIS_PARTIAL_VISIBILITY)
                                continue;
                        } else if (container_vis_it->second >= VIS_BASIC_VISIBILITY) {
                            // general case: for non-fleets, having visible
                            // contained object grants basic vis only.  if
                            // container already has this or better for the current
                            // empire, don't need to propegate anything
                            continue;
                        }
                    }


                    // find contained object's entry in visibility map
                    Universe::ObjectVisibilityMap::iterator contained_vis_it = vis_map.find(contained_obj_id);
                    if (contained_vis_it != vis_map.end()) {
                        // get contained object's visibility for current empire
                        Visibility contained_obj_vis = contained_vis_it->second;

                        // no need to propegate if contained object isn't visible to current empire
                        if (contained_obj_vis <= VIS_NO_VISIBILITY)
                            continue;

                        //Logger().debugStream() << " ... ... contained object vis: " << contained_obj_vis;

                        // contained object is at least basically visible.
                        // container should be at least partially visible, but don't
                        // want to decrease visibility of container if it is already
                        // higher than partially visible
                        if (container_vis_it->second < VIS_BASIC_VISIBILITY)
                            container_vis_it->second = VIS_BASIC_VISIBILITY;

                        // special case for fleets: grant partial visibility if
                        // visible contained object is partially or better visible
                        // this way fleet ownership is known to players who can 
                        // see ships with partial or better visibility (and thus
                        // know the owner of the ships and thus should know the
                        // owners of the fleet)
                        if (container_fleet && contained_obj_vis >= VIS_PARTIAL_VISIBILITY &&
                            container_vis_it->second < VIS_PARTIAL_VISIBILITY)
                        { container_vis_it->second = VIS_PARTIAL_VISIBILITY; }
                    }
                }   // end for empire visibility entries
            }   // end for contained objects
        }   // end for container objects
    }

    void PropegateVisibilityToSystemsAlongStarlanes(const ObjectMap& objects,
                                                    Universe::EmpireObjectVisibilityMap& empire_object_visibility) {
        const std::vector<const System*> systems = objects.FindObjects<System>();
        for (std::vector<const System*>::const_iterator it = systems.begin(); it != systems.end(); ++it) {
            const System* system = *it;
            int system_id = system->ID();

            // for each empire with a visibility map
            for (Universe::EmpireObjectVisibilityMap::iterator empire_it = empire_object_visibility.begin();
                 empire_it != empire_object_visibility.end(); ++empire_it)
            {
                Universe::ObjectVisibilityMap& vis_map = empire_it->second;

                // find current system's visibility
                Universe::ObjectVisibilityMap::iterator system_vis_it = vis_map.find(system_id);
                if (system_vis_it == vis_map.end())
                    continue;

                // skip systems that aren't at least partially visible; they can't propegate visibility along starlanes
                Visibility system_vis = system_vis_it->second;
                if (system_vis <= VIS_BASIC_VISIBILITY)
                    continue;

                // get all starlanes emanating from this system, and loop through them
                System::StarlaneMap starlane_map = system->StarlanesWormholes();
                for (System::StarlaneMap::const_iterator lane_it = starlane_map.begin();
                     lane_it != starlane_map.end(); ++lane_it)
                {
                    bool is_wormhole = lane_it->second;
                    if (is_wormhole)
                        continue;

                    // find entry for system on other end of starlane in visibility
                    // map, and upgrade to basic visibility if not already at that
                    // leve, so that starlanes will be visible if either system it
                    // ends at is partially visible or better
                    int lane_end_sys_id = lane_it->first;
                    Universe::ObjectVisibilityMap::iterator lane_end_vis_it = vis_map.find(lane_end_sys_id);
                    if (lane_end_vis_it == vis_map.end())
                        vis_map[lane_end_sys_id] = VIS_BASIC_VISIBILITY;
                    else if (lane_end_vis_it->second < VIS_BASIC_VISIBILITY)
                        lane_end_vis_it->second = VIS_BASIC_VISIBILITY;
                }
            }
        }

    }

    void SetTravelledStarlaneEndpointsVisible(const ObjectMap& objects,
                                              Universe::EmpireObjectVisibilityMap& empire_object_visibility)
    {
        // ensure systems on either side of a starlane along which a fleet is
        // moving are at least basically visible, so that the starlane itself can /
        // will be visible
        std::vector<const Fleet*> moving_fleets;
        std::vector<const UniverseObject*> moving_fleet_objects = objects.FindObjects(MovingFleetVisitor());
        for (std::vector<const UniverseObject*>::iterator it = moving_fleet_objects.begin();
             it != moving_fleet_objects.end(); ++it)
        {
            const UniverseObject* obj = *it;
            if (obj->Unowned() || obj->SystemID() == INVALID_OBJECT_ID || obj->ObjectType() != OBJ_FLEET)
                continue;
            const Fleet* fleet = universe_object_cast<const Fleet*>(obj);
            if (!fleet)
                continue;

            int prev = fleet->PreviousSystemID();
            int next = fleet->NextSystemID();

            // ensure fleet's owner has at least basic visibility of the next
            // and previous systems on the fleet's path
            Universe::ObjectVisibilityMap& vis_map = empire_object_visibility[fleet->Owner()];

            Universe::ObjectVisibilityMap::iterator system_vis_it = vis_map.find(prev);
            if (system_vis_it == vis_map.end()) {
                vis_map[prev] = VIS_BASIC_VISIBILITY;
            } else {
                if (system_vis_it->second < VIS_BASIC_VISIBILITY)
                    system_vis_it->second = VIS_BASIC_VISIBILITY;
            }

            system_vis_it = vis_map.find(next);
            if (system_vis_it == vis_map.end()) {
                vis_map[next] = VIS_BASIC_VISIBILITY;
            } else {
                if (system_vis_it->second < VIS_BASIC_VISIBILITY)
                    system_vis_it->second = VIS_BASIC_VISIBILITY;
            }
        }
    }

    void SetEmpireSpecialVisibilities(const ObjectMap& objects,
                                      Universe::EmpireObjectVisibilityMap& empire_object_visibility,
                                      Universe::EmpireObjectSpecialsMap& empire_object_visible_specials)
    {
        // after setting object visibility, similarly set visibility of objects'
        // specials for each empire
        for (EmpireManager::iterator empire_it = Empires().begin();
             empire_it != Empires().end(); ++empire_it)
        {
            int empire_id = empire_it->first;
            Universe::ObjectVisibilityMap& obj_vis_map = empire_object_visibility[empire_id];
            Universe::ObjectSpecialsMap& obj_specials_map = empire_object_visible_specials[empire_id];

            const Empire* empire = empire_it->second;
            const Meter* detection_meter = empire->GetMeter("METER_DETECTION_STRENGTH");
            if (!detection_meter)
                continue;
            double detection_strength = detection_meter->Current();

            // every object empire has visibility of might have specials
            for (Universe::ObjectVisibilityMap::const_iterator obj_it = obj_vis_map.begin();
                 obj_it != obj_vis_map.end(); ++obj_it)
            {
                if (obj_it->second <= VIS_NO_VISIBILITY)
                    continue;

                int object_id = obj_it->first;
                const UniverseObject* obj = objects.Object(object_id);
                if (!obj)
                    continue;
                const std::map<std::string, int>& all_object_specials = obj->Specials();
                if (all_object_specials.empty())
                    continue;

                std::set<std::string>& visible_specials = obj_specials_map[object_id];

                // check all object's specials.
                std::vector<std::string> potentially_detectable_object_specials;
                for (std::map<std::string, int>::const_iterator special_it = all_object_specials.begin();
                     special_it != all_object_specials.end(); ++special_it)
                {
                    const Special* special = GetSpecial(special_it->first);
                    if (!special)
                        continue;
                    double special_stealth = special->Stealth();
                    // if special is 0 stealth, or has stealth less than empire's
                    // detection strength, mark as visible
                    if (special_stealth <= 0.0 || special_stealth <= detection_strength) {
                        visible_specials.insert(special_it->first);
                        //Logger().debugStream() << "Special " << special_it->first << " on " << obj->Name() << " is visible to empire " << empire_id;
                    }
                }
            }
        }
    }
}

void Universe::UpdateEmpireObjectVisibilities() {
    //Logger().debugStream() << "Universe::UpdateEmpireObjectVisibilities()";

    // ensure Universe knows empires have knowledge of designs the empire is specifically remembering
    for (EmpireManager::iterator empire_it = Empires().begin();
         empire_it != Empires().end(); ++empire_it)
    {
        const Empire* empire = empire_it->second;
        int empire_id = empire_it->first;
        const std::set<int>& empire_known_ship_designs = empire->ShipDesigns();
        for (std::set<int>::const_iterator design_it = empire_known_ship_designs.begin();
             design_it != empire_known_ship_designs.end(); ++design_it)
        { m_empire_known_ship_design_ids[empire_id].insert(*design_it); }
    }

    m_empire_object_visibility.clear();
    m_empire_object_visible_specials.clear();

    if (m_all_objects_visible) {
        SetAllObjectsVisibleToAllEmpires();
        return;
    }


    SetEmpireOwnedObjectVisibilities();

    std::map<int, std::map<std::pair<double, double>, float> >
        empire_position_detection_ranges = GetEmpiresPositionDetectionRanges();

    std::map<int, std::map<std::pair<double, double>, std::vector<int> > >
        empire_position_potentially_detectable_objects =
            GetEmpiresPositionsPotentiallyDetectableObjects(Objects());

    SetEmpireObjectVisibilitiesFromRanges(empire_position_detection_ranges,
                                          empire_position_potentially_detectable_objects);
    SetEmpireFieldVisibilitiesFromRanges(empire_position_detection_ranges, Objects());

    SetSameSystemPlanetsVisible(Objects());

    PropegateVisibilityToContainerObjects(Objects(), m_empire_object_visibility);

    PropegateVisibilityToSystemsAlongStarlanes(Objects(), m_empire_object_visibility);

    SetTravelledStarlaneEndpointsVisible(Objects(), m_empire_object_visibility);

    SetEmpireSpecialVisibilities(Objects(), m_empire_object_visibility, m_empire_object_visible_specials);
}

void Universe::UpdateEmpireLatestKnownObjectsAndVisibilityTurns() {
    //Logger().debugStream() << "Universe::UpdateEmpireLatestKnownObjectsAndVisibilityTurns()";

    // assumes m_empire_object_visibility has been updated

    //  for each object in universe
    //      for each empire that can see object this turn
    //          update empire's information about object, based on visibility
    //          update empire's visbilility turn history

    int current_turn = CurrentTurn();
    if (current_turn == INVALID_GAME_TURN)
        return;

    // for each object in universe
    for (ObjectMap::const_iterator<> it = m_objects.const_begin(); it != m_objects.const_end(); ++it) {
        int object_id = it->ID();
        const UniverseObject* full_object = *it; // not filtered on server by visibility
        if (!full_object) {
            Logger().errorStream() << "UpdateEmpireLatestKnownObjectsAndVisibilityTurns found null object in m_objects with id " << object_id;
            continue;
        }

        // for each empire with a visibility map
        for (EmpireObjectVisibilityMap::const_iterator empire_it = m_empire_object_visibility.begin();
             empire_it != m_empire_object_visibility.end(); ++empire_it)
        {
            // can empire see object?
            const ObjectVisibilityMap& vis_map = empire_it->second;    // stores level of visibility empire has for each object it can detect this turn
            ObjectVisibilityMap::const_iterator vis_it = vis_map.find(object_id);
            if (vis_it == vis_map.end())
                continue;   // empire can't see current object, so move to next empire
            const Visibility vis = vis_it->second;
            if (vis <= VIS_NO_VISIBILITY)
                continue;   // empire can't see current object, so move to next empire

            // empire can see object.  need to update empire's latest known
            // information about object, and historical turns on which object
            // was seen at various visibility levels.

            int empire_id = empire_it->first;

            ObjectMap&                  known_object_map = m_empire_latest_known_objects[empire_id];        // creates empty map if none yet present
            ObjectVisibilityTurnMap&    object_vis_turn_map = m_empire_object_visibility_turns[empire_id];  // creates empty map if none yet present
            VisibilityTurnMap&          vis_turn_map = object_vis_turn_map[object_id];                      // creates empty map if none yet present


            // update empire's latest known data about object, based on current visibility and historical visibility and knowledge of object

            // is there already last known version of an UniverseObject stored for this empire?
            if (UniverseObject* known_obj = known_object_map.Object(object_id)) {
                known_obj->Copy(full_object, empire_id);                    // already a stored version of this object for this empire.  update it, limited by visibility this empire has for this object this turn
            } else {
                if (UniverseObject* new_obj = full_object->Clone(empire_id))    // no previously-recorded version of this object for this empire.  create a new one, copying only the information limtied by visibility, leaving the rest as default values
                    known_object_map.Insert(new_obj);
            }

            //Logger().debugStream() << "Empire " << empire_id << " can see object " << object_id << " with vis level " << vis;

            // update empire's visibility turn history for current vis, and lesser vis levels
            if (vis >= VIS_BASIC_VISIBILITY) {
                vis_turn_map[VIS_BASIC_VISIBILITY] = current_turn;
                if (vis >= VIS_PARTIAL_VISIBILITY) {
                    vis_turn_map[VIS_PARTIAL_VISIBILITY] = current_turn;
                    if (vis >= VIS_FULL_VISIBILITY) {
                        vis_turn_map[VIS_FULL_VISIBILITY] = current_turn;
                    }
                }
                //Logger().debugStream() << " ... Setting empire " << empire_id << " object " << full_object->Name() << " (" << object_id << ") vis " << vis << " (and higher) turn to " << current_turn;
            } else {
                Logger().errorStream() << "Universe::UpdateEmpireLatestKnownObjectsAndVisibilityTurns() found invalid visibility for object with id " << object_id << " by empire with id " << empire_id;
                continue;
            }
        }
    }
}

void Universe::UpdateEmpireStaleObjectKnowledge() {
    // if any objects in the latest known objects for an empire are not
    // currently visible, but that empire has detectors in range of the objects'
    // latest known location and the objects' latest known stealth is low enough to be
    // detectable by that empire, then the latest known state of the objects
    // (including stealth and position) appears to be stale / out of date.

    const std::map<int, std::map<std::pair<double, double>, float> >
        empire_location_detection_ranges = GetEmpiresPositionDetectionRanges();

    for (EmpireObjectMap::iterator empire_it = m_empire_latest_known_objects.begin();
         empire_it != m_empire_latest_known_objects.end(); ++empire_it)
    {
        int empire_id = empire_it->first;
        const ObjectMap& latest_known_objects = empire_it->second;
        const ObjectVisibilityMap& vis_map = m_empire_object_visibility[empire_id];
        std::set<int>& stale_set = m_empire_stale_knowledge_object_ids[empire_id];
        const std::set<int>& destroyed_set = m_empire_known_destroyed_object_ids[empire_id];

        // remove stale marking for any known destroyed or currently visible objects
        for (std::set<int>::iterator stale_it = stale_set.begin(); stale_it != stale_set.end();) {
            int object_id = *stale_it;
            if (vis_map.find(object_id) != vis_map.end() ||
                destroyed_set.find(object_id) != destroyed_set.end())
            {
                std::set<int>::iterator temp = stale_it;    ++temp;
                stale_set.erase(stale_it);
                stale_it = temp;
            } else {
                ++stale_it;
            }
        }


        // get empire latest known objects that are potentially detectable
        std::map<int, std::map<std::pair<double, double>, std::vector<int> > >
            empires_latest_known_objects_that_should_be_detectable =
                GetEmpiresPositionsPotentiallyDetectableObjects(latest_known_objects, empire_id);

        const std::map<std::pair<double, double>, std::vector<int> >&
            empire_latest_known_should_be_still_detectable_objects =
                empires_latest_known_objects_that_should_be_detectable[empire_id];

        // get empire detection ranges
        std::map<int, std::map<std::pair<double, double>, float> >::const_iterator
            empire_detectors_it = empire_location_detection_ranges.find(empire_id);
        if (empire_detectors_it == empire_location_detection_ranges.end())
            continue;
        const std::map<std::pair<double, double>, float>& empire_detector_positions_ranges =
            empire_detectors_it->second;

        // filter should-be-still-detectable objects by whether they are
        // in range of a detector
        std::vector<int> should_still_be_detectable_latest_known_objects =
            FilterObjectPositionsByDetectorPositionsAndRanges(
                empire_latest_known_should_be_still_detectable_objects,
                empire_detector_positions_ranges);

        // filter to exclude objects that are known to have been destroyed
        FilterObjectIDsByKnownDestruction(should_still_be_detectable_latest_known_objects,
                                          empire_id, m_empire_known_destroyed_object_ids);

        // any objects that pass filters but aren't actually still visible
        // represent out-of-date info in empire's latest known objects.  these
        // entries need to be removed / flagged to indicate this
        for (std::vector<int>::const_iterator
             should_still_be_detectable_object_it = should_still_be_detectable_latest_known_objects.begin();
             should_still_be_detectable_object_it != should_still_be_detectable_latest_known_objects.end();
             ++should_still_be_detectable_object_it)
        {
            int object_id = *should_still_be_detectable_object_it;
            ObjectVisibilityMap::const_iterator vis_it = vis_map.find(object_id);
            if (vis_it == vis_map.end() || vis_it->second < VIS_BASIC_VISIBILITY) {
                // object not visible even though the latest known info about it
                // for this empire suggests it should be.  info is stale.
                stale_set.insert(object_id);
            }
        }

        // fleets that are not visible and that contain no non-stale ships are
        // stale
        for (ObjectMap::const_iterator<> obj_it = latest_known_objects.const_begin();
             obj_it != latest_known_objects.const_end(); ++obj_it)
        {
            const UniverseObject* obj = *obj_it;
            if (obj->ObjectType() != OBJ_FLEET)
                continue;
            if (obj->GetVisibility(empire_id) >= VIS_BASIC_VISIBILITY)
                continue;
            const Fleet* fleet = universe_object_cast<const Fleet*>(obj);
            if (!fleet)
                continue;
            int fleet_id = obj_it->ID();
            // are there any non-stale contained ships?
            bool non_stale_contained_ship = false;
            const std::set<int>& ship_ids = fleet->ShipIDs();
            for (std::set<int>::const_iterator ship_it = ship_ids.begin();
                 ship_it != ship_ids.end(); ++ship_it)
            {
                int ship_id = *ship_it;
                // is supposedly contained fleet unknown or not in this fleet?
                // if not known or not in this fleet, doesn't make fleet non-stale
                const Ship* ship = latest_known_objects.Object<Ship>(ship_id);
                if (!ship || ship->FleetID() != fleet_id)
                    continue;

                // is contained ship visible?  If so, makes fleet non-stale
                ObjectVisibilityMap::const_iterator vis_it = vis_map.find(ship_id);
                if (vis_it != vis_map.end() && vis_it->second > VIS_NO_VISIBILITY) {
                    non_stale_contained_ship = true;
                    break;
                }

                // contained ship is not visible.  if it is also stale, then it
                // doesn't make fleet non-stale
                if (stale_set.find(ship_id) != stale_set.end())
                    continue;

                // ship is not stale, so its fleet non-stale
                non_stale_contained_ship = true;
                break;
            }
            if (!non_stale_contained_ship)
                stale_set.insert(fleet_id);
        }

        //for (std::set<int>::const_iterator stale_it = stale_set.begin();
        //     stale_it != stale_set.end(); ++stale_it)
        //{
        //    const UniverseObject* obj = latest_known_objects.Object(*stale_it);
        //    Logger().debugStream() << "Object " << *stale_it << " : " << (obj ? obj->Name() : "(unknown)") << " is stale for empire " << empire_id ;
        //}
    }
}

void Universe::SetEmpireKnowledgeOfDestroyedObject(int object_id, int empire_id) {
    if (object_id == INVALID_OBJECT_ID) {
        Logger().errorStream() << "SetEmpireKnowledgeOfDestroyedObject called with INVALID_OBJECT_ID";
        return;
    }

    const Empire* empire = Empires().Lookup(empire_id);
    if (!empire) {
        Logger().errorStream() << "SetEmpireKnowledgeOfDestroyedObject called for invalid empire id: " << empire_id;
    }
    m_empire_known_destroyed_object_ids[empire_id].insert(object_id);
}

void Universe::SetEmpireKnowledgeOfShipDesign(int ship_design_id, int empire_id) {
    if (ship_design_id == ShipDesign::INVALID_DESIGN_ID) {
        Logger().errorStream() << "SetEmpireKnowledgeOfShipDesign called with INVALID_DESIGN_ID";
        return;
    }
    if (empire_id == ALL_EMPIRES)
        return;
    if (!Empires().Lookup(empire_id))
        Logger().errorStream() << "SetEmpireKnowledgeOfShipDesign called for invalid empire id: " << empire_id;

    m_empire_known_ship_design_ids[empire_id].insert(ship_design_id);
}

void Universe::Destroy(int object_id, bool update_destroyed_object_knowers/* = true*/) {
    // remove object from any containing UniverseObject
    UniverseObject* obj = m_objects.Object(object_id);
    if (!obj) {
        Logger().errorStream() << "Universe::Destroy called for nonexistant object with id: " << object_id;
        return;
    }
    //Logger().debugStream() << "Destroying object : " << id << " : " << obj->Name();

    m_destroyed_object_ids.insert(object_id);

    if (update_destroyed_object_knowers) {
        // record empires that know this object has been destroyed
        for (EmpireManager::iterator emp_it = Empires().begin(); emp_it != Empires().end(); ++emp_it) {
            int empire_id = emp_it->first;
            if (obj->GetVisibility(empire_id) >= VIS_BASIC_VISIBILITY) {
                SetEmpireKnowledgeOfDestroyedObject(object_id, empire_id);
                // TODO: Update m_empire_latest_known_objects somehow?
            }
        }
    }


    // move object to its own position, thereby removing it from anything
    // that contained it and propegating associated signals
    obj->MoveTo(obj->X(), obj->Y());


    // remove from existing objects set
    UniverseObjectDeleteSignal(obj);
    delete m_objects.Remove(object_id);
}

void Universe::RecursiveDestroy(int object_id) {
    UniverseObject* obj = m_objects.Object(object_id);
    if (!obj) {
        Logger().debugStream() << "Universe::RecursiveDestroy asked to destroy nonexistant object with id " << object_id;
        return;
    }

    if (Ship* ship = universe_object_cast<Ship*>(obj)) {
        // if a ship is being deleted, and it is the last ship in its fleet, then the empty fleet should also be deleted
        Fleet* fleet = GetFleet(ship->FleetID());
        Destroy(object_id);
        if (fleet && fleet->Empty())
            Destroy(fleet->ID());

    } else if (Fleet* fleet = universe_object_cast<Fleet*>(obj)) {
        std::set<int> fleet_ships = fleet->ShipIDs();           // copy, so destroying ships can't change the initial list / invalidate iterators, etc.
        for (std::set<int>::const_iterator it = fleet_ships.begin();
             it != fleet_ships.end(); ++it)
        { Destroy(*it); }
        Destroy(object_id);

    } else if (Planet* planet = universe_object_cast<Planet*>(obj)) {
        std::set<int> planet_buildings = planet->Buildings();   // copy, so destroying buildings can't change the initial list / invalidate iterators, etc.
        for (std::set<int>::const_iterator it = planet_buildings.begin();
             it != planet_buildings.end(); ++it)
        { Destroy(*it); }
        Destroy(object_id);

    } else if (System* system = universe_object_cast<System*>(obj)) {
        std::vector<int> system_objs = system->FindObjectIDs();
        for (std::vector<int>::const_iterator it = system_objs.begin();
             it != system_objs.end(); ++it)
        { Destroy(*it); }
        Destroy(object_id);

    } else if (obj->ObjectType() == OBJ_BUILDING) {
        Destroy(object_id);

    } else if (obj->ObjectType() == OBJ_FIELD) {
        Destroy(object_id);
    }
    // else ??? object is of some type unknown as of this writing.
}

bool Universe::Delete(int object_id) {
    // find object amongst existing objects and delete directly, without storing
    // any info about the previous object (as is done for destroying an object)
    UniverseObject* obj = m_objects.Object(object_id);
    if (!obj) {
        Logger().errorStream() << "Tried to delete a nonexistant object with id: " << object_id;
        return false;
    }

    // move object to invalid position, thereby removing it from anything that
    // contained it and propegating associated signals
    obj->MoveTo(UniverseObject::INVALID_POSITION, UniverseObject::INVALID_POSITION);

    // remove from existing objects set
    delete m_objects.Remove(object_id);

    // TODO: Should this also remove the object from the latest known objects
    // and known destroyed objects for each empire?

    return true;
}

void Universe::EffectDestroy(int object_id)
{ m_marked_destroyed.insert(object_id); }

void Universe::EffectVictory(int object_id, const std::string& reason_string)
{ m_marked_for_victory.insert(std::pair<int, std::string>(object_id, reason_string)); }

void Universe::HandleEmpireElimination(int empire_id) {
    //for (Effect::AccountingMap::iterator obj_it = m_effect_accounting_map.begin(); obj_it != m_effect_accounting_map.end(); ++obj_it) {
    //    // ever meter has a value at the start of the turn, and a value after updating with known effects
    //    for (std::map<MeterType, std::vector<Effect::AccountingInfo> >::iterator meter_type_it = obj_it->second.begin(); meter_type_it != obj_it->second.end(); ++meter_type_it) {
    //        for (std::size_t i = 0; i < meter_type_it->second.size(); ) {
    //            if (meter_type_it->second[i].caused_by_empire_id == empire_id)
    //                meter_type_it->second.erase(meter_type_it->second.begin() + i);
    //            else
    //                ++i;
    //        }
    //    }
    //}
}

void Universe::InitializeSystemGraph(int for_empire_id) {
    typedef boost::graph_traits<GraphImpl::SystemGraph>::edge_descriptor EdgeDescriptor;

    for (int i = static_cast<int>(boost::num_vertices(m_graph_impl->system_graph)) - 1; i >= 0; --i) {
        boost::clear_vertex(i, m_graph_impl->system_graph);
        boost::remove_vertex(i, m_graph_impl->system_graph);
    }

    std::vector<int> system_ids = ::Objects().FindObjectIDs<System>();
    //Logger().debugStream() << "InitializeSystemGraph(" << for_empire_id << ") system_ids: (" << system_ids.size() << ")";
    //for (std::vector<int>::const_iterator it = system_ids.begin(); it != system_ids.end(); ++it)
    //    Logger().debugStream() << " ... " << *it;

    GraphImpl::SystemIDPropertyMap sys_id_property_map =
        boost::get(vertex_system_id_t(), m_graph_impl->system_graph);

    GraphImpl::EdgeWeightPropertyMap edge_weight_map =
        boost::get(boost::edge_weight, m_graph_impl->system_graph);

    std::map<int, int> system_id_graph_index_reverse_lookup_map;    // key is system ID, value is index in m_graph_impl->system_graph of system's vertex

    for (int i = 0; i < static_cast<int>(system_ids.size()); ++i) {
        // add a vertex to the graph for this system, and assign it the system's universe ID as a property
        boost::add_vertex(m_graph_impl->system_graph);
        int system_id = system_ids[i];
        sys_id_property_map[i] = system_id;
        // add record of index in m_graph_impl->system_graph of this system
        system_id_graph_index_reverse_lookup_map[system_id] = i;
    }

    m_system_distances.resize(system_ids.size(), system_ids.size());
    for (int i = 0; i < static_cast<int>(system_ids.size()); ++i) {
        int system1_id = system_ids[i];
        const System* system1 = GetEmpireKnownSystem(system1_id, for_empire_id);

        m_system_id_to_graph_index[system1_id] = i;

        // add edges and edge weights
        for (System::const_lane_iterator it = system1->begin_lanes(); it != system1->end_lanes(); ++it) {
            // get id in universe of system at other end of lane
            const int lane_dest_id = it->first;
            // skip null lanes and only add edges in one direction, to avoid
            // duplicating edges ( since this is an undirected graph, A->B
            // duplicates B->A )
            if (lane_dest_id >= system1_id)
                continue;

            // get m_graph_impl->system_graph index for this system
            std::map<int, int>::iterator reverse_lookup_map_it = system_id_graph_index_reverse_lookup_map.find(lane_dest_id);
            if (reverse_lookup_map_it == system_id_graph_index_reverse_lookup_map.end())
                continue;   // couldn't find destination system id in reverse lookup map; don't add to graph
            int lane_dest_graph_index = reverse_lookup_map_it->second;

            std::pair<EdgeDescriptor, bool> add_edge_result =
                boost::add_edge(i, lane_dest_graph_index, m_graph_impl->system_graph);

            if (add_edge_result.second) {                   // if this is a non-duplicate starlane or wormhole
                if (it->second) {                               // if this is a wormhole
                    edge_weight_map[add_edge_result.first] = 0.1;   // arbitrary small distance
                } else {                                        // if this is a starlane
                    const UniverseObject* system2 = GetUniverseObject(it->first);
                    double x_dist = system2->X() - system1->X();
                    double y_dist = system2->Y() - system1->Y();
                    edge_weight_map[add_edge_result.first] = std::sqrt(x_dist*x_dist + y_dist*y_dist);
                    //std::cout << "edge_weight_map " << system1_id << " to " << lane_dest_id << ": " << edge_weight_map[add_edge_result.first] << std::endl;
                }
            }
        }

        // define the straight-line system distances for this system
        for (int j = 0; j < i; ++j) {
            int system2_id = system_ids[j];
            const UniverseObject* system2 = GetUniverseObject(system2_id);
            double x_dist = system2->X() - system1->X();
            double y_dist = system2->Y() - system1->Y();
            m_system_distances[i][j] = std::sqrt(x_dist*x_dist + y_dist*y_dist);
        }
        m_system_distances[i][i] = 0.0;
    }

    m_system_jumps.resize(system_ids.size(), system_ids.size());
    constant_property<EdgeDescriptor, short> jump_weight = { 1 };
    boost::johnson_all_pairs_shortest_paths(m_graph_impl->system_graph, m_system_jumps, boost::weight_map(jump_weight));

    UpdateEmpireVisibilityFilteredSystemGraphs(for_empire_id);
}

void Universe::UpdateEmpireVisibilityFilteredSystemGraphs(int for_empire_id) {
    m_graph_impl->empire_system_graph_views.clear();

    // if building system graph views for all empires, then each empire's graph
    // should accurately filter for that empire's visibility.  if building
    // graphs for one empire, that empire won't know what systems other empires
    // have visibility of, so instead, have all empires' filtered graphs be
    // equal to the empire for which filtering is being done.  this way, on the
    // clients, enemy fleets can have move paths even though the client doesn't
    // know what systems those empires know about (so can't make an accurate
    // filtered graph for other empires)

    if (for_empire_id == ALL_EMPIRES) {
        // all empires get their own, accurately filtered graph
        for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
            int empire_id = it->first;
            GraphImpl::EdgeVisibilityFilter filter(&m_graph_impl->system_graph, empire_id);
            boost::shared_ptr<GraphImpl::EmpireViewSystemGraph> filtered_graph_ptr(
                new GraphImpl::EmpireViewSystemGraph(m_graph_impl->system_graph, filter));
            m_graph_impl->empire_system_graph_views[empire_id] = filtered_graph_ptr;
        }

    } else {
        // all empires share a single filtered graph, filtered by the for_empire_id
        GraphImpl::EdgeVisibilityFilter filter(&m_graph_impl->system_graph, for_empire_id);
        boost::shared_ptr<GraphImpl::EmpireViewSystemGraph> filtered_graph_ptr(
            new GraphImpl::EmpireViewSystemGraph(m_graph_impl->system_graph, filter));

        for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
            int empire_id = it->first;
            m_graph_impl->empire_system_graph_views[empire_id] = filtered_graph_ptr;
        }
    }
}

int& Universe::EncodingEmpire()
{ return m_encoding_empire; }

double Universe::UniverseWidth() const
{ return m_universe_width; }

const bool& Universe::UniverseObjectSignalsInhibited()
{ return m_inhibit_universe_object_signals; }

void Universe::InhibitUniverseObjectSignals(bool inhibit)
{ m_inhibit_universe_object_signals = inhibit; }

void Universe::GetShipDesignsToSerialize(ShipDesignMap& designs_to_serialize, int encoding_empire) const {
    if (encoding_empire == ALL_EMPIRES) {
        designs_to_serialize = m_ship_designs;
    } else {
        designs_to_serialize.clear();

        std::map<int, std::set<int> >::const_iterator it = m_empire_known_ship_design_ids.find(encoding_empire);
        if (it == m_empire_known_ship_design_ids.end())
            return; // no known designs to serialize

        const std::set<int>& empire_designs = it->second;

        // add all ship designs of ships this empire knows about 
        for (std::set<int>::const_iterator known_design_it = empire_designs.begin(); known_design_it != empire_designs.end(); ++known_design_it) {
            int design_id = *known_design_it;
            ShipDesignMap::const_iterator universe_design_it = m_ship_designs.find(design_id);
            if (universe_design_it != m_ship_designs.end())
                designs_to_serialize[design_id] = universe_design_it->second;
            else
                Logger().errorStream() << "Universe::GetShipDesignsToSerialize empire " << encoding_empire << " should know about design with id " << design_id << " but no such design exists in the Universe!";
        }
    }
}

void Universe::GetObjectsToSerialize(ObjectMap& objects, int encoding_empire) const {
    if (&objects == &m_objects)
        return;

    objects.Clear();

    if (encoding_empire == ALL_EMPIRES || !ENABLE_VISIBILITY_EMPIRE_MEMORY) {
        // if encoding for all empires, copy true full universe state (by specifying ALL_EMPIRES)
        // or if encoding without memory, copy all info visible to specified empire
        objects.Copy(m_objects, encoding_empire);
    } else {
        // if encoding for a specific empire with memory...

        // find indicated empire's knowledge about objects, current and previous
        EmpireObjectMap::const_iterator it = m_empire_latest_known_objects.find(encoding_empire);
        if (it == m_empire_latest_known_objects.end())
            return;                 // empire has no object knowledge, so there is nothing to send

        objects.Copy(it->second);
    }
}

void Universe::GetDestroyedObjectsToSerialize(std::set<int>& destroyed_object_ids, int encoding_empire) const {
    if (&destroyed_object_ids == &m_destroyed_object_ids)
        return;

    if (encoding_empire == ALL_EMPIRES) {
        // all destroyed objects
        destroyed_object_ids = m_destroyed_object_ids;
    } else {
        destroyed_object_ids.clear();
        // get empire's known destroyed objects
        ObjectKnowledgeMap::const_iterator it = m_empire_known_destroyed_object_ids.find(encoding_empire);
        if (it != m_empire_known_destroyed_object_ids.end())
            destroyed_object_ids = it->second;
    }
}

void Universe::GetEmpireKnownObjectsToSerialize(EmpireObjectMap& empire_latest_known_objects, int encoding_empire) const {
    if (&empire_latest_known_objects == &m_empire_latest_known_objects)
        return;

    Logger().debugStream() << "GetEmpireKnownObjectsToSerialize";

    for (EmpireObjectMap::iterator it = empire_latest_known_objects.begin(); it != empire_latest_known_objects.end(); ++it)
        it->second.Clear();
    empire_latest_known_objects.clear();

    if (!ENABLE_VISIBILITY_EMPIRE_MEMORY)
        return;

    if (encoding_empire == ALL_EMPIRES) {
        // copy all ObjectMaps' contents
        for (EmpireObjectMap::const_iterator it = m_empire_latest_known_objects.begin(); it != m_empire_latest_known_objects.end(); ++it) {
            int empire_id = it->first;
            const ObjectMap& map = it->second;
            empire_latest_known_objects[empire_id].Copy(map, ALL_EMPIRES);
        }
        return;
    }
}

void Universe::GetEmpireObjectVisibilityMap(EmpireObjectVisibilityMap& empire_object_visibility, int encoding_empire) const {
    if (encoding_empire == ALL_EMPIRES) {
        empire_object_visibility = m_empire_object_visibility;
        return;
    }

    // include just requested empire's visibility for each object it has better
    // than no visibility of.  TODO: include what requested empire knows about
    // other empires' visibilites of objects
    empire_object_visibility.clear();
    for (ObjectMap::const_iterator<> it = m_objects.const_begin(); it != m_objects.const_end(); ++it) {
        int object_id = it->ID();
        Visibility vis = GetObjectVisibilityByEmpire(object_id, encoding_empire);
        if (vis > VIS_NO_VISIBILITY)
            empire_object_visibility[encoding_empire][object_id] = vis;
    }
}

void Universe::GetEmpireObjectVisibilityTurnMap(EmpireObjectVisibilityTurnMap& empire_object_visibility_turns, int encoding_empire) const {
    if (encoding_empire == ALL_EMPIRES) {
        empire_object_visibility_turns = m_empire_object_visibility_turns;
        return;
    }

    // include just requested empire's visibility turn information
    empire_object_visibility_turns.clear();
    EmpireObjectVisibilityTurnMap::const_iterator it = m_empire_object_visibility_turns.find(encoding_empire);
    if (it != m_empire_object_visibility_turns.end())
        empire_object_visibility_turns[encoding_empire] = it->second;
}

void Universe::GetEmpireKnownDestroyedObjects(ObjectKnowledgeMap& empire_known_destroyed_object_ids, int encoding_empire) const {
    if (&empire_known_destroyed_object_ids == &m_empire_known_destroyed_object_ids)
        return;

    if (encoding_empire == ALL_EMPIRES) {
        empire_known_destroyed_object_ids = m_empire_known_destroyed_object_ids;
        return;
    }

    empire_known_destroyed_object_ids.clear();

    // copy info about what encoding empire knows
    ObjectKnowledgeMap::const_iterator it = m_empire_known_destroyed_object_ids.find(encoding_empire);
    if (it != m_empire_known_destroyed_object_ids.end())
        empire_known_destroyed_object_ids[encoding_empire] = it->second;
}

void Universe::GetEmpireStaleKnowledgeObjects(ObjectKnowledgeMap& empire_stale_knowledge_object_ids, int encoding_empire) const {
    if (&empire_stale_knowledge_object_ids == &m_empire_stale_knowledge_object_ids)
        return;

    if (encoding_empire == ALL_EMPIRES) {
        empire_stale_knowledge_object_ids = m_empire_stale_knowledge_object_ids;
        return;
    }

    empire_stale_knowledge_object_ids.clear();

    // copy stale data for this empire
    ObjectKnowledgeMap::const_iterator it = m_empire_stale_knowledge_object_ids.find(encoding_empire);
    if (it != m_empire_stale_knowledge_object_ids.end())
        empire_stale_knowledge_object_ids[encoding_empire] = it->second;
}
