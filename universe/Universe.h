// -*- C++ -*-
#ifndef _Universe_h_
#define _Universe_h_

#include "Enums.h"
#include "ObjectMap.h"
#include "TemporaryPtr.h"

#include <boost/signals2/signal.hpp>
#include <boost/unordered_map.hpp>
#include <boost/numeric/ublas/symmetric.hpp>
#include <boost/serialization/access.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <vector>
#include <list>
#include <map>
#include <string>
#include <set>

#include "../util/Export.h"

#ifdef FREEORION_WIN32
// because the linker gets confused about Win32 API functions...
#  undef GetObject
#  undef GetObjectA
#endif

class Empire;
struct UniverseObjectVisitor;
class XMLElement;
class ShipDesign;
class UniverseObject;
class System;
namespace Condition {
    struct ConditionBase;
    typedef std::vector<TemporaryPtr<const UniverseObject> > ObjectSet;
}

namespace Effect {
    struct AccountingInfo;
    struct TargetsAndCause;
    struct SourcedEffectsGroup;
    class EffectsGroup;
    typedef std::vector<TemporaryPtr<UniverseObject> > TargetSet;
    typedef std::map<int, std::map<MeterType, std::vector<AccountingInfo> > > AccountingMap;
    typedef std::vector<std::pair<SourcedEffectsGroup, TargetsAndCause> > TargetsCauses;
    typedef std::map<int, std::map<MeterType, double> > DiscrepancyMap;
}

/** The Universe class contains the majority of FreeOrion gamestate: All the
  * UniverseObjects in a game, and (of less importance) all ShipDesigns in a
  * game.  (Other gamestate is contained in the Empire class.)
  * The Universe class also provides functions with which to access objects in
  * it, information about the objects, and information about the objects'
  * relationships to each other.  As well, there are functions that generate
  * and populate new Universe gamestates when new games are started. */
class FO_COMMON_API Universe {
private:
    typedef std::map<int, ObjectMap>                EmpireObjectMap;                ///< Known information each empire had about objects in the Universe; keyed by empire id

public:
    typedef std::map<Visibility, int>               VisibilityTurnMap;              ///< Most recent turn number on which a something, such as a Universe object, was observed at various Visibility ratings or better

private:
    typedef std::map<int, VisibilityTurnMap>        ObjectVisibilityTurnMap;        ///< Most recent turn number on which the objects were observed at various Visibility ratings; keyed by object id
    typedef std::map<int, ObjectVisibilityTurnMap>  EmpireObjectVisibilityTurnMap;  ///< Each empire's most recent turns on which object information was known; keyed by empire id

    typedef std::map<int, std::set<int> >           ObjectKnowledgeMap;             ///< IDs of Empires which know information about an object (or deleted object); keyed by object id

public:
    typedef std::map<int, Visibility>               ObjectVisibilityMap;            ///< map from object id to Visibility level for a particular empire
    typedef std::map<int, ObjectVisibilityMap>      EmpireObjectVisibilityMap;      ///< map from empire id to ObjectVisibilityMap for that empire

    typedef std::map<int, std::set<std::string> >   ObjectSpecialsMap;              ///< map from object id to names of specials on an object
    typedef std::map<int, ObjectSpecialsMap>        EmpireObjectSpecialsMap;        ///< map from empire id to ObjectSpecialsMap of known specials for objects for that empire

    typedef std::map<int, ShipDesign*>              ShipDesignMap;                  ///< ShipDesigns in universe; keyed by design id
    typedef ShipDesignMap::const_iterator           ship_design_iterator;           ///< const iterator over ship designs created by players that are known by this client

    /** \name Signal Types */ //@{
    /** emitted just before the UniverseObject is deleted */
    typedef boost::signals2::signal<void (TemporaryPtr<const UniverseObject>)> UniverseObjectDeleteSignalType;
    //@}

    /** \name Structors */ //@{
    Universe();                                     ///< default ctor
    virtual ~Universe();                            ///< dtor
    //@}

    /** \name Accessors */ //@{
    /** Returns objects in this Universe. */
    const ObjectMap&        Objects() const { return m_objects; }
    ObjectMap&              Objects()       { return m_objects; }

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

    /** Returns IDs of objects that have been destroyed. */
    const std::set<int>&    DestroyedObjectIds() const;

