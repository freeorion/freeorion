// -*- C++ -*-
#ifndef _Universe_h_
#define _Universe_h_

#ifndef _SitRepEntry_h_
#include "../util/SitRepEntry.h"
#endif

#ifndef BOOST_SIGNAL_HPP
#include <boost/signal.hpp>
#endif

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

#include <vector>
#include <list>
#include <map>
#include <string>
#include <set>

class System;
class UniverseObject;
#ifdef __GNUC__
  // GCC doesn't allow us to forward-declare PlayerSetupData
#  ifndef _MultiplayerCommon_h_
#    include "util/MultiplayerCommon.h"
#  endif
#else
  struct PlayerSetupData;
#endif
namespace GG {class XMLElement;}

#include "GGEnum.h"

// this is here to break a circular dependendy between Planet and ProdCenter, both of which need this enum
/** the types of planets in FreeOrion*/
enum PlanetType {PT_SWAMP,
                PT_TOXIC,
                PT_INFERNO,
                PT_RADIATED,
                PT_BARREN,
                PT_TUNDRA,
                PT_DESERT,
                PT_TERRAN,                   //changed the order to be clockwise around the 
                PT_OCEAN,                    // wheel of EP, added Inferno and Swamp types
                PT_GAIA,
                PT_ASTEROIDS,                //these need to be types also so they can have an environment
                PT_GASGIANT,                     
                MAX_PLANET_TYPE   //keep this last
                };

// define EnumMap and stream operators for PlanetType
// this is done because these labels are used in some artist-created XML files
ENUM_MAP_BEGIN(PlanetType)
    ENUM_MAP_INSERT(PT_SWAMP)
    ENUM_MAP_INSERT(PT_TOXIC)
    ENUM_MAP_INSERT(PT_INFERNO)
    ENUM_MAP_INSERT(PT_RADIATED)
    ENUM_MAP_INSERT(PT_BARREN)
    ENUM_MAP_INSERT(PT_TUNDRA)
    ENUM_MAP_INSERT(PT_DESERT)
    ENUM_MAP_INSERT(PT_TERRAN)
    ENUM_MAP_INSERT(PT_OCEAN)
    ENUM_MAP_INSERT(PT_GAIA)
    ENUM_MAP_INSERT(PT_ASTEROIDS)
    ENUM_MAP_INSERT(PT_GASGIANT)
ENUM_MAP_END

ENUM_STREAM_IN(PlanetType)
ENUM_STREAM_OUT(PlanetType)


class Universe
{
protected:
    typedef std::map<int, UniverseObject*> ObjectMap; ///< the container type that is used internally to hold the objects in the universe; keyed by ID number

public:
    /** the types of universe shapes available in FreeOrion*/
    enum Shape {SPIRAL_2,      ///< a two-armed spiral galaxy
                SPIRAL_3,      ///< a three-armed spiral galaxy
                SPIRAL_4,      ///< a four-armed spiral galaxy
                CLUSTER,       ///< a cluster galaxy
                ELLIPTICAL,    ///< an elliptical galaxy
                IRREGULAR,     ///< an irregular galaxy
				RING,          ///< a ring galaxy
                GALAXY_SHAPES  ///< the number of shapes in this enum (leave this last)
               };

    /** types of Univervse ages*/
    enum Age {AGE_YOUNG,
              AGE_MATURE,
              AGE_ANCIENT,
              NUM_UNIVERSE_AGES    // keep this last, the number of universe age options
             };

    /** types of Planet Density */
    enum PlanetDensity {PD_LOW,
                        PD_AVERAGE,
                        PD_HIGH,
                        NUM_UNIVERSE_PLANET_DENSITIES        //keep this last, the number of planet density options
                       };

    /** types of starlane frequencies */
    enum StarlaneFreqency {LANES_NONE, 
                           LANES_FEW, 
                           LANES_SOME, 
                           LANES_SEVERAL, 
                           LANES_MANY, 
                           LANES_VERY_MANY,
                           NUM_STARLANE_FREQENCIES    // keep this last, the number of starlane frequency options
                          };

    /** types of starlane frequencies */
    enum SpecialsFreqency {SPECIALS_NONE, 
                           SPECIALS_RARE, 
                           SPECIALS_UNCOMMON, 
                           SPECIALS_COMMON, 
                           NUM_SPECIALS_FREQENCIES    // keep this last, the number of specials frequency options
                          };

    /** the value passed to XMLEncode() when the entire object is desired, not just the portion visible to one empire */
    enum {ALL_EMPIRES = -1};

