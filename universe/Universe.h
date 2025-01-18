#ifndef _Universe_h_
#define _Universe_h_


#include <map>
#include <memory>
#include <set>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../util/boost_fix.h"
#include <boost/container/flat_map.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/thread/shared_mutex.hpp>
#include "ConstantsFwd.h"
#include "EnumsFwd.h"
#include "ObjectMap.h"
#include "Pathfinder.h"
#include "UnlockableItem.h"
#include "UniverseObject.h"
#include "../util/Export.h"
#include "../util/Pending.h"


class Empire;
class EmpireManager;
class XMLElement;
class ShipDesign;
class System;
class IDAllocator;
class FleetPlan;
class MonsterFleetPlan;
struct ScriptingContext;


namespace Condition {
    struct Condition;
    using ObjectSet = std::vector<const UniverseObject*>;
}

namespace Effect {
    struct AccountingInfo;
    struct TargetsAndCause;     // struct TargetsAndCause { TargetSet target_set; EffectCause effect_cause; };
    struct SourcedEffectsGroup; // struct SourcedEffectsGroup { int source_object_id; const EffectsGroup* effects_group; };
    class EffectsGroup;
    using TargetSet = std::vector<UniverseObject*>;
    using AccountingMap = std::unordered_map<int, boost::container::flat_map<MeterType, std::vector<AccountingInfo>>>;
    using SourcesEffectsTargetsAndCause = std::pair<SourcedEffectsGroup, TargetsAndCause>;
    using SourcesEffectsTargetsAndCausesVec = std::vector<SourcesEffectsTargetsAndCause>;
}

namespace ValueRef {
    template <typename T>
    struct ValueRef;
}


/** The Universe class contains the majority of FreeOrion gamestate: All the
  * UniverseObjects in a game, and (of less importance) all ShipDesigns in a
  * game.  (Other gamestate is contained in the Empire class.)
  * The Universe class also provides functions with which to access objects in
  * it, information about the objects, and information about the objects'
  * relationships to each other.  As well, there are functions that generate
  * and populate new Universe gamestates when new games are started. */
class FO_COMMON_API Universe {
public:
    typedef std::map<int, ObjectMap>                EmpireObjectMap;                ///< Known information each empire had about objects in the Universe; keyed by empire id
    typedef std::map<Visibility, int>               VisibilityTurnMap;              ///< Most recent turn number on which a something, such as a Universe object, was observed at various Visibility ratings or better
    typedef std::map<int, VisibilityTurnMap>        ObjectVisibilityTurnMap;        ///< Most recent turn number on which the objects were observed at various Visibility ratings; keyed by object id
    typedef std::map<int, ObjectVisibilityTurnMap>  EmpireObjectVisibilityTurnMap;  ///< Each empire's most recent turns on which object information was known; keyed by empire id
    using IDSet = UniverseObject::IDSet;

private:
    typedef std::map<int, std::unordered_set<int>>  ObjectKnowledgeMap;             ///< IDs of Empires which know information about an object (or deleted object); keyed by object id

    typedef const ValueRef::ValueRef<Visibility>*   VisValRef;
    typedef std::vector<std::pair<int, VisValRef>>  SrcVisValRefVec;
    typedef std::map<int, SrcVisValRefVec>          ObjSrcVisValRefVecMap;
    typedef std::map<int, ObjSrcVisValRefVecMap>    EmpireObjectVisValueRefMap;

    /** Discrepancy between meter's value at start of turn, and the value that
      * this client calculate that the meter should have with the knowledge
      * available -> the unknown factor affecting the meter.  This is used
      * when generating effect accounting, in the case where the expected
      * and actual meter values don't match. */
    typedef std::unordered_map<int, boost::container::flat_map<MeterType, double>> DiscrepancyMap;

public:
    typedef std::map<int, Visibility>               ObjectVisibilityMap;            ///< map from object id to Visibility level for a particular empire
    typedef std::map<int, ObjectVisibilityMap>      EmpireObjectVisibilityMap;      ///< map from empire id to ObjectVisibilityMap for that empire

