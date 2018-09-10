#include "Pathfinder.h"

#include "../util/Logger.h"
#include "../util/ScopedTimer.h"
#include "../Empire/EmpireManager.h"
#include "Field.h"
#include "Fleet.h"
#include "Ship.h"
#include "System.h"
#include "UniverseObject.h"
#include "ValueRef.h"
#include "Universe.h"
#include "Predicates.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/variant/variant.hpp>

#include <algorithm>
#include <stdexcept>
#include <limits>

namespace {
    const double    WORMHOLE_TRAVEL_DISTANCE = 0.1;         // the effective distance for ships travelling along a wormhole, for determining how much of their speed is consumed by the jump
}

FO_COMMON_API extern const int ALL_EMPIRES;

namespace {

    /** distance_matrix_storage implements the storage and the mutexes
        for distance in number of hops from system to system.

        For N systems there are N rows of N integer types T.
        Each row has a mutex and the whole table has a mutex.

        The table is assumed symmetric.  If present row i element j will
        equal row j element i.
     */
    template <class T> struct distance_matrix_storage {
        typedef T value_type;  ///< An integral type for number of hops.
        typedef std::vector<T>& row_ref;  ///< A reference to row type

        distance_matrix_storage() {};
        distance_matrix_storage(const distance_matrix_storage<T>& src)
        { resize(src.m_data.size()); };
        //TODO C++11 Move Constructor.

        /**Number of rows and columns. (N)*/
        size_t size()
        { return m_data.size(); }

        /**Resize and clear all mutexes.  Assumes that table is locked.*/
        void resize(size_t a_size) {
            m_data.clear();
            m_data.resize(a_size);
            m_row_mutexes.resize(a_size);
            for (auto &row_mutex : m_row_mutexes)
                row_mutex = std::make_shared<boost::shared_mutex>();
        }

        /**N x N table of hop distances in row column form.*/
        std::vector< std::vector<T>> m_data;
        /**Per row mutexes.*/
        std::vector< std::shared_ptr<boost::shared_mutex>> m_row_mutexes;
        /**Table mutex*/
        boost::shared_mutex m_mutex;
    };


    /**distance_matrix_cache is a cache of symmetric hop distances
       based on distance_matrix_storage.

    It enforces the locking convention with get_T which returns either a single
    integral value or calls a cache miss handler to fill an entire write locked
    row with data and then returns the requested value.

    All operations first lock the table and then when necessary the row.
    */
    //TODO: Consider replacing this scheme with thread local storage.  It
    //may be true that because this is a computed value that depends on
    //the system topology that almost never changes that the
    //synchronization costs way out-weigh the saved computation costs.
    template <class Storage, class T = typename Storage::value_type, class Row = typename Storage::row_ref>
    class distance_matrix_cache {
        public:
        distance_matrix_cache(Storage& the_storage) : m_storage(the_storage) {}
        /**Read lock the table and return the size N.*/
        size_t size() {
            boost::shared_lock<boost::shared_mutex> guard(m_storage.m_mutex);
            return m_storage.size();
        }
        /**Write lock the table and resize to N = \p a_size.*/
        void resize(size_t a_size) {
            boost::unique_lock<boost::shared_mutex> guard(m_storage.m_mutex);
            m_storage.resize(a_size);
        }

        public:

        // Note: The signatures for the cache miss handler and the cache hit handler have a void
        // return type, because this code can not anticipate all expected return types.  In order
        // to return a result function the calling code will need to bind the return parameter to
        // the function so that the function still has the required signature.

        /// Cache miss handler
        typedef boost::function<void (size_t &/*ii*/, Row /*row*/)> cache_miss_handler;

        /// A function to examine an entire row cache hit
        typedef boost::function<void (size_t &/*ii*/, const Row /*row*/)> cache_hit_handler;

        /** Retrieve a single element at (\p ii, \p jj).
          * On cache miss call the \p fill_row which must fill the row
          * at \p ii with NN items.
          * Throws if either index is out of range or if \p fill_row
          * does not fill the row  on a cache miss.
          */
        T get_T(size_t ii, size_t jj, cache_miss_handler fill_row) const {
            boost::shared_lock<boost::shared_mutex> guard(m_storage.m_mutex);

            size_t NN = m_storage.size();
            if ((ii >= NN) || (jj >= NN)) {
                ErrorLogger() << "distance_matrix_cache::get_T passed invalid node indices: "
                              << ii << "," << jj << " matrix size: " << NN;
                throw std::out_of_range("row and/or column index is invalid.");
            }
            {
                boost::shared_lock<boost::shared_mutex> row_guard(*m_storage.m_row_mutexes[ii]);
                const Row &row_data = m_storage.m_data[ii];

                if (NN == row_data.size())
                    return row_data[jj];
            }
            {
                boost::shared_lock<boost::shared_mutex> column_guard(*m_storage.m_row_mutexes[jj]);
                const Row &column_data = m_storage.m_data[jj];

                if (NN == column_data.size())
                    return column_data[ii];
            }
            {
                boost::unique_lock<boost::shared_mutex> row_guard(*m_storage.m_row_mutexes[ii]);
                Row &row_data = m_storage.m_data[ii];

                if (NN == row_data.size()) {
                    return row_data[jj];
                } else {
                    fill_row(ii, row_data);
                    if (row_data.size() != NN) {
                        std::stringstream ss;
                        ss << "Cache miss handler only filled cache row with "
                           << row_data.size() << " items when " << NN
                           << " items where expected ";
                        ErrorLogger() << ss.str();
                        throw std::out_of_range(ss.str());
                    }
                    return row_data[jj];
                }
            }
        }

        /** Retrieve a single row at \p ii.
          * On cache miss call \p fill_row which must fill the row
          * at \p ii with NN items.
          * On cache hit call \p use_row to handle the cache hit
          * Throws if index is out of range or if \p fill_row
          * does not fill the row  on a cache miss.
          */
        void examine_row(size_t ii, cache_miss_handler fill_row, cache_hit_handler use_row) const {
            boost::shared_lock<boost::shared_mutex> guard(m_storage.m_mutex);

            size_t NN = m_storage.size();
            if (ii >= NN) {
                ErrorLogger() << "distance_matrix_cache::get_row passed invalid index: "
                              << ii << " matrix size: " << NN;
                throw std::out_of_range("row index is invalid.");
            }
            {
                boost::shared_lock<boost::shared_mutex> row_guard(*m_storage.m_row_mutexes[ii]);
                const Row &row_data = m_storage.m_data[ii];

                if (NN == row_data.size())
                    return use_row(ii, row_data);
            }
            {
                boost::unique_lock<boost::shared_mutex> row_guard(*m_storage.m_row_mutexes[ii]);
                Row &row_data = m_storage.m_data[ii];

                if (NN == row_data.size()) {
                    return use_row(ii, row_data);
                } else {
                    fill_row(ii, row_data);
                    if (row_data.size() != NN) {
                        std::stringstream ss;
                        ss << "Cache miss handler only filled cache row with "
                           << row_data.size() << " items when " << NN
                           << " items where expected ";
                        ErrorLogger() << ss.str();
                        throw std::range_error(ss.str());
                    }
                    return use_row(ii, row_data);
                }
            }
        }

