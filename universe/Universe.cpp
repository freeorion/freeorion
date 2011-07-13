#include "Universe.h"

#include "../util/AppInterface.h"
#include "../util/DataTable.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/Random.h"
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
#include "Parser.h"
#include "ParserUtil.h"


#include <boost/filesystem/fstream.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/timer.hpp>

#include <cmath>
#include <stdexcept>

namespace {
    const bool ENABLE_VISIBILITY_EMPIRE_MEMORY = true;      // toggles using memory with visibility, so that empires retain knowledge of objects viewed on previous turns
}

namespace {
    const double  OFFROAD_SLOWDOWN_FACTOR = 1000000000.0;   // the factor by which non-starlane travel is slower than starlane travel

    DataTableMap& UniverseDataTables() {
        static DataTableMap map;
        if (map.empty())
            LoadDataTables((GetResourceDir() / "universe_tables.txt").string(), map);
        return map;
    }

    void LoadSystemNames(std::list<std::string>& names) {
        boost::filesystem::ifstream ifs(GetResourceDir() / "starnames.txt");
        while (ifs) {
            std::string latest_name;
            std::getline(ifs, latest_name);
            if (!latest_name.empty())
                names.push_back(latest_name.substr(0, latest_name.find_last_not_of(" \t") + 1)); // strip off trailing whitespace
        }
    }

    void LoadEmpireNames(std::list<std::string>& names) {
        boost::filesystem::ifstream ifs(GetResourceDir() / "empire_names.txt");
        while (ifs) {
            std::string latest_name;
            std::getline(ifs, latest_name);
            if (!latest_name.empty())
                names.push_back(latest_name.substr(0, latest_name.find_last_not_of(" \t") + 1)); // strip off trailing whitespace
        }
    }
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

    ////////////////////////////////////////////////////////////////
    // templated implementations of Universe graph search methods //
    ////////////////////////////////////////////////////////////////
    struct vertex_system_id_t {typedef boost::vertex_property_tag kind;}; ///< a system graph property map type

    // returns the \a graph index for system with \a system_id
    template <class Graph>
    int SystemGraphIndex(const Graph& graph, int system_id)
    {
        typedef typename boost::property_map<Graph, vertex_system_id_t>::const_type ConstSystemIDPropertyMap;
        ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), graph);

        for (unsigned int i = 0; i < boost::num_vertices(graph); ++i) {
            const int loop_sys_id = sys_id_property_map[i];    // get system ID of this vertex
            if (loop_sys_id == system_id)
                return i;
        }

        throw std::out_of_range("SystemGraphIndex cannot be found due to invalid system ID " + boost::lexical_cast<std::string>(system_id));
        return -1;
    }

    /** Returns the path between vertices \a system1_id and \a system2_id of
      * \a graph that travels the shorest distance on starlanes, and the path
      * length.  If system1_id is the same vertex as system2_id, the path has
      * just that system in it, and the path lenth is 0.  If there is no path
      * between the two vertices, then the list is empty and the path length
      * is -1.0 */
    template <class Graph>
    std::pair<std::list<int>, double> ShortestPathImpl(const Graph& graph, int system1_id, int system2_id, double linear_distance)
    {
        typedef typename boost::property_map<Graph, vertex_system_id_t>::const_type     ConstSystemIDPropertyMap;
        typedef typename boost::property_map<Graph, boost::vertex_index_t>::const_type  ConstIndexPropertyMap;
        typedef typename boost::property_map<Graph, boost::edge_weight_t>::const_type   ConstEdgeWeightPropertyMap;

        std::pair<std::list<int>, double> retval;

        ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), graph);


        int system1_index = SystemGraphIndex(graph, system1_id);
        int system2_index = SystemGraphIndex(graph, system2_id);


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
    std::pair<std::list<int>, int> LeastJumpsPathImpl(const Graph& graph, int system1_id, int system2_id)
    {
        typedef typename boost::property_map<Graph, vertex_system_id_t>::const_type ConstSystemIDPropertyMap;

        ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), graph);
        std::pair<std::list<int>, int> retval;

        int system1_index = SystemGraphIndex(graph, system1_id);
        int system2_index = SystemGraphIndex(graph, system2_id);

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
        try {
            boost::queue<int> buf;
            std::vector<int> colors(boost::num_vertices(graph));
            boost::breadth_first_search(graph, system1_index, buf,
                                        boost::make_bfs_visitor(std::make_pair(PathFindingShortCircuitingVisitor(system2_index),
                                                                               boost::record_predecessors(&predecessors[0],
                                                                                                          boost::on_tree_edge()))),
                                        &colors[0]);
        } catch (const PathFindingShortCircuitingVisitor::FoundDestination&) {
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
    std::map<double, int> ImmediateNeighborsImpl(const Graph& graph, int system_id)
    {
        typedef typename Graph::out_edge_iterator OutEdgeIterator;
        typedef typename boost::property_map<Graph, vertex_system_id_t>::const_type ConstSystemIDPropertyMap;
        typedef typename boost::property_map<Graph, boost::edge_weight_t>::const_type ConstEdgeWeightPropertyMap;

        std::map<double, int> retval;
        ConstEdgeWeightPropertyMap edge_weight_map = boost::get(boost::edge_weight, graph);
        ConstSystemIDPropertyMap sys_id_property_map = boost::get(vertex_system_id_t(), graph);
        std::pair<OutEdgeIterator, OutEdgeIterator> edges = boost::out_edges(SystemGraphIndex(graph, system_id), graph);
        for (OutEdgeIterator it = edges.first; it != edges.second; ++it) {
            retval[edge_weight_map[*it]] = sys_id_property_map[boost::target(*it, graph)];
        }
        return retval;
    }
}
using namespace SystemPathing;  // to keep GCC 4.2 on OSX happy

const int ALL_EMPIRES = -1;


/////////////////////////////////////////////
// class ObjectMap
/////////////////////////////////////////////
ObjectMap::ObjectMap()
{}

ObjectMap::~ObjectMap()
{
    // Make sure to call ObjectMap::Clear() before destruction somewhere if
    // this ObjectMap contains any unique pointers to UniverseObject objects.
    // Otherwise, the pointed-to UniverseObjects will be leaked memory...
}

void ObjectMap::Copy(const ObjectMap& copied_map, int empire_id)
{
    if (&copied_map == this)
        return;

    // loop through objects in copied map, copying or cloning each depending
    // on whether there already is a corresponding object in this map
    for (ObjectMap::const_iterator it = copied_map.const_begin(); it != copied_map.const_end(); ++it) {
        int object_id = it->first;

        // can empire see object at all?  if not, skip copying object's info
        if (GetUniverse().GetObjectVisibilityByEmpire(object_id, empire_id) <= VIS_NO_VISIBILITY)
            continue;

        // copy object...
        const UniverseObject* copy_from_object = it->second;
        if (UniverseObject* copy_to_object = this->Object(object_id)) {
            copy_to_object->Copy(copy_from_object, empire_id);          // there already is a version of this object present in this ObjectMap, so just update it
        } else {
            UniverseObject* clone = copy_from_object->Clone(empire_id); // this object is not yet present in this ObjectMap, so add a new UniverseObject object for it
            this->Insert(object_id, clone);
        }
    }
}

void ObjectMap::CompleteCopyVisible(const ObjectMap& copied_map, int empire_id)
{
    if (&copied_map == this)
        return;

    // loop through objects in copied map, copying or cloning each depending
    // on whether there already is a corresponding object in this map
    for (ObjectMap::const_iterator it = copied_map.const_begin(); it != copied_map.const_end(); ++it) {
        int object_id = it->first;

        // can empire see object at all?  if not, skip copying object's info
        if (GetUniverse().GetObjectVisibilityByEmpire(object_id, empire_id) <= VIS_NO_VISIBILITY)
            continue;

        // copy object...  Note that the difference between this function and
        // the ObjectMap::Copy function is that any copied object here is
        // copied completely, whereas Copy only transfers the visible
        // information about each object that is copied.  If an object is
        // basic or partially visible, this function will still copy all
        // information about it in the copied_map.
        const UniverseObject* copy_from_object = it->second;
        if (UniverseObject* copy_to_object = this->Object(object_id)) {
            copy_to_object->Copy(copy_from_object, ALL_EMPIRES);            // there already is a version of this object present in this ObjectMap, so just update it
        } else {
            UniverseObject* clone = copy_from_object->Clone(ALL_EMPIRES);   // this object is not yet present in this ObjectMap, so add a new UniverseObject object for it
            this->Insert(object_id, clone);
        }
    }
}

ObjectMap* ObjectMap::Clone(int empire_id) const
{
    ObjectMap* retval = new ObjectMap();
    retval->Copy(*this, empire_id);
    return retval;
}

int ObjectMap::NumObjects() const
{
    return static_cast<int>(m_objects.size());
}

bool ObjectMap::Empty() const
{
    return m_objects.empty();
}

const UniverseObject* ObjectMap::Object(int id) const
{
    const_iterator it = m_const_objects.find(id);
    return (it != m_const_objects.end() ? it->second : 0);
}

UniverseObject* ObjectMap::Object(int id)
{
    iterator it = m_objects.find(id);
    return (it != m_objects.end() ? it->second : 0);
}

std::vector<const UniverseObject*> ObjectMap::FindObjects(const std::vector<int>& object_ids) const
{
    std::vector<const UniverseObject*> retval;
    for (std::vector<int>::const_iterator it = object_ids.begin(); it != object_ids.end(); ++it)
        if (const UniverseObject* obj = Object(*it))
            retval.push_back(obj);
        else
            Logger().errorStream() << "ObjectMap::FindObjects couldn't find object with id " << *it;
    return retval;
}

std::vector<UniverseObject*> ObjectMap::FindObjects(const std::vector<int>& object_ids)
{
    std::vector<UniverseObject*> retval;
    for (std::vector<int>::const_iterator it = object_ids.begin(); it != object_ids.end(); ++it)
        if (UniverseObject* obj = Object(*it))
            retval.push_back(obj);
        else
            Logger().errorStream() << "ObjectMap::FindObjects couldn't find object with id " << *it;
    return retval;
}

std::vector<const UniverseObject*> ObjectMap::FindObjects(const UniverseObjectVisitor& visitor) const
{
    std::vector<const UniverseObject*> retval;
    for (const_iterator it = m_const_objects.begin(); it != m_const_objects.end(); ++it) {
        if (UniverseObject* obj = it->second->Accept(visitor))
            retval.push_back(obj);
    }
    return retval;
}

std::vector<UniverseObject*> ObjectMap::FindObjects(const UniverseObjectVisitor& visitor)
{
    std::vector<UniverseObject*> retval;
    for (iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (UniverseObject* obj = it->second->Accept(visitor))
            retval.push_back(obj);
    }
    return retval;
}

std::vector<int> ObjectMap::FindObjectIDs(const UniverseObjectVisitor& visitor) const
{
    std::vector<int> retval;
    for (const_iterator it = m_const_objects.begin(); it != m_const_objects.end(); ++it) {
        if (it->second->Accept(visitor))
            retval.push_back(it->first);
    }
    return retval;
}

std::vector<int> ObjectMap::FindObjectIDs() const
{
    std::vector<int> retval;
    for (const_iterator it = m_const_objects.begin(); it != m_const_objects.end(); ++it)
        retval.push_back(it->first);
    return retval;
}

ObjectMap::iterator ObjectMap::begin()
{
    return m_objects.begin();
}

ObjectMap::iterator ObjectMap::end()
{
    return m_objects.end();
}

ObjectMap::const_iterator ObjectMap::const_begin() const
{
    return m_const_objects.begin();
}

ObjectMap::const_iterator ObjectMap::const_end() const
{
    return m_const_objects.end();
}

UniverseObject* ObjectMap::Insert(int id, UniverseObject* obj)
{
    // safety checks...
    if (!obj || id == UniverseObject::INVALID_OBJECT_ID)
        return 0;

    if (obj->ID() != id) {
        Logger().errorStream() << "ObjectMap::Insert passed object and id that doesn't match the object's id";
        obj->SetID(id);
    }

    // check if an object is in the map already with specified id
    std::map<int, UniverseObject*>::iterator it = m_objects.find(id);
    if (it == m_objects.end()) {
        // no pre-existing object was stored under specified id, so just insert
        // the new object
        m_objects[id] = obj;
        m_const_objects[id] = obj;
        return 0;
    }

    // pre-existing object is present.  need to get it and store it first...
    UniverseObject* old_obj = it->second;

    // and update maps
    it->second = obj;
    m_const_objects[id] = obj;

    // and return old object for external handling
    return old_obj;
}

UniverseObject* ObjectMap::Remove(int id)
{
    // search for object in objects maps
    std::map<int, UniverseObject*>::iterator it = m_objects.find(id);
    if (it == m_objects.end())
        return 0;

    // object found, so store pointer for later...
    UniverseObject* retval = it->second;

    // and erase from pointer maps
    m_objects.erase(it);
    m_const_objects.erase(id);

    return retval;
}

void ObjectMap::Delete(int id)
{
    delete Remove(id);
}

void ObjectMap::Clear()
{
    for (iterator it = m_objects.begin(); it != m_objects.end(); ++it)
        delete it->second;
    m_objects.clear();
    m_const_objects.clear();
}

void ObjectMap::swap(ObjectMap& rhs)
{
    m_objects.swap(rhs.m_objects);
    m_const_objects.swap(rhs.m_const_objects);
}

void ObjectMap::CopyObjectsToConstObjects()
{
    // remove existing entries in const objects and replace with values from non-const objects
    m_const_objects.clear();
    m_const_objects.insert(m_objects.begin(), m_objects.end());
}

std::string ObjectMap::Dump() const
{
    std::ostringstream os;
    os << "ObjectMap contains UniverseObjects: " << std::endl;
    for (ObjectMap::const_iterator it = const_begin(); it != const_end(); ++it) {
        os << it->second->Dump() << std::endl;
    }
    os << std::endl;
    return os.str();
}


/////////////////////////////////////////////
// struct Universe::GraphImpl
/////////////////////////////////////////////
struct Universe::GraphImpl
{
    typedef boost::property<vertex_system_id_t, int,
                            boost::property<boost::vertex_index_t, int> >   vertex_property_t;  ///< a system graph property map type
    typedef boost::property<boost::edge_weight_t, double>                   edge_property_t;    ///< a system graph property map type

    // declare main graph types, including properties declared above
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                                  vertex_property_t, edge_property_t> SystemGraph;