    typedef std::map<int, std::set<std::string>>    ObjectSpecialsMap;              ///< map from object id to names of specials on an object
    typedef std::map<int, ObjectSpecialsMap>        EmpireObjectSpecialsMap;        ///< map from empire id to ObjectSpecialsMap of known specials for objects for that empire

    typedef std::map<int, ShipDesign>               ShipDesignMap;                  ///< ShipDesigns in universe; keyed by design id
    typedef ShipDesignMap::const_iterator           ship_design_iterator;           ///< const iterator over ship designs created by players that are known by this client

    /** emitted just before the UniverseObject is deleted */
    typedef boost::signals2::signal<void (std::shared_ptr<const UniverseObject>)> UniverseObjectDeleteSignalType;

    Universe();
    Universe& operator=(Universe&& other) noexcept;
    ~Universe();


    /** Returns objects in this Universe. */
    [[nodiscard]] const ObjectMap& Objects() const noexcept { return m_objects; }
    [[nodiscard]] ObjectMap&       Objects() noexcept       { return m_objects; }

    /** Returns latest known state of objects for the Empire with
      * id \a empire_id or the true / complete state of all objects in this
      * Universe (the same as calling Objects()) if \a empire_id is
      * ALL_EMPIRES*/
    [[nodiscard]] const ObjectMap& EmpireKnownObjects(int empire_id = ALL_EMPIRES) const;
    [[nodiscard]] ObjectMap&       EmpireKnownObjects(int empire_id = ALL_EMPIRES);

    /** Returns IDs of objects that the Empire with id \a empire_id has vision
      * of on the current turn, or objects that at least one empire has vision
      * of on the current turn if \a empire_id = ALL_EMPIRES */
    [[nodiscard]] IDSet EmpireVisibleObjectIDs(int empire_id, const EmpireManager& empires) const;

    /** Returns IDs of objects that have been destroyed. */
    [[nodiscard]] auto& DestroyedObjectIds() const noexcept { return m_destroyed_object_ids; }
    [[nodiscard]] int   HighestDestroyedObjectID() const;

    /** Returns IDs of objects that the Empire with id \a empire_id knows have
      * been destroyed.  Each empire's latest known objects data contains the
      * last known information about each object, whether it has been destroyed
      * or not. */
    [[nodiscard]] const std::unordered_set<int>& EmpireKnownDestroyedObjectIDs(int empire_id) const;

    /** Returns IDs of objects that the Empire with id \a empire_id has stale
      * knowledge of in its latest known objects.  The latest known data about
      * these objects suggests that they should be visible, but they are not. */
    [[nodiscard]] const std::unordered_set<int>& EmpireStaleKnowledgeObjectIDs(int empire_id) const;

    [[nodiscard]] const ShipDesign* GetShipDesign(int ship_design_id) const;    ///< returns the ship design with id \a ship_design id, or 0 if non exists

    void RenameShipDesign(int design_id, std::string name = "", std::string description = "");

    [[nodiscard]] const auto& ShipDesigns() const noexcept { return m_ship_designs; }

    [[nodiscard]] const ShipDesign* GetGenericShipDesign(std::string_view name) const;

    /** Returns IDs of ship designs that the Empire with id \a empire_id has
      * seen during the game.  If \a empire_id = ALL_EMPIRES an empty set of
      * ids is returned */
    [[nodiscard]] const std::set<int>& EmpireKnownShipDesignIDs(int empire_id) const;

    /** Returns the Visibility level of empire with id \a empire_id of UniverseObject with
      * id \a object_id as determined by calling UpdateEmpireObjectVisibilities.
      * Monsters/neutrals are treated as an empire with id ALL_EMPIRES. */
    [[nodiscard]] Visibility GetObjectVisibilityByEmpire(int object_id, int empire_id) const;

    /* Return the map from empire id to (map from id to that empire's current
     * visibility of that object) */
    [[nodiscard]] auto& GetEmpireObjectVisibility() const noexcept { return m_empire_object_visibility; }

    /* Returns the map from empire id to (map from object id to (map from
     * visibility level to turn number on which the empire last detected that
     * object at that visibility level)). */
    [[nodiscard]] auto& GetEmpireObjectVisibilityTurnMap() const noexcept { return m_empire_object_visibility_turns; }

