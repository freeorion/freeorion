#include "Universe.h"

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
#include "Effect.h"
#include "Predicates.h"
#include "Special.h"
#include "Species.h"
#include "Condition.h"
#include "ValueRef.h"
#include "Enums.h"
#include "Pathfinder.h"

#include <boost/property_map/property_map.hpp>
#include <boost/timer.hpp>


#if defined(_MSC_VER)
#  if (_MSC_VER == 1900)
namespace boost {
    const volatile Universe* get_pointer(const volatile Universe* p) { return p; }
}
#  endif
#endif

namespace {
    const bool ENABLE_VISIBILITY_EMPIRE_MEMORY = true;      // toggles using memory with visibility, so that empires retain knowledge of objects viewed on previous turns

    void AddOptions(OptionsDB& db) {
        db.Add("verbose-logging",           UserStringNop("OPTIONS_DB_VERBOSE_LOGGING_DESC"),           false,  Validator<bool>());
        db.Add("verbose-combat-logging",    UserStringNop("OPTIONS_DB_VERBOSE_COMBAT_LOGGING_DESC"),    false,  Validator<bool>());
        db.Add("effects-threads-ui",        UserStringNop("OPTIONS_DB_EFFECTS_THREADS_UI_DESC"),        8,      RangedValidator<int>(1, 32));
        db.Add("effects-threads-ai",        UserStringNop("OPTIONS_DB_EFFECTS_THREADS_AI_DESC"),        2,      RangedValidator<int>(1, 32));
        db.Add("effects-threads-server",    UserStringNop("OPTIONS_DB_EFFECTS_THREADS_SERVER_DESC"),    8,      RangedValidator<int>(1, 32));
        db.Add("effect-accounting",         UserStringNop("OPTIONS_DB_EFFECT_ACCOUNTING"),              true,   Validator<bool>());
        db.Add("reseed-prng-server",        UserStringNop("OPTIONS_DB_PRNG_RESEEDING"),                 true, Validator<bool>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    const double    WORMHOLE_TRAVEL_DISTANCE = 0.1;         // the effective distance for ships travelling along a wormhole, for determining how much of their speed is consumed by the jump

    template <class Key, class Value> struct constant_property
    { Value m_value; };
}

namespace boost {
    template <class Key, class Value> struct property_traits<constant_property<Key, Value> > {
        typedef Value value_type;
        typedef Key key_type;
        typedef readable_property_map_tag category;
    };
    template <class Key, class Value> const Value& get(const constant_property<Key, Value>& pmap, const Key&) { return pmap.m_value; }
}


extern const int ALL_EMPIRES            = -1;
// TODO: implement a robust, thread-safe solution for creating multiple client-local temporary objects with unique IDs that will never conflict with each other or the server.
extern const int MAX_ID                 = 2000000000;

namespace EmpireStatistics {
    const std::map<std::string, ValueRef::ValueRefBase<double>*>& GetEmpireStats() {
        static std::map<std::string, ValueRef::ValueRefBase<double>*> s_stats;
        if (s_stats.empty()) {
            try {
                parse::statistics(s_stats);
            } catch (const std::exception& e) {
                ErrorLogger() << "Failed parsing empire_statistics.txt: error: " << e.what();
                throw e;
            }
        }
        return s_stats;
    }
}

/////////////////////////////////////////////
// class Universe
/////////////////////////////////////////////
Universe::Universe() :
    m_pathfinder(new Pathfinder),
    m_last_allocated_object_id(-1), // this is conicidentally equal to INVALID_OBJECT_ID as of this writing, but the reason for this to be -1 is so that the first object has id 0, and all object ids are non-negative
    m_last_allocated_design_id(-1), // same, but for ShipDesign::INVALID_DESIGN_ID
    m_universe_width(1000.0),
    m_inhibit_universe_object_signals(false),
    m_encoding_empire(ALL_EMPIRES),
    m_all_objects_visible(false)
{}

Universe::~Universe() {
    Clear();
}

void Universe::Clear() {
    // empty object maps
    m_objects.Clear();
    for (EmpireObjectMap::value_type& entry : m_empire_latest_known_objects)
        entry.second.Clear();
    m_empire_latest_known_objects.clear();

    // clean up ship designs
    for (ShipDesignMap::value_type& entry : m_ship_designs)
        delete entry.second;
    m_ship_designs.clear();

    m_destroyed_object_ids.clear();

    m_empire_object_visibility.clear();
    m_empire_object_visibility_turns.clear();

    m_empire_object_visible_specials.clear();

    m_effect_accounting_map.clear();
    m_effect_discrepancy_map.clear();

    m_last_allocated_object_id = -1;
    m_last_allocated_design_id = -1;

    m_empire_known_destroyed_object_ids.clear();
    m_empire_stale_knowledge_object_ids.clear();

    m_empire_known_ship_design_ids.clear();

    m_marked_destroyed.clear();
}

const ObjectMap& Universe::EmpireKnownObjects(int empire_id) const {
    if (empire_id == ALL_EMPIRES)
        return m_objects;

    EmpireObjectMap::const_iterator it = m_empire_latest_known_objects.find(empire_id);
    if (it != m_empire_latest_known_objects.end())
        return it->second;

    static const ObjectMap const_empty_map;
    return const_empty_map;
}

ObjectMap& Universe::EmpireKnownObjects(int empire_id) {
    if (empire_id == ALL_EMPIRES)
        return m_objects;

    EmpireObjectMap::iterator it = m_empire_latest_known_objects.find(empire_id);
    if (it != m_empire_latest_known_objects.end())
        return it->second;

    static ObjectMap empty_map;
    empty_map.Clear();
    return empty_map;
}

std::set<int> Universe::EmpireVisibleObjectIDs(int empire_id/* = ALL_EMPIRES*/) const {
    std::set<int> retval;

    // get id(s) of all empires to consider visibility of...
    std::set<int> empire_ids;
    if (empire_id != ALL_EMPIRES)
        empire_ids.insert(empire_id);
    else
        for (const std::map<int, Empire*>::value_type& empire_entry : Empires())
            empire_ids.insert(empire_entry.first);

    // check each object's visibility against all empires, including the object
    // if an empire has visibility of it
    for (ObjectMap::const_iterator<> obj_it = m_objects.const_begin(); obj_it != m_objects.const_end(); ++obj_it) {
        int id = obj_it->ID();
        for (int empire_id : empire_ids) {
            Visibility vis = GetObjectVisibilityByEmpire(id, empire_id);
            if (vis >= VIS_BASIC_VISIBILITY) {
                retval.insert(id);
                break;
            }
        }
    }

    return retval;
}

const std::set<int>& Universe::DestroyedObjectIds() const
{ return m_destroyed_object_ids; }

const std::set<int>& Universe::EmpireKnownDestroyedObjectIDs(int empire_id) const {
    ObjectKnowledgeMap::const_iterator it = m_empire_known_destroyed_object_ids.find(empire_id);
    if (it != m_empire_known_destroyed_object_ids.end())
        return it->second;
    return m_destroyed_object_ids;
}

const std::set<int>& Universe::EmpireStaleKnowledgeObjectIDs(int empire_id) const {
    ObjectKnowledgeMap::const_iterator it = m_empire_stale_knowledge_object_ids.find(empire_id);
    if (it != m_empire_stale_knowledge_object_ids.end())
        return it->second;
    static const std::set<int> empty_set;
    return empty_set;
}

const ShipDesign* Universe::GetShipDesign(int ship_design_id) const {
    if (ship_design_id == ShipDesign::INVALID_DESIGN_ID)
        return nullptr;
    ship_design_iterator it = m_ship_designs.find(ship_design_id);
    return (it != m_ship_designs.end() ? it->second : nullptr);
}

void Universe::RenameShipDesign(int design_id, const std::string& name/* = ""*/, const std::string& description/* = ""*/) {
    ShipDesignMap::iterator design_it = m_ship_designs.find(design_id);
    if (design_it == m_ship_designs.end()) {
        DebugLogger() << "Universe::RenameShipDesign tried to rename a ship design that doesn't exist!";
        return;
    }
    ShipDesign* design = design_it->second;

    if (name != "") {
        design->SetName(name);
    }
    if (description != "") {
        design->SetDescription(description);
    }
}

const ShipDesign* Universe::GetGenericShipDesign(const std::string& name) const {
    if (name.empty())
        return nullptr;
    for (const std::map<int, ShipDesign*>::value_type& entry : m_ship_designs) {
        const ShipDesign* design = entry.second;
        const std::string& design_name = design->Name(false);
        if (name == design_name)
            return design;
    }
    return nullptr;
}

const std::set<int>& Universe::EmpireKnownShipDesignIDs(int empire_id) const {
    std::map<int, std::set<int> >::const_iterator it = m_empire_known_ship_design_ids.find(empire_id);
    if (it != m_empire_known_ship_design_ids.end())
        return it->second;
    static const std::set<int> empty_set;
    return empty_set;
}

Visibility Universe::GetObjectVisibilityByEmpire(int object_id, int empire_id) const {
    if (empire_id == ALL_EMPIRES || GetUniverse().AllObjectsVisible())
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

const Universe::VisibilityTurnMap& Universe::GetObjectVisibilityTurnMapByEmpire(int object_id, int empire_id) const {
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

std::set<std::string> Universe::GetObjectVisibleSpecialsByEmpire(int object_id, int empire_id) const {
    if (empire_id != ALL_EMPIRES) {
        EmpireObjectSpecialsMap::const_iterator empire_it = m_empire_object_visible_specials.find(empire_id);
        if (empire_it == m_empire_object_visible_specials.end())
            return std::set<std::string>();
        const ObjectSpecialsMap& object_specials_map = empire_it->second;
        ObjectSpecialsMap::const_iterator object_it = object_specials_map.find(object_id);
        if (object_it == object_specials_map.end())
            return std::set<std::string>();
        return object_it->second;
    } else {
        std::shared_ptr<const UniverseObject> obj = m_objects.Object(object_id);
        if (!obj)
            return std::set<std::string>();
        // all specials visible
        std::set<std::string> retval;
        for (const std::map<std::string, std::pair<int, float>>::value_type& entry : obj->Specials()) {
            retval.insert(entry.first);
        }
        return retval;
    }
}

const int Universe::GetNumCombatRounds() const
{ return 3; }

int Universe::GenerateObjectID() {
    if (m_last_allocated_object_id + 1 < MAX_ID)
        return ++m_last_allocated_object_id;
    // the object id number space is exhausted, which means we're screwed
    ErrorLogger() << "Universe::GenerateObjectID: object id number space exhausted!";
    return INVALID_OBJECT_ID;
}

template <class T>
std::shared_ptr<T> Universe::Insert(T* obj) {
    if (!obj)
        return nullptr;

    int id = GenerateObjectID();
    if (id != INVALID_OBJECT_ID) {
        obj->SetID(id);
        return m_objects.Insert(obj);
    }

    // Avoid leaking memory if there are more than 2^31 objects in the Universe.
    // Realistically, we should probably do something a little more drastic in this case,
    // like terminate the program and call 911 or something.
    delete obj;
    return nullptr;
}

template <class T>
std::shared_ptr<T> Universe::InsertID(T* obj, int id) {
    if (id == INVALID_OBJECT_ID)
        return Insert(obj);
    if (!obj || id >= MAX_ID)
        return nullptr;

    obj->SetID(id);
    std::shared_ptr<T> result = m_objects.Insert(obj);
    if (id > m_last_allocated_object_id )
        m_last_allocated_object_id = id;
    DebugLogger() << "Inserting object with id " << id;
    return result;
}

int Universe::InsertShipDesign(ShipDesign* ship_design) {
    int retval = ShipDesign::INVALID_DESIGN_ID;
    if (ship_design) {
        if (m_last_allocated_design_id + 1 < MAX_ID) {
            m_ship_designs[++m_last_allocated_design_id] = ship_design;
            ship_design->SetID(m_last_allocated_design_id);
            retval = m_last_allocated_design_id;
        } else { // we'll probably never execute this branch, considering how many IDs are available
            // find a hole in the assigned IDs in which to place the object
            int last_id_seen = ShipDesign::INVALID_DESIGN_ID;
            for (ShipDesignMap::value_type& entry : m_ship_designs) {
                if (1 < entry.first - last_id_seen) {
                    m_ship_designs[last_id_seen + 1] = ship_design;
                    ship_design->SetID(last_id_seen + 1);
                    retval = last_id_seen + 1;
                    break;
                }
            }
        }
    }
    return retval;
}

bool Universe::InsertShipDesignID(ShipDesign* ship_design, int id) {
    bool retval = false;

    if (ship_design  &&  id != ShipDesign::INVALID_DESIGN_ID  &&  id < ShipDesign::MAX_ID) {
        ship_design->SetID(id);
        m_ship_designs[id] = ship_design;
        retval = true;
    }
    return retval;
}

bool Universe::DeleteShipDesign(int design_id) {
    ShipDesignMap::iterator it = m_ship_designs.find(design_id);
    if (it != m_ship_designs.end()) {
        m_ship_designs.erase(it);
        return true;
    } else { return false; }
}

void Universe::ApplyAllEffectsAndUpdateMeters(bool do_accounting) {
    ScopedTimer timer("Universe::ApplyAllEffectsAndUpdateMeters");

    if (do_accounting) {
        // override if option disabled
        do_accounting = GetOptionsDB().Get<bool>("effect-accounting");
    }

    m_effect_specified_empire_object_visibilities.clear();

    // cache all activation and scoping condition results before applying
    // Effects, since the application of these Effects may affect the activation
    // and scoping evaluations
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes);

    // revert all current meter values (which are modified by effects) to
    // their initial state for this turn, so that max/target/unpaired meter
    // value can be calculated (by accumulating all effects' modifications this
    // turn) and active meters have the proper baseline from which to
    // accumulate changes from effects
    for (std::shared_ptr<UniverseObject> object : m_objects) {
        object->ResetTargetMaxUnpairedMeters();
        object->ResetPairedActiveMeters();
    }
    for (std::map<int, Empire*>::value_type& entry : Empires())
        entry.second->ResetMeters();

    ExecuteEffects(targets_causes, do_accounting, false, false, true);
    // clamp max meters to [DEFAULT_VALUE, LARGE_VALUE] and current meters to [DEFAULT_VALUE, max]
    // clamp max and target meters to [DEFAULT_VALUE, LARGE_VALUE] and current meters to [DEFAULT_VALUE, max]
    for (std::shared_ptr<UniverseObject> object : m_objects)
        object->ClampMeters();
}

void Universe::ApplyMeterEffectsAndUpdateMeters(const std::vector<int>& object_ids, bool do_accounting) {
    if (object_ids.empty())
        return;
    ScopedTimer timer("Universe::ApplyMeterEffectsAndUpdateMeters on " + std::to_string(object_ids.size()) + " objects");
    if (do_accounting) {
        // override if disabled
        do_accounting = GetOptionsDB().Get<bool>("effect-accounting");
    }
    // cache all activation and scoping condition results before applying Effects, since the application of
    // these Effects may affect the activation and scoping evaluations
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes, object_ids);

    std::vector<std::shared_ptr<UniverseObject>> objects = m_objects.FindObjects(object_ids);

    // revert all current meter values (which are modified by effects) to
    // their initial state for this turn, so meter
    // value can be calculated (by accumulating all effects' modifications this
    // turn) and active meters have the proper baseline from which to
    // accumulate changes from effects
    for (std::shared_ptr<UniverseObject> object : objects) {
        object->ResetTargetMaxUnpairedMeters();
        object->ResetPairedActiveMeters();
    }
    // could also reset empire meters here, but unless all objects have meters
    // recalculated, some targets that lead to empire meters being modified may
    // be missed, and estimated empire meters would be inaccurate

    ExecuteEffects(targets_causes, do_accounting, true);

    for (std::shared_ptr<UniverseObject> object : objects)
        object->ClampMeters();
}

void Universe::ApplyMeterEffectsAndUpdateMeters(bool do_accounting) {
    ScopedTimer timer("Universe::ApplyMeterEffectsAndUpdateMeters on all objects");
    if (do_accounting) {
        // override if disabled
        do_accounting = GetOptionsDB().Get<bool>("effect-accounting");
    }

    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes);

    for (std::shared_ptr<UniverseObject> object : m_objects) {
        object->ResetTargetMaxUnpairedMeters();
        object->ResetPairedActiveMeters();
    }
    for (std::map<int, Empire*>::value_type& entry : Empires())
        entry.second->ResetMeters();
    ExecuteEffects(targets_causes, do_accounting, true, false, true);

    for (std::shared_ptr<UniverseObject> object : m_objects)
        object->ClampMeters();
}

void Universe::ApplyMeterEffectsAndUpdateTargetMaxUnpairedMeters(bool do_accounting) {
    ScopedTimer timer("Universe::ApplyMeterEffectsAndUpdateMeters on all objects");
    if (do_accounting) {
        // override if disabled
        do_accounting = GetOptionsDB().Get<bool>("effect-accounting");
    }
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes);

