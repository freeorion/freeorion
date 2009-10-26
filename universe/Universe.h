// -*- C++ -*-
#ifndef _Universe_h_
#define _Universe_h_

#include "Enums.h"
#include "Predicates.h"

#include <boost/signal.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <vector>
#include <list>
#include <map>
#include <string>
#include <set>

struct PlayerSetupData;
class Empire;
struct UniverseObjectVisitor;
class XMLElement;
class ShipDesign;
class UniverseObject;
class System;
namespace Condition {
    typedef std::set<UniverseObject*> ObjectSet;
}
namespace Effect {
    class EffectsGroup;
}

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
    typedef std::map<int, UniverseObject*>          ObjectMap;                      ///< Objects in universe; keyed by id
    typedef std::map<int, ObjectMap>                EmpireLatestKnownObjectMap;     ///< Most recent known information each empire had about objects in the Universe; keyed by empire id

    typedef std::map<Visibility, int>               VisibilityTurnMap;              ///< Most recent turn number on which a something, such as a Universe object, was observed at various Visibility ratings or better
    typedef std::map<int, VisibilityTurnMap>        ObjectVisibilityTurnMap;        ///< Most recent turn number on which the objects were observed at various Visibility ratings; keyed by object id
    typedef std::map<int, ObjectVisibilityTurnMap>  EmpireObjectVisibilityTurnMap;  ///< Each empire's most recent turns on which object information was known; keyed by empire id

    typedef std::map<int, std::set<int> >           ObjectKnowledgeMap;             ///< IDs of Empires which know information about an object (or deleted object); keyed by object id

    typedef std::map<int, ShipDesign*>              ShipDesignMap;                  ///< ShipDesigns in universe; keyed by design id