    private:
        Storage& m_storage;
    };
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

        void initialize_vertex(const Vertex& v, const Graph& g) {}

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
                                                       double linear_distance, const boost::unordered_map<int, size_t>& id_to_graph_index)
    {
        typedef typename boost::property_map<Graph, vertex_system_id_t>::const_type     ConstSystemIDPropertyMap;
        typedef typename boost::property_map<Graph, boost::vertex_index_t>::const_type  ConstIndexPropertyMap;
        typedef typename boost::property_map<Graph, boost::edge_weight_t>::const_type   ConstEdgeWeightPropertyMap;

        std::pair<std::list<int>, double> retval(std::list<int>(), -1.0);

        ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), graph);

        // convert system IDs to graph indices.  try/catch for invalid input system ids.
        size_t system1_index, system2_index;
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
                                                      const boost::unordered_map<int, size_t>& id_to_graph_index,
                                                      int max_jumps = INT_MAX)
    {
        typedef typename boost::property_map<Graph, vertex_system_id_t>::const_type ConstSystemIDPropertyMap;

        ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), graph);
        std::pair<std::list<int>, int> retval;

        size_t system1_index = id_to_graph_index.at(system1_id);
        size_t system2_index = id_to_graph_index.at(system2_id);

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
                                                      const boost::unordered_map<int, size_t>& id_to_graph_index)
    {
        typedef typename Graph::out_edge_iterator OutEdgeIterator;
        typedef typename boost::property_map<Graph, vertex_system_id_t>::const_type ConstSystemIDPropertyMap;
        typedef typename boost::property_map<Graph, boost::edge_weight_t>::const_type ConstEdgeWeightPropertyMap;

        std::multimap<double, int> retval;
        ConstEdgeWeightPropertyMap edge_weight_map = boost::get(boost::edge_weight, graph);
        ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), graph);
        auto edges = boost::out_edges(id_to_graph_index.at(system_id), graph);
        for (OutEdgeIterator it = edges.first; it != edges.second; ++it) {
            retval.insert({edge_weight_map[*it],
                           sys_id_property_map[boost::target(*it, graph)]});
        }
        return retval;
    }


}
using namespace SystemPathing;  // to keep GCC 4.2 on OSX happy

namespace {
    /////////////////////////////////////////////
    // struct GraphImpl
    /////////////////////////////////////////////
    struct GraphImpl {
        typedef boost::property<vertex_system_id_t, int,
                                boost::property<boost::vertex_index_t, int>>   vertex_property_t;  ///< a system graph property map type
        typedef boost::property<boost::edge_weight_t, double>                   edge_property_t;    ///< a system graph property map type

        // declare main graph types, including properties declared above
        // could add boost::disallow_parallel_edge_tag GraphProperty but it doesn't
        // work for vecS vector-based lists and parallel edges can be avoided while
        // creating the graph by filtering the edges to be added
        typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                                      vertex_property_t, edge_property_t> SystemGraph;

        struct EdgeVisibilityFilter     {
            EdgeVisibilityFilter() :
                m_graph(nullptr),
                m_empire_id(ALL_EMPIRES)
            {}

            EdgeVisibilityFilter(const SystemGraph* graph, int empire_id) :
                m_graph(graph),
                m_empire_id(empire_id)
            {
                if (!graph)
                    ErrorLogger() << "EdgeVisibilityFilter passed null graph pointer";
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
                std::shared_ptr<const System> system1 = GetEmpireKnownSystem(sys_id_1, m_empire_id);
                if (!system1) {
                    ErrorLogger() << "EdgeDescriptor::operator() couldn't find system with id " << sys_id_1;
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
        typedef std::map<int, std::shared_ptr<EmpireViewSystemGraph>> EmpireViewSystemGraphMap;


        void AddSystemPredicate(const Pathfinder::SystemExclusionPredicateType& pred) {
            for (auto empire : Empires()) {
                auto empire_id = empire.first;
                SystemPredicateFilter sys_pred_filter(&system_graph, empire_id, pred);
                auto sys_pred_filtered_graph_ptr = std::make_shared<SystemPredicateGraph>(system_graph, sys_pred_filter);

                auto pred_it = system_pred_graph_views.find(pred);
                if (pred_it == system_pred_graph_views.end()) {
                    EmpireSystemPredicateMap empire_graph_map;
                    empire_graph_map.emplace(empire_id, std::move(sys_pred_filtered_graph_ptr));
                    system_pred_graph_views.emplace(pred, std::move(empire_graph_map));
                } else if (pred_it->second.count(empire_id)) {
                    pred_it->second.at(empire_id) = std::move(sys_pred_filtered_graph_ptr);
                } else {
                    pred_it->second.emplace(empire_id, std::move(sys_pred_filtered_graph_ptr));
                }
            }
        }

        struct SystemPredicateFilter {
            SystemPredicateFilter() :
                m_graph(nullptr),
                m_empire_id(ALL_EMPIRES),
                m_pred(nullptr)
            {}

            SystemPredicateFilter(const SystemGraph* graph, int empire_id,
                                  const Pathfinder::SystemExclusionPredicateType& pred) :
                m_graph(graph),
                m_empire_id(empire_id),
                m_pred(pred)
            {
                if (!graph)
                    ErrorLogger() << "ExcludeObjectFilter passed null graph pointer";
            }

            template <typename EdgeDescriptor>
            bool operator()(const EdgeDescriptor& edge) const {
                if (!m_graph)
                    return true;

                // get system ids from graph indices

                // for reverse-lookup System universe ID from graph index
                ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), *m_graph);
                int sys_graph_index_1 = boost::source(edge, *m_graph);
                int sys_id_1 = sys_id_property_map[sys_graph_index_1];
                int sys_graph_index_2 = boost::target(edge, *m_graph);
                int sys_id_2 = sys_id_property_map[sys_graph_index_2];

                // look up objects in system
                auto system1 = GetEmpireKnownSystem(sys_id_1, m_empire_id);
                if (!system1) {
                    ErrorLogger() << "Invalid source system " << sys_id_1;
                    return true;
                }
                auto system2 = GetEmpireKnownSystem(sys_id_2, m_empire_id);
                if (!system2) {
                    ErrorLogger() << "Invalid target system " << sys_id_2;
                    return true;
                }

                if (!system1->HasStarlaneTo(system2->ID())) {
                    DebugLogger() << "No starlane from " << system1->ID() << " to " << system2->ID();
                    return false;
                }

                // Discard edge if it finds a contained object or matches either system for visitor
                for (auto object : EmpireKnownObjects(m_empire_id).FindObjects(*m_pred.get())) {
                    if (!object)
                        continue;
                    // object is destination system
                    if (object->ID() == system2->ID())
                        return false;

                    // object contained by destination system
                    if (object->ContainedBy(system2->ID()))
                        return false;
                }

                return true;
            }

        private:
            const SystemGraph* m_graph;
            int m_empire_id;
            Pathfinder::SystemExclusionPredicateType m_pred;
        };
        typedef boost::filtered_graph<SystemGraph, SystemPredicateFilter> SystemPredicateGraph;
        typedef std::map<int, std::shared_ptr<SystemPredicateGraph>> EmpireSystemPredicateMap;
        typedef std::map<Pathfinder::SystemExclusionPredicateType, EmpireSystemPredicateMap> SystemPredicateGraphMap;

