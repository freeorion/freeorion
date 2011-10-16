// -*- C++ -*-
#ifndef _Universe_h_
#define _Universe_h_

#include "Enums.h"
#include "Predicates.h"
#include "EffectAccounting.h"
#include "ObjectMap.h"
#include "../util/AppInterface.h"

#include <boost/signal.hpp>
#include <boost/unordered_map.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/numeric/ublas/symmetric.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <vector>
#include <list>
#include <map>
#include <string>
#include <set>

#ifdef FREEORION_WIN32
// because the linker gets confused about Win32 API functions...
#  undef GetObject
#  undef GetObjectA
#endif

struct PlayerSetupData;
class Empire;
struct UniverseObjectVisitor;
class XMLElement;
class ShipDesign;
class UniverseObject;
class System;
namespace Condition {
    struct ConditionBase;
    typedef std::vector<const UniverseObject*> ObjectSet;
}

extern const std::size_t RESERVE_SET_SIZE;


/** The Universe class contains the majority of FreeOrion gamestate: All the
  * UniverseObjects in a game, and (of less importance) all ShipDesigns in a
  * game.  (Other gamestate is contained in the Empire class.)
  * The Universe class also provides functions with which to access objects in
  * it, information about the objects, and information about the objects'
  * relationships to each other.  As well, there are functions that generate
  * and populate new Universe gamestates when new games are started. */
class Universe
{
private:
    typedef std::map<int, ObjectMap>                EmpireObjectMap;                ///< Known information each empire had about objects in the Universe; keyed by empire id

public:
    typedef std::map<Visibility, int>               VisibilityTurnMap;              ///< Most recent turn number on which a something, such as a Universe object, was observed at various Visibility ratings or better

private:
    typedef std::map<int, VisibilityTurnMap>        ObjectVisibilityTurnMap;        ///< Most recent turn number on which the objects were observed at various Visibility ratings; keyed by object id
    typedef std::map<int, ObjectVisibilityTurnMap>  EmpireObjectVisibilityTurnMap;  ///< Each empire's most recent turns on which object information was known; keyed by empire id

    typedef std::map<int, std::set<int> >           ObjectKnowledgeMap;             ///< IDs of Empires which know information about an object (or deleted object); keyed by object id

public:
    static const bool ALL_OBJECTS_VISIBLE;                                          ///< Set to true to make everything visible for everyone. Useful for debugging.

    typedef std::map<int, Visibility>               ObjectVisibilityMap;            ///< map from object id to Visibility level for a particular empire
    typedef std::map<int, ObjectVisibilityMap>      EmpireObjectVisibilityMap;      ///< map from empire id to ObjectVisibilityMap for that empire

    typedef std::map<int, ShipDesign*>              ShipDesignMap;                  ///< ShipDesigns in universe; keyed by design id
    typedef ShipDesignMap::const_iterator           ship_design_iterator;           ///< const iterator over ship designs created by players that are known by this client

    /** \name Signal Types */ //@{
    typedef boost::signal<void (const UniverseObject *)> UniverseObjectDeleteSignalType; ///< emitted just before the UniverseObject is deleted
    //@}

    /** \name Structors */ //@{
    Universe();                                     ///< default ctor
    virtual ~Universe();                            ///< dtor
    //@}

    /** \name Accessors */ //@{
    /** Returns objects in this Universe. */
    const ObjectMap&        Objects() const;
    ObjectMap&              Objects();

    /** Returns latest known state of objects for the Empire with
      * id \a empire_id or the true / complete state of all objects in this
      * Universe (the same as calling Objects()) if \a empire_id is
      * ALL_EMPIRES*/
    const ObjectMap&        EmpireKnownObjects(int empire_id = ALL_EMPIRES) const;
    ObjectMap&              EmpireKnownObjects(int empire_id = ALL_EMPIRES);