    for (std::shared_ptr<UniverseObject> object : m_objects) {
        object->ResetTargetMaxUnpairedMeters();
    }

    ExecuteEffects(targets_causes, do_accounting, true, false, true);

    for (std::shared_ptr<UniverseObject> object : m_objects)
        object->ClampMeters();
}

void Universe::ApplyAppearanceEffects(const std::vector<int>& object_ids) {
    if (object_ids.empty())
        return;
    ScopedTimer timer("Universe::ApplyAppearanceEffects on " + std::to_string(object_ids.size()) + " objects");

    // cache all activation and scoping condition results before applying
    // Effects, since the application of these Effects may affect the
    // activation and scoping evaluations
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes, object_ids);
    ExecuteEffects(targets_causes, false, false, true);
}

void Universe::ApplyAppearanceEffects() {
    ScopedTimer timer("Universe::ApplyAppearanceEffects on all objects");

    // cache all activation and scoping condition results before applying
    // Effects, since the application of Effects in general (even if not these
    // particular Effects) may affect the activation and scoping evaluations
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes);
    ExecuteEffects(targets_causes, false, false, true);
}

void Universe::ApplyGenerateSitRepEffects() {
    ScopedTimer timer("Universe::ApplyGenerateSitRepEffects on all objects");

    // cache all activation and scoping condition results before applying
    // Effects, since the application of Effects in general (even if not these
    // particular Effects) may affect the activation and scoping evaluations
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes);
    ExecuteEffects(targets_causes, false, false, false, false, true);
}

void Universe::InitMeterEstimatesAndDiscrepancies() {
    DebugLogger() << "Universe::InitMeterEstimatesAndDiscrepancies";
    ScopedTimer timer("Universe::InitMeterEstimatesAndDiscrepancies");

    // clear old discrepancies and accounting
    m_effect_discrepancy_map.clear();
    m_effect_accounting_map.clear();

    DebugLogger() << "IMEAD: updating meter estimates";

    // generate new estimates (normally uses discrepancies, but in this case will find none)
    UpdateMeterEstimates();

    DebugLogger() << "IMEAD: determining discrepancies";
    // determine meter max discrepancies
    for (Effect::AccountingMap::value_type& entry : m_effect_accounting_map) {
        int object_id = entry.first;
        // skip destroyed objects
        if (m_destroyed_object_ids.find(object_id) != m_destroyed_object_ids.end())
            continue;
        // get object
        std::shared_ptr<UniverseObject> obj = m_objects.Object(object_id);
        if (!obj) {
            ErrorLogger() << "Universe::InitMeterEstimatesAndDiscrepancies couldn't find an object that was in the effect accounting map...?";
            continue;
        }

        // every meter has a value at the start of the turn, and a value after updating with known effects
        for (std::map<MeterType, Meter>::value_type& entry : obj->Meters()) {
            MeterType type = entry.first;
            Meter& meter = entry.second;

            // discrepancy is the difference between expected and actual meter values at start of turn
            float discrepancy = meter.Initial() - meter.Current();

            if (discrepancy == 0.0f) continue;   // no discrepancy for this meter

            // add to discrepancy map
            m_effect_discrepancy_map[object_id][type] = discrepancy;

            // correct current max meter estimate for discrepancy
            meter.AddToCurrent(discrepancy);

            // add discrepancy adjustment to meter accounting
            Effect::AccountingInfo info;
            info.cause_type = ECT_UNKNOWN_CAUSE;
            info.meter_change = discrepancy;
            info.running_meter_total = meter.Current();

            m_effect_accounting_map[object_id][type].push_back(info);
        }
    }
}

void Universe::UpdateMeterEstimates()
{ UpdateMeterEstimates(INVALID_OBJECT_ID, false); }

void Universe::UpdateMeterEstimates(int object_id, bool update_contained_objects) {
    if (object_id == INVALID_OBJECT_ID) {
        for (int obj_id : m_objects.FindExistingObjectIDs())
            m_effect_accounting_map[obj_id].clear();
        // update meters for all objects.  Value of updated_contained_objects is irrelivant and is ignored in this case.
        UpdateMeterEstimatesImpl(std::vector<int>());// will cause it to process all existing objects
        return;
    }

    // collect objects to update meter for.  this may be a single object, a group of related objects, or all objects
    // in the (known) universe.  also clear effect accounting for meters that are to be updated.
    std::set<int> objects_set;
    std::list<int> objects_list;
    objects_list.push_back(object_id);

    for (int cur_object_id : objects_list) {
        std::shared_ptr<UniverseObject> cur_object = m_objects.Object(cur_object_id);
        if (!cur_object) {
            ErrorLogger() << "Universe::UpdateMeterEstimates tried to get an invalid object...";
            return;
        }

        // add object and clear effect accounting for all its meters
        objects_set.insert(cur_object_id);
        m_effect_accounting_map[cur_object_id].clear();

        // add contained objects to list of objects to process, if requested.
        // assumes no objects contain themselves (which could cause infinite loops)
        if (update_contained_objects) {
            const std::set<int>& contained_objects = cur_object->ContainedObjectIDs();
            std::copy(contained_objects.begin(), contained_objects.end(), std::back_inserter(objects_list));
        }
    }
    std::vector<int> objects_vec;
    objects_vec.reserve(objects_set.size());
    std::copy(objects_set.begin(), objects_set.end(), std::back_inserter(objects_vec));
    if (!objects_vec.empty())
        UpdateMeterEstimatesImpl(objects_vec);
}

void Universe::UpdateMeterEstimates(const std::vector<int>& objects_vec) {
    std::set<int> objects_set;  // ensures no duplicates

    for (int object_id : objects_vec) {
        // skip destroyed objects
        if (m_destroyed_object_ids.find(object_id) != m_destroyed_object_ids.end())
            continue;
        m_effect_accounting_map[object_id].clear();
        objects_set.insert(object_id);
    }
    std::vector<int> final_objects_vec;
    std::copy(objects_set.begin(), objects_set.end(), std::back_inserter(final_objects_vec));
    if (!final_objects_vec.empty())
        UpdateMeterEstimatesImpl(final_objects_vec);
}

void Universe::UpdateMeterEstimatesImpl(const std::vector<int>& objects_vec) {
    ScopedTimer timer("Universe::UpdateMeterEstimatesImpl on " + std::to_string(objects_vec.size()) + " objects", true);
    bool do_accounting = GetOptionsDB().Get<bool>("effect-accounting");

    // get all pointers to objects once, to avoid having to do so repeatedly
    // when iterating over the list in the following code
    std::vector<std::shared_ptr<UniverseObject>> object_ptrs = m_objects.FindObjects(objects_vec);
    if (objects_vec.empty()) {
        object_ptrs.reserve(m_objects.ExistingObjects().size());
        std::transform(Objects().ExistingObjects().begin(), Objects().ExistingObjects().end(),
                       std::back_inserter(object_ptrs),
                       boost::bind(&std::map<int, std::shared_ptr<UniverseObject>>::value_type::second, _1));
    }

    for (std::shared_ptr<UniverseObject> obj : object_ptrs) {
        int obj_id = obj->ID();

        // Reset max meters to DEFAULT_VALUE and current meters to initial value at start of this turn
        obj->ResetTargetMaxUnpairedMeters();
        obj->ResetPairedActiveMeters();

        if (!do_accounting)
            continue;

        // record current value(s) of meters after resetting
        for (MeterType type = MeterType(0); type != NUM_METER_TYPES; type = MeterType(type + 1)) {
            if (Meter* meter = obj->GetMeter(type)) {
                Effect::AccountingInfo info;
                info.source_id = INVALID_OBJECT_ID;
                info.cause_type = ECT_INHERENT;
                info.meter_change = meter->Current() - Meter::DEFAULT_VALUE;
                info.running_meter_total = meter->Current();

                if (info.meter_change != 0.0f)
                    m_effect_accounting_map[obj_id][type].push_back(info);
            }
        }
    }

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        DebugLogger() << "UpdateMeterEstimatesImpl after resetting meters objects:";
        for (std::shared_ptr<UniverseObject> obj : object_ptrs) {
            DebugLogger() << obj->Dump();
        }
    }

    // cache all activation and scoping condition results before applying Effects, since the application of
    // these Effects may affect the activation and scoping evaluations
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes, objects_vec);

    // Apply and record effect meter adjustments
    ExecuteEffects(targets_causes, do_accounting, true, false, false, false);

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        DebugLogger() << "UpdateMeterEstimatesImpl after executing effects objects:";
        for (std::shared_ptr<UniverseObject> obj : object_ptrs) {
            DebugLogger() << obj->Dump();
        }
    }

    // Apply known discrepancies between expected and calculated meter maxes at start of turn.  This
    // accounts for the unknown effects on the meter, and brings the estimate in line with the actual
    // max at the start of the turn
    if (!m_effect_discrepancy_map.empty() && do_accounting) {
        for (std::shared_ptr<UniverseObject> obj : object_ptrs) {
            int obj_id = obj->ID();

            // check if this object has any discrepancies
            Effect::DiscrepancyMap::iterator dis_it = m_effect_discrepancy_map.find(obj_id);
            if (dis_it == m_effect_discrepancy_map.end())
                continue;   // no discrepancy, so skip to next object

            // apply all meters' discrepancies
            for (std::map<MeterType, double>::value_type& entry : dis_it->second) {
                MeterType type = entry.first;
                double discrepancy = entry.second;

                //if (discrepancy == 0.0) continue;

                Meter* meter = obj->GetMeter(type);

                if (meter) {
                    if (GetOptionsDB().Get<bool>("verbose-logging"))
                        DebugLogger() << "object " << obj_id << " has meter " << type
                                      << ": discrepancy: " << discrepancy << " and : " << meter->Dump();

                    meter->AddToCurrent(discrepancy);

                    Effect::AccountingInfo info;
                    info.cause_type = ECT_UNKNOWN_CAUSE;
                    info.meter_change = discrepancy;
                    info.running_meter_total = meter->Current();

                    m_effect_accounting_map[obj_id][type].push_back(info);
                }
            }
        }
    }

    // clamp meters to valid range of max values, and so current is less than max
    for (std::shared_ptr<UniverseObject> obj : object_ptrs) {
        // currently this clamps all meters, even if not all meters are being processed by this function...
        // but that shouldn't be a problem, as clamping meters that haven't changed since they were last
        // updated should have no effect
        obj->ClampMeters();
    }

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        DebugLogger() << "UpdateMeterEstimatesImpl after discrepancies and clamping objects:";
        for (std::shared_ptr<UniverseObject> obj : object_ptrs) {
            DebugLogger() << obj->Dump();
        }
    }
}

void Universe::BackPropagateObjectMeters(const std::vector<int>& object_ids) {
    // copy current meter values to initial values
    for (std::shared_ptr<UniverseObject> obj : m_objects.FindObjects(object_ids))
        obj->BackPropagateMeters();
}

void Universe::BackPropagateObjectMeters()
{ BackPropagateObjectMeters(m_objects.FindObjectIDs()); }

namespace {
    /** Used by GetEffectsAndTargets to process a vector of effects groups.
      * Stores target set of specified \a effects_groups and \a source_object_id
      * in \a targets_causes
      * NOTE: this method will modify target_objects temporarily, but restore
      * its contents before returning. 
      * This is a calleable class instead of an ordinary method so that we can
      * use it as work item in parallel scheduling.
      */
    class StoreTargetsAndCausesOfEffectsGroupsWorkItem {
    public:
        struct ConditionCache : public boost::noncopyable {
        public:
            std::pair<bool, Effect::TargetSet>* Find(const Condition::ConditionBase* cond, bool insert);
            void MarkComplete(std::pair<bool, Effect::TargetSet>* cache_entry);
            void LockShared(boost::shared_lock<boost::shared_mutex>& guard);
        private:
            std::map<const Condition::ConditionBase*, std::pair<bool, Effect::TargetSet> > m_entries;
            boost::shared_mutex m_mutex;
            boost::condition_variable_any m_state_changed;
        };
        StoreTargetsAndCausesOfEffectsGroupsWorkItem(
            const std::shared_ptr<Effect::EffectsGroup>& the_effects_group,
            const std::vector<std::shared_ptr<const UniverseObject>>& the_sources,
            EffectsCauseType                                        the_effect_cause_type,
            const std::string&                                      the_specific_cause_name,
            Effect::TargetSet&                                      the_target_objects,
            Effect::TargetsCauses&                                  the_targets_causes,
            std::map<int, std::shared_ptr<ConditionCache>>& the_source_cached_condition_matches,
            ConditionCache&                                         the_invariant_cached_condition_matches,
            boost::shared_mutex&                                    the_global_mutex
        );
        void operator ()();
    private:
        // WARNING: do NOT copy the shared_pointers! Use raw pointers, shared_ptr may not be thread-safe. 
        std::shared_ptr<Effect::EffectsGroup> m_effects_group;
        const std::vector<std::shared_ptr<const UniverseObject>>* m_sources;
        EffectsCauseType                                        m_effect_cause_type;
        const std::string                                       m_specific_cause_name;
        Effect::TargetSet*                                      m_target_objects;
        Effect::TargetsCauses*                                  m_targets_causes;
        std::map<int, std::shared_ptr<ConditionCache>>* m_source_cached_condition_matches;
        ConditionCache*                                         m_invariant_cached_condition_matches;
        boost::shared_mutex*                                    m_global_mutex;