        // declare property map types for properties declared above
        typedef boost::property_map<SystemGraph, vertex_system_id_t>::const_type        ConstSystemIDPropertyMap;
        typedef boost::property_map<SystemGraph, vertex_system_id_t>::type              SystemIDPropertyMap;
        typedef boost::property_map<SystemGraph, boost::vertex_index_t>::const_type     ConstIndexPropertyMap;
        typedef boost::property_map<SystemGraph, boost::vertex_index_t>::type           IndexPropertyMap;
        typedef boost::property_map<SystemGraph, boost::edge_weight_t>::const_type      ConstEdgeWeightPropertyMap;
        typedef boost::property_map<SystemGraph, boost::edge_weight_t>::type            EdgeWeightPropertyMap;

        SystemGraph                 system_graph;                 ///< a graph in which the systems are vertices and the starlanes are edges
        EmpireViewSystemGraphMap    empire_system_graph_views;    ///< a map of empire IDs to the views of the system graph by those empires
        /** Empire system graphs indexed by object predicate */
        SystemPredicateGraphMap     system_pred_graph_views;
        std::unordered_set<Pathfinder::SystemExclusionPredicateType> system_predicates;
    };

}

/////////////////////////////////////////////
// class PathfinderImpl
/////////////////////////////////////////////
class Pathfinder::PathfinderImpl {
    public:

    PathfinderImpl(): m_graph_impl(new GraphImpl) {}

    double LinearDistance(int object1_id, int object2_id) const;
    short JumpDistanceBetweenSystems(int system1_id, int system2_id) const;
    int JumpDistanceBetweenObjects(int object1_id, int object2_id) const;

    std::pair<std::list<int>, double> ShortestPath(int system1_id, int system2_id, int empire_id = ALL_EMPIRES) const;
    std::pair<std::list<int>, double> ShortestPath(int system1_id, int system2_id, int empire_id,
                                                   const Pathfinder::SystemExclusionPredicateType& sys_pred) const;
    double ShortestPathDistance(int object1_id, int object2_id) const;
    std::pair<std::list<int>, int> LeastJumpsPath(
        int system1_id, int system2_id, int empire_id = ALL_EMPIRES, int max_jumps = INT_MAX) const;
    bool SystemsConnected(int system1_id, int system2_id, int empire_id = ALL_EMPIRES) const;
    bool SystemHasVisibleStarlanes(int system_id, int empire_id = ALL_EMPIRES) const;
    std::multimap<double, int> ImmediateNeighbors(int system_id, int empire_id = ALL_EMPIRES) const;

    boost::unordered_multimap<int, int> Neighbors(
        int system_id, size_t n_outer = 1, size_t n_inner = 0) const;

    /** When a cache hit occurs use \p row to populate and return the
        multimap for Neighbors.*/
     void NeighborsCacheHit(
         boost::unordered_multimap<int, int>& result, size_t _n_outer, size_t _n_inner,
         size_t ii, distance_matrix_storage<short>::row_ref row) const;

    std::unordered_set<int> WithinJumps(size_t jumps, const std::vector<int>& candidates) const;
    void WithinJumpsCacheHit(
        std::unordered_set<int>* result, size_t jump_limit,
        size_t ii, distance_matrix_storage<short>::row_ref row) const;

    std::pair<std::vector<std::shared_ptr<const UniverseObject>>, std::vector<std::shared_ptr<const UniverseObject>>>
    WithinJumpsOfOthers(
        int jumps,
        const std::vector<std::shared_ptr<const UniverseObject>>& candidates,
        const std::vector<std::shared_ptr<const UniverseObject>>& stationary) const;

    /**Return true if \p system_id is within \p jumps of any of \p others*/
    bool WithinJumpsOfOthers(
        int jumps, int system_id,
        const std::vector<std::shared_ptr<const UniverseObject>>& others) const;

    /** If any of \p others are within \p jumps of \p ii return true in \p answer.

        The return value must be in a parameter so that after being bound to \p answer and \p jumps
        the function signature is that required by the cache_hit_handler type.
     */
    void WithinJumpsOfOthersCacheHit(
        bool& answer, int jumps,
        const std::vector<std::shared_ptr<const UniverseObject>>& others,
        size_t ii, distance_matrix_storage<short>::row_ref row) const;

    int NearestSystemTo(double x, double y) const;
    void InitializeSystemGraph(const std::vector<int> system_ids, int for_empire_id = ALL_EMPIRES);
    void UpdateEmpireVisibilityFilteredSystemGraphs(int for_empire_id = ALL_EMPIRES);

    /** When a cache miss occurs fill \p row with the distances
        from index \p ii to every other index.*/
    void HandleCacheMiss(size_t ii, distance_matrix_storage<short>::row_ref row) const;