    /** Returns the map from Visibility level to turn number on which the empire
      * with id \a empire_id last had the various Visibility levels of the
      * UniverseObject with id \a object_id .  The returned map may be empty or
      * not have entries for all visibility levels, if the empire has not seen
      * the object at that visibility level yet. */
    [[nodiscard]] const VisibilityTurnMap& GetObjectVisibilityTurnMapByEmpire(int object_id, int empire_id) const;

    /** Returns the set of specials attached to the object with id \a object_id
      * that the empire with id \a empire_id can see this turn. */
    [[nodiscard]] std::set<std::string> GetObjectVisibleSpecialsByEmpire(int object_id, int empire_id) const;

    /** Returns map from empire ID to map from location (X, Y) to detection range
      * that empire has at that location. */
    [[nodiscard]] std::map<int, std::map<std::pair<double, double>, float>>
        GetEmpiresAndNeutralPositionDetectionRanges(const ObjectMap& objects) const;

    [[nodiscard]] std::map<int, std::map<std::pair<double, double>, float>>
        GetEmpiresAndNeutralPositionDetectionRanges(const ObjectMap& objects,
                                                    const std::unordered_set<int>& exclude_ids) const;

    /** Returns map from empire ID to map from location (X, Y) to detection range
      * that empire is expected to have at that location after the next turn's
      * fleet movement. */
    [[nodiscard]] std::map<int, std::map<std::pair<double, double>, float>>
        GetEmpiresPositionNextTurnFleetDetectionRanges(const ScriptingContext& context) const;

    /** Return the Pathfinder */
    [[nodiscard]] const auto& GetPathfinder() const noexcept { return m_pathfinder; }

    /** Returns map, indexed by object id, to map, indexed by MeterType,
      * to vector of EffectAccountInfo for the meter, in order effects
      * were applied to the meter. */
    [[nodiscard]] const Effect::AccountingMap& GetEffectAccountingMap() const noexcept { return m_effect_accounting_map; }
    [[nodiscard]] Effect::AccountingMap& GetEffectAccountingMap() noexcept { return m_effect_accounting_map; }

    [[nodiscard]] const auto& GetStatRecords() const noexcept { return m_stat_records; }

    [[nodiscard]] std::size_t SizeInMemory() const;

    mutable UniverseObjectDeleteSignalType UniverseObjectDeleteSignal; ///< the state changed signal object for this UniverseObject

    /** Inserts \a ship_design into the universe. Return the design's assigned
      * ID on success, or INVALID_DESIGN_ID on failure.
      * \note Universe gains ownership of \a ship_design once inserted. */
    int InsertShipDesign(ShipDesign ship_design);

    /** Inserts \a ship_design into the universe with given \a id; returns true
      * on success, or false on failure.  An empire id of none indicates that
      * the server is allocating an id on behalf of itself.  This can be removed
      * when no longer supporting legacy id allocation in pending Orders. \note
      * Universe gains ownership of \a ship_design once inserted. */
    bool InsertShipDesignID(ShipDesign ship_design, boost::optional<int> empire_id, int id);

   /** Reset object and ship design id allocation for a new game. */
    void ResetAllIDAllocation(const std::vector<int>& empire_ids = std::vector<int>());

    /** Clears main ObjectMap, empires' latest known objects map, and
      * ShipDesign map. */
    void Clear();

    /** Resets meters */
    void ResetAllObjectMeters(bool target_max_unpaired = true, bool active = true);

    /** Determines all effectsgroups' target sets, then resets meters and
      * executes all effects on all objects.  Then clamps meter values so
      * target and max meters are within a reasonable range and any current
      * meters with associated max meters are limited by their max. */
    void ApplyAllEffectsAndUpdateMeters(ScriptingContext& context, bool do_accounting = true);

    /** Determines all effectsgroups' target sets, then resets meters and
      * executes only SetMeter effects on all objects whose ids are listed in
      * \a object_ids.  Then clamps meter values so target and max meters are
      * within a reasonable range and any current meters with associated max
      * meters are limited by their max. */
    void ApplyMeterEffectsAndUpdateMeters(const std::vector<int>& object_ids, ScriptingContext& context,
                                          bool do_accounting = true);