    /** Returns IDs of objects that the Empire with id \a empire_id has vision
      * of on the current turn, or objects that at least one empire has vision
      * of on the current turn if \a empire_id = ALL_EMPIRES */
    std::set<int>           EmpireVisibleObjectIDs(int empire_id = ALL_EMPIRES) const;

    /** Returns IDs of objects that the Empire with id \a empire_id knows have
      * been destroyed.  Each empire's latest known objects data contains the
      * last known information about each object, whether it has been destroyed
      * or not.  If \a empire_id = ALL_EMPIRES an empty set of IDs is
      * returned. */
    const std::set<int>&    EmpireKnownDestroyedObjectIDs(int empire_id) const;

    const ShipDesign*       GetShipDesign(int ship_design_id) const;                        ///< returns the ship design with id \a ship_design id, or 0 if non exists
    ship_design_iterator    beginShipDesigns() const   {return m_ship_designs.begin();}     ///< returns the begin iterator for ship designs
    ship_design_iterator    endShipDesigns() const     {return m_ship_designs.end();}       ///< returns the end iterator for ship designs

    const ShipDesign*       GetGenericShipDesign(const std::string& name) const;

    /** Returns IDs of ship designs that the Empire with id \a empire_id has
      * seen during the game.  If \a empire_id = ALL_EMPIRES an empty set of
      * ids is returned */
    const std::set<int>&    EmpireKnownShipDesignIDs(int empire_id) const;

    Visibility              GetObjectVisibilityByEmpire(int object_id, int empire_id) const;///< returns the Visibility level of empire with id \a empire_id of UniverseObject with id \a object_id as determined by calling UpdateEmpireObjectVisibilities
    const VisibilityTurnMap&GetObjectVisibilityTurnMapByEmpire(int object_id, int empire_id) const; ///< returns the map from Visibility level to turn number on which the empire with id \a empire_id last had the various Visibility levels of the UniverseObject with id \a object_id .  The returned map may be empty or not have entries for all visibility levels, if the empire has not seen the object at that visibility level yet.

    double                  LinearDistance(int system1_id, int system2_id) const;           ///< returns the straight-line distance between the systems with the given IDs. \throw std::out_of_range This function will throw if either system ID is out of range.

    /** Returns the number of starlane jumps between the systems with the given
      * IDs. If there is no path between the systems, -1 is returned.
      * \throw std::out_of_range This function will throw if either system
      * ID is not a valid system id. */
    short                   JumpDistance(int system1_id, int system2_id) const;

    /** Returns the sequence of systems, including \a system1_id and
      * \a system2_id, that defines the shortest path from \a system1 to
      * \a system2, and the distance travelled to get there.  If no such path
      * exists, the list will be empty.  Note that the path returned may be via
      * one or more starlane, or may be "offroad".  The path is calculated
      * using the visibility for empire \a empire_id, or without regard to
      * visibility if \a empire_id == ALL_EMPIRES.
      * \throw std::out_of_range This function will throw if either system ID
      * is out of range. */
    std::pair<std::list<int>, double>
                            ShortestPath(int system1_id, int system2_id, int empire_id = ALL_EMPIRES) const;

    /** Returns the sequence of systems, including \a system1 and \a system2,
      * that defines the path with the fewest jumps from \a system1 to
      * \a system2, and the number of jumps to get there.  If no such path
      * exists, the list will be empty.  The path is calculated using the
      * visibility for empire \a empire_id, or without regard to visibility if
      * \a empire_id == ALL_EMPIRES.  \throw std::out_of_range This function
      * will throw if either system ID is out of range. */
    std::pair<std::list<int>, int>
                            LeastJumpsPath(int system1_id, int system2_id, int empire_id = ALL_EMPIRES, int max_jumps = INT_MAX) const;
    
    /** Returns whether there is a path known to empire \a empire_id between
      * system \a system1 and system \a system2.  The path is calculated using
      * the visibility for empire \a empire_id, or without regard to visibility
      * if \a empire_id == ALL_EMPIRES.  \throw std::out_of_range This function
      * will throw if either system ID is out of range. */
    bool                    SystemsConnected(int system1_id, int system2_id, int empire_id = ALL_EMPIRES) const;

