#include "Pathfinder.h"

#include <algorithm>
#include <limits>
#include <shared_mutex>
#include <stdexcept>
#include <boost/circular_buffer.hpp>
#include <boost/container/container_fwd.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/variant/variant.hpp>
#include "Field.h"
#include "Fleet.h"
#include "Ship.h"
#include "System.h"
#include "UniverseObject.h"
#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../util/Logger.h"
#include "../util/ScopedTimer.h"


namespace {
    /** distance_matrix_storage implements the storage and the mutexes
        for distance in number of hops from system to system.

        For N systems there are N rows of N integer types T.
        Each row has a mutex and the whole table has a mutex.

        The table is assumed symmetric.  If present row i element j will
        equal row j element i.
     */
    template <typename T>
    struct distance_matrix_storage {
        typedef T value_type;  ///< An integral type for number of hops.
        typedef std::vector<T>& row_ref;  ///< A reference to row type

        distance_matrix_storage() = default;
        distance_matrix_storage(const distance_matrix_storage<T>& src)
        { resize(src.m_data.size()); };
        //TODO C++11 Move Constructor.

        /**Number of rows and columns. (N)*/
        auto size() const noexcept
        { return m_data.size(); }

        /**Resize and clear all mutexes.  Assumes that table is locked.*/
        void resize(std::size_t a_size) {
            m_data.clear();
            m_data.resize(a_size);
            m_row_mutexes.resize(a_size);
            for (auto &row_mutex : m_row_mutexes)
                row_mutex = std::make_shared<std::shared_mutex>();
        }

        /**N x N table of hop distances in row column form.*/
        std::vector<std::vector<T>> m_data;
        /**Per row mutexes.*/
        std::vector<std::shared_ptr<std::shared_mutex>> m_row_mutexes;
        /**Table mutex*/
        std::shared_mutex m_mutex;
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
    template <typename Storage, typename T = typename Storage::value_type, typename Row = typename Storage::row_ref>
    class distance_matrix_cache {
    public:
        distance_matrix_cache(Storage& the_storage) : m_storage(the_storage) {}
        /**Read lock the table and return the size N.*/
        std::size_t size() {
            std::shared_lock<std::shared_mutex> guard(m_storage.m_mutex);
            return m_storage.size();
        }
        /**Write lock the table and resize to N = \p a_size.*/
        void resize(std::size_t a_size) {
            std::unique_lock<std::shared_mutex> guard(m_storage.m_mutex);
            m_storage.resize(a_size);
        }

        // Note: The signatures for the cache miss handler and the cache hit handler have a void
        // return type, because this code can not anticipate all expected return types.  In order
        // to return a result function the calling code will need to bind the return parameter to
        // the function so that the function still has the required signature.

        /// Cache miss handler
        typedef std::function<void (std::size_t&, Row)> cache_miss_handler;

        /// A function to examine an entire row cache hit
        typedef std::function<void (std::size_t &/*ii*/, const Row /*row*/)> cache_hit_handler;

