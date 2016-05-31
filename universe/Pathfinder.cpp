#include "Pathfinder.h"

#include "../util/OptionsDB.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/RunQueue.h"
#include "../util/ScopedTimer.h"
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
#include "Predicates.h"
#include "Special.h"
#include "Species.h"
#include "Condition.h"
#include "ValueRef.h"
#include "Universe.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/timer.hpp>

#include <algorithm>
#include <cmath>
#include <list>
#include <stdexcept>

namespace {
    const double    WORMHOLE_TRAVEL_DISTANCE = 0.1;         // the effective distance for ships travelling along a wormhole, for determining how much of their speed is consumed by the jump
}

extern const int ALL_EMPIRES;

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
        typedef std::vector<T>& row_ref; ///< A row type protected by a mutex.

        distance_matrix_storage() {};
        distance_matrix_storage(const distance_matrix_storage<T>& src)
        { resize(src.m_data.size()); };
        //TODO C++11 Move Constructor.

        /**Number of rows and columns. (N)*/
        size_t size()
        { return m_data.size(); }

        /**Resize and clear all mutexes.  Assumes that table is locked.*/
        void resize(size_t a_size) {
            const size_t old_size = size();

            m_data.clear();
            m_data.resize(a_size);
            m_row_mutexes.resize(a_size);
            for (size_t i = old_size; i < a_size; ++i)
                m_row_mutexes[i] = boost::shared_ptr<boost::shared_mutex>(new boost::shared_mutex());
        }

        /**N x N table of hop distances in row column form.*/
        std::vector< std::vector<T> > m_data;
        /**Per row mutexes.*/
        std::vector< boost::shared_ptr<boost::shared_mutex> > m_row_mutexes;
        /**Table mutex*/
        boost::shared_mutex m_mutex;
    };


    /**distance_matrix_cache is a cache of symmetric hop distances
       based on distance_matrix_storage.

    It enforces the locking convention with get_or_row_lock which
    returns either a single intgral value or an entire write locked
    row.

    All operations first lock the table and then when necessary the row.
    */
    //TODO: Consider replacing this scheme with thread local storage.  It
    //may be true that because this is a computed value that depends on
    //the system topology that almost never changes that the
    //synchronization costs way out-weight the save computation costs.
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

        /**row_lock has a mutex for the whole table and for the given row.*/
        class row_lock {
            private:
            /**Table lock.*/
            boost::shared_lock<boost::shared_mutex> m_lock;
            /**Row lock.*/
            boost::unique_lock<boost::shared_mutex> m_row_lock;

            /**Swap the table lock with \p guard and the row lock with
               \p row_guard.*/
            void swap(boost::shared_lock<boost::shared_mutex>& guard, boost::unique_lock<boost::shared_mutex>& row_guard) {
                m_lock.swap(guard);
                m_row_lock.swap(row_guard);
            }
            friend class distance_matrix_cache<Storage, T, Row>;

            public:
            row_lock() {};
            /**Swap locks with \p other row_lock.*/
            void swap(row_lock& other) {
                m_lock.swap(other.m_lock);
                m_row_lock.swap(other.m_row_lock);
            }
            /**Unlock.*/
            void unlock() {
                m_row_lock.unlock();
                m_lock.unlock();
            }
        };

        public:
        /** try to retrieve an element, lock the whole row on cache miss
          * if \p lock already holds a lock, it will be unlocked after locking the row.
          */
        boost::optional<T> get_or_lock_row(size_t row_index, size_t column_index, row_lock& lock) const {
            boost::shared_lock<boost::shared_mutex> guard(m_storage.m_mutex);

            if (row_index < m_storage.size() && column_index < m_storage.size()) {
                {
                    boost::shared_lock<boost::shared_mutex> row_guard(*m_storage.m_row_mutexes[row_index]);
                    Row row_data = m_storage.m_data[row_index];

                    if (column_index < row_data.size())
                        return row_data[column_index];
                }
                {
                    boost::shared_lock<boost::shared_mutex> column_guard(*m_storage.m_row_mutexes[column_index]);
                    Row column_data = m_storage.m_data[column_index];

                    if (row_index < column_data.size())
                        return column_data[row_index];
                }
                {
                    boost::unique_lock<boost::shared_mutex> row_guard(*m_storage.m_row_mutexes[row_index]);
                    Row row_data = m_storage.m_data[row_index];

                    if (column_index < row_data.size()) {
                        return row_data[column_index];
                    } else {
                        lock.swap(guard, row_guard);

                        return boost::optional<T>();
                    }
                }
            } else {
                ErrorLogger() << "distance_matrix_cache::get_or_lock_row passed invalid node indices: "
                              << row_index << "," << column_index << " matrix size: " << m_storage.size();
                if (row_index < m_storage.size())
                    throw std::out_of_range("column_index invalid");
                else
                    throw std::out_of_range("row_index invalid");
            }

            return boost::optional<T>(); // unreachable
        }

        /** replace the contents of a row with \p new_data.
          * precondition: \p lock must hold a lock to the specified row.
          */
        void swap_and_unlock_row(size_t row_index, Row new_data, row_lock& lock) {
            if (row_index < m_storage.size()) {
                Row row_data = m_storage.m_data[row_index];

                row_data.swap(new_data);
            } else {
                ErrorLogger() << "distance_matrix_cache::swap_and_unlock_row passed invalid node index: "
                              << row_index << " matrix size: " << m_storage.size();
                throw std::out_of_range("row_index invalid");
            }

            lock.unlock(); // only unlock on success
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
        std::pair<OutEdgeIterator, OutEdgeIterator> edges = boost::out_edges(id_to_graph_index.at(system_id), graph);
        for (OutEdgeIterator it = edges.first; it != edges.second; ++it)
        { retval.insert(std::make_pair(edge_weight_map[*it], sys_id_property_map[boost::target(*it, graph)])); }
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
                TemporaryPtr<const System> system1 = GetEmpireKnownSystem(sys_id_1, m_empire_id);
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
    double ShortestPathDistance(int object1_id, int object2_id) const;
    std::pair<std::list<int>, int> LeastJumpsPath(
        int system1_id, int system2_id, int empire_id = ALL_EMPIRES, int max_jumps = INT_MAX) const;
    bool SystemsConnected(int system1_id, int system2_id, int empire_id = ALL_EMPIRES) const;
    bool SystemHasVisibleStarlanes(int system_id, int empire_id = ALL_EMPIRES) const;
    std::multimap<double, int> ImmediateNeighbors(int system_id, int empire_id = ALL_EMPIRES) const;
    int NearestSystemTo(double x, double y) const;
    void InitializeSystemGraph(const std::vector<int> system_ids, int for_empire_id = ALL_EMPIRES);
    void UpdateEmpireVisibilityFilteredSystemGraphs(int for_empire_id = ALL_EMPIRES);


    mutable distance_matrix_storage<short>  m_system_jumps;             ///< indexed by system graph index (not system id), caches the smallest number of jumps to travel between all the systems
    boost::shared_ptr<GraphImpl>            m_graph_impl;               ///< a graph in which the systems are vertices and the starlanes are edges
    boost::unordered_map<int, size_t>       m_system_id_to_graph_index;
};