    /** Returns true iff \a system is reachable from another system (i.e. it
      * has at least one known starlane to it).   This does not guarantee that
      * the system is reachable from any specific other system, as two separate
      * groups of locally but not globally interconnected systems may exist.
      * The starlanes considered depend on their visibility for empire
      * \a empire_id, or without regard to visibility if
      * \a empire_id == ALL_EMPIRES.
      * \throw std::out_of_range This function will throw if the system ID is
      * out of range. */
    bool                    SystemHasVisibleStarlanes(int system_id, int empire_id = ALL_EMPIRES) const;

    /** Returns the systems that are one starlane hop away from system
      * \a system.  The returned systems are indexed by distance from
      * \a system.  The neighborhood is calculated using the visibility
      * for empire \a empire_id, or without regard to visibility if
      * \a empire_id == ALL_EMPIRES.
      * \throw std::out_of_range This function will throw if the  system
      * ID is out of range. */
    std::map<double, int>   ImmediateNeighbors(int system_id, int empire_id = ALL_EMPIRES) const;

    /** Returns map, indexed by object id, to map, indexed by MeterType,
      * to vector of EffectAccountInfo for the meter, in order effects
      * were applied to the meter. */
    const Effect::AccountingMap&            GetEffectAccountingMap() const {return m_effect_accounting_map;}

    /** Returns set of objects that have been marked by the Victory effect
      * to grant their owners victory. */
    const std::multimap<int, std::string>&  GetMarkedForVictory() const {return m_marked_for_victory;}

    mutable UniverseObjectDeleteSignalType UniverseObjectDeleteSignal; ///< the state changed signal object for this UniverseObject
    //@}

    /** \name Mutators */ //@{
    /** Inserts object \a obj into the universe; returns the ID number
      * assigned to the object, or -1 on failure.
      * \note Universe gains ownership of \a obj once it is inserted; the
      * caller should \a never delete \a obj after passing it to Insert(). */
    int             Insert(UniverseObject* obj);

    /** Inserts object \a obj of given ID into the universe; returns true
      * on proper insert, or false on failure.
      * \note Universe gains ownership of \a obj once it is inserted; the
      * caller should \a never delete \a obj after
      * passing it to InsertID().
      * Useful mostly for times when ID needs to be consistent on client
      * and server */
    bool            InsertID(UniverseObject* obj, int id);

    /** Inserts \a ship_design into the universe; returns the ship design ID
      * assigned to it, or -1 on failure.
      * \note Universe gains ownership of \a ship_design once inserted. */
    int             InsertShipDesign(ShipDesign* ship_design);

    /** Inserts \a ship_design into the universe with given \a id;  returns
      * true on success, or false on failure.
      * \note Universe gains ownership of \a ship_design once inserted. */
    bool            InsertShipDesignID(ShipDesign* ship_design, int id);

    /** Generates systems and planets, assigns homeworlds and populates them
      * with people, industry and bases, and places starting fleets.  Uses
      * predefined galaxy shapes. */
    void            CreateUniverse(int size, Shape shape,
                                   GalaxySetupOption age, GalaxySetupOption starlane_freq,
                                   GalaxySetupOption planet_density, GalaxySetupOption specials_freq,
                                   GalaxySetupOption monster_freq, GalaxySetupOption native_freq,
                                   const std::map<int, PlayerSetupData>& player_setup_data);

    /** Clears main ObjectMap, empires' latest known objects map, and
      * ShipDesign map. */
    void            Clear();

    /** Determines all effectsgroups' target sets, then resets meters and
      * executes all effects on all objects.  Then clamps meter values so
      * target and max meters are within a reasonable range and any current
      * meters with associated max meters are limited by their max. */
    void            ApplyAllEffectsAndUpdateMeters();

