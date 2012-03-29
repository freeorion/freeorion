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


const std::size_t RESERVE_SET_SIZE = 2048;

namespace {
    const bool ENABLE_VISIBILITY_EMPIRE_MEMORY = true;      // toggles using memory with visibility, so that empires retain knowledge of objects viewed on previous turns

    void AddOptions(OptionsDB& db) {
        db.Add("verbose-logging", "OPTIONS_DB_VERBOSE_LOGGING_DESC",  false,  Validator<bool>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    /** Wrapper for boost::timer that outputs time during which this object
      * existed.  Created in the scope of a function, and passed the appropriate
      * name, it will output to Logger().debugStream() the time elapsed while
      * the function was executing. */
    class ScopedTimer {
    public:
        ScopedTimer(const std::string& timed_name = "scoped timer") :
            m_timer(),
            m_name(timed_name)
        {}
        ~ScopedTimer() {
            if (m_timer.elapsed() * 1000.0 > 1) {
                Logger().debugStream() << m_name << " time: " << (m_timer.elapsed() * 1000.0);
            }
        }
    private:
        boost::timer    m_timer;
        std::string     m_name;
    };

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

        void discover_vertex(const Vertex& v, const Graph& g)
        {
            m_predecessors[static_cast<int>(v)] = m_source;

            if (v == m_stop)
                throw FoundDestination();

            if (m_level_complete) {
                m_marker = v;
                m_level_complete = false;
            }
        }

        void examine_vertex(const Vertex& v, const Graph& g)
        {
            if (v == m_marker) {
                if (!m_levels_remaining)
                    throw ReachedDepthLimit();
                m_levels_remaining--;
                m_level_complete = true;
            }
            
            m_source = v; // avoid re-calculating source from edge
        }

        void examine_edge(const Edge& e, const Graph& g) {}

        void tree_edge(const Edge& e, const Graph& g)
        {
            // wait till target is calculated
        }

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
    std::pair<std::list<int>, double> ShortestPathImpl(const Graph& graph, int system1_id, int system2_id, double linear_distance, const boost::unordered_map<int, int>& id_to_graph_index)
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
    std::map<double, int> ImmediateNeighborsImpl(const Graph& graph, int system_id, const boost::unordered_map<int, int>& id_to_graph_index)
    {
        typedef typename Graph::out_edge_iterator OutEdgeIterator;
        typedef typename boost::property_map<Graph, vertex_system_id_t>::const_type ConstSystemIDPropertyMap;
        typedef typename boost::property_map<Graph, boost::edge_weight_t>::const_type ConstEdgeWeightPropertyMap;

        std::map<double, int> retval;
        ConstEdgeWeightPropertyMap edge_weight_map = boost::get(boost::edge_weight, graph);
        ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), graph);
        std::pair<OutEdgeIterator, OutEdgeIterator> edges = boost::out_edges(id_to_graph_index.at(system_id), graph);
        for (OutEdgeIterator it = edges.first; it != edges.second; ++it) {
            retval[edge_weight_map[*it]] = sys_id_property_map[boost::target(*it, graph)];
        }
        return retval;
    }
}
using namespace SystemPathing;  // to keep GCC 4.2 on OSX happy

const int ALL_EMPIRES = -1;
const int INVALID_OBJECT_ID = -1;   //the ID number assigned to a UniverseObject upon construction; it is assigned an ID later when it is placed in the universe
const int MAX_ID            = 2000000000;


/////////////////////////////////////////////
// struct Universe::GraphImpl
/////////////////////////////////////////////
struct Universe::GraphImpl {
    typedef boost::property<vertex_system_id_t, int,
                            boost::property<boost::vertex_index_t, int> >   vertex_property_t;  ///< a system graph property map type
    typedef boost::property<boost::edge_weight_t, double>                   edge_property_t;    ///< a system graph property map type

    // declare main graph types, including properties declared above
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
// static(s)
const bool  Universe::ALL_OBJECTS_VISIBLE =                 false;
double      Universe::s_universe_width =                    1000.0;
bool        Universe::s_inhibit_universe_object_signals =   false;
int         Universe::s_encoding_empire =                   ALL_EMPIRES;

Universe::Universe() :
    m_graph_impl(new GraphImpl),
    m_last_allocated_object_id(-1), // this is conicidentally equal to INVALID_OBJECT_ID as of this writing, but the reason for this to be -1 is so that the first object has id 0, and all object ids are non-negative
    m_last_allocated_design_id(-1)  // same, but for ShipDesign::INVALID_DESIGN_ID
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
}