        static Effect::TargetSet& GetConditionMatches(
            const Condition::ConditionBase*    cond,
            ConditionCache&                    cached_condition_matches,
            std::shared_ptr<const UniverseObject> source,
            const ScriptingContext&            source_context,
            Effect::TargetSet&                 target_objects);
    };

    StoreTargetsAndCausesOfEffectsGroupsWorkItem::StoreTargetsAndCausesOfEffectsGroupsWorkItem(
            const std::shared_ptr<Effect::EffectsGroup>& the_effects_group,
            const std::vector<std::shared_ptr<const UniverseObject>>& the_sources,
            EffectsCauseType                                        the_effect_cause_type,
            const std::string&                                      the_specific_cause_name,
            Effect::TargetSet&                                      the_target_objects,
            Effect::TargetsCauses&                                  the_targets_causes,
            std::map<int, std::shared_ptr<ConditionCache>>& the_source_cached_condition_matches,
            ConditionCache&                                         the_invariant_cached_condition_matches,
            boost::shared_mutex&                                    the_global_mutex
        ) :
            m_effects_group                         (the_effects_group),
            m_sources                               (&the_sources),
            m_effect_cause_type                     (the_effect_cause_type),
            // create a deep copy just in case string methods do unlocked copy-on-write or other unsafe things
            m_specific_cause_name                   (the_specific_cause_name.c_str()),
            m_target_objects                        (&the_target_objects),
            m_targets_causes                        (&the_targets_causes),
            m_source_cached_condition_matches       (&the_source_cached_condition_matches),
            m_invariant_cached_condition_matches    (&the_invariant_cached_condition_matches),
            m_global_mutex                          (&the_global_mutex)
    {}

    std::pair<bool, Effect::TargetSet>* StoreTargetsAndCausesOfEffectsGroupsWorkItem::ConditionCache::Find(
        const Condition::ConditionBase* cond, bool insert)
    {
        // have to iterate through cached condition matches, rather than using
        // find, since there is no operator< for comparing conditions by value
        // and by pointer is irrelivant.
        boost::unique_lock<boost::shared_mutex> unique_guard(m_mutex, boost::defer_lock_t());
        boost::shared_lock<boost::shared_mutex> shared_guard(m_mutex, boost::defer_lock_t());

        if (insert) unique_guard.lock(); else shared_guard.lock();

        for (std::map<const Condition::ConditionBase*, std::pair<bool,Effect::TargetSet>>::value_type& entry : m_entries) {
            if (*cond == *(entry.first)) {
                //DebugLogger() << "Reused target set!";

                if (insert) {
                    // no need to insert. downgrade lock
                    unique_guard.unlock();
                    shared_guard.lock();
                }

                // wait for cache fill
                while (!entry.second.first)
                    m_state_changed.wait(shared_guard);

                return &entry.second;
            }
        }

        // nothing found
        if (insert)
            // set up storage
            return &m_entries[cond];

        return nullptr;
    }

    void StoreTargetsAndCausesOfEffectsGroupsWorkItem::ConditionCache::MarkComplete(std::pair<bool, Effect::TargetSet>* cache_entry)
    {
        boost::unique_lock<boost::shared_mutex> cache_guard(m_mutex); // make sure threads are waiting for completion, not checking for completion
        cache_entry->first = true;
        m_state_changed.notify_all(); // signal cachefill
    }

    void StoreTargetsAndCausesOfEffectsGroupsWorkItem::ConditionCache::LockShared(boost::shared_lock<boost::shared_mutex>& guard) {
        boost::shared_lock<boost::shared_mutex> tmp_guard(m_mutex);
        guard.swap(tmp_guard);
    }

    Effect::TargetSet EMPTY_TARGET_SET;

    Effect::TargetSet& StoreTargetsAndCausesOfEffectsGroupsWorkItem::GetConditionMatches(
        const Condition::ConditionBase*                               cond,
        StoreTargetsAndCausesOfEffectsGroupsWorkItem::ConditionCache& cached_condition_matches,
        std::shared_ptr<const UniverseObject> source,
        const ScriptingContext&                                       source_context,
        Effect::TargetSet&                                            target_objects)
    {
        std::pair<bool, Effect::TargetSet>* cache_entry = nullptr;

        if (!cond)
            return EMPTY_TARGET_SET;

        // the passed-in cached_condition_matches are here expected be specific for the current source object
        cache_entry = cached_condition_matches.Find(cond, false);
        if (cache_entry)
            return cache_entry->second;

        // no cached result (yet). create cache entry
        cache_entry = cached_condition_matches.Find(cond, true);
        if (cache_entry->first)
            return cache_entry->second; // some other thread was faster creating the cache entry

        // no cached result. calculate it...

        Effect::TargetSet* target_set = &cache_entry->second;
        Condition::ObjectSet& matched_target_objects =
            *reinterpret_cast<Condition::ObjectSet*>(target_set);
        if (target_objects.empty()) {
            // move matches from default target candidates into target_set
            cond->Eval(source_context, matched_target_objects);
        } else {
            // move matches from candidates in target_objects into target_set
            Condition::ObjectSet& potential_target_objects =
                *reinterpret_cast<Condition::ObjectSet*>(&target_objects);

            // move matches from candidates in target_objects into target_set
            cond->Eval(source_context, matched_target_objects, potential_target_objects);
            // restore target_objects by copying objects back from targets to target_objects
            // this should be cheaper than doing a full copy because target_set is usually small
            target_objects.insert(target_objects.end(), target_set->begin(), target_set->end());
        }

        cached_condition_matches.MarkComplete(cache_entry);

        //DebugLogger() << "Generated new target set!";
        return *target_set; 
    }

    void StoreTargetsAndCausesOfEffectsGroupsWorkItem::operator()()
    {
        ScopedTimer timer("StoreTargetsAndCausesOfEffectsGroups");

        if (GetOptionsDB().Get<bool>("verbose-logging")) {
            boost::unique_lock<boost::shared_mutex> guard(*m_global_mutex);
            std::string sources_ids;
            for (std::shared_ptr<const UniverseObject> obj : *m_sources) {
                sources_ids += obj->Name() + " (" + std::to_string(obj->ID()) + ")  ";
            }
            DebugLogger() << "StoreTargetsAndCausesOfEffectsGroups: effects_group: " << m_effects_group->AccountingLabel()
                          << "  specific_cause: " << m_specific_cause_name
                          << "  sources: " << sources_ids << ")";
        }

        // get objects matched by scope
        const Condition::ConditionBase* scope = m_effects_group->Scope();
        if (!scope)
            return;

        // create temporary container for concurrent work
        Effect::TargetSet target_objects(*m_target_objects);
        // process all sources in set provided
        for (std::shared_ptr<const UniverseObject> source : *m_sources) {
            ScriptingContext source_context(source);
            int source_object_id = (source ? source->ID() : INVALID_OBJECT_ID);
            ScopedTimer update_timer("... StoreTargetsAndCausesOfEffectsGroups done processing source " +
                                     std::to_string(source_object_id) +
                                     " cause: " + m_specific_cause_name);

            // skip inactive sources
            // FIXME: is it safe to move this out of the loop? 
            // Activation condition must not contain "Source" subconditions in that case
            const Condition::ConditionBase* activation = m_effects_group->Activation();
            if (activation && !activation->Eval(source_context, source))
                continue;

            // if scope is source-invariant, use the source-invariant cache of condition results.
            // if scope depends on the source object, use a cache of condition results for that souce object.
            bool source_invariant = !source || scope->SourceInvariant();
            ConditionCache* condition_cache = source_invariant ? m_invariant_cached_condition_matches : (*m_source_cached_condition_matches)[source_object_id].get();
            // look up scope condition in the cache. if not found, calculate it and store in the cache. either way, return the result.
            Effect::TargetSet& target_set = GetConditionMatches(scope,
                                                                *condition_cache,
                                                                source,
                                                                source_context,
                                                                target_objects);
            {
                boost::shared_lock<boost::shared_mutex> cache_guard;

                condition_cache->LockShared(cache_guard);
                if (target_set.empty())
                    continue;
            }

            {
                // NOTE: std::shared_ptr copying is not thread-safe.
                // FIXME: use std::shared_ptr here, or a dedicated lock
                boost::unique_lock<boost::shared_mutex> guard(*m_global_mutex);

                // combine effects group and source object id into a sourced effects group
                Effect::SourcedEffectsGroup sourced_effects_group(source_object_id, m_effects_group);

                // combine cause type and specific cause into effect cause
                Effect::EffectCause effect_cause(m_effect_cause_type, m_specific_cause_name,
                                                 m_effects_group->AccountingLabel());

                // combine target set and effect cause
                Effect::TargetsAndCause target_and_cause(target_set, effect_cause);

                // store effect cause and targets info in map, indexed by sourced effects group
                m_targets_causes->push_back(std::make_pair(sourced_effects_group, target_and_cause));
            }
        }
    }

} // namespace

void Universe::GetEffectsAndTargets(Effect::TargetsCauses& targets_causes) {
    targets_causes.clear();
    GetEffectsAndTargets(targets_causes, std::vector<int>());
}