    /** Determines all effectsgroups' target sets, then eesets meters and
      * executes only SetMeter effects on all objects whose ids are listed in
      * \a object_ids.  Then clamps meter values so target and max meters are
      * within a reasonable range and any current meters with associated max
      * meters are limited by their max. */
    void            ApplyMeterEffectsAndUpdateMeters(const std::vector<int>& object_ids);

    /** Calls above ApplyMeterEffectsAndUpdateMeters() function on all objects.*/
    void            ApplyMeterEffectsAndUpdateMeters();


    /** For all objects and meters, determines discrepancies between actual meter
      * maxes and what the known universe should produce, and and stores in
      * m_effect_discrepancy_map. */
    void            InitMeterEstimatesAndDiscrepancies();

    /** Based on (known subset of, if in a client) universe and any orders
      * given so far this turn, updates estimated meter maxes for next turn
      * for the objects with ids indicated in \a objects_vec. */
    void            UpdateMeterEstimates(const std::vector<int>& objects_vec);

    /** Updates indicated object's meters, and if applicable, the
      * meters of objects contained within the indicated object.
      * If \a object_id is UniverseObject::INVALID_OBJECT_ID, then all
      * objects' meters are updated. */
    void            UpdateMeterEstimates(int object_id, bool update_contained_objects = false);

    /** Updates all meters for all (known) objects */
    void            UpdateMeterEstimates();

    /** Sets all objects' meters' initial values to their current values. */
    void            BackPropegateObjectMeters();

    /** Sets indicated objects' meters' initial values to their current values. */
    void            BackPropegateObjectMeters(const std::vector<int>& object_ids);

    /** Determines which empires can see which objects at what visibility
      * level, based on  */
    void            UpdateEmpireObjectVisibilities();

    /** Stores latest known information about each object for each empire and
      * updates the record of the last turn on which each empire has visibility
      * of object that can be seen on the current turn with the level of
      * visibility that the empire has this turn. */
    void            UpdateEmpireLatestKnownObjectsAndVisibilityTurns();

    /** Reconstructs the per-empire system graph views needed to calculate
      * routes based on visibility. */
    void            RebuildEmpireViewSystemGraphs(int for_empire_id = ALL_EMPIRES);

    /** Adds the object ID \a object_id to the set of object ids for the empire
      * with id \a empire_id that the empire knows have been destroyed. */
    void            SetEmpireKnowledgeOfDestroyedObject(int object_id, int empire_id);

    /** Adds the ship design ID \a ship_design_id to the set of ship design ids
      * known by the empire with id \a empire_id */
    void            SetEmpireKnowledgeOfShipDesign(int ship_design_id, int empire_id);

    /** Removes the object with ID number \a object_id from the universe's map
      * of existing objects, and adds the object's id to the set of destroyed
      * object ids.  If \a update_destroyed_object_knowers is true, empires
      * that currently have visibility of the object have its id added to
      * their set of objects' ids that are known to have been destroyed.  Older
      * or limited versions of objects remain in empires latest known objects
      * ObjectMap, regardless of whether the empire knows the object is
      * destroyed. */
    void            Destroy(int object_id, bool update_destroyed_object_knowers = true);

    /** Destroys object with ID \a object_id, and destroys any associted
      * objects, such as contained buildings of planets, contained anything of
      * systems, or fleets if their last ship has id \a object_id and the fleet
      * is thus empty. */
    void            RecursiveDestroy(int object_id);

    /** Used by the Destroy effect to mark an object for destruction later
      * during turn processing. (objects can't be destroyed immediately as
      * other effects might depend on their existence) */
    void            EffectDestroy(int object_id);

    /** Permanently deletes object with ID number \a object_id.  no
      * information about this object is retained in the Universe.  Can be
      * performed on objects whether or not the have been destroyed.  Returns
      * true if such an object was found, false otherwise. */
    bool            Delete(int object_id);

    /** Cleans up internal storage of now-invalidated empire ID. */
    void            HandleEmpireElimination(int empire_id);

    /** Used by the Victory effect to mark an object to give it owner victory. */
    void            EffectVictory(int object_id, const std::string& reason_string);