    mutable distance_matrix_storage<short>  m_system_jumps;             ///< indexed by system graph index (not system id), caches the smallest number of jumps to travel between all the systems
    std::shared_ptr<GraphImpl>            m_graph_impl;               ///< a graph in which the systems are vertices and the starlanes are edges
    boost::unordered_map<int, size_t>       m_system_id_to_graph_index;
};

/////////////////////////////////////////////
// class Pathfinder
/////////////////////////////////////////////
Pathfinder::Pathfinder() :
    pimpl(new PathfinderImpl)
{}

Pathfinder::~Pathfinder()
{}

namespace {
    std::shared_ptr<const Fleet> FleetFromObject(const std::shared_ptr<const UniverseObject>& obj) {
        std::shared_ptr<const Fleet> retval = std::dynamic_pointer_cast<const Fleet>(obj);
        if (!retval) {
            if (auto ship = std::dynamic_pointer_cast<const Ship>(obj))
                retval = GetFleet(ship->FleetID());
        }
        return retval;
    }
}

/** HandleCacheMiss requires that \p row be locked by exterior context.
 */
void Pathfinder::PathfinderImpl::HandleCacheMiss(size_t ii, distance_matrix_storage<short>::row_ref row) const {

    typedef boost::iterator_property_map<std::vector<short>::iterator,
                                         boost::identity_property_map> DistancePropertyMap;

    distance_matrix_storage<short>::row_ref distance_buffer = row;
    distance_buffer.assign(m_system_jumps.size(), SHRT_MAX);
    distance_buffer[ii] = 0;
    DistancePropertyMap distance_property_map(distance_buffer.begin());
    boost::distance_recorder<DistancePropertyMap, boost::on_tree_edge> distance_recorder(distance_property_map);

    // FIXME: we have computed the i row and the j column, but
    // we are only utilizing the i row.

    boost::breadth_first_search(m_graph_impl->system_graph, ii,
                                boost::visitor(boost::make_bfs_visitor(distance_recorder)));
}



double Pathfinder::LinearDistance(int system1_id, int system2_id) const {
    return pimpl->LinearDistance(system1_id, system2_id);
}

double Pathfinder::PathfinderImpl::LinearDistance(int system1_id, int system2_id) const {
    std::shared_ptr<const System> system1 = GetSystem(system1_id);
    if (!system1) {
        ErrorLogger() << "Universe::LinearDistance passed invalid system id: " << system1_id;
        throw std::out_of_range("system1_id invalid");
    }
    std::shared_ptr<const System> system2 = GetSystem(system2_id);
    if (!system2) {
        ErrorLogger() << "Universe::LinearDistance passed invalid system id: " << system2_id;
        throw std::out_of_range("system2_id invalid");
    }
    double x_dist = system2->X() - system1->X();
    double y_dist = system2->Y() - system1->Y();
    return std::sqrt(x_dist*x_dist + y_dist*y_dist);
}

short Pathfinder::JumpDistanceBetweenSystems(int system1_id, int system2_id) const {
    return pimpl->JumpDistanceBetweenSystems(system1_id, system2_id);
}

short Pathfinder::PathfinderImpl::JumpDistanceBetweenSystems(int system1_id, int system2_id) const {
    if (system1_id == system2_id)
        return 0;

    try {
        distance_matrix_cache< distance_matrix_storage<short>> cache(m_system_jumps);

        size_t system1_index = m_system_id_to_graph_index.at(system1_id);
        size_t system2_index = m_system_id_to_graph_index.at(system2_id);
        size_t smaller_index = (std::min)(system1_index, system2_index);
        size_t other_index   = (std::max)(system1_index, system2_index);
        // prefer filling the smaller row/column for increased cache locality
        short jumps = cache.get_T(
            smaller_index, other_index,
            boost::bind(&Pathfinder::PathfinderImpl::HandleCacheMiss, this, _1, _2));
        if (jumps == SHRT_MAX)  // value returned for no valid path
            return -1;
        return jumps;
    } catch (const std::out_of_range&) {
        ErrorLogger() << "PathfinderImpl::JumpDistanceBetweenSystems passed invalid system id(s): "
                      << system1_id << " & " << system2_id;
        throw;
    }
}

namespace {
    /**GeneralizedLocationType abstracts the location of a UniverseObject.

       GeneralizedLocationType can be nowhere, one system or two systems in the
       case of a ship or fleet in transit.
       The returned variant is
       null_ptr            -- nowhere
       System id           -- somewhere
       pair of System ids  -- in transit

    */

    typedef boost::variant<std::nullptr_t, int, std::pair<int, int>> GeneralizedLocationType;

    /** Return the location of \p obj.*/
    GeneralizedLocationType GeneralizedLocation(const std::shared_ptr<const UniverseObject>& obj) {
        if (!obj)
            return nullptr;

        int system_id = obj->SystemID();
        auto system = GetSystem(system_id);
        if (system)
            return system_id;

        auto fleet = FleetFromObject(obj);
        if (fleet)
            return std::make_pair(fleet->PreviousSystemID(), fleet->NextSystemID());

        if (std::dynamic_pointer_cast<const Field>(obj))
            return nullptr;

        // Don't generate an error message for temporary objects.
        if (obj->ID() == TEMPORARY_OBJECT_ID)
            return nullptr;

        ErrorLogger() << "GeneralizedLocationType unable to locate " << obj->Name() << "(" << obj->ID() << ")";
        return nullptr;
    }

    /** Return the location of the object with id \p object_id.*/
    GeneralizedLocationType GeneralizedLocation(int object_id) {
        auto obj = GetUniverseObject(object_id);
        return GeneralizedLocation(obj);
    }

}

/** JumpDistanceSys1Visitor and JumpDistanceSys2Visitor are variant visitors
    that can be used to determine the distance between a pair of objects
    locations represented as GeneralizedLocations.*/

/** JumpDistanceSys2Visitor determines the distance between \p _sys_id1 and the
    GeneralizedLocation that it is visiting.*/
struct JumpDistanceSys2Visitor : public boost::static_visitor<int> {
    JumpDistanceSys2Visitor(const Pathfinder::PathfinderImpl& _pf, int _sys_id1) :
        pf(_pf), sys_id1(_sys_id1)
    {}

    /** Return the maximum distance if this end is nowhere.  */
    int operator()(std::nullptr_t) const { return INT_MAX; }

    /** Simple case of two system ids, return the distance between systems.*/
    int operator()(int sys_id2) const {
        short sjumps = -1;
        try {
            sjumps = pf.JumpDistanceBetweenSystems(sys_id1, sys_id2);
        } catch (const std::out_of_range&) {
            ErrorLogger() << "JumpsBetweenObjects caught out of range exception sys_id1 = "
                          << sys_id1 << " sys_id2 = " << sys_id2;
            return INT_MAX;
        }
        int jumps = (sjumps == -1) ? INT_MAX : static_cast<int>(sjumps);
        return jumps;
    }