void Universe::GetEffectsAndTargets(Effect::TargetsCauses& targets_causes,
                                    const std::vector<int>& target_objects)
{
    ScopedTimer timer("Universe::GetEffectsAndTargets");

    // transfer target objects from input vector to a set
    Effect::TargetSet all_potential_targets = m_objects.FindObjects(target_objects);

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        DebugLogger() << "target objects:";
        for (std::shared_ptr<UniverseObject> obj : all_potential_targets) {
            DebugLogger() << obj->Dump();
        }
    }


    // caching space for each source object's results of finding matches for
    // scope conditions. Index INVALID_OBJECT_ID stores results for
    // source-invariant conditions
    typedef StoreTargetsAndCausesOfEffectsGroupsWorkItem::ConditionCache ConditionCache;
    std::map<int, std::shared_ptr<ConditionCache>> cached_source_condition_matches;

    // prepopulate the cache for safe concurrent access
    for (int obj_id : m_objects.FindObjectIDs()) {
        cached_source_condition_matches[obj_id] = std::make_shared<ConditionCache>();
    }

    cached_source_condition_matches[INVALID_OBJECT_ID] = std::make_shared<ConditionCache>();

    ConditionCache& invariant_condition_matches = *cached_source_condition_matches[INVALID_OBJECT_ID];

    boost::timer type_timer;
    boost::timer eval_timer;

    std::list<Effect::TargetsCauses> targets_causes_reorder_buffer; // create before run_queue, destroy after run_queue
    unsigned int num_threads = static_cast<unsigned int>(std::max(1, EffectsProcessingThreads()));
    RunQueue<StoreTargetsAndCausesOfEffectsGroupsWorkItem> run_queue(num_threads);
    boost::shared_mutex global_mutex;
    boost::unique_lock<boost::shared_mutex> global_lock(global_mutex); // create after run_queue, destroy before run_queue

    eval_timer.restart();

    // 1) EffectsGroups from Species
    if (GetOptionsDB().Get<bool>("verbose-logging"))
        DebugLogger() << "Universe::GetEffectsAndTargets for SPECIES";
    type_timer.restart();

    // find each species planets in single pass, maintaining object map order per-species
    std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>> species_objects;
    for (std::shared_ptr<Planet> planet : m_objects.FindObjects<Planet>()) {
        if (m_destroyed_object_ids.find(planet->ID()) != m_destroyed_object_ids.end())
            continue;
        const std::string& species_name = planet->SpeciesName();
        if (species_name.empty())
            continue;
        const Species* species = GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "GetEffectsAndTargets couldn't get Species " << species_name;
            continue;
        }
        species_objects[species_name].push_back(planet);
    }

    double planet_species_time = type_timer.elapsed();
    type_timer.restart();

    // find each species ships in single pass, maintaining object map order per-species
    for (std::shared_ptr<Ship> ship : m_objects.FindObjects<Ship>()) {
        if (m_destroyed_object_ids.find(ship->ID()) != m_destroyed_object_ids.end())
            continue;
        const std::string& species_name = ship->SpeciesName();
        if (species_name.empty())
            continue;
        const Species* species = GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "GetEffectsAndTargets couldn't get Species " << species_name;
            continue;
        }
        species_objects[species_name].push_back(ship);
    }
    double ship_species_time = type_timer.elapsed();

    // enforce species effects order
    for (const std::map<std::string, Species*>::value_type& entry : GetSpeciesManager()) {
        const std::string& species_name = entry.first;
        const Species*     species      = entry.second;
        std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>>::iterator species_objects_it =
            species_objects.find(species_name);

        if (species_objects_it == species_objects.end())
            continue;

        for (std::shared_ptr<Effect::EffectsGroup> effects_group : species->Effects()) {
            targets_causes_reorder_buffer.push_back(Effect::TargetsCauses());
            run_queue.AddWork(new StoreTargetsAndCausesOfEffectsGroupsWorkItem(
                effects_group, species_objects_it->second, ECT_SPECIES, species_name,
                all_potential_targets, targets_causes_reorder_buffer.back(),
                cached_source_condition_matches,
                invariant_condition_matches,
                global_mutex));
        }
    }

    // 2) EffectsGroups from Specials
    if (GetOptionsDB().Get<bool>("verbose-logging"))
        DebugLogger() << "Universe::GetEffectsAndTargets for SPECIALS";
    type_timer.restart();
    std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>> specials_objects;
    // determine objects with specials in a single pass
    for (std::shared_ptr<const UniverseObject> obj : m_objects) {
        int source_object_id = obj->ID();
        if (m_destroyed_object_ids.find(source_object_id) != m_destroyed_object_ids.end())
            continue;
        for (const std::map<const std::string, std::pair<int, float>>::value_type& entry : obj->Specials()) {
            const std::string& special_name = entry.first;
            const Special*     special      = GetSpecial(special_name);
            if (!special) {
                ErrorLogger() << "GetEffectsAndTargets couldn't get Special " << special_name;
                continue;
            }
            specials_objects[special_name].push_back(obj);
        }
    }
    // enforce specials effects order
    for (const std::string& special_name : SpecialNames()) {
        const Special*     special      = GetSpecial(special_name);
        std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>>::iterator specials_objects_it = specials_objects.find(special_name);

        if (specials_objects_it == specials_objects.end())
            continue;

        for (std::shared_ptr<Effect::EffectsGroup> effects_group : special->Effects()) {
            targets_causes_reorder_buffer.push_back(Effect::TargetsCauses());
            run_queue.AddWork(new StoreTargetsAndCausesOfEffectsGroupsWorkItem(
                effects_group, specials_objects_it->second, ECT_SPECIAL, special_name,
                all_potential_targets, targets_causes_reorder_buffer.back(),
                cached_source_condition_matches,
                invariant_condition_matches,
                global_mutex));
        }
    }
    double special_time = type_timer.elapsed();

    // 3) EffectsGroups from Techs
    if (GetOptionsDB().Get<bool>("verbose-logging"))
        DebugLogger() << "Universe::GetEffectsAndTargets for TECHS";
    type_timer.restart();
    std::list<std::vector<std::shared_ptr<const UniverseObject>>> tech_sources;
    for (std::map<int, Empire*>::value_type& entry : Empires()) {
        const Empire* empire = entry.second;
        std::shared_ptr<const UniverseObject> source = empire->Source();
        if (!source)
            continue;

        tech_sources.push_back(std::vector<std::shared_ptr<const UniverseObject>>(1U, source));
        for (const std::string& tech_name : empire->AvailableTechs()) {
            const Tech* tech = GetTech(tech_name);
            if (!tech) continue;

            for (std::shared_ptr<Effect::EffectsGroup> effects_group : tech->Effects()) {
                targets_causes_reorder_buffer.push_back(Effect::TargetsCauses());
                run_queue.AddWork(new StoreTargetsAndCausesOfEffectsGroupsWorkItem(
                    effects_group, tech_sources.back(), ECT_TECH, tech->Name(),
                    all_potential_targets, targets_causes_reorder_buffer.back(),
                    cached_source_condition_matches,
                    invariant_condition_matches,
                    global_mutex));
            }
        }
    }
    double tech_time = type_timer.elapsed();

    // 4) EffectsGroups from Buildings
    if (GetOptionsDB().Get<bool>("verbose-logging"))
        DebugLogger() << "Universe::GetEffectsAndTargets for BUILDINGS";
    type_timer.restart();

    // determine buildings of each type in a single pass
    std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>> buildings_by_type;
    for (std::shared_ptr<Building> building : m_objects.FindObjects<Building>()) {
        if (m_destroyed_object_ids.find(building->ID()) != m_destroyed_object_ids.end())
            continue;
        const std::string&  building_type_name = building->BuildingTypeName();
        const BuildingType* building_type = GetBuildingType(building_type_name);
        if (!building_type) {
            ErrorLogger() << "GetEffectsAndTargets couldn't get BuildingType " << building->BuildingTypeName();
            continue;
        }

        buildings_by_type[building_type_name].push_back(building);
    }

    // enforce building types effects order
    for (const std::map<std::string, BuildingType*>::value_type& entry : GetBuildingTypeManager()) {
        const std::string&  building_type_name = entry.first;
        const BuildingType* building_type      = entry.second;
        std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>>::iterator buildings_by_type_it =
            buildings_by_type.find(building_type_name);

        if (buildings_by_type_it == buildings_by_type.end())
            continue;

        for (std::shared_ptr<Effect::EffectsGroup> effects_group : building_type->Effects()) {
            targets_causes_reorder_buffer.push_back(Effect::TargetsCauses());
            run_queue.AddWork(new StoreTargetsAndCausesOfEffectsGroupsWorkItem(
                effects_group, buildings_by_type_it->second, ECT_BUILDING, building_type_name,
                all_potential_targets, targets_causes_reorder_buffer.back(),
                cached_source_condition_matches,
                invariant_condition_matches,
                global_mutex));
        }
    }
    double building_time = type_timer.elapsed();

    // 5) EffectsGroups from Ship Hull and Ship Parts
    if (GetOptionsDB().Get<bool>("verbose-logging"))
        DebugLogger() << "Universe::GetEffectsAndTargets for SHIPS hulls and parts";
    type_timer.restart();
    // determine ship hulls and parts of each type in a single pass
    // the same ship might be added multiple times if it contains the part multiple times
    // recomputing targets for the same ship and part is kind of silly here, but shouldn't hurt
    std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>> ships_by_hull_type;
    std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>> ships_by_part_type;
    for (std::shared_ptr<const Ship> ship : m_objects.FindObjects<Ship>()) {
        if (m_destroyed_object_ids.find(ship->ID()) != m_destroyed_object_ids.end())
            continue;

        const ShipDesign* ship_design = ship->Design();
        if (!ship_design)
            continue;
        const HullType* hull_type = ship_design->GetHull();
        if (!hull_type) {
            ErrorLogger() << "GetEffectsAndTargets couldn't get HullType";
            continue;
        }

        ships_by_hull_type[hull_type->Name()].push_back(ship);

        for (const std::string& part : ship_design->Parts()) {
            if (part.empty())
                continue;
            const PartType* part_type = GetPartType(part);
            if (!part_type) {
                ErrorLogger() << "GetEffectsAndTargets couldn't get PartType";
                continue;
            }

            ships_by_part_type[part].push_back(ship);
        }
    }

    // enforce hull types effects order
    for (const std::map<std::string, HullType*>::value_type& entry : GetHullTypeManager()) {
        const std::string& hull_type_name = entry.first;
        const HullType*    hull_type      = entry.second;
        std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>>::iterator ships_by_hull_type_it = ships_by_hull_type.find(hull_type_name);

        if (ships_by_hull_type_it == ships_by_hull_type.end())
            continue;

        for (std::shared_ptr<Effect::EffectsGroup> effects_group : hull_type->Effects()) {
            targets_causes_reorder_buffer.push_back(Effect::TargetsCauses());
            run_queue.AddWork(new StoreTargetsAndCausesOfEffectsGroupsWorkItem(
                effects_group, ships_by_hull_type_it->second, ECT_SHIP_HULL, hull_type_name,
                all_potential_targets, targets_causes_reorder_buffer.back(),
                cached_source_condition_matches,
                invariant_condition_matches,
                global_mutex));
        }
    }
    // enforce part types effects order
    for (const std::map<std::string, PartType*>::value_type& entry : GetPartTypeManager()) {
        const std::string& part_type_name = entry.first;
        const PartType*    part_type      = entry.second;
        std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>>::iterator ships_by_part_type_it = ships_by_part_type.find(part_type_name);

        if (ships_by_part_type_it == ships_by_part_type.end())
            continue;

        for (std::shared_ptr<Effect::EffectsGroup> effects_group : part_type->Effects()) {
            targets_causes_reorder_buffer.push_back(Effect::TargetsCauses());
            run_queue.AddWork(new StoreTargetsAndCausesOfEffectsGroupsWorkItem(
                effects_group, ships_by_part_type_it->second, ECT_SHIP_PART, part_type_name,
                all_potential_targets, targets_causes_reorder_buffer.back(),
                cached_source_condition_matches,
                invariant_condition_matches,
                global_mutex));
        }
    }
    double ships_time = type_timer.elapsed();

    // 6) EffectsGroups from Fields
    if (GetOptionsDB().Get<bool>("verbose-logging"))
        DebugLogger() << "Universe::GetEffectsAndTargets for FIELDS";
    type_timer.restart();
    // determine fields of each type in a single pass
    std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>> fields_by_type;
    for (std::shared_ptr<const Field> field : m_objects.FindObjects<Field>()) {
        if (m_destroyed_object_ids.find(field->ID()) != m_destroyed_object_ids.end())
            continue;

        const std::string& field_type_name = field->FieldTypeName();
        const FieldType* field_type = GetFieldType(field_type_name);
        if (!field_type) {
            ErrorLogger() << "GetEffectsAndTargets couldn't get FieldType " << field->FieldTypeName();
            continue;
        }

        fields_by_type[field_type_name].push_back(field);
    }

    // enforce field types effects order
    for (const std::map<std::string, FieldType*>::value_type& entry : GetFieldTypeManager()) {
        const std::string& field_type_name = entry.first;
        const FieldType*   field_type      = entry.second;
        std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>>::iterator fields_by_type_it = fields_by_type.find(field_type_name);

        if (fields_by_type_it == fields_by_type.end())
            continue;

        for (std::shared_ptr<Effect::EffectsGroup> effects_group : field_type->Effects()) {
            targets_causes_reorder_buffer.push_back(Effect::TargetsCauses());
            run_queue.AddWork(new StoreTargetsAndCausesOfEffectsGroupsWorkItem(
                effects_group, fields_by_type_it->second, ECT_FIELD, field_type_name,
                all_potential_targets, targets_causes_reorder_buffer.back(),
                cached_source_condition_matches,
                invariant_condition_matches,
                global_mutex));
        }
    }
    double fields_time = type_timer.elapsed();

    run_queue.Wait(global_lock);
    double eval_time = eval_timer.elapsed();

    eval_timer.restart();
    // add results to targets_causes in issue order
    // FIXME: each job is an effectsgroup, and we need that separation for
    // execution anyway, so maintain it here instead of merging.
    for (const Effect::TargetsCauses& job_results : targets_causes_reorder_buffer) {
        for (const std::pair<Effect::SourcedEffectsGroup, Effect::TargetsAndCause>& result : job_results) {
            targets_causes.push_back(result);
        }
    }
    double reorder_time = eval_timer.elapsed();
    DebugLogger() << "Issue times: planet species: " << planet_species_time*1000
                  << " ship species: " << ship_species_time*1000
                  << " specials: " << special_time*1000
                  << " techs: " << tech_time*1000
                  << " buildings: " << building_time*1000
                  << " hulls/parts: " << ships_time*1000
                  << " fields: " << fields_time*1000;
    DebugLogger() << "Evaluation time: " << eval_time*1000
                  << " reorder time: " << reorder_time*1000;
}

void Universe::ExecuteEffects(const Effect::TargetsCauses& targets_causes,
                              bool update_effect_accounting,
                              bool only_meter_effects/* = false*/,
                              bool only_appearance_effects/* = false*/,
                              bool include_empire_meter_effects/* = false*/,
                              bool only_generate_sitrep_effects/* = false*/)
{
    ScopedTimer timer("Universe::ExecuteEffects", true);

    m_marked_destroyed.clear();
    std::map< std::string, std::set<int> > executed_nonstacking_effects;
    bool log_verbose = GetOptionsDB().Get<bool>("verbose-logging");

    // grouping targets causes by effects group
    // sorting by effects group has already been done in GetEffectsAndTargets()
    // FIXME: GetEffectsAndTargets already produces this separation, exploit that
    std::map<int, std::vector<std::pair<Effect::EffectsGroup*, Effect::TargetsCauses> > > dispatched_targets_causes;
    {
        const Effect::EffectsGroup* last_effects_group   = nullptr;
        Effect::TargetsCauses*      group_targets_causes = nullptr;

        for (const std::pair<Effect::SourcedEffectsGroup, Effect::TargetsAndCause>& targets_cause : targets_causes) {
            const Effect::SourcedEffectsGroup& sourced_effects_group = targets_cause.first;
            Effect::EffectsGroup* effects_group = sourced_effects_group.effects_group.get();

            if (effects_group != last_effects_group) {
                last_effects_group = effects_group;
                dispatched_targets_causes[effects_group->Priority()].push_back(std::make_pair(effects_group, Effect::TargetsCauses()));
                group_targets_causes = &dispatched_targets_causes[effects_group->Priority()].back().second;
            }
            group_targets_causes->push_back(targets_cause);
        }
    }

    // execute each effects group one by one
    for (std::map<int, std::vector<std::pair<Effect::EffectsGroup*, Effect::TargetsCauses>>>::value_type& priority_group : dispatched_targets_causes) {
        for (std::vector<std::pair<Effect::EffectsGroup*, Effect::TargetsCauses>>::value_type& effect_group_entry : priority_group.second) {
            Effect::EffectsGroup* effects_group = effect_group_entry.first;

            if (only_meter_effects && !effects_group->HasMeterEffects())
                continue;
            if (only_appearance_effects && !effects_group->HasAppearanceEffects())
                continue;
            if (only_generate_sitrep_effects && !effects_group->HasSitrepEffects())
                continue;

            Effect::TargetsCauses&  group_targets_causes = effect_group_entry.second;
            std::string             stacking_group       = effects_group->StackingGroup();
            ScopedTimer update_timer(
                "Universe::ExecuteEffects effgrp (" + effects_group->AccountingLabel() + ") from "
                + std::to_string(group_targets_causes.size()) + " sources"
            );

            // if other EffectsGroups or sources with the same stacking group have affected some of the
            // targets in the scope of the current EffectsGroup, skip them
            // and add the remaining objects affected by it to executed_nonstacking_effects
            if (!stacking_group.empty()) {
                std::set<int>& non_stacking_targets = executed_nonstacking_effects[stacking_group];

                for (Effect::TargetsCauses::iterator targets_it = group_targets_causes.begin();
                     targets_it != group_targets_causes.end();)
                {
                    Effect::TargetsAndCause&           targets_and_cause     = targets_it->second;
                    Effect::TargetSet&                 targets               = targets_and_cause.target_set;

                    // this is a set difference/union algorithm: 
                    // targets              -= non_stacking_targets
                    // non_stacking_targets += targets
                    for (Effect::TargetSet::iterator object_it = targets.begin();
                         object_it != targets.end(); )
                    {
                        int object_id                    = (*object_it)->ID();
                        std::set<int>::const_iterator it = non_stacking_targets.find(object_id);

                        if (it != non_stacking_targets.end()) {
                            *object_it = targets.back();
                            targets.pop_back();
                        } else {
                            non_stacking_targets.insert(object_id);
                            ++object_it;
                        }
                    }

                    if (targets.empty()) {
                        *targets_it = group_targets_causes.back();
                        group_targets_causes.pop_back();
                    } else {
                        ++targets_it;
                    }
                }
            }

            if (group_targets_causes.empty())
                continue;

            if (log_verbose)
                DebugLogger() << "\n\n * * * * * * * * * * * (new effects group log entry)";

            // execute Effects in the EffectsGroup
            effects_group->Execute(group_targets_causes,
                                   update_effect_accounting ? &m_effect_accounting_map : nullptr,
                                   only_meter_effects,
                                   only_appearance_effects,
                                   include_empire_meter_effects,
                                   only_generate_sitrep_effects);
        }
    }

    // actually do destroy effect action.  Executing the effect just marks
    // objects to be destroyed, but doesn't actually do so in order to ensure
    // no interaction in order of effects and source or target objects being
    // destroyed / deleted between determining target sets and executing effects.
    // but, do now collect info about source objects for destruction, to sure
    // their info is available even if they are destroyed by the upcoming effect
    // destruction

    for (std::map<int, std::set<int>>::value_type& entry : m_marked_destroyed) {
        int obj_id = entry.first;
        std::shared_ptr<UniverseObject> obj = GetUniverseObject(obj_id);
        if (!obj)
            continue;

        // recording of what species/empire destroyed what other stuff in
        // empire statistics for this destroyed object and any contained objects
        for (int destructor : entry.second) {
            CountDestructionInStats(obj_id, destructor);
        }

        for (int contained_obj_id : obj->ContainedObjectIDs()) {
            for (int destructor : entry.second) {
                CountDestructionInStats(contained_obj_id, destructor);
            }
        }
        // not worried about fleets being deleted because all their ships were
        // destroyed...  as of this writing there are no stats tracking
        // destruction of fleets.

        // do actual recursive destruction.
        RecursiveDestroy(obj_id);
    }
}

namespace {
    static const std::string EMPTY_STRING;

    const std::string& GetSpeciesFromObject(std::shared_ptr<const UniverseObject> obj) {
        std::shared_ptr<const Fleet> obj_fleet;
        std::shared_ptr<const Ship> obj_ship;
        std::shared_ptr<const Building> obj_building;

        switch (obj->ObjectType()) {
        case OBJ_PLANET: {
            std::shared_ptr<const Planet> obj_planet = std::static_pointer_cast<const Planet>(obj);
            return obj_planet->SpeciesName();
            break;
        }
        case OBJ_SHIP: {
            std::shared_ptr<const Ship> obj_ship = std::static_pointer_cast<const Ship>(obj);
            return obj_ship->SpeciesName();
            break;
        }
        default:
            return EMPTY_STRING;
        }
    }