const ObjectMap& Universe::EmpireKnownObjects(int empire_id) const {
    if (empire_id == ALL_EMPIRES)
        return m_objects;

    EmpireObjectMap::const_iterator it = m_empire_latest_known_objects.find(empire_id);
    if (it != m_empire_latest_known_objects.end()) {
        return it->second;
    }

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
    for (ObjectMap::const_iterator obj_it = m_objects.const_begin(); obj_it != m_objects.const_end(); ++obj_it) {
        int id = obj_it->first;
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

const std::set<int>& Universe::EmpireKnownDestroyedObjectIDs(int empire_id) const {
    ObjectKnowledgeMap::const_iterator it = m_empire_known_destroyed_object_ids.find(empire_id);
    if (it != m_empire_known_destroyed_object_ids.end())
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
        int designed_by_empire_id = design->DesignedByEmpire();
        if (name == design_name && designed_by_empire_id == ALL_EMPIRES)
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
    if (empire_id == ALL_EMPIRES || Universe::ALL_OBJECTS_VISIBLE)
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

std::map<double, int> Universe::ImmediateNeighbors(int system_id, int empire_id/* = ALL_EMPIRES*/) const {
    if (empire_id == ALL_EMPIRES) {
        return ImmediateNeighborsImpl(m_graph_impl->system_graph, system_id, m_system_id_to_graph_index);
    } else {
        GraphImpl::EmpireViewSystemGraphMap::const_iterator graph_it = m_graph_impl->empire_system_graph_views.find(empire_id);
        if (graph_it != m_graph_impl->empire_system_graph_views.end())
            return ImmediateNeighborsImpl(*graph_it->second, system_id, m_system_id_to_graph_index);
    }
    return std::map<double, int>();
}

int Universe::Insert(UniverseObject* obj) {
    if (!obj)
        return INVALID_OBJECT_ID;

    if (m_last_allocated_object_id + 1 < MAX_ID) {
        int id = ++m_last_allocated_object_id;
        obj->SetID(id);
        m_objects.Insert(id, obj);
        return id;
    }

    // we'll probably never execute this branch, considering how many IDs are available
    // find a hole in the assigned IDs in which to place the object
    int last_id_seen = INVALID_OBJECT_ID;
    for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (1 < it->first - last_id_seen) {
            int id = last_id_seen + 1;
            obj->SetID(id);
            m_objects.Insert(id, obj);
            return id;
        }
    }

    return INVALID_OBJECT_ID;
}

bool Universe::InsertID(UniverseObject* obj, int id) {
    if (!obj || id == INVALID_OBJECT_ID || id >= MAX_ID)
        return false;

    obj->SetID(id);
    m_objects.Insert(id, obj);
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

void Universe::ApplyAllEffectsAndUpdateMeters() {
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
    for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        it->second->ResetTargetMaxUnpairedMeters();
        it->second->ResetPairedActiveMeters();
    }

    ExecuteEffects(targets_causes, true);

    // clamp max meters to [DEFAULT_VALUE, LARGE_VALUE] and current meters to [DEFAULT_VALUE, max]
    // clamp max and target meters to [DEFAULT_VALUE, LARGE_VALUE] and current meters to [DEFAULT_VALUE, max]
    for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
        it->second->ClampMeters();
}

void Universe::ApplyMeterEffectsAndUpdateMeters(const std::vector<int>& object_ids) {
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

    ExecuteEffects(targets_causes, true, true);

    for (std::vector<UniverseObject*>::iterator it = objects.begin(); it != objects.end(); ++it) {
        (*it)->ClampMeters();  // clamp max, target and unpaired meters to [DEFAULT_VALUE, LARGE_VALUE] and active meters with max meters to [DEFAULT_VALUE, max]
    }
}

void Universe::ApplyMeterEffectsAndUpdateMeters()
{ ApplyMeterEffectsAndUpdateMeters(m_objects.FindObjectIDs()); }

void Universe::ApplyAppearanceEffects(const std::vector<int>& object_ids) {
    // cache all activation and scoping condition results before applying Effects, since the application of
    // these Effects may affect the activation and scoping evaluations
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes, object_ids);
    ExecuteEffects(targets_causes, false, false, true);
}

void Universe::ApplyAppearanceEffects()
{ ApplyAppearanceEffects(m_objects.FindObjectIDs()); }

void Universe::InitMeterEstimatesAndDiscrepancies() {
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
        UniverseObject* obj = m_objects.Object(obj_it->first);    // object that has some meters
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
            int object_id = obj->ID();

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
        std::vector<int> object_ids;
        for (ObjectMap::iterator obj_it = m_objects.begin(); obj_it != m_objects.end(); ++obj_it)
            object_ids.push_back(obj_it->first);

        UpdateMeterEstimates(object_ids);
        return;
    }

    // collect objects to update meter for.  this may be a single object, a group of related objects, or all objects
    // in the (known) universe.  also clear effect accounting for meters that are to be updated.
    std::set<int> objects_set;
    std::list<int> objects_list;
    objects_list.push_back(object_id);

    for (std::list<int>::iterator list_it = objects_list.begin(); list_it !=  objects_list.end(); ++list_it) {
        // get next object on list
        int cur_object_id = *list_it;
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
    ScopedTimer timer("Universe::UpdateMeterEstimates");

    std::set<int> objects_set;  // ensures no duplicates

    for (std::vector<int>::const_iterator obj_it = objects_vec.begin(); obj_it != objects_vec.end(); ++obj_it) {
        int cur_object_id = *obj_it;
        m_effect_accounting_map[cur_object_id].clear();
        objects_set.insert(cur_object_id);
    }
    std::vector<int> final_objects_vec;
    std::copy(objects_set.begin(), objects_set.end(), std::back_inserter(final_objects_vec));
    UpdateMeterEstimatesImpl(final_objects_vec);
}

void Universe::UpdateMeterEstimatesImpl(const std::vector<int>& objects_vec) {
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
        Logger().debugStream() << "UpdateMeterEstimatesImpl after resetting meters";
        Logger().debugStream() << m_objects.Dump();
    }

    // cache all activation and scoping condition results before applying Effects, since the application of
    // these Effects may affect the activation and scoping evaluations
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes, objects_vec);

    // Apply and record effect meter adjustments
    ExecuteEffects(targets_causes, true, true);

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        Logger().debugStream() << "UpdateMeterEstimatesImpl after executing effects";
        Logger().debugStream() << m_objects.Dump();
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
                    //Logger().debugStream() << "object " << obj_id << " has meter " << type << " discrepancy: " << discrepancy << " and final max: " << meter->Max();

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
        Logger().debugStream() << "UpdateMeterEstimatesImpl after discrepancies and clamping";
        Logger().debugStream() << m_objects.Dump();
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