    /** Returns IDs of objects that the Empire with id \a empire_id knows have
      * been destroyed.  Each empire's latest known objects data contains the
      * last known information about each object, whether it has been destroyed
      * or not.  If \a empire_id = ALL_EMPIRES an empty set of IDs is
      * returned. */
    const std::set<int>&    EmpireKnownDestroyedObjectIDs(int empire_id) const;

    /** Returns IDs of objects that the Empire with id \a empire_id has stale
      * knowledge of in its latest known objects.  The latest known data about
      * these objects suggests that they should be visible, but they are not. */
    const std::set<int>&    EmpireStaleKnowledgeObjectIDs(int empire_id) const;

    const ShipDesign*       GetShipDesign(int ship_design_id) const;                    ///< returns the ship design with id \a ship_design id, or 0 if non exists
    void                    RenameShipDesign(int design_id, const std::string& name = "", const std::string& description = "");
    ship_design_iterator    beginShipDesigns() const   {return m_ship_designs.begin();} ///< returns the begin iterator for ship designs
    ship_design_iterator    endShipDesigns() const     {return m_ship_designs.end();}   ///< returns the end iterator for ship designs

    const ShipDesign*       GetGenericShipDesign(const std::string& name) const;

    /** Returns IDs of ship designs that the Empire with id \a empire_id has
      * seen during the game.  If \a empire_id = ALL_EMPIRES an empty set of
      * ids is returned */
    const std::set<int>&    EmpireKnownShipDesignIDs(int empire_id) const;

    /** Returns the Visibility level of empire with id \a empire_id of
      * UniverseObject with id \a object_id as determined by calling
      * UpdateEmpireObjectVisibilities. */
    Visibility              GetObjectVisibilityByEmpire(int object_id, int empire_id) const;

    /** Returns the map from Visibility level to turn number on which the empire
      * with id \a empire_id last had the various Visibility levels of the
      * UniverseObject with id \a object_id .  The returned map may be empty or
      * not have entries for all visibility levels, if the empire has not seen
      * the object at that visibility level yet. */
    const VisibilityTurnMap&
                            GetObjectVisibilityTurnMapByEmpire(int object_id, int empire_id) const;

    /** Returns the set of specials attached to the object with id \a object_id
      * that the empire with id \a empire_id can see this turn. */
    std::set<std::string>   GetObjectVisibleSpecialsByEmpire(int object_id, int empire_id) const;

    /** Returns the straight-line distance between the objects with the given
      * IDs. \throw std::out_of_range This function will throw if either object
      * ID is out of range. */
    double                  LinearDistance(int object1_id, int object2_id) const;

    /** Returns the number of starlane jumps between the systems with the given
      * IDs. If there is no path between the systems, -1 is returned.
      * \throw std::out_of_range This function will throw if either system
      * ID is not a valid system id. */
    short                   JumpDistanceBetweenSystems(int system1_id, int system2_id) const;

    /** Returns the number of starlane jumps between any two objects, accounting
      * for cases where one or the other are fleets / ships on starlanes between
      * systems. Returns INT_MAX when no path exists, or either object does not
      * exist. */
    int                     JumpDistanceBetweenObjects(int object1_id, int object2_id) const;


    /** Returns the sequence of systems, including \a system1_id and
      * \a system2_id, that defines the shortest path from \a system1 to
      * \a system2, and the distance travelled to get there.  If no such path
      * exists, the list will be empty.  Note that the path returned may be via
      * one or more starlane, or may be "offroad".  The path is calculated
      * using the visibility for empire \a empire_id, or without regard to
      * visibility if \a empire_id == ALL_EMPIRES.
      * \throw std::out_of_range This function will throw if either system ID
      * is out of range, or if the empire ID is not known. */
    std::pair<std::list<int>, double>
                            ShortestPath(int system1_id, int system2_id, int empire_id = ALL_EMPIRES) const;

    /** Returns the sequence of systems, including \a system1 and \a system2,
      * that defines the path with the fewest jumps from \a system1 to
      * \a system2, and the number of jumps to get there.  If no such path
      * exists, the list will be empty.  The path is calculated using the
      * visibility for empire \a empire_id, or without regard to visibility if
      * \a empire_id == ALL_EMPIRES.  \throw std::out_of_range This function
      * will throw if either system ID is out of range or if the empire ID is
      * not known. */
    std::pair<std::list<int>, int>
                            LeastJumpsPath(int system1_id, int system2_id, int empire_id = ALL_EMPIRES,
                                           int max_jumps = INT_MAX) const;