    /** Calls above ApplyMeterEffectsAndUpdateMeters() function on all objects.*/
    void ApplyMeterEffectsAndUpdateMeters(ScriptingContext& context, bool do_accounting = true);

    /** Executes effects that modify objects' appearance in the human client. */
    void ApplyAppearanceEffects(const std::vector<int>& object_ids, ScriptingContext& context);

    /** Executes effects that modify objects' apperance for all objects. */
    void ApplyAppearanceEffects(ScriptingContext& context);

    /** Executes effects that modify objects' apperance for all objects. */
    void ApplyGenerateSitRepEffects(ScriptingContext& context);

    /** For all objects and meters, determines discrepancies between actual meter
      * maxes and what the known universe should produce, and and stores in
      * m_effect_discrepancy_map. */
    void InitMeterEstimatesAndDiscrepancies(ScriptingContext& context);

    /** Based on (known subset of, if in a client) universe and any orders
      * given so far this turn, updates estimated meter maxes for next turn
      * for the objects with ids indicated in \a objects_vec. */
    void UpdateMeterEstimates(const std::vector<int>& objects_vec, ScriptingContext& context);

    /** Updates indicated object's meters, and if applicable, the
      * meters of objects contained within the indicated object.
      * If \a object_id is INVALID_OBJECT_ID, then all
      * objects' meters are updated. */
    void UpdateMeterEstimates(int object_id, ScriptingContext& context, bool update_contained_objects = false);

    /** Updates all meters for all (known) objects */
    void UpdateMeterEstimates(ScriptingContext& context);
    void UpdateMeterEstimates(ScriptingContext& context, bool do_accounting);

    /** Sets all objects' meters' initial values to their current values. */
    void BackPropagateObjectMeters();

    /** Determines which empires, or neutrals, can see which objects at what visibility
      * level, based on detection strengths and ranges and any applicable overrides to that. */
    void UpdateEmpireObjectVisibilities(const ScriptingContext& context);

    /** Sets visibility by empires of objects with specified ids. */
    void SetObjectVisibilityOverrides(std::map<int, std::vector<int>> empires_ids);
    void ApplyObjectVisibilityOverrides();

    /** Sets a special record of visibility that overrides the standard
      * empire-object visibility after the latter is processed. */
    void SetEffectDerivedVisibility(int empire_id, int object_id, int source_id,
                                    const ValueRef::ValueRef<Visibility>* vis);

    /** Applies empire-object visibilities set by effects. */
    void ApplyEffectDerivedVisibilities(const ScriptingContext& context);

    /** If an \p empire_id can't currently see \p object_id, then remove
     * \p object_id' object from the object map and the set of known objects. */
    void ForgetKnownObject(int empire_id, int object_id);

    /** Sets visibility for indicated \a empire_id of object with \a object_id
      * to at least * \a vis. If visibility is already equal or higher, does nothing.
      * For ship object ids, also sets the ship's design to be known to the empire. */
    void SetEmpireObjectVisibility(int empire_id, int object_id, Visibility vis);

    /** Sets visibility for indicated \a empire_id for the indicated \a special */
    void SetEmpireSpecialVisibility(int empire_id, int object_id,
                                    const std::string& special_name, bool visible = true);

    /** Stores latest known information about each object for each empire and
      * updates the record of the last turn on which each empire has visibility
      * of object that can be seen on the current turn with the level of
      * visibility that the empire has this turn. */
    void UpdateEmpireLatestKnownObjectsAndVisibilityTurns(int current_turn);

    /** Checks latest known information about each object for each empire and,
      * in cases when the latest known state (stealth and location) suggests
      * that the empire should be able to see the object, but the object can't
      * be seen by the empire, updates the latest known state to note this. */
    void UpdateEmpireStaleObjectKnowledge(EmpireManager& empires);

    /** Fills pathfinding data structure and determines least jumps distances
      * between systems based on the objects in \a objects */
    void InitializeSystemGraph(const EmpireManager& empires, const ObjectMap& objects);
    void InitializeSystemGraph(const EmpireManager& empires) { InitializeSystemGraph(empires, m_objects); }

