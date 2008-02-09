// -*- C++ -*-
#ifndef _Universe_h_
#define _Universe_h_

#ifndef BOOST_SERIALIZATION_SHARED_PTR_HPP
#include <boost/serialization/shared_ptr.hpp>
#endif

#ifndef BOOST_SIGNAL_HPP
#include <boost/signal.hpp>
#endif

#include "Enums.h"
#include "Predicates.h"
#include "Effect.h"

#if defined(_MSC_VER)
  // HACK! this keeps VC 7.x from barfing when it sees "typedef __int64 int64_t;"
  // in boost/cstdint.h when compiling under windows
#  if defined(int64_t)
#    undef int64_t
#  endif
#elif defined(WIN32)
  // HACK! this keeps gcc 3.x from barfing when it sees "typedef long long uint64_t;"
  // in boost/cstdint.h when compiling under windows
#  define BOOST_MSVC -1
#endif
#ifndef BOOST_GRAPH_ADJACENCY_LIST_HPP
#include <boost/graph/adjacency_list.hpp>
#endif
#ifndef BOOST_FILTERED_GRAPH_HPP
#include <boost/graph/filtered_graph.hpp>
#endif

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <vector>
#include <list>
#include <map>
#include <string>
#include <set>

#ifdef __GNUC__
  // GCC doesn't allow us to forward-declare PlayerSetupData
#  ifndef _MultiplayerCommon_h_
#    include "../util/MultiplayerCommon.h"
#  endif
#else
  struct PlayerSetupData;
#endif

class Empire;
struct UniverseObjectVisitor;
class XMLElement;
struct ShipDesign;
class UniverseObject;
class System;

/** The Universe class contains the majority of FreeOrion gamestate: All the UniverseObjects in a game, and (of 
    less importance) all ShipDesigns in a game.  (Other gamestate is contained in the Empire class.)

    The Universe class also provides many functions with which to access objects in it, information about the
    objects, and infromation about the objects' relationships to eachother.  As well, there are functions that
    generate and populate new Universe gamestates when new games are started.
*/
class Universe
{
protected:
    typedef std::map<int, UniverseObject*>  ObjectMap;          ///< container type used to hold the objects in the universe; keyed by ID number
    typedef std::map<int, std::set<int> >   ObjectKnowledgeMap; ///< container type used to hold sets of IDs of Empires which known information about an object (or deleted object); keyed by object ID number
    typedef std::map<int, ShipDesign*>      ShipDesignMap;      ///< container type used to hold ShipDesigns created by players, keyed by design id number

public:
    /** Set to true to make everything visible for everyone. Useful for debugging. */
    static const bool ALL_OBJECTS_VISIBLE;

    struct vertex_system_pointer_t {typedef boost::vertex_property_tag kind;}; ///< a system graph property map type
    typedef boost::property<vertex_system_pointer_t, System*,
                            boost::property<boost::vertex_index_t, int> > 
    vertex_property_t; ///< a system graph property map type
    typedef boost::property<boost::edge_weight_t, double> 
    edge_property_t; ///< a system graph property map type

    typedef ObjectMap::const_iterator            const_iterator;   ///< a const_iterator for sequences over the objects in the universe
    typedef ObjectMap::iterator                  iterator;         ///< an iterator for sequences over the objects in the universe

    typedef ShipDesignMap::const_iterator        ship_design_iterator;  ///< const iterator over ship designs created by players that are known by this client

    typedef std::vector<const UniverseObject*>   ConstObjectVec;   ///< the return type of FindObjects()
    typedef std::vector<UniverseObject*>         ObjectVec;        ///< the return type of the non-const FindObjects()
    typedef std::vector<int>                     ObjectIDVec;      ///< the return type of FindObjectIDs()

    //!< contains info about a single effect's alterations to a meter
    struct EffectAccountingInfo
    {
        EffectAccountingInfo();         // default ctor
        int source_id;                  // source object of effect
        int caused_by_empire_id;        // empire that causes effect to occur, if applicable.  Tech effects are caused by the empire that researched them
        EffectsCauseType cause_type;    // is the effect due to a tech, building, special, or unknown cause?
        std::string specific_cause;     // what tech, building or special was the cause?
        double meter_change;            // net change on meter due to this effect, as best known by client's empire
        double running_meter_total; 
    };
    typedef std::map<int, std::map<MeterType, std::vector<EffectAccountingInfo> > > EffectAccountingMap;    //!< Effect accounting info for all meters