    /** Returns whether there is a path known to empire \a empire_id between
      * system \a system1 and system \a system2.  The path is calculated using
      * the visibility for empire \a empire_id, or without regard to visibility
      * if \a empire_id == ALL_EMPIRES.  \throw std::out_of_range This function
      * will throw if either system ID is out of range or if the empire ID is
      * not known. */
    bool                    SystemsConnected(int system1_id, int system2_id, int empire_id = ALL_EMPIRES) const;

    /** Returns true iff \a system is reachable from another system (i.e. it
      * has at least one known starlane to it).   This does not guarantee that
      * the system is reachable from any specific other system, as two separate
      * groups of locally but not globally interconnected systems may exist.
      * The starlanes considered depend on their visibility for empire
      * \a empire_id, or without regard to visibility if
      * \a empire_id == ALL_EMPIRES. */
    bool                    SystemHasVisibleStarlanes(int system_id, int empire_id = ALL_EMPIRES) const;

    /** Returns the systems that are one starlane hop away from system
      * \a system.  The returned systems are indexed by distance from
      * \a system.  The neighborhood is calculated using the visibility
      * for empire \a empire_id, or without regard to visibility if
      * \a empire_id == ALL_EMPIRES.
      * \throw std::out_of_range This function will throw if the  system
      * ID is out of range. */
    std::multimap<double, int>              ImmediateNeighbors(int system_id, int empire_id = ALL_EMPIRES) const;

    /** Returns the id of the System object that is closest to the specified
      * (\a x, \a y) location on the map, by direct-line distance. */
    int                                     NearestSystemTo(double x, double y) const;

    /** Returns map, indexed by object id, to map, indexed by MeterType,
      * to vector of EffectAccountInfo for the meter, in order effects
      * were applied to the meter. */
    const Effect::AccountingMap&            GetEffectAccountingMap() const {return m_effect_accounting_map;}

    /** Returns set of objects that have been marked by the Victory effect
      * to grant their owners victory. */
    const std::multimap<int, std::string>&  GetMarkedForVictory() const {return m_marked_for_victory;}

    const std::map<std::string, std::map<int, std::map<int, double> > >&
                                            GetStatRecords() const { return m_stat_records; }

    mutable UniverseObjectDeleteSignalType UniverseObjectDeleteSignal; ///< the state changed signal object for this UniverseObject
    //@}

    /** \name Mutators */ //@{
    /** Inserts \a ship_design into the universe; returns the ship design ID
      * assigned to it, or -1 on failure.
      * \note Universe gains ownership of \a ship_design once inserted. */
    int             InsertShipDesign(ShipDesign* ship_design);

    /** Inserts \a ship_design into the universe with given \a id;  returns
      * true on success, or false on failure.
      * \note Universe gains ownership of \a ship_design once inserted. */
    bool            InsertShipDesignID(ShipDesign* ship_design, int id);
    
    // Resets the universe object to prepare for generation of a new universe
    void            ResetUniverse();

    /** Clears main ObjectMap, empires' latest known objects map, and
      * ShipDesign map. */
    void            Clear();

    /** Determines all effectsgroups' target sets, then resets meters and
      * executes all effects on all objects.  Then clamps meter values so
      * target and max meters are within a reasonable range and any current
      * meters with associated max meters are limited by their max. */
    void            ApplyAllEffectsAndUpdateMeters();

    /** Determines all effectsgroups' target sets, then resets meters and
      * executes only SetMeter effects on all objects whose ids are listed in
      * \a object_ids.  Then clamps meter values so target and max meters are
      * within a reasonable range and any current meters with associated max
      * meters are limited by their max. */
    void            ApplyMeterEffectsAndUpdateMeters(const std::vector<int>& object_ids);

    /** Calls above ApplyMeterEffectsAndUpdateMeters() function on all objects.*/
    void            ApplyMeterEffectsAndUpdateMeters();
    /** Like ApplyMeterEffectsAndUpdateMeters(), but only target, max and unpaired meters are updated.*/
    void            ApplyMeterEffectsAndUpdateTargetMaxUnpairedMeters();

    /** Executes effects that modify objects' appearance in the human client. */
    void            ApplyAppearanceEffects(const std::vector<int>& object_ids);

    /** Executes effects that modify objects' apperance for all objects. */
    void            ApplyAppearanceEffects();

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
      * If \a object_id is INVALID_OBJECT_ID, then all
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