    typedef ObjectMap::const_iterator            const_iterator;   ///< a const_iterator for iteration over the objects in the universe
    typedef ObjectMap::iterator                  iterator;

    typedef std::vector<const UniverseObject*>   ConstObjectVec;   ///< the return type of FindObjects()
    typedef std::vector<UniverseObject*>         ObjectVec;        ///< the return type of the non-const FindObjects()
    typedef std::vector<int>                     ObjectIDVec;      ///< the return type of FindObjectIDs()

    /** \name Signal Types */ //@{
    typedef boost::signal<void (const UniverseObject *)> UniverseObjectDeleteSignalType; ///< emitted just before the UniverseObject is deleted
    //@}


    /** \name Structors */ //@{
    Universe(); ///< default ctor
    Universe(const GG::XMLElement& elem); ///< ctor that constructs a Universe object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Universe object
    virtual ~Universe(); ///< dtor
    //@}

    /** \name Accessors */ //@{
    const UniverseObject* Object(int id) const; ///< returns a pointer to the universe object with ID number \a id, or 0 if none exists
    UniverseObject* Object(int id);  ///< returns a pointer to the universe object with ID number \a id, or 0 if none exists

    template <class T> const T* Object(int id) const; ///< returns a pointer to the object of type T with ID number \a id. Returns 0 if none exists or the object with ID \a id is not of type T.
    template <class T> T* Object(int id);  ///< returns a pointer to the object of type T with ID number \a id. Returns 0 if none exists or the object with ID \a id is not of type T.

    /** returns all the objects that match \a pred.  Predicates used with this function must take a single const
        UniverseObject* parameter and must return a bool or a type for which there is a conversion to bool.*/
    template <class Pred> ConstObjectVec FindObjects(Pred pred) const;

    /** returns all the objects that match \a pred.  Predicates used with this function must take a single
        UniverseObject* parameter and must return a bool or a type for which there is a conversion to bool.*/
    template <class Pred> ObjectVec FindObjects(Pred pred);

    /** returns all the objects of type T. */
    template <class T> std::vector<const T*> FindObjects() const;

    /** returns all the objects of type T. */
    template <class T> std::vector<T*> FindObjects();

    /** returns the IDs of all the objects that match \a pred.  Predicates used with this function must take a single const
    UniverseObject* parameter and must return a bool or a type for which there is a conversion to bool.*/
    template <class Pred> ObjectIDVec FindObjectIDs(Pred pred) const;

    /** returns the IDs of all the objects of type T. */
    template <class T> ObjectIDVec FindObjectIDs() const;

    iterator begin() { return m_objects.begin();}
    iterator end() { return m_objects.end();}

    const_iterator begin() const  {return m_objects.begin();}   ///< returns the begin const_iterator for the objects in the universe
    const_iterator end() const    {return m_objects.end();}     ///< returns the end const_iterator for the objects in the universe

    double LinearDistance(System* system1, System* system2) const; ///< returns the straight-line distance between the given systems
    double LinearDistance(int system1, int system2) const; ///< returns the straight-line distance between the systems with the given IDs. \throw std::out_of_range This function will throw if either system ID is out of range.

    /** returns the sequence of systems, including \a system1 and \a system2, that defines the shortest path from \a system1 to 
        \a system2, and the distance travelled to get there.  If no such path exists, the list will be empty.  Note that the 
        path returned may be via one or more starlane, or may be "offroad". */
    std::pair<std::list<System*>, double> ShortestPath(System* system1, System* system2) const;

    /** returns the sequence of systems, including \a system1 and \a system2, that defines the shortest path from \a system1 to 
        \a system2, and the distance travelled to get there.  If no such path exists, the list will be empty.  Note that the 
        path returned may be via one or more starlane, or may be "offroad".  \throw std::out_of_range This function will throw 
        if either system ID is out of range. */
    std::pair<std::list<System*>, double> ShortestPath(int system1, int system2) const;

    /** returns the systems that are one starlane hop away from system \a system.  The returned systems are indexed by 
        distance from \a system. */
    std::map<double, System*> ImmediateNeighbors(System* system) const;

    /** returns the systems that are one starlane hop away from system \a system.  The returned systems are indexed by 
        distance from \a system.  \throw std::out_of_range This function will throw if the system ID is out of range. */
    std::map<double, System*> ImmediateNeighbors(int system) const;

    virtual GG::XMLElement XMLEncode(int empire_id = ALL_EMPIRES) const; ///< constructs an XMLElement from a Universe object with visibility restrictions for the given empire

    UniverseObjectDeleteSignalType& UniverseObjectDeleteSignal() const {return m_universe_object_delete_sig;} ///< returns the state changed signal object for this UniverseObject
    //@}