    int GetDesignIDFromObject(std::shared_ptr<const UniverseObject> obj) {
        if (obj->ObjectType() != OBJ_SHIP)
            return ShipDesign::INVALID_DESIGN_ID;
        std::shared_ptr<const Ship> shp = std::static_pointer_cast<const Ship>(obj);
        return shp->DesignID();
    }
}

void Universe::CountDestructionInStats(int object_id, int source_object_id) {
    std::shared_ptr<const UniverseObject> obj = GetUniverseObject(object_id);
    if (!obj)
        return;
    std::shared_ptr<const UniverseObject> source = GetUniverseObject(source_object_id);
    if (!source)
        return;

    const std::string& species_for_obj = GetSpeciesFromObject(obj);

    int empire_for_obj_id = obj->Owner();
    int empire_for_source_id = source->Owner();

    int design_for_obj_id = GetDesignIDFromObject(obj);

    if (Empire* source_empire = GetEmpire(empire_for_source_id)) {
        source_empire->EmpireShipsDestroyed()[empire_for_obj_id]++;

        if (design_for_obj_id != ShipDesign::INVALID_DESIGN_ID)
            source_empire->ShipDesignsDestroyed()[design_for_obj_id]++;

        if (species_for_obj.empty())
            source_empire->SpeciesShipsDestroyed()[species_for_obj]++;
    }

    if (Empire* obj_empire = GetEmpire(empire_for_obj_id)) {
        if (!species_for_obj.empty())
            obj_empire->SpeciesShipsLost()[species_for_obj]++;

        if (design_for_obj_id != ShipDesign::INVALID_DESIGN_ID)
            obj_empire->ShipDesignsLost()[design_for_obj_id]++;
    }
}

void Universe::SetEffectDerivedVisibility(int empire_id, int object_id, Visibility vis) {
    if (empire_id == ALL_EMPIRES)
        return;
    if (object_id <= INVALID_OBJECT_ID)
        return;
    if (vis == INVALID_VISIBILITY)
        return;
    m_effect_specified_empire_object_visibilities[empire_id][object_id] = vis;
}

void Universe::ApplyEffectDerivedVisibilities() {
    // for each empire with a visibility map
    for (std::map<int, ObjectVisibilityMap>::value_type& empire_entry : m_effect_specified_empire_object_visibilities) {
        if (empire_entry.first == ALL_EMPIRES)
            continue;   // can't set a non-empire's visibility
        for (const ObjectVisibilityMap::value_type& object_entry : empire_entry.second) {
            if (object_entry.first <= INVALID_OBJECT_ID)
                continue;
            m_empire_object_visibility[empire_entry.first][object_entry.first] = object_entry.second;
        }
    }
}

void Universe::ForgetKnownObject(int empire_id, int object_id) {
    // Note: Client calls this with empire_id == ALL_EMPIRES to
    // immediately forget information without waiting for the turn update.
    ObjectMap& objects(EmpireKnownObjects(empire_id));

    if (objects.Empty())
        return;

    std::shared_ptr<UniverseObject> obj = objects.Object(object_id);
    if (!obj) {
        ErrorLogger() << "ForgetKnownObject empire: " << empire_id
                      << " bad object id: " << object_id;
        return;
    }

    if (empire_id != ALL_EMPIRES && obj->OwnedBy(empire_id)) {
        ErrorLogger() << "ForgetKnownObject empire: " << empire_id
                      << " object: " << object_id
                      << ". Trying to forget visibility of own object.";
        return;
    }

    for (int child_id : obj->VisibleContainedObjectIDs(empire_id)) {
        if (std::shared_ptr<UniverseObject> child = objects.Object(child_id))
            ForgetKnownObject(empire_id, child->ID());
    }

    if (int container_id = obj->ContainerObjectID() != INVALID_OBJECT_ID) {
        if (std::shared_ptr<UniverseObject> container = objects.Object(container_id)) {
            if (std::shared_ptr<System> system = std::dynamic_pointer_cast<System>(container))
                system->Remove(object_id);
            else if (std::shared_ptr<Planet> planet = std::dynamic_pointer_cast<Planet>(container))
                planet->RemoveBuilding(object_id);
            else if (std::shared_ptr<Fleet> fleet = std::dynamic_pointer_cast<Fleet>(container))
                fleet->RemoveShip(object_id);
        }
    }

    objects.Remove(object_id);
}

void Universe::SetEmpireObjectVisibility(int empire_id, int object_id, Visibility vis) {
    if (empire_id == ALL_EMPIRES || object_id == INVALID_OBJECT_ID)
        return;

    // get visibility map for empire and find object in it
    Universe::ObjectVisibilityMap& vis_map = m_empire_object_visibility[empire_id];
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
        if (std::shared_ptr<const Ship> ship = GetShip(object_id)) {
            int design_id = ship->DesignID();
            if (design_id == ShipDesign::INVALID_DESIGN_ID) {
                ErrorLogger() << "SetEmpireObjectVisibility got invalid design id for ship with id " << object_id;
            } else {
                m_empire_known_ship_design_ids[empire_id].insert(design_id);
            }
        }
    }
}

void Universe::SetEmpireSpecialVisibility(int empire_id, int object_id,
                                          const std::string& special_name,
                                          bool visible/* = true*/)
{
    if (empire_id == ALL_EMPIRES || special_name.empty() || object_id == INVALID_OBJECT_ID)
        return;
    //std::shared_ptr<const UniverseObject> obj = GetUniverseObject(object_id);
    //if (!obj)
    //    return;
    //if (!obj->HasSpecial(special_name))
    //    return;
    if (visible)
        m_empire_object_visible_specials[empire_id][object_id].insert(special_name);
    else
        m_empire_object_visible_specials[empire_id][object_id].erase(special_name);
}

namespace {
    /** for each empire: for each position where the empire has detector objects,
      * what is the empire's detection range at that location?  (this is the
      * largest of the detection ranges of objects the empire has at that spot) */
    std::map<int, std::map<std::pair<double, double>, float> > GetEmpiresPositionDetectionRanges() {
        std::map<int, std::map<std::pair<double, double>, float> > retval;

        for (std::shared_ptr<const UniverseObject> obj : Objects()) {
            // skip unowned objects, which can't provide detection to any empire
            if (obj->Unowned())
                continue;

            // skip objects with no detection range
            const Meter* detection_meter = obj->GetMeter(METER_DETECTION);
            if (!detection_meter)
                continue;
            float object_detection_range = detection_meter->Current();
            if (object_detection_range <= 0.0f)
                continue;

            // don't allow moving ships / fleets to give detection
            std::shared_ptr<const Fleet> fleet;
            if (obj->ObjectType() == OBJ_FLEET) {
                fleet = std::dynamic_pointer_cast<const Fleet>(obj);
            } else if (obj->ObjectType() == OBJ_SHIP) {
                std::shared_ptr<const Ship> ship = std::dynamic_pointer_cast<const Ship>(obj);
                if (ship)
                    fleet = Objects().Object<Fleet>(ship->FleetID());
            }
            if (fleet) {
                int cur_id = fleet->SystemID();
                if (cur_id == INVALID_OBJECT_ID) // fleets do not grant detection when in a starlane
                    continue;
            }

            // record object's detection range for owner
            int object_owner_empire_id = obj->Owner();
            std::pair<double, double> object_pos(obj->X(), obj->Y());
            // store range in output map (if new for location or larger than any
            // previously-found range at this location)
            std::map<std::pair<double, double>, float>& retval_empire_pos_range = retval[object_owner_empire_id];
            std::map<std::pair<double, double>, float>::iterator retval_pos_it = retval_empire_pos_range.find(object_pos);
            if (retval_pos_it == retval_empire_pos_range.end())
                retval_empire_pos_range[object_pos] = object_detection_range;
            else
                retval_pos_it->second = std::max(retval_pos_it->second, object_detection_range);
        }
        return retval;
    }

    std::map<int, float> GetEmpiresDetectionStrengths(int empire_id = ALL_EMPIRES) {
        std::map<int, float> retval;
        if (empire_id == ALL_EMPIRES) {
            for (const std::map<int, Empire*>::value_type& empire_entry : Empires()) {
                const Meter* meter = empire_entry.second->GetMeter("METER_DETECTION_STRENGTH");
                float strength = meter ? meter->Current() : 0.0f;
                retval[empire_entry.first] = strength;
            }
        } else {
            if (const Empire* empire = GetEmpire(empire_id))
                if (const Meter* meter = empire->GetMeter("METER_DETECTION_STRENGTH"))
                    retval[empire_id] = meter->Current();
        }
        return retval;
    }

    /** for each empire: for each position, what objects have low enough stealth
      * that the empire could detect them if an detector owned by the empire is in
      * range? */
    std::map<int, std::map<std::pair<double, double>, std::vector<int> > >
        GetEmpiresPositionsPotentiallyDetectableObjects(const ObjectMap& objects, int empire_id = ALL_EMPIRES)
    {
        std::map<int, std::map<std::pair<double, double>, std::vector<int> > > retval;

        std::map<int, float> empire_detection_strengths = GetEmpiresDetectionStrengths(empire_id);

        // filter objects as detectors for this empire or detectable objects
        for (ObjectMap::const_iterator<> object_it = objects.const_begin();
             object_it != objects.const_end(); ++object_it)
        {
            std::shared_ptr<const UniverseObject> obj = *object_it;
            int object_id = object_it->ID();
            const Meter* stealth_meter = obj->GetMeter(METER_STEALTH);
            if (!stealth_meter)
                continue;
            float object_stealth = stealth_meter->Current();
            std::pair<double, double> object_pos(obj->X(), obj->Y());

            // for each empire being checked for, check if each object could be
            // detected by the empire if the empire has a detector in range.
            // being detectable by an empire requires the object to have
            // low enough stealth (0 or below the empire's detection strength)
            for (const std::map<int, float>::value_type& empire_entry : empire_detection_strengths) {
                int empire_id = empire_entry.first;
                if (object_stealth <= empire_entry.second || object_stealth == 0.0f || obj->OwnedBy(empire_id))
                    retval[empire_id][object_pos].push_back(object_id);
            }
        }
        return retval;
    }

    /** filters set of objects at locations by which of those locations are
      * within range of a set of detectors and ranges */
    std::vector<int> FilterObjectPositionsByDetectorPositionsAndRanges(
        const std::map<std::pair<double, double>, std::vector<int> >& object_positions,
        const std::map<std::pair<double, double>, float>& detector_position_ranges)
    {
        std::vector<int> retval;
        // check each detector position and range against each object position
        for (const std::map<std::pair<double, double>, std::vector<int>>::value_type& object_position_entry : object_positions) {
            const std::pair<double, double>& object_pos = object_position_entry.first;
            const std::vector<int>& objects = object_position_entry.second;
            // search through detector positions until one is found in range
            for (const std::map<std::pair<double, double>, float>::value_type& detector_position_entry : detector_position_ranges) {
                // check range for this detector location for this detectables location
                float detector_range2 = detector_position_entry.second * detector_position_entry.second;
                const std::pair<double, double>& detector_pos = detector_position_entry.first;
                double x_dist = detector_pos.first - object_pos.first;
                double y_dist = detector_pos.second - object_pos.second;
                double dist2 = x_dist*x_dist + y_dist*y_dist;
                if (dist2 > detector_range2)
                    continue;   // object out of range
                // add objects at position to return value
                std::copy(objects.begin(), objects.end(), std::back_inserter(retval));
                break;  // done searching for a detector position in range
            }
        }
        return retval;
    }

    /** removes ids of objects that the indicated empire knows have been
      * destroyed */
    void FilterObjectIDsByKnownDestruction(std::vector<int>& object_ids, int empire_id,
                                           const std::map<int, std::set<int> >& empire_known_destroyed_object_ids)
    {
        if (empire_id == ALL_EMPIRES)
            return;
        for (std::vector<int>::iterator it = object_ids.begin(); it != object_ids.end();) {
            int object_id = *it;
            std::map<int, std::set<int> >::const_iterator obj_it =
                empire_known_destroyed_object_ids.find(object_id);
            if (obj_it == empire_known_destroyed_object_ids.end()) {
                ++it;
                continue;
            }
            const std::set<int>& empires_that_know = obj_it->second;
            if (empires_that_know.find(empire_id) == empires_that_know.end()) {
                ++it;
                continue;
            }
            // remove object from vector...
            *it = object_ids.back();
            object_ids.pop_back();
        }
    }

    /** sets visibility of field objects for empires based on input locations
      * and stealth of fields in supplied ObjectMap and input empire detection
      * ranges at locations. the rules for detection of fields are more
      * permissive than other object types, so a special function for them is
      * needed in addition to SetEmpireObjectVisibilitiesFromRanges(...) */
    void SetEmpireFieldVisibilitiesFromRanges(
        const std::map<int, std::map<std::pair<double, double>, float> >&
            empire_location_detection_ranges,
        const ObjectMap& objects)
    {
        Universe& universe = GetUniverse();

        for (const std::map<int, std::map<std::pair<double, double>, float>>::value_type& detecting_empire_entry : empire_location_detection_ranges)
        {
            int detecting_empire_id = detecting_empire_entry.first;
            double detection_strength = 0.0;
            const Empire* empire = GetEmpire(detecting_empire_id);
            if (!empire)
                continue;
            const Meter* meter = empire->GetMeter("METER_DETECTION_STRENGTH");
            if (!meter)
                continue;
            detection_strength = meter->Current();

            // get empire's locations of detection ranges
            const std::map<std::pair<double, double>, float>& detector_position_ranges =
                detecting_empire_entry.second;

            // for each field, try to find a detector position in range for this empire
            for (std::shared_ptr<const Field> field : objects.FindObjects<Field>()) {
                if (field->GetMeter(METER_STEALTH)->Current() > detection_strength)
                    continue;
                double field_size = field->GetMeter(METER_SIZE)->Current();
                const std::pair<double, double> object_pos(field->X(), field->Y());

                // search through detector positions until one is found in range
                for (const std::map<std::pair<double, double>, float>::value_type& detector_position_entry : detector_position_ranges) {
                    // check range for this detector location, for field of this
                    // size, against distance between field and detector
                    float detector_range = detector_position_entry.second;
                    const std::pair<double, double>& detector_pos = detector_position_entry.first;
                    double x_dist = detector_pos.first - object_pos.first;
                    double y_dist = detector_pos.second - object_pos.second;
                    double dist = std::sqrt(x_dist*x_dist + y_dist*y_dist);
                    double effective_dist = dist - field_size;
                    if (effective_dist > detector_range)
                        continue;   // object out of range

                    universe.SetEmpireObjectVisibility(detecting_empire_id, field->ID(),
                                                       VIS_PARTIAL_VISIBILITY);
                }
            }
        }
    }