/////////////////////////////////////////////
// class Pathfinder
/////////////////////////////////////////////
Pathfinder::Pathfinder() :
    pimpl(new PathfinderImpl)
{}

Pathfinder::~Pathfinder() {
}

namespace {
    TemporaryPtr<const Fleet> FleetFromObject(TemporaryPtr<const UniverseObject> obj) {
        TemporaryPtr<const Fleet> retval = boost::dynamic_pointer_cast<const Fleet>(obj);
        if (!retval) {
            if (TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(obj))
                retval = GetFleet(ship->FleetID());
        }
        return retval;
    }
}

double Pathfinder::LinearDistance(int system1_id, int system2_id) const {
    return pimpl->LinearDistance(system1_id, system2_id);
}

double Pathfinder::PathfinderImpl::LinearDistance(int system1_id, int system2_id) const {
    TemporaryPtr<const System> system1 = GetSystem(system1_id);
    if (!system1) {
        ErrorLogger() << "Universe::LinearDistance passed invalid system id: " << system1_id;
        throw std::out_of_range("system1_id invalid");
    }
    TemporaryPtr<const System> system2 = GetSystem(system2_id);
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
        distance_matrix_cache< distance_matrix_storage<short> > cache(m_system_jumps);
        distance_matrix_cache< distance_matrix_storage<short> >::row_lock cache_guard;
        size_t system1_index = m_system_id_to_graph_index.at(system1_id);
        size_t system2_index = m_system_id_to_graph_index.at(system2_id);
        size_t smaller_index = (std::min)(system1_index, system2_index);
        size_t other_index   = (std::max)(system1_index, system2_index);
        boost::optional<short> maybe_jumps = cache.get_or_lock_row(smaller_index, other_index, cache_guard); // prefer filling the smaller row/column for increased cache locality
        short jumps;

        if (maybe_jumps) {
            // cache hit, any locks are already released
            // get_value_or() in order to silence potentially-uninitialized warning
            jumps = maybe_jumps.get_value_or(SHRT_MAX);
        } else {
            // cache miss, still holding a lock in cache_guard
            // we are keeping the row locked during computation so other
            // threads waiting for the same row will see a cache hit
            typedef boost::iterator_property_map<std::vector<short>::iterator, boost::identity_property_map> DistancePropertyMap;

            std::vector<short> private_distance_buffer(m_system_jumps.size(), SHRT_MAX);
            DistancePropertyMap distance_property_map(private_distance_buffer.begin());
            boost::distance_recorder<DistancePropertyMap, boost::on_tree_edge> distance_recorder(distance_property_map);

            // FIXME: dont compute m_system_jumps[i][j] again as m_system_jumps[j][i]
            private_distance_buffer[smaller_index] = 0;
            boost::breadth_first_search(m_graph_impl->system_graph, smaller_index, boost::visitor(boost::make_bfs_visitor(distance_recorder)));
            jumps = private_distance_buffer[other_index];
            cache.swap_and_unlock_row(smaller_index, private_distance_buffer, cache_guard);
        }
        if (jumps == SHRT_MAX)  // value returned for no valid path
            return -1;

        return jumps;
    } catch (const std::out_of_range&) {
        ErrorLogger() << "PathfinderImpl::JumpDistanceBetweenSystems passed invalid system id(s): "
                               << system1_id << " & " << system2_id;
        throw;
    }
}