    /** Sets visibility for indicated \a empire_id of object with \a object_id
      * a vis */
    void            SetEmpireObjectVisibility(int empire_id, int object_id, Visibility vis);

    /** Sets visibility for indicated \a empire_id for the indicated \a special */
    void            SetEmpireSpecialVisibility(int empire_id, int object_id,
                                               const std::string& special_name, bool visible = true);

    /** Stores latest known information about each object for each empire and
      * updates the record of the last turn on which each empire has visibility
      * of object that can be seen on the current turn with the level of
      * visibility that the empire has this turn. */
    void            UpdateEmpireLatestKnownObjectsAndVisibilityTurns();

    /** Checks latest known information about each object for each empire and,
      * in cases when the latest known state (stealth and location) suggests
      * that the empire should be able to see the object, but the object can't
      * be seen by the empire, updates the latest known state to note this. */
    void            UpdateEmpireStaleObjectKnowledge();

    /** Fills pathfinding data structure and determines least jumps distances
      * between systems for the empire with id \a for_empire_id or uses the
      * main / true / visible objects if \a for_empire_id is ALL_EMPIRES*/
    void            InitializeSystemGraph(int for_empire_id = ALL_EMPIRES);

    /** Regenerates per-empire system view graphs by filtering the complete
      * system graph based on empire visibility.  Does not regenerate the base
      * graph to account for actual system-starlane connectivity changes. */
    void            UpdateEmpireVisibilityFilteredSystemGraphs(int for_empire_id = ALL_EMPIRES);

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
      * is thus empty. Returns the ids of all destroyed objects. */
    std::set<int>   RecursiveDestroy(int object_id);

    /** Used by the Destroy effect to mark an object for destruction later
      * during turn processing. (objects can't be destroyed immediately as
      * other effects might depend on their existence) */
    void            EffectDestroy(int object_id);

    /** Permanently deletes object with ID number \a object_id.  no
      * information about this object is retained in the Universe.  Can be
      * performed on objects whether or not the have been destroyed.  Returns
      * true if such an object was found, false otherwise. */
    bool            Delete(int object_id);

    /** Permanently deletes the ship design with ID number \a design_id. No
      * information about this design is retained in the Universe. */
    bool            DeleteShipDesign(int design_id);

    /** Cleans up internal storage of now-invalidated empire ID. */
    void            HandleEmpireElimination(int empire_id);

    /** Used by the Victory effect to mark an object to give it owner victory. */
    void            EffectVictory(int object_id, const std::string& reason_string);

    /** Sets whether to inhibit UniverseObjectSignals.  Inhibits if \a inhibit
      * is true, and (re)enables UniverseObjectSignals if \a inhibit is false. */
    void            InhibitUniverseObjectSignals(bool inhibit = true);

    void            UpdateStatRecords();
    //@}


    /** Generates an object ID for a future object. Usually used by the server
      * to service new ID requests. */
    int             GenerateObjectID();

    /** Generates design ID for a new (ship) design. Usually used by the
      * server to service new ID requests. */
    int             GenerateDesignID() { return ++m_last_allocated_design_id; } // TODO: See GenerateObjectID()

    /** Returns true if UniverseOjbectSignals are inhibited, false otherwise. */
    const bool&     UniverseObjectSignalsInhibited();

    /** HACK! This must be set to the encoding empire's id when serializing a
      * Universe, so that only the relevant parts of the Universe are
      * serialized.  The use of this global variable is done just so I don't
      * have to rewrite any custom boost::serialization classes that implement
      * empire-dependent visibility. */
    int&            EncodingEmpire();

    double          UniverseWidth() const;
    void            SetUniverseWidth(double width) { m_universe_width = width; }
    bool            AllObjectsVisible() const { return m_all_objects_visible; }

    /** \name Generators */ //@{
    TemporaryPtr<Ship> CreateShip(int id = INVALID_OBJECT_ID);
    TemporaryPtr<Ship> CreateShip(int empire_id, int design_id, const std::string& species_name,
                                  int produced_by_empire_id = ALL_EMPIRES, int id = INVALID_OBJECT_ID);

    TemporaryPtr<Fleet> CreateFleet(int id = INVALID_OBJECT_ID);
    TemporaryPtr<Fleet> CreateFleet(const std::string& name, double x, double y, int owner, int id = INVALID_OBJECT_ID);