    /** \name Mutators */ //@{
    void SetUniverse(const GG::XMLElement& elem ); ///< wipes out the current object map and sets the map to the XMLElement passed in.

    /** inserts object \a obj into the universe; returns the ID number assigned to the object, or -1 on failure.
    \note Universe gains ownership of \a obj once it is inserted; the caller should \a never delete \a obj after
    passing it to Insert().*/
    int               Insert(UniverseObject* obj);

    /** inserts object \a obj of given ID into the universe; returns true on proper insert , or false on failure.
    \note Universe gains ownership of \a obj once it is inserted; the caller should \a never delete \a obj after
    passing it to InsertID().
    Useful mostly for times when ID needs to be consistant on client and server*/
    bool               InsertID(UniverseObject* obj, int id );

    /** generates systems and planets, assigns homeworlds and populates them with people, industry and bases, and places starting fleets.  Uses predefined galaxy shapes.  */
    void              CreateUniverse(int size, Shape shape, Age age, StarlaneFreqency starlane_freq, PlanetDensity planet_density, 
                                     SpecialsFreqency specials_freq, int players, int ai_players, 
                                     const std::vector<PlayerSetupData>& player_setup_data = std::vector<PlayerSetupData>());

    /** removes the object with ID number \a id from the universe, and returns it; returns 0 if there is no such object*/
    UniverseObject*   Remove(int id);

    /** removes the object with ID number \a id; returns true if such an object was found, false otherwise*/
    bool              Delete(int id);

    /** moves UniverseObjects that are in movement or are starting movement as a result of orders this turn.  This must be
    called after all movement Orders have been processed but before combats are resolved.  Any SitRepEntrys that
    are generated are pushed onto the end of \a sit_reps.*/
    void MovementPhase(std::vector<SitRepEntry>& sit_reps);

    /** grows pop, and executes production and research.  This must be called after all Orders have been processed on the
    Universe and all combats are resolved.  Any SitRepEntrys that are generated are pushed onto the end of \a sit_reps.*/
    void PopGrowthProductionResearch(std::vector<SitRepEntry>& sit_reps);
    //@}

    /** returns the size of the galaxy map.  Does not measure absolute distances; the ratio between map coordinates and actual distance varies
    depending on universe size */
    static double UniverseWidth() {return s_universe_width;}

    int GenerateObjectID( );  ///< generates an object ID for a future object. Usually used by the server to service new ID requests

    typedef std::vector<std::vector<std::set<System*> > > AdjacencyGrid;

protected:
    typedef std::vector< std::vector<double> > DistanceMatrix;

    // declare property map types
    struct vertex_system_pointer_t {typedef boost::vertex_property_tag kind;};
    typedef boost::property<vertex_system_pointer_t, System*,
                            boost::property<boost::vertex_index_t, int> > 
                            vertex_property_t;
    typedef boost::property<boost::edge_weight_t, double> 
                            edge_property_t;

    // declare main graph types, including properties declared above
    typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::undirectedS, 
                                   vertex_property_t, edge_property_t>
                                   SystemGraph;

    // declare types for iteration over graph
    typedef SystemGraph::vertex_iterator   VertexIterator;
    typedef SystemGraph::out_edge_iterator OutEdgeIterator;

    // declare property map types for properties declared above
    typedef boost::property_map<SystemGraph, vertex_system_pointer_t>::const_type ConstSystemPointerPropertyMap;
    typedef boost::property_map<SystemGraph, vertex_system_pointer_t>::type       SystemPointerPropertyMap;
    typedef boost::property_map<SystemGraph, boost::vertex_index_t>::const_type   ConstIndexPropertyMap;
    typedef boost::property_map<SystemGraph, boost::vertex_index_t>::type         IndexPropertyMap;
    typedef boost::property_map<SystemGraph, boost::edge_weight_t>::const_type    ConstEdgeWeightPropertyMap;
    typedef boost::property_map<SystemGraph, boost::edge_weight_t>::type          EdgeWeightPropertyMap;