    /** A single system id and a fleet with two locations.  For an object in
        transit return the distance to the closest system.*/
    int operator()(std::pair<int, int> prev_next) const {
        short sjumps1 = -1, sjumps2 = -1;
        int prev_sys_id = prev_next.first, next_sys_id = prev_next.second;
        try {
            if (prev_sys_id != INVALID_OBJECT_ID)
                sjumps1 = pf.JumpDistanceBetweenSystems(sys_id1, prev_sys_id);
            if (next_sys_id != INVALID_OBJECT_ID)
                sjumps2 = pf.JumpDistanceBetweenSystems(sys_id1, next_sys_id);
        } catch (...) {
            ErrorLogger() << "JumpsBetweenObjects caught exception when calling JumpDistanceBetweenSystems";
        }
        int jumps1 = (sjumps1 == -1) ? INT_MAX : static_cast<int>(sjumps1);
        int jumps2 = (sjumps2 == -1) ? INT_MAX : static_cast<int>(sjumps2);

        int jumps = std::min(jumps1, jumps2);
        return jumps;
   }
    const Pathfinder::PathfinderImpl& pf;
    int sys_id1;
};

/** JumpDistanceSys1Visitor visits the first system and uses
    JumpDistanceSysVisitor2 to determines the distance between \p _sys_id2 and
    the GeneralizedLocation that it is visiting.*/
struct JumpDistanceSys1Visitor : public boost::static_visitor<int> {
    JumpDistanceSys1Visitor(const Pathfinder::PathfinderImpl& _pf,
                            const GeneralizedLocationType&_sys2_ids) :
        pf(_pf), sys2_ids(_sys2_ids)
    {}

    /** Return the maximum distance if the first object is nowhere.*/
    int operator()(std::nullptr_t) const { return INT_MAX; }

    /** For a single system, return the application of JumpDistanceSys2Visitor
        to the second system.*/
    int operator()(int sys_id1) const {
        JumpDistanceSys2Visitor visitor(pf, sys_id1);
        return boost::apply_visitor(visitor, sys2_ids);
    }