void Universe::GetEffectsAndTargets(Effect::TargetsCauses& targets_causes, const std::vector<int>& target_objects) {
    // transfer target objects from input vector to a set
    Effect::TargetSet all_potential_targets;
    all_potential_targets.reserve(RESERVE_SET_SIZE);
    for (std::vector<int>::const_iterator it = target_objects.begin(); it != target_objects.end(); ++it)
        all_potential_targets.push_back(m_objects.Object(*it));

    Logger().debugStream() << "Universe::GetEffectsAndTargets";

    // 1) EffectsGroups from Specials
    Logger().debugStream() << "Universe::GetEffectsAndTargets for SPECIALS";
    for (ObjectMap::const_iterator it = m_objects.const_begin(); it != m_objects.const_end(); ++it) {
        int source_object_id = it->first;
        const std::map<std::string, int>& specials = it->second->Specials();
        for (std::map<std::string, int>::const_iterator special_it = specials.begin(); special_it != specials.end(); ++special_it) {
            const Special* special = GetSpecial(special_it->first);
            if (!special) {
                Logger().errorStream() << "GetEffectsAndTargets couldn't get Special " << special_it->first;
                continue;
            }

            StoreTargetsAndCausesOfEffectsGroups(special->Effects(), source_object_id, ECT_SPECIAL, special->Name(),
                                                 all_potential_targets, targets_causes);
        }
    }

    // 2) EffectsGroups from Techs
    Logger().debugStream() << "Universe::GetEffectsAndTargets for TECHS";
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
            std::vector<int> empire_planets = m_objects.FindObjectIDs(OwnedVisitor<Planet>(empire->EmpireID()));
            if (!empire_planets.empty()) {
                source_id = *empire_planets.begin();
            } else {
                // if no planet, use any owned object
                std::vector<int> empire_objects = m_objects.FindObjectIDs(OwnedVisitor<UniverseObject>(empire->EmpireID()));
                if (!empire_objects.empty()) {
                    source_id = *empire_objects.begin();
                } else {
                    continue;   // can't do techs for this empire
                }
            }
        }
        for (Empire::TechItr tech_it = empire->TechBegin(); tech_it != empire->TechEnd(); ++tech_it) {
            const Tech* tech = GetTech(*tech_it);
            if (!tech) continue;

            StoreTargetsAndCausesOfEffectsGroups(tech->Effects(), source_id, ECT_TECH, tech->Name(),
                                                 all_potential_targets, targets_causes);
        }
    }

    // 3) EffectsGroups from Buildings
    Logger().debugStream() << "Universe::GetEffectsAndTargets for BUILDINGS";
    std::vector<Building*> buildings = m_objects.FindObjects<Building>();
    for (std::vector<Building*>::const_iterator building_it = buildings.begin(); building_it != buildings.end(); ++building_it) {
        const Building* building = *building_it;
        if (!building) {
            Logger().errorStream() << "GetEffectsAndTargets couldn't get Building";
            continue;
        }
        const BuildingType* building_type = building->GetBuildingType();
        if (!building_type) {
            Logger().errorStream() << "GetEffectsAndTargets couldn't get BuildingType " << building->BuildingTypeName();
            continue;
        }

        StoreTargetsAndCausesOfEffectsGroups(building_type->Effects(), building->ID(), ECT_BUILDING, building_type->Name(),
                                             all_potential_targets, targets_causes);
    }

    // 4) EffectsGroups from Ship Hull and Ship Parts
    Logger().debugStream() << "Universe::GetEffectsAndTargets for SHIPS";
    std::vector<Ship*> ships = m_objects.FindObjects<Ship>();
    for (std::vector<Ship*>::const_iterator ship_it = ships.begin(); ship_it != ships.end(); ++ship_it) {
        const Ship* ship = *ship_it;
        if (!ship) {
            Logger().errorStream() << "GetEffectsAndTargets couldn't get Ship";
            continue;
        }
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

        StoreTargetsAndCausesOfEffectsGroups(hull_type->Effects(), ship->ID(), ECT_SHIP_HULL, hull_type->Name(),
                                             all_potential_targets, targets_causes);

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
            StoreTargetsAndCausesOfEffectsGroups(part_type->Effects(), ship->ID(), ECT_SHIP_PART, part_type->Name(),
                                                 all_potential_targets, targets_causes);
        }
    }

    // 5) EffectsGroups from Species
    Logger().debugStream() << "Universe::GetEffectsAndTargets for SPECIES";
    for (ObjectMap::const_iterator it = m_objects.const_begin(); it != m_objects.const_end(); ++it) {
        Logger().debugStream() << "... object (" << it->first << "): " << it->second->Name();
        const PopCenter* pc = dynamic_cast<const PopCenter*>(it->second);
        const Ship* ship = 0;
        if (!pc) {
            ship = dynamic_cast<const Ship*>(it->second);
            if (!ship) continue;
        }
        const std::string& species_name = (pc ? pc->SpeciesName() : ship->SpeciesName());
        //Logger().debugStream() << "... ... PopCenter species: " << species_name;
        if (species_name.empty())
            continue;
        const Species* species = GetSpecies(species_name);
        if (!species) {
            Logger().errorStream() << "GetEffectsAndTargets couldn't get Species " << species_name;
            continue;
        }
        StoreTargetsAndCausesOfEffectsGroups(species->Effects(), it->first, ECT_SPECIES, species_name,
                                             all_potential_targets, targets_causes);
    }

}