    /** \name Signal Types */ //@{
    typedef boost::signal<void (const UniverseObject *)> UniverseObjectDeleteSignalType; ///< emitted just before the UniverseObject is deleted
    //@}


    /** \name Structors */ //@{
    Universe(); ///< default ctor
    const Universe& operator=(Universe& rhs); ///< assignment operator (move semantics)
    virtual ~Universe(); ///< dtor
    //@}

    /** \name Accessors */ //@{
    const UniverseObject* Object(int id) const; ///< returns a pointer to the universe object with ID number \a id, or 0 if none exists
    UniverseObject* Object(int id);  ///< returns a pointer to the universe object with ID number \a id, or 0 if none exists

    template <class T> const T* Object(int id) const; ///< returns a pointer to the object of type T with ID number \a id. Returns 0 if none exists or the object with ID \a id is not of type T.
    template <class T> T* Object(int id);  ///< returns a pointer to the object of type T with ID number \a id. Returns 0 if none exists or the object with ID \a id is not of type T.

    /** returns all the objects that match \a visitor */
    ConstObjectVec FindObjects(const UniverseObjectVisitor& visitor) const;

    /** returns all the objects that match \a visitor */
    ObjectVec FindObjects(const UniverseObjectVisitor& visitor);

    /** returns all the objects of type T. */
    template <class T> std::vector<const T*> FindObjects() const;

    /** returns all the objects of type T. */
    template <class T> std::vector<T*> FindObjects();

    /** returns the IDs of all the objects that match \a visitor */
    ObjectIDVec FindObjectIDs(const UniverseObjectVisitor& visitor) const;

    /** returns the IDs of all the objects of type T. */
    template <class T> ObjectIDVec FindObjectIDs() const;

    iterator begin();
    iterator end();

    const_iterator begin() const;   ///< returns the begin const_iterator for the objects in the universe
    const_iterator end() const;     ///< returns the end const_iterator for the objects in the universe


    const UniverseObject* DestroyedObject(int id) const;          ///< returns a pointer to the destroyed universe object with ID number \a id, or 0 if none exists
    
    const_iterator beginDestroyed() const  {return m_destroyed_objects.begin();}    ///< returns the begin const_iterator for the destroyed objects from the universe
    const_iterator endDestroyed() const    {return m_destroyed_objects.end();}      ///< returns the end const_iterator for the destroyed objects from the universe


    const ShipDesign* GetShipDesign(int ship_design_id) const;                      ///< returns the ship design with id \a ship_design id, or 0 if non exists

    ship_design_iterator beginShipDesigns() const   {return m_ship_designs.begin();}    ///< returns the begin iterator for ship designs
    ship_design_iterator endShipDesigns() const     {return m_ship_designs.end();}      ///< returns the end iterator for ship designs

    double LinearDistance(int system1_id, int system2_id) const; ///< returns the straight-line distance between the systems with the given IDs. \throw std::out_of_range This function will throw if either system ID is out of range.

    /** returns the sequence of systems, including \a system1 and \a system2, that defines the shortest path from \a
        system1 to \a system2, and the distance travelled to get there.  If no such path exists, the list will be empty.
        Note that the path returned may be via one or more starlane, or may be "offroad".  The path is calculated using
        the visiblity for empire \a empire_id, or without regard to visibility if \a empire_id == ALL_EMPIRES.  \throw
        std::out_of_range This function will throw if either system ID is out of range. */
    std::pair<std::list<System*>, double> ShortestPath(int system1_id, int system2_id, int empire_id = ALL_EMPIRES) const;

    /** returns the sequence of systems, including \a system1 and \a system2, that defines the path with the fewest
        jumps from \a system1 to \a system2, and the number of jumps to get there.  If no such path exists, the list
        will be empty.  The path is calculated using the visiblity for empire \a empire_id, or without regard to
        visibility if \a empire_id == ALL_EMPIRES.  \throw std::out_of_range This function will throw if either system
        ID is out of range. */
    std::pair<std::list<System*>, int> LeastJumpsPath(int system1_id, int system2_id, int empire_id = ALL_EMPIRES) const;