int Pathfinder::JumpDistanceBetweenObjects(int object1_id, int object2_id) const {
    return pimpl->JumpDistanceBetweenObjects(object1_id, object2_id);
}

int Pathfinder::PathfinderImpl::JumpDistanceBetweenObjects(int object1_id, int object2_id) const {
    TemporaryPtr<const UniverseObject> obj1 = GetUniverseObject(object1_id);
    if (!obj1)
        return INT_MAX;

    TemporaryPtr<const UniverseObject> obj2 = GetUniverseObject(object2_id);
    if (!obj2)
        return INT_MAX;

    TemporaryPtr<const System> system_one = GetSystem(obj1->SystemID());
    TemporaryPtr<const System> system_two = GetSystem(obj2->SystemID());

    if (system_one && system_two) {
        // both condition-matching object and candidate are / in systems.
        // can just find the shortest path between the two systems
        short jumps = -1;
        try {
            jumps = JumpDistanceBetweenSystems(system_one->ID(), system_two->ID());
        } catch (...) {
            ErrorLogger() << "JumpsBetweenObjects caught exception when calling JumpDistanceBetweenSystems";
        }
        if (jumps != -1)    // if jumps is -1, no path exists between the systems
            return static_cast<int>(jumps);
        else
            return INT_MAX;

    } else if (system_one) {
        // just object one is / in a system.
        // TODO generalize Object to report multiple systems for things
        // like fleets in transit
        if (TemporaryPtr<const Fleet> fleet = FleetFromObject(obj2)) {
            // other object is a fleet that is between systems
            // need to check shortest path from systems on either side of starlane fleet is on
            short jumps1 = -1, jumps2 = -1;
            try {
                if (fleet->PreviousSystemID() != -1)
                    jumps1 = JumpDistanceBetweenSystems(system_one->ID(), fleet->PreviousSystemID());
                if (fleet->NextSystemID() != -1)
                    jumps2 = JumpDistanceBetweenSystems(system_one->ID(), fleet->NextSystemID());
            } catch (...) {
                ErrorLogger() << "JumpsBetweenObjects caught exception when calling JumpDistanceBetweenSystems";
            }
            int jumps = static_cast<int>(std::max(jumps1, jumps2));
            if (jumps != -1) {
                return jumps - 1;
            } else {
                // no path
                return INT_MAX;
            }
        }

    } else if (system_two) {
        // just object two is a system.
        if (TemporaryPtr<const Fleet> fleet = FleetFromObject(obj1)) {
            // other object is a fleet that is between systems
            // need to check shortest path from systems on either side of starlane fleet is on
            short jumps1 = -1, jumps2 = -1;
            try {
                if (fleet->PreviousSystemID() != -1)
                    jumps1 = JumpDistanceBetweenSystems(system_two->ID(), fleet->PreviousSystemID());
                if (fleet->NextSystemID() != -1)
                    jumps2 = JumpDistanceBetweenSystems(system_two->ID(), fleet->NextSystemID());
            } catch (...) {
                ErrorLogger() << "JumpsBetweenObjects caught exception when calling JumpDistanceBetweenSystems";
            }
            int jumps = static_cast<int>(std::max(jumps1, jumps2));
            if (jumps != -1) {
                return jumps - 1;
            } else {
                // no path
                return INT_MAX;
            }
        }
    } else {
        // neither object is / in a system

        TemporaryPtr<const Fleet> fleet_one = FleetFromObject(obj1);
        TemporaryPtr<const Fleet> fleet_two = FleetFromObject(obj2);

        if (fleet_one && fleet_two) {
            // both objects are / in a fleet.
            // need to check all combinations of systems on either sides of
            // starlanes condition-matching object and candidate are on
            int fleet_one_prev_system_id = fleet_one->PreviousSystemID();
            int fleet_one_next_system_id = fleet_one->NextSystemID();
            int fleet_two_prev_system_id = fleet_two->PreviousSystemID();
            int fleet_two_next_system_id = fleet_two->NextSystemID();
            short jumps1 = -1, jumps2 = -1, jumps3 = -1, jumps4 = -1;
            try {
                if (fleet_one_prev_system_id != -1 && fleet_two_prev_system_id != -1)
                    jumps1 = JumpDistanceBetweenSystems(fleet_one_prev_system_id, fleet_two_prev_system_id);
                if (fleet_one_prev_system_id != -1 && fleet_two_next_system_id != -1)
                    jumps2 = JumpDistanceBetweenSystems(fleet_one_prev_system_id, fleet_two_next_system_id);
                if (fleet_one_next_system_id != -1 && fleet_two_prev_system_id != -1)
                    jumps3 = JumpDistanceBetweenSystems(fleet_one_next_system_id, fleet_two_prev_system_id);
                if (fleet_one_next_system_id != -1 && fleet_two_next_system_id != -1)
                    jumps4 = JumpDistanceBetweenSystems(fleet_one_next_system_id, fleet_two_next_system_id);
            } catch (...) {
                ErrorLogger() << "JumpsBetweenObjects caught exception when calling JumpDistanceBetweenSystems";
            }
            int jumps = static_cast<int>(std::max(jumps1, std::max(jumps2, std::max(jumps3, jumps4))));
            if (jumps != -1) {
                return jumps - 1;
            } else {
                // no path
                return INT_MAX;
            }
        }
    }
    return INT_MAX;
}