    /** Sets whether to inhibit UniverseObjectSignals.  Inhibits if \a inhibit
      * is true, and (re)enables UniverseObjectSignals if \a inhibit is false. */
    static void     InhibitUniverseObjectSignals(bool inhibit = true);
    //@}

    /** Returns the size of the galaxy map.  Does not measure absolute
      * distances; the ratio between map coordinates and actual distance varies
      * depending on universe size. */
    static double   UniverseWidth();

    /** Generates an object ID for a future object. Usually used by the server
      * to service new ID requests. */
    int             GenerateObjectID();

    /** Generates design ID for a new (ship) design. Usually used by the
      * server to service new ID requests. */
    int             GenerateDesignID();

    typedef std::vector<std::vector<std::set<System*> > > AdjacencyGrid;

    /** Returns true if UniverseOjbectSignals are inhibited, false otherwise. */
    static const bool&  UniverseObjectSignalsInhibited();

    /** HACK! This must be set to the encoding empire's id when serializing a
      * Universe, so that only the relevant parts of the Universe are
      * serialized.  The use of this global variable is done just so I don't
      * have to rewrite any custom boost::serialization classes that implement
      * empire-dependent visibility. */
    static int                      s_encoding_empire;

private:
    template <typename T>
    class distance_matrix
    {
    public:
        typedef boost::numeric::ublas::symmetric_matrix<
            T,
            boost::numeric::ublas::lower
        > storage_type;

        struct const_row_ref
        {
            const_row_ref(std::size_t i, const storage_type& m) : m_i(i), m_m(m) {}
            T operator[](std::size_t j) const { return m_m(m_i, j); }
        private:
            std::size_t m_i;
            const storage_type& m_m;
        };

        struct row_ref
        {
            row_ref(std::size_t i, storage_type& m) : m_i(i), m_m(m) {}
            T& operator[](std::size_t j) { return m_m(m_i, j); }
        private:
            std::size_t m_i;
            storage_type& m_m;
        };

        distance_matrix() :
            m_m()
        {}

        const_row_ref operator[](std::size_t i) const
        { return const_row_ref(i, m_m); }

        row_ref operator[](std::size_t i)
        { return row_ref(i, m_m); }

        void resize(std::size_t rows, std::size_t columns)
        { m_m.resize(rows, columns); }

    private:
        storage_type m_m;
    };

    typedef distance_matrix<double> DistanceMatrix;
    typedef distance_matrix<short> JumpsMatrix;

    struct GraphImpl;

    /** Clears \a effects_targets_map, and then populates with all
      * EffectsGroups and their targets in the known universe. */
    void    GetEffectsAndTargets(Effect::TargetsCausesMap& effects_targets_map);

    /** Removes entries in \a effects_targets_map about effects groups acting
      * on objects in \a target_objects, and then repopulates for EffectsGroups
      * that act on at least one of the objects in \a target_objects. */
    void    GetEffectsAndTargets(Effect::TargetsCausesMap& targets_causes_map, const std::vector<int>& target_objects);

    /** Used by GetEffectsAndTargets to process a vector of effects groups.
      * Stores target set of specified \a effects_groups and \a source_object_id
      * in \a targets_causes_map 
      * NOTE: this method will modify target_objects temporarily, but restore
      * its contents before returning. */
    void    StoreTargetsAndCausesOfEffectsGroups(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups,
                                                 int source_object_id, EffectsCauseType effect_cause_type,
                                                 const std::string& specific_cause_name,
                                                 Effect::TargetSet& target_objects, Effect::TargetsCausesMap& targets_causes_map);

    /** Executes all effects.  For use on server when processing turns.
      * If \a only_meter_effects is true, then only SetMeter effects are
      * executed.  This is useful on server or clients to update meter
      * values after the rest of effects (including non-meter effects) have
      * been executed. */
    void    ExecuteEffects(const Effect::TargetsCausesMap& targets_causes_map,
                           bool update_effect_accounting, bool only_meter_effects = false);