void Universe::StoreTargetsAndCausesOfEffectsGroups(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups,
                                                    int source_object_id, EffectsCauseType effect_cause_type,
                                                    const std::string& specific_cause_name,
                                                    Effect::TargetSet& target_objects, Effect::TargetsCauses& targets_causes)
{
    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        Logger().debugStream() << "Universe::StoreTargetsAndCausesOfEffectsGroups( , source id: " << source_object_id << ", , specific cause: " << specific_cause_name << ", , )";
    }

    // process all effects groups in set provided
    int eg_count = 1;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >::const_iterator effects_it;
    for (effects_it = effects_groups.begin(); effects_it != effects_groups.end(); ++effects_it) {
        ScopedTimer update_timer("... Universe::StoreTargetsAndCausesOfEffectsGroups done processing source " + boost::lexical_cast<std::string>(source_object_id) + " cause: " + specific_cause_name + " effects group " + boost::lexical_cast<std::string>(eg_count++));

        // create non_targets and targets sets for current effects group
        Effect::TargetSet target_set;                                    // initially empty
        target_set.reserve(RESERVE_SET_SIZE);

        // get effects group to process for this iteration
        boost::shared_ptr<const Effect::EffectsGroup> effects_group = *effects_it;

        {
            ScopedTimer update_timer2("... ... Universe::StoreTargetsAndCausesOfEffectsGroups get target set");
            // get set of target objects for this effects group from potential targets specified
            effects_group->GetTargetSet(source_object_id, target_set, target_objects);    // transfers objects from target_objects to target_set if they meet the condition
        }
        //effects_group->GetTargetSet(source_object_id, target_set, potential_target_set);    // transfers objects from potential_target_set to target_set if they meet the condition

        // abort if no targets
        if (target_set.empty())
            continue;

        // combine effects group and source object id into a sourced effects group
        Effect::SourcedEffectsGroup sourced_effects_group(source_object_id, effects_group);

        // combine cause type and specific cause into effect cause
        Effect::EffectCause effect_cause(effect_cause_type, specific_cause_name);

        // combine target set and effect cause
        Effect::TargetsAndCause target_and_cause(target_set, effect_cause);

        // store effect cause and targets info in map, indexed by sourced effects group
        targets_causes.push_back(std::make_pair(sourced_effects_group, target_and_cause));

        // restore target_objects by moving objects back from targets to target_objects
        // this should be cheaper than doing a full copy because target_set is usually small
        target_objects.insert(target_objects.end(), target_set.begin(), target_set.end());
    }
}

void Universe::ExecuteEffects(const Effect::TargetsCauses& targets_causes, bool update_effect_accounting,
                              bool only_meter_effects/* = false*/, bool only_appearance_effects/* = false*/)
{
    m_marked_destroyed.clear();
    m_marked_for_victory.clear();
    std::map<std::string, Effect::TargetSet> executed_nonstacking_effects;

    for (Effect::TargetsCauses::const_iterator targets_it = targets_causes.begin(); targets_it != targets_causes.end(); ++targets_it) {
        const UniverseObject* source = GetUniverseObject(targets_it->first.source_object_id);
        ScopedTimer update_timer("Universe::ExecuteEffects execute one effects group (source " +
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
            Logger().debugStream() << "ExecuteEffects effectsgroup: " << effects_group->Dump();
            Logger().debugStream() << "ExecuteEffects Targets before: ";
            for (Effect::TargetSet::const_iterator t_it = targets.begin(); t_it != targets.end(); ++t_it)
                Logger().debugStream() << " ... " << (*t_it)->Dump();
        }

        // execute Effects in the EffectsGroup
        if (only_appearance_effects)
            effects_group->ExecuteAppearanceModifications(sourced_effects_group.source_object_id, filtered_targets_and_cause.target_set);
        else if (update_effect_accounting && only_meter_effects)
            effects_group->ExecuteSetMeter(sourced_effects_group.source_object_id, filtered_targets_and_cause, m_effect_accounting_map);
        else if (only_meter_effects)
            effects_group->ExecuteSetMeter(sourced_effects_group.source_object_id, filtered_targets_and_cause.target_set);
        else if (update_effect_accounting)
            effects_group->Execute(sourced_effects_group.source_object_id, filtered_targets_and_cause, m_effect_accounting_map);
        else
            effects_group->Execute(sourced_effects_group.source_object_id, filtered_targets_and_cause.target_set);

        if (GetOptionsDB().Get<bool>("verbose-logging")) {
            Logger().debugStream() << "ExecuteEffects Targets after: ";
            for (Effect::TargetSet::const_iterator t_it = targets.begin(); t_it != targets.end(); ++t_it)
                Logger().debugStream() << " ... " << (*t_it)->Dump();
            Logger().debugStream() << "";
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

namespace {
    /** Sets visibilities for indicated \a empire_id of object with \a object_id
      * in the passed-in \a empire_vis_map to \a vis */
    void SetEmpireObjectVisibility(Universe::EmpireObjectVisibilityMap& empire_vis_map,
                                   std::map<int, std::set<int> >& empire_known_design_ids,
                                   int empire_id, int object_id, Visibility vis)
    {
        if (empire_id == ALL_EMPIRES)
            return;

        // get visibility map for empire and find object in it
        Universe::ObjectVisibilityMap& vis_map = empire_vis_map[empire_id];
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
                    empire_known_design_ids[empire_id].insert(design_id);
                }
            }
        }
    }
}