    /** Regenerates per-empire system view graphs by filtering the complete
      * system graph based on empire visibility.  Does not regenerate the base
      * graph to account for actual system-starlane connectivity changes. */
    void UpdateEmpireVisibilityFilteredSystemGraphsWithOwnObjectMaps(const EmpireManager& empires);
    void UpdateCommonFilteredSystemGraphsWithMainObjectMap(const EmpireManager& empires);

    /** Adds the object ID \a object_id to the set of object ids for the empire
      * with id \a empire_id that the empire knows have been destroyed. */
    void SetEmpireKnowledgeOfDestroyedObject(int object_id, int empire_id);

    /** Adds the ship design ID \a ship_design_id to the set of ship design ids
      * known by the empire with id \a empire_id */
    void SetEmpireKnowledgeOfShipDesign(int ship_design_id, int empire_id);

    /** Record in statistics that \a object_id was destroyed by species/empire
      * associated with \a source_object_id */
    void CountDestructionInStats(int object_id, int source_object_id, const std::map<int, std::shared_ptr<Empire>>& empires);

    /** Removes the object with ID number \a object_id from the universe's map
      * of existing objects, and adds the object's id to the set of destroyed
      * object ids.  If \a update_destroyed_object_knowers is true, empires
      * whose ID is in \a empire_ids and that currently have visibility of the
      * object have its id added to their set of objects' ids that are known
      * to have been destroyed.  Older or limited versions of objects remain
      * in empires latest known objects ObjectMap, regardless of whether the
      * empire knows the object is destroyed. */
    void Destroy(int object_id, const std::span<const int> empire_ids,
                 bool update_destroyed_object_knowers = true);

    /** Destroys object with ID \a object_id, and destroys any associted
      * objects, such as contained buildings of planets, contained anything of
      * systems, or fleets if their last ship has id \a object_id and the fleet
      * is thus empty. Returns the ids of all destroyed objects. Empires
      * whose ID is in \a empire_ids and that currently have visibility of the
      * object have its id added to their set of objects' ids that are known
      * to have been destroyed. */
    std::vector<int> RecursiveDestroy(int object_id, const std::span<const int> empire_ids);

    /** Used by the Destroy effect to mark an object for destruction later
      * during turn processing. (objects can't be destroyed immediately as
      * other effects might depend on their existence) */
    void EffectDestroy(int destroyed_object_id, int source_object_id = INVALID_OBJECT_ID);

    /** Permanently deletes object with ID number \a object_id.
      * No information about this object is retained in the Universe.
      * Can be performed on objects whether or not the have been destroyed.
      * Returns true if such an object was found, false otherwise. */
    bool Delete(int object_id);

    /** Permanently deletes the ship design with ID number \a design_id. No
      * information about this design is retained in the Universe. */
    bool DeleteShipDesign(int design_id);

    /** Sets whether to inhibit UniverseObjectSignals.  Inhibits if \a inhibit
      * is true, and (re)enables UniverseObjectSignals if \a inhibit is false. */
    void InhibitUniverseObjectSignals(bool inhibit = true) noexcept { m_inhibit_universe_object_signals = inhibit; }

    void UpdateStatRecords(const ScriptingContext& context);

    /** Returns true if UniverseOjbectSignals are inhibited, false otherwise. */
    const bool& UniverseObjectSignalsInhibited() const noexcept { return m_inhibit_universe_object_signals; }

    [[nodiscard]] double UniverseWidth() const noexcept { return m_universe_width; }
    void SetUniverseWidth(double width) noexcept { m_universe_width = width; }

    /** InsertNew constructs and inserts a UniverseObject into the object map with a new
        id. It returns the new object. */
    template <typename T, typename... Args>
    std::shared_ptr<T> InsertNew(Args&&... args)
    { return InsertID<T>(GenerateObjectID(), std::forward<Args>(args)...); }

    /** InsertTemp constructs and inserts a temporary UniverseObject into the object map with a
        temporary id. It returns the new object. */
    template <typename T, typename... Args>
    std::shared_ptr<T> InsertTemp(Args&&... args)
    { return InsertID<T>(TEMPORARY_OBJECT_ID, std::forward<Args>(args)...); }