std::pair<std::list<int>, double> Pathfinder::ShortestPath(int system1_id, int system2_id, int empire_id/* = ALL_EMPIRES*/) const {
    return pimpl->ShortestPath(system1_id, system2_id, empire_id);
}

std::pair<std::list<int>, double> Pathfinder::PathfinderImpl::ShortestPath(int system1_id, int system2_id, int empire_id/* = ALL_EMPIRES*/) const {
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
    GraphImpl::EmpireViewSystemGraphMap::const_iterator graph_it =
        m_graph_impl->empire_system_graph_views.find(empire_id);
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

double Pathfinder::ShortestPathDistance(int object1_id, int object2_id) const {
    return pimpl->ShortestPathDistance(object1_id, object2_id);
}

double Pathfinder::PathfinderImpl::ShortestPathDistance(int object1_id, int object2_id) const {
    // If one or both objects are (in) a fleet between systems, use the destination system
    // and add the distance from the fleet to the destination system, essentially calculating
    // the distance travelled until both could be in the same system.
    TemporaryPtr<const UniverseObject> obj1 = GetUniverseObject(object1_id);
    if (!obj1)
        return -1;

    TemporaryPtr<const UniverseObject> obj2 = GetUniverseObject(object2_id);
    if (!obj2)
        return -1;

    TemporaryPtr<const System> system_one = GetSystem(obj1->SystemID());
    TemporaryPtr<const System> system_two = GetSystem(obj2->SystemID());
    std::pair< std::list< int >, double > path_len_pair;
    double dist1(0.0), dist2(0.0);
    TemporaryPtr<const Fleet> fleet;

    if (!system_one) {
        fleet = FleetFromObject(obj1);
        if (!fleet)
            return -1;
        if (TemporaryPtr<const System> next_sys = GetSystem(fleet->NextSystemID())) {
            system_one = next_sys;
            dist1 = std::sqrt(pow((next_sys->X() - fleet->X()), 2) + pow((next_sys->Y() - fleet->Y()), 2));
        }
    }

    if (!system_two) {
        fleet = FleetFromObject(obj2);
        if (!fleet)
            return -1;
        if (TemporaryPtr<const System> next_sys = GetSystem(fleet->NextSystemID())) {
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
    GraphImpl::EmpireViewSystemGraphMap::const_iterator graph_it =
        m_graph_impl->empire_system_graph_views.find(empire_id);
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

bool Pathfinder::SystemsConnected(int system1_id, int system2_id, int empire_id) const {
    return pimpl->SystemsConnected(system1_id, system2_id, empire_id);
}

bool Pathfinder::PathfinderImpl::SystemsConnected(int system1_id, int system2_id, int empire_id) const {
    //DebugLogger() << "SystemsConnected(" << system1_id << ", " << system2_id << ", " << empire_id << ")";
    std::pair<std::list<int>, int> path = LeastJumpsPath(system1_id, system2_id, empire_id);
    //DebugLogger() << "SystemsConnected returned path of size: " << path.first.size();
    bool retval = !path.first.empty();
    //DebugLogger() << "SystemsConnected retval: " << retval;
    return retval;
}

bool Pathfinder::SystemHasVisibleStarlanes(int system_id, int empire_id) const {
    return pimpl->SystemHasVisibleStarlanes(system_id, empire_id);
}

bool Pathfinder::PathfinderImpl::SystemHasVisibleStarlanes(int system_id, int empire_id) const {
    if (TemporaryPtr<const System> system = GetEmpireKnownSystem(system_id, empire_id))
        if (!system->StarlanesWormholes().empty())
            return true;
    return false;
}

std::multimap<double, int> Pathfinder::ImmediateNeighbors(int system_id, int empire_id/* = ALL_EMPIRES*/) const {
    return pimpl->ImmediateNeighbors(system_id, empire_id);
}

std::multimap<double, int> Pathfinder::PathfinderImpl::ImmediateNeighbors(int system_id, int empire_id/* = ALL_EMPIRES*/) const {
    if (empire_id == ALL_EMPIRES) {
        return ImmediateNeighborsImpl(m_graph_impl->system_graph, system_id, m_system_id_to_graph_index);
    } else {
        GraphImpl::EmpireViewSystemGraphMap::const_iterator graph_it = m_graph_impl->empire_system_graph_views.find(empire_id);
        if (graph_it != m_graph_impl->empire_system_graph_views.end())
            return ImmediateNeighborsImpl(*graph_it->second, system_id, m_system_id_to_graph_index);
    }
    return std::multimap<double, int>();
}

int Pathfinder::NearestSystemTo(double x, double y) const {
    return pimpl->NearestSystemTo(x, y);
}

int Pathfinder::PathfinderImpl::NearestSystemTo(double x, double y) const {
    double min_dist2 = DBL_MAX;
    int min_dist2_sys_id = INVALID_OBJECT_ID;

    std::vector<TemporaryPtr<System> > systems = Objects().FindObjects<System>();

    for (std::vector<TemporaryPtr< System> >::const_iterator sys_it = systems.begin();
         sys_it != systems.end(); ++sys_it)
    {
        double xs = (*sys_it)->X();
        double ys = (*sys_it)->Y();
        double dist2 = (xs-x)*(xs-x) + (ys-y)*(ys-y);
        if (dist2 == 0.0) {
            return (*sys_it)->ID();
        } else if (dist2 < min_dist2) {
            min_dist2 = dist2;
            min_dist2_sys_id = (*sys_it)->ID();
        }
    }
    return min_dist2_sys_id;
}


void Pathfinder::InitializeSystemGraph(const std::vector<int> system_ids, int for_empire_id) {
    return pimpl->InitializeSystemGraph(system_ids, for_empire_id);
}

void Pathfinder::PathfinderImpl::InitializeSystemGraph(const std::vector<int> system_ids, int for_empire_id) {
    typedef boost::graph_traits<GraphImpl::SystemGraph>::edge_descriptor EdgeDescriptor;
    boost::shared_ptr<GraphImpl> new_graph_impl(new GraphImpl());
    // std::vector<int> system_ids = ::EmpireKnownObjects(for_empire_id).FindObjectIDs<System>();
    // NOTE: this initialization of graph_changed prevents testing for edges between nonexistant vertices
    bool graph_changed = system_ids.size() != boost::num_vertices(m_graph_impl->system_graph);
    //DebugLogger() << "InitializeSystemGraph(" << for_empire_id << ") system_ids: (" << system_ids.size() << ")";
    //for (std::vector<int>::const_iterator it = system_ids.begin(); it != system_ids.end(); ++it)
    //    DebugLogger() << " ... " << *it;

    GraphImpl::SystemIDPropertyMap sys_id_property_map =
        boost::get(vertex_system_id_t(), new_graph_impl->system_graph);

    GraphImpl::EdgeWeightPropertyMap edge_weight_map =
        boost::get(boost::edge_weight, new_graph_impl->system_graph);

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
        TemporaryPtr<const System> system1 = GetEmpireKnownSystem(system1_id, for_empire_id);
        //TemporaryPtr<const System> & system1 = systems[system1_index];

        // add edges and edge weights
        for (std::map<int, bool>::const_iterator it = system1->StarlanesWormholes().begin();
             it != system1->StarlanesWormholes().end(); ++it)
        {
            // get id in universe of system at other end of lane
            const int lane_dest_id = it->first;
            // skip null lanes and only add edges in one direction, to avoid
            // duplicating edges ( since this is an undirected graph, A->B
            // duplicates B->A )
            if (lane_dest_id >= system1_id)
                continue;

            // get new_graph_impl->system_graph index for this system
            boost::unordered_map<int, size_t>::iterator reverse_lookup_map_it = m_system_id_to_graph_index.find(lane_dest_id);
            if (reverse_lookup_map_it == m_system_id_to_graph_index.end())
                continue;   // couldn't find destination system id in vertex lookup map; don't add to graph
            size_t lane_dest_graph_index = reverse_lookup_map_it->second;

            std::pair<EdgeDescriptor, bool> add_edge_result =
                boost::add_edge(system1_index, lane_dest_graph_index, new_graph_impl->system_graph);

            if (add_edge_result.second) {   // if this is a non-duplicate starlane or wormhole
                if (it->second) {               // if this is a wormhole
                    edge_weight_map[add_edge_result.first] = WORMHOLE_TRAVEL_DISTANCE;
                } else {                        // if this is a starlane
                    edge_weight_map[add_edge_result.first] = LinearDistance(system1_id, lane_dest_id);
                }
                graph_changed = graph_changed || !boost::edge(system1_index, lane_dest_graph_index, m_graph_impl->system_graph).second;
            }
        }
    }

    // if all previous edges still exist in the new graph, and the number of vertices and edges hasn't changed,
    // then no vertices or edges can have been added either, so it is still the same graph
    graph_changed = graph_changed || boost::num_edges(new_graph_impl->system_graph) != boost::num_edges(m_graph_impl->system_graph);

    if (graph_changed) {
        new_graph_impl.swap(m_graph_impl);
        // clear jumps distance cache
        // NOTE: re-filling the cache is O(#vertices * (#vertices + #edges)) in the worst case!
        m_system_jumps.resize(system_ids.size());
    }
    UpdateEmpireVisibilityFilteredSystemGraphs(for_empire_id);
}

void Pathfinder::UpdateEmpireVisibilityFilteredSystemGraphs(int for_empire_id) {
    return pimpl->UpdateEmpireVisibilityFilteredSystemGraphs(for_empire_id);
}

void Pathfinder::PathfinderImpl::UpdateEmpireVisibilityFilteredSystemGraphs(int for_empire_id) {
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