    /** returns whether there is a path known to empire \a empire_id between system \a system1 and system \a system2.
        The path is calculated using the visiblity for empire \a empire_id, or without regard to visibility if
        \a empire_id == ALL_EMPIRES.  \throw std::out_of_range This function will throw if either system
        ID is out of range. */
    bool SystemsConnected(int system1_id, int system2_id, int empire_id = ALL_EMPIRES) const;

    /** returns true iff \a system is reachable from another system (i.e. it has at least one known starlane to it).
        This does not guarantee that the system is reachable from any other system, as two separate groups of locally
        but not globally internonnected systems may exist. The starlanes considered depend on their visiblity for 
        empire \a empire_id, or without regard to visibility if \a empire_id == ALL_EMPIRES.  \throw std::out_of_range
        This function will throw if the system ID is out of range. */
    bool SystemReachable(int system_id, int empire_id = ALL_EMPIRES) const;

    /** returns the systems that are one starlane hop away from system \a system.  The returned systems are indexed by
        distance from \a system.  The neighborhood is calculated using the visiblity for empire \a empire_id, or without
        regard to visibility if \a empire_id == ALL_EMPIRES.  \throw std::out_of_range This function will throw if the
        system ID is out of range. */
    std::map<double, System*> ImmediateNeighbors(int system_id, int empire_id = ALL_EMPIRES) const;

    /** returns map, indexed by object id, to map, indexed by MeterType, to vector of EffectAccountInfo for the meter,
        in order effects were applied to the meter. */
    const EffectAccountingMap& GetEffectAccountingMap() const {return m_effect_accounting_map;}

    mutable UniverseObjectDeleteSignalType UniverseObjectDeleteSignal; ///< the state changed signal object for this UniverseObject
    //@}

    /** \name Mutators */ //@{
    /** inserts object \a obj into the universe; returns the ID number assigned to the object, or -1 on failure.
        \note Universe gains ownership of \a obj once it is inserted; the caller should \a never delete \a obj after
        passing it to Insert().*/
    int               Insert(UniverseObject* obj);

    /** inserts object \a obj of given ID into the universe; returns true on proper insert, or false on failure.
        \note Universe gains ownership of \a obj once it is inserted; the caller should \a never delete \a obj after
        passing it to InsertID().
        Useful mostly for times when ID needs to be consistant on client and server*/
    bool              InsertID(UniverseObject* obj, int id);

    /** Inserts \a ship_design into the universe; returns the ship design ID assigned to it, or -1 on failure.
        \note Unvierse gains ownership of \a ship_design once inserted. */
    int               InsertShipDesign(ShipDesign* ship_design);

    /** Inserts \a ship_design into the universe with given \a id;  returns true on success, or false on failure.
        \note Unvierse gains ownership of \a ship_design once inserted. */
    bool              InsertShipDesignID(ShipDesign* ship_design, int id);

    /** generates systems and planets, assigns homeworlds and populates them with people, industry and bases, and places starting fleets.  Uses predefined galaxy shapes.  */
    void              CreateUniverse(int size, Shape shape, Age age, StarlaneFrequency starlane_freq, PlanetDensity planet_density, 
                                     SpecialsFrequency specials_freq, int players, int ai_players, 
                                     const std::map<int, PlayerSetupData>& player_setup_data);

    /** Applies all Effects from Buildings, Specials, Techs, etc. */
    void              ApplyEffects();

    void              InitMeterEstimatesAndDiscrepancies();  ///< determines discrepancies and stores in m_effect_discrepancy_map, using UpdateMeterEstimates() in process

    void              UpdateMeterEstimates();    ///< based on orders given up to this point, and known universe, estimates what meter maxes will be next turn, and updates them accordingly

    /** Reconstructs the per-empire system graph views needed to calculate routes based on visibility. */
    void              RebuildEmpireViewSystemGraphs();

    /** removes the object with ID number \a id from the universe's map of existing objects and places it into the map of destroyed objects.
        removes the object from any containing UniverseObjects, though leaves the object's own records of what contained it intact, so that
        this information may be retained for later reference */
    void              Destroy(int id);