    /** InsertTemp inserts the provided \a objets into the object map with a series of
      * temporary ids. It returns the ids of the inserted objects. */
    template <typename T>
    std::vector<int> InsertTemp(const std::vector<std::shared_ptr<T>>& objects)
    {
        auto ID = TEMPORARY_OBJECT_ID;
        std::vector<int> retval;
        retval.reserve(objects.size());
        for (auto& obj : objects) {
            if (!obj)
                continue;
            retval.push_back(ID);
            obj->SetID(ID); // assign and decrement temporary ID
            m_objects.insert(std::move(obj)); // directly insert into objects, skipping ID validation
            ID--;
        }
        return retval;
    }

    /** \p empire_id inserts object \p obj into the universe with the given \p id.
        This is provided for use with Orders which are run once on the client
        and receive a new id and then are run a second time on the server using
        the id assigned on the client. */
    template <typename T, typename... Args>
    std::shared_ptr<T> InsertByEmpireWithID(int empire_id, int id, Args&&... args)
    {
        if (!VerifyUnusedObjectID(empire_id, id))
            return nullptr;
        return InsertID<T>(id, std::forward<Args>(args)...);
    }

    //! Set items unlocked before turn 1 from \p future.
    void SetInitiallyUnlockedItems(Pending::Pending<std::vector<UnlockableItem>>&& future);
    //! Items unlocked before turn 1.
    const std::vector<UnlockableItem>& InitiallyUnlockedItems() const;

    //! Set buildings unlocked before turn 1 from \p future.
    void SetInitiallyUnlockedBuildings(Pending::Pending<std::vector<UnlockableItem>>&& future);
    //! Buildings unlocked before turn 1.
    const std::vector<UnlockableItem>& InitiallyUnlockedBuildings() const;

    /** Set fleets unlocked before turn 1 from \p future.*/
    void SetInitiallyUnlockedFleetPlans(Pending::Pending<std::vector<std::unique_ptr<FleetPlan>>>&& future);
    /** Fleets unlocked before turn 1.*/
    std::vector<const FleetPlan*> InitiallyUnlockedFleetPlans() const;

    /** Set items unlocked before turn 1 from \p future..*/
    void SetMonsterFleetPlans(Pending::Pending<std::vector<std::unique_ptr<MonsterFleetPlan>>>&& future);
    /** Items unlocked before turn 1.*/
    std::vector<const MonsterFleetPlan*> MonsterFleetPlans() const;

    /** Set the empire stats from \p future. */
    using EmpireStatsMap = std::map<std::string, std::unique_ptr<ValueRef::ValueRef<double>>>;
    void SetEmpireStats(Pending::Pending<EmpireStatsMap> future);
private:
    const EmpireStatsMap& EmpireStats() const;
public:
    /** ObfuscateIDGenerator applies randomization to the IDAllocator to prevent clients from
        inferring too much information about other client's id generation activities. */
    void ObfuscateIDGenerator();

private:
    Pathfinder m_pathfinder;

    /** Generates an object ID for a future object. Usually used by the server to service new ID requests. */
    int GenerateObjectID();

    /** Generates design ID for a new (ship) design. Usually used by the server to service new ID requests. */
    int GenerateDesignID();

    /** Verify that an object ID \p id could be generated by \p empire_id. */
    bool VerifyUnusedObjectID(const int empire_id, const int id);

    /** Inserts object \p obj into the universe with the given \p id. */
    template <typename T, typename... Args>
    std::shared_ptr<T> InsertID(int id, Args&&... args) {
        static_assert(std::is_base_of_v<UniverseObject, T>);
        static_assert(!std::is_same_v<UniverseObject, T>);
        auto obj = std::make_shared<T>(std::forward<Args>(args)...);
        InsertIDCore(obj, id);
        return obj;
    }

    /** Inserts object \p obj into the universe with the given \p id. */
    void InsertIDCore(std::shared_ptr<UniverseObject> obj, int id);

    /** Clears \a source_effects_targets_causes, and then populates with all
      * EffectsGroups and their targets in the known universe. */
    void GetEffectsAndTargets(std::map<int, Effect::SourcesEffectsTargetsAndCausesVec>& source_effects_targets_causes,
                              const ScriptingContext& context,
                              bool only_meter_effects = false) const;