    TemporaryPtr<Planet> CreatePlanet(int id = INVALID_OBJECT_ID);
    TemporaryPtr<Planet> CreatePlanet(PlanetType type, PlanetSize size, int id = INVALID_OBJECT_ID);

    TemporaryPtr<System> CreateSystem(int id = INVALID_OBJECT_ID);
    TemporaryPtr<System> CreateSystem(StarType star, const std::string& name, double x, double y, int id = INVALID_OBJECT_ID);
    TemporaryPtr<System> CreateSystem(StarType star, const std::map<int, bool>& lanes_and_holes,
                                      const std::string& name, double x, double y, int id = INVALID_OBJECT_ID);

    TemporaryPtr<Building> CreateBuilding(int id = INVALID_OBJECT_ID);
    TemporaryPtr<Building> CreateBuilding(int empire_id, const std::string& building_type,
                                          int produced_by_empire_id = ALL_EMPIRES, int id = INVALID_OBJECT_ID);

    TemporaryPtr<Field> CreateField(int id = INVALID_OBJECT_ID);
    TemporaryPtr<Field> CreateField(const std::string& field_type, double x, double y, double radius, int id = INVALID_OBJECT_ID);
    //@}

private:
    /// minimal public interface for distance caches
    template <class T> struct distance_matrix_storage {
        typedef T value_type;
        typedef std::vector<T>& row_ref;

        distance_matrix_storage() {};
        distance_matrix_storage(const distance_matrix_storage<T>& src)
        { resize(src.m_data.size()); };

        size_t size() 
        { return m_data.size(); }

        void resize(size_t a_size) {
            const size_t old_size = size();

            m_data.clear();
            m_data.resize(a_size);
            m_row_mutexes.resize(a_size);
            for (size_t i = old_size; i < a_size; ++i)
                m_row_mutexes[i] = boost::shared_ptr<boost::shared_mutex>(new boost::shared_mutex());
        }

        std::vector< std::vector<T> > m_data;
        std::vector< boost::shared_ptr<boost::shared_mutex> > m_row_mutexes;
        boost::shared_mutex m_mutex;
    };

    /** Inserts object \a obj into the universe; returns a TemporaryPtr
      * to the inserted object. */
    template <class T>
    TemporaryPtr<T>             Insert(T* obj);

    /** Inserts object \a obj of given ID into the universe; returns a
      * TemporaryPtr to the inserted object on proper insert, or a null
      * TemporaryPtr on failure.  Useful mostly for times when ID needs
      * to be consistent on client and server */
    template <class T>
    TemporaryPtr<T>             InsertID(T* obj, int id);

    struct GraphImpl;

    /** Clears \a targets_causes, and then populates with all
      * EffectsGroups and their targets in the known universe. */
    void    GetEffectsAndTargets(Effect::TargetsCauses& targets_causes);

    /** Removes entries in \a targets_causes about effects groups acting
      * on objects in \a target_objects, and then repopulates for EffectsGroups
      * that act on at least one of the objects in \a target_objects. If 
      * \a target_objects is empty then default target candidates will be used. */
    void    GetEffectsAndTargets(Effect::TargetsCauses& targets_causes,
                                 const std::vector<int>& target_objects);

    /** Executes all effects.  For use on server when processing turns.
      * If \a only_meter_effects is true, then only SetMeter effects are
      * executed.  This is useful on server or clients to update meter
      * values after the rest of effects (including non-meter effects) have
      * been executed. */
    void    ExecuteEffects(const Effect::TargetsCauses& targets_causes,
                           bool update_effect_accounting,
                           bool only_meter_effects = false,
                           bool only_appearance_effects = false,
                           bool include_empire_meter_effects = false);

    /** Does actual updating of meter estimates after the public function have
      * processed objects_vec or whatever they were passed and cleared the
      * relevant effect accounting for those objects and meters. If an empty 
      * vector is passed, it will instead update all existing objects. */
    void    UpdateMeterEstimatesImpl(const std::vector<int>& objects_vec);

    ObjectMap                       m_objects;                          ///< map from object id to UniverseObjects in the universe.  for the server: all of them, up to date and true information about object is stored;  for clients, only limited information based on what the client knows about is sent.
    EmpireObjectMap                 m_empire_latest_known_objects;      ///< map from empire id to (map from object id to latest known information about each object by that empire)