    /** sets visibility of objects for empires based on input locations of
      * potentially detectable objects (if in range) and and input empire
      * detection ranges at locations. */
    void SetEmpireObjectVisibilitiesFromRanges(
        const std::map<int, std::map<std::pair<double, double>, float> >&
            empire_location_detection_ranges,
        const std::map<int, std::map<std::pair<double, double>, std::vector<int> > >&
            empire_location_potentially_detectable_objects)
    {
        Universe& universe = GetUniverse();

        for (const std::map<int, std::map<std::pair<double, double>, float>>::value_type& detecting_empire_entry : empire_location_detection_ranges)
        {
            int detecting_empire_id = detecting_empire_entry.first;
            // get empire's locations of detection ability
            const std::map<std::pair<double, double>, float>& detector_position_ranges =
                detecting_empire_entry.second;
            // for this empire, get objects it could potentially detect
            const std::map<int, std::map<std::pair<double, double>, std::vector<int> > >::const_iterator
                empire_detectable_objects_it = empire_location_potentially_detectable_objects.find(detecting_empire_id);
            if (empire_detectable_objects_it == empire_location_potentially_detectable_objects.end())
                continue;   // empire can't detect anything!
            const std::map<std::pair<double, double>, std::vector<int> >& detectable_position_objects =
                empire_detectable_objects_it->second;
            if (detectable_position_objects.empty())
                continue;

            // filter potentially detectable objects by which are within range
            // of a detector
            std::vector<int> in_range_detectable_objects =
                FilterObjectPositionsByDetectorPositionsAndRanges(detectable_position_objects,
                                                                  detector_position_ranges);
            if (in_range_detectable_objects.empty())
                continue;

            // set all in-range detectable objects as partially visible (unless
            // any are already full vis, in which case do nothing)
            for (int detected_object_id : in_range_detectable_objects) {
                universe.SetEmpireObjectVisibility(detecting_empire_id, detected_object_id,
                                                   VIS_PARTIAL_VISIBILITY);
            }
        }
    }

    /** sets visibility of objects that empires own for those objects */
    void SetEmpireOwnedObjectVisibilities() {
        Universe& universe = GetUniverse();
        for (std::shared_ptr<const UniverseObject> obj : Objects()) {
            if (obj->Unowned())
                continue;
            universe.SetEmpireObjectVisibility(obj->Owner(), obj->ID(), VIS_FULL_VISIBILITY);
        }
    }

    /** sets all objects visible to all empires */
    void SetAllObjectsVisibleToAllEmpires() {
        Universe& universe = GetUniverse();
        // set every object visible to all empires
        for (ObjectMap::const_iterator<> obj_it = Objects().const_begin();
             obj_it != Objects().const_end(); ++obj_it)
        {
            for (std::map<int, Empire*>::value_type& empire_entry : Empires()) {
                // objects
                universe.SetEmpireObjectVisibility(empire_entry.first, obj_it->ID(), VIS_FULL_VISIBILITY);
                // specials on objects
                for (const std::map<std::string, std::pair<int, float>>::value_type& special_entry : obj_it->Specials()) {
                    universe.SetEmpireSpecialVisibility(empire_entry.first, obj_it->ID(), special_entry.first);
                }
            }
        }
    }

    /** sets planets in system where an empire owns an object to be basically
      * visible, and those systems to be partially visible */
    void SetSameSystemPlanetsVisible(const ObjectMap& objects) {
        Universe& universe = GetUniverse();
        // map from empire ID to ID of systems where those empires own at least one object
        std::map<int, std::set<int> > empires_systems_with_owned_objects;
        // get systems where empires have owned objects
        for (ObjectMap::const_iterator<> it = objects.const_begin(); it != objects.const_end(); ++it) {
            std::shared_ptr<const UniverseObject> obj = *it;
            if (obj->Unowned() || obj->SystemID() == INVALID_OBJECT_ID)
                continue;
            empires_systems_with_owned_objects[obj->Owner()].insert(obj->SystemID());
        }

        // set system visibility
        for (std::map<int, std::set<int>>::value_type& empire_entry : empires_systems_with_owned_objects) {
            int empire_id = empire_entry.first;

            for (int system_id : empire_entry.second) {
                universe.SetEmpireObjectVisibility(empire_id, system_id, VIS_PARTIAL_VISIBILITY);
            }
        }

        // get planets, check their locations...
        std::vector<std::shared_ptr<const Planet>> planets = objects.FindObjects<Planet>();
        for (std::shared_ptr<const Planet> planet : objects.FindObjects<Planet>()) {
            int system_id = planet->SystemID();
            if (system_id == INVALID_OBJECT_ID)
                continue;

            int planet_id = planet->ID();
            for (const std::map<int, std::set<int>>::value_type& empire_entry : empires_systems_with_owned_objects) {
                int empire_id = empire_entry.first;
                const std::set<int>& empire_systems = empire_entry.second;
                if (empire_systems.find(system_id) == empire_systems.end())
                    continue;
                // ensure planets are at least basicaly visible.  does not
                // overwrite higher visibility levels
                universe.SetEmpireObjectVisibility(empire_id, planet_id, VIS_BASIC_VISIBILITY);
            }
        }
    }