    /** Removes entries in \a source_effects_targets_causes about effects groups acting
      * on objects in \a target_objects, and then repopulates for EffectsGroups
      * that act on at least one of the objects in \a target_objects. If
      * \a target_objects is empty then default target candidates will be used. */
    void GetEffectsAndTargets(std::map<int, Effect::SourcesEffectsTargetsAndCausesVec>& source_effects_targets_causes,
                              const std::vector<int>& target_objects,
                              const ScriptingContext& context,
                              bool only_meter_effects = false) const;

    /** Executes all effects.  For use on server when processing turns.
      * If \a only_meter_effects is true, then only SetMeter effects are
      * executed.  This is useful on server or clients to update meter
      * values after the rest of effects (including non-meter effects) have
      * been executed. */
    void ExecuteEffects(std::map<int, Effect::SourcesEffectsTargetsAndCausesVec>& source_effects_targets_causes,
                        ScriptingContext& context,
                        bool update_effect_accounting,
                        bool only_meter_effects = false,
                        bool only_appearance_effects = false,
                        bool include_empire_meter_effects = false,
                        bool only_generate_sitrep_effects = false);

    /** Does actual updating of meter estimates after the public function have
      * processed objects_vec or whatever they were passed and cleared the
      * relevant effect accounting for those objects and meters. If an empty
      * vector is passed, it will instead update all existing objects. */
    void UpdateMeterEstimatesImpl(const std::vector<int>& objects_vec, ScriptingContext& context, bool do_accounting);

    ObjectMap                       m_objects;                          ///< map from object id to UniverseObjects in the universe.  for the server: all of them, up to date and true information about object is stored;  for clients, only limited information based on what the client knows about is sent.
    EmpireObjectMap                 m_empire_latest_known_objects;      ///< map from empire id to (map from object id to latest known information about each object by that empire)

    std::unordered_set<int>         m_destroyed_object_ids;             ///< all ids of objects that have been destroyed (on server) or that a player knows were destroyed (on clients)

    EmpireObjectVisibilityMap       m_empire_object_visibility;         ///< map from empire id to (map from object id to visibility of that object for that empire)
    EmpireObjectVisibilityTurnMap   m_empire_object_visibility_turns;   ///< map from empire id to (map from object id to (map from Visibility rating to turn number on which the empire last saw the object at the indicated Visibility rating or higher)

    std::map<int, std::vector<int>> m_fleet_blockade_ship_visibility_overrides; // map from empire id to (list of ship ids that are visibile to that empire due to being revealed by participating in a blockade)
    EmpireObjectVisValueRefMap      m_effect_specified_empire_object_visibilities;

    EmpireObjectSpecialsMap         m_empire_object_visible_specials;   ///< map from empire id to (map from object id to (set of names of specials that empire can see are on that object) )

    ObjectKnowledgeMap              m_empire_known_destroyed_object_ids;///< map from empire id to (set of object ids that the empire knows have been destroyed)
    ObjectKnowledgeMap              m_empire_stale_knowledge_object_ids;///< map from empire id to (set of object ids that the empire has previously observed but has subsequently been unable to detect at its last known location despite expecting to be able to detect it based on stealth of the object and having detectors in range)

    ShipDesignMap                   m_ship_designs;                     ///< ship designs in the universe
    std::map<int, std::set<int>>    m_empire_known_ship_design_ids;     ///< ship designs known to each empire

    Effect::AccountingMap           m_effect_accounting_map;            ///< map from target object id, to map from target meter, to orderered list of structs with details of an effect and what it does to the meter

    /// map from target object id, to map from target meter, to discrepancy
    /// between meter's actual initial value, and the initial value that this
    /// meter should have as far as the client can tell: the unknown factor
    /// affecting the meter.
    DiscrepancyMap                  m_effect_discrepancy_map;

    std::map<int, std::set<int>>    m_marked_destroyed;                 ///< used while applying effects to cache objects that have been destroyed.  this allows to-be-destroyed objects to remain undestroyed until all effects have been processed, which ensures that to-be-destroyed objects still exist when other effects need to access them as a source object. key is destroyed object, and value set are the ids of objects that caused the destruction (may be multiples destroying a single target on a given turn)

    double                          m_universe_width = 1000.0;
    bool                            m_inhibit_universe_object_signals = false;