    /** removes from the universe (whether existing or destroyed) and deletes the object with ID number \a id; returns true if such an object was found, false otherwise*/
    bool              Delete(int id);

    /** marks an object for destruction by the Destroy effect. */
    void              EffectDestroy(int id);

    /** cleans up internal storage of now-invalidated empire ID */
    void              HandleEmpireElimination(int empire_id);

    /** sets whether to inhibit UniverseObjectSignals.  Inhibits if \a inhibit is true, and (re)enables UniverseObjectSignals if \a inhibit is false. */
    static void       InhibitUniverseObjectSignals(bool inhibit = true);
    //@}

    /** returns the size of the galaxy map.  Does not measure absolute distances; the ratio between map coordinates and actual distance varies
        depending on universe size */
    static double UniverseWidth();

    int GenerateObjectID();  ///< generates an object ID for a future object. Usually used by the server to service new ID requests

    int GenerateDesignID();  ///< generates adesign ID for a new (ship) design. Usually used by the server to service new ID requests

    typedef std::vector<std::vector<std::set<System*> > > AdjacencyGrid;

    static const bool& UniverseObjectSignalsInhibited();    // returns true if UniverseOjbectSignals are inhibited, false otherwise

    /** HACK! This must be set to the encoding empire's id when serializing a Universe, so that only the relevant parts
        of the Universe are serialized.  The use of this global variable is done just so I don't have to rewrite any
        custom boost::serialization classes that implement empire-dependent visibility. */
    static int s_encoding_empire;

protected:
    typedef std::vector< std::vector<double> > DistanceMatrix;

    // declare main graph types, including properties declared above
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, 
                                  vertex_property_t, edge_property_t>
    SystemGraph;

    // declare types for iteration over graph
    typedef SystemGraph::vertex_iterator   VertexIterator;
    typedef SystemGraph::out_edge_iterator OutEdgeIterator;

    struct EdgeVisibilityFilter
    {
        EdgeVisibilityFilter();
        EdgeVisibilityFilter(const SystemGraph* graph, int empire_id);
        template <typename EdgeDescriptor>
        bool operator()(const EdgeDescriptor& edge) const;
        static bool CanSeeAtLeastOneSystem(const Empire* empire, int system1, int system2);
    private:
        const SystemGraph* m_graph;
        const Empire* m_empire;
    };
    typedef boost::filtered_graph<SystemGraph, EdgeVisibilityFilter> EmpireViewSystemGraph;
    typedef std::map<int, boost::shared_ptr<EmpireViewSystemGraph> > EmpireViewSystemGraphMap;

    // declare property map types for properties declared above
    typedef boost::property_map<SystemGraph, vertex_system_pointer_t>::const_type ConstSystemPointerPropertyMap;
    typedef boost::property_map<SystemGraph, vertex_system_pointer_t>::type       SystemPointerPropertyMap;
    typedef boost::property_map<SystemGraph, boost::vertex_index_t>::const_type   ConstIndexPropertyMap;
    typedef boost::property_map<SystemGraph, boost::vertex_index_t>::type         IndexPropertyMap;
    typedef boost::property_map<SystemGraph, boost::edge_weight_t>::const_type    ConstEdgeWeightPropertyMap;
    typedef boost::property_map<SystemGraph, boost::edge_weight_t>::type          EdgeWeightPropertyMap;

    // effects processing stuff
    /** Multimap from effects group to pairs (one per effect in the effects group), consisting of source object
        ID and the set of targets to which the effects should be applied.  EffectsGroup does not contain any info
        about its source object, so it needs to be stored here. */
    typedef std::pair<boost::shared_ptr<const Effect::EffectsGroup>, std::pair<int, Effect::EffectsGroup::TargetSet> > EffectsAndTargetsMapElem;
    typedef std::multimap<boost::shared_ptr<const Effect::EffectsGroup>, std::pair<int, Effect::EffectsGroup::TargetSet> > EffectsAndTargetsMap;
    
    /** Discrepancy between meter's value at start of turn, and the value that this client calculate that the
        meter should have with the knowledge available -> the unknown factor affecting the meter. */
    typedef std::map<int, std::map<MeterType, double> > EffectDiscrepancyMap;