    void PropagateVisibilityToContainerObjects(const ObjectMap& objects,
                                               Universe::EmpireObjectVisibilityMap& empire_object_visibility)
    {
        // propagate visibility from contained to container objects
        for (ObjectMap::const_iterator<> container_object_it = objects.const_begin();
             container_object_it != objects.const_end(); ++container_object_it)
        {
            int container_obj_id = container_object_it->ID();

            // get container object
            std::shared_ptr<const UniverseObject> container_obj = *container_object_it;
            if (!container_obj)
                continue;   // shouldn't be necessary, but I like to be safe...

            // check if container object is a fleet, for special case later...
            bool container_fleet = container_obj->ObjectType() == OBJ_FLEET;

            //DebugLogger() << "Container object " << container_obj->Name() << " (" << container_obj->ID() << ")";

            // for each contained object within container
            for (int contained_obj_id : container_obj->ContainedObjectIDs()) {
                //DebugLogger() << " ... contained object (" << contained_obj_id << ")";

                // for each empire with a visibility map
                for (Universe::EmpireObjectVisibilityMap::value_type& empire_entry : empire_object_visibility) {
                    Universe::ObjectVisibilityMap& vis_map = empire_entry.second;

                    //DebugLogger() << " ... ... empire id " << empire_entry.first;

                    // find current empire's visibility entry for current container object
                    Universe::ObjectVisibilityMap::iterator container_vis_it = vis_map.find(container_obj_id);
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
                            // empire, don't need to propagate anything
                            continue;
                        }
                    }


                    // find contained object's entry in visibility map
                    Universe::ObjectVisibilityMap::iterator contained_vis_it = vis_map.find(contained_obj_id);
                    if (contained_vis_it != vis_map.end()) {
                        // get contained object's visibility for current empire
                        Visibility contained_obj_vis = contained_vis_it->second;

                        // no need to propagate if contained object isn't visible to current empire
                        if (contained_obj_vis <= VIS_NO_VISIBILITY)
                            continue;

                        //DebugLogger() << " ... ... contained object vis: " << contained_obj_vis;

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
                        if (container_fleet && contained_obj_vis >= VIS_PARTIAL_VISIBILITY &&
                            container_vis_it->second < VIS_PARTIAL_VISIBILITY)
                        { container_vis_it->second = VIS_PARTIAL_VISIBILITY; }
                    }
                }   // end for empire visibility entries
            }   // end for contained objects
        }   // end for container objects
    }

    void PropagateVisibilityToSystemsAlongStarlanes(const ObjectMap& objects,
                                                    Universe::EmpireObjectVisibilityMap& empire_object_visibility) {
        for (std::shared_ptr<const System> system : objects.FindObjects<System>()) {
            int system_id = system->ID();

            // for each empire with a visibility map
            for (Universe::EmpireObjectVisibilityMap::value_type& empire_entry : empire_object_visibility) {
                Universe::ObjectVisibilityMap& vis_map = empire_entry.second;

                // find current system's visibility
                Universe::ObjectVisibilityMap::iterator system_vis_it = vis_map.find(system_id);
                if (system_vis_it == vis_map.end())
                    continue;

                // skip systems that aren't at least partially visible; they can't propagate visibility along starlanes
                Visibility system_vis = system_vis_it->second;
                if (system_vis <= VIS_BASIC_VISIBILITY)
                    continue;

                // get all starlanes emanating from this system, and loop through them
                for (const std::map<int, bool>::value_type& lane : system->StarlanesWormholes()) {
                    bool is_wormhole = lane.second;
                    if (is_wormhole)
                        continue;

                    // find entry for system on other end of starlane in visibility
                    // map, and upgrade to basic visibility if not already at that
                    // leve, so that starlanes will be visible if either system it
                    // ends at is partially visible or better
                    int lane_end_sys_id = lane.first;
                    Universe::ObjectVisibilityMap::iterator lane_end_vis_it = vis_map.find(lane_end_sys_id);
                    if (lane_end_vis_it == vis_map.end())
                        vis_map[lane_end_sys_id] = VIS_BASIC_VISIBILITY;
                    else if (lane_end_vis_it->second < VIS_BASIC_VISIBILITY)
                        lane_end_vis_it->second = VIS_BASIC_VISIBILITY;
                }
            }
        }

    }

    void SetTravelledStarlaneEndpointsVisible(const ObjectMap& objects,
                                              Universe::EmpireObjectVisibilityMap& empire_object_visibility)
    {
        // ensure systems on either side of a starlane along which a fleet is
        // moving are at least basically visible, so that the starlane itself can /
        // will be visible
        std::vector<std::shared_ptr<const Fleet>> moving_fleets;
        for (std::shared_ptr<const UniverseObject> obj : objects.FindObjects(MovingFleetVisitor())) {
            if (obj->Unowned() || obj->SystemID() == INVALID_OBJECT_ID || obj->ObjectType() != OBJ_FLEET)
                continue;
            std::shared_ptr<const Fleet> fleet = std::dynamic_pointer_cast<const Fleet>(obj);
            if (!fleet)
                continue;

            int prev = fleet->PreviousSystemID();
            int next = fleet->NextSystemID();

            // ensure fleet's owner has at least basic visibility of the next
            // and previous systems on the fleet's path
            Universe::ObjectVisibilityMap& vis_map = empire_object_visibility[fleet->Owner()];

            Universe::ObjectVisibilityMap::iterator system_vis_it = vis_map.find(prev);
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

    void SetEmpireSpecialVisibilities(const ObjectMap& objects,
                                      Universe::EmpireObjectVisibilityMap& empire_object_visibility,
                                      Universe::EmpireObjectSpecialsMap& empire_object_visible_specials)
    {
        // after setting object visibility, similarly set visibility of objects'
        // specials for each empire
        for (std::map<int, Empire*>::value_type& empire_entry : Empires()) {
            int empire_id = empire_entry.first;
            Universe::ObjectVisibilityMap& obj_vis_map = empire_object_visibility[empire_id];
            Universe::ObjectSpecialsMap& obj_specials_map = empire_object_visible_specials[empire_id];

            const Empire* empire = empire_entry.second;
            const Meter* detection_meter = empire->GetMeter("METER_DETECTION_STRENGTH");
            if (!detection_meter)
                continue;
            float detection_strength = detection_meter->Current();

            // every object empire has visibility of might have specials
            for (Universe::ObjectVisibilityMap::value_type& obj_entry : obj_vis_map) {
                if (obj_entry.second <= VIS_NO_VISIBILITY)
                    continue;

                int object_id = obj_entry.first;
                std::shared_ptr<const UniverseObject> obj = objects.Object(object_id);
                if (!obj)
                    continue;

                if (obj->Specials().empty())
                    continue;

                std::set<std::string>& visible_specials = obj_specials_map[object_id];

                // check all object's specials.
                for (const std::map<std::string, std::pair<int, float>>::value_type& special_entry : obj->Specials()) {
                    const Special* special = GetSpecial(special_entry.first);
                    if (!special)
                        continue;

                    float stealth = 0.0f;
                    const ValueRef::ValueRefBase<double>* special_stealth = special->Stealth();
                    if (special_stealth)
                        stealth = special_stealth->Eval(ScriptingContext(obj));

                    // if special is 0 stealth, or has stealth less than empire's detection strength, mark as visible
                    if (stealth <= 0.0f || stealth <= detection_strength) {
                        visible_specials.insert(special_entry.first);
                        //DebugLogger() << "Special " << special_entry.first << " on " << obj->Name() << " is visible to empire " << empire_id;
                    }
                }
            }
        }
    }

    void ShareVisbilitiesBetweenAllies(Universe::EmpireObjectVisibilityMap& empire_object_visibility,
                                       Universe::EmpireObjectSpecialsMap& empire_object_visible_specials)
    {
        // make copy of input vis map, iterate over that, not the output as
        // iterating over the output while modifying it would result in 
        // second-order visibility sharing (but only through allies with lower
        // empire id)
        Universe::EmpireObjectVisibilityMap input_eov_copy = empire_object_visibility;
        Universe::EmpireObjectSpecialsMap input_eovs_copy = empire_object_visible_specials;

        for (std::map<int, Empire*>::value_type& empire_entry : Empires()) {
            int empire_id = empire_entry.first;
            // output maps for this empire
            Universe::ObjectVisibilityMap& obj_vis_map = empire_object_visibility[empire_id];
            Universe::ObjectSpecialsMap& obj_specials_map = empire_object_visible_specials[empire_id];

            for (auto allied_empire_id : Empires().GetEmpireIDsWithDiplomaticStatusWithEmpire(empire_id, DIPLO_ALLIED)) {
                if (empire_id == allied_empire_id) {
                    ErrorLogger() << "ShareVisbilitiesBetweenAllies : Empire apparent allied with itself!";
                    continue;
                }

                // inpu maps for this ally empire
                Universe::ObjectVisibilityMap& allied_obj_vis_map = input_eov_copy[allied_empire_id];
                Universe::ObjectSpecialsMap& allied_obj_specials_map = input_eovs_copy[allied_empire_id];

                // add allied visibilities to outer-loop empire visibilities
                // whenever the ally has better visibility of an object
                // (will do the reverse in another loop iteration)
                for (auto const& allied_obj_id_vis_pair : allied_obj_vis_map) {
                    int obj_id = allied_obj_id_vis_pair.first;
                    Visibility allied_vis = allied_obj_id_vis_pair.second;
                    std::map<int, Visibility>::iterator it = obj_vis_map.find(obj_id);
                    if (it == obj_vis_map.end() || it->second < allied_vis)
                        obj_vis_map[obj_id] = allied_vis;
                }
            }
        }
    }
}

void Universe::UpdateEmpireObjectVisibilities() {
    // ensure Universe knows empires have knowledge of designs the empire is specifically remembering
    for (std::map<int, Empire*>::value_type& empire_entry : Empires()) {
        int empire_id = empire_entry.first;
        const Empire* empire = empire_entry.second;
        for (int design_id : empire->ShipDesigns()) {
            m_empire_known_ship_design_ids[empire_id].insert(design_id);
        }
    }

    m_empire_object_visibility.clear();
    m_empire_object_visible_specials.clear();

    if (m_all_objects_visible) {
        SetAllObjectsVisibleToAllEmpires();
        return;
    }


    SetEmpireOwnedObjectVisibilities();

    std::map<int, std::map<std::pair<double, double>, float> >
        empire_position_detection_ranges = GetEmpiresPositionDetectionRanges();

    std::map<int, std::map<std::pair<double, double>, std::vector<int> > >
        empire_position_potentially_detectable_objects =
            GetEmpiresPositionsPotentiallyDetectableObjects(Objects());

    SetEmpireObjectVisibilitiesFromRanges(empire_position_detection_ranges,
                                          empire_position_potentially_detectable_objects);
    SetEmpireFieldVisibilitiesFromRanges(empire_position_detection_ranges, Objects());

    SetSameSystemPlanetsVisible(Objects());

    ApplyEffectDerivedVisibilities();

    PropagateVisibilityToContainerObjects(Objects(), m_empire_object_visibility);

    PropagateVisibilityToSystemsAlongStarlanes(Objects(), m_empire_object_visibility);

    SetTravelledStarlaneEndpointsVisible(Objects(), m_empire_object_visibility);

    SetEmpireSpecialVisibilities(Objects(), m_empire_object_visibility, m_empire_object_visible_specials);

    ShareVisbilitiesBetweenAllies(m_empire_object_visibility, m_empire_object_visible_specials);
}

void Universe::UpdateEmpireLatestKnownObjectsAndVisibilityTurns() {
    //DebugLogger() << "Universe::UpdateEmpireLatestKnownObjectsAndVisibilityTurns()";

    // assumes m_empire_object_visibility has been updated

    //  for each object in universe
    //      for each empire that can see object this turn
    //          update empire's information about object, based on visibility
    //          update empire's visbilility turn history

    int current_turn = CurrentTurn();
    if (current_turn == INVALID_GAME_TURN)
        return;

    // for each object in universe
    for (std::shared_ptr<const UniverseObject> full_object : m_objects) {
        int object_id = full_object->ID();
        if (!full_object) {
            ErrorLogger() << "UpdateEmpireLatestKnownObjectsAndVisibilityTurns found null object in m_objects with id " << object_id;
            continue;
        }

        // for each empire with a visibility map
        for (EmpireObjectVisibilityMap::value_type& empire_entry : m_empire_object_visibility) {
            // can empire see object?
            const ObjectVisibilityMap& vis_map = empire_entry.second;    // stores level of visibility empire has for each object it can detect this turn
            ObjectVisibilityMap::const_iterator vis_it = vis_map.find(object_id);
            if (vis_it == vis_map.end())
                continue;   // empire can't see current object, so move to next empire
            const Visibility vis = vis_it->second;
            if (vis <= VIS_NO_VISIBILITY)
                continue;   // empire can't see current object, so move to next empire

            // empire can see object.  need to update empire's latest known
            // information about object, and historical turns on which object
            // was seen at various visibility levels.

            int empire_id = empire_entry.first;

            ObjectMap&                  known_object_map = m_empire_latest_known_objects[empire_id];        // creates empty map if none yet present
            ObjectVisibilityTurnMap&    object_vis_turn_map = m_empire_object_visibility_turns[empire_id];  // creates empty map if none yet present
            VisibilityTurnMap&          vis_turn_map = object_vis_turn_map[object_id];                      // creates empty map if none yet present


            // update empire's latest known data about object, based on current visibility and historical visibility and knowledge of object

            // is there already last known version of an UniverseObject stored for this empire?
            if (std::shared_ptr<UniverseObject> known_obj = known_object_map.Object(object_id)) {
                known_obj->Copy(full_object, empire_id);                    // already a stored version of this object for this empire.  update it, limited by visibility this empire has for this object this turn
            } else {
                if (UniverseObject* new_obj = full_object->Clone(empire_id))    // no previously-recorded version of this object for this empire.  create a new one, copying only the information limtied by visibility, leaving the rest as default values
                    known_object_map.Insert(new_obj);
            }

            //DebugLogger() << "Empire " << empire_id << " can see object " << object_id << " with vis level " << vis;

            // update empire's visibility turn history for current vis, and lesser vis levels
            if (vis >= VIS_BASIC_VISIBILITY) {
                vis_turn_map[VIS_BASIC_VISIBILITY] = current_turn;
                if (vis >= VIS_PARTIAL_VISIBILITY) {
                    vis_turn_map[VIS_PARTIAL_VISIBILITY] = current_turn;
                    if (vis >= VIS_FULL_VISIBILITY) {
                        vis_turn_map[VIS_FULL_VISIBILITY] = current_turn;
                    }
                }
                //DebugLogger() << " ... Setting empire " << empire_id << " object " << full_object->Name() << " (" << object_id << ") vis " << vis << " (and higher) turn to " << current_turn;
            } else {
                ErrorLogger() << "Universe::UpdateEmpireLatestKnownObjectsAndVisibilityTurns() found invalid visibility for object with id " << object_id << " by empire with id " << empire_id;
                continue;
            }
        }
    }
}

void Universe::UpdateEmpireStaleObjectKnowledge() {
    // if any objects in the latest known objects for an empire are not
    // currently visible, but that empire has detectors in range of the objects'
    // latest known location and the objects' latest known stealth is low enough to be
    // detectable by that empire, then the latest known state of the objects
    // (including stealth and position) appears to be stale / out of date.

    const std::map<int, std::map<std::pair<double, double>, float> >
        empire_location_detection_ranges = GetEmpiresPositionDetectionRanges();

    for (const EmpireObjectMap::value_type& empire_entry : m_empire_latest_known_objects) {
        int empire_id = empire_entry.first;
        const ObjectMap& latest_known_objects = empire_entry.second;
        const ObjectVisibilityMap& vis_map = m_empire_object_visibility[empire_id];
        std::set<int>& stale_set = m_empire_stale_knowledge_object_ids[empire_id];
        const std::set<int>& destroyed_set = m_empire_known_destroyed_object_ids[empire_id];

        // remove stale marking for any known destroyed or currently visible objects
        for (std::set<int>::iterator stale_it = stale_set.begin(); stale_it != stale_set.end();) {
            int object_id = *stale_it;
            if (vis_map.find(object_id) != vis_map.end() ||
                destroyed_set.find(object_id) != destroyed_set.end())
            {
                stale_set.erase(stale_it++);
            } else {
                ++stale_it;
            }
        }


        // get empire latest known objects that are potentially detectable
        std::map<int, std::map<std::pair<double, double>, std::vector<int> > >
            empires_latest_known_objects_that_should_be_detectable =
                GetEmpiresPositionsPotentiallyDetectableObjects(latest_known_objects, empire_id);
        std::map<std::pair<double, double>, std::vector<int> >&
            empire_latest_known_should_be_still_detectable_objects =
                empires_latest_known_objects_that_should_be_detectable[empire_id];


        // get empire detection ranges
        std::map<int, std::map<std::pair<double, double>, float> >::const_iterator
            empire_detectors_it = empire_location_detection_ranges.find(empire_id);
        if (empire_detectors_it == empire_location_detection_ranges.end())
            continue;
        const std::map<std::pair<double, double>, float>& empire_detector_positions_ranges =
            empire_detectors_it->second;


        // filter should-be-still-detectable objects by whether they are
        // in range of a detector
        std::vector<int> should_still_be_detectable_latest_known_objects =
            FilterObjectPositionsByDetectorPositionsAndRanges(
                empire_latest_known_should_be_still_detectable_objects,
                empire_detector_positions_ranges);


        // filter to exclude objects that are known to have been destroyed, as
        // their last state is not stale information
        FilterObjectIDsByKnownDestruction(should_still_be_detectable_latest_known_objects,
                                          empire_id, m_empire_known_destroyed_object_ids);


        // any objects that pass filters but aren't actually still visible
        // represent out-of-date info in empire's latest known objects.  these
        // entries need to be removed / flagged to indicate this
        for (int object_id : should_still_be_detectable_latest_known_objects) {
            ObjectVisibilityMap::const_iterator vis_it = vis_map.find(object_id);
            if (vis_it == vis_map.end() || vis_it->second < VIS_BASIC_VISIBILITY) {
                // object not visible even though the latest known info about it
                // for this empire suggests it should be.  info is stale.
                stale_set.insert(object_id);
            }
        }


        // fleets that are not visible and that contain no ships or only stale ships are stale
        for (ObjectMap::const_iterator<> obj_it = latest_known_objects.const_begin();
             obj_it != latest_known_objects.const_end(); ++obj_it)
        {
            if (obj_it->ObjectType() != OBJ_FLEET)
                continue;
            if (obj_it->GetVisibility(empire_id) >= VIS_BASIC_VISIBILITY)
                continue;
            std::shared_ptr<const Fleet> fleet = std::dynamic_pointer_cast<const Fleet>(*obj_it);
            if (!fleet)
                continue;
            int fleet_id = obj_it->ID();

            // destroyed? not stale
            if (destroyed_set.find(fleet_id) != destroyed_set.end()) {
                stale_set.insert(fleet_id);
                continue;
            }

            // no ships? -> stale
            if (fleet->Empty()) {
                stale_set.insert(fleet_id);
                continue;
            }

            bool fleet_stale = true;
            // check each ship. if any are visible or not visible but not stale,
            // fleet is not stale
            for (int ship_id : fleet->ShipIDs()) {
                std::shared_ptr<const Ship> ship = latest_known_objects.Object<Ship>(ship_id);

                // if ship doesn't think it's in this fleet, doesn't count.
                if (!ship || ship->FleetID() != fleet_id)
                    continue;

                // if ship is destroyed, doesn't count
                if (destroyed_set.find(ship_id) != destroyed_set.end())
                    continue;

                // is contained ship visible? If so, fleet is not stale.
                ObjectVisibilityMap::const_iterator vis_it = vis_map.find(ship_id);
                if (vis_it != vis_map.end() && vis_it->second > VIS_NO_VISIBILITY) {
                    fleet_stale = false;
                    break;
                }

                // is contained ship not visible and not stale? if so, fleet is not stale
                if (stale_set.find(ship_id) == stale_set.end()) {
                    fleet_stale = false;
                    break;
                }
            }
            if (fleet_stale)
                stale_set.insert(fleet_id);
        }

        //for (int stale_id : stale_set) {
        //    std::shared_ptr<const UniverseObject> obj = latest_known_objects.Object(stale_id);
        //    DebugLogger() << "Object " << stale_id << " : " << (obj ? obj->Name() : "(unknown)") << " is stale for empire " << empire_id ;
        //}
    }
}

void Universe::SetEmpireKnowledgeOfDestroyedObject(int object_id, int empire_id) {
    if (object_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "SetEmpireKnowledgeOfDestroyedObject called with INVALID_OBJECT_ID";
        return;
    }
    if (!GetEmpire(empire_id)) {
        ErrorLogger() << "SetEmpireKnowledgeOfDestroyedObject called for invalid empire id: " << empire_id;
        return;
    }
    m_empire_known_destroyed_object_ids[empire_id].insert(object_id);
}

void Universe::SetEmpireKnowledgeOfShipDesign(int ship_design_id, int empire_id) {
    if (ship_design_id == ShipDesign::INVALID_DESIGN_ID) {
        ErrorLogger() << "SetEmpireKnowledgeOfShipDesign called with INVALID_DESIGN_ID";
        return;
    }
    if (empire_id == ALL_EMPIRES)
        return;
    if (!GetEmpire(empire_id))
        ErrorLogger() << "SetEmpireKnowledgeOfShipDesign called for invalid empire id: " << empire_id;

    m_empire_known_ship_design_ids[empire_id].insert(ship_design_id);
}

void Universe::Destroy(int object_id, bool update_destroyed_object_knowers/* = true*/) {
    // remove object from any containing UniverseObject
    std::shared_ptr<UniverseObject> obj = m_objects.Object(object_id);
    if (!obj) {
        ErrorLogger() << "Universe::Destroy called for nonexistant object with id: " << object_id;
        return;
    }

    m_destroyed_object_ids.insert(object_id);

    if (update_destroyed_object_knowers) {
        // record empires that know this object has been destroyed
        for (std::map<int, Empire*>::value_type& empire_entry : Empires()) {
            int empire_id = empire_entry.first;
            if (obj->GetVisibility(empire_id) >= VIS_BASIC_VISIBILITY) {
                SetEmpireKnowledgeOfDestroyedObject(object_id, empire_id);
                // TODO: Update m_empire_latest_known_objects somehow?
            }
        }
    }

    // signal that an object has been deleted
    UniverseObjectDeleteSignal(obj);
    m_objects.Remove(object_id);
}

std::set<int> Universe::RecursiveDestroy(int object_id) {
    std::set<int> retval;

    std::shared_ptr<UniverseObject> obj = m_objects.Object(object_id);
    if (!obj) {
        DebugLogger() << "Universe::RecursiveDestroy asked to destroy nonexistant object with id " << object_id;
        return retval;
    }

    std::shared_ptr<System> system = GetSystem(obj->SystemID());

    if (std::shared_ptr<Ship> ship = std::dynamic_pointer_cast<Ship>(obj)) {
        // if a ship is being deleted, and it is the last ship in its fleet, then the empty fleet should also be deleted
        std::shared_ptr<Fleet> fleet = GetFleet(ship->FleetID());
        if (fleet) {
            fleet->RemoveShip(ship->ID());
            if (fleet->Empty()) {
                if (system)
                    system->Remove(fleet->ID());
                Destroy(fleet->ID());
                retval.insert(fleet->ID());
            }
        }
        if (system)
            system->Remove(object_id);
        Destroy(object_id);
        retval.insert(object_id);

    } else if (std::shared_ptr<Fleet> fleet = std::dynamic_pointer_cast<Fleet>(obj)) {
        for (int ship_id : fleet->ShipIDs()) {
            if (system)
                system->Remove(ship_id);
            Destroy(ship_id);
            retval.insert(ship_id);
        }
        if (system)
            system->Remove(object_id);
        Destroy(object_id);
        retval.insert(object_id);

    } else if (std::shared_ptr<Planet> planet = std::dynamic_pointer_cast<Planet>(obj)) {
        for (int building_id : planet->BuildingIDs()) {
            if (system)
                system->Remove(building_id);
            Destroy(building_id);
            retval.insert(building_id);
        }
        if (system)
            system->Remove(object_id);
        Destroy(object_id);
        retval.insert(object_id);

    } else if (std::shared_ptr<System> obj_system = std::dynamic_pointer_cast<System>(obj)) {
        // destroy all objects in system
        for (int system_id : obj_system->ObjectIDs()) {
            Destroy(system_id);
            retval.insert(system_id);
        }

        // remove any starlane connections to this system
        int this_sys_id = obj_system->ID();
        for (std::shared_ptr<System> sys : m_objects.FindObjects<System>()) {
            sys->RemoveStarlane(this_sys_id);
        }

        // remove fleets / ships moving along destroyed starlane
        for (std::shared_ptr<Fleet> fleet : m_objects.FindObjects<Fleet>()) {
            if (fleet->SystemID() == INVALID_OBJECT_ID && (
                fleet->NextSystemID() == this_sys_id ||
                fleet->PreviousSystemID() == this_sys_id))
            { RecursiveDestroy(fleet->ID()); }
        }

        // then destroy system itself
        Destroy(object_id);
        retval.insert(object_id);
        // don't need to bother with removing things from system, fleets, or
        // ships, since everything in system is being destroyed

    } else if (std::shared_ptr<Building> building = std::dynamic_pointer_cast<Building>(obj)) {
        std::shared_ptr<Planet> planet = GetPlanet(building->PlanetID());
        if (planet)
            planet->RemoveBuilding(object_id);
        if (system)
            system->Remove(object_id);
        Destroy(object_id);
        retval.insert(object_id);

    } else if (obj->ObjectType() == OBJ_FIELD) {
        if (system)
            system->Remove(object_id);
        Destroy(object_id);
        retval.insert(object_id);
    }
    // else ??? object is of some type unknown as of this writing.
    return retval;
}

bool Universe::Delete(int object_id) {
    DebugLogger() << "Universe::Delete with ID: " << object_id;
    // find object amongst existing objects and delete directly, without storing
    // any info about the previous object (as is done for destroying an object)
    std::shared_ptr<UniverseObject> obj = m_objects.Object(object_id);
    if (!obj) {
        ErrorLogger() << "Tried to delete a nonexistant object with id: " << object_id;
        return false;
    }

    // move object to invalid position, thereby removing it from anything that
    // contained it and propagating associated signals
    obj->MoveTo(UniverseObject::INVALID_POSITION, UniverseObject::INVALID_POSITION);
    // remove from existing objects set
    m_objects.Remove(object_id);

    // TODO: Should this also remove the object from the latest known objects
    // and known destroyed objects for each empire?

    return true;
}

void Universe::EffectDestroy(int object_id, int source_object_id) {
    if (m_marked_destroyed.find(object_id) != m_marked_destroyed.end())
        return;
    m_marked_destroyed[object_id].insert(source_object_id);
}

void Universe::InitializeSystemGraph(int for_empire_id) {
    std::vector<int> system_ids = ::EmpireKnownObjects(for_empire_id).FindObjectIDs<System>();
    std::vector<std::shared_ptr<const System> > systems;
    for (size_t system1_index = 0; system1_index < system_ids.size(); ++system1_index) {
        int system1_id = system_ids[system1_index];
        systems.push_back(GetEmpireKnownSystem(system1_id, for_empire_id));
    }

    m_pathfinder->InitializeSystemGraph(system_ids, for_empire_id);
}

//TODO Universe::UpdateEmpireVisibilityFilteredSystemGraphs is never
//used.  Decide if the functionality permanently belongs in Pathfinder
void Universe::UpdateEmpireVisibilityFilteredSystemGraphs(int empire_id) {
    m_pathfinder->UpdateEmpireVisibilityFilteredSystemGraphs(empire_id);
}

int& Universe::EncodingEmpire()
{ return m_encoding_empire; }

double Universe::UniverseWidth() const
{ return m_universe_width; }

const bool& Universe::UniverseObjectSignalsInhibited()
{ return m_inhibit_universe_object_signals; }

void Universe::InhibitUniverseObjectSignals(bool inhibit)
{ m_inhibit_universe_object_signals = inhibit; }

void Universe::UpdateStatRecords() {
    int current_turn = CurrentTurn();
    if (current_turn == INVALID_GAME_TURN)
        return;
    if (current_turn == 0)
        m_stat_records.clear();

    std::map<int, std::shared_ptr<const UniverseObject>> empire_sources;
    for (std::map<int, Empire*>::value_type& empire_entry : Empires()) {
        std::shared_ptr<const UniverseObject> source = empire_entry.second->Source();
        if (!source) {
            ErrorLogger() << "Universe::UpdateStatRecords() unable to find source for empire.  Skipping.";
            continue;
        }
        empire_sources[empire_entry.first] = source;
    }

    // process each stat
    for (const std::map<std::string, ValueRef::ValueRefBase<double>*>::value_type& stat_entry : EmpireStatistics::GetEmpireStats())
    {
        const std::string& stat_name = stat_entry.first;

        const ValueRef::ValueRefBase<double>* value_ref = stat_entry.second;
        if (!value_ref)
            continue;

        std::map<int, std::map<int, double> >& stat_records = m_stat_records[stat_name];

        // calculate stat for each empire, store in records for current turn
        for (std::map<int, std::shared_ptr<const UniverseObject>>::value_type entry : empire_sources) {
            int empire_id = entry.first;

            if (value_ref->SourceInvariant()) {
                stat_records[empire_id][current_turn] = value_ref->Eval();
            } else if (entry.second) {
                stat_records[empire_id][current_turn] = value_ref->Eval(ScriptingContext(entry.second));
            }
        }
    }
}

void Universe::GetShipDesignsToSerialize(ShipDesignMap& designs_to_serialize, int encoding_empire) const {
    if (encoding_empire == ALL_EMPIRES) {
        designs_to_serialize = m_ship_designs;
    } else {
        designs_to_serialize.clear();

        // add generic monster ship designs so they always appear in players' pedias
        for (const ShipDesignMap::value_type& ship_design_entry : m_ship_designs) {
            ShipDesign* design = ship_design_entry.second;
            if (design->IsMonster() && design->DesignedByEmpire() == ALL_EMPIRES)
                designs_to_serialize[design->ID()] = design;
        }

        // get empire's known ship designs
        std::map<int, std::set<int> >::const_iterator it = m_empire_known_ship_design_ids.find(encoding_empire);
        if (it == m_empire_known_ship_design_ids.end())
            return; // no known designs to serialize

        const std::set<int>& empire_designs = it->second;

        // add all ship designs of ships this empire knows about
        for (int design_id : empire_designs) {
            ShipDesignMap::const_iterator universe_design_it = m_ship_designs.find(design_id);
            if (universe_design_it != m_ship_designs.end())
                designs_to_serialize[design_id] = universe_design_it->second;
            else
                ErrorLogger() << "Universe::GetShipDesignsToSerialize empire " << encoding_empire << " should know about design with id " << design_id << " but no such design exists in the Universe!";
        }
    }
}

void Universe::GetObjectsToSerialize(ObjectMap& objects, int encoding_empire) const {
    if (&objects == &m_objects)
        return;

    objects.Clear();

    if (encoding_empire == ALL_EMPIRES) {
        // if encoding for all empires, copy true full universe state, and use the
        // streamlined option
        objects.CopyForSerialize(m_objects);

    } else if (!ENABLE_VISIBILITY_EMPIRE_MEMORY) {
        // if encoding without memory, copy all info visible to specified empire
        objects.Copy(m_objects, encoding_empire);

    } else {
        // if encoding for a specific empire with memory...

        // find indicated empire's knowledge about objects, current and previous
        EmpireObjectMap::const_iterator it = m_empire_latest_known_objects.find(encoding_empire);
        if (it == m_empire_latest_known_objects.end())
            return;                 // empire has no object knowledge, so there is nothing to send

        //the empire_latest_known_objects are already processed for visibility, so can be copied streamlined
        objects.CopyForSerialize(it->second);

        std::map< int, std::set< int > >::const_iterator destroyed_ids_it =
                m_empire_known_destroyed_object_ids.find(encoding_empire);
        bool map_avail = (destroyed_ids_it != m_empire_known_destroyed_object_ids.end());
        const std::set<int>& destroyed_object_ids = map_avail ? destroyed_ids_it->second : std::set<int>();

        objects.AuditContainment(destroyed_object_ids);
    }
}

void Universe::GetDestroyedObjectsToSerialize(std::set<int>& destroyed_object_ids, int encoding_empire) const {
    if (&destroyed_object_ids == &m_destroyed_object_ids)
        return;

    if (encoding_empire == ALL_EMPIRES) {
        // all destroyed objects
        destroyed_object_ids = m_destroyed_object_ids;
    } else {
        destroyed_object_ids.clear();
        // get empire's known destroyed objects
        ObjectKnowledgeMap::const_iterator it = m_empire_known_destroyed_object_ids.find(encoding_empire);
        if (it != m_empire_known_destroyed_object_ids.end())
            destroyed_object_ids = it->second;
    }
}

void Universe::GetEmpireKnownObjectsToSerialize(EmpireObjectMap& empire_latest_known_objects, int encoding_empire) const {
    if (&empire_latest_known_objects == &m_empire_latest_known_objects)
        return;

    DebugLogger() << "GetEmpireKnownObjectsToSerialize";

    for (EmpireObjectMap::value_type& entry : empire_latest_known_objects)
        entry.second.Clear();

    empire_latest_known_objects.clear();

    if (!ENABLE_VISIBILITY_EMPIRE_MEMORY)
        return;

    if (encoding_empire == ALL_EMPIRES) {
        // copy all ObjectMaps' contents
        for (const EmpireObjectMap::value_type& entry : m_empire_latest_known_objects) {
            int empire_id = entry.first;
            const ObjectMap& map = entry.second;
            //the maps in m_empire_latest_known_objects are already processed for visibility, so can be copied fully
            empire_latest_known_objects[empire_id].CopyForSerialize(map);
        }
        return;
    }
}

void Universe::GetEmpireObjectVisibilityMap(EmpireObjectVisibilityMap& empire_object_visibility, int encoding_empire) const {
    if (encoding_empire == ALL_EMPIRES) {
        empire_object_visibility = m_empire_object_visibility;
        return;
    }

    // include just requested empire's visibility for each object it has better
    // than no visibility of.  TODO: include what requested empire knows about
    // other empires' visibilites of objects
    empire_object_visibility.clear();
    for (ObjectMap::const_iterator<> it = m_objects.const_begin(); it != m_objects.const_end(); ++it) {
        int object_id = it->ID();
        Visibility vis = GetObjectVisibilityByEmpire(object_id, encoding_empire);
        if (vis > VIS_NO_VISIBILITY)
            empire_object_visibility[encoding_empire][object_id] = vis;
    }
}

void Universe::GetEmpireObjectVisibilityTurnMap(EmpireObjectVisibilityTurnMap& empire_object_visibility_turns, int encoding_empire) const {
    if (encoding_empire == ALL_EMPIRES) {
        empire_object_visibility_turns = m_empire_object_visibility_turns;
        return;
    }

    // include just requested empire's visibility turn information
    empire_object_visibility_turns.clear();
    EmpireObjectVisibilityTurnMap::const_iterator it = m_empire_object_visibility_turns.find(encoding_empire);
    if (it != m_empire_object_visibility_turns.end())
        empire_object_visibility_turns[encoding_empire] = it->second;
}

void Universe::GetEmpireKnownDestroyedObjects(ObjectKnowledgeMap& empire_known_destroyed_object_ids, int encoding_empire) const {
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

void Universe::GetEmpireStaleKnowledgeObjects(ObjectKnowledgeMap& empire_stale_knowledge_object_ids, int encoding_empire) const {
    if (&empire_stale_knowledge_object_ids == &m_empire_stale_knowledge_object_ids)
        return;

    if (encoding_empire == ALL_EMPIRES) {
        empire_stale_knowledge_object_ids = m_empire_stale_knowledge_object_ids;
        return;
    }

    empire_stale_knowledge_object_ids.clear();

    // copy stale data for this empire
    ObjectKnowledgeMap::const_iterator it = m_empire_stale_knowledge_object_ids.find(encoding_empire);
    if (it != m_empire_stale_knowledge_object_ids.end())
        empire_stale_knowledge_object_ids[encoding_empire] = it->second;
}

template <class T>
std::shared_ptr<T> Universe::InsertNewObject(T* object) {
    m_objects.Insert(object);
    return m_objects.Object<T>(object->ID());
}

std::shared_ptr<Ship> Universe::CreateShip(int id/* = INVALID_OBJECT_ID*/)
{ return InsertID(new Ship(), id); }

std::shared_ptr<Ship> Universe::CreateShip(int empire_id, int design_id, const std::string& species_name,
                                           int produced_by_empire_id/*= ALL_EMPIRES*/, int id/* = INVALID_OBJECT_ID*/)
{ return InsertID(new Ship(empire_id, design_id, species_name, produced_by_empire_id), id); }

std::shared_ptr<Fleet> Universe::CreateFleet(int id/* = INVALID_OBJECT_ID*/)
{ return InsertID(new Fleet(), id); }

std::shared_ptr<Fleet> Universe::CreateFleet(const std::string& name, double x, double y, int owner, int id/* = INVALID_OBJECT_ID*/)
{ return InsertID(new Fleet(name, x, y, owner), id); }

std::shared_ptr<Planet> Universe::CreatePlanet(int id/* = INVALID_OBJECT_ID*/)
{ return InsertID(new Planet(), id); }

std::shared_ptr<Planet> Universe::CreatePlanet(PlanetType type, PlanetSize size, int id/* = INVALID_OBJECT_ID*/)
{ return InsertID(new Planet(type, size), id); }

std::shared_ptr<System> Universe::CreateSystem(int id/* = INVALID_OBJECT_ID*/)
{ return InsertID(new System(), id); }

std::shared_ptr<System> Universe::CreateSystem(StarType star, const std::string& name, double x, double y, int id/* = INVALID_OBJECT_ID*/)
{ return InsertID(new System(star, name, x, y), id); }

std::shared_ptr<System> Universe::CreateSystem(StarType star, const std::map<int, bool>& lanes_and_holes,
                                               const std::string& name, double x, double y, int id/* = INVALID_OBJECT_ID*/)
{ return InsertID(new System(star, lanes_and_holes, name, x, y), id); }

std::shared_ptr<Building> Universe::CreateBuilding(int id/* = INVALID_OBJECT_ID*/)
{ return InsertID(new Building(), id); }

std::shared_ptr<Building> Universe::CreateBuilding(int empire_id, const std::string& building_type,
                                                   int produced_by_empire_id/* = ALL_EMPIRES*/, int id/* = INVALID_OBJECT_ID*/)
{ return InsertID(new Building(empire_id, building_type, produced_by_empire_id), id); }

std::shared_ptr<Field> Universe::CreateField(int id/* = INVALID_OBJECT_ID*/)
{ return InsertID(new Field(), id); }

std::shared_ptr<Field> Universe::CreateField(const std::string& field_type, double x, double y, double radius, int id/* = INVALID_OBJECT_ID*/)
{ return InsertID(new Field(field_type, x, y, radius), id); }

void Universe::ResetUniverse() {
    m_objects.Clear();  // wipe out anything present in the object map

    m_empire_known_destroyed_object_ids.clear();
    m_empire_known_ship_design_ids.clear();
    m_empire_latest_known_objects.clear();
    m_effect_accounting_map.clear();
    m_empire_object_visibility.clear();
    m_empire_object_visibility_turns.clear();
    m_empire_object_visible_specials.clear();
    m_effect_accounting_map.clear();
    m_effect_discrepancy_map.clear();
    m_marked_destroyed.clear();
    m_destroyed_object_ids.clear();
    m_ship_designs.clear();
    m_stat_records.clear();
    m_universe_width = 1000.0;

    // these happen to be equal to INVALID_OBJECT_ID and INVALID_DESIGN_ID,
    // but the point here is that the latest used ID is incremented before
    // being assigned, so using -1 here means the first assigned ID will be 0,
    // which is a valid ID
    m_last_allocated_object_id = -1;
    m_last_allocated_design_id = -1;

    GetSpeciesManager().ClearSpeciesHomeworlds();
}