void Universe::UpdateEmpireObjectVisibilities() {
    //Logger().debugStream() << "Universe::UpdateEmpireObjectVisibilities()";

    // ensure Universe knows empires have knowledge of designs the empire is specifically remembering
    for (EmpireManager::iterator empire_it = Empires().begin(); empire_it != Empires().end(); ++empire_it) {
        const Empire* empire = empire_it->second;
        int empire_id = empire_it->first;
        const std::set<int>& empire_known_ship_designs = empire->ShipDesigns();
        for (std::set<int>::const_iterator design_it = empire_known_ship_designs.begin();
             design_it != empire_known_ship_designs.end(); ++design_it)
        {
            m_empire_known_ship_design_ids[empire_id].insert(*design_it);
        }
    }


    m_empire_object_visibility.clear();
    m_empire_object_visible_specials.clear();


    if (ALL_OBJECTS_VISIBLE) {
        // set every object visible to all empires
        std::set<int> all_empire_ids;
        for (EmpireManager::iterator empire_it = Empires().begin(); empire_it != Empires().end(); ++empire_it)
            all_empire_ids.insert(empire_it->first);

        for (ObjectMap::const_iterator obj_it = m_objects.const_begin(); obj_it != m_objects.const_end(); ++obj_it) {
            for (std::set<int>::const_iterator empire_it = all_empire_ids.begin(); empire_it != all_empire_ids.end(); ++empire_it) {
                // objects
                SetEmpireObjectVisibility(m_empire_object_visibility, m_empire_known_ship_design_ids, *empire_it, obj_it->first, VIS_FULL_VISIBILITY);
                // specials on objects
                const std::map<std::string, int>& specials = obj_it->second->Specials();
                for (std::map<std::string, int>::const_iterator special_it = specials.begin();
                     special_it != specials.end(); ++special_it)
                {
                    m_empire_object_visible_specials[*empire_it][obj_it->first].insert(special_it->first);
                }
            }
        }

        return;
    }

    // for each empire
    for (EmpireManager::iterator empire_it = Empires().begin(); empire_it != Empires().end(); ++empire_it) {
        const Empire* empire = empire_it->second;
        int empire_id = empire_it->first;
        const Meter* detection_meter = empire->GetMeter("METER_DETECTION");
        double detection_strength = detection_meter ? detection_meter->Current() : 0.0;
        ObjectVisibilityMap& obj_vis_map = m_empire_object_visibility[empire_id];

        std::multimap<double, const UniverseObject*>    empire_detectors;   // objects empire owns and their detection ranges if above 0
        std::vector<const UniverseObject*>              detectable_objects; // objects with low enough stealth that empire could see them if in range

        for (ObjectMap::const_iterator object_it = m_objects.const_begin();
             object_it != m_objects.const_end(); ++object_it)
        {
            const UniverseObject* obj = object_it->second;
            int object_id = object_it->first;

            if (obj->OwnedBy(empire_id)) {
                // objects empire owns -> full visibility
                SetEmpireObjectVisibility(m_empire_object_visibility, m_empire_known_ship_design_ids,
                                          empire_id, object_id, VIS_FULL_VISIBILITY);
                // is object a detector?
                if (const Meter* detection_meter = obj->GetMeter(METER_DETECTION)) {
                    if (detection_meter->Current() > 0.0) {

                        // don't allow moving fleets or ships to provide detection
                        const Fleet* fleet = universe_object_cast<const Fleet*>(obj);
                        if (!fleet)
                            if (const Ship* ship = universe_object_cast<const Ship*>(obj))
                                fleet = m_objects.Object<Fleet>(ship->FleetID());
                        if (fleet) {
                            int next_id = fleet->NextSystemID();
                            int cur_id = fleet->SystemID();
                            if (next_id != INVALID_OBJECT_ID && next_id != cur_id)
                                continue;
                        }

                        // object can act as a detector for this empire
                        empire_detectors.insert(std::make_pair(detection_meter->Current(), obj));
                    }
                }
            } else {
                if (const Meter* stealth_meter = obj->GetMeter(METER_STEALTH)) {
                    // is object potentially detectable by empire? (low enough stealth compared with empire's detection strength)
                    if (stealth_meter->Current() <= detection_strength)
                        detectable_objects.push_back(obj);
                    // even without a detector, is object detectable due to having zero stealth?
                    if (stealth_meter->Current() <= 0) {
                        SetEmpireObjectVisibility(m_empire_object_visibility, m_empire_known_ship_design_ids,
                                                  empire_id, object_id, VIS_BASIC_VISIBILITY);
                    }
                }

            }
        }


        // check if each potentially detectable object is within range of any of the detectors
        for (std::vector<const UniverseObject*>::const_iterator detectable_it = detectable_objects.begin();
             detectable_it != detectable_objects.end(); ++detectable_it)
        {
            const UniverseObject* detectable_obj = *detectable_it;
            ObjectVisibilityMap::iterator obj_vis_map_it = obj_vis_map.find(detectable_obj->ID());
            Visibility obj_vis = VIS_NO_VISIBILITY;
            if (obj_vis_map_it != obj_vis_map.end())
                obj_vis = obj_vis_map_it->second;

            if (obj_vis >= VIS_PARTIAL_VISIBILITY)
                continue;   // Visibility can't be improved beyond partial by non-ownership detection

            for (std::multimap<double, const UniverseObject*>::reverse_iterator detector_it = empire_detectors.rbegin();
                 detector_it != empire_detectors.rend(); ++detector_it)
            {
                const UniverseObject* detector_obj = detector_it->second;
                if (detector_obj->ID() == detectable_obj->ID())
                    continue;

                double range_limit = detector_it->first;
                double x_dist = detectable_obj->X() - detector_obj->X();
                double y_dist = detectable_obj->Y() - detector_obj->Y();
                double dist2 = x_dist*x_dist + y_dist*y_dist;
                if (range_limit*range_limit >= dist2) {
                    SetEmpireObjectVisibility(m_empire_object_visibility, m_empire_known_ship_design_ids,
                                              empire_id, detectable_obj->ID(), VIS_PARTIAL_VISIBILITY);
                    break;
                } else if (dist2 == 0.0 && obj_vis <= VIS_NO_VISIBILITY) {
                    // planets always basically visible if at same location as a detector
                    if (universe_object_cast<const Planet*>(detectable_obj))
                        SetEmpireObjectVisibility(m_empire_object_visibility, m_empire_known_ship_design_ids,
                                                  empire_id, detectable_obj->ID(), VIS_BASIC_VISIBILITY);
                }
            }
        }
    }


    // propegate visibility from contained to container objects
    for (ObjectMap::const_iterator container_object_it = m_objects.const_begin();
         container_object_it != m_objects.const_end(); ++container_object_it)
    {
        int container_obj_id = container_object_it->first;

        // get container object
        const UniverseObject* container_obj = container_object_it->second;
        if (!container_obj)
            continue;   // shouldn't be necessary, but I like to be safe...

        // does object actually contain any other objects?
        std::vector<int> contained_objects = container_obj->FindObjectIDs();
        if (contained_objects.empty())
            continue;   // nothing to propegate if no objects contained

        // check if container object is a fleet, for special case later...
        const Fleet* container_fleet = universe_object_cast<const Fleet*>(container_obj);


        //Logger().debugStream() << "Container object " << container_obj->Name() << " (" << container_obj->ID() << ")";

        // for each contained object within container
        for (std::vector<int>::iterator contained_obj_it = contained_objects.begin(); contained_obj_it != contained_objects.end(); ++contained_obj_it) {
            int contained_obj_id = *contained_obj_it;

            //Logger().debugStream() << " ... contained object (" << contained_obj_id << ")";

            // for each empire with a visibility map
            for (EmpireObjectVisibilityMap::iterator empire_it = m_empire_object_visibility.begin(); empire_it != m_empire_object_visibility.end(); ++empire_it) {
                ObjectVisibilityMap& vis_map = empire_it->second;

                //Logger().debugStream() << " ... ... empire id " << empire_it->first;

                // find current empire's visibility entry for current container object
                ObjectVisibilityMap::iterator container_vis_it = vis_map.find(container_obj_id);
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
                ObjectVisibilityMap::iterator contained_vis_it = vis_map.find(contained_obj_id);
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
                    if (container_fleet && contained_obj_vis >= VIS_PARTIAL_VISIBILITY && container_vis_it->second < VIS_PARTIAL_VISIBILITY)
                        container_vis_it->second = VIS_PARTIAL_VISIBILITY;
                }
            }   // end for empire visibility entries
        }   // end for contained objects
    }   // end for container objects


    // propegate visibility along starlanes
    const std::vector<System*> systems = m_objects.FindObjects<System>();
    for (std::vector<System*>::const_iterator it = systems.begin(); it != systems.end(); ++it) {
        const System* system = *it;
        int system_id = system->ID();

        // for each empire with a visibility map
        for (EmpireObjectVisibilityMap::iterator empire_it = m_empire_object_visibility.begin(); empire_it != m_empire_object_visibility.end(); ++empire_it) {
            ObjectVisibilityMap& vis_map = empire_it->second;

            // find current system's visibility
            ObjectVisibilityMap::iterator system_vis_it = vis_map.find(system_id);
            if (system_vis_it == vis_map.end())
                continue;

            // skip systems that aren't at least partially visible; they can't propegate visibility along starlanes
            Visibility system_vis = system_vis_it->second;
            if (system_vis <= VIS_BASIC_VISIBILITY)
                continue;

            // get all starlanes emanating from this system, and loop through them
            System::StarlaneMap starlane_map = system->StarlanesWormholes();
            for (System::StarlaneMap::const_iterator lane_it = starlane_map.begin(); lane_it != starlane_map.end(); ++lane_it) {
                bool is_wormhole = lane_it->second;
                if (is_wormhole)
                    continue;

                // find entry for system on other end of starlane in visibility
                // map, and upgrade to basic visibility if not already at that
                // leve, so that starlanes will be visible if either system it
                // ends at is partially visible or better
                int lane_end_sys_id = lane_it->first;
                ObjectVisibilityMap::iterator lane_end_vis_it = vis_map.find(lane_end_sys_id);
                if (lane_end_vis_it == vis_map.end())
                    vis_map[lane_end_sys_id] = VIS_BASIC_VISIBILITY;
                else if (lane_end_vis_it->second < VIS_BASIC_VISIBILITY)
                    lane_end_vis_it->second = VIS_BASIC_VISIBILITY;
            }
        }
    }


    // ensure systems on either side of a starlane along which a fleet is
    // moving are at least basically visible, so that the starlane itself can /
    // will be visible
    std::vector<const Fleet*> moving_fleets;
    std::vector<UniverseObject*> moving_fleet_objects = m_objects.FindObjects(MovingFleetVisitor());
    for (std::vector<UniverseObject*>::iterator it = moving_fleet_objects.begin(); it != moving_fleet_objects.end(); ++it) {
        if (const Fleet* fleet = universe_object_cast<const Fleet*>(*it)) {
            if (fleet->SystemID() != INVALID_OBJECT_ID || fleet->Unowned())
                continue;

            int prev = fleet->PreviousSystemID();
            int next = fleet->NextSystemID();

            // ensure fleet's owner has at least basic visibility of the next
            // and previous systems on the fleet's path
            ObjectVisibilityMap& vis_map = m_empire_object_visibility[fleet->Owner()];

            ObjectVisibilityMap::iterator system_vis_it = vis_map.find(prev);
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


    // after setting object visibility, similarly set visibility of objects'
    // specials for each empire
    for (ObjectMap::const_iterator detector_it = m_objects.const_begin();
         detector_it != m_objects.const_end(); ++detector_it)
    {
        // get detector object
        const UniverseObject* detector = detector_it->second;
        if (!detector) continue;
        // unowned detectors can't contribute to empires' visibility
        if (detector->Unowned())
            continue;
        int detector_id = detector_it->first;

        // don't allow moving fleets or ships to provide detection
        const Fleet* fleet = universe_object_cast<const Fleet*>(detector);
        if (!fleet)
            if (const Ship* ship = universe_object_cast<const Ship*>(detector))
                fleet = m_objects.Object<Fleet>(ship->FleetID());
        if (fleet) {
            int next_id = fleet->NextSystemID();
            int cur_id = fleet->SystemID();
            if (next_id != INVALID_OBJECT_ID && next_id != cur_id)
                continue;
        }


        // get detection ability
        const Meter* detection_meter = detector->GetMeter(METER_DETECTION);
        if (!detection_meter) continue;
        double detection = detection_meter->Current();

        EmpireObjectVisibilityMap::const_iterator empire_obj_vis_map_it =
            m_empire_object_visibility.find(detector->Owner());
        if (empire_obj_vis_map_it == m_empire_object_visibility.end())
            continue;
        const ObjectVisibilityMap& obj_vis_map = empire_obj_vis_map_it->second;


        //Logger().debugStream() << "Detector object: " << detector->Name() << " (" << detector->ID() << ") detection: " << detection;

        // position of detector
        double xd = detector->X();
        double yd = detector->Y();


        // for each detectable object
        for (ObjectMap::const_iterator target_it = m_objects.const_begin(); target_it != m_objects.const_end(); ++target_it) {
            // is detectable object at least basically visible to detector's owner?
            // if not, skip it, as no specials on it can be seen on objects that
            // can't be seen by an empire
            ObjectVisibilityMap::const_iterator obj_vis_map_it = obj_vis_map.find(target_it->first);
            if (obj_vis_map_it == obj_vis_map.end())
                continue;
            if (obj_vis_map_it->second < VIS_BASIC_VISIBILITY)
                continue;

            // this object is visibile to this empire

            // get stealthy object's specials
            const UniverseObject* target = target_it->second;
            if (!target) continue;


            const std::map<std::string, int>& specials = target->Specials();
            for (std::map<std::string, int>::const_iterator special_it = specials.begin();
                 special_it != specials.end(); ++special_it)
            {
                // get special's stealth
                const std::string& special_name = special_it->first;

                // is special already visible for detector's owner?  If so, skip it
                ObjectSpecialsMap& obj_specials_map = m_empire_object_visible_specials[detector->Owner()];
                std::set<std::string>& visible_specials = obj_specials_map[target_it->first];
                if (visible_specials.find(special_name) != visible_specials.end())
                    continue;

                const Special* special = GetSpecial(special_name);
                if (!special)
                    continue;
                double stealth = special->Stealth();

                // zero-or-less stealth specials are always visible if their object is visible
                if (stealth <= 0) {
                    visible_specials.insert(special_name);
                    continue;
                }

                // test if detection strength and range are adequate to see special

                // position of target
                double xt = target->X();
                double yt = target->Y();

                // distance squared
                double dist2 = (xt-xd)*(xt-xd) + (yt-yd)*(yt-yd);

                // To determine if a detector can detect a special on a target,
                // the special's stealth is subtracted from the detector's range,
                // and the result is compared to the distance between them. If the
                // distance is less than (detector_detection - special_stealth),
                // then the special is seen by the detector
                double detect_range = detection - stealth;
                if (detect_range >= 0.0 && dist2 <= detect_range*detect_range) {
                    visible_specials.insert(special_name);
                    Logger().debugStream() << "Special " << special_name << " on " << target->Name() << " is visible to empire " << detector->Owner();
                }
            }
        }
    }
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
    for (ObjectMap::const_iterator it = m_objects.const_begin(); it != m_objects.const_end(); ++it) {
        int object_id = it->first;
        const UniverseObject* full_object = it->second; // not filtered on server by visibility
        if (!full_object) {
            Logger().errorStream() << "UpdateEmpireLatestKnownObjectsAndVisibilityTurns found null object in m_objects with id " << object_id;
            continue;
        }

        // for each empire with a visibility map
        for (EmpireObjectVisibilityMap::const_iterator empire_it = m_empire_object_visibility.begin(); empire_it != m_empire_object_visibility.end(); ++empire_it) {

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
                    known_object_map.Insert(object_id, new_obj);
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
        for (Fleet::iterator it = fleet->begin(); it != fleet->end(); ++it)
            Destroy(*it);
        Destroy(object_id);

    } else if (Planet* planet = universe_object_cast<Planet*>(obj)) {
        for (std::set<int>::const_iterator it = planet->Buildings().begin(); it != planet->Buildings().end(); ++it)
            Destroy(*it);
        Destroy(object_id);

    } else if (universe_object_cast<System*>(obj)) {
        // unsupported: do nothing

    } else if (universe_object_cast<Building*>(obj)) {
        Destroy(object_id);
    }
    // else ??? object is of some type unknown as of this writing.
}

bool Universe::Delete(int object_id) {
    // find object amongst existing objects and delete directly, without storing any info
    // about the previous object (as is done for destroying an object)
    UniverseObject* obj = m_objects.Object(object_id);
    if (!obj) {
        Logger().errorStream() << "Tried to delete a nonexistant object with id: " << object_id;
        return false;
    }

    // move object to invalid position, thereby removing it from anything that contained it
    // and propegating associated signals
    obj->MoveTo(UniverseObject::INVALID_POSITION, UniverseObject::INVALID_POSITION);

    // remove from existing objects set
    delete m_objects.Remove(object_id);

    // TODO: Should this not also remove the object from the latest known objects and known destroyed objects for each empire?

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
            // skip null lanes
            if (lane_dest_id == system1_id)
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

    RebuildEmpireViewSystemGraphs(for_empire_id);
}

void Universe::RebuildEmpireViewSystemGraphs(int for_empire_id) {
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
            boost::shared_ptr<GraphImpl::EmpireViewSystemGraph> filtered_graph_ptr(new GraphImpl::EmpireViewSystemGraph(m_graph_impl->system_graph, filter));
            m_graph_impl->empire_system_graph_views[empire_id] = filtered_graph_ptr;
        }

    } else {
        // all empires share a single filtered graph, filtered by the for_empire_id
        GraphImpl::EdgeVisibilityFilter filter(&m_graph_impl->system_graph, for_empire_id);
        boost::shared_ptr<GraphImpl::EmpireViewSystemGraph> filtered_graph_ptr(new GraphImpl::EmpireViewSystemGraph(m_graph_impl->system_graph, filter));

        for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
            int empire_id = it->first;
            m_graph_impl->empire_system_graph_views[empire_id] = filtered_graph_ptr;
        }
    }
}