    /** Multimap from effects group to pairs (one per effect in the effects group), consistent of source object
        ID and another pair, consisting of the type of cause for the effect (eg. special, building, tech) and the
        name of the specific cause.  Specific cause is the name of the tech, building type or special that contains
        the effects group.  The causes should be stored in the same order as the targets in a simultaneously built
        EffectsAndTargetsMap, allowing a particular effects group's targets and cause info to be looked up together. */
    typedef std::pair<boost::shared_ptr<const Effect::EffectsGroup>, std::pair<int, std::pair<EffectsCauseType, std::string> > > EffectsAndCausesMapElem;
    typedef std::multimap<boost::shared_ptr<const Effect::EffectsGroup>, std::pair<int, std::pair<EffectsCauseType, std::string> > > EffectsAndCausesMap;

    /** Populates \a effects_targets_map with all EffectsGroups and their targets in the known universe.  If
        \a effects_causes_map is provided (nonzero pointer) then this map will be simultaneously populated with
        information about the causes of each effects group. */
    void GetEffectsAndTargets(EffectsAndTargetsMap& effects_targets_map, EffectsAndCausesMap* effects_causes_map = 0);
    
    void ExecuteEffects(EffectsAndTargetsMap& effects_targets_map);         ///< executes all effects

    /** Executes only meter-altering effects; ignores other effects.  If \a effects_causes_map is provided
        (nonzero pointer) then its contents will be used to records effect details in m_effect_accounting_map;
        if it is not provided, then no effect accounting is done. */
    void ExecuteMeterEffects(EffectsAndTargetsMap& effects_targets_map, EffectsAndCausesMap* effects_causes_map = 0);


    void GenerateIrregularGalaxy(int stars, Age age, AdjacencyGrid& adjacency_grid);   ///< creates an irregular galaxy and stores the empire homeworlds in the homeworlds vector

    void PopulateSystems(PlanetDensity density, SpecialsFrequency specials_freq);  ///< Will generate planets for all systems that have empty object maps (ie those that aren't homeworld systems)
    void GenerateStarlanes(StarlaneFrequency freq, const AdjacencyGrid& adjacency_grid); ///< creates starlanes and adds them systems already generated
    bool ConnectedWithin(int system1, int system2, int maxLaneJumps, std::vector<std::set<int> >& laneSetArray); // used by GenerateStarlanes.  Determines if two systems are connected by maxLaneJumps or less edges on graph
    void CullAngularlyTooCloseLanes(double maxLaneUVectDotProd, std::vector<std::set<int> >& laneSetArray, std::vector<System*> &systems); // Removes lanes from passed graph that are angularly too close to eachother
    void CullTooLongLanes(double maxLaneLength, std::vector<std::set<int> >& laneSetArray, std::vector<System*> &systems); // Removes lanes from passed graph that are too long
    void GrowSpanningTrees(std::vector<int> roots, std::vector<std::set<int> >& potentialLaneSetArray, std::vector<std::set<int> >& laneSetArray); // grows trees to connect stars...  takes an array of sets of potential starlanes for each star, and puts the starlanes of the tree into another set
    void InitializeSystemGraph(); ///< resizes the system graph to the appropriate size and populates m_system_distances 
    void GenerateHomeworlds(int players, std::vector<int>& homeworlds);  ///< Picks systems to host homeworlds, generates planets for them, stores the ID's of the homeworld planets into the homeworld vector
    void NamePlanets(); ///< Names the planets in each system, based on the system's name
    /** Will create empire objects, assign them homeworlds, setup the homeworld population, industry, and starting fleets */
    void GenerateEmpires(int players, std::vector<int>& homeworlds, const std::map<int, PlayerSetupData>& player_setup_data);

    void DestroyImpl(int id);

    ObjectMap m_objects;                                    ///< UniverseObjects in the universe

    ObjectMap m_destroyed_objects;                          ///< objects that have been destroyed from the universe.  for the server: all of them;  for clients, only those that the local client knows about, not including previously-seen objects that the client no longer can see
    ObjectKnowledgeMap m_destroyed_object_knowers;          ///< keyed by (destroyed) object ID, map of sets of Empires' IDs that know the objects have been destroyed (ie. could see the object when it was destroyed)