        /** Retrieve a single element at (\p ii, \p jj).
          * On cache miss call the \p fill_row which must fill the row
          * at \p ii with NN items.
          * Throws if either index is out of range or if \p fill_row
          * does not fill the row  on a cache miss.
          */
        T get_T(std::size_t ii, std::size_t jj, cache_miss_handler fill_row) const {
            std::shared_lock<std::shared_mutex> guard(m_storage.m_mutex);

            auto NN = m_storage.size();
            if ((ii >= NN) || (jj >= NN)) {
                ErrorLogger() << "distance_matrix_cache::get_T passed invalid node indices: "
                              << ii << "," << jj << " matrix size: " << NN;
                throw std::out_of_range("row and/or column index is invalid.");
            }
            {
                std::shared_lock<std::shared_mutex> row_guard(*m_storage.m_row_mutexes[ii]);
                const Row &row_data = m_storage.m_data[ii];

                if (NN == row_data.size())
                    return row_data[jj];
            }
            {
                std::shared_lock<std::shared_mutex> column_guard(*m_storage.m_row_mutexes[jj]);
                const Row &column_data = m_storage.m_data[jj];

                if (NN == column_data.size())
                    return column_data[ii];
            }
            {
                std::unique_lock<std::shared_mutex> row_guard(*m_storage.m_row_mutexes[ii]);
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
        void examine_row(std::size_t ii, cache_miss_handler fill_row, cache_hit_handler use_row) const {
            std::shared_lock<std::shared_mutex> guard(m_storage.m_mutex);

            std::size_t NN = m_storage.size();
            if (ii >= NN) {
                ErrorLogger() << "distance_matrix_cache::get_row passed invalid index: "
                              << ii << " matrix size: " << NN;
                throw std::out_of_range("row index is invalid.");
            }

            {
                // check for existing cache data for requested row
                std::shared_lock<std::shared_mutex> row_guard(*m_storage.m_row_mutexes[ii]);
                const Row& row_data = m_storage.m_data[ii];

                if (NN == row_data.size()) {
                    use_row(ii, row_data);
                    return;
                }
            }

            {
                std::unique_lock<std::shared_mutex> row_guard(*m_storage.m_row_mutexes[ii]);
                Row& row_data = m_storage.m_data[ii];

                if (NN != row_data.size()) { // re-check for data after getting unique lock
                    // fill row with so-far missing data...
                    fill_row(ii, row_data);

                    if (row_data.size() != NN) {
                        std::stringstream ss;
                        ss << "Cache miss handler only filled cache row with "
                           << row_data.size() << " items when " << NN
                           << " items where expected ";
                        ErrorLogger() << ss.str();
                        throw std::range_error(ss.str());
                    }
                }

                use_row(ii, row_data);
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
        template <typename Vertex, typename Graph>
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
    template <typename Graph, typename Edge, typename Vertex>
    class BFSVisitorImpl
    {
    public:
        class FoundDestination {};
        class ReachedDepthLimit {};

    private:
        Vertex m_marker;
        Vertex m_stop;
        Vertex m_source;
        Vertex* m_predecessors;
        int m_levels_remaining;
        bool m_level_complete = false;

    public:
        BFSVisitorImpl(const Vertex& start, const Vertex& stop, Vertex predecessors[], int max_depth)
            : m_marker(start),
              m_stop(stop),
              m_source(start),
              m_predecessors(predecessors),
              m_levels_remaining(max_depth)
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
    std::pair<std::vector<int>, double> ShortestPathImpl(
        const auto& graph, int system1_id, int system2_id, const auto& id_to_graph_index)
    {
        // convert system IDs to graph indices.  try/catch for invalid input system ids.
        std::size_t system1_index, system2_index;
        try {
            system1_index = id_to_graph_index.at(system1_id);
            system2_index = id_to_graph_index.at(system2_id);
        } catch (...) {
            return {{}, -1.0}; // specified system(s) do no exist, so no path between them exists
        }

        // early exit if systems are the same
        if (system1_id == system2_id)
            return {{system2_id}, 0.0}; // no jumps needed -> 0 distance

        /* initializing all vertices' predecessors to themselves prevents endless loops
           when back traversing the tree in the case where one of the end systems is
           system 0, because systems that are not connected to the root system (system2)
           are not visited by the search, and so their predecessors are left unchanged.
           Default initialization of the vector may be 0 or undefined which could lead
           to out of bounds errors, or endless loops if a system's default predecessor
           is 0 (debug mode), and 0's predecessor is that system */
        const std::size_t num_verts = boost::num_vertices(graph);

        std::vector<int> predecessors;
        predecessors.reserve(num_verts);
        for (std::size_t i = 0; i < num_verts; ++i)
            predecessors.push_back(static_cast<int>(i));

        std::vector<double> distances(num_verts, -1.0);


        // do the actual path finding using verbose boost magic...
        const auto& index_map = boost::get(boost::vertex_index, graph);
        const auto& edge_weight_map = boost::get(boost::edge_weight, graph);
        try {
            boost::dijkstra_shortest_paths(
                graph, system1_index, predecessors.data(), distances.data(),
                edge_weight_map, index_map, std::less<double>(), std::plus<double>(),
                std::numeric_limits<int>::max(), 0,
                boost::make_dijkstra_visitor(PathFindingShortCircuitingVisitor(system2_index)));
        } catch (const PathFindingShortCircuitingVisitor::FoundDestination&) {
            // catching this just means that the destination was found, and so
            // the algorithm was exited early, via exception
        }

        boost::circular_buffer<int> buf(id_to_graph_index.size()); // expect that path can at most visit all systems once

        const auto& sys_id_property_map = boost::get(vertex_system_id_t(), graph);
        int current_system = system2_index;
        while (predecessors[current_system] != current_system) {
            buf.push_front(sys_id_property_map[current_system]);
            current_system = predecessors[current_system];
        }

        if (buf.empty())
            return {{}, -1.0}; // there is no path between the specified nodes

        // add start system to path, as it wasn't added by traversing predecessors array
        if (buf.full()) {
            ErrorLogger() << "ShortestPathImpl buffer full before expected!";
            buf.set_capacity(buf.capacity() + 1);
        }
        buf.push_front(sys_id_property_map[system1_index]);

        return {{buf.begin(), buf.end()}, distances[system2_index]};
    }

    /** Returns the path between vertices \a system1_id and \a system2_id of
      * \a graph that takes the fewest number of jumps (edge traversals), and
      * the number of jumps this path takes.  If system1_id is the same vertex
      * as system2_id, the path has just that system in it, and the path lenth
      * is 0.  If there is no path between the two vertices, then the list is
      * empty and the path length is -1 */
    template <typename Graph>
    std::pair<std::vector<int>, int> LeastJumpsPathImpl(
        const Graph& graph, int system1_id, int system2_id,
        const boost::container::flat_map<int, std::size_t>& id_to_graph_index,
        int max_jumps = INT_MAX)
    {
        // early exit if systems are the same
        if (system1_id == system2_id)
            return {{system2_id}, 0}; // no jumps needed

        // convert system IDs to graph indices.  try/catch for invalid input system ids.
        std::size_t system1_index, system2_index;
        try {
            system1_index = id_to_graph_index.at(system1_id);
            system2_index = id_to_graph_index.at(system2_id);
        } catch (...) {
            return {{}, -1}; // specified system(s) do no exist, so no path between them exists
        }

        /* initializing all vertices' predecessors to themselves prevents endless loops
           when back traversing the tree in the case where one of the end systems is
           system 0, because systems that are not connected to the root system (system2)
           are not visited by the search, and so their predecessors are left unchanged.
           Default initialization of the vector may be 0 or undefined which could lead
           to out of bounds errors, or endless loops if a system's default predecessor
           is 0, (debug mode) and 0's predecessor is that system */
        std::vector<int> predecessors(boost::num_vertices(graph));
        for (unsigned int i = 0; i < boost::num_vertices(graph); ++i)
            predecessors[i] = i;


        // do the actual path finding using verbose boost magic...
        using BFSVisitor = BFSVisitorImpl<Graph, typename boost::graph_traits<Graph>::edge_descriptor, int>;
        try {
            boost::queue<int> buf;
            std::vector<int> colors(boost::num_vertices(graph));

            BFSVisitor bfsVisitor(system1_index, system2_index, predecessors.data(), max_jumps);
            boost::breadth_first_search(graph, system1_index, buf, bfsVisitor, colors.data());
        } catch (const typename BFSVisitor::ReachedDepthLimit&) {
            // catching this means the algorithm explored the neighborhood until max_jumps and didn't find anything
            return {{}, -1};
        } catch (const typename BFSVisitor::FoundDestination&) {
            // catching this just means that the destination was found, and so the
            // algorithm was exited early, via exception
        }

        boost::circular_buffer<int> buf(id_to_graph_index.size()); // expect that path can at most visit all systems once

        const auto& sys_id_property_map = boost::get(vertex_system_id_t(), graph);
        int current_system = system2_index;
        while (predecessors[current_system] != current_system) {
            buf.push_front(sys_id_property_map[current_system]);
            current_system = predecessors[current_system];
        }

        if (buf.empty())
            return {{}, -1}; // there is no path between the specified nodes

        // add start system to path, as it wasn't added by traversing predecessors array
        if (buf.full()) {
            ErrorLogger() << "LeastJumpsPathImpl buffer full before expected!";
            buf.set_capacity(buf.capacity() + 1);
        }
        buf.push_front(system1_id);

        return {{buf.begin(), buf.end()}, static_cast<int>(buf.size() - 1)};
    }

    template <typename Graph>
    auto ImmediateNeighborsImpl(
        const Graph& graph, int system_id, const boost::container::flat_map<int, std::size_t>& id_to_graph_index)
    {
        using edge_weights_t = decltype(boost::get(boost::edge_weight, graph));
        using vertex_sys_id_t = decltype(boost::get(vertex_system_id_t(), graph));
        static_assert(!std::is_reference_v<edge_weights_t>);
        static_assert(!std::is_reference_v<vertex_sys_id_t>);

        const auto edge_weight_map = boost::get(boost::edge_weight, graph);
        const auto sys_id_property_map = boost::get(vertex_system_id_t(), graph);
        const auto [first_edge, last_edge] = boost::out_edges(id_to_graph_index.at(system_id), graph);

        using val_t = std::pair<std::decay_t<decltype(edge_weight_map[*first_edge])>,
                                std::decay_t<decltype(sys_id_property_map[boost::target(*first_edge, graph)])>>;
        std::vector<val_t> retval; // probably vector<pair<double, int>>
        retval.reserve(std::distance(first_edge, last_edge));

        std::transform(first_edge, last_edge, std::back_inserter(retval), [&](const auto& e) -> val_t
                       { return {edge_weight_map[e], sys_id_property_map[boost::target(e, graph)]}; });
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
                                boost::property<boost::vertex_index_t, int>> vertex_property_t; ///< a system graph property map type
        typedef boost::property<boost::edge_weight_t, double>                edge_property_t;   ///< a system graph property map type

        // declare main graph types, including properties declared above.
        // could add boost::disallow_parallel_edge_tag GraphProperty but it doesn't
        // work for vecS vector-based lists and parallel edges can be avoided while
        // creating the graph by filtering the edges to be added
        typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                                      vertex_property_t, edge_property_t> SystemGraph;

        struct EdgeVisibilityFilter {
            EdgeVisibilityFilter() = default;

            EdgeVisibilityFilter(const SystemGraph* graph, const ObjectMap& objects) :
                m_graph(graph)
            {
                if (!m_graph)
                    throw std::invalid_argument("EdgeVisibilityFilter passed null system graph");

                // collect all edges
                decltype(edges)::sequence_type edges_vec;
                edges_vec.reserve(objects.size<System>() * 10); // guesstimate
                for (const auto& sys : objects.allRaw<System>()) {
                    const auto sys_id{sys->ID()};
                    for (auto lane_id : sys->Starlanes())
                        edges_vec.emplace_back(std::min(sys_id, lane_id), std::max(sys_id, lane_id));
                }
                // sort and ensure uniqueness of entries before moving into flat_set
                std::sort(edges_vec.begin(), edges_vec.end());
                edges_vec.erase(std::unique(edges_vec.begin(), edges_vec.end()), edges_vec.end());
                edges.adopt_sequence(boost::container::ordered_unique_range, std::move(edges_vec));
            }

            template <typename EdgeDescriptor>
            bool operator()(const EdgeDescriptor& edge) const
            {
                if (!m_graph) {
                    ErrorLogger() << "EdgeVisibilityFilter has null graph?";
                    return false;
                }
                const auto& graph = *m_graph;
                // get system ids from graph indices
                const auto& sys_id_property_map = boost::get(vertex_system_id_t(), graph); // for reverse-lookup System universe ID from graph index
                int sys_graph_index_1 = boost::source(edge, graph);
                int sys_id_1 = sys_id_property_map[sys_graph_index_1];
                int sys_graph_index_2 = boost::target(edge, graph);
                int sys_id_2 = sys_id_property_map[sys_graph_index_2];

                // look up lane between systems
                return edges.contains(std::pair{std::min(sys_id_1, sys_id_2), std::max(sys_id_1, sys_id_2)});
            }

        private:
            const SystemGraph* m_graph = nullptr;
            boost::container::flat_set<std::pair<int, int>> edges;
        };
        typedef boost::filtered_graph<SystemGraph, EdgeVisibilityFilter> EmpireViewSystemGraph;
        typedef std::map<int, EmpireViewSystemGraph> EmpireViewSystemGraphMap;


        struct SystemPredicateFilter {
            SystemPredicateFilter() = default;

            SystemPredicateFilter(const SystemGraph* graph, const ObjectMap* objects,
                                  const Pathfinder::SystemExclusionPredicateType& pred) :
                m_objects(objects),
                m_graph(graph),
                m_pred(pred)
            {
                if (!graph)
                    ErrorLogger() << "ExcludeObjectFilter passed null graph pointer";
            }

            template <typename EdgeDescriptor>
            bool operator()(const EdgeDescriptor& edge) const {
                if (!m_graph || !m_objects)
                    return true;

                // get system ids from graph indices

                // for reverse-lookup System universe ID from graph index
                const auto& sys_id_property_map = boost::get(vertex_system_id_t(), *m_graph);
                int sys_graph_index_1 = boost::source(edge, *m_graph);
                int sys_id_1 = sys_id_property_map[sys_graph_index_1];
                int sys_graph_index_2 = boost::target(edge, *m_graph);
                int sys_id_2 = sys_id_property_map[sys_graph_index_2];

                // look up objects in system
                auto system1 = m_objects->get<System>(sys_id_1);
                if (!system1) {
                    ErrorLogger() << "Invalid source system " << sys_id_1;
                    return true;
                }
                auto system2 = m_objects->get<System>(sys_id_2);
                if (!system2) {
                    ErrorLogger() << "Invalid target system " << sys_id_2;
                    return true;
                }

                if (!system1->HasStarlaneTo(system2->ID())) {
                    DebugLogger() << "No starlane from " << system1->ID() << " to " << system2->ID();
                    return false;
                }

                // Discard edge if it finds a contained object or matches either system for visitor
                for (const auto& object : m_objects->find(m_pred)) {
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
            const ObjectMap*                         m_objects = nullptr;
            const SystemGraph*                       m_graph = nullptr;
            Pathfinder::SystemExclusionPredicateType m_pred;
        };

        using SystemPredicateGraph = boost::filtered_graph<SystemGraph, SystemPredicateFilter>;

        // declare property map types for properties declared above
        using ConstSystemIDPropertyMap = boost::property_map<SystemGraph, vertex_system_id_t>::const_type;
        using SystemIDPropertyMap = boost::property_map<SystemGraph, vertex_system_id_t>::type;
        using ConstIndexPropertyMap = boost::property_map<SystemGraph, boost::vertex_index_t>::const_type;
        using IndexPropertyMap = boost::property_map<SystemGraph, boost::vertex_index_t>::type;
        using ConstEdgeWeightPropertyMap = boost::property_map<SystemGraph, boost::edge_weight_t>::const_type;
        using EdgeWeightPropertyMap = boost::property_map<SystemGraph, boost::edge_weight_t>::type;

        void Reset() {
            empire_system_graph_views.clear();
            if (system_graph) [[likely]]
                system_graph->clear();
            else
                system_graph = std::make_unique<SystemGraph>();
        }

        std::unique_ptr<SystemGraph> system_graph = std::make_unique<SystemGraph>();///< a graph in which the systems are vertices and the starlanes are edges
        EmpireViewSystemGraphMap     empire_system_graph_views;                     ///< a map of empire IDs to the views of the system graph by those empires
    };
}

/////////////////////////////////////////////
// class PathfinderImpl
/////////////////////////////////////////////
class Pathfinder::PathfinderImpl {
public:
    double LinearDistance(int object1_id, int object2_id, const ObjectMap& objects) const;
    int16_t JumpDistanceBetweenSystems(int system1_id, int system2_id) const;
    int JumpDistanceBetweenObjects(int object1_id, int object2_id, const ObjectMap& objects) const;

    std::pair<std::vector<int>, double> ShortestPath(
        int system1_id, int system2_id, const ObjectMap& objects, int empire_id = ALL_EMPIRES) const;
    std::pair<std::vector<int>, double> ShortestPath(
        int system1_id, int system2_id, const ObjectMap& objects,
        const Pathfinder::SystemExclusionPredicateType& sys_pred) const;

    double ShortestPathDistance(int object1_id, int object2_id, const ObjectMap& objects) const;

    std::pair<std::vector<int>, int> LeastJumpsPath(
        int system1_id, int system2_id, int empire_id = ALL_EMPIRES, int max_jumps = INT_MAX) const;

    bool SystemsConnected(int system1_id, int system2_id, int empire_id = ALL_EMPIRES) const;
    bool SystemHasVisibleStarlanes(int system_id, const ObjectMap& objects) const;
    std::vector<std::pair<double, int>> ImmediateNeighbors(int system_id, int empire_id = ALL_EMPIRES) const;

    std::vector<int> WithinJumps(std::size_t jumps, int candidate) const;
    std::vector<int> WithinJumps(std::size_t jumps, std::vector<int> candidates) const;
    void WithinJumpsCacheHit(
        std::vector<int>& result, std::size_t jump_limit,
        std::size_t ii, const std::vector<int16_t>& row) const;

    std::pair<Condition::ObjectSet, Condition::ObjectSet>
    WithinJumpsOfOthers(
        int jumps, const ObjectMap& objects,
        const Condition::ObjectSet& candidates,
        const Condition::ObjectSet& stationary) const;

    /** Return true if \p system_id is within \p jumps of any of \p others */
    bool WithinJumpsOfOthers(
        int jumps, int system_id, const ObjectMap& objects,
        const Condition::ObjectSet& others) const;

    /** If any of \p others are within \p jumps of \p ii return true in \p answer.

        The return value must be in a parameter so that after being bound to \p answer and \p jumps
        the function signature is that required by the cache_hit_handler type.
     */
    void WithinJumpsOfOthersCacheHit(
        bool& answer, int jumps, const ObjectMap& objects,
        const Condition::ObjectSet& others,
        std::size_t ii, distance_matrix_storage<int16_t>::row_ref row) const;

    int NearestSystemTo(double x, double y, const ObjectMap& objects) const;

    void InitializeSystemGraph(const ObjectMap& objects, const EmpireManager& empires);

    void UpdateCommonFilteredSystemGraphs(const EmpireManager& empires, const ObjectMap& objects);
    void UpdateEmpireVisibilityFilteredSystemGraphs(const EmpireManager& empires,
                                                    const Universe::EmpireObjectMap& empire_object_maps);

    /** When a cache miss occurs fill \p row with the distances
        from index \p ii to every other index.*/
    void HandleCacheMiss(std::size_t ii, distance_matrix_storage<int16_t>::row_ref row) const;


    mutable distance_matrix_storage<int16_t>     m_system_jumps; ///< indexed by system graph index (not system id), caches the smallest number of jumps to travel between all the systems
    GraphImpl                                    m_graph_impl{}; ///< a graph in which the systems are vertices and the starlanes are edges
    boost::container::flat_map<int, std::size_t> m_system_id_to_graph_index;
};

/////////////////////////////////////////////
// class Pathfinder
/////////////////////////////////////////////
Pathfinder::Pathfinder() :
    pimpl(std::make_unique<PathfinderImpl>())
{}

Pathfinder::~Pathfinder() = default;

Pathfinder& Pathfinder::operator=(Pathfinder&&) noexcept = default;

namespace {
    const Fleet* FleetFromObject(const UniverseObject* obj, const ObjectMap& objects) {
        if (obj->ObjectType() == UniverseObjectType::OBJ_FLEET)
            return static_cast<const Fleet*>(obj);
        if (obj->ObjectType() == UniverseObjectType::OBJ_SHIP)
            return objects.getRaw<const Fleet>(static_cast<const Ship*>(obj)->FleetID());
        return nullptr;
    }
}

/** HandleCacheMiss requires that \p row be locked by exterior context. */
void Pathfinder::PathfinderImpl::HandleCacheMiss(
    std::size_t ii, distance_matrix_storage<int16_t>::row_ref row) const
{
    using DistancePropertyMap = boost::iterator_property_map<std::vector<int16_t>::iterator,
                                                             boost::identity_property_map>;
    // FIXME: compute the i row and the j column, but only utilize the i row.

    TraceLogger() << "Cache MISS ii: " << ii;

    row.assign(m_system_jumps.size(), SHRT_MAX);
    row[ii] = 0;
    DistancePropertyMap distance_property_map(row.begin());
    boost::distance_recorder<DistancePropertyMap, boost::on_tree_edge> distance_recorder(distance_property_map);

    if (!m_graph_impl.system_graph) [[unlikely]] {
        ErrorLogger() << "No system graph in PathfinderImpl::HandleCacheMiss";
        return;
    }
    boost::breadth_first_search(*m_graph_impl.system_graph, ii,
                                boost::visitor(boost::make_bfs_visitor(distance_recorder)));
}

double Pathfinder::LinearDistance(int system1_id, int system2_id, const ObjectMap& objects) const
{ return pimpl->LinearDistance(system1_id, system2_id, objects); }

double Pathfinder::PathfinderImpl::LinearDistance(int system1_id, int system2_id,
                                                  const ObjectMap& objects) const
{
    const auto system1 = objects.getRaw<System>(system1_id);
    if (!system1) [[unlikely]] {
        ErrorLogger() << "Universe::LinearDistance passed invalid system id: " << system1_id;
        throw std::out_of_range("system1_id invalid");
    }
    const auto system2 = objects.getRaw<System>(system2_id);
    if (!system2) [[unlikely]] {
        ErrorLogger() << "Universe::LinearDistance passed invalid system id: " << system2_id;
        throw std::out_of_range("system2_id invalid");
    }
    const double x_dist = system2->X() - system1->X();
    const double y_dist = system2->Y() - system1->Y();
    return std::sqrt(x_dist*x_dist + y_dist*y_dist);
}

int16_t Pathfinder::JumpDistanceBetweenSystems(int system1_id, int system2_id) const
{ return pimpl->JumpDistanceBetweenSystems(system1_id, system2_id); }

int16_t Pathfinder::PathfinderImpl::JumpDistanceBetweenSystems(int system1_id, int system2_id) const {
    if (system1_id == system2_id)
        return 0;

    try {
        const distance_matrix_cache<distance_matrix_storage<int16_t>> cache(m_system_jumps);

        const std::size_t system1_index = m_system_id_to_graph_index.at(system1_id);
        const std::size_t system2_index = m_system_id_to_graph_index.at(system2_id);
        const std::size_t smaller_index = std::min(system1_index, system2_index);
        const std::size_t other_index   = std::max(system1_index, system2_index);

        using row_ref = distance_matrix_storage<short>::row_ref;

        // prefer filling the smaller row/column for increased cache locality
        const auto jumps = cache.get_T(smaller_index, other_index,
                                       [this](size_t ii, row_ref row) { HandleCacheMiss(ii, row); });
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
    GeneralizedLocationType GeneralizedLocation(const UniverseObject* obj,
                                                const ObjectMap& objects)
    {
        if (!obj)
            return nullptr;

        if (objects.getRaw<System>(obj->SystemID())) {
            TraceLogger() << "GeneralizedLocation of " << obj->Name() << " (" << obj->ID()
                          << ") is system id: " << obj->SystemID();
            return obj->SystemID();
        }

        if (auto fleet = FleetFromObject(obj, objects)) {
            auto fleet_sys_pair = std::pair(fleet->PreviousSystemID(), fleet->NextSystemID());
            if (fleet_sys_pair.first == INVALID_OBJECT_ID || fleet_sys_pair.second == INVALID_OBJECT_ID) {
                ErrorLogger() << "GeneralizedLocation of " << obj->Name() << " (" << obj->ID()
                              << ") is between " << fleet_sys_pair.first << " and " << fleet_sys_pair.second;
                return nullptr;
            }
            TraceLogger() << "GeneralizedLocation of " << obj->Name() << " (" << obj->ID()
                          << ") is between " << fleet_sys_pair.first << " and " << fleet_sys_pair.second;
            return fleet_sys_pair;
        }

        if (obj->ObjectType() == UniverseObjectType::OBJ_FIELD)
            return nullptr;

        // Don't generate an error message for temporary objects.
        if (obj->ID() == TEMPORARY_OBJECT_ID)
            return nullptr;

        ErrorLogger() << "GeneralizedLocationType unable to locate " << obj->Name() << "(" << obj->ID() << ")";
        return nullptr;
    }

    /** Return the location of the object with id \p object_id.*/
    GeneralizedLocationType GeneralizedLocation(int object_id, const ObjectMap& objects)
    { return GeneralizedLocation(objects.getRaw(object_id), objects); }
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
        int16_t sjumps = -1;
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
        int16_t sjumps1 = -1, sjumps2 = -1;
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
                            GeneralizedLocationType _sys2_ids) :
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
        int sjumps1 = -1, sjumps2 = -1;
        int prev_sys_id = prev_next.first, next_sys_id = prev_next.second;
        if (prev_sys_id != INVALID_OBJECT_ID) {
            JumpDistanceSys2Visitor visitor(pf, prev_sys_id);
            sjumps1 = boost::apply_visitor(visitor, sys2_ids);
        }
        if (next_sys_id!= INVALID_OBJECT_ID) {
            JumpDistanceSys2Visitor visitor(pf, next_sys_id);
            sjumps2 = boost::apply_visitor(visitor, sys2_ids);
        }

        int jumps1 = (sjumps1 == -1) ? INT_MAX : sjumps1;
        int jumps2 = (sjumps2 == -1) ? INT_MAX : sjumps2;

        return std::min(jumps1, jumps2);
    }
    const Pathfinder::PathfinderImpl& pf;
    const GeneralizedLocationType sys2_ids;
};

int Pathfinder::JumpDistanceBetweenObjects(int object1_id, int object2_id, const ObjectMap& objects) const
{ return pimpl->JumpDistanceBetweenObjects(object1_id, object2_id, objects); }

int Pathfinder::PathfinderImpl::JumpDistanceBetweenObjects(int object1_id, int object2_id,
                                                           const ObjectMap& objects) const
{
    GeneralizedLocationType obj1 = GeneralizedLocation(object1_id, objects);
    GeneralizedLocationType obj2 = GeneralizedLocation(object2_id, objects);
    JumpDistanceSys1Visitor visitor(*this, obj2);
    return boost::apply_visitor(visitor, obj1);
}

std::pair<std::vector<int>, double> Pathfinder::ShortestPath(
    int system1_id, int system2_id, int empire_id, const ObjectMap& objects) const
{ return pimpl->ShortestPath(system1_id, system2_id, objects, empire_id); }

std::pair<std::vector<int>, double> Pathfinder::PathfinderImpl::ShortestPath(
    int system1_id, int system2_id, const ObjectMap& objects, int empire_id) const
{
    const auto get_path = [system1_id, system2_id, this](const auto& graph)
    { return ShortestPathImpl(graph, system1_id, system2_id, m_system_id_to_graph_index); };

    try {
        if (empire_id == ALL_EMPIRES) {
            if (!m_graph_impl.system_graph) [[unlikely]] {
                ErrorLogger() << "No system graph in PathfinderImpl::ShortestPath";
                return {{}, -1.0};
            }
            // find path on full / complete system graph
            return get_path(*m_graph_impl.system_graph);
        }

        const auto& graph_views = m_graph_impl.empire_system_graph_views;

        // check if a filtered view to use for all empires is present
        auto graph_it = graph_views.find(ALL_EMPIRES);
        // if not, then check for a filtered view for specific empire
        if (graph_it == graph_views.end())
            graph_it = graph_views.find(empire_id);
        if (graph_it == graph_views.end()) {
            ErrorLogger() << "PathfinderImpl::ShortestPath passed unknown empire id: " << empire_id;
            throw std::out_of_range("PathfinderImpl::ShortestPath passed unknown empire id");
        }

        return get_path(graph_it->second);

    } catch (const std::out_of_range&) {
        ErrorLogger() << "PathfinderImpl::ShortestPath passed invalid system id(s): "
                      << system1_id << " & " << system2_id;
        return {{}, -1.0};
    }
}

std::pair<std::vector<int>, double> Pathfinder::ShortestPath(
    int system1_id, int system2_id, const SystemExclusionPredicateType& system_predicate,
    const ObjectMap& objects) const
{ return pimpl->ShortestPath(system1_id, system2_id, objects, system_predicate); }

std::pair<std::vector<int>, double> Pathfinder::PathfinderImpl::ShortestPath(
    int system1_id, int system2_id, const ObjectMap& objects,
    const Pathfinder::SystemExclusionPredicateType& sys_pred) const
{
    if (!m_graph_impl.system_graph) [[unlikely]] {
        ErrorLogger() << "No system graph in ShortestPath";
        throw std::runtime_error("No system graph in ShortestPath");
    }
    const GraphImpl::SystemPredicateGraph sys_pred_graph(
        *m_graph_impl.system_graph,
        GraphImpl::SystemPredicateFilter{m_graph_impl.system_graph.get(), &objects, sys_pred});
    
    try {
        return ShortestPathImpl(sys_pred_graph, system1_id, system2_id, m_system_id_to_graph_index);
    } catch (const std::out_of_range&) {
        ErrorLogger() << "ShortestPath: Invalid system id(s): " << system1_id << ", " << system2_id;
        throw;
    }
}

double Pathfinder::ShortestPathDistance(int object1_id, int object2_id, const ObjectMap& objects) const
{ return pimpl->ShortestPathDistance(object1_id, object2_id, objects); }

double Pathfinder::PathfinderImpl::ShortestPathDistance(int object1_id, int object2_id,
                                                        const ObjectMap& objects) const
{
    ScopedTimer timer("PathfinderImpl::ShortestPathDistance(" + std::to_string(object1_id) + ", " + std::to_string(object2_id) + ")",
                      true);

    // If one or both objects are (in) a fleet between systems, use the destination system
    // and add the distance from the fleet to the destination system, essentially calculating
    // the distance travelled until both could be in the same system.
    const auto obj1 = objects.getRaw(object1_id);
    if (!obj1)
        return -1.0;

    const auto obj2 = objects.getRaw(object2_id);
    if (!obj2)
        return -1.0;

    double dist{0.0};

    auto* system_one = obj1->ObjectType() == UniverseObjectType::OBJ_SYSTEM ?
        static_cast<const System*>(obj1) : objects.getRaw<System>(obj1->SystemID());
    if (!system_one) {
        auto fleet = FleetFromObject(obj1, objects);
        if (!fleet)
            return -1.0;
        if (auto next_sys = objects.getRaw<System>(fleet->NextSystemID())) {
            dist = std::sqrt(pow((next_sys->X() - fleet->X()), 2) + pow((next_sys->Y() - fleet->Y()), 2));
            system_one = next_sys;
        } else {
            ErrorLogger() << "ShortestPathDistance couldn't get fleet " << fleet->ID() << " next system " << fleet->NextSystemID();
            return -1.0;
        }
    }

    auto* system_two = obj2->ObjectType() == UniverseObjectType::OBJ_SYSTEM ?
        static_cast<const System*>(obj2) : objects.getRaw<System>(obj2->SystemID());
    if (!system_two) {
        auto fleet = FleetFromObject(obj2, objects);
        if (!fleet)
            return -1.0;
        if (auto next_sys = objects.getRaw<System>(fleet->NextSystemID())) {
            dist += std::sqrt(pow((next_sys->X() - fleet->X()), 2) + pow((next_sys->Y() - fleet->Y()), 2));
            system_two = next_sys;
        } else {
            ErrorLogger() << "ShortestPathDistance couldn't get fleet " << fleet->ID() << " next system " << fleet->NextSystemID();
            return -1.0;
        }
    }

    try {
        auto path_len_pair = ShortestPath(system_one->ID(), system_two->ID(), objects);
        return path_len_pair.second + dist;
    } catch (...) {
        ErrorLogger() << "ShortestPathDistance caught exception when calling ShortestPath";
        return -1.0;
    }
}

std::pair<std::vector<int>, int> Pathfinder::LeastJumpsPath(
    int system1_id, int system2_id, int empire_id, int max_jumps) const
{ return pimpl->LeastJumpsPath(system1_id, system2_id, empire_id, max_jumps); }

std::pair<std::vector<int>, int> Pathfinder::PathfinderImpl::LeastJumpsPath(
    int system1_id, int system2_id, int empire_id, int max_jumps) const
{
    if (empire_id == ALL_EMPIRES) {
        if (!m_graph_impl.system_graph) [[unlikely]] {
            ErrorLogger() << "No system graph in PathfinderImpl::LeastJumpsPath";
            throw std::runtime_error("No system graph in PathfinderImpl::LeastJumpsPath");
        }
        // find path on full / complete system graph
        try {
            return LeastJumpsPathImpl(*m_graph_impl.system_graph, system1_id, system2_id,
                                      m_system_id_to_graph_index, max_jumps);
        } catch (const std::out_of_range&) {
            ErrorLogger() << "PathfinderImpl::LeastJumpsPath passed invalid system id(s): "
                          << system1_id << " & " << system2_id;
            throw;
        }
    }

    // find path on single empire's view of system graph
    auto graph_it = m_graph_impl.empire_system_graph_views.find(empire_id);
    if (graph_it == m_graph_impl.empire_system_graph_views.end()) {
        ErrorLogger() << "PathfinderImpl::LeastJumpsPath passed unknown empire id: " << empire_id;
        throw std::out_of_range("PathfinderImpl::LeastJumpsPath passed unknown empire id");
    }
    try {
        return LeastJumpsPathImpl(graph_it->second, system1_id, system2_id, m_system_id_to_graph_index, max_jumps);
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

bool Pathfinder::SystemHasVisibleStarlanes(int system_id, const ObjectMap& objects) const
{ return pimpl->SystemHasVisibleStarlanes(system_id, objects); }

bool Pathfinder::PathfinderImpl::SystemHasVisibleStarlanes(int system_id, const ObjectMap& objects) const {
    if (auto system = objects.getRaw<System>(system_id))
        if (system->NumStarlanes() > 0)
            return true;
    return false;
}

std::vector<std::pair<double, int>> Pathfinder::ImmediateNeighbors(int system_id, int empire_id) const
{ return pimpl->ImmediateNeighbors(system_id, empire_id); }

std::vector<std::pair<double, int>> Pathfinder::PathfinderImpl::ImmediateNeighbors(
    int system_id, int empire_id) const
{
    if (empire_id == ALL_EMPIRES) {
        if (!m_graph_impl.system_graph) [[unlikely]] {
            ErrorLogger() << "No system graph in PathfinderImpl::ImmediateNeighbors";
            return {};
        }
        return ImmediateNeighborsImpl(*m_graph_impl.system_graph, system_id,
                                      m_system_id_to_graph_index);
    } else {
        auto graph_it = m_graph_impl.empire_system_graph_views.find(empire_id);
        if (graph_it != m_graph_impl.empire_system_graph_views.end())
            return ImmediateNeighborsImpl(graph_it->second, system_id, m_system_id_to_graph_index);
    }
    return {};
}

void Pathfinder::PathfinderImpl::WithinJumpsCacheHit(
    std::vector<int>& result, std::size_t jump_limit, std::size_t ii, const std::vector<int16_t>& row) const
{
    TraceLogger() << "Cache Hit ii: " << ii << "  jumps: " << jump_limit;
    // Scan the LUT of system ids and add any result from the row within
    // the neighborhood range to the results.
    for (auto& [sys_id, sys_idx] : m_system_id_to_graph_index) {
        auto hops = row[sys_idx];
        if (hops <= static_cast<decltype(hops)>(jump_limit))
            result.push_back(sys_id);
    }
}

std::vector<int> Pathfinder::WithinJumps(std::size_t jumps, std::vector<int> candidates) const
{ return pimpl->WithinJumps(jumps, std::move(candidates)); }

std::vector<int> Pathfinder::WithinJumps(std::size_t jumps, int candidate) const
{ return pimpl->WithinJumps(jumps, candidate); }

std::vector<int> Pathfinder::PathfinderImpl::WithinJumps(
    std::size_t jumps, std::vector<int> near) const
{
    if (near.empty())
        return near;
    if (near.size() == 1)
        return WithinJumps(jumps, near.front());

    // if jumps is 0, then just return the input systems (after filtering for
    // duplicates below), as they are always near themselves
    if (jumps > 0) {
        distance_matrix_cache<distance_matrix_storage<int16_t>> cache(m_system_jumps);

        std::vector<std::vector<int>> candidate_results(near.size());

        auto get_neighbours = [&cache, jumps, this](int candidate) {
            std::vector<int> row_result;

            auto index_it = m_system_id_to_graph_index.find(candidate);
            if (index_it == m_system_id_to_graph_index.end())
                return row_result;
            auto system_index = index_it->second;

            using row_ref = distance_matrix_storage<int16_t>::row_ref;
            cache.examine_row(
                system_index,
                [this](std::size_t ii, row_ref row) { HandleCacheMiss(ii, row); }, // boost::bind(&Pathfinder::PathfinderImpl::HandleCacheMiss, this, ph::_1, ph::_2)
                [this, jumps, &row_result](std::size_t ii, row_ref row) { WithinJumpsCacheHit(row_result, jumps, ii, row); }); // boost::bind(&Pathfinder::PathfinderImpl::WithinJumpsCacheHit, this, row_result, jumps, ph::_1, ph::_2));
            return row_result;
        };

        // get results for each candidate
        std::transform(near.begin(), near.end(), candidate_results.begin(), get_neighbours);

        // reserve needed space to combine
        auto results_sz = near.size();
        std::for_each(candidate_results.begin(), candidate_results.end(),
                      [&results_sz](const auto& res) { results_sz += res.size(); });
        near.reserve(results_sz);

        // combine into single list, probably with duplicate entries
        std::for_each(candidate_results.begin(), candidate_results.end(),
                      [&near](std::vector<int>& res) { std::copy(res.begin(), res.end(), std::back_inserter(near)); });
    }

    // ensure uniqueness of results
    std::sort(near.begin(), near.end());
    auto it = std::unique(near.begin(), near.end());
    near.resize(std::distance(near.begin(), it));

    return near;
}

std::vector<int> Pathfinder::PathfinderImpl::WithinJumps(std::size_t jumps, int candidate) const {
    auto index_it = m_system_id_to_graph_index.find(candidate);
    if (index_it == m_system_id_to_graph_index.end())
        return {};
    auto system_index = index_it->second;

    if (jumps == 0) // after check for graph index to avoid returning a candidate if it's not actually a system
        return {candidate};

    std::vector<int> row_result;

    distance_matrix_cache<distance_matrix_storage<int16_t>> cache(m_system_jumps);
    using row_ref = distance_matrix_storage<int16_t>::row_ref;
    cache.examine_row(
        system_index,
        [this](std::size_t ii, row_ref row) { HandleCacheMiss(ii, row); },
        [this, jumps, &row_result](std::size_t ii, row_ref row) { WithinJumpsCacheHit(row_result, jumps, ii, row); });

    // ensure uniqueness of results
    std::sort(row_result.begin(), row_result.end());
    auto it = std::unique(row_result.begin(), row_result.end());
    row_result.resize(std::distance(row_result.begin(), it));

    return row_result;
}

/** Examine a single universe object and determine if it is within jumps
    of any object in others.*/
struct WithinJumpsOfOthersObjectVisitor : public boost::static_visitor<bool> {
    WithinJumpsOfOthersObjectVisitor(const Pathfinder::PathfinderImpl& _pf,
                                     int _jumps,
                                     const ObjectMap& _objects,
                                     const Condition::ObjectSet& _others) :
        pf(_pf),
        jumps(_jumps),
        objects(_objects),
        others(_others)
    {}

    bool operator()(std::nullptr_t) const { return false; }
    bool operator()(int sys_id) const
    { return pf.WithinJumpsOfOthers(jumps, sys_id, objects, others); }
    bool operator()(std::pair<int, int> prev_next) const {
        return pf.WithinJumpsOfOthers(jumps, prev_next.first, objects, others)
            || pf.WithinJumpsOfOthers(jumps, prev_next.second, objects, others);
    }
    const Pathfinder::PathfinderImpl& pf;
    int jumps;
    const ObjectMap& objects;
    const Condition::ObjectSet& others;
};

/** Examine a single other in the cache to see if any of its locations
    are within jumps*/
struct WithinJumpsOfOthersOtherVisitor : public boost::static_visitor<bool> {
    WithinJumpsOfOthersOtherVisitor(const Pathfinder::PathfinderImpl& _pf,
                                    int _jumps,
                                    distance_matrix_storage<int16_t>::row_ref _row) :
        pf(_pf), jumps(_jumps), row(_row)
    {}

    bool single_result(int other_id) const {
        int index;
        try {
            index = pf.m_system_id_to_graph_index.at(other_id);
        } catch (const std::out_of_range&) {
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
    distance_matrix_storage<int16_t>::row_ref row;
};


void Pathfinder::PathfinderImpl::WithinJumpsOfOthersCacheHit(
    bool& answer, int jumps,
    const ObjectMap& objects,
    const Condition::ObjectSet& others,
    std::size_t ii, distance_matrix_storage<int16_t>::row_ref row) const
{
    // Check if any of the others are within jumps of candidate, by looping
    // through all of the others and applying the WithinJumpsOfOthersOtherVisitor.
    answer = false;
    for (const auto& other : others) {
        WithinJumpsOfOthersOtherVisitor check_if_location_is_within_jumps(*this, jumps, row);
        GeneralizedLocationType location = GeneralizedLocation(other, objects);
        if (boost::apply_visitor(check_if_location_is_within_jumps, location)) {
            answer = true;
            return;
        }
    }
}

std::pair<Condition::ObjectSet, Condition::ObjectSet>
Pathfinder::WithinJumpsOfOthers(
    int jumps, const ObjectMap& objects,
    const Condition::ObjectSet& candidates,
    const Condition::ObjectSet& stationary) const
{
    return pimpl->WithinJumpsOfOthers(jumps, objects, candidates, stationary);
}

std::pair<Condition::ObjectSet, Condition::ObjectSet>
Pathfinder::PathfinderImpl::WithinJumpsOfOthers(
    int jumps, const ObjectMap& objects,
    const Condition::ObjectSet& candidates,
    const Condition::ObjectSet& stationary) const
{
    // Examine each candidate and copy those within jumps of the
    // others into near and the rest into far.
    WithinJumpsOfOthersObjectVisitor visitor(*this, jumps, objects, stationary);

    std::pair<Condition::ObjectSet, Condition::ObjectSet> retval;
    auto& [near, far] = retval;
    near.reserve(candidates.size());
    far.reserve(candidates.size());

    for (const auto* candidate : candidates) {
        GeneralizedLocationType candidate_systems = GeneralizedLocation(candidate, objects);
        bool is_near = boost::apply_visitor(visitor, candidate_systems);

        if (is_near)
            near.push_back(candidate);
        else
            far.push_back(candidate);
    }

    return retval; // was: {near, far}; //, wherever you are...
}

bool Pathfinder::PathfinderImpl::WithinJumpsOfOthers(
    int jumps, int system_id,
    const ObjectMap& objects,
    const Condition::ObjectSet& others) const
{
    if (others.empty())
        return false;

    std::size_t system_index;
    try {
        system_index = m_system_id_to_graph_index.at(system_id);
    } catch (const std::out_of_range&) {
        ErrorLogger() << "Passed invalid system id: " << system_id;
        return false;
    }

    using row_ref = distance_matrix_storage<short>::row_ref;

    // Examine the cache to see if \p system_id is within \p jumps of \p others
    bool within_jumps(false);
    distance_matrix_cache<distance_matrix_storage<int16_t>> cache(m_system_jumps);
    cache.examine_row(system_index,
        [this](size_t ii, row_ref row) { HandleCacheMiss(ii, row); }, // boost::bind(&Pathfinder::PathfinderImpl::HandleCacheMiss, this, ph::_1, ph::_2),
        [this, &within_jumps, jumps, &objects, &others](size_t ii, row_ref row)
        { WithinJumpsOfOthersCacheHit(within_jumps, jumps, objects, others, ii, row); }); // boost::bind(&Pathfinder::PathfinderImpl::WithinJumpsOfOthersCacheHit, this,
                                                                                          //             std::ref(within_jumps), jumps, std::ref(objects), std::ref(others), ph::_1, ph::_2));

    return within_jumps;
}

int Pathfinder::NearestSystemTo(double x, double y, const ObjectMap& objects) const
{ return pimpl->NearestSystemTo(x, y, objects); }

int Pathfinder::PathfinderImpl::NearestSystemTo(double x, double y, const ObjectMap& objects) const {
    double min_dist2 = std::numeric_limits<double>::max();
    int min_dist2_sys_id = INVALID_OBJECT_ID;

    for (auto const& system : objects.allRaw<System>()) {
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


void Pathfinder::InitializeSystemGraph(const ObjectMap& objects, const EmpireManager& empires)
{ return pimpl->InitializeSystemGraph(objects, empires); }

void Pathfinder::PathfinderImpl::InitializeSystemGraph(const ObjectMap& objects, const EmpireManager& empires) {
    if (!m_graph_impl.system_graph) [[unlikely]]
        DebugLogger() << "GraphImpl had null system graph in InitializeSystemGraph...?";

    m_graph_impl.Reset();
    auto& system_graph = *m_graph_impl.system_graph;

    const auto& sys_id_property_map = boost::get(vertex_system_id_t(), system_graph);
    const auto& edge_weight_map = boost::get(boost::edge_weight, system_graph);

    // add vertices to graph for all systems
    std::vector<int> system_ids;
    system_ids.reserve(objects.allExisting<System>().size());
    range_copy(objects.allExisting<System>() | range_keys, std::back_inserter(system_ids));

    decltype(m_system_id_to_graph_index)::sequence_type system_id_to_graph_idx_vec;
    system_id_to_graph_idx_vec.reserve(system_ids.size());

    for (std::size_t system_index = 0; system_index < system_ids.size(); ++system_index) {
        // add a vertex to the graph for this system, and assign it the system's universe ID as a property
        boost::add_vertex(system_graph);
        int system_id = system_ids[system_index];
        sys_id_property_map[system_index] = system_id;
        // add record of index in system_graph of this system
        system_id_to_graph_idx_vec.emplace_back(system_id, system_index);
    }
    std::sort(system_id_to_graph_idx_vec.begin(), system_id_to_graph_idx_vec.end());
    m_system_id_to_graph_index.adopt_sequence(boost::container::ordered_unique_range,
                                              std::move(system_id_to_graph_idx_vec));

    // add edges for all starlanes
    for (std::size_t system1_index = 0; system1_index < system_ids.size(); ++system1_index) {
        int system1_id = system_ids[system1_index];
        auto system1 = objects.getRaw<System>(system1_id);

        // add edges and edge weights
        for (auto const lane_dest_id : system1->Starlanes()) {
            // skip null lanes and only add edges in one direction, to avoid
            // duplicating edges ( since this is an undirected graph, A->B
            // duplicates B->A )
            if (lane_dest_id >= system1_id)
                continue;

            // get system_graph index for this system
            auto reverse_lookup_map_it = m_system_id_to_graph_index.find(lane_dest_id);
            if (reverse_lookup_map_it == m_system_id_to_graph_index.end())
                continue;   // couldn't find destination system id in vertex lookup map; don't add to graph
            std::size_t lane_dest_graph_index = reverse_lookup_map_it->second;

            auto [edge_descriptor, add_success] =
                boost::add_edge(system1_index, lane_dest_graph_index, system_graph);
            //DebugLogger() << "Adding graph edge from " << system1_id << " to " << lane_dest_id;

            if (add_success) // if this is a non-duplicate starlane or wormhole
                edge_weight_map[std::move(edge_descriptor)] = LinearDistance(system1_id, lane_dest_id, objects);
        }
    }

    // clear jumps distance cache
    // NOTE: re-filling the cache is O(#vertices * (#vertices + #edges)) in the worst case!
    m_system_jumps.resize(system_ids.size());
}

void Pathfinder::UpdateCommonFilteredSystemGraphs(const EmpireManager& empires, const ObjectMap& objects)
{ pimpl->UpdateCommonFilteredSystemGraphs(empires, objects); }

void Pathfinder::UpdateEmpireVisibilityFilteredSystemGraphs(const EmpireManager& empires,
                                                            const std::map<int, ObjectMap>& empire_object_maps)
{ pimpl->UpdateEmpireVisibilityFilteredSystemGraphs(empires, empire_object_maps); }


void Pathfinder::PathfinderImpl::UpdateCommonFilteredSystemGraphs(
    const EmpireManager& empires, const ObjectMap& objects)
{
    // empires all use the same filtered graph, stored at index ALL_EMPIRES
    m_graph_impl.empire_system_graph_views.clear();

    if (!m_graph_impl.system_graph) [[unlikely]] {
        ErrorLogger() << "No system graph in UpdateCommonFilteredSystemGraphs";
        return;
    }

    const auto [eit, success] = m_graph_impl.empire_system_graph_views.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(ALL_EMPIRES),
        std::forward_as_tuple(*m_graph_impl.system_graph, 
                              GraphImpl::EdgeVisibilityFilter{m_graph_impl.system_graph.get(), objects}));
}

void Pathfinder::PathfinderImpl::UpdateEmpireVisibilityFilteredSystemGraphs(
    const EmpireManager& empires, const Universe::EmpireObjectMap& empire_object_maps)
{
    m_graph_impl.empire_system_graph_views.clear();
    const auto* system_graph = m_graph_impl.system_graph.get();
    if (!system_graph) [[unlikely]] {
        ErrorLogger() << "UpdateEmpireVisibilityFilteredSystemGraphs have null system graph...";
        return;
    }

    // each empire has its own filtered graph
    for (const auto& empire_id : empires.EmpireIDs()) {
        auto map_it = empire_object_maps.find(empire_id);
        if (map_it == empire_object_maps.end()) {
            ErrorLogger() << "UpdateEmpireVisibilityFilteredSystemGraphs can't find object map for empire with id " << empire_id;
            continue;
        }
        const auto& emp_objs = map_it->second;
        m_graph_impl.empire_system_graph_views.emplace(
            std::piecewise_construct,
            std::make_tuple(empire_id),
            std::make_tuple(*system_graph, GraphImpl::EdgeVisibilityFilter{system_graph, emp_objs}));
    }
}