double Universe::UniverseWidth()
{ return s_universe_width; }

const bool& Universe::UniverseObjectSignalsInhibited()
{ return s_inhibit_universe_object_signals; }

void Universe::InhibitUniverseObjectSignals(bool inhibit)
{ s_inhibit_universe_object_signals = inhibit; }

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

        // completely copy visible objects, rather than copying only the
        // currently visible information as would be done with 
        // objects.Copy as the empire's latest known objects map already
        // contains only information known to the empire for which objects are
        // being serialized.
        objects.CompleteCopyVisible(it->second, encoding_empire);
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

    // copy just encoding_empire's known objects
    EmpireObjectMap::const_iterator it = m_empire_latest_known_objects.find(encoding_empire);
    if (it != m_empire_latest_known_objects.end()) {
        const ObjectMap& map = it->second;

        //Logger().debugStream() << "empire " << encoding_empire << " latest known map initial: ";
        //for (ObjectMap::const_iterator oit = map.const_begin(); oit != map.const_end(); ++oit)
        //    Logger().debugStream() << oit->second->TypeName() << "(" << oit->second->ID() << ")";

        empire_latest_known_objects[encoding_empire].Copy(map, ALL_EMPIRES);

        //Logger().debugStream() << "empire " << encoding_empire << " latest known map after copying: ";
        //for (ObjectMap::const_iterator oit = map.const_begin(); oit != map.const_end(); ++oit)
        //    Logger().debugStream() << oit->second->TypeName() << "(" << oit->second->ID() << ")";

        //Logger().debugStream() << "empire_latest_known_objects[" << encoding_empire << "] after copying: ";
        //for (ObjectMap::const_iterator oit = empire_latest_known_objects[encoding_empire].const_begin(); oit != empire_latest_known_objects[encoding_empire].const_end(); ++oit)
        //    Logger().debugStream() << oit->second->TypeName() << "(" << oit->second->ID() << ")";
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
    for (ObjectMap::const_iterator it = m_objects.const_begin(); it != m_objects.const_end(); ++it) {
        int object_id = it->first;
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