    /** Does actual updating of meter estimates after the public function have
      * processed objects_vec or whatever they were passed and cleared the
      * relevant effect accounting for those objects and meters. */
    void    UpdateMeterEstimatesImpl(const std::vector<int>& objects_vec);


    /** Generates planets for all systems that have empty object maps (ie those
      * that aren't homeworld systems).*/
    void    PopulateSystems(GalaxySetupOption density);

    /** Adds start-of-game specials to objects. */
    void    AddStartingSpecials(GalaxySetupOption specials_freq);

    /** Adds non-empire-affiliated native populations to planets. */
    void    GenerateNatives(GalaxySetupOption freq);

    /** Adds space monsters to systems. */
    void    GenerateSpaceMonsters(GalaxySetupOption freq);

    /** Creates starlanes and adds them systems already generated. */
    void    GenerateStarlanes(GalaxySetupOption freq, const AdjacencyGrid& adjacency_grid);

    /** Resizes the system graph to the appropriate size and populates
      * m_system_distances.  Uses the Universe latest known set of objects for
      * the empire with id \a for_empire_id or uses the main / true / visible
      * objects if \a for_empire_id is ALL_EMPIRES*/
    void    InitializeSystemGraph(int for_empire_id = ALL_EMPIRES);

    /** Picks systems to host homeworlds, generates planets for them, stores
      * the ID's of the homeworld planets into the homeworld vector. */
    void    GenerateHomeworlds(int players, std::vector<int>& homeworld_planet_ids);

    /** Names the planets in each system, based on the system's name. */
    void    NamePlanets();

    /** Creates Empires objects for each entry in \a player_setup_data with
      * empire id equal to the specified player ids (so that the calling code
      * can know which empire belongs to which player).  Homeworlds are
      * associated with the empires, and starting buildings and fleets are
      * created, and empire starting ship designs are created and added. */
    void    GenerateEmpires(std::vector<int>& homeworld_planet_ids, const std::map<int, PlayerSetupData>& player_setup_data);

    ObjectMap                       m_objects;                          ///< map from object id to UniverseObjects in the universe.  for the server: all of them, up to date and true information about object is stored;  for clients, only limited information based on what the client knows about is sent.
    EmpireObjectMap                 m_empire_latest_known_objects;      ///< map from empire id to (map from object id to latest known information about each object by that empire)

    EmpireObjectVisibilityMap       m_empire_object_visibility;         ///< map from empire id to (map from object id to visibility of that object for that empire)
    EmpireObjectVisibilityTurnMap   m_empire_object_visibility_turns;   ///< map from empire id to (map from object id to (map from Visibility rating to turn number on which the empire last saw the object at the indicated Visibility rating or higher)

    ObjectKnowledgeMap              m_empire_known_destroyed_object_ids;///< map from empire id to (set of object ids that the empire knows have been destroyed)

    ShipDesignMap                   m_ship_designs;                     ///< ship designs in the universe
    std::map<int, std::set<int> >   m_empire_known_ship_design_ids;     ///< ship designs known to each empire

    DistanceMatrix                  m_system_distances;                 ///< the straight-line distances between all the systems
    JumpsMatrix                     m_system_jumps;                     ///< the least-jumps distances between all the systems
    GraphImpl*                      m_graph_impl;                       ///< a graph in which the systems are vertices and the starlanes are edges
    boost::unordered_map<int, int>  m_system_id_to_graph_index;

    Effect::AccountingMap           m_effect_accounting_map;            ///< map from target object id, to map from target meter, to orderered list of structs with details of an effect and what it does to the meter
    Effect::DiscrepancyMap          m_effect_discrepancy_map;           ///< map from target object id, to map from target meter, to discrepancy between meter's actual initial value, and the initial value that this meter should have as far as the client can tell: the unknown factor affecting the meter

    int                             m_last_allocated_object_id;
    int                             m_last_allocated_design_id;