    // factory class
    template <class T> class NumberedElementFactory
    {
    public:
        typedef T* (*Generator)(const GG::XMLElement&); ///< this defines the function signature for object generators

        /** \name Structors */ //@{
        NumberedElementFactory() {} ///< ctor
        //@}

        /** Generates objects whose tag name contains but is not limited to the generator name  */ //@{
        T* GenerateObject(const GG::XMLElement& elem) const ///< returns a heap-allocated subclass object of the appropriate type
        {
            T* retval = 0;

            for ( typename std::map<std::string, Generator>::const_iterator it = m_generators.begin(); it != m_generators.end(); ++it )
            {
                // is the string anywhere in the tag?
                std::string tag_name = elem.Tag();
                std::string tag_alpha = tag_name.substr(0, tag_name.find_first_of("0123456789") );

                if ( tag_alpha == it->first )
                {
                    retval = it->second(elem);
                    break;
                }
            }
            return retval;
        }

        /** \name Mutators */ //@{
        /** adds (or overrides) a new generator that can generate subclass objects described by \a name */
        void AddGenerator(const std::string& name, Generator gen) {m_generators[name] = gen;}
        //@}

    private:
        /** mapping from strings to functions that can create the type of object that corresponds to the string */
        std::map<std::string, Generator> m_generators;
    };

    void GenerateIrregularGalaxy(int stars, Age age, AdjacencyGrid& adjacency_grid);   ///< creates an irregular galaxy and stores the empire homeworlds in the homeworlds vector

    void PopulateSystems(Universe::PlanetDensity density);  ///< Will generate planets for all systems that have empty object maps (ie those that aren't homeworld systems)
    void GenerateStarlanes(StarlaneFreqency freq, const AdjacencyGrid& adjacency_grid); ///< creates starlanes and adds them systems already generated
    void InitializeSystemGraph(); ///< resizes the system graph to the appropriate size and populates m_system_distances 
    void GenerateHomeworlds(int players, std::vector<int>& homeworlds);  ///< Picks systems to host homeworlds, generates planets for them, stores the ID's of the homeworld planets into the homeworld vector

    /// Will create empire objects, assign them homeworlds, setup the homeworld population, industry, and starting fleets
    /// NOTE: does nothing if executed client-side. This is a hack to deal with the
    /// dependency on ServerEmpireManager -- jdb
    void GenerateEmpires(int players, std::vector<int>& homeworlds, const std::vector<PlayerSetupData>& player_setup_data);

    ObjectMap m_objects;                                ///< note that for the system graph algorithms to work more easily, the first N elements should be the N systems
    DistanceMatrix m_system_distances;                  ///< the straight-line distances between all the systems; this is an lower-triangular matrix, so only access the elements in (highID, lowID) order
    SystemGraph m_system_graph;                         ///< a graph in which the systems are vertices and the starlanes are edges
    NumberedElementFactory<UniverseObject> m_factory;   ///< generates new object IDs for all new objects
    int m_last_allocated_id;

    static double s_universe_width;

    mutable UniverseObjectDeleteSignalType m_universe_object_delete_sig;
};


// template implementations
template <class T> 
const T* Universe::Object(int id) const
{
    const_iterator it = m_objects.find(id);
    return (it != m_objects.end() ? dynamic_cast<const T*>(it->second) : 0);
}

template <class T> 
T* Universe::Object(int id)
{
    const_iterator it = m_objects.find(id);
    return (it != m_objects.end() ? dynamic_cast<T*>(it->second) : 0);
}

template <class Pred>
Universe::ConstObjectVec Universe::FindObjects(Pred pred) const
{
    ConstObjectVec retval;
    for (ObjectMap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        const UniverseObject* o = it->second;
        if (pred(o))
            retval.push_back(o);
    }
    return retval;
}

template <class Pred>
Universe::ObjectVec Universe::FindObjects(Pred pred)
{
    ObjectVec retval;
    for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        UniverseObject* o = it->second;
        if (pred(o))
            retval.push_back(o);
    }
    return retval;
}

template <class T>
std::vector<const T*> Universe::FindObjects() const
{
    std::vector<const T*> retval;
    for (ObjectMap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (const T* t = dynamic_cast<T*>(it->second))
            retval.push_back(t);
    }
    return retval;
}

template <class T>
std::vector<T*> Universe::FindObjects()
{
    std::vector<T*> retval;
    for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (T* t = dynamic_cast<T*>(it->second))
            retval.push_back(t);
    }
    return retval;
}

template <class Pred>
Universe::ObjectIDVec Universe::FindObjectIDs(Pred pred) const
{
    ObjectIDVec retval;
    for (ObjectMap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        const UniverseObject* o = it->second;
        if (pred(o))
            retval.push_back(it->first);
    }
    return retval;
}

template <class T>
Universe::ObjectIDVec Universe::FindObjectIDs() const
{
    Universe::ObjectIDVec retval;
    for (ObjectMap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (dynamic_cast<T*>(it->second))
            retval.push_back(it->first);
    }
    return retval;
}

#endif // _Universe_h_