    /** For an object in transit, apply the JumpDistanceSys2Visitor and return
        the shortest distance.*/
    int operator()(std::pair<int, int> prev_next) const {
        short sjumps1 = -1, sjumps2 = -1;
        int prev_sys_id = prev_next.first, next_sys_id = prev_next.second;
        if (prev_sys_id != INVALID_OBJECT_ID) {
            JumpDistanceSys2Visitor visitor(pf, prev_sys_id);
            sjumps1 = boost::apply_visitor(visitor, sys2_ids);
        }
        if (next_sys_id!= INVALID_OBJECT_ID) {
            JumpDistanceSys2Visitor visitor(pf, next_sys_id);
            sjumps2 = boost::apply_visitor(visitor, sys2_ids);
        }

        int jumps1 = (sjumps1 == -1) ? INT_MAX : static_cast<int>(sjumps1);
        int jumps2 = (sjumps2 == -1) ? INT_MAX : static_cast<int>(sjumps2);

        int jumps = std::min(jumps1, jumps2);
        return jumps;
    }
    const Pathfinder::PathfinderImpl& pf;
    const GeneralizedLocationType& sys2_ids;
};

int Pathfinder::JumpDistanceBetweenObjects(int object1_id, int object2_id) const
{ return pimpl->JumpDistanceBetweenObjects(object1_id, object2_id); }

int Pathfinder::PathfinderImpl::JumpDistanceBetweenObjects(int object1_id, int object2_id) const {
    GeneralizedLocationType obj1 = GeneralizedLocation(object1_id);
    GeneralizedLocationType obj2 = GeneralizedLocation(object2_id);
    JumpDistanceSys1Visitor visitor(*this, obj2);
    return boost::apply_visitor(visitor, obj1);
}

std::pair<std::list<int>, double> Pathfinder::ShortestPath(int system1_id, int system2_id,
                                                           int empire_id/* = ALL_EMPIRES*/) const
{ return pimpl->ShortestPath(system1_id, system2_id, empire_id); }

std::pair<std::list<int>, double> Pathfinder::PathfinderImpl::ShortestPath(int system1_id, int system2_id,
                                                                           int empire_id/* = ALL_EMPIRES*/) const
{
    if (empire_id == ALL_EMPIRES) {
        // find path on full / complete system graph
        try {
            double linear_distance = LinearDistance(system1_id, system2_id);
            return ShortestPathImpl(m_graph_impl->system_graph, system1_id, system2_id,
                                    linear_distance, m_system_id_to_graph_index);
        } catch (const std::out_of_range&) {
            ErrorLogger() << "PathfinderImpl::ShortestPath passed invalid system id(s): "
                                   << system1_id << " & " << system2_id;
            throw;
        }
    }

    // find path on single empire's view of system graph
    auto graph_it = m_graph_impl->empire_system_graph_views.find(empire_id);
    if (graph_it == m_graph_impl->empire_system_graph_views.end()) {
        ErrorLogger() << "PathfinderImpl::ShortestPath passed unknown empire id: " << empire_id;
        throw std::out_of_range("PathfinderImpl::ShortestPath passed unknown empire id");
    }
    try {
        double linear_distance = LinearDistance(system1_id, system2_id);
        return ShortestPathImpl(*graph_it->second, system1_id, system2_id,
                                linear_distance, m_system_id_to_graph_index);
    } catch (const std::out_of_range&) {
        ErrorLogger() << "PathfinderImpl::ShortestPath passed invalid system id(s): "
                      << system1_id << " & " << system2_id;
        throw;
    }
}

std::pair<std::list<int>, double> Pathfinder::ShortestPath(int system1_id, int system2_id, int empire_id,
                                                           const SystemExclusionPredicateType& system_predicate) const
{ return pimpl->ShortestPath(system1_id, system2_id, empire_id, system_predicate); }

std::pair<std::list<int>, double> Pathfinder::PathfinderImpl::ShortestPath(
    int system1_id, int system2_id, int empire_id, const Pathfinder::SystemExclusionPredicateType& sys_pred) const
{
    if (empire_id == ALL_EMPIRES) {
        ErrorLogger() << "Invalid empire " << empire_id;
        throw std::out_of_range("PathfinderImpl::ShortestPath passed invalid empire id");
    }

    auto func_it = m_graph_impl->system_pred_graph_views.find(sys_pred);
    if (func_it == m_graph_impl->system_pred_graph_views.end()) {
        m_graph_impl->AddSystemPredicate(sys_pred);
        func_it = m_graph_impl->system_pred_graph_views.find(sys_pred);
        if (func_it == m_graph_impl->system_pred_graph_views.end()) {
            ErrorLogger() << "No graph views found for predicate";
            throw std::out_of_range("PathfinderImpl::ShortestPath No graph views found for predicate");
        }
    }
    auto graph_it = func_it->second.find(empire_id);
    if (graph_it == func_it->second.end()) {
        ErrorLogger() << "No graph view found for empire " << empire_id;
        throw std::out_of_range("PathfinderImpl::ShortestPath No graph view for empire");
    }

    try {
        auto linear_distance = LinearDistance(system1_id, system2_id);
        return ShortestPathImpl(*graph_it->second, system1_id, system2_id,
                                linear_distance, m_system_id_to_graph_index);
    } catch (const std::out_of_range&) {
        ErrorLogger() << "Invalid system id(s): " << system1_id << ", " << system2_id;
        throw;
    }
}

double Pathfinder::ShortestPathDistance(int object1_id, int object2_id) const {
    return pimpl->ShortestPathDistance(object1_id, object2_id);
}

double Pathfinder::PathfinderImpl::ShortestPathDistance(int object1_id, int object2_id) const {
    // If one or both objects are (in) a fleet between systems, use the destination system
    // and add the distance from the fleet to the destination system, essentially calculating
    // the distance travelled until both could be in the same system.
    std::shared_ptr<const UniverseObject> obj1 = GetUniverseObject(object1_id);
    if (!obj1)
        return -1;

    std::shared_ptr<const UniverseObject> obj2 = GetUniverseObject(object2_id);
    if (!obj2)
        return -1;

    std::shared_ptr<const System> system_one = GetSystem(obj1->SystemID());
    std::shared_ptr<const System> system_two = GetSystem(obj2->SystemID());
    std::pair< std::list< int >, double > path_len_pair;
    double dist1(0.0), dist2(0.0);
    std::shared_ptr<const Fleet> fleet;

    if (!system_one) {
        fleet = FleetFromObject(obj1);
        if (!fleet)
            return -1;
        if (std::shared_ptr<const System> next_sys = GetSystem(fleet->NextSystemID())) {
            system_one = next_sys;
            dist1 = std::sqrt(pow((next_sys->X() - fleet->X()), 2) + pow((next_sys->Y() - fleet->Y()), 2));
        }
    }

    if (!system_two) {
        fleet = FleetFromObject(obj2);
        if (!fleet)
            return -1;
        if (std::shared_ptr<const System> next_sys = GetSystem(fleet->NextSystemID())) {
            system_two = next_sys;
            dist2 = std::sqrt(pow((next_sys->X() - fleet->X()), 2) + pow((next_sys->Y() - fleet->Y()), 2));
        }
    }

    try {
        path_len_pair = ShortestPath(system_one->ID(), system_two->ID());
    } catch (...) {
        ErrorLogger() << "ShortestPathDistance caught exception when calling ShortestPath";
        return -1;
    }
    return path_len_pair.second + dist1 + dist2;
}

std::pair<std::list<int>, int> Pathfinder::LeastJumpsPath(
    int system1_id, int system2_id, int empire_id/* = ALL_EMPIRES*/, int max_jumps/* = INT_MAX*/) const {
    return pimpl->LeastJumpsPath(system1_id, system2_id, empire_id, max_jumps);
}

std::pair<std::list<int>, int> Pathfinder::PathfinderImpl::LeastJumpsPath(
    int system1_id, int system2_id, int empire_id/* = ALL_EMPIRES*/, int max_jumps/* = INT_MAX*/) const
{
    if (empire_id == ALL_EMPIRES) {
        // find path on full / complete system graph
        try {
            return LeastJumpsPathImpl(m_graph_impl->system_graph, system1_id, system2_id,
                                      m_system_id_to_graph_index, max_jumps);
        } catch (const std::out_of_range&) {
            ErrorLogger() << "PathfinderImpl::LeastJumpsPath passed invalid system id(s): "
                                   << system1_id << " & " << system2_id;
            throw;
        }
    }

    // find path on single empire's view of system graph
    auto graph_it = m_graph_impl->empire_system_graph_views.find(empire_id);
    if (graph_it == m_graph_impl->empire_system_graph_views.end()) {
        ErrorLogger() << "PathfinderImpl::LeastJumpsPath passed unknown empire id: " << empire_id;
        throw std::out_of_range("PathfinderImpl::LeastJumpsPath passed unknown empire id");
    }
    try {
        return LeastJumpsPathImpl(*graph_it->second, system1_id, system2_id,
                                  m_system_id_to_graph_index, max_jumps);
    } catch (const std::out_of_range&) {
        ErrorLogger() << "PathfinderImpl::LeastJumpsPath passed invalid system id(s): "
                               << system1_id << " & " << system2_id;
        throw;
    }
}

bool Pathfinder::SystemsConnected(int system1_id, int system2_id, int empire_id) const
{ return pimpl->SystemsConnected(system1_id, system2_id, empire_id); }

bool Pathfinder::PathfinderImpl::SystemsConnected(int system1_id, int system2_id, int empire_id) const {
    TraceLogger() << "SystemsConnected(" << system1_id << ", " << system2_id << ", " << empire_id << ")";
    auto path = LeastJumpsPath(system1_id, system2_id, empire_id);
    TraceLogger() << "SystemsConnected returned path of size: " << path.first.size();
    bool retval = !path.first.empty();
    TraceLogger() << "SystemsConnected retval: " << retval;
    return retval;
}

bool Pathfinder::SystemHasVisibleStarlanes(int system_id, int empire_id) const
{ return pimpl->SystemHasVisibleStarlanes(system_id, empire_id); }

bool Pathfinder::PathfinderImpl::SystemHasVisibleStarlanes(int system_id, int empire_id) const {
    if (auto system = GetEmpireKnownSystem(system_id, empire_id))
        if (!system->StarlanesWormholes().empty())
            return true;
    return false;
}

std::multimap<double, int> Pathfinder::ImmediateNeighbors(int system_id, int empire_id/* = ALL_EMPIRES*/) const
{ return pimpl->ImmediateNeighbors(system_id, empire_id); }

std::multimap<double, int> Pathfinder::PathfinderImpl::ImmediateNeighbors(
    int system_id, int empire_id/* = ALL_EMPIRES*/) const
{
    if (empire_id == ALL_EMPIRES) {
        return ImmediateNeighborsImpl(m_graph_impl->system_graph, system_id,
                                      m_system_id_to_graph_index);
    } else {
        auto graph_it = m_graph_impl->empire_system_graph_views.find(empire_id);
        if (graph_it != m_graph_impl->empire_system_graph_views.end())
            return ImmediateNeighborsImpl(*graph_it->second, system_id,
                                          m_system_id_to_graph_index);
    }
    return std::multimap<double, int>();
}


void Pathfinder::PathfinderImpl::WithinJumpsCacheHit(
    std::unordered_set<int>* result, size_t jump_limit,
    size_t ii, distance_matrix_storage<short>::row_ref row) const
{
    // Scan the LUT of system ids and add any result from the row within
    // the neighborhood range to the results.
    for (auto system_id_and_ii : m_system_id_to_graph_index) {
        size_t hops = row[system_id_and_ii.second];
        if (hops <= jump_limit)
            result->insert(system_id_and_ii.first);
    }
}

std::unordered_set<int> Pathfinder::WithinJumps(size_t jumps, const std::vector<int>& candidates) const
{ return pimpl->WithinJumps(jumps, candidates); }

std::unordered_set<int> Pathfinder::PathfinderImpl::WithinJumps(
    size_t jumps, const std::vector<int>& candidates) const
{
    std::unordered_set<int> near;
    distance_matrix_cache< distance_matrix_storage<short>> cache(m_system_jumps);
    for (auto candidate : candidates) {
        size_t system_index;
        try {
            system_index = m_system_id_to_graph_index.at(candidate);
        } catch (const std::out_of_range& e) {
            ErrorLogger() << "Passed invalid system id: " << candidate;
            continue;
        }

        near.insert(candidate);
        if (jumps == 0)
            continue;

        cache.examine_row(system_index,
                          boost::bind(&Pathfinder::PathfinderImpl::HandleCacheMiss, this, _1, _2),
                          boost::bind(&Pathfinder::PathfinderImpl::WithinJumpsCacheHit, this,
                                      &near, jumps, _1, _2));
    }
    return near;
}



/** Examine a single universe object and determine if it is within jumps
    of any object in others.*/
struct WithinJumpsOfOthersObjectVisitor : public boost::static_visitor<bool> {
    WithinJumpsOfOthersObjectVisitor(const Pathfinder::PathfinderImpl& _pf,
                                     int _jumps,
                                     const std::vector<std::shared_ptr<const UniverseObject>>& _others) :
        pf(_pf), jumps(_jumps), others(_others)
    {}