    ShipDesignMap m_ship_designs;                           ///< ship designs in the universe

    DistanceMatrix m_system_distances;                      ///< the straight-line distances between all the systems; this is an lower-triangular matrix, so only access the elements in (highID, lowID) order
    SystemGraph m_system_graph;                             ///< a graph in which the systems are vertices and the starlanes are edges
    EmpireViewSystemGraphMap m_empire_system_graph_views;   ///< a map of empire IDs to the views of the system graph by those empires

    EffectAccountingMap m_effect_accounting_map;            ///< map from target object id, to map from target meter, to orderered list of structs with details of an effect and what it does to the meter
    EffectDiscrepancyMap m_effect_discrepancy_map;          ///< map from target object id, to map from target meter, to discrepancy between meter's actual initial value, and the initial value that this meter should have as far as the client can tell: the unknown factor affecting the meter


    int m_last_allocated_object_id;
    int m_last_allocated_design_id;

    std::set<int> m_marked_destroyed;                       ///< used while applying effects to cache objects that have been destroyed.  this allows to-be-destroyed objects to remain undestroyed until all effects have been processed, which ensures that to-be-destroyed objects still exist when other effects need to access them as a source object

    static double s_universe_width;

private:
    static bool s_inhibit_universe_object_signals;

    void GetShipDesignsToSerialize(const ObjectMap& serialized_objects, ShipDesignMap& designs_to_serialize);

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


// template implementations
#if (10 * __GNUC__ + __GNUC_MINOR__ > 33) && (!defined _UniverseObject_h_)
#  include "UniverseObject.h"
#endif

template <class Archive>
void Universe::serialize(Archive& ar, const unsigned int version)
{
    ObjectMap objects;
    if (Archive::is_saving::value) {
        for (ObjectMap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
            if (Universe::ALL_OBJECTS_VISIBLE ||
                it->second->GetVisibility(s_encoding_empire) != UniverseObject::NO_VISIBILITY ||
                universe_object_cast<System*>(it->second))
            {
                objects.insert(*it);
            }
        }
    }
    ShipDesignMap ship_designs;
    if (Archive::is_saving::value)
        GetShipDesignsToSerialize(objects, ship_designs);

    ar  & BOOST_SERIALIZATION_NVP(s_universe_width)
        & BOOST_SERIALIZATION_NVP(objects)
        & BOOST_SERIALIZATION_NVP(ship_designs)
        & BOOST_SERIALIZATION_NVP(m_last_allocated_object_id)
        & BOOST_SERIALIZATION_NVP(m_last_allocated_design_id);
    if (Archive::is_loading::value) {
        m_objects = objects;
        m_ship_designs = ship_designs;
        InitializeSystemGraph();
    }
}

template <class T> 
const T* Universe::Object(int id) const
{
    const_iterator it = m_objects.find(id);
    return (it != m_objects.end() ?
            static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())) :
            0);
}

template <class T> 
T* Universe::Object(int id)
{
    iterator it = m_objects.find(id);
    return (it != m_objects.end() ?
            static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())) :
            0);
}

template <class T>
std::vector<const T*> Universe::FindObjects() const
{
    std::vector<const T*> retval;
    for (ObjectMap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (const T* obj = static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())))
            retval.push_back(obj);
    }
    return retval;
}

template <class T>
std::vector<T*> Universe::FindObjects()
{
    std::vector<T*> retval;
    for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (T* obj = static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())))
            retval.push_back(obj);
    }
    return retval;
}

template <class T>
Universe::ObjectIDVec Universe::FindObjectIDs() const
{
    Universe::ObjectIDVec retval;
    for (ObjectMap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())))
            retval.push_back(it->first);
    }
    return retval;
}


template <typename EdgeDescriptor>
bool Universe::EdgeVisibilityFilter::operator()(const EdgeDescriptor& edge) const
{
    return m_empire && m_graph ? CanSeeAtLeastOneSystem(m_empire, boost::source(edge, *m_graph), boost::target(edge, *m_graph)) : false;
}

#endif // _Universe_h_