    std::set<int>                   m_destroyed_object_ids;             ///< all ids of objects that have been destroyed (on server) or that a player knows were destroyed (on clients)

    EmpireObjectVisibilityMap       m_empire_object_visibility;         ///< map from empire id to (map from object id to visibility of that object for that empire)
    EmpireObjectVisibilityTurnMap   m_empire_object_visibility_turns;   ///< map from empire id to (map from object id to (map from Visibility rating to turn number on which the empire last saw the object at the indicated Visibility rating or higher)

    EmpireObjectSpecialsMap         m_empire_object_visible_specials;   ///< map from empire id to (map from object id to (set of names of specials that empire can see are on that object) )

    ObjectKnowledgeMap              m_empire_known_destroyed_object_ids;///< map from empire id to (set of object ids that the empire knows have been destroyed)
    ObjectKnowledgeMap              m_empire_stale_knowledge_object_ids;///< map from empire id to (set of object ids that the empire has previously observed but has subsequently been unable to detect at its last known location despite expecting to be able to detect it based on stealth of the object and having detectors in range)

    ShipDesignMap                   m_ship_designs;                     ///< ship designs in the universe
    std::map<int, std::set<int> >   m_empire_known_ship_design_ids;     ///< ship designs known to each empire

    mutable distance_matrix_storage<short>
                                    m_system_jumps;                     ///< indexed by system graph index (not system id), caches the smallest number of jumps to travel between all the systems
    boost::shared_ptr<GraphImpl>    m_graph_impl;                       ///< a graph in which the systems are vertices and the starlanes are edges
    boost::unordered_map<int, size_t>  m_system_id_to_graph_index;

    Effect::AccountingMap           m_effect_accounting_map;            ///< map from target object id, to map from target meter, to orderered list of structs with details of an effect and what it does to the meter
    Effect::DiscrepancyMap          m_effect_discrepancy_map;           ///< map from target object id, to map from target meter, to discrepancy between meter's actual initial value, and the initial value that this meter should have as far as the client can tell: the unknown factor affecting the meter

    int                             m_last_allocated_object_id;
    int                             m_last_allocated_design_id;

    std::set<int>                   m_marked_destroyed;                 ///< used while applying effects to cache objects that have been destroyed.  this allows to-be-destroyed objects to remain undestroyed until all effects have been processed, which ensures that to-be-destroyed objects still exist when other effects need to access them as a source object
    std::multimap<int, std::string> m_marked_for_victory;               ///< used while applying effects to cache objects whose owner should be victorious.  Victory testing is done separately from effects execution, so this needs to be stored temporarily...

    double                          m_universe_width;
    bool                            m_inhibit_universe_object_signals;
    int                             m_encoding_empire;                  ///< used during serialization to globally set what empire knowledge to use
    bool                            m_all_objects_visible;              ///< flag set to skip visibility tests and make everything visible to all players

    std::map<std::string, std::map<int, std::map<int, double> > >
                                    m_stat_records;                     ///< storage for statistics calculated for empires. Indexed by stat name (string), contains a map indexed by empire id, contains a map from turn number (int) to stat value (double).

    /** Fills \a designs_to_serialize with ShipDesigns known to the empire with
      * the ID \a encoding empire.  If encoding_empire is ALL_EMPIRES, then all
      * designs are included. */
    void    GetShipDesignsToSerialize(ShipDesignMap& designs_to_serialize, int encoding_empire) const;

    /** Fills \a objects with copies of UniverseObjects that should be sent
      * to the empire with id \a encoding_empires */
    void    GetObjectsToSerialize(ObjectMap& objects, int encoding_empire) const;

    /** Fills \a destroyed_object_ids with ids of objects known to be destroyed
      * by the empire with ID \a encoding empire. If encoding_empire is
      * ALL_EMPIRES, then all destroyed objects are included. */
    void    GetDestroyedObjectsToSerialize(std::set<int>& destroyed_object_ids, int encoding_empire) const;

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
    void    GetEmpireKnownDestroyedObjects(ObjectKnowledgeMap& empire_known_destroyed_object_ids, int encoding_empire) const;

    /***/
    void    GetEmpireStaleKnowledgeObjects(ObjectKnowledgeMap& empire_stale_knowledge_object_ids, int encoding_empire) const;

    template <class T>
    TemporaryPtr<T> InsertNewObject(T* object);

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


#endif // _Universe_h_