public:
    static const bool ALL_OBJECTS_VISIBLE;                                          ///< Set to true to make everything visible for everyone. Useful for debugging.

    typedef ObjectMap::const_iterator               const_iterator;                 ///< a const_iterator for sequences over the objects in the universe
    typedef ObjectMap::iterator                     iterator;                       ///< an iterator for sequences over the objects in the universe

    typedef ShipDesignMap::const_iterator           ship_design_iterator;           ///< const iterator over ship designs created by players that are known by this client

    typedef std::vector<const UniverseObject*>      ConstObjectVec;                 ///< the return type of FindObjects()
    typedef std::vector<UniverseObject*>            ObjectVec;                      ///< the return type of the non-const FindObjects()
    typedef std::vector<int>                        ObjectIDVec;                    ///< the return type of FindObjectIDs()

    typedef std::map<int, Visibility>               ObjectVisibilityMap;            ///< map from object id to Visibility level for a particular empire
    typedef std::map<int, ObjectVisibilityMap>      EmpireObjectVisibilityMap;      ///< map from empire id to ObjectVisibilityMap for that empire


    /** Combination of an EffectsGroup and the id of a source object. */
    struct SourcedEffectsGroup {
        SourcedEffectsGroup();
        SourcedEffectsGroup(int source_object_id_, const boost::shared_ptr<const Effect::EffectsGroup>& effects_group_);
        bool operator<(const SourcedEffectsGroup& right) const;
        int                                             source_object_id;
        boost::shared_ptr<const Effect::EffectsGroup>   effects_group;
    };
    /** Description of cause of an effect: the general cause type, and the
      * specific cause.  eg. Building and a particular BuildingType. */
    struct EffectCause {
        EffectCause();
        EffectCause(EffectsCauseType cause_type_, const std::string& specific_cause_);
        EffectsCauseType                                cause_type;         ///< general type of effect cause, eg. tech, building, special...
        std::string                                     specific_cause;     ///< name of specific cause, eg. "Wonder Farm", "Antenna Mk. VI"
    };
    /** Combination of targets and cause for an effects group. */
    struct EffectTargetAndCause {
        EffectTargetAndCause();
        EffectTargetAndCause(const Condition::ObjectSet& target_set_, const EffectCause& effect_cause_);
        Condition::ObjectSet                            target_set;
        EffectCause                                     effect_cause;
    };
    /** Map from (effects group and source object) to target set of for
      * that effects group with that source object.  A multimap is used
      * so that a single source object can have multiple instances of the
      * same effectsgroup.  This is useful when a Ship has multiple copies
      * of the same effects group due to having multiple copies of the same
      * ship part in its design. */
    typedef std::multimap<SourcedEffectsGroup, EffectTargetAndCause> EffectsTargetsCausesMap;

    /** Accounting information about what the causes are and changes produced
      * by effects groups acting on meters of objects. */
    struct EffectAccountingInfo : public EffectCause {
        EffectAccountingInfo();                                                 ///< default ctor
        int                                             source_id;              ///< source object of effect
        double                                          meter_change;           ///< net change on meter due to this effect, as best known by client's empire
        double                                          running_meter_total;    ///< meter total as of this effect.
    };
    /** Effect accounting information for all meters of all objects that are
      * acted on by effects. */
    typedef std::map<int, std::map<MeterType, std::vector<EffectAccountingInfo> > > EffectAccountingMap;


    /** \name Signal Types */ //@{
    typedef boost::signal<void (const UniverseObject *)> UniverseObjectDeleteSignalType; ///< emitted just before the UniverseObject is deleted
    //@}


    /** \name Structors */ //@{
    Universe();                                     ///< default ctor
    virtual ~Universe();                            ///< dtor
    //@}

    /** \name Accessors */ //@{
    const UniverseObject*   Object(int id) const;   ///< returns a pointer to the universe object with ID number \a id, or 0 if none exists
    UniverseObject*         Object(int id);         ///< returns a pointer to the universe object with ID number \a id, or 0 if none exists

    template <class T>
    const T*                Object(int id) const;   ///< returns a pointer to the object of type T with ID number \a id. Returns 0 if none exists or the object with ID \a id is not of type T.
    template <class T>
    T*                      Object(int id);         ///< returns a pointer to the object of type T with ID number \a id. Returns 0 if none exists or the object with ID \a id is not of type T.

    /** Returns all the objects that match \a visitor */
    ConstObjectVec          FindObjects(const UniverseObjectVisitor& visitor) const;

    /** Returns all the objects that match \a visitor */
    ObjectVec               FindObjects(const UniverseObjectVisitor& visitor);

    /** Returns all the objects of type T */
    template <class T>
    std::vector<const T*>   FindObjects() const;

    /** Returns all the objects of type T */
    template <class T>
    std::vector<T*>         FindObjects();

    /** Returns the IDs of all the objects that match \a visitor */
    ObjectIDVec             FindObjectIDs(const UniverseObjectVisitor& visitor) const;

    /** Returns the IDs of all the objects of type T */
    template <class T>
    ObjectIDVec             FindObjectIDs() const;

    iterator                begin();
    iterator                end();
    const_iterator          begin() const;                                                  ///< returns the begin const_iterator for the objects in the universe
    const_iterator          end() const;                                                    ///< returns the end const_iterator for the objects in the universe


    const UniverseObject*   DestroyedObject(int id) const;                                  ///< returns a pointer to the destroyed universe object with ID number \a id, or 0 if none exists
    const_iterator          beginDestroyed() const  {return m_destroyed_objects.begin();}   ///< returns the begin const_iterator for the destroyed objects from the universe
    const_iterator          endDestroyed() const    {return m_destroyed_objects.end();}     ///< returns the end const_iterator for the destroyed objects from the universe


    const ShipDesign*       GetShipDesign(int ship_design_id) const;                        ///< returns the ship design with id \a ship_design id, or 0 if non exists
    ship_design_iterator    beginShipDesigns() const   {return m_ship_designs.begin();}     ///< returns the begin iterator for ship designs
    ship_design_iterator    endShipDesigns() const     {return m_ship_designs.end();}       ///< returns the end iterator for ship designs


    Visibility              GetObjectVisibilityByEmpire(int object_id, int empire_id) const;///< returns the Visibility level of empire with id \a empire_id of UniverseObject with id \a object_id as determined by calling UpdateEmpireObjectVisibilities

    double                  LinearDistance(int system1_id, int system2_id) const;           ///< returns the straight-line distance between the systems with the given IDs. \throw std::out_of_range This function will throw if either system ID is out of range.

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
                            LeastJumpsPath(int system1_id, int system2_id, int empire_id = ALL_EMPIRES) const;

    /** returns whether there is a path known to empire \a empire_id between
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
    const EffectAccountingMap&  GetEffectAccountingMap() const {return m_effect_accounting_map;}

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
    void            CreateUniverse(int size, Shape shape, Age age,
                                   StarlaneFrequency starlane_freq, PlanetDensity planet_density,
                                   SpecialsFrequency specials_freq, int players, int ai_players,
                                   const std::map<int, PlayerSetupData>& player_setup_data);

    /** Determines all effectsgroups' target sets, resets meters and applies
      * universe table adjustments.  Then executes all effects on all objects
      * (meter effects and non-meter effects, including destroying objects).
      * Then clamps meter values so max is within acceptable range, and current
      * is within range limited by max. */
    void            ApplyAllEffectsAndUpdateMeters();

    /** Determines all effectsgroups' target sets, resets meters and applies
      * universe table adjustments.  then executes only SetMeter effects on all
      * objects.  then clamps meter values so max is within acceptable range,
      * and current is within range limited by max. */
    void            ApplyMeterEffectsAndUpdateMeters();


    /** For all objects and meters, determines discrepancies between actual meter
      * maxes and what the known universe should produce, and and stores in
      * m_effect_discrepancy_map. */
    void            InitMeterEstimatesAndDiscrepancies();

    /** Based on (known subset of, if in a client) universe and any orders
      * given so far this turn, updates estimated meter maxes for next turn
      * for the objects with ids indicated in \a objects_vec.  If
      * \a meter_type is INVALID_METER_TYPE, all meter types are updated, but
      * if \a meter_type is a valid meter type, just that type of meter is
      * updated. */
    void            UpdateMeterEstimates(const std::vector<int>& objects_vec,
                                         MeterType meter_type = INVALID_METER_TYPE);

    /** Updates indicated object's indicated meter, and if applicable, the
      * indicated meters of objects contained within the indicated object
      * If \a object_id is UniverseObject::INVALID_OBJECT_ID, then all
      * objects' meters are updated.  If \a meter_type is INVALID_METER_TYPE,
      * then all meter types are updated. */
    void            UpdateMeterEstimates(int object_id,
                                         MeterType meter_type = INVALID_METER_TYPE,
                                         bool update_contained_objects = false);

    /** Updates all meters for all (known) objects */
    void            UpdateMeterEstimates();

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
    void            RebuildEmpireViewSystemGraphs();

    /** Removes the object with ID number \a id from the universe's map of
      * existing objects and places it into the map of destroyed objects.
      * removes the object from any containing UniverseObjects, though leaves
      * the object's own records of what contained it intact, so that
      * this information may be retained for later reference */
    void            Destroy(int id);

    /** Used by the Destroy effect to mark an object for destruction later
      * during turn processing. (objects can't be destroyed immediately as
      * other effects might depend on their existence) */
    void            EffectDestroy(int id);

    /** Permanently deletes object with ID number \a id.  no information about
      * this object is retained in the Universe.  Can be performed on objects
      * whether or not the have been destroyed.  Returns true if such an object
      * was found, false otherwise. */
    bool            Delete(int id);

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
    typedef std::vector< std::vector<double> > DistanceMatrix;

    /** Discrepancy between meter's value at start of turn, and the value that
      * this client calculate that the meter should have with the knowledge
      * available -> the unknown factor affecting the meter.  This is used
      * when generating effect accounting, in the case where the expected
      * and actual meter values don't match. */
    typedef std::map<int, std::map<MeterType, double> > EffectDiscrepancyMap;

    struct GraphImpl;

    /** Clears \a effects_targets_map, and then populates with all
      * EffectsGroups and their targets in the known universe.  If
      * \a effects_causes_map is provided (nonzero pointer) then this map will
      * be simultaneously* populated with information about the causes of each
      * effects group. */
    void    GetEffectsAndTargets(EffectsTargetsCausesMap& effects_targets_map);

    /** Removes entries in \a effects_targets_map about effects groups acting
      * on objects in \a target_objects, and then repopulates for EffectsGroups
      * that act on at least one of the objects in \a target_objects.  If
      * \a effects_causes_map is provided (nonzero pointer) then this map will
      * be simultaneously populated with information about the causes of each
      * effects group. */
    void    GetEffectsAndTargets(EffectsTargetsCausesMap& targets_causes_map, const std::vector<int>& target_objects);

    /** Used by GetEffectsAndTargets to process a vector of effects groups.
      * Stores target set of specified \a effects_groups and \a source_object_id
      * in \a targets_causes_map */
    void    StoreTargetsAndCausesOfEffectsGroups(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups,
                                                 int source_object_id, EffectsCauseType effect_cause_type,
                                                 const std::string& specific_cause_name,
                                                 const std::vector<int>& target_objects, EffectsTargetsCausesMap& targets_causes_map);

    /** Executes all effects.  Does not store effect accounting information.
      * For use on server when processing turns. */
    void    ExecuteEffects(EffectsTargetsCausesMap& targets_causes_map);

    /** Executes only meter-altering effects; ignores other effects..  Stores
      * effect accounting information in \a targets_causes_map.  Can be used
      * on server or on clients to determine meter values after effects are
      * applied */
    void    ExecuteMeterEffects(EffectsTargetsCausesMap& targets_causes_map);

    /** Does actual updating of meter estimates after the public function have
      * processed objects_vec or whatever they were passed and cleared the
      * relevant effect accounting for those objects and meters. */
    void    UpdateMeterEstimatesImpl(const std::vector<int>& objects_vec, MeterType meter_type = INVALID_METER_TYPE);


    /** Generates planets for all systems that have empty object maps (ie those
      * that aren't homeworld systems).*/
    void    PopulateSystems(PlanetDensity density, SpecialsFrequency specials_freq);

    /** Creates starlanes and adds them systems already generated. */
    void    GenerateStarlanes(StarlaneFrequency freq, const AdjacencyGrid& adjacency_grid);

    /** Used by GenerateStarlanes.  Determines if two systems are connected by
      * maxLaneJumps or less edges on graph. */
    bool    ConnectedWithin(int system1, int system2, int maxLaneJumps, std::vector<std::set<int> >& laneSetArray);

    /** Removes lanes from passed graph that are angularly too close to
      * each other. */
    void    CullAngularlyTooCloseLanes(double maxLaneUVectDotProd, std::vector<std::set<int> >& laneSetArray, std::vector<System*> &systems);

    /** Removes lanes from passed graph that are too long. */
    void    CullTooLongLanes(double maxLaneLength, std::vector<std::set<int> >& laneSetArray, std::vector<System*> &systems);

    /** Grows trees to connect stars...  takes an array of sets of potential
      * starlanes for each star, and puts the starlanes of the tree into
      * another set. */
    void    GrowSpanningTrees(std::vector<int> roots, std::vector<std::set<int> >& potentialLaneSetArray, std::vector<std::set<int> >& laneSetArray);

    /** Resizes the system graph to the appropriate size and populates
      * m_system_distances. */
    void    InitializeSystemGraph();

    /** Picks systems to host homeworlds, generates planets for them, stores
      * the ID's of the homeworld planets into the homeworld vector. */
    void    GenerateHomeworlds(int players, std::vector<int>& homeworlds);

    /** Names the planets in each system, based on the system's name. */
    void    NamePlanets();

    /** Will create empire objects, assign them homeworlds, setup the homeworld
      * population, industry, and starting fleets. */
    void    GenerateEmpires(int players, std::vector<int>& homeworlds, const std::map<int, PlayerSetupData>& player_setup_data);

    void    DestroyImpl(int id);

    ObjectMap                       m_objects;                          ///< map from object id to UniverseObjects in the universe.  for the server: all of them, up to date and true information about object is stored;  for clients, only limited information based on what the client knows about is sent.

    ObjectMap                       m_destroyed_objects;                ///< map from object id to objects that have been destroyed from the universe.  for the server: all of them;  for clients, only those that the local client knows about, not including previously-seen objects that the client no longer can see
    ObjectKnowledgeMap              m_destroyed_object_knowers;         ///< keyed by (destroyed) object ID, map of sets of Empires' IDs that know the objects have been destroyed (ie. could see the object when it was destroyed)

    EmpireObjectVisibilityMap       m_empire_object_visibility;         ///< map from empire id to (map from object id to visibility of that object for that empire)

    EmpireLatestKnownObjectMap      m_empire_latest_known_objects;      ///< map from empire id to (map from object id to latest known information about each object by that empire)
    EmpireObjectVisibilityTurnMap   m_empire_object_visibility_turns;   ///< map from empire id to (map from object id to (map from Visibility rating to turn number on which the empire last saw the object at the indicated Visibility rating or higher)


    ShipDesignMap                   m_ship_designs;                     ///< ship designs in the universe

    DistanceMatrix                  m_system_distances;                 ///< the straight-line distances between all the systems; this is an lower-triangular matrix, so only access the elements in (highID, lowID) order
    GraphImpl*                      m_graph_impl;                       ///< a graph in which the systems are vertices and the starlanes are edges

    EffectAccountingMap             m_effect_accounting_map;            ///< map from target object id, to map from target meter, to orderered list of structs with details of an effect and what it does to the meter
    EffectDiscrepancyMap            m_effect_discrepancy_map;           ///< map from target object id, to map from target meter, to discrepancy between meter's actual initial value, and the initial value that this meter should have as far as the client can tell: the unknown factor affecting the meter

    int                             m_last_allocated_object_id;
    int                             m_last_allocated_design_id;

    std::set<int>                   m_marked_destroyed;                 ///< used while applying effects to cache objects that have been destroyed.  this allows to-be-destroyed objects to remain undestroyed until all effects have been processed, which ensures that to-be-destroyed objects still exist when other effects need to access them as a source object
    std::multimap<int, std::string> m_marked_for_victory;               ///< used while applying effects to cache objects whose owner should be victorious.  Victory testing is done separately from effects execution, so this needs to be stored temporarily...

    static double                   s_universe_width;
    static bool                     s_inhibit_universe_object_signals;

    void    GetShipDesignsToSerialize(const ObjectMap& serialized_objects, ShipDesignMap& designs_to_serialize) const;
    void    GetObjectsToSerialize(ObjectMap& objects, int encoding_empire) const;
    void    GetEmpireObjectVisibilityMap(EmpireObjectVisibilityMap& empire_object_visibility, int encoding_empire) const;
    void    GetEmpireObjectVisibilityTurnMap(EmpireObjectVisibilityTurnMap& empire_object_visibility_turns, int encoding_empire) const;
    void    GetDestroyedObjectsToSerialize(ObjectMap& destroyed_objects, int encoding_empire) const;
    void    GetDestroyedObjectKnowers(ObjectKnowledgeMap& destroyed_object_knowers, int encoding_empire) const;

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
    ObjectMap                       objects;
    ObjectMap                       destroyed_objects;
    ObjectKnowledgeMap              destroyed_object_knowers;
    EmpireObjectVisibilityMap       empire_object_visibility;
    EmpireObjectVisibilityTurnMap   empire_object_visibility_turns;

    if (Archive::is_saving::value) {
        GetObjectsToSerialize(              objects,                        s_encoding_empire);
        GetEmpireObjectVisibilityMap(       empire_object_visibility,       s_encoding_empire);
        GetEmpireObjectVisibilityTurnMap(   empire_object_visibility_turns, s_encoding_empire);
        GetDestroyedObjectsToSerialize(     destroyed_objects,              s_encoding_empire);
        GetDestroyedObjectKnowers(          destroyed_object_knowers,       s_encoding_empire);
    }

    // ship designs
    ShipDesignMap ship_designs;
    if (Archive::is_saving::value)
        GetShipDesignsToSerialize(objects, ship_designs);

    ar  & BOOST_SERIALIZATION_NVP(s_universe_width)
        & BOOST_SERIALIZATION_NVP(objects)
        & BOOST_SERIALIZATION_NVP(empire_object_visibility)
        & BOOST_SERIALIZATION_NVP(empire_object_visibility_turns)
        & BOOST_SERIALIZATION_NVP(destroyed_objects)
        & BOOST_SERIALIZATION_NVP(destroyed_object_knowers)
        & BOOST_SERIALIZATION_NVP(ship_designs)
        & BOOST_SERIALIZATION_NVP(m_last_allocated_object_id)
        & BOOST_SERIALIZATION_NVP(m_last_allocated_design_id);

    if (s_encoding_empire == ALL_EMPIRES) {
        ar  & BOOST_SERIALIZATION_NVP(m_empire_latest_known_objects);
    }

    if (Archive::is_loading::value) {
        m_objects = objects;
        m_empire_object_visibility = empire_object_visibility;
        m_empire_object_visibility_turns = empire_object_visibility_turns;
        m_destroyed_objects = destroyed_objects;
        m_destroyed_object_knowers = destroyed_object_knowers;
        m_ship_designs = ship_designs;
        InitializeSystemGraph();
    }

    //Logger().debugStream() << "Universe::serialize destroyed objects at end: ";
    //for (ObjectMap::const_iterator it = destroyed_objects.begin(); it != destroyed_objects.end(); ++it) {
    //    Logger().debugStream() << " ... " << it->second->Name() << " with id " << it->first;
    //}
    //Logger().debugStream() << "Universe::serialize destroyed object knowers at end: ";
    //for (ObjectKnowledgeMap::const_iterator know_it = destroyed_object_knowers.begin(); know_it != destroyed_object_knowers.end(); ++know_it) {
    //    Logger().debugStream() << " ... object id: " << know_it->first << " known by: ";
    //    const std::set<int>& knowers = know_it->second;
    //    for (std::set<int>::const_iterator it = knowers.begin(); it != knowers.end(); ++it)
    //        Logger().debugStream() << " ... ... " << *it;
    //}
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

#endif // _Universe_h_