    struct EdgeVisibilityFilter
    {
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
            const System* system1 = GetEmpireKnownObject<System>(sys_id_1, m_empire_id);
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
// struct Universe::SourcedEffectsGroup
/////////////////////////////////////////////
Universe::SourcedEffectsGroup::SourcedEffectsGroup() :
    source_object_id(UniverseObject::INVALID_OBJECT_ID),
    effects_group()
{}
Universe::SourcedEffectsGroup::SourcedEffectsGroup(int source_object_id_, const boost::shared_ptr<const Effect::EffectsGroup>& effects_group_) :
    source_object_id(source_object_id_),
    effects_group(effects_group_)
{}

bool Universe::SourcedEffectsGroup::operator<(const SourcedEffectsGroup& right) const
{
    return (this->source_object_id < right.source_object_id ||
        (this->source_object_id == right.source_object_id) && this->effects_group < right.effects_group);
}


/////////////////////////////////////////////
// struct Universe::EffectCause
/////////////////////////////////////////////
Universe::EffectCause::EffectCause() :
    cause_type(INVALID_EFFECTS_GROUP_CAUSE_TYPE),
    specific_cause()
{}

Universe::EffectCause::EffectCause(EffectsCauseType cause_type_, const std::string& specific_cause_) :
    cause_type(cause_type_),
    specific_cause(specific_cause_)
{}


/////////////////////////////////////////////
// struct Universe::EffectTargetAndCause
/////////////////////////////////////////////
Universe::EffectTargetAndCause::EffectTargetAndCause() :
    target_set(),
    effect_cause()
{}

Universe::EffectTargetAndCause::EffectTargetAndCause(const Effect::TargetSet& target_set_, const EffectCause& effect_cause_) :
    target_set(target_set_),
    effect_cause(effect_cause_)
{}

/////////////////////////////////////////////
// struct Universe::EffectAccountingInfo
/////////////////////////////////////////////
Universe::EffectAccountingInfo::EffectAccountingInfo() :
    EffectCause(),
    source_id(UniverseObject::INVALID_OBJECT_ID),
    meter_change(0.0),
    running_meter_total(Meter::DEFAULT_VALUE)
{}


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
    m_last_allocated_object_id(-1), // this is conicidentally equal to UniverseObject::INVALID_OBJECT_ID as of this writing, but the reason for this to be -1 is so that the first object has id 0, and all object ids are non-negative
    m_last_allocated_design_id(-1)  // same, but for ShipDesign::INVALID_DESIGN_ID
{}

Universe::~Universe()
{
    Clear();

    delete m_graph_impl;
}

void Universe::Clear()
{
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

const ObjectMap& Universe::Objects() const
{ return m_objects; }

ObjectMap& Universe::Objects()
{ return m_objects; }

const ObjectMap& Universe::EmpireKnownObjects(int empire_id) const
{
    if (empire_id == ALL_EMPIRES)
        return m_objects;

    EmpireObjectMap::const_iterator it = m_empire_latest_known_objects.find(empire_id);
    if (it != m_empire_latest_known_objects.end()) {
        return it->second;
    }

    static const ObjectMap const_empty_map;
    return const_empty_map;
}

ObjectMap& Universe::EmpireKnownObjects(int empire_id)
{
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

std::set<int> Universe::EmpireVisibleObjectIDs(int empire_id/* = ALL_EMPIRES*/) const
{
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

const std::set<int>& Universe::EmpireKnownDestroyedObjectIDs(int empire_id) const
{
    ObjectKnowledgeMap::const_iterator it = m_empire_known_destroyed_object_ids.find(empire_id);
    if (it != m_empire_known_destroyed_object_ids.end())
        return it->second;
    static const std::set<int> empty_set;
    return empty_set;
}

const ShipDesign* Universe::GetShipDesign(int ship_design_id) const
{
    if (ship_design_id == ShipDesign::INVALID_DESIGN_ID)
        return 0;
    ship_design_iterator it = m_ship_designs.find(ship_design_id);
    return (it != m_ship_designs.end() ? it->second : 0);
}

const ShipDesign* Universe::GetGenericShipDesign(const std::string& name) const
{
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

const std::set<int>& Universe::EmpireKnownShipDesignIDs(int empire_id) const
{
    std::map<int, std::set<int> >::const_iterator it = m_empire_known_ship_design_ids.find(empire_id);
    if (it != m_empire_known_ship_design_ids.end())
        return it->second;
    static const std::set<int> empty_set;
    return empty_set;
}

Visibility Universe::GetObjectVisibilityByEmpire(int object_id, int empire_id) const
{
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

const Universe::VisibilityTurnMap& Universe::GetObjectVisibilityTurnMapByEmpire(int object_id, int empire_id) const
{
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

double Universe::LinearDistance(int system1_id, int system2_id) const
{
    //Logger().debugStream() << "LinearDistance(" << system1_id << ", " << system2_id << ")";
    int system1_index = SystemGraphIndex(m_graph_impl->system_graph, system1_id);
    int system2_index = SystemGraphIndex(m_graph_impl->system_graph, system2_id);
    return m_system_distances.at(std::max(system1_index, system2_index)).at(std::min(system1_index, system2_index));
}

std::pair<std::list<int>, double> Universe::ShortestPath(int system1_id, int system2_id, int empire_id/* = ALL_EMPIRES*/) const
{
    double linear_distance = LinearDistance(system1_id, system2_id);
    if (empire_id == ALL_EMPIRES) {
        return ShortestPathImpl(m_graph_impl->system_graph, system1_id, system2_id, linear_distance);
    } else {
        GraphImpl::EmpireViewSystemGraphMap::const_iterator graph_it = m_graph_impl->empire_system_graph_views.find(empire_id);
        if (graph_it != m_graph_impl->empire_system_graph_views.end())
            return ShortestPathImpl(*graph_it->second, system1_id, system2_id, linear_distance);
    }
    return std::pair<std::list<int>, double>();
}

std::pair<std::list<int>, int> Universe::LeastJumpsPath(int system1_id, int system2_id, int empire_id/* = ALL_EMPIRES*/) const
{
    if (empire_id == ALL_EMPIRES) {
        return LeastJumpsPathImpl(m_graph_impl->system_graph, system1_id, system2_id);
    } else {
        GraphImpl::EmpireViewSystemGraphMap::const_iterator graph_it = m_graph_impl->empire_system_graph_views.find(empire_id);
        if (graph_it != m_graph_impl->empire_system_graph_views.end())
            return LeastJumpsPathImpl(*graph_it->second, system1_id, system2_id);
    }
    return std::pair<std::list<int>, int>();
}

bool Universe::SystemsConnected(int system1_id, int system2_id, int empire_id) const
{
    std::pair<std::list<int>, int> path = LeastJumpsPath(system1_id, system2_id, empire_id);
    return (!path.first.empty());
}

bool Universe::SystemHasVisibleStarlanes(int system_id, int empire_id) const
{
    if (const System* system = GetEmpireKnownObject<System>(system_id, empire_id))
        if (!system->StarlanesWormholes().empty())
            return true;
    return false;
}

std::map<double, int> Universe::ImmediateNeighbors(int system_id, int empire_id/* = ALL_EMPIRES*/) const
{
    if (empire_id == ALL_EMPIRES) {
        return ImmediateNeighborsImpl(m_graph_impl->system_graph, system_id);
    } else {
        GraphImpl::EmpireViewSystemGraphMap::const_iterator graph_it = m_graph_impl->empire_system_graph_views.find(empire_id);
        if (graph_it != m_graph_impl->empire_system_graph_views.end())
            return ImmediateNeighborsImpl(*graph_it->second, system_id);
    }
    return std::map<double, int>();
}

int Universe::Insert(UniverseObject* obj)
{
    if (!obj)
        return UniverseObject::INVALID_OBJECT_ID;

    if (m_last_allocated_object_id + 1 < UniverseObject::MAX_ID) {
        int id = ++m_last_allocated_object_id;
        obj->SetID(id);
        m_objects.Insert(id, obj);
        return id;
    }

    // we'll probably never execute this branch, considering how many IDs are available
    // find a hole in the assigned IDs in which to place the object
    int last_id_seen = UniverseObject::INVALID_OBJECT_ID;
    for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (1 < it->first - last_id_seen) {
            int id = last_id_seen + 1;
            obj->SetID(id);
            m_objects.Insert(id, obj);
            return id;
        }
    }

    return UniverseObject::INVALID_OBJECT_ID;
}

bool Universe::InsertID(UniverseObject* obj, int id)
{
    if (!obj || id == UniverseObject::INVALID_OBJECT_ID || id >= UniverseObject::MAX_ID)
        return false;

    obj->SetID(id);
    m_objects.Insert(id, obj);
    return true;
}

int Universe::InsertShipDesign(ShipDesign* ship_design)
{
    int retval = ShipDesign::INVALID_DESIGN_ID;
    if (ship_design) {
        if (m_last_allocated_design_id + 1 < UniverseObject::MAX_ID) {
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

bool Universe::InsertShipDesignID(ShipDesign* ship_design, int id)
{
    bool retval = false;

    if (ship_design  &&  id != ShipDesign::INVALID_DESIGN_ID  &&  id < ShipDesign::MAX_ID) {
        ship_design->SetID(id);
        m_ship_designs[id] = ship_design;
        retval = true;
    }
    return retval;
}

void Universe::ApplyAllEffectsAndUpdateMeters(const std::vector<int>& object_ids)
{
    // cache all activation and scoping condition results before applying Effects, since the application of
    // these Effects may affect the activation and scoping evaluations
    EffectsTargetsCausesMap targets_causes_map;
    GetEffectsAndTargets(targets_causes_map, object_ids);

    std::vector<UniverseObject*> objects = m_objects.FindObjects(object_ids);

    // revert all current meter values (which are modified by effects) to
    // their initial state for this turn, so that max/target/unpaired meter
    // value can be calculated (by accumulating all effects' modifications this
    // turn) and active meters have the proper baseline from which to
    // accumulate changes from effects
    for (std::vector<UniverseObject*>::iterator it = objects.begin(); it != objects.end(); ++it) {
        (*it)->ResetTargetMaxUnpairedMeters();
        (*it)->ResetPairedActiveMeters();
    }

    ExecuteEffects(targets_causes_map);


    // re-get objects vector so that any destroyed objects won't still be in the list
    objects = m_objects.FindObjects(object_ids);

    // clamp max meters to [DEFAULT_VALUE, LARGE_VALUE] and current meters to [DEFAULT_VALUE, max]
    // clamp max and target meters to [DEFAULT_VALUE, LARGE_VALUE] and current meters to [DEFAULT_VALUE, max]
    for (std::vector<UniverseObject*>::iterator it = objects.begin(); it != objects.end(); ++it) {
        (*it)->ClampMeters();
    }
}

void Universe::ApplyAllEffectsAndUpdateMeters()
{
    ApplyAllEffectsAndUpdateMeters(m_objects.FindObjectIDs());
}

void Universe::ApplyMeterEffectsAndUpdateMeters(const std::vector<int>& object_ids)
{
    // cache all activation and scoping condition results before applying Effects, since the application of
    // these Effects may affect the activation and scoping evaluations
    EffectsTargetsCausesMap targets_causes_map;
    GetEffectsAndTargets(targets_causes_map, object_ids);

    std::vector<UniverseObject*> objects = m_objects.FindObjects(object_ids);

    // revert all current meter values (which are modified by effects) to
    // their initial state for this turn, so that max/target/unpaired meter
    // value can be calculated (by accumulating all effects' modifications this
    // turn) and active meters have the proper baseline from which to
    // accumulate changes from effects
    for (std::vector<UniverseObject*>::iterator it = objects.begin(); it != objects.end(); ++it) {
        (*it)->ResetTargetMaxUnpairedMeters();
        (*it)->ResetPairedActiveMeters();
    }

    ExecuteMeterEffects(targets_causes_map);

    for (std::vector<UniverseObject*>::iterator it = objects.begin(); it != objects.end(); ++it) {
        (*it)->ClampMeters();  // clamp max, target and unpaired meters to [DEFAULT_VALUE, LARGE_VALUE] and active meters with max meters to [DEFAULT_VALUE, max]
    }
}

void Universe::ApplyMeterEffectsAndUpdateMeters()
{
    ApplyMeterEffectsAndUpdateMeters(m_objects.FindObjectIDs());
}

void Universe::InitMeterEstimatesAndDiscrepancies()
{
    boost::timer timer;

    // clear old discrepancies and accounting
    m_effect_discrepancy_map.clear();
    m_effect_accounting_map.clear();

    //Logger().debugStream() << "Universe::InitMeterEstimatesAndDiscrepancies";

    // generate new estimates (normally uses discrepancies, but in this case will find none)
    UpdateMeterEstimates();

    // determine meter max discrepancies
    for (EffectAccountingMap::iterator obj_it = m_effect_accounting_map.begin(); obj_it != m_effect_accounting_map.end(); ++obj_it) {
        UniverseObject* obj = m_objects.Object(obj_it->first);    // object that has some meters
        if (!obj) {
            Logger().errorStream() << "Universe::InitMeterEstimatesAndDiscrepancies couldn't find an object that was in the effect accounting map...?";
            continue;
        }
        std::map<MeterType, std::vector<EffectAccountingInfo> >& meters_map = obj_it->second;

        // every meter has a value at the start of the turn, and a value after updating with known effects
        for (std::map<MeterType, std::vector<EffectAccountingInfo> >::iterator meter_type_it = meters_map.begin(); meter_type_it != meters_map.end(); ++meter_type_it) {
            MeterType type = meter_type_it->first;
            Meter* meter = obj->GetMeter(type);
            assert(meter);  // all objects should only have accounting info for a meter if that meter exists
            int object_id = obj->ID();

            // discrepancy is the difference between expected and actual meter values at start of turn
            double discrepancy = meter->Initial() - meter->Current();

            if (discrepancy == 0.0) continue;   // no discrepancy for this meter

            // add to discrepancy map
            m_effect_discrepancy_map[object_id][type] = discrepancy;

            // correct current max meter estimate for discrepancy
            meter->AddToCurrent(discrepancy);

            // add discrepancy adjustment to meter accounting
            EffectAccountingInfo info;
            info.cause_type = ECT_UNKNOWN_CAUSE;
            info.meter_change = discrepancy;
            info.running_meter_total = meter->Current();

            m_effect_accounting_map[object_id][type].push_back(info);
        }
    }

    Logger().debugStream() << "Universe::InitMeterEstimatesAndDiscrepancies time: " << (timer.elapsed() * 1000.0);
}

void Universe::UpdateMeterEstimates()
{
    UpdateMeterEstimates(UniverseObject::INVALID_OBJECT_ID, INVALID_METER_TYPE, false);
}

void Universe::UpdateMeterEstimates(int object_id, MeterType meter_type, bool update_contained_objects)
{
    if (object_id == UniverseObject::INVALID_OBJECT_ID) {
        // update meters for all objects.  Value of updated_contained_objects is irrelivant and is ignored in this case.
        std::vector<int> object_ids;
        for (ObjectMap::iterator obj_it = m_objects.begin(); obj_it != m_objects.end(); ++obj_it)
            object_ids.push_back(obj_it->first);

        UpdateMeterEstimates(object_ids, meter_type);
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

        // add current object to set if it has the appropriate meter
        if (meter_type == INVALID_METER_TYPE) {
            // add object and clear effect accounting for all its meters
            objects_set.insert(cur_object_id);
            m_effect_accounting_map[cur_object_id].clear();
        } else if (cur_object->GetMeter(meter_type)) {
            // add object and clear effect accounting for just the relevant meter
            objects_set.insert(cur_object_id);
            m_effect_accounting_map[cur_object_id][meter_type].clear();
        }

        // add contained objects to list of objects to process, if requested.  assumes no objects contain themselves (which could cause infinite loops)
        if (update_contained_objects) {
            std::vector<int> contained_objects = cur_object->FindObjectIDs(); // get all contained objects
            std::copy(contained_objects.begin(), contained_objects.end(), std::back_inserter(objects_list));
        }
    }
    std::vector<int> objects_vec;
    std::copy(objects_set.begin(), objects_set.end(), std::back_inserter(objects_vec));
    UpdateMeterEstimatesImpl(objects_vec, meter_type);
}

void Universe::UpdateMeterEstimates(const std::vector<int>& objects_vec, MeterType meter_type)
{
    boost::timer timer;

    std::set<int> objects_set;  // ensures no duplicates

    if (meter_type == INVALID_METER_TYPE) {
        for (std::vector<int>::const_iterator obj_it = objects_vec.begin(); obj_it != objects_vec.end(); ++obj_it) {
            int cur_object_id = *obj_it;
            m_effect_accounting_map[cur_object_id].clear();
            objects_set.insert(cur_object_id);    // all meters are accepted, so don't need to check if an object has a particular meter (ignoring case where objects have no meters)
        }
    } else {
        // need to check that each object has the requested meter type
        for (std::vector<int>::const_iterator obj_it = objects_vec.begin(); obj_it != objects_vec.end(); ++obj_it) {
            int cur_object_id = *obj_it;
            UniverseObject* cur_object = m_objects.Object(cur_object_id);
            if (cur_object->GetMeter(meter_type)) {
                m_effect_accounting_map[cur_object_id][meter_type].clear();
                objects_set.insert(cur_object_id);
            }
        }
    }
    std::vector<int> final_objects_vec;
    std::copy(objects_set.begin(), objects_set.end(), std::back_inserter(final_objects_vec));
    UpdateMeterEstimatesImpl(final_objects_vec, meter_type);

    Logger().debugStream() << "Universe::UpdateMeterEstimates time: " << (timer.elapsed() * 1000.0);
}

void Universe::UpdateMeterEstimatesImpl(const std::vector<int>& objects_vec, MeterType meter_type)
{
    for (std::vector<int>::const_iterator obj_it = objects_vec.begin(); obj_it != objects_vec.end(); ++obj_it) {
        int obj_id = *obj_it;
        UniverseObject* obj = m_objects.Object(obj_id);

        // Reset max meters to DEFAULT_VALUE
        obj->ResetTargetMaxUnpairedMeters(meter_type);

        // Reset current meters to initial value at start of this turn
        obj->ResetPairedActiveMeters(meter_type);

        // record current value(s) of meter(s) after resetting
        MeterType start, end;
        if (meter_type == INVALID_METER_TYPE) {
            // record data for all meters
            start = MeterType(0);
            end = NUM_METER_TYPES;
        } else {
            // record for only relevant meter type
            start = meter_type;
            end = MeterType(meter_type + 1);
        }
        for (MeterType type = start; type != end; type = MeterType(type + 1)) {
            if (Meter* meter = obj->GetMeter(type)) {
                EffectAccountingInfo info;
                info.source_id = UniverseObject::INVALID_OBJECT_ID;
                info.cause_type = ECT_INHERENT;
                info.meter_change = meter->Current() - Meter::DEFAULT_VALUE;
                info.running_meter_total = meter->Current();

                if (info.meter_change > 0.0)
                    m_effect_accounting_map[obj_id][type].push_back(info);
            }
        }
    }

    Logger().debugStream() << "UpdateMeterEstimatesImpl after resetting meters";
    Logger().debugStream() << m_objects.Dump();

    // cache all activation and scoping condition results before applying Effects, since the application of
    // these Effects may affect the activation and scoping evaluations
    EffectsTargetsCausesMap targets_causes_map;
    GetEffectsAndTargets(targets_causes_map, objects_vec);

    // Apply and record effect meter adjustments
    ExecuteMeterEffects(targets_causes_map);      // TODO: make act only on contents of objects vector

    Logger().debugStream() << "UpdateMeterEstimatesImpl after executing effects";
    Logger().debugStream() << m_objects.Dump();

    // Apply known discrepancies between expected and calculated meter maxes at start of turn.  This
    // accounts for the unknown effects on the meter, and brings the estimate in line with the actual
    // max at the start of the turn
    if (!m_effect_discrepancy_map.empty()) {
        for (std::vector<int>::const_iterator obj_it = objects_vec.begin(); obj_it != objects_vec.end(); ++obj_it) {
            int obj_id = *obj_it;
            UniverseObject* obj = m_objects.Object(obj_id);

            // check if this object has any discrepancies
            EffectDiscrepancyMap::iterator dis_it = m_effect_discrepancy_map.find(obj_id);
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

                    EffectAccountingInfo info;
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

    Logger().debugStream() << "UpdateMeterEstimatesImpl after discrepancies and clamping";
    Logger().debugStream() << m_objects.Dump();
}

void Universe::BackPropegateObjectMeters(const std::vector<int>& object_ids)
{
    std::vector<UniverseObject*> objects = m_objects.FindObjects(object_ids);

    // copy current meter values to initial values
    for (std::vector<UniverseObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
        (*it)->BackPropegateMeters();
}

void Universe::BackPropegateObjectMeters()
{
    BackPropegateObjectMeters(m_objects.FindObjectIDs());
}

void Universe::GetEffectsAndTargets(EffectsTargetsCausesMap& targets_causes_map)
{
    targets_causes_map.clear();

    std::vector<int> all_objects = m_objects.FindObjectIDs();
    GetEffectsAndTargets(targets_causes_map, all_objects);
}

void Universe::GetEffectsAndTargets(EffectsTargetsCausesMap& targets_causes_map, const std::vector<int>& target_objects)
{
    Logger().debugStream() << "Universe::GetEffectsAndTargets";
    // 0) EffectsGroups from Species
    Logger().debugStream() << "Universe::GetEffectsAndTargets for SPECIES";
    for (ObjectMap::const_iterator it = m_objects.const_begin(); it != m_objects.const_end(); ++it) {
        //Logger().debugStream() << "... object (" << it->first << "): " << it->second->Name();
        const PopCenter* pc = dynamic_cast<const PopCenter*>(it->second);
        if (!pc) continue;
        const std::string& species_name = pc->SpeciesName();
        //Logger().debugStream() << "... ... PopCenter species: " << species_name;
        if (species_name.empty())
            continue;
        const Species* species = GetSpecies(species_name);
        if (!species) {
            Logger().errorStream() << "GetEffectsAndTargets couldn't get Species " << species_name;
            continue;
        }
        StoreTargetsAndCausesOfEffectsGroups(species->Effects(), it->first, ECT_SPECIES, species_name,
                                             target_objects, targets_causes_map);
    }

    // 1) EffectsGroups from Specials
    Logger().debugStream() << "Universe::GetEffectsAndTargets for SPECIALS";
    for (ObjectMap::const_iterator it = m_objects.const_begin(); it != m_objects.const_end(); ++it) {
        int source_object_id = it->first;
        const std::set<std::string>& specials = it->second->Specials();
        for (std::set<std::string>::const_iterator special_it = specials.begin(); special_it != specials.end(); ++special_it) {
            const Special* special = GetSpecial(*special_it);
            if (!special) {
                Logger().errorStream() << "GetEffectsAndTargets couldn't get Special " << *special_it;
                continue;
            }

            StoreTargetsAndCausesOfEffectsGroups(special->Effects(), source_object_id, ECT_SPECIAL, special->Name(),
                                                 target_objects, targets_causes_map);
        }
    }

    // 2) EffectsGroups from Techs
    Logger().debugStream() << "Universe::GetEffectsAndTargets for TECHS";
    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        const Empire* empire = it->second;
        for (Empire::TechItr tech_it = empire->TechBegin(); tech_it != empire->TechEnd(); ++tech_it) {
            const Tech* tech = GetTech(*tech_it);
            if (!tech) continue;

            StoreTargetsAndCausesOfEffectsGroups(tech->Effects(), empire->CapitolID(), ECT_TECH, tech->Name(),
                                                 target_objects, targets_causes_map);
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
                                             target_objects, targets_causes_map);
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
                                             target_objects, targets_causes_map);

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
                                                 target_objects, targets_causes_map);
        }
    }
}

void Universe::StoreTargetsAndCausesOfEffectsGroups(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups,
                                                    int source_object_id, EffectsCauseType effect_cause_type,
                                                    const std::string& specific_cause_name,
                                                    const std::vector<int>& target_objects, EffectsTargetsCausesMap& targets_causes_map)
{
    Logger().debugStream() << "Universe::StoreTargetsAndCausesOfEffectsGroups( , source id: " << source_object_id << ", , specific cause: " << specific_cause_name << ", , )";

    // transfer target objects from input vector to a set
    Effect::TargetSet all_potential_targets;
    for (std::vector<int>::const_iterator it = target_objects.begin(); it != target_objects.end(); ++it)
        all_potential_targets.insert(m_objects.Object(*it));

    // process all effects groups in set provided
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >::const_iterator effects_it;
    for (effects_it = effects_groups.begin(); effects_it != effects_groups.end(); ++effects_it) {
        // create non_targets and targets sets for current effects group
        Effect::TargetSet target_set;                                    // initially empty
        Effect::TargetSet potential_target_set = all_potential_targets;  // copy again each iteration, so previous iterations don't affect starting potential targets of next iteration

        // get effects group to process for this iteration
        boost::shared_ptr<const Effect::EffectsGroup> effects_group = *effects_it;

        // get set of target objects for this effects group from potential targets specified
        effects_group->GetTargetSet(source_object_id, target_set, potential_target_set);    // transfers objects from potential_target_set to target_set if they meet the condition

        // abort if no targets
        if (target_set.empty())
            continue;

        // combine effects group and source object id into a sourced effects group
        SourcedEffectsGroup sourced_effects_group(source_object_id, effects_group);

        // combine cause type and specific cause into effect cause
        EffectCause effect_cause(effect_cause_type, specific_cause_name);

        // combine target set and effect cause
        EffectTargetAndCause target_and_cause(target_set, effect_cause);

        // store effect cause and targets info in map, indexed by sourced effects group
        targets_causes_map.insert(std::make_pair(sourced_effects_group, target_and_cause));
    }
}

void Universe::ExecuteEffects(const EffectsTargetsCausesMap& targets_causes_map)
{
    m_marked_destroyed.clear();
    m_marked_for_victory.clear();
    std::map<std::string, Effect::TargetSet> executed_nonstacking_effects;

    for (EffectsTargetsCausesMap::const_iterator targets_it = targets_causes_map.begin(); targets_it != targets_causes_map.end(); ++targets_it) {
        // if other EffectsGroups with the same stacking group have affected some of the targets in
        // the scope of the current EffectsGroup, skip them

        const SourcedEffectsGroup& sourced_effects_group = targets_it->first;
        const boost::shared_ptr<const Effect::EffectsGroup> effects_group = sourced_effects_group.effects_group;

        const EffectTargetAndCause& targets_and_cause = targets_it->second;
        Effect::TargetSet targets = targets_and_cause.target_set;

        std::map<std::string, std::set<UniverseObject*> >::iterator non_stacking_it = executed_nonstacking_effects.find(effects_group->StackingGroup());
        if (non_stacking_it != executed_nonstacking_effects.end()) {
            for (Effect::TargetSet::const_iterator object_it = non_stacking_it->second.begin(); object_it != non_stacking_it->second.end(); ++object_it) {
                targets.erase(*object_it);
            }
        }


        // execute the Effects in the EffectsGroup
        effects_group->Execute(sourced_effects_group.source_object_id, targets);


        // if this EffectsGroup belongs to a stacking group, add the objects just affected by it to executed_nonstacking_effects
        if (effects_group->StackingGroup() != "") {
            Effect::TargetSet& affected_targets = executed_nonstacking_effects[effects_group->StackingGroup()];
            for (Effect::TargetSet::const_iterator object_it = targets.begin(); object_it != targets.end(); ++object_it) {
                affected_targets.insert(*object_it);
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

void Universe::ExecuteMeterEffects(const EffectsTargetsCausesMap& targets_causes_map)
{
    //Logger().debugStream() << "Universe::ExecuteMeterEffects";
    std::map<std::string, Effect::TargetSet> executed_nonstacking_effects;

    for (EffectsTargetsCausesMap::const_iterator targets_it = targets_causes_map.begin(); targets_it != targets_causes_map.end(); ++targets_it) {
        // if other EffectsGroups with the same stacking group have affected some of the targets in
        // the scope of the current EffectsGroup, skip them

        const SourcedEffectsGroup& sourced_effects_group = targets_it->first;
        const boost::shared_ptr<const Effect::EffectsGroup> effects_group = sourced_effects_group.effects_group;

        const EffectTargetAndCause& targets_and_cause = targets_it->second;
        Effect::TargetSet targets = targets_and_cause.target_set;

        std::map<std::string, std::set<UniverseObject*> >::iterator non_stacking_it = executed_nonstacking_effects.find(effects_group->StackingGroup());
        if (non_stacking_it != executed_nonstacking_effects.end()) {
            for (Effect::TargetSet::const_iterator object_it = non_stacking_it->second.begin(); object_it != non_stacking_it->second.end(); ++object_it) {
                targets.erase(*object_it);
            }
        }


        // execute only the SetMeter and SetShipPartMeter effects in the EffectsGroup
        const std::vector<Effect::EffectBase*>& effects = effects_group->EffectsList();
        for (unsigned int i = 0; i < effects.size(); ++i) {
            const Effect::SetMeter* meter_effect = dynamic_cast<Effect::SetMeter*>(effects[i]);
            if (!meter_effect && !dynamic_cast<Effect::SetShipPartMeter*>(effects[i]))
                continue;

            // determine meter to be altered by this effect
            MeterType meter_type = INVALID_METER_TYPE;

            if (meter_effect) {
                meter_type = meter_effect->GetMeterType();

                // record pre-effect meter values
                for (Effect::TargetSet::iterator target_it = targets.begin(); target_it != targets.end(); ++target_it) {
                    UniverseObject* target = *target_it;
                    const Meter* meter = target->GetMeter(meter_type);
                    if (!meter) continue;   // some objects might match target conditions, but not actually have the relevant meter

                    // accounting info for this effect on this meter
                    EffectAccountingInfo info;
                    info.cause_type =           targets_and_cause.effect_cause.cause_type;
                    info.specific_cause =       targets_and_cause.effect_cause.specific_cause;
                    info.source_id =            sourced_effects_group.source_object_id;
                    info.running_meter_total =  meter->Current();

                    // add accounting for this effect to end of vector
                    m_effect_accounting_map[target->ID()][meter_type].push_back(info);
                }
            }

            // apply meter-altering effect to targets
            effects_group->Execute(sourced_effects_group.source_object_id, targets, i);

            if (meter_effect) {
                // find change in meter due to effect: equal to post-meter minus pre-meter value
                for (Effect::TargetSet::iterator target_it = targets.begin(); target_it != targets.end(); ++target_it) {
                    UniverseObject* target = *target_it;
                    const Meter* meter = target->GetMeter(meter_type);
                    if (!meter) continue;   // some objects might match target conditions, but not actually have the relevant meter

                    // retreive info for this effect
                    EffectAccountingInfo& info = m_effect_accounting_map[target->ID()][meter_type].back();

                    // update accounting info with meter change and new total
                    info.meter_change = meter->Current() - info.running_meter_total;
                    info.running_meter_total = meter->Current();
                }
            }
        }

        // if this EffectsGroup belongs to a stacking group, add the objects just affected by it to executed_nonstacking_effects
        if (effects_group->StackingGroup() != "") {
            Effect::TargetSet& affected_targets = executed_nonstacking_effects[effects_group->StackingGroup()];
            for (Effect::TargetSet::const_iterator object_it = targets.begin(); object_it != targets.end(); ++object_it)
                affected_targets.insert(*object_it);
        }
    }
}

namespace {
    /** Sets visibilities for indicated \a empires of object with \a object_id
      * in the passed-in \a empire_vis_map to \a vis */
    void SetEmpireObjectVisibility(Universe::EmpireObjectVisibilityMap& empire_vis_map,
                                   std::map<int, std::set<int> >& empire_known_design_ids,
                                   const std::set<int>& empires,
                                   int object_id,
                                   Visibility vis)
    {
        for (std::set<int>::const_iterator empire_it = empires.begin(); empire_it != empires.end(); ++empire_it) {
            int empire_id = *empire_it;

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
                if (const Ship* ship = GetObject<Ship>(object_id)) {
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
}

void Universe::UpdateEmpireObjectVisibilities()
{
    //Logger().debugStream() << "Universe::UpdateEmpireObjectVisibilities()";

    // ensure Universe knows empires have knowledge of designs the empire is specifically remembering
    for (EmpireManager::iterator empire_it = Empires().begin(); empire_it != Empires().end(); ++empire_it) {
        const Empire* empire = empire_it->second;
        int empire_id = empire_it->first;
        const std::set<int>& empire_known_ship_designs = empire->ShipDesigns();
        for (std::set<int>::const_iterator design_it = empire_known_ship_designs.begin(); design_it != empire_known_ship_designs.end(); ++design_it)
            m_empire_known_ship_design_ids[empire_id].insert(*design_it);
    }


    m_empire_object_visibility.clear();


    if (ALL_OBJECTS_VISIBLE) {
        // set every object visible to all empires
        std::set<int> all_empire_ids;
        for (EmpireManager::iterator empire_it = Empires().begin(); empire_it != Empires().end(); ++empire_it)
            all_empire_ids.insert(empire_it->first);

        for (ObjectMap::const_iterator obj_it = m_objects.const_begin(); obj_it != m_objects.const_end(); ++obj_it)
            SetEmpireObjectVisibility(m_empire_object_visibility, m_empire_known_ship_design_ids, all_empire_ids, obj_it->first, VIS_FULL_VISIBILITY);

        return;
    }


    // for each detecting object
    for (ObjectMap::const_iterator detector_it = m_objects.const_begin(); detector_it != m_objects.const_end(); ++detector_it) {
        // get detector object
        const UniverseObject* detector = detector_it->second;
        if (!detector) continue;

        int detector_id = detector->ID();


        // get owners of detector
        const std::set<int> detector_owners = detector->Owners();
        if (detector_owners.empty()) continue;  // no point in continuing if object has no owners... no-one can get vision from this object


        // owners of an object get full visibility of it
        SetEmpireObjectVisibility(m_empire_object_visibility, m_empire_known_ship_design_ids, detector_owners, detector_id, VIS_FULL_VISIBILITY);


        // get detection ability
        const Meter* detection_meter = detector->GetMeter(METER_DETECTION);
        if (!detection_meter) continue;
        double detection = detection_meter->Current();


        //Logger().debugStream() << "Detector object: " << detector->Name() << " (" << detector->ID() << ") detection: " << detection;

        // position of detector
        double xd = detector->X();
        double yd = detector->Y();


        // for each detectable object
        for (ObjectMap::const_iterator target_it = m_objects.const_begin(); target_it != m_objects.const_end(); ++target_it) {
            // special case for pairs

            if (target_it == detector_it)
                continue;   // can't detect any better than done above

            // get stealthy object
            const UniverseObject* target = target_it->second;
            if (!target) continue;


            //Logger().debugStream() << " ... considering stealthy object " << target->Name() << " (" << target->ID() << ")";

            //// check if stealthy object is a fleet.  if so, don't bother
            //// setting its visibility now, but later get visibility by
            //// propegating from ships in fleet
            //if (universe_object_cast<const Fleet*>(target))
            //    continue;


            // get stealth
            double stealth = 0.0;
            const Meter* stealth_meter = target->GetMeter(METER_STEALTH);
            if (stealth_meter)
                stealth = stealth_meter->Current();

            //Logger().debugStream() << " ... ... target stealth: " << stealth;

            Visibility target_visibility_to_detector = VIS_NO_VISIBILITY;

            // zero-stealth objects are always at least basic-level visible
            if (stealth <= 0)
                target_visibility_to_detector = VIS_BASIC_VISIBILITY;


            // compare stealth, detection ability and distance between
            // detector and target to find visibility level

            // position of target
            double xt = target->X();
            double yt = target->Y();

            // distance squared
            double dist2 = (xt-xd)*(xt-xd) + (yt-yd)*(yt-yd);


            // To determine if a detector can detect a target, the target's
            // stealth is subtracted from the detector's range, and the result
            // is compared to the distance between them. If the distance is
            // less than 10*(detector_detection - target_stealth), then the
            // target is seen by the detector with partial visibility.
            double detect_range = std::max(0.0, 10.0*(detection - stealth));

            if (dist2 <= detect_range * detect_range)
                target_visibility_to_detector = VIS_PARTIAL_VISIBILITY;


            if (target_visibility_to_detector <= VIS_NO_VISIBILITY)
                continue;


            // Note that owning an object grants FULL visibility in the containing loop


            int target_id = target->ID();

            // if target visible to detector, update visibility of target for all empires that own detector
            SetEmpireObjectVisibility(m_empire_object_visibility, m_empire_known_ship_design_ids, detector_owners, target_id, target_visibility_to_detector);
        }
    }


    // propegate visibility from contained to container objects
    for (ObjectMap::const_iterator container_object_it = m_objects.const_begin(); container_object_it != m_objects.const_end(); ++container_object_it) {
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
            if (fleet->SystemID() != UniverseObject::INVALID_OBJECT_ID || fleet->Unowned())
                continue;

            int prev = fleet->PreviousSystemID();
            int next = fleet->NextSystemID();

            // for each empire that owns the fleet, ensure that empire has
            // at least basic visibility of the next and previous systems
            // on the fleet's path
            const std::set<int>& owners = fleet->Owners();
            for (std::set<int>::const_iterator empire_it = owners.begin(); empire_it != owners.end(); ++empire_it) {
                ObjectVisibilityMap& vis_map = m_empire_object_visibility[*empire_it];

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
    }
}

void Universe::UpdateEmpireLatestKnownObjectsAndVisibilityTurns()
{
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

void Universe::SetEmpireKnowledgeOfDestroyedObject(int object_id, int empire_id)
{
    if (object_id == UniverseObject::INVALID_OBJECT_ID) {
        Logger().errorStream() << "SetEmpireKnowledgeOfDestroyedObject called with INVALID_OBJECT_ID";
        return;
    }

    const Empire* empire = Empires().Lookup(empire_id);
    if (!empire) {
        Logger().errorStream() << "SetEmpireKnowledgeOfDestroyedObject called for invalid empire id: " << empire_id;
    }
    m_empire_known_destroyed_object_ids[empire_id].insert(object_id);
}

void Universe::SetEmpireKnowledgeOfShipDesign(int ship_design_id, int empire_id)
{
    if (ship_design_id == ShipDesign::INVALID_DESIGN_ID) {
        Logger().errorStream() << "SetEmpireKnowledgeOfShipDesign called with INVALID_DESIGN_ID";
        return;
    }

    if (!Empires().Lookup(empire_id))
        Logger().errorStream() << "SetEmpireKnowledgeOfShipDesign called for invalid empire id: " << empire_id;

    m_empire_known_ship_design_ids[empire_id].insert(ship_design_id);
}

void Universe::Destroy(int object_id, bool update_destroyed_object_knowers/* = true*/)
{
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


    // remove from existing objects set and insert into destroyed objects set
    UniverseObjectDeleteSignal(obj);
    delete m_objects.Remove(object_id);
}

void Universe::RecursiveDestroy(int object_id)
{
    UniverseObject* obj = m_objects.Object(object_id);
    if (!obj) {
        Logger().debugStream() << "Universe::RecursiveDestroy asked to destroy nonexistant object with id " << object_id;
        return;
    }

    if (Ship* ship = universe_object_cast<Ship*>(obj)) {
        // if a ship is being deleted, and it is the last ship in its fleet, then the empty fleet should also be deleted
        Fleet* fleet = GetObject<Fleet>(ship->FleetID());
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

bool Universe::Delete(int object_id)
{
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
{
    m_marked_destroyed.insert(object_id);
}

void Universe::EffectVictory(int object_id, const std::string& reason_string)
{
    m_marked_for_victory.insert(std::pair<int, std::string>(object_id, reason_string));
}

void Universe::HandleEmpireElimination(int empire_id)
{
    //for (EffectAccountingMap::iterator obj_it = m_effect_accounting_map.begin(); obj_it != m_effect_accounting_map.end(); ++obj_it) {
    //    // ever meter has a value at the start of the turn, and a value after updating with known effects
    //    for (std::map<MeterType, std::vector<EffectAccountingInfo> >::iterator meter_type_it = obj_it->second.begin(); meter_type_it != obj_it->second.end(); ++meter_type_it) {
    //        for (std::size_t i = 0; i < meter_type_it->second.size(); ) {
    //            if (meter_type_it->second[i].caused_by_empire_id == empire_id)
    //                meter_type_it->second.erase(meter_type_it->second.begin() + i);
    //            else
    //                ++i;
    //        }
    //    }
    //}
}

void Universe::InitializeSystemGraph(int for_empire_id)
{
    const ObjectMap& objects = EmpireKnownObjects(for_empire_id);

    for (int i = static_cast<int>(boost::num_vertices(m_graph_impl->system_graph)) - 1; i >= 0; --i) {
        boost::clear_vertex(i, m_graph_impl->system_graph);
        boost::remove_vertex(i, m_graph_impl->system_graph);
    }

    std::vector<int> system_ids = objects.FindObjectIDs<System>();
    Logger().debugStream() << "InitializeSystemGraph(" << for_empire_id << ") system_ids: (" << system_ids.size() << ")";
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

    for (int i = 0; i < static_cast<int>(system_ids.size()); ++i) {
        int system1_id = system_ids[i];
        const System* system1 = objects.Object<System>(system1_id);

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

            typedef boost::graph_traits<GraphImpl::SystemGraph>::edge_descriptor EdgeDescriptor;
            std::pair<EdgeDescriptor, bool> add_edge_result =
                boost::add_edge(i, lane_dest_graph_index, m_graph_impl->system_graph);

            if (add_edge_result.second) {                   // if this is a non-duplicate starlane or wormhole
                if (it->second) {                               // if this is a wormhole
                    edge_weight_map[add_edge_result.first] = 0.1;   // arbitrary small distance
                } else {                                        // if this is a starlane
                    const UniverseObject* system2 = objects.Object(it->first);
                    double x_dist = system2->X() - system1->X();
                    double y_dist = system2->Y() - system1->Y();
                    edge_weight_map[add_edge_result.first] = std::sqrt(x_dist*x_dist + y_dist*y_dist);
                    //std::cout << "edge_weight_map " << system1_id << " to " << lane_dest_id << ": " << edge_weight_map[add_edge_result.first] << std::endl;
                }
            }
        }

        // define the straight-line system distances for this system
        m_system_distances.resize(system_ids.size());
        m_system_distances[i].clear();
        for (int j = 0; j < i; ++j) {
            int system2_id = system_ids[j];
            const UniverseObject* system2 = objects.Object(system2_id);
            double x_dist = system2->X() - system1->X();
            double y_dist = system2->Y() - system1->Y();
            m_system_distances[i].push_back(std::sqrt(x_dist*x_dist + y_dist*y_dist));
            //std::cout << "m_system_distances: " << system1_id << " to " << system2_id << ": " << m_system_distances[i].back() << std::endl;
        }
        m_system_distances[i].push_back(0.0);   // distance from system to itself
    }

    RebuildEmpireViewSystemGraphs(for_empire_id);
}

void Universe::RebuildEmpireViewSystemGraphs(int for_empire_id)
{
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
{
    return s_universe_width;
}

const bool& Universe::UniverseObjectSignalsInhibited()
{
    return s_inhibit_universe_object_signals;
}

void Universe::InhibitUniverseObjectSignals(bool inhibit)
{
    s_inhibit_universe_object_signals = inhibit;
}

void Universe::GetShipDesignsToSerialize(ShipDesignMap& designs_to_serialize, int encoding_empire) const
{
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

void Universe::GetObjectsToSerialize(ObjectMap& objects, int encoding_empire) const
{
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

void Universe::GetEmpireKnownObjectsToSerialize(EmpireObjectMap& empire_latest_known_objects, int encoding_empire) const
{
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

void Universe::GetEmpireObjectVisibilityMap(EmpireObjectVisibilityMap& empire_object_visibility, int encoding_empire) const
{
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

void Universe::GetEmpireObjectVisibilityTurnMap(EmpireObjectVisibilityTurnMap& empire_object_visibility_turns, int encoding_empire) const
{
    if (encoding_empire == ALL_EMPIRES) {
        empire_object_visibility_turns = m_empire_object_visibility_turns;
        return;
    }

    // include just requested empire's visibility turn information
    empire_object_visibility_turns.clear();
    EmpireObjectVisibilityTurnMap::const_iterator it = m_empire_object_visibility_turns.find(encoding_empire);
    if (it != m_empire_object_visibility_turns.end()) {
        empire_object_visibility_turns[encoding_empire] = it->second;
    }
}

void Universe::GetEmpireKnownDestroyedObjects(ObjectKnowledgeMap& empire_known_destroyed_object_ids, int encoding_empire) const
{
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


//////////////////////////////////////////
//    Server-Only General Functions     //
//////////////////////////////////////////
int Universe::GenerateObjectID()
{
#ifdef FREEORION_BUILD_SERVER
    return ++m_last_allocated_object_id;
#else
    throw std::runtime_error("Non-server called Universe::GenerateObjectID");
#endif
}

int Universe::GenerateDesignID()
{
#ifdef FREEORION_BUILD_SERVER
    return ++m_last_allocated_design_id;
#else
    throw std::runtime_error("Non-server called Universe::GenerateDesignID");
#endif
}


//////////////////////////////////////////
//  Server-Only Galaxy Setup Functions  //
//////////////////////////////////////////
namespace {
    const double        MIN_SYSTEM_SEPARATION       = 35.0;                         // in universe units [0.0, s_universe_width]
    const double        MIN_HOME_SYSTEM_SEPARATION  = 200.0;                        // in universe units [0.0, s_universe_width]
    const double        AVG_UNIVERSE_WIDTH          = 1000.0 / std::sqrt(150.0);    // so a 150 star universe is 1000 units across
    const int           ADJACENCY_BOXES             = 25;
    const double        PI                          = 3.141592653589793;
    const int           MAX_SYSTEM_ORBITS           = 9;                            // maximum slots where planets can be
    SmallIntDistType    g_hundred_dist              = SmallIntDist(1, 100);         // a linear distribution [1, 100] used in most universe generation
    const int           MAX_ATTEMPTS_PLACE_SYSTEM   = 100;

    double CalcNewPosNearestNeighbour(const std::pair<double, double>& position,const std::vector<std::pair<double, double> >& positions) {
        if (positions.size() == 0)
            return 0.0;

        unsigned int j;
        double lowest_dist=  (positions[0].first  - position.first ) * (positions[0].first  - position.first ) 
            + (positions[0].second - position.second) * (positions[0].second - position.second),distance=0.0;

        for (j = 1; j < positions.size(); ++j) {
            distance =  (positions[j].first  - position.first ) * (positions[j].first  - position.first ) 
                + (positions[j].second - position.second) * (positions[j].second - position.second);
            if(lowest_dist>distance)
                lowest_dist = distance;
        }
        return lowest_dist;
    }

    void SpiralGalaxyCalcPositions(std::vector<std::pair<double, double> >& positions, unsigned int arms, unsigned int stars, double width, double height) {
        double arm_offset     = RandDouble(0.0,2.0*PI);
        double arm_angle      = 2.0*PI / arms;
        double arm_spread     = 0.3 * PI / arms;
        double arm_length     = 1.5 * PI;
        double center         = 0.25;
        double x,y;

        int i, attempts;

        GaussianDistType  random_gaussian = GaussianDist(0.0,arm_spread);
        SmallIntDistType  random_arm      = SmallIntDist(0  ,arms);
        DoubleDistType    random_angle    = DoubleDist  (0.0,2.0*PI);
        DoubleDistType    random_radius   = DoubleDist  (0.0,  1.0);

        for (i = 0, attempts = 0; i < static_cast<int>(stars) && attempts < MAX_ATTEMPTS_PLACE_SYSTEM; ++i, ++attempts) {
            double radius = random_radius();

            if (radius < center) {
                double angle = random_angle();
                x = radius * cos( arm_offset + angle );
                y = radius * sin( arm_offset + angle );
            } else {
                double arm    = static_cast<double>(random_arm()) * arm_angle;
                double angle  = random_gaussian();

                x = radius * cos( arm_offset + arm + angle + radius * arm_length );
                y = radius * sin( arm_offset + arm + angle + radius * arm_length );
            }

            x = (x + 1) * width / 2.0;
            y = (y + 1) * height / 2.0;

            if (x < 0 || width <= x || y < 0 || height <= y)
                continue;

            // See if new star is too close to any existing star.
            double lowest_dist = CalcNewPosNearestNeighbour(std::pair<double,double>(x,y), positions);

            // If so, we try again.
            if (lowest_dist < MIN_SYSTEM_SEPARATION * MIN_SYSTEM_SEPARATION && attempts < MAX_ATTEMPTS_PLACE_SYSTEM - 1) {
                --i;
                continue;
            }

            // Add the new star location.
            positions.push_back(std::make_pair(x, y));

            // Note that attempts is reset for every star.
            attempts = 0;
        }
    }

    void EllipticalGalaxyCalcPositions(std::vector<std::pair<double,double> >& positions, unsigned int stars, double width, double height) {
        const double ellipse_width_vs_height = RandDouble(0.4, 0.6);
        const double rotation = RandDouble(0.0, PI),
                     rotation_sin = std::sin(rotation),
                     rotation_cos = std::cos(rotation);
        const double gap_constant = .95;
        const double gap_size = 1.0 - gap_constant * gap_constant * gap_constant;

        // Random number generators.
        DoubleDistType radius_dist = DoubleDist(0.0, gap_constant);
        DoubleDistType random_angle  = DoubleDist(0.0, 2.0 * PI);

        // Used to give up when failing to place a star too often.
        int attempts = 0;

        // For each attempt to place a star...
        for (unsigned int i = 0; i < stars && attempts < MAX_ATTEMPTS_PLACE_SYSTEM; ++i, ++attempts){
            double radius = radius_dist();
            // Adjust for bigger density near center and create gap.
            radius = radius * radius * radius + gap_size;
            double angle  = random_angle();

            // Rotate for individual angle and apply elliptical shape.
            double x1 = radius * std::cos(angle);
            double y1 = radius * std::sin(angle) * ellipse_width_vs_height;

            // Rotate for ellipse angle.
            double x = x1 * rotation_cos - y1 * rotation_sin;
            double y = x1 * rotation_sin + y1 * rotation_cos;

            // Move from [-1.0, 1.0] universe coordinates.
            x = (x + 1.0) * width / 2.0;
            y = (y + 1.0) * height / 2.0;

            // Discard stars that are outside boundaries (due to possible rounding errors).
            if (x < 0 || x >= width || y < 0 || y >= height)
                continue;

            // See if new star is too close to any existing star.
            double lowest_dist = CalcNewPosNearestNeighbour(std::pair<double,double>(x, y), positions);

            // If so, we try again.
            if (lowest_dist < MIN_SYSTEM_SEPARATION * MIN_SYSTEM_SEPARATION && attempts < MAX_ATTEMPTS_PLACE_SYSTEM - 1) {
                --i;
                continue;
            }

            // Add the new star location.
            positions.push_back(std::make_pair(x, y));

            // Note that attempts is reset for every star.
            attempts = 0;
        }
    }

    void ClusterGalaxyCalcPositions(std::vector<std::pair<double,double> >& positions, unsigned int clusters, unsigned int stars, double width, double height) {
        assert(clusters);
        assert(stars);

        // probability of systems which don't belong to a cluster
        const double system_noise = 0.15;
        double ellipse_width_vs_height = RandDouble(0.2,0.5);
        // first innermost pair hold cluster position, second innermost pair stores help values for cluster rotation (sin,cos)
        std::vector<std::pair<std::pair<double,double>,std::pair<double,double> > > clusters_position;
        unsigned int i,j,attempts;

        DoubleDistType    random_zero_to_one = DoubleDist  (0.0,  1.0);
        DoubleDistType    random_angle  = DoubleDist  (0.0,2.0*PI);

        for (i = 0, attempts = 0; i < clusters && static_cast<int>(attempts) < MAX_ATTEMPTS_PLACE_SYSTEM; i++ , attempts++) {
            // prevent cluster position near borders (and on border)
            double x = ((random_zero_to_one()*2.0-1.0) /(clusters+1.0))*clusters,
                y = ((random_zero_to_one()*2.0-1.0) /(clusters+1.0))*clusters;


            // ensure all clusters have a min separation to each other (search isn't opimized, not worth the effort)
            for (j = 0; j < clusters_position.size(); j++) {
                if ((clusters_position[j].first.first - x)*(clusters_position[j].first.first - x)+ (clusters_position[j].first.second - y)*(clusters_position[j].first.second - y)
                    < (2.0/clusters))
                    break;
            }
            if (j < clusters_position.size()) {
                i--;
                continue;
            }

            attempts = 0;
            double rotation = RandDouble(0.0,PI);
            clusters_position.push_back(std::pair<std::pair<double,double>,std::pair<double,double> >(std::pair<double,double>(x,y),std::pair<double,double>(sin(rotation),cos(rotation))));
        }

        for (i = 0, attempts = 0; i < stars && attempts<100; i++, attempts++) {
            double x,y;
            if (random_zero_to_one()<system_noise) {
                x = random_zero_to_one() * 2.0 - 1.0;
                y = random_zero_to_one() * 2.0 - 1.0;
            } else {
                short  cluster = i%clusters_position.size();
                double radius  = random_zero_to_one();
                double angle   = random_angle();
                double x1,y1;

                x1 = radius * cos(angle);
                y1 = radius * sin(angle)*ellipse_width_vs_height;

                x = x1*clusters_position[cluster].second.second + y1*clusters_position[cluster].second.first;
                y =-x1*clusters_position[cluster].second.first  + y1*clusters_position[cluster].second.second;

                x = x/sqrt((double)clusters) + clusters_position[cluster].first.first;
                y = y/sqrt((double)clusters) + clusters_position[cluster].first.second;
            }
            x = (x+1)*width /2.0;
            y = (y+1)*height/2.0;

            if (x<0 || width<=x || y<0 || height<=y)
                continue;

            // See if new star is too close to any existing star.
            double lowest_dist=CalcNewPosNearestNeighbour(std::pair<double,double>(x,y),positions);

            // If so, we try again.
            if (lowest_dist < MIN_SYSTEM_SEPARATION * MIN_SYSTEM_SEPARATION && attempts < MAX_ATTEMPTS_PLACE_SYSTEM - 1) {
                --i;
                continue;
            }

            // Add the new star location.
            positions.push_back(std::make_pair(x, y));

            // Note that attempts is reset for every star.
            attempts = 0;
        }
    }

    void RingGalaxyCalcPositions(std::vector<std::pair<double, double> >& positions, unsigned int stars, double width, double height) {
        double RING_WIDTH = width / 4.0;
        double RING_RADIUS = (width - RING_WIDTH) / 2.0;

        DoubleDistType   theta_dist = DoubleDist(0.0, 2.0 * PI);
        GaussianDistType radius_dist = GaussianDist(RING_RADIUS, RING_WIDTH / 3.0);

        for (unsigned int i = 0, attempts = 0; i < stars && static_cast<int>(attempts) < MAX_ATTEMPTS_PLACE_SYSTEM; ++i, ++attempts) {
            double theta = theta_dist();
            double radius = radius_dist();

            double x = width / 2.0 + radius * std::cos(theta);
            double y = height / 2.0 + radius * std::sin(theta);

            if (x < 0 || width <= x || y < 0 || height <= y)
                continue;

            // See if new star is too close to any existing star.
            double lowest_dist=CalcNewPosNearestNeighbour(std::pair<double,double>(x,y),positions);

            // If so, we try again.
            if (lowest_dist < MIN_SYSTEM_SEPARATION * MIN_SYSTEM_SEPARATION && attempts < MAX_ATTEMPTS_PLACE_SYSTEM - 1) {
                --i;
                continue;
            }

            // Add the new star location.
            positions.push_back(std::make_pair(x, y));

            // Note that attempts is reset for every star.
            attempts = 0;
        }
    }

    void IrregularGalaxyPositions(std::vector<std::pair<double, double> >& positions, unsigned int stars, double width, double height) {
        Logger().debugStream() << "IrregularGalaxyPositions";

        unsigned int positions_placed = 0;
        for (unsigned int i = 0, attempts = 0; i < stars && static_cast<int>(attempts) < MAX_ATTEMPTS_PLACE_SYSTEM; ++i, ++attempts) {

            double x = width * RandZeroToOne();
            double y = height * RandZeroToOne();

            Logger().debugStream() << "... potential position: (" << x << ", " << y << ")";

            // reject positions outside of galaxy: minimum 0, maximum height or width.  shouldn't be a problem,
            // but I'm copying this from one of the other generation functions and figure it might as well remain
            if (x < 0 || width <= x || y < 0 || height <= y)
                continue;

            // See if new star is too close to any existing star.
            double lowest_dist = CalcNewPosNearestNeighbour(std::pair<double,double>(x,y), positions);

            // If so, we try again.
            if (lowest_dist < MIN_SYSTEM_SEPARATION * MIN_SYSTEM_SEPARATION && attempts < MAX_ATTEMPTS_PLACE_SYSTEM - 1) {
                --i;
                continue;
            }

            // Add the new star location.
            positions.push_back(std::make_pair(x, y));

            Logger().debugStream() << "... added system at (" << x << ", " << y << ") after " << attempts << " attempts";

            positions_placed++;

            // Note that attempts is reset for every star.
            attempts = 0;
        }
        Logger().debugStream() << "... placed " << positions_placed << " systems";
    }


    System* GenerateSystem(Universe &universe, Age age, double x, double y) {
        //Logger().debugStream() << "GenerateSystem at (" << x << ", " << y << ")";

        const std::vector<int>& base_star_type_dist = UniverseDataTables()["BaseStarTypeDist"][0];
        const std::vector<std::vector<int> >& universe_age_mod_to_star_type_dist = UniverseDataTables()["UniverseAgeModToStarTypeDist"];

        static std::list<std::string> star_names;
        if (star_names.empty())
            LoadSystemNames(star_names);

        // generate new star

        // pick a star type
        StarType star_type = STAR_NONE;
        // make a series of "rolls" (1-100) for each star type, and take the highest modified roll
        int idx = 0;
        int max_roll = 0;
        for (unsigned int i = 0; i < NUM_STAR_TYPES; ++i) {
            int roll = g_hundred_dist() + universe_age_mod_to_star_type_dist[age][i] + base_star_type_dist[i];
            if (max_roll < roll) {
                max_roll = roll;
                idx = i;
            }
        }
        star_type = StarType(idx);

        // pick a name for the system
        std::string star_name;
        if (!star_names.empty()) {
            int star_name_idx = RandSmallInt(0, static_cast<int>(star_names.size()) - 1);
            std::list<std::string>::iterator it = star_names.begin();
            std::advance(it, star_name_idx);
            star_name = *it;
            // erase chosen name from list, to avoid duplicates
            star_names.erase(it);
        }

        // create new system
        System* system = new System(star_type, MAX_SYSTEM_ORBITS, star_name, x, y);

        int new_system_id = universe.Insert(system);
        if (new_system_id == UniverseObject::INVALID_OBJECT_ID) {
            throw std::runtime_error("Universe::GenerateSystem() : Attempt to insert system into the object map failed.");
        }

        return system;
    }

    void GenerateStarField(Universe &universe, Age age, const std::vector<std::pair<double, double> >& positions, 
                           Universe::AdjacencyGrid& adjacency_grid, double adjacency_box_size)
    {
        Logger().debugStream() << "GenerateStarField with " << positions.size() << " positions";
        // generate star field
        for (unsigned int star_cnt = 0; star_cnt < positions.size(); ++star_cnt) {
            System* system = GenerateSystem(universe, age, positions[star_cnt].first, positions[star_cnt].second);
            adjacency_grid[static_cast<int>(system->X() / adjacency_box_size)]
                [static_cast<int>(system->Y() / adjacency_box_size)].insert(system);
        }
    }

    void GetNeighbors(double x, double y, const Universe::AdjacencyGrid& adjacency_grid, std::set<System*>& neighbors) {
        const double ADJACENCY_BOX_SIZE = Universe::UniverseWidth() / ADJACENCY_BOXES;
        std::pair<unsigned int, unsigned int> grid_box(static_cast<unsigned int>(x / ADJACENCY_BOX_SIZE),
                                                       static_cast<unsigned int>(y / ADJACENCY_BOX_SIZE));

        // look in the box into which this system falls, and those boxes immediately around that box
        neighbors = adjacency_grid[grid_box.first][grid_box.second];

        if (0 < grid_box.first) {
            if (0 < grid_box.second) {
                const std::set<System*>& grid_square = adjacency_grid[grid_box.first - 1][grid_box.second - 1];
                neighbors.insert(grid_square.begin(), grid_square.end());
            }
            const std::set<System*>& grid_square = adjacency_grid[grid_box.first - 1][grid_box.second];
            neighbors.insert(grid_square.begin(), grid_square.end());
            if (grid_box.second < adjacency_grid[grid_box.first].size() - 1) {
                const std::set<System*>& grid_square = adjacency_grid[grid_box.first - 1][grid_box.second + 1];
                neighbors.insert(grid_square.begin(), grid_square.end());
            }
        }
        if (0 < grid_box.second) {
            const std::set<System*>& grid_square = adjacency_grid[grid_box.first][grid_box.second - 1];
            neighbors.insert(grid_square.begin(), grid_square.end());
        }
        if (grid_box.second < adjacency_grid[grid_box.first].size() - 1) {
            const std::set<System*>& grid_square = adjacency_grid[grid_box.first][grid_box.second + 1];
            neighbors.insert(grid_square.begin(), grid_square.end());
        }

        if (grid_box.first < adjacency_grid.size() - 1) {
            if (0 < grid_box.second) {
                const std::set<System*>& grid_square = adjacency_grid[grid_box.first + 1][grid_box.second - 1];
                neighbors.insert(grid_square.begin(), grid_square.end());
            }
            const std::set<System*>& grid_square = adjacency_grid[grid_box.first + 1][grid_box.second];
            neighbors.insert(grid_square.begin(), grid_square.end());
            if (grid_box.second < adjacency_grid[grid_box.first].size() - 1) {
                const std::set<System*>& grid_square = adjacency_grid[grid_box.first + 1][grid_box.second + 1];
                neighbors.insert(grid_square.begin(), grid_square.end());
            }
        }
    }
}

namespace Delauney {
    /** simple 2D point.  would have used array of systems, but System
      * class has limits on the range of positions that would interfere
      * with the triangulation algorithm (need a single large covering
      * triangle that overlaps all actual points being triangulated) */
    class DTPoint {
    public:
        DTPoint() :
            x(0.0),
            y(0.0)
        {}
        DTPoint(double x_, double y_) :
            x(x_),
            y(y_)
        {}
        double  x;
        double  y;
    };

    /* simple class for an integer that has an associated "sorting value",
     * so the integer can be stored in a list sorted by something other than
     * the value of the integer */
    class SortValInt {
    public:
        SortValInt(int num_, double sort_val_) :
            num(num_),
            sortVal(sort_val_)
        {}
        int     num;
        double  sortVal;
    };


    /** list of three interger array indices, and some additional info about
      * the triangle that the corresponding points make up, such as the
      * circumcentre and radius, and a function to find if another point is in
      * the circumcircle */
    class DTTriangle {
    public:
        DTTriangle();
        DTTriangle(int vert1, int vert2, int vert3, std::vector<Delauney::DTPoint> &points);

        bool                    PointInCircumCircle(Delauney::DTPoint &p);  ///< determines whether a specified point is within the circumcircle of the triangle
        const std::vector<int>& Verts() {return verts;}

    private:
        std::vector<int>    verts;      ///< indices of vertices of triangle
        Delauney::DTPoint   centre;     ///< location of circumcentre of triangle
        double              radius2;    ///< radius of circumcircle squared
    };

    DTTriangle::DTTriangle(int vert1, int vert2, int vert3, std::vector<Delauney::DTPoint>& points) {
        double a, Sx, Sy, b;
        double x1, x2, x3, y1, y2, y3;

        if (vert1 == vert2 || vert1 == vert3 || vert2 == vert3)
            throw std::runtime_error("Attempted to create Triangle with two of the same vertex indices.");

        verts = std::vector<int>(3);

        // record indices of vertices of triangle
        verts[0] = vert1;
        verts[1] = vert2;
        verts[2] = vert3;

        // extract position info for vertices
        x1 = points[vert1].x;
        x2 = points[vert2].x;
        x3 = points[vert3].x;
        y1 = points[vert1].y;
        y2 = points[vert2].y;
        y3 = points[vert3].y;

        // calculate circumcircle and circumcentre of triangle
        a = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);

        Sx = 0.5 * ((x1 * x1 + y1 * y1) * (y2 - y3) +
                    (x2 * x2 + y2 * y2) * (y3 - y1) +
                    (x3 * x3 + y3 * y3) * (y1 - y2));

        Sy = -0.5* ((x1 * x1 + y1 * y1) * (x2 - x3) +
                    (x2 * x2 + y2 * y2) * (x3 - x1) +
                    (x3 * x3 + y3 * y3) * (x1 - x2));

        b =        ((x1 * x1 + y1 * y1) * (x2 * y3 - x3 * y2) +
                    (x2 * x2 + y2 * y2) * (x3 * y1 - x1 * y3) +
                    (x3 * x3 + y3 * y3) * (x1 * y2 - x2 * y1));

        // make sure nothing funky's going on...
        if (std::abs(a) < 0.01)
            throw std::runtime_error("Attempted to find circumcircle for a triangle with vertices in a line.");

        // finish!
        centre.x = Sx / a;
        centre.y = Sy / a;
        radius2 = (Sx*Sx + Sy*Sy)/(a*a) + b/a;
    };

    DTTriangle::DTTriangle() :
        verts(3, 0),
        centre(0.0, 0.0),
        radius2(0.0)
    {};

    bool DTTriangle::PointInCircumCircle(Delauney::DTPoint &p) {
        double vectX, vectY;

        vectX = p.x - centre.x;
        vectY = p.y - centre.y;

        if (vectX*vectX + vectY*vectY < radius2)
            return true;
        return false;
    };



    /** runs a Delauney Triangulation routine on a set of 2D points extracted
      * from an array of systems returns the list of triangles produced */
    std::list<Delauney::DTTriangle>* DelauneyTriangulate(std::vector<System*> &systems) {

        int n, c, theSize, num, num2; // loop counters, storage for retreived size of a vector, temp storage
        std::list<Delauney::DTTriangle>::iterator itCur, itEnd;
        std::list<Delauney::SortValInt>::iterator itCur2, itEnd2;
        // vector of x and y positions of stars
        std::vector<Delauney::DTPoint> points;
        // pointer to main list of triangles algorithm works with.
        std::list<Delauney::DTTriangle> *triList;
        // list of indices in vector of points extracted from removed triangles that need to be retriangulated
        std::list<Delauney::SortValInt> pointNumList;
        double vx, vy, mag;  // vector components, magnitude

        // ensure a useful list of systems was passed...
        if (systems.empty())
            throw std::runtime_error("Attempted to run Delauney Triangulation on empty array of systems");

        // extract systems positions, and store in vector.  Can't use actual systems data since
        // systems have position limitations which would interfere with algorithm
        theSize = static_cast<int>(systems.size());
        for (n = 0; n < theSize; n++) {
            points.push_back(Delauney::DTPoint(systems[n]->X(), systems[n]->Y()));
        }

        // add points for covering triangle.  the point positions should be big enough to form a triangle
        // that encloses all the systems of the galaxy (or at least one whose circumcircle covers all points)
        points.push_back(Delauney::DTPoint(-1.0, -1.0));
        points.push_back(Delauney::DTPoint(2.0 * (Universe::UniverseWidth() + 1.0), -1.0));
        points.push_back(Delauney::DTPoint(-1.0, 2.0 * (Universe::UniverseWidth() + 1.0)));

        // initialize triList.  algorithm adds and removes triangles from this list, and the resulting
        // list is returned (so should be deleted externally)
        triList = new std::list<Delauney::DTTriangle>;

        // add last three points into the first triangle, the "covering triangle"
        theSize = static_cast<int>(points.size());
        triList->push_front(Delauney::DTTriangle(theSize-1, theSize-2, theSize-3, points));

        // loop through "real" points (from systems, not the last three added to make the covering triangle)
        for (n = 0; n < theSize - 3; n++) {
            pointNumList.clear();

            // check each triangle in list, to see if the new point lies in its circumcircle.  if so, delete
            // the triangle and add its vertices to a list 
            itCur = triList->begin();
            itEnd = triList->end();
            while (itCur != itEnd) {
                // get current triangle
                Delauney::DTTriangle& tri = *itCur;

                // check if point to be added to triangulation is within the circumcircle for the current triangle
                if (tri.PointInCircumCircle(points[n])) {
                    // if so, insert the triangle's vertices' indices into the list.  add in sorted position
                    // based on angle of direction to current point n being inserted.  don't add if doing
                    // so would duplicate an index already in the list
                    for (c = 0; c < 3; c++) {
                        num = (tri.Verts())[c];  // store "current point"

                        // get sorting value to order points clockwise circumferentially around point n
                        // vector from point n to current point
                        vx = points[num].x - points[n].x;
                        vy = points[num].y - points[n].y;
                        mag = sqrt(vx*vx + vy*vy);
                        // normalize
                        vx /= mag;
                        vy /= mag;
                        // dot product with (0, 1) is vy, magnitude of cross product is vx
                        // this gives a range of "sortValue" from -2 to 2, around the circle
                        if (vx >= 0) mag = vy + 1; else mag = -vy - 1;
                        // sorting value in "mag"

                        // iterate through list, finding insert spot and verifying uniqueness (or add if list is empty)
                        itCur2 = pointNumList.begin();
                        itEnd2 = pointNumList.end();
                        if (itCur2 == itEnd2) {
                            // list is empty
                            pointNumList.push_back(Delauney::SortValInt(num, mag));
                        } else {
                            while (itCur2 != itEnd2) {
                                if (itCur2->num == num) 
                                    break;
                                if (itCur2->sortVal > mag) {
                                    pointNumList.insert(itCur2, Delauney::SortValInt(num, mag));
                                    break;
                                }
                                itCur2++;
                            }
                            if (itCur2 == itEnd2) {
                                // point wasn't added, so should go at end
                                pointNumList.push_back(Delauney::SortValInt(num, mag));
                            }
                        }
                    } // end for c

                    // remove current triangle from list of triangles
                    itCur = triList->erase(itCur);
                } else {
                    // point not in circumcircle for this triangle
                    // to go next triangle in list
                    ++itCur;
                }
            } // end while

            // go through list of points, making new triangles out of them
            itCur2 = pointNumList.begin();
            itEnd2 = pointNumList.end();
            assert(itCur2 != itEnd2);

            // add triangle for last and first points and n
            triList->push_front(Delauney::DTTriangle(n, (pointNumList.front()).num, (pointNumList.back()).num, points));

            num = itCur2->num;
            ++itCur2;
            while (itCur2 != itEnd2) {
                num2 = num;
                num = itCur2->num;

                triList->push_front(Delauney::DTTriangle(n, num2, num, points));

                ++itCur2;
            } // end while

        } // end for
        return triList;
    } // end function
}

namespace {
    struct store_fleet_plan_impl
    {
        template <class T1, class T2>
        struct result {typedef void type;};
        template <class T>
        void operator()(std::vector<FleetPlan>& fleet_plans, const T& fleet_plan) const
        {
            fleet_plans.push_back(fleet_plan);
        }
    };

    const phoenix::function<store_fleet_plan_impl> store_fleet_plan_;

    class FleetPlanManager {
    public:
        typedef std::vector<FleetPlan>::const_iterator iterator;

        /** \name Accessors */ //@{
        /** returns iterator pointing to first plan. */
        iterator    begin() const                           { return m_plans.begin(); }

        /** returns iterator pointing one past last plan. */
        iterator    end() const                             { return m_plans.end(); }
        //@}

        /** returns the instance of this singleton class; you should use the
          * free function GetFleetPlanManager() instead */
        static const FleetPlanManager& GetFleetPlanManager();

    private:
        FleetPlanManager();

        std::vector<FleetPlan>      m_plans;

        static FleetPlanManager*    s_instance;
    };
    // static(s)
    FleetPlanManager* FleetPlanManager::s_instance = 0;

    const FleetPlanManager& FleetPlanManager::GetFleetPlanManager() {
        static FleetPlanManager manager;
        return manager;
    }

    FleetPlanManager::FleetPlanManager() {
        if (s_instance)
            throw std::runtime_error("Attempted to create more than one FleetPlanManager.");

        s_instance = this;

        Logger().debugStream() << "Initializing FleetPlanManager";

        std::string file_name = "starting_fleets.txt";
        std::string input;

        boost::filesystem::ifstream ifs(GetResourceDir() / file_name);
        if (ifs) {
            std::getline(ifs, input, '\0');
            ifs.close();
        } else {
            Logger().errorStream() << "Unable to open data file " << file_name;
            return;
        }

        using namespace boost::spirit::classic;
        using namespace phoenix;
        parse_info<const char*> result =
            parse(input.c_str(),
                  as_lower_d[*fleet_plan_p[store_fleet_plan_(var(m_plans), arg1)]]
                  >> end_p,
                  skip_p);
        if (!result.full)
            ReportError(input.c_str(), result);

#ifdef OUTPUT_PLANS_LIST
        Logger().debugStream() << "Starting Fleet Plans:";
        for (iterator it = begin(); it != end(); ++it) {
            const FleetPlan& p = *it;
            Logger().debugStream() << " ... " << p.Name();
        }
#endif
    }

    /** returns the singleton fleet plan manager */
    const FleetPlanManager& GetFleetPlanManager() {
        return FleetPlanManager::GetFleetPlanManager();
    }
};

namespace {
    struct store_item_spec_impl
    {
        template <class T1, class T2>
        struct result {typedef void type;};
        template <class T>
        void operator()(std::vector<ItemSpec>& item_specs, const T& item_spec) const
        {
            item_specs.push_back(item_spec);
        }
    };

    const phoenix::function<store_item_spec_impl> store_item_spec_;

    class ItemSpecManager {
    public:
        typedef std::vector<ItemSpec>::const_iterator iterator;

        /** \name Accessors */ //@{
        /** returns iterator pointing to first item spec. */
        iterator    begin() const                           { return m_items.begin(); }

        /** returns iterator pointing one past last plan. */
        iterator    end() const                             { return m_items.end(); }
        //@}

        /** Adds unlocked items in this manager to the specified \a empire
          * using that Empire's UnlockItem function. */
        void        AddItemsToEmpire(Empire* empire) const;

        /** returns the instance of this singleton class; you should use the
          * free function GetFleetPlanManager() instead */
        static const ItemSpecManager& GetItemSpecManager();

    private:
        ItemSpecManager();

        std::vector<ItemSpec>   m_items;

        static ItemSpecManager* s_instance;
    };
    // static(s)
    ItemSpecManager* ItemSpecManager::s_instance = 0;

    const ItemSpecManager& ItemSpecManager::GetItemSpecManager() {
        static ItemSpecManager manager;
        return manager;
    }

    ItemSpecManager::ItemSpecManager() {
        if (s_instance)
            throw std::runtime_error("Attempted to create more than one ItemSpecManager.");

        s_instance = this;

        Logger().debugStream() << "Initializing ItemSpecManager";

        std::string file_name = "preunlocked_items.txt";
        std::string input;

        boost::filesystem::ifstream ifs(GetResourceDir() / file_name);
        if (ifs) {
            std::getline(ifs, input, '\0');
            ifs.close();
        } else {
            Logger().errorStream() << "Unable to open data file " << file_name;
            return;
        }

        using namespace boost::spirit::classic;
        using namespace phoenix;
        parse_info<const char*> result =
            parse(input.c_str(),
                  as_lower_d[*item_spec_p[store_item_spec_(var(m_items), arg1)]]
                  >> end_p,
                  skip_p);
        if (!result.full)
            ReportError(input.c_str(), result);

#ifdef OUTPUT_ITEM_SPECS_LIST
        Logger().debugStream() << "Starting Unlocked Item Specs:";
        for (iterator it = begin(); it != end(); ++it) {
            const ItemSpec& item = *it;
            Logger().debugStream() << " ... " << boost::lexical_cast<std::string>(item.type) << " : " << item.name;
        }
#endif
    }

    void ItemSpecManager::AddItemsToEmpire(Empire* empire) const {
        if (!empire)
            return;
        for (iterator it = begin(); it != end(); ++it)
            empire->UnlockItem(*it);
    }

    /** returns the singleton item spec manager */
    const ItemSpecManager& GetItemSpecManager() {
        return ItemSpecManager::GetItemSpecManager();
    }
};

namespace {
    /** Used by GenerateStarlanes.  Determines if two systems are connected by
      * maxLaneJumps or less edges on graph. */
    bool ConnectedWithin(int system1, int system2, int maxLaneJumps, std::vector<std::set<int> >& laneSetArray) {
        // list of indices of systems that are accessible from previously visited systems.
        // when a new system is found to be accessible, it is added to the back of the
        // list.  the list is iterated through from front to back to find systems
        // to examine
        std::list<int> accessibleSystemsList;
        std::list<int>::iterator sysListIter, sysListEnd;

        // map using star index number as the key, and also storing the number of starlane
        // jumps away from system1 a given system is.  this is used to determine if a
        // system has already been added to the accessibleSystemsList without needing
        // to iterate through the list.  it also provides some indication of the
        // current depth of the search, which allows the serch to terminate after searching
        // to the depth of maxLaneJumps without finding system2
        // (considered using a vector for this, but felt that for large galaxies, the
        // size of the vector and the time to intialize would be too much)
        std::map<int, int> accessibleSystemsMap;

        // system currently being investigated, destination of a starlane origination at curSys
        int curSys, curLaneDest;
        // "depth" level in tree of system currently being investigated
        int curDepth;

        // iterators to set of starlanes, in graph, for the current system    
        std::set<int>::iterator curSysLanesSetIter, curSysLanesSetEnd;

        // check for simple cases for quick termination
        if (system1 == system2) return true; // system is always connected to itself
        if (0 == maxLaneJumps) return false; // no system is connected to any other system by less than 1 jump
        if (0 == (laneSetArray[system1]).size()) return false; // no lanes out of start system
        if (0 == (laneSetArray[system2]).size()) return false; // no lanes into destination system
        if (system1 >= static_cast<int>(laneSetArray.size()) || system2 >= static_cast<int>(laneSetArray.size())) return false; // out of range
        if (system1 < 0 || system2 < 0) return false; // out of range

        // add starting system to list and set of accessible systems
        accessibleSystemsList.push_back(system1);
        accessibleSystemsMap.insert(std::pair<int, int>(system1, 0));

        // loop through visited systems
        sysListIter = accessibleSystemsList.begin();
        sysListEnd = accessibleSystemsList.end();
        while (sysListIter != sysListEnd) {
            curSys = *sysListIter;

            // check that iteration hasn't reached maxLaneJumps levels deep, which would 
            // mean that system2 isn't within maxLaneJumps starlane jumps of system1
            curDepth = (*accessibleSystemsMap.find(curSys)).second;

            if (curDepth >= maxLaneJumps) return false;

            // get set of starlanes for this system
            curSysLanesSetIter = (laneSetArray[curSys]).begin();
            curSysLanesSetEnd = (laneSetArray[curSys]).end();

            // add starlanes accessible from this system to list and set of accessible starlanes
            // (and check for the goal starlane)
            while (curSysLanesSetIter != curSysLanesSetEnd) {
                curLaneDest = *curSysLanesSetIter;

                // check if curLaneDest has been added to the map of accessible systems
                if (0 == accessibleSystemsMap.count(curLaneDest)) {

                    // check for goal
                    if (curLaneDest == system2) return true;

                    // add curLaneDest to accessible systems list and map
                    accessibleSystemsList.push_back(curLaneDest);
                    accessibleSystemsMap.insert(std::pair<int, int>(curLaneDest, curDepth + 1));
                   }

                curSysLanesSetIter++;
            }

            sysListIter++;
        }
        return false; // default
    }

    /** Removes lanes from passed graph that are angularly too close to
      * each other. */
    void CullAngularlyTooCloseLanes(double maxLaneUVectDotProd, std::vector<std::set<int> >& laneSetArray, std::vector<System*> &systems) {
        // start and end systems of a new lane being considered, and end points of lanes that already exist with that
        // start at the start or destination of the new lane
        int curSys, dest1, dest2;

        // geometry stuff... points componenets, vector componenets dot product & magnitudes of vectors
        double startX, startY, vectX1, vectX2, vectY1, vectY2, dotProd, mag1, mag2;
        // 2 component vector and vect + magnitude typedefs

        typedef std::pair<double, double> VectTypeQQ;
        typedef std::pair<VectTypeQQ, double> VectAndMagTypeQQ;
        typedef std::pair<int, VectAndMagTypeQQ> MapInsertableTypeQQ;

        std::map<int, VectAndMagTypeQQ> laneVectsMap;  // componenets of vectors of lanes of current system, indexed by destination system number
        std::map<int, VectAndMagTypeQQ>::iterator laneVectsMapIter;

        VectTypeQQ tempVect;
        VectAndMagTypeQQ tempVectAndMag;

        // iterators to go through sets of lanes in array
        std::set<int>::iterator laneSetIter1, laneSetIter2, laneSetEnd;

        std::set<std::pair<int, int> > lanesToRemoveSet;  // start and end stars of lanes to be removed in final step...
        std::set<std::pair<int, int> >::iterator lanesToRemoveIter, lanesToRemoveEnd;
        std::pair<int, int> lane1, lane2;

        int curNumLanes;

        int numSys = systems.size();
        // make sure data is consistent
        if (static_cast<int>(laneSetArray.size()) != numSys) {
            Logger().errorStream() << "CullAngularlyTooCloseLanes got different size vectors of lane sets and systems.  Doing nothing.";
            return;
        }
        
        if (numSys < 3) return;  // nothing worth doing for less than three systems

        //Logger().debugStream() << "Culling Too Close Angularly Lanes";

        // loop through systems
        for (curSys = 0; curSys < numSys; curSys++) {
            // get position of current system (for use in calculated vectors)
            startX = systems[curSys]->X();
            startY = systems[curSys]->Y();

            // get number of starlanes current system has
            curNumLanes = laneSetArray[curSys].size();

            // can't have pairs of lanes with less than two lanes...
            if (curNumLanes > 1) {

                // remove any old lane Vector Data
                laneVectsMap.clear();

                // get unit vectors for all lanes of this system
                laneSetIter1 = laneSetArray[curSys].begin();
                laneSetEnd = laneSetArray[curSys].end();
                while (laneSetIter1 != laneSetEnd) {
                    // get destination for this lane
                    dest1 = *laneSetIter1;
                    // get vector to this lane destination
                    vectX1 = systems[dest1]->X() - startX;
                    vectY1 = systems[dest1]->Y() - startY;
                    // normalize
                    mag1 = std::sqrt(vectX1 * vectX1 + vectY1 * vectY1);
                    vectX1 /= mag1;
                    vectY1 /= mag1;

                    // store lane in map of lane vectors
                    tempVect = VectTypeQQ(vectX1, vectY1);
                    tempVectAndMag = VectAndMagTypeQQ(tempVect, mag1);
                    laneVectsMap.insert( MapInsertableTypeQQ(dest1, tempVectAndMag) );

                    laneSetIter1++;
                }

                // iterate through lanes of curSys
                laneSetIter1 = laneSetArray[curSys].begin();
                laneSetIter1++;  // start at second, since iterators are used in pairs, and starting both at the first wouldn't be a valid pair
                while (laneSetIter1 != laneSetEnd) {
                    // get destination of current starlane
                    dest1 = *laneSetIter1;

                    if (curSys < dest1) 
                        lane1 = std::pair<int, int>(curSys, dest1);
                    else
                        lane1 = std::pair<int, int>(dest1, curSys);

                    // check if this lane has already been added to the set of lanes to remove
                    if (0 == lanesToRemoveSet.count(lane1)) {

                        // extract data on starlane vector...
                        laneVectsMapIter = laneVectsMap.find(dest1);
                        assert(laneVectsMapIter != laneVectsMap.end());
                        tempVectAndMag = laneVectsMapIter->second;
                        tempVect = tempVectAndMag.first;
                        vectX1 = tempVect.first;
                        vectY1 = tempVect.second;
                        mag1 = tempVectAndMag.second;

                        // iterate through other lanes of curSys, in order to get all possible pairs of lanes
                        laneSetIter2 = laneSetArray[curSys].begin();
                        while (laneSetIter2 != laneSetIter1) {
                            dest2 = *laneSetIter2;

                            if (curSys < dest2) 
                                lane2 = std::pair<int, int>(curSys, dest2);
                            else
                                lane2 = std::pair<int, int>(dest2, curSys);

                            // check if this lane has already been added to the set of lanes to remove
                            if (0 == lanesToRemoveSet.count(lane2)) {

                                // extract data on starlane vector...
                                laneVectsMapIter = laneVectsMap.find(dest2);
                                assert(laneVectsMapIter != laneVectsMap.end());
                                tempVectAndMag = laneVectsMapIter->second;
                                tempVect = tempVectAndMag.first;
                                vectX2 = tempVect.first;
                                vectY2 = tempVect.second;
                                mag2 = tempVectAndMag.second;

                                // find dot product
                                dotProd = vectX1 * vectX2 + vectY1 * vectY2;

                                // if dotProd is big enough, then lanes are too close angularly
                                // thus one needs to be removed.
                                if (dotProd > maxLaneUVectDotProd) {

                                     // preferentially remove the longer lane
                                    if (mag1 > mag2) {
                                        lanesToRemoveSet.insert(lane1);
                                        break;  // don't need to check any more lanes against lane1, since lane1 has been removed
                                    }
                                    else {
                                        lanesToRemoveSet.insert(lane2);
                                    }
                                }
                            }

                            laneSetIter2++;
                        }
                    }

                    laneSetIter1++;
                }
            }
        }

        // iterate through set of lanes to remove, and remove them in turn...
        lanesToRemoveIter = lanesToRemoveSet.begin();
        lanesToRemoveEnd = lanesToRemoveSet.end();
        while (lanesToRemoveIter != lanesToRemoveEnd) {
            lane1 = *lanesToRemoveIter;

            laneSetArray[lane1.first].erase(lane1.second);
            laneSetArray[lane1.second].erase(lane1.first);

            // check that removing lane hasn't disconnected systems
            if (!ConnectedWithin(lane1.first, lane1.second, numSys, laneSetArray)) {
                // they aren't connected... reconnect them
                laneSetArray[lane1.first].insert(lane1.second);
                laneSetArray[lane1.second].insert(lane1.first);
            }

            lanesToRemoveIter++;
        }
    }

    /** Removes lanes from passed graph that are angularly too close to
      * each other. */
    void CullTooLongLanes(double maxLaneLength, std::vector<std::set<int> >& laneSetArray, std::vector<System*> &systems)
    {
        // start and end systems of a new lane being considered, and end points of lanes that already exist with that start
        // at the start or destination of the new lane
        int curSys, dest;

        // geometry stuff... points components, vector componenets
        double startX, startY, vectX, vectY;

        // iterators to go through sets of lanes in array
        std::set<int>::iterator laneSetIter, laneSetEnd;

        // map, indexed by lane length, of start and end stars of lanes to be removed
        std::multimap<double, std::pair<int, int>, std::greater<double> > lanesToRemoveMap;
        std::multimap<double, std::pair<int, int>, std::greater<double> >::iterator lanesToRemoveIter, lanesToRemoveEnd;
        std::pair<int, int> lane;
        typedef std::pair<double, std::pair<int, int> > MapInsertableTypeQQ;

        int numSys = systems.size();
        // make sure data is consistent
        if (static_cast<int>(laneSetArray.size()) != numSys) {
            return;
        }

        if (numSys < 2) return;  // nothing worth doing for less than two systems (no lanes!)

        // get squared max lane lenth, so as to eliminate the need to take square roots of lane lenths...
        double maxLaneLength2 = maxLaneLength*maxLaneLength;

        // loop through systems
        for (curSys = 0; curSys < numSys; curSys++) {
            // get position of current system (for use in calculating vector)
            startX = systems[curSys]->X();
            startY = systems[curSys]->Y();

            // iterate through all lanes in system, checking lengths and marking to be removed if necessary
            laneSetIter = laneSetArray[curSys].begin();
            laneSetEnd = laneSetArray[curSys].end();
            while (laneSetIter != laneSetEnd) {
                // get destination for this lane
                dest = *laneSetIter;
                // convert start and end into ordered pair to represent lane
                if (curSys < dest) 
                    lane = std::pair<int, int>(curSys, dest);
                else
                    lane = std::pair<int, int>(dest, curSys);

                // get vector to this lane destination
                vectX = systems[dest]->X() - startX;
                vectY = systems[dest]->Y() - startY;

                // compare magnitude of vector to max allowed
                double laneLength2 = vectX*vectX + vectY*vectY;
                if (laneLength2 > maxLaneLength2) {
                    // lane is too long!  mark it to be removed
                    lanesToRemoveMap.insert( MapInsertableTypeQQ(laneLength2, lane) );
                } 

                laneSetIter++;
            }
        }

        // Iterate through set of lanes to remove, and remove them in turn.  Since lanes were inserted in the map indexed by
        // their length, iteration starting with begin starts with the longest lane first, then moves through the lanes as
        // they get shorter, ensuring that the longest lanes are removed first.
        lanesToRemoveIter = lanesToRemoveMap.begin();
        lanesToRemoveEnd = lanesToRemoveMap.end();
        while (lanesToRemoveIter != lanesToRemoveEnd) {
            lane = lanesToRemoveIter->second;

            // ensure the lane still exists
            if (laneSetArray[lane.first].count(lane.second) > 0 &&
                laneSetArray[lane.second].count(lane.first) > 0) {

                // remove lane
                laneSetArray[lane.first].erase(lane.second);
                laneSetArray[lane.second].erase(lane.first);

                // if removing lane has disconnected systems, reconnect them
                if (!ConnectedWithin(lane.first, lane.second, numSys, laneSetArray)) {
                    laneSetArray[lane.first].insert(lane.second);
                    laneSetArray[lane.second].insert(lane.first);
                }
            }
            lanesToRemoveIter++;
        }
    }

    /** Grows trees to connect stars...  takes an array of sets of potential
      * starlanes for each star, and puts the starlanes of the tree into
      * another set. */
    void GrowSpanningTrees(std::vector<int> roots, std::vector<std::set<int> >& potentialLaneSetArray, std::vector<std::set<int> >& laneSetArray) {
        // array to keep track of whether a given system (index #) has been connected to by growing tree algorithm
        std::vector<int> treeOfSystemArray; // which growing tree a particular system has been assigned to

        //  map index by tree number, containing a list for each tree, each of which contains the systems in a particular tree
        std::map<int, std::list<int> > treeSysListsMap;
        std::map<int, std::list<int> >::iterator treeSysListsMapIter, treeSysListsMapEnd;
        std::pair<int, std::list<int> > mapInsertable;
        std::list<int> treeSysList, *pTreeSysList, *pTreeToMergeSysList;
        std::list<int>::iterator sysListIter;
        std::set<int>::iterator lanesSetIter, lanesSetEnd;

        int n, q, d, curTree, destTree, curSys, destSys, mergeSys;

        int numSys = potentialLaneSetArray.size();
        int numTrees = roots.size();

        // number of new connections to make from each connected node that is processed.  
        // could be made a parameter, possibly a function of the starlane frequency

        // make sure data is consistent
        if (static_cast<int>(laneSetArray.size()) != numSys) {
            Logger().errorStream() << "GrowSpanningTrees got different size vectors of potential lane set(s) and systems.  Doing nothing.";
            return;
        }
        if ((numTrees < 1) || (numTrees > numSys)) {
            Logger().errorStream() << "GrowSpanningTrees was asked to grow too many or too few trees simultaneously.  Doing nothing.";
            return;
        }
        if (static_cast<int>(roots.size()) > numSys) {
            Logger().errorStream() << "GrowSpanningTrees was asked to grow more separate trees than there are systems to grow from.  Doing nothing.";
            return;
        }

        laneSetArray.resize(numSys);

        // set up data structures...
        treeOfSystemArray.resize(numSys);
        for (n = 0; n < numSys; n++) 
            treeOfSystemArray[n] = -1;  // sentinel value for not connected to any tree

        treeSysListsMap.clear();
        for (n = 0; n < numTrees; n++) {
            // check that next root is within valid range...
            q = roots[n];
            if ((q >= numSys) || (q < 0)) {
                //Logger().debugStream() << "GrowSpanningTrees was asked to grow to grow a tree from a system that doesn't exist.";
                return;
            }

            // make new tree to put into map
            treeSysList.clear();        
            treeSysList.push_front(q);

            // put new list into into map (for tree n), indexed by tree number
            mapInsertable = std::pair<int, std::list<int> >(n, treeSysList);
            treeSysListsMap.insert(mapInsertable);

            // record the tree to which root system of tree n, roots[n], belongs (tree n)
            treeOfSystemArray[q] = n;
        }

        //Logger().debugStream() << "Growing Trees Algorithm Starting...";

        // loop through map (indexed by tree number) of lists of systems, until map (and all lists) are empty...
        treeSysListsMapIter = treeSysListsMap.begin();
        treeSysListsMapEnd = treeSysListsMap.end();
        while (treeSysListsMapIter != treeSysListsMapEnd) {
            // extract number and list of tree
            curTree = treeSysListsMapIter->first;
            pTreeSysList = &(treeSysListsMapIter->second);

            if (pTreeSysList->empty()) {
                // no systems left for tree to grow.  Remove it from map of growing trees.
                treeSysListsMap.erase(curTree);
                //Logger().debugStream() << "Tree " << curTree << " was empty, so was removed from map of trees.";

                // check if set is empty...
                if (treeSysListsMap.empty()) break;  // and stop loop if it is
                // (iterator invalidated by erasing, so set to first tree remaining in map)
                treeSysListsMapIter = treeSysListsMap.begin();
            }
            else {
                //Logger().debugStream() << "Tree " << curTree << " contains " << pTreeSysList->size() << " systems.";
                // tree has systems left to grow.

                // extract and remove a random system from the list

                // iterate to the position of the random system
                sysListIter = pTreeSysList->begin();
                for (d = RandSmallInt(0, pTreeSysList->size() - 1); d > 0; --d) // RandSmallInt(int min, int max);
                    sysListIter++;

                curSys = *sysListIter; // extract
                pTreeSysList->erase(sysListIter); // erase

                //Logger().debugStream() << "Processing system " << curSys << " from tree " << curTree;

                // iterate through list of potential lanes for current system
                lanesSetIter = potentialLaneSetArray[curSys].begin();
                lanesSetEnd = potentialLaneSetArray[curSys].end();
                while (lanesSetIter != lanesSetEnd) {
                    // get destination system of potential lane
                    destSys = *lanesSetIter;

                    // get which, if any, tree the destination system belongs to currently
                    destTree = treeOfSystemArray[destSys];

                    //Logger().debugStream() << "Considering lane from system " << curSys << " to system " << destSys << " of tree " << destTree;

                    // check if the destination system already belongs to the current tree.
                    if (curTree != destTree) {
                        // destination system is either in no tree, or is in a tree other than the current tree

                        // add lane between current and destination systems
                        laneSetArray[curSys].insert(destSys);
                        laneSetArray[destSys].insert(curSys);

                        // mark destination system as part of this tree
                        treeOfSystemArray[destSys] = curTree;

                        //Logger().debugStream() << "Added lane from " << curSys << " to " << destSys << ", and added " << destSys << " to list of systems to process in tree " << curTree;
                    }
                    //else
                    //    Logger().debugStream() << "Both systems were already part of the same tree, so no lane was added";

                    // check what, if any, tree the destination system was before being added to the current tree
                    if (-1 == destTree) {
                        // destination system was not yet part of any tree.
                        // add system to list of systems to consider for this tree in future
                        pTreeSysList->push_back(destSys);

                        //Logger().debugStream() << "System was not yet part of an tree, so was added to the list of systems to process for tree " << curTree;
                    }
                    else if (destTree != curTree) {
                        // tree was already part of another tree
                        // merge the two trees.

                        //Logger().debugStream() << "Merging tree " << destTree << " into current tree " << curTree;

                        pTreeToMergeSysList = &((treeSysListsMap.find(destTree))->second);

                        //Logger().debugStream() << "...got pointer to systems list for tree to merge into current tree";
                        //Logger().debugStream() << "...list to merge has " << pTreeToMergeSysList->size() << " systems.";

                        // extract systems from tree to be merged into current tree
                        while (!pTreeToMergeSysList->empty()) {
                            // get system from list
                            mergeSys = pTreeToMergeSysList->front();
                            pTreeToMergeSysList->pop_front();
                            // add to current list
                            pTreeSysList->push_back(mergeSys);

                            //Logger().debugStream() << "Adding system " << mergeSys << " to current tree " << curTree << " from old tree " << destTree;
                        }

                        // reassign all systems from destination tree to current tree (gets systems even after they're removed
                        // from the list of systems for the dest tree)
                        for (q = 0; q < numSys; q++) 
                            if (treeOfSystemArray[q] == destTree)
                                treeOfSystemArray[q] = curTree;

                        treeSysListsMap.erase(destTree);
                    }

                    lanesSetIter++;
                }
            }

            //Logger().debugStream() << "Moving to next tree...";

            treeSysListsMapIter++;
            treeSysListsMapEnd = treeSysListsMap.end();  // incase deleting or merging trees messed things up
            if (treeSysListsMapIter == treeSysListsMapEnd)
                treeSysListsMapIter = treeSysListsMap.begin();
        }
    }

    /** Set active meter current values equal to target/max meter current
      * values.  Useful when creating new object after applying effects. */
    void SetActiveMetersToTargetMaxCurrentValues(ObjectMap& object_map) {
        std::map<MeterType, MeterType> meters;
        meters[METER_POPULATION] = METER_TARGET_POPULATION;
        meters[METER_HEALTH] = METER_TARGET_HEALTH;
        meters[METER_FARMING] = METER_TARGET_FARMING;
        meters[METER_INDUSTRY] = METER_TARGET_INDUSTRY;
        meters[METER_RESEARCH] = METER_TARGET_RESEARCH;
        meters[METER_TRADE] = METER_TARGET_TRADE;
        meters[METER_MINING] = METER_TARGET_MINING;
        meters[METER_CONSTRUCTION] = METER_TARGET_CONSTRUCTION;
        meters[METER_FUEL] = METER_MAX_FUEL;
        meters[METER_SHIELD] = METER_MAX_SHIELD;
        meters[METER_STRUCTURE] = METER_MAX_STRUCTURE;
        meters[METER_DEFENSE] = METER_MAX_DEFENSE;

        // check for each pair of meter types.  if both exist, set active
        // meter current value equal to target meter current value.
        for (ObjectMap::iterator it = object_map.begin(); it != object_map.end(); ++it) {
            UniverseObject* obj = it->second;

            for (std::map<MeterType, MeterType>::const_iterator meter_it = meters.begin(); meter_it != meters.end(); ++meter_it)
                if (!obj->Owners().empty())
                    if (Meter* meter = obj->GetMeter(meter_it->first))
                        if (Meter* targetmax_meter = obj->GetMeter(meter_it->second))
                            meter->SetCurrent(targetmax_meter->Current());
        }
    }
}

void Universe::CreateUniverse(int size, Shape shape, Age age, StarlaneFrequency starlane_freq, PlanetDensity planet_density,
                              SpecialsFrequency specials_freq, const std::map<int, PlayerSetupData>& player_setup_data)
{
#ifdef FREEORION_RELEASE
    ClockSeed();
#endif

    m_objects.Clear();  // wipe out anything present in the object map

    // these happen to be equal to INVALID_OBJECT_ID and INVALID_DESIGN_ID,
    // but the point here is that the latest used ID is incremented before
    // being assigned, so using -1 here means the first assigned ID will be 0,
    // which is a valid ID
    m_last_allocated_object_id = -1;
    m_last_allocated_design_id = -1;

    int total_players = player_setup_data.size();


    // ensure there are enough systems to give all players adequately-separated homeworlds
    const int MIN_SYSTEMS_PER_PLAYER = 3;
    if (size < total_players*MIN_SYSTEMS_PER_PLAYER) {
        Logger().debugStream() << "Universe creation requested with " << size << " systems, but this is too few for " << total_players << " players.  Creating a universe with " << total_players*MIN_SYSTEMS_PER_PLAYER << " systems instead";
        size = total_players * MIN_SYSTEMS_PER_PLAYER;
    }

    Logger().debugStream() << "Creating universe with " << size << " stars and " << total_players << " players";

    std::vector<int> homeworld_planet_ids;

    // a grid of ADJACENCY_BOXES x ADJACENCY_BOXES boxes to hold the positions of the systems as they are generated,
    // in order to ensure that they get spaced out properly
    AdjacencyGrid adjacency_grid(ADJACENCY_BOXES, std::vector<std::set<System*> >(ADJACENCY_BOXES));

    s_universe_width = std::sqrt(static_cast<double>(size)) * AVG_UNIVERSE_WIDTH;

    std::vector<std::pair<double, double> > positions;

    // generate the stars
    switch (shape) {
    case SPIRAL_2:
    case SPIRAL_3:
    case SPIRAL_4:
        SpiralGalaxyCalcPositions(positions, 2 + (shape - SPIRAL_2), size, s_universe_width, s_universe_width);
        break;
    case CLUSTER: {
        int average_clusters = size / 20; // chosen so that a "typical" size of 100 yields about 5 clusters
        if (!average_clusters)
            average_clusters = 2;
        int clusters = RandSmallInt(average_clusters * 8 / 10, average_clusters * 12 / 10); // +/- 20%
        ClusterGalaxyCalcPositions(positions, clusters, size, s_universe_width, s_universe_width);
        break;
    }
    case ELLIPTICAL:
        EllipticalGalaxyCalcPositions(positions, size, s_universe_width, s_universe_width);
        break;
    case IRREGULAR:
        IrregularGalaxyPositions(positions, size, s_universe_width, s_universe_width);
        break;
    case RING:
        RingGalaxyCalcPositions(positions, size, s_universe_width, s_universe_width);
        break;
    default:
        Logger().errorStream() << "Universe::Universe : Unknown galaxy shape: " << shape << ".  Using IRREGULAR as default.";
        IrregularGalaxyPositions(positions, size, s_universe_width, s_universe_width);
    }
    GenerateStarField(*this, age, positions, adjacency_grid, s_universe_width / ADJACENCY_BOXES);

    PopulateSystems(planet_density, specials_freq);
    GenerateStarlanes(starlane_freq, adjacency_grid);
    InitializeSystemGraph();
    GenerateHomeworlds(total_players, homeworld_planet_ids);
    NamePlanets();
    GenerateEmpires(homeworld_planet_ids, player_setup_data);

    GetPredefinedShipDesignManager().AddShipDesignsToUniverse();

    Logger().debugStream() << "Applying first turn effects and updating meters";

    // Apply effects for 1st turn.
    ApplyAllEffectsAndUpdateMeters();
    // Set active meters to targets or maxes after first meter effects application
    SetActiveMetersToTargetMaxCurrentValues(m_objects);

    BackPropegateObjectMeters();

    // Re-apply meter effects, so that results depending on meter values can be
    // re-checked after initial setting of those meter values
    ApplyMeterEffectsAndUpdateMeters();
    // Re-set active meters to targets after re-application of effects
    SetActiveMetersToTargetMaxCurrentValues(m_objects);

    BackPropegateObjectMeters();

    Logger().debugStream() << "!!!!!!!!!!!!!!!!!!! After setting active meters to targets";
    Logger().debugStream() << m_objects.Dump();

    UpdateEmpireObjectVisibilities();
}

void Universe::PopulateSystems(PlanetDensity density, SpecialsFrequency specials_freq)
{
    Logger().debugStream() << "PopulateSystems";

    std::vector<System*> sys_vec = Objects().FindObjects<System>();

    if (sys_vec.empty())
        throw std::runtime_error("Attempted to populate an empty galaxy.");

    const std::vector<std::vector<int> >& density_mod_to_planet_size_dist = UniverseDataTables()["DensityModToPlanetSizeDist"];
    const std::vector<std::vector<int> >& star_color_mod_to_planet_size_dist = UniverseDataTables()["StarColorModToPlanetSizeDist"];
    const std::vector<std::vector<int> >& slot_mod_to_planet_size_dist = UniverseDataTables()["SlotModToPlanetSizeDist"];
    const std::vector<std::vector<int> >& planet_size_mod_to_planet_type_dist = UniverseDataTables()["PlanetSizeModToPlanetTypeDist"];
    const std::vector<std::vector<int> >& slot_mod_to_planet_type_dist = UniverseDataTables()["SlotModToPlanetTypeDist"];
    const std::vector<std::vector<int> >& star_color_mod_to_planet_type_dist = UniverseDataTables()["StarColorModToPlanetTypeDist"];

    double planetary_special_chance = UniverseDataTables()["SpecialsFrequency"][0][specials_freq] / 10000.0;
    const std::set<std::string>& special_names = PlanetSpecialNames();
    SmallIntDistType specials_dist = SmallIntDist(0, special_names.size() - 1);

    for (std::vector<System*>::iterator it = sys_vec.begin(); it != sys_vec.end(); ++it) {
        System* system = *it;

        for (int orbit = 0; orbit < system->Orbits(); orbit++) {
            // make a series of "rolls" (1-100) for each planet size, and take the highest modified roll
            int idx = 0;
            int max_roll = 0;
            for (unsigned int i = 0; i < NUM_PLANET_SIZES; ++i) {
                int roll = g_hundred_dist() + star_color_mod_to_planet_size_dist[system->GetStarType()][i] + slot_mod_to_planet_size_dist[orbit][i]
                    + density_mod_to_planet_size_dist[density][i];
                if (max_roll < roll) {
                    max_roll = roll;
                    idx = i;
                }
            }
            PlanetSize planet_size = PlanetSize(idx);

            if (planet_size == SZ_NOWORLD)
                continue;

            if (planet_size == SZ_ASTEROIDS) {
                idx = PT_ASTEROIDS;
            } else if (planet_size == SZ_GASGIANT) {
                idx = PT_GASGIANT;
            } else {
                // make another series of modified rolls for planet type
                for (unsigned int i = 0; i < NUM_PLANET_TYPES; ++i) {
                    int roll = g_hundred_dist() + planet_size_mod_to_planet_type_dist[planet_size][i] + slot_mod_to_planet_type_dist[orbit][i] + 
                        star_color_mod_to_planet_type_dist[system->GetStarType()][i];
                    if (max_roll < roll) {
                        max_roll = roll;
                        idx = i;
                    }
                }
            }
            PlanetType planet_type = PlanetType(idx);

            if (planet_type == PT_ASTEROIDS)
                planet_size = SZ_ASTEROIDS;
            if (planet_type == PT_GASGIANT)
                planet_size = SZ_GASGIANT;

            Planet* planet = new Planet(planet_type, planet_size);

            //Logger().debugStream() << "Created new planet with current population: " << planet->CurrentMeterValue(METER_POPULATION) << " and initial  population: " << planet->InitialMeterValue(METER_POPULATION);

            bool tidal_lock = false;
            if (planet_type != PT_ASTEROIDS && planet_type != PT_GASGIANT && !special_names.empty() && RandZeroToOne() < planetary_special_chance) {
                std::set<std::string>::const_iterator name_it = special_names.begin();
                std::advance(name_it, specials_dist());
                planet->AddSpecial(*name_it);

                if (*name_it == "TIDAL_LOCK_SPECIAL")
                    tidal_lock = true;
                else if (*name_it == "SLOW_ROTATION_SPECIAL")
                    planet->SetRotationalPeriod(planet->RotationalPeriod() * 10.0);
                else if (*name_it == "HIGH_AXIAL_TILT_SPECIAL")
                    planet->SetHighAxialTilt();
            }

            Insert(planet); // add planet to universe map
            system->Insert(planet, orbit);  // add planet to system map

            planet->SetOrbitalPeriod(orbit, tidal_lock);
        }
    }
}

void Universe::GenerateStarlanes(StarlaneFrequency freq, const AdjacencyGrid& adjacency_grid)
{
    if (freq == LANES_NONE)
        return;

    int numSys, s1, s2, s3; // numbers of systems, indices in vec_sys
    int n; // loop counter

    std::vector<int> triVerts;  // indices of stars that form vertices of a triangle

    // array of set to store final, included starlanes for each star
    std::vector<std::set<int> > laneSetArray;

    // array of set to store possible starlanes for each star, as extracted form triangulation
    std::vector<std::set<int> > potentialLaneSetArray;

    // iterators for traversing lists of starlanes
    std::set<int>::iterator laneSetIter, laneSetEnd, laneSetIter2, laneSetEnd2;

    // get systems
    std::vector<System*> sys_vec = Objects().FindObjects<System>();

    // pass systems to Delauney Triangulation routine, getting array of triangles back
    std::list<Delauney::DTTriangle>* triList = Delauney::DelauneyTriangulate(sys_vec);
    if (!triList ||triList->empty()) {
        Logger().errorStream() << "Got no list or blank list of triangles from Triangulation.";
        return;
    }

    Delauney::DTTriangle tri;

    // convert passed StarlaneFrequency freq into maximum number of starlane jumps between systems that are
    // "adjacent" in the delauney triangulation.  (separated by a single potential starlane).
    // these numbers can and should be tweaked or extended
    int maxJumpsBetweenSystems = UniverseDataTables()["MaxJumpsBetweenSystems"][0][freq];

    numSys = sys_vec.size();  // (actually = number of systems + 1)

    // initialize arrays...
    potentialLaneSetArray.resize(numSys);
    for (n = 0; n < numSys; n++) {
        potentialLaneSetArray[n].clear();
    }
    laneSetArray.resize(numSys);
    for (n = 0; n < numSys; n++) {
        laneSetArray[n].clear();
    }

    // extract triangles from list, add edges to sets of potential starlanes for each star (in array)
    while (!triList->empty()) {
        tri = triList->front();

        triVerts = tri.Verts();
        s1 = triVerts[0];
        s2 = triVerts[1];
        s3 = triVerts[2];

        // add starlanes to list of potential starlanes for each star, making sure each pair involves
        // only stars that actually exist.  triangle generation uses three extra points which don't
        // represent actual systems and which need to be weeded out here.
        if ((s1 >= 0) && (s2 >= 0) && (s3 >= 0)) {
            if ((s1 < numSys) && (s2 < numSys)) {
                potentialLaneSetArray[s1].insert(s2);
                potentialLaneSetArray[s2].insert(s1);
            }
            if ((s1 < numSys) && (s3 < numSys)) {
                potentialLaneSetArray[s1].insert(s3);
                potentialLaneSetArray[s3].insert(s1);
            }
            if ((s2 < numSys) && (s3 < numSys)) {
                potentialLaneSetArray[s2].insert(s3);
                potentialLaneSetArray[s3].insert(s2);
            }
        }

        triList->pop_front();
    }

    // cleanup
    delete triList;

    //Logger().debugStream() << "Extracted Potential Starlanes from Triangulation";

    double maxStarlaneLength = UniverseDataTables()["MaxStarlaneLength"][0][0];
    CullTooLongLanes(maxStarlaneLength, potentialLaneSetArray, sys_vec);

    CullAngularlyTooCloseLanes(0.98, potentialLaneSetArray, sys_vec);

    //Logger().debugStream() << "Culled Agularly Too Close Lanes";

    laneSetArray = potentialLaneSetArray;

    // attempt removing lanes, but don't do so if it would make the systems
    // the lane connects too far apart
    for (n = 0; n < numSys; ++n) {
        laneSetIter = potentialLaneSetArray[n].begin();

        while (laneSetIter != potentialLaneSetArray[n].end()) {
            s1 = *laneSetIter;

            // try removing lane
            laneSetArray[n].erase(s1);
            laneSetArray[s1].erase(n);

            if (!ConnectedWithin(n, s1, maxJumpsBetweenSystems, laneSetArray)) {
                // lane removal was a bad idea.  restore it
                laneSetArray[n].insert(s1);
                laneSetArray[s1].insert(n);
            }

            laneSetIter++;
        } // end while
    }

     // add the starlane to the stars
    for (n = 0; n < numSys; ++n) {
        const std::set<int>& lanes = laneSetArray[n];
        for (std::set<int>::const_iterator it = lanes.begin(); it != lanes.end(); ++it)
            sys_vec[n]->AddStarlane(*it);
    }


    //for (n = 0; n < numSys; n++)
    //    laneSetArray[n].clear();

    //// array of indices of systems from which to start growing spanning tree(s).  This can later be replaced with
    //// some sort of user input.  It can also be ommited entirely, so just the ConnectedWithin loop below is used.
    //std::vector<int> roots(4);
    //roots[0] = 0;  roots[1] = 1;  roots[2] = 2;  roots[3] = 3;
    //GrowSpanningTrees(roots, potentialLaneSetArray, laneSetArray);
    ////Logger().debugStream() << "Constructed initial spanning trees.";

    //// add starlanes of spanning tree to stars
    //for (n = 0; n < numSys; n++) {
    //    laneSetIter = laneSetArray[n].begin();
    //    laneSetEnd = laneSetArray[n].end();
    //    while (laneSetIter != laneSetEnd) {
    //        s1 = *laneSetIter;
    //        // add the starlane to the stars
    //        sys_vec[n]->AddStarlane(s1);
    //        sys_vec[s1]->AddStarlane(n);
    //        laneSetIter++;
    //    } // end while
    //} // end for n


    //// loop through stars, seeing if any are too far away from stars they could be connected to by a
    //// potential starlane.  If so, add the potential starlane to the stars to directly connect them
    //for (n = 0; n < numSys; n++) {
    //    laneSetIter = potentialLaneSetArray[n].begin();
    //    laneSetEnd = potentialLaneSetArray[n].end();

    //    while (laneSetIter != laneSetEnd) {
    //        s1 = *laneSetIter;

    //        if (!ConnectedWithin(n, s1, maxJumpsBetweenSystems, laneSetArray)) {

    //            // add the starlane to the sets of starlanes for each star
    //            laneSetArray[n].insert(s1);
    //            laneSetArray[s1].insert(n);
    //            // add the starlane to the stars
    //            sys_vec[n]->AddStarlane(s1);
    //            sys_vec[s1]->AddStarlane(n);
    //        }

    //        laneSetIter++;
    //    } // end while
    //} // end for n
}

void Universe::GenerateHomeworlds(int players, std::vector<int>& homeworld_planet_ids)
{
    homeworld_planet_ids.clear();

    std::vector<System*> sys_vec = Objects().FindObjects<System>();
    //Logger().debugStream() << "Universe::GenerateHomeworlds sys_vec:";
    //for (std::vector<System*>::const_iterator it = sys_vec.begin(); it != sys_vec.end(); ++it) {
    //    const System* sys = *it;
    //    Logger().debugStream() << "... sys ptr: " << sys << " name: " << (sys ? sys->Name() : "no system!?") << " id: " << (sys ? boost::lexical_cast<std::string>(sys->ID()) : "none?!");
    //}

    if (sys_vec.empty())
        throw std::runtime_error("Attempted to generate homeworlds in an empty galaxy.");

    for (int i = 0; i < players; ++i) {
        int system_index;
        System* system;

        // make sure it has planets and it's not too close to the other homeworlds
        bool too_close = true;
        int attempts = 0;
        do {
            too_close = false;
            system_index = RandSmallInt(0, static_cast<int>(sys_vec.size()) - 1);
            //Logger().debugStream() << "Universe::GenerateHomeworlds trying to put homeworld on system with index: " << system_index;
            system = sys_vec[system_index];
            //Logger().debugStream() << "... system ptr: " << system << " name: " << (system ? system->Name() : "no system!?") << " id: " << (system ? boost::lexical_cast<std::string>(system->ID()) : "none?!");

            for (unsigned int j = 0; j < homeworld_planet_ids.size(); ++j) {
                //Logger().debugStream() << "Universe::GenerateHomeworlds checking previously-existing homeworld with id " << homeworld_planet_ids[j];
                Planet* homeworld = GetObject<Planet>(homeworld_planet_ids[j]);
                if (!homeworld) {
                    Logger().errorStream() << "couldn't find homeworld!";
                    continue;
                }

                System* existing_system = GetObject<System>(homeworld->SystemID());
                //Logger().debugStream() << ".... existing system ptr: " << existing_system;

                if (!existing_system) {
                    Logger().errorStream() << "couldn't find existing system!";
                    continue;
                }

                double x_dist = existing_system->X() - system->X();
                double y_dist = existing_system->Y() - system->Y();
                if (x_dist * x_dist + y_dist * y_dist < MIN_HOME_SYSTEM_SEPARATION * MIN_HOME_SYSTEM_SEPARATION) {
                    too_close = true;
                    break;
                }
            }
        } while ((!system->Orbits() || system->FindObjectIDs<Planet>().empty() || too_close) && ++attempts < 50);

        sys_vec.erase(sys_vec.begin() + system_index);

        // find a place to put the homeworld, and replace whatever planet is there already
        int planet_id, home_orbit; std::string planet_name;

        // we can only select a planet if there are planets in this system.
        if (system->Orbits() >0 && !system->FindObjectIDs<Planet>().empty()) {
            std::vector<int> vec_orbits;
            for (int i = 0; i < system->Orbits(); i++)
                if (system->FindObjectIDsInOrbit<Planet>(i).size() > 0)
                    vec_orbits.push_back(i);

            int planet_index = vec_orbits.size() > 1   ?   RandSmallInt(0, vec_orbits.size() - 1)   :   0;
            planet_name = system->Name() + " " + RomanNumber(planet_index + 1);
            home_orbit = vec_orbits[planet_index];
            Delete(system->FindObjectIDsInOrbit<Planet>(home_orbit).back());
        } else {
            home_orbit = 0;
            planet_name = system->Name() + " " + RomanNumber(home_orbit + 1);
        }

        Planet* planet = new Planet(PT_TERRAN, SZ_MEDIUM);
        planet_id = Insert(planet);
        planet->Rename(planet_name);
        system->Insert(planet, home_orbit);

        homeworld_planet_ids.push_back(planet_id);
    }
}

void Universe::NamePlanets()
{
    std::vector<System*> sys_vec = Objects().FindObjects<System>();
    for (std::vector<System*>::iterator it = sys_vec.begin(); it != sys_vec.end(); ++it) {
        System* system = *it;
        int num_planets_in_system = 0;
        for (int i = 0; i < system->Orbits(); i++) {
            std::vector<int> planet_ids = system->FindObjectIDsInOrbit<Planet>(i);
            if (!planet_ids.empty()) {
                assert(planet_ids.size() == 1);
                Planet* planet = GetObject<Planet>(*planet_ids.begin());
                if (!planet) {
                    Logger().errorStream() << "Universe::NamePlanet couldn't get planet with id " << *planet_ids.begin();
                    continue;
                }
                if (planet->Type() == PT_ASTEROIDS)
                    planet->Rename(UserString("PL_ASTEROID_BELT"));
                else
                    planet->Rename(system->Name() + " " + RomanNumber(++num_planets_in_system));
            }
        }
    }
}

void Universe::GenerateEmpires(std::vector<int>& homeworld_planet_ids, const std::map<int, PlayerSetupData>& player_setup_data)
{
    Logger().debugStream() << "Generating " << player_setup_data.size() << " empires";
    // create empires and assign homeworlds, names, colors, and fleet ranges to each one

    // load default empire names
    static std::list<std::string> empire_names;
    if (empire_names.empty())
        LoadEmpireNames(empire_names);

    std::vector<GG::Clr> colors = EmpireColors();   // copy, not reference, so that individual colours can be removed after they're used

    SpeciesManager&                     species_manager =           GetSpeciesManager();
    species_manager.ClearSpeciesHomeworlds();

    const PredefinedShipDesignManager&  predefined_ship_designs =   GetPredefinedShipDesignManager();
    const FleetPlanManager&             starting_fleet_plans =      GetFleetPlanManager();
    const ItemSpecManager&              starting_unlocked_items =   GetItemSpecManager();

    // create empire and starting conditions for each player
    int player_i = 0;
    for (std::map<int, PlayerSetupData>::const_iterator setup_data_it = player_setup_data.begin();
         setup_data_it != player_setup_data.end(); ++setup_data_it, ++player_i)
    {
        int         player_id =                 setup_data_it->first;
        if (player_id == Networking::INVALID_PLAYER_ID)
            Logger().errorStream() << "Universe::GenerateEmpires player id (" << player_id << ") is invalid";
        // use player ID for empire ID so that the calling code can get the
        // correct empire for each player ID  in player_setup_data
        int         empire_id =                 player_id;

        std::string player_name =               setup_data_it->second.m_player_name;
        std::string empire_name =               setup_data_it->second.m_empire_name;
        GG::Clr     empire_colour =             setup_data_it->second.m_empire_color;
        std::string empire_starting_species =   setup_data_it->second.m_starting_species_name;

        int         homeworld_id =              homeworld_planet_ids[player_i];

        // validate or generate name/colour/species

        // ensure no other empire gets auto-assigned this colour automatically
        std::vector<GG::Clr>::iterator color_it = std::find(colors.begin(), colors.end(), empire_colour);
        if (color_it != colors.end())
            colors.erase(color_it);


        // if no colour already set, do so automatically
        if (empire_colour == GG::Clr(0, 0, 0, 0)) {
            if (!colors.empty()) {
                // a list of colours is available.  pick a colour
                int color_idx = RandInt(0, colors.size() - 1);
                empire_colour = colors[color_idx];
                colors.erase(colors.begin() + color_idx);
            } else {
                // as a last resort, make up a colour
                empire_colour = GG::FloatClr(static_cast<float>(RandZeroToOne()), static_cast<float>(RandZeroToOne()),
                                             static_cast<float>(RandZeroToOne()), 1.0f);
            }
        }

        // if no empire name already set, do so automatically
        if (empire_name.empty()) {
            // automatically pick a name
            if (!empire_names.empty()) {
                // pick a name from the list of empire names
                int empire_name_idx = RandSmallInt(0, static_cast<int>(empire_names.size()) - 1);
                std::list<std::string>::iterator it = empire_names.begin();
                std::advance(it, empire_name_idx);
                empire_name = *it;
                empire_names.erase(it);
            } else {
                // use a generic name
                empire_name = UserString("EMPIRE") + boost::lexical_cast<std::string>(player_i);
            }
        }

        // if no empire starting species already set, do so automatically
        if (empire_starting_species.empty()) {
            // automatically pick a species
            if (species_manager.empty()) {
                Logger().errorStream() << "Universe::GenerateEmpires found an empty species manager!  Can't assign species to empires.";
            } else {
                int species_name_idx = 0;
                if (species_manager.NumSpecies() > 1)
                    species_name_idx = RandSmallInt(0, species_manager.NumSpecies() - 1);
                SpeciesManager::iterator it = species_manager.begin();
                std::advance(it, species_name_idx);
                empire_starting_species = it->first;
            }
        }

        Logger().debugStream() << "Universe::GenerateEmpires creating empire named: " << empire_name
                               << " with empire id: " << empire_id << " for player: " << player_name << " (with player id: " << player_id << ")"
                               << " starting with species: " << empire_starting_species
                               << " at homeworld id: " << homeworld_id;

        // create new Empire object through empire manager
        Empire* empire = Empires().CreateEmpire(empire_id, empire_name, player_name, empire_colour);


        // set ownership of home planet
        Planet* home_planet = GetObject<Planet>(homeworld_id);
        System* home_system = GetObject<System>(home_planet->SystemID());
        if (!home_planet || !home_system) {
            Logger().errorStream() << "Couldn't get homeworld or system for generated empire...";
            continue;
        }

        Logger().debugStream() << "Universe::GenerateEmpires Setting " << home_system->Name() << " (Planet #" <<  home_planet->ID()
                               << ") to be home system for Empire " << empire_id;

        home_planet->AddOwner(empire_id);
        //home_system->AddOwner(empire_id);   // should be redundant

        empire->SetCapitolID(home_planet->ID());

        empire->AddExploredSystem(home_planet->SystemID());

        home_planet->SetSpecies(empire_starting_species);
        if (Species* species = species_manager.GetSpecies(empire_starting_species)) {
            species->AddHomeworld(homeworld_id);

            // set homeword's planet type to the preferred type for this species
            const std::map<PlanetType, PlanetEnvironment>& spte = species->PlanetEnvironments();
            if (!spte.empty()) {
                // invert map from planet type to environments to map from
                // environments to type, sorted by environment
                std::map<PlanetEnvironment, PlanetType> sept;
                for (std::map<PlanetType, PlanetEnvironment>::const_iterator it = spte.begin(); it != spte.end(); ++it)
                    sept[it->second] = it->first;
                // assuming enum values are ordered in increasing goodness...
                PlanetType preferred_planet_type = sept.rbegin()->second;

                home_planet->SetType(preferred_planet_type);
            }

        } else {
            Logger().errorStream() << "Universe::GenerateEmpires Couldn't get species \"" << empire_starting_species << "\" to set with homeworld id " << homeworld_id;
        }

        // find a focus to give planets by default.  use first defined available focus.
        // the planet's AvailableFoci function should return a vector of all names of
        // available foci, although this might be buggy since the universe isn't fully
        // created yet at this point in unverse generation.
        std::vector<std::string> available_foci = home_planet->AvailableFoci();
        if (!available_foci.empty())
            home_planet->SetFocus(*available_foci.begin());


        // give homeworlds a shipyard and drydock so players can build scouts, colony ships and basic attack ships immediately
        Building* building = new Building(empire_id, "BLD_SHIPYARD_BASE", empire_id);
        int building_id = Insert(building);
        home_planet->AddBuilding(building_id);

        building = new Building(empire_id, "BLD_SHIPYARD_ORBITAL_DRYDOCK", empire_id);
        building_id = Insert(building);
        home_planet->AddBuilding(building_id);

        building = new Building(empire_id, "BLD_CULTURE_ARCHIVES", empire_id);
        building_id = Insert(building);
        home_planet->AddBuilding(building_id);

        building = new Building(empire_id, "BLD_IMPERIAL_PALACE", empire_id);
        building_id = Insert(building);
        home_planet->AddBuilding(building_id);

        // give new empire items and ship designs it should start with
        starting_unlocked_items.AddItemsToEmpire(empire);

        std::map<std::string, int> design_ids = predefined_ship_designs.AddShipDesignsToEmpire(empire);


        // create new empire's starting fleets
        for (FleetPlanManager::iterator it = starting_fleet_plans.begin(); it != starting_fleet_plans.end(); ++it) {

            // create fleet itself
            const std::string& fleet_name = it->Name();
            Fleet* fleet = new Fleet(fleet_name, home_system->X(), home_system->Y(), empire_id);
            if (!fleet) {
                Logger().errorStream() << "unable to create new fleet!";
                break;
            }
            Insert(fleet);
            home_system->Insert(fleet);


            // create ships and add to fleet
            const std::vector<std::string>& ship_design_names = it->ShipDesigns();
            for (std::vector<std::string>::const_iterator ship_it = ship_design_names.begin(); ship_it != ship_design_names.end(); ++ship_it) {
                // get universe id of design by looking up name in this empire's map from name to design id
                const std::string& design_name = *ship_it;
                std::map<std::string, int>::const_iterator design_it = design_ids.find(design_name);
                if (design_it != design_ids.end()) {
                    // get actual design from universe
                    int design_id = design_it->second;
                    const ShipDesign* design = GetShipDesign(design_id);
                    if (!design) {
                        Logger().errorStream() << "unable to get ShipDesign with id " << design_id << " and name " << design_name;
                        continue;
                    }

                    // create new ship
                    Ship* ship = new Ship(empire_id, design_id, empire_starting_species, empire_id);
                    if (!ship) {
                        Logger().errorStream() << "unable to create new ship!";
                        break;
                    }

                    ship->Rename(empire->NewShipName());
                    int ship_id = Insert(ship);

                    // add ship to fleet
                    fleet->AddShip(ship_id);    // also moves ship to fleet's location and inserts into system
                } else {    // design_it == design_ids.end()
                    Logger().errorStream() << "couldn't find design name " << design_name << " in map from design names to ids of designs added to empire";
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
UniverseObject* GetObject(int object_id)
{
    return GetUniverse().Objects().Object(object_id);
}

UniverseObject* GetEmpireKnownObject(int object_id, int empire_id)
{
    return GetUniverse().EmpireKnownObjects(empire_id).Object(object_id);
}