    std::set<int>                   m_marked_destroyed;                 ///< used while applying effects to cache objects that have been destroyed.  this allows to-be-destroyed objects to remain undestroyed until all effects have been processed, which ensures that to-be-destroyed objects still exist when other effects need to access them as a source object
    std::multimap<int, std::string> m_marked_for_victory;               ///< used while applying effects to cache objects whose owner should be victorious.  Victory testing is done separately from effects execution, so this needs to be stored temporarily...

    static double                   s_universe_width;
    static bool                     s_inhibit_universe_object_signals;

    /** Fills \a designs_to_serialize with ShipDesigns known to the empire with
      * the ID \a encoding empire.  If encoding_empire is ALL_EMPIRES, then all
      * designs are included. */
    void    GetShipDesignsToSerialize(ShipDesignMap& designs_to_serialize, int encoding_empire) const;

    /** Fills \a objects with copies of UniverseObjects that should be sent
      * to the empire with id \a encoding_empires */
    void    GetObjectsToSerialize(ObjectMap& objects, int encoding_empire) const;

    /** Fills \a empire_latest_known_objects map with the latest known data
      * about UniverseObjects for the empire with id \a encoding_empire.  If
      * the encoding empire is ALL_EMPIRES then all stored empire object
      * knowledge is included. */
    void    GetEmpireKnownObjectsToSerialize(EmpireObjectMap& empire_latest_known_objects, int encoding_empire) const;

    /***/
    void    GetEmpireObjectVisibilityMap(EmpireObjectVisibilityMap& empire_object_visibility, int encoding_empire) const;

    /***/
    void    GetEmpireObjectVisibilityTurnMap(EmpireObjectVisibilityTurnMap& empire_object_visibility_turns, int encoding_empire) const;

    /***/
    void    GetEmpireKnownDestroyedObjects(ObjectKnowledgeMap& m_empire_known_destroyed_object_ids, int encoding_empire) const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Free function UniverseObject getters. */
UniverseObject*         GetObject(int object_id);
UniverseObject*         GetEmpireKnownObject(int object_id, int empire_id);

template <class T>
T*                      GetObject(int object_id)
{ return GetUniverse().Objects().template Object<T>(object_id); }

template <class T>
T*                      GetEmpireKnownObject(int object_id, int empire_id)
{ return GetUniverse().EmpireKnownObjects(empire_id).template Object<T>(object_id); }


/** A combination of names of ShipDesign that can be put together to make a
  * fleet of ships, and a name for such a fleet, loaded from starting_fleets.txt
  * ShipDesign names refer to designs listed in premade_ship_designs.txt.
  * Useful for saving or specifying prearranged combinations of prearranged
  * ShipDesigns to automatically put together, such as during universe creation.*/
class FleetPlan
{
public:
    FleetPlan(const std::string& fleet_name, const std::vector<std::string>& ship_design_names,
              bool lookup_name_userstring = false);
    FleetPlan();
    virtual ~FleetPlan();
    const std::string&              Name() const;
    const std::vector<std::string>& ShipDesigns() const;
protected:
    std::string                     m_name;
    std::vector<std::string>        m_ship_designs;
    bool                            m_name_in_stringtable;
};

/** The combination of a FleetPlan and spawning instructions for start of game
  * monsters. */
class MonsterFleetPlan : public FleetPlan
{
public:
    MonsterFleetPlan(const std::string& fleet_name, const std::vector<std::string>& ship_design_names,
                     double spawn_rate = 1.0, int spawn_limit = 9999, const Condition::ConditionBase* location = 0,
                     bool lookup_name_userstring = false);
    MonsterFleetPlan();
    virtual ~MonsterFleetPlan();
    double                          SpawnRate() const;
    int                             SpawnLimit() const;
    const Condition::ConditionBase* Location() const;
protected:
    double                          m_spawn_rate;
    int                             m_spawn_limit;
    const Condition::ConditionBase* m_location;
};

#endif // _Universe_h_