    std::map<std::string, std::map<int, std::map<int, double>>>
                                    m_stat_records;                     ///< storage for statistics calculated for empires. Indexed by stat name (string), contains a map indexed by empire id, contains a map from turn number (int) to stat value (double).

    //! @name Parsed items
    //! Various unlocked items are kept as a Pending::Pending while being parsed and
    //! then transfered.  They are mutable to allow processing in const accessors.
    //! @{
    mutable boost::optional<Pending::Pending<std::vector<UnlockableItem>>>                      m_pending_items = boost::none;
    mutable boost::optional<Pending::Pending<std::vector<UnlockableItem>>>                      m_pending_buildings = boost::none;
    mutable boost::optional<Pending::Pending<std::vector<std::unique_ptr<FleetPlan>>>>          m_pending_fleet_plans = boost::none;
    mutable boost::optional<Pending::Pending<std::vector<std::unique_ptr<MonsterFleetPlan>>>>   m_pending_monster_fleet_plans = boost::none;
    mutable boost::optional<Pending::Pending<EmpireStatsMap>>                                   m_pending_empire_stats = boost::none;

    mutable std::vector<UnlockableItem>                     m_unlocked_items;
    mutable std::vector<UnlockableItem>                     m_unlocked_buildings;
    mutable std::vector<std::unique_ptr<FleetPlan>>         m_unlocked_fleet_plans;
    mutable std::vector<std::unique_ptr<MonsterFleetPlan>>  m_monster_fleet_plans;
    mutable EmpireStatsMap                                  m_empire_stats;
    //! @}

    /** Fills \a designs_to_serialize with ShipDesigns known to the empire with
      * the ID \a encoding empire.  If encoding_empire is ALL_EMPIRES, then all
      * designs are included. */
    const ShipDesignMap& GetShipDesignsToSerialize(ShipDesignMap& designs_to_serialize, int encoding_empire) const;

    /** Fills \a objects with copies of UniverseObjects that should be sent
      * to the empire with id \a encoding_empires */
    void GetObjectsToSerialize(ObjectMap& objects, int encoding_empire) const;

    /** Fills \a destroyed_object_ids with ids of objects known to be destroyed
      * by the empire with ID \a encoding empire. If encoding_empire is
      * ALL_EMPIRES, then all destroyed objects are included. */
    void GetDestroyedObjectsToSerialize(std::set<int>& destroyed_object_ids, int encoding_empire) const;

    /** Fills \a empire_latest_known_objects map with the latest known data
      * about UniverseObjects for the empire with id \a encoding_empire.  If
      * the encoding empire is ALL_EMPIRES then all stored empire object
      * knowledge is included. */
    void GetEmpireKnownObjectsToSerialize(EmpireObjectMap& empire_latest_known_objects, int encoding_empire) const;

    /***/
    void GetEmpireObjectVisibilityMap(EmpireObjectVisibilityMap& empire_object_visibility, int encoding_empire) const;

    /***/
    void GetEmpireObjectVisibilityTurnMap(EmpireObjectVisibilityTurnMap& empire_object_visibility_turns, int encoding_empire) const;

    /***/
    //void GetEffectSpecifiedVisibilities(EmpireObjectVisibilityMap& effect_specified_empire_object_visibilities, int encoding_empire) const;

    /***/
    void GetEmpireKnownDestroyedObjects(ObjectKnowledgeMap& empire_known_destroyed_object_ids, int encoding_empire) const;

    /***/
    void GetEmpireStaleKnowledgeObjects(ObjectKnowledgeMap& empire_stale_knowledge_object_ids, int encoding_empire) const;

    /** Manages allocating and verifying new object ids.*/
    std::unique_ptr<IDAllocator> m_object_id_allocator;

    /** Manages allocating and verifying new ship design ids.*/
    std::unique_ptr<IDAllocator> m_design_id_allocator;

    template <typename Archive>
    friend void serialize(Archive&, Universe&, unsigned int const);
};

class SpeciesManager;

/** Compute a checksum for each of the universe's content managers. Each value will be of the form
    ("BuildingManager", <checksum>) */
FO_COMMON_API std::map<std::string, unsigned int> CheckSumContent(const SpeciesManager& species);


#endif