    bool operator()(std::nullptr_t) const { return false; }
    bool operator()(int sys_id) const {
        bool retval = pf.WithinJumpsOfOthers(jumps, sys_id, others);
        return retval;
    }
    bool operator()(std::pair<int, int> prev_next) const {
        return pf.WithinJumpsOfOthers(jumps, prev_next.first, others)
            || pf.WithinJumpsOfOthers(jumps, prev_next.second, others);
    }
    const Pathfinder::PathfinderImpl& pf;
    int jumps;
    const std::vector<std::shared_ptr<const UniverseObject>>& others;
};

/** Examine a single other in the cache to see if any of its locations
    are within jumps*/
struct WithinJumpsOfOthersOtherVisitor : public boost::static_visitor<bool> {
    WithinJumpsOfOthersOtherVisitor(const Pathfinder::PathfinderImpl& _pf,
                                    int _jumps,
                                    distance_matrix_storage<short>::row_ref _row) :
        pf(_pf), jumps(_jumps), row(_row)
    {}

    bool single_result(int other_id) const {
        int index;
        try {
            index = pf.m_system_id_to_graph_index.at(other_id);
        } catch (const std::out_of_range& e) {
            ErrorLogger() << "Passed invalid system id: " << other_id;
            return false;
        }
        bool retval = (row[index] <= jumps);
        return retval;
    }

    bool operator()(std::nullptr_t) const { return false; }
    bool operator()(int sys_id) const {
        return single_result(sys_id);
    }
    bool operator()(std::pair<int, int> prev_next) const {
        return single_result(prev_next.first)
            || single_result(prev_next.second);
    }
    const Pathfinder::PathfinderImpl& pf;
    int jumps;
    distance_matrix_storage<short>::row_ref row;
};


void Pathfinder::PathfinderImpl::WithinJumpsOfOthersCacheHit(
    bool& answer, int jumps,
    const std::vector<std::shared_ptr<const UniverseObject>>& others,
    size_t ii, distance_matrix_storage<short>::row_ref row) const
{
    // Check if any of the others are within jumps of candidate, by looping
    // through all of the others and applying the WithinJumpsOfOthersOtherVisitor.
    answer = false;
    for (const auto& other : others) {
        WithinJumpsOfOthersOtherVisitor check_if_location_is_within_jumps(*this, jumps, row);
        GeneralizedLocationType location = GeneralizedLocation(other);
        if (boost::apply_visitor(check_if_location_is_within_jumps, location)) {
            answer = true;
            return;
        }
    }
}

std::pair<std::vector<std::shared_ptr<const UniverseObject>>,
          std::vector<std::shared_ptr<const UniverseObject>>>
Pathfinder::WithinJumpsOfOthers(
    int jumps,
    const std::vector<std::shared_ptr<const UniverseObject>>& candidates,
    const std::vector<std::shared_ptr<const UniverseObject>>& stationary) const
{
    return pimpl->WithinJumpsOfOthers(jumps, candidates, stationary);
}

std::pair<std::vector<std::shared_ptr<const UniverseObject>>,
          std::vector<std::shared_ptr<const UniverseObject>>>
Pathfinder::PathfinderImpl::WithinJumpsOfOthers(
    int jumps,
    const std::vector<std::shared_ptr<const UniverseObject>>& candidates,
    const std::vector<std::shared_ptr<const UniverseObject>>& stationary) const
{
    // Examine each candidate and copy those within jumps of the
    // others into near and the rest into far.
    WithinJumpsOfOthersObjectVisitor visitor(*this, jumps, stationary);
    std::vector<std::shared_ptr<const UniverseObject>> near, far;
    size_t size = candidates.size();
    near.reserve(size);
    far.reserve(size);

    for (const auto& candidate : candidates) {
        GeneralizedLocationType candidate_systems = GeneralizedLocation(candidate);
        bool is_near = boost::apply_visitor(visitor, candidate_systems);

        if (is_near)
            near.push_back(candidate);
        else
            far.push_back(candidate);
    }

    return {near, far}; //, wherever you are...
}

bool Pathfinder::PathfinderImpl::WithinJumpsOfOthers(
    int jumps, int system_id,
    const std::vector<std::shared_ptr<const UniverseObject>>& others) const
{
    if (others.empty())
        return false;

    size_t system_index;
    try {
        system_index = m_system_id_to_graph_index.at(system_id);
    } catch (const std::out_of_range& e) {
        ErrorLogger() << "Passed invalid system id: " << system_id;
        return false;
    }

    // Examine the cache to see if \p system_id is within \p jumps of \p others
    bool within_jumps(false);
    distance_matrix_cache<distance_matrix_storage<short>> cache(m_system_jumps);
    cache.examine_row(system_index,
                      boost::bind(&Pathfinder::PathfinderImpl::HandleCacheMiss, this, _1, _2),
                      boost::bind(&Pathfinder::PathfinderImpl::WithinJumpsOfOthersCacheHit, this,
                                  boost::ref(within_jumps), jumps, others, _1, _2));
    return within_jumps;
}

int Pathfinder::NearestSystemTo(double x, double y) const
{ return pimpl->NearestSystemTo(x, y); }

int Pathfinder::PathfinderImpl::NearestSystemTo(double x, double y) const {
    double min_dist2 = DBL_MAX;
    int min_dist2_sys_id = INVALID_OBJECT_ID;

    auto systems = Objects().FindObjects<System>();

    for (auto const& system : systems) {
        double xs = system->X();
        double ys = system->Y();
        double dist2 = (xs-x)*(xs-x) + (ys-y)*(ys-y);
        if (dist2 == 0.0) {
            return system->ID();
        } else if (dist2 < min_dist2) {
            min_dist2 = dist2;
            min_dist2_sys_id = system->ID();
        }
    }
    return min_dist2_sys_id;
}


void Pathfinder::InitializeSystemGraph(const std::vector<int> system_ids, int for_empire_id)
{ return pimpl->InitializeSystemGraph(system_ids, for_empire_id); }

void Pathfinder::PathfinderImpl::InitializeSystemGraph(
    const std::vector<int> system_ids, int for_empire_id)
{
    auto new_graph_impl = std::make_shared<GraphImpl>();
    // auto system_ids = ::EmpireKnownObjects(for_empire_id).FindObjectIDs<System>();
    // NOTE: this initialization of graph_changed prevents testing for edges between nonexistant vertices
    bool graph_changed = system_ids.size() != boost::num_vertices(m_graph_impl->system_graph);

    auto ints_to_string = [](const std::vector<int>& ints_vec) {
        std::stringstream o;
        for (auto id : ints_vec)
            o << id << " ";
        return o.str();
    };
    TraceLogger() << "InitializeSystemGraph(" << for_empire_id
                  << ") system_ids: (" << system_ids.size() << "): "
                  << ints_to_string(system_ids);

    GraphImpl::SystemIDPropertyMap sys_id_property_map =
        boost::get(vertex_system_id_t(), new_graph_impl->system_graph);

    GraphImpl::EdgeWeightPropertyMap edge_weight_map =
        boost::get(boost::edge_weight, new_graph_impl->system_graph);

    const GraphImpl::EdgeWeightPropertyMap& current_edge_weight_map =
        boost::get(boost::edge_weight, m_graph_impl->system_graph);

    // add vertices to graph for all systems
    for (size_t system_index = 0; system_index < system_ids.size(); ++system_index) {
        // add a vertex to the graph for this system, and assign it the system's universe ID as a property
        boost::add_vertex(new_graph_impl->system_graph);
        int system_id = system_ids[system_index];
        sys_id_property_map[system_index] = system_id;
        // add record of index in new_graph_impl->system_graph of this system
        m_system_id_to_graph_index[system_id] = system_index;
    }

    // add edges for all starlanes
    for (size_t system1_index = 0; system1_index < system_ids.size(); ++system1_index) {
        int system1_id = system_ids[system1_index];
        std::shared_ptr<const System> system1 = GetEmpireKnownSystem(system1_id, for_empire_id);
        //std::shared_ptr<const System> & system1 = systems[system1_index];

        // add edges and edge weights
        for (auto const& lane_dest : system1->StarlanesWormholes()) {
            // get id in universe of system at other end of lane
            const int lane_dest_id = lane_dest.first;
            // skip null lanes and only add edges in one direction, to avoid
            // duplicating edges ( since this is an undirected graph, A->B
            // duplicates B->A )
            if (lane_dest_id >= system1_id)
                continue;

            // get new_graph_impl->system_graph index for this system
            auto reverse_lookup_map_it = m_system_id_to_graph_index.find(lane_dest_id);
            if (reverse_lookup_map_it == m_system_id_to_graph_index.end())
                continue;   // couldn't find destination system id in vertex lookup map; don't add to graph
            size_t lane_dest_graph_index = reverse_lookup_map_it->second;

            auto add_edge_result = boost::add_edge(system1_index, lane_dest_graph_index,
                                                   new_graph_impl->system_graph);

            if (add_edge_result.second) {   // if this is a non-duplicate starlane or wormhole
                if (lane_dest.second) {         // if this is a wormhole
                    edge_weight_map[add_edge_result.first] = WORMHOLE_TRAVEL_DISTANCE;
                } else {                        // if this is a starlane
                    edge_weight_map[add_edge_result.first] = LinearDistance(system1_id, lane_dest_id);
                }

                if (!graph_changed) {
                    const auto maybe_current_edge = boost::edge(system1_index, lane_dest_graph_index, m_graph_impl->system_graph);
                    // Does the current edge exist with the same weight in the old graph
                    graph_changed = (!maybe_current_edge.second
                                     || (edge_weight_map[add_edge_result.first]
                                         != current_edge_weight_map[maybe_current_edge.first]));
                }
            }
        }
    }

    // if all previous edges still exist in the new graph, and the number of vertices and edges hasn't changed,
    // then no vertices or edges can have been added either, so it is still the same graph
    graph_changed = graph_changed ||
        boost::num_edges(new_graph_impl->system_graph) !=
            boost::num_edges(m_graph_impl->system_graph);

    if (graph_changed) {
        new_graph_impl.swap(m_graph_impl);
        // clear jumps distance cache
        // NOTE: re-filling the cache is O(#vertices * (#vertices + #edges)) in the worst case!
        m_system_jumps.resize(system_ids.size());
    }
    UpdateEmpireVisibilityFilteredSystemGraphs(for_empire_id);
}

void Pathfinder::UpdateEmpireVisibilityFilteredSystemGraphs(int for_empire_id)
{ return pimpl->UpdateEmpireVisibilityFilteredSystemGraphs(for_empire_id); }

void Pathfinder::PathfinderImpl::UpdateEmpireVisibilityFilteredSystemGraphs(int for_empire_id) {
    m_graph_impl->empire_system_graph_views.clear();
    m_graph_impl->system_pred_graph_views.clear();

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
        for (auto const& empire : Empires()) {
            int empire_id = empire.first;
            GraphImpl::EdgeVisibilityFilter filter(&m_graph_impl->system_graph, empire_id);
            auto filtered_graph_ptr = std::make_shared<GraphImpl::EmpireViewSystemGraph>(m_graph_impl->system_graph, filter);
            m_graph_impl->empire_system_graph_views[empire_id] = filtered_graph_ptr;
        }

    } else {
        // all empires share a single filtered graph, filtered by the for_empire_id
        GraphImpl::EdgeVisibilityFilter filter(&m_graph_impl->system_graph, for_empire_id);
        auto filtered_graph_ptr = std::make_shared<GraphImpl::EmpireViewSystemGraph>(m_graph_impl->system_graph, filter);

        for (auto const& empire : Empires()) {
            int empire_id = empire.first;
            m_graph_impl->empire_system_graph_views[empire_id] = filtered_graph_ptr;
        }
    }
    for (const auto& prev_pred : m_graph_impl->system_predicates)
        m_graph_impl->AddSystemPredicate(prev_pred);
}
