#include "Universe.h"

#include "../util/OptionsDB.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/RunQueue.h"
#include "../util/ScopedTimer.h"
#include "../util/CheckSums.h"
#include "../util/GameRules.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "IDAllocator.h"
#include "Building.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "System.h"
#include "Field.h"
#include "UniverseObject.h"
#include "UniverseGenerator.h"
#include "Effect.h"
#include "Predicates.h"
#include "Special.h"
#include "Species.h"
#include "Tech.h"
#include "Condition.h"
#include "ValueRef.h"
#include "Enums.h"
#include "Pathfinder.h"
#include "Encyclopedia.h"

#include <boost/property_map/property_map.hpp>
#include <boost/timer.hpp>

FO_COMMON_API extern const int INVALID_DESIGN_ID;

namespace {
    DeclareThreadSafeLogger(effects);
}

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
        db.Add("effects.ui.threads",                UserStringNop("OPTIONS_DB_EFFECTS_THREADS_UI_DESC"),        8,      RangedValidator<int>(1, 32));
        db.Add("effects.ai.threads",                UserStringNop("OPTIONS_DB_EFFECTS_THREADS_AI_DESC"),        2,      RangedValidator<int>(1, 32));
        db.Add("effects.server.threads",            UserStringNop("OPTIONS_DB_EFFECTS_THREADS_SERVER_DESC"),    8,      RangedValidator<int>(1, 32));
        db.Add("effects.accounting.enabled",        UserStringNop("OPTIONS_DB_EFFECT_ACCOUNTING"),              true,   Validator<bool>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    void AddRules(GameRules& rules) {
        // makes all PRNG be reseeded frequently
        rules.Add<bool>("RULE_RESEED_PRNG_SERVER", "RULE_RESEED_PRNG_SERVER_DESC",
                        "", true, true);
        rules.Add<bool>("RULE_STARLANES_EVERYWHERE", "RULE_STARLANES_EVERYWHERE_DESC",
                        "TEST", false, true);
    }
    bool temp_bool2 = RegisterGameRules(&AddRules);


    // the effective distance for ships travelling along a wormhole, for
    // determining how much of their speed is consumed by the jump
    // unused variable const double    WORMHOLE_TRAVEL_DISTANCE = 0.1;

    template <class Key, class Value> struct constant_property
    { Value m_value; };
}

namespace boost {
    template <class Key, class Value> struct property_traits<constant_property<Key, Value>> {
        typedef Value value_type;
        typedef Key key_type;
        typedef readable_property_map_tag category;
    };
    template <class Key, class Value> const Value& get(const constant_property<Key, Value>& pmap, const Key&) { return pmap.m_value; }
}


extern FO_COMMON_API const int ALL_EMPIRES = -1;

/////////////////////////////////////////////
// class Universe
/////////////////////////////////////////////
Universe::Universe() :
    m_pathfinder(std::make_shared<Pathfinder>()),
    m_universe_width(1000.0),
    m_inhibit_universe_object_signals(false),
    m_encoding_empire(ALL_EMPIRES),
    m_all_objects_visible(false),
    m_object_id_allocator(new IDAllocator(ALL_EMPIRES, std::vector<int>(), INVALID_OBJECT_ID,
                                          TEMPORARY_OBJECT_ID, INVALID_OBJECT_ID)),
    m_design_id_allocator(new IDAllocator(ALL_EMPIRES, std::vector<int>(), INVALID_DESIGN_ID,
                                          TEMPORARY_OBJECT_ID, INVALID_DESIGN_ID))
{}

Universe::~Universe()
{ Clear(); }

void Universe::Clear() {
    // empty object maps
    m_objects.Clear();

    ResetAllIDAllocation();

    m_marked_destroyed.clear();
    m_destroyed_object_ids.clear();

    // clean up ship designs
    for (auto& entry : m_ship_designs)
        delete entry.second;
    m_ship_designs.clear();

    m_empire_object_visibility.clear();
    m_empire_object_visibility_turns.clear();
    m_empire_object_visible_specials.clear();

    m_empire_known_destroyed_object_ids.clear();
    m_empire_latest_known_objects.clear();
    m_empire_stale_knowledge_object_ids.clear();
    m_empire_known_ship_design_ids.clear();

    m_effect_accounting_map.clear();
    m_effect_discrepancy_map.clear();
    m_effect_specified_empire_object_visibilities.clear();

    m_stat_records.clear();

    m_universe_width = 1000.0;

    m_pathfinder = std::make_shared<Pathfinder>();
}

void Universe::ResetAllIDAllocation(const std::vector<int>& empire_ids) {

    // Find the highest already allocated id for saved games that did not partition ids by client
    int highest_allocated_id = INVALID_OBJECT_ID;
    for (const auto& obj: m_objects)
        highest_allocated_id = std::max(highest_allocated_id, obj->ID());

    *m_object_id_allocator = IDAllocator(ALL_EMPIRES, empire_ids, INVALID_OBJECT_ID,
                                         TEMPORARY_OBJECT_ID, highest_allocated_id);

    // Find the highest already allocated id for saved games that did not partition ids by client
    int highest_allocated_design_id = INVALID_DESIGN_ID;
    for (const auto& id_and_obj: m_ship_designs)
        highest_allocated_design_id = std::max(highest_allocated_design_id, id_and_obj.first);

    *m_design_id_allocator = IDAllocator(ALL_EMPIRES, empire_ids, INVALID_DESIGN_ID,
                                         TEMPORARY_OBJECT_ID, highest_allocated_design_id);

    DebugLogger() << "Reset id allocators with highest object id = " << highest_allocated_id
                  << " and highest design id = " << highest_allocated_design_id;
}

void Universe::SetInitiallyUnlockedItems(Pending::Pending<std::vector<ItemSpec>>&& future)
{ m_pending_items = std::move(future); }

const std::vector<ItemSpec>& Universe::InitiallyUnlockedItems() const
{ return Pending::SwapPending(m_pending_items, m_unlocked_items); }

void Universe::SetInitiallyUnlockedBuildings(Pending::Pending<std::vector<ItemSpec>>&& future)
{ m_pending_buildings = std::move(future); }

const std::vector<ItemSpec>& Universe::InitiallyUnlockedBuildings() const
{ return Pending::SwapPending(m_pending_buildings, m_unlocked_buildings); }

void Universe::SetInitiallyUnlockedFleetPlans(Pending::Pending<std::vector<std::unique_ptr<FleetPlan>>>&& future)
{ m_pending_fleet_plans = std::move(future);}

const std::vector<FleetPlan*> Universe::InitiallyUnlockedFleetPlans() const {
    Pending::SwapPending(m_pending_fleet_plans, m_unlocked_fleet_plans);
    std::vector<FleetPlan*> retval;
    for (const auto& plan : m_unlocked_fleet_plans)
        retval.push_back(plan.get());
    return retval;
}

void Universe::SetMonsterFleetPlans(Pending::Pending<std::vector<std::unique_ptr<MonsterFleetPlan>>>&& future)
{ m_pending_monster_fleet_plans = std::move(future); }

const std::vector<MonsterFleetPlan*> Universe::MonsterFleetPlans() const {
    Pending::SwapPending(m_pending_monster_fleet_plans, m_monster_fleet_plans);
    std::vector<MonsterFleetPlan*> retval;
    for (const auto& plan : m_monster_fleet_plans)
        retval.push_back(plan.get());
    return retval;
}

void Universe::SetEmpireStats(Pending::Pending<EmpireStatsMap> future)
{ m_pending_empire_stats = std::move(future); }

const Universe::EmpireStatsMap& Universe::EmpireStats() const
{ return Pending::SwapPending(m_pending_empire_stats, m_empire_stats); }

const ObjectMap& Universe::EmpireKnownObjects(int empire_id) const {
    if (empire_id == ALL_EMPIRES)
        return m_objects;

    auto it = m_empire_latest_known_objects.find(empire_id);
    if (it != m_empire_latest_known_objects.end())
        return it->second;

    static const ObjectMap const_empty_map;
    return const_empty_map;
}

ObjectMap& Universe::EmpireKnownObjects(int empire_id) {
    if (empire_id == ALL_EMPIRES)
        return m_objects;

    auto it = m_empire_latest_known_objects.find(empire_id);
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
        for (const auto& empire_entry : Empires())
            empire_ids.insert(empire_entry.first);

    // check each object's visibility against all empires, including the object
    // if an empire has visibility of it
    for (auto obj_it = m_objects.const_begin(); obj_it != m_objects.const_end(); ++obj_it) {
        int id = obj_it->ID();
        for (int detector_empire_id : empire_ids) {
            Visibility vis = GetObjectVisibilityByEmpire(id, detector_empire_id);
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

int Universe::HighestDestroyedObjectID() const {
    if (m_destroyed_object_ids.empty())
        return INVALID_OBJECT_ID;
    return *m_destroyed_object_ids.rbegin();
}

const std::set<int>& Universe::EmpireKnownDestroyedObjectIDs(int empire_id) const {
    auto it = m_empire_known_destroyed_object_ids.find(empire_id);
    if (it != m_empire_known_destroyed_object_ids.end())
        return it->second;
    return m_destroyed_object_ids;
}

const std::set<int>& Universe::EmpireStaleKnowledgeObjectIDs(int empire_id) const {
    auto it = m_empire_stale_knowledge_object_ids.find(empire_id);
    if (it != m_empire_stale_knowledge_object_ids.end())
        return it->second;
    static const std::set<int> empty_set;
    return empty_set;
}

const ShipDesign* Universe::GetShipDesign(int ship_design_id) const {
    if (ship_design_id == INVALID_DESIGN_ID)
        return nullptr;
    ship_design_iterator it = m_ship_designs.find(ship_design_id);
    return (it != m_ship_designs.end() ? it->second : nullptr);
}

void Universe::RenameShipDesign(int design_id, const std::string& name/* = ""*/,
                                const std::string& description/* = ""*/)
{
    auto design_it = m_ship_designs.find(design_id);
    if (design_it == m_ship_designs.end()) {
        DebugLogger() << "Universe::RenameShipDesign tried to rename a ship design that doesn't exist!";
        return;
    }
    ShipDesign* design = design_it->second;

    design->SetName(name);
    design->SetDescription(description);
}

const ShipDesign* Universe::GetGenericShipDesign(const std::string& name) const {
    if (name.empty())
        return nullptr;
    for (const auto& entry : m_ship_designs) {
        const ShipDesign* design = entry.second;
        const std::string& design_name = design->Name(false);
        if (name == design_name)
            return design;
    }
    return nullptr;
}

const std::set<int>& Universe::EmpireKnownShipDesignIDs(int empire_id) const {
    auto it = m_empire_known_ship_design_ids.find(empire_id);
    if (it != m_empire_known_ship_design_ids.end())
        return it->second;
    static const std::set<int> empty_set;
    return empty_set;
}

Visibility Universe::GetObjectVisibilityByEmpire(int object_id, int empire_id) const {
    if (empire_id == ALL_EMPIRES || GetUniverse().AllObjectsVisible())
        return VIS_FULL_VISIBILITY;

    auto empire_it = m_empire_object_visibility.find(empire_id);
    if (empire_it == m_empire_object_visibility.end())
        return VIS_NO_VISIBILITY;

    const ObjectVisibilityMap& vis_map = empire_it->second;

    auto vis_map_it = vis_map.find(object_id);
    if (vis_map_it == vis_map.end())
        return VIS_NO_VISIBILITY;

    return vis_map_it->second;
}

const Universe::VisibilityTurnMap& Universe::GetObjectVisibilityTurnMapByEmpire(int object_id, int empire_id) const {
    static const std::map<Visibility, int> empty_map;

    auto empire_it = m_empire_object_visibility_turns.find(empire_id);
    if (empire_it == m_empire_object_visibility_turns.end())
        return empty_map;

    const ObjectVisibilityTurnMap& obj_vis_turn_map = empire_it->second;
    auto object_it = obj_vis_turn_map.find(object_id);
    if (object_it == obj_vis_turn_map.end())
        return empty_map;

    return object_it->second;
}

std::set<std::string> Universe::GetObjectVisibleSpecialsByEmpire(int object_id, int empire_id) const {
    if (empire_id != ALL_EMPIRES) {
        auto empire_it = m_empire_object_visible_specials.find(empire_id);
        if (empire_it == m_empire_object_visible_specials.end())
            return std::set<std::string>();
        const ObjectSpecialsMap& object_specials_map = empire_it->second;
        auto object_it = object_specials_map.find(object_id);
        if (object_it == object_specials_map.end())
            return std::set<std::string>();
        return object_it->second;
    } else {
        auto obj = m_objects.Object(object_id);
        if (!obj)
            return std::set<std::string>();
        // all specials visible
        std::set<std::string> retval;
        for (const auto& entry : obj->Specials()) {
            retval.insert(entry.first);
        }
        return retval;
    }
}

int Universe::GenerateObjectID() {
    auto new_id = m_object_id_allocator->NewID();
    return new_id;
}

int Universe::GenerateDesignID() {
    auto new_id = m_design_id_allocator->NewID();
    return new_id;
}

void Universe::ObfuscateIDGenerator() {
    m_object_id_allocator->ObfuscateBeforeSerialization();
    m_design_id_allocator->ObfuscateBeforeSerialization();
}

bool Universe::VerifyUnusedObjectID(const int empire_id, const int id) {
    auto good_id_and_possible_legacy = m_object_id_allocator->IsIDValidAndUnused(id, empire_id);
    if (!good_id_and_possible_legacy.second) // Possibly from old save game
        ErrorLogger() << "object id = " << id << " should not have been assigned by empire = " << empire_id;

    return good_id_and_possible_legacy.first && good_id_and_possible_legacy.second;
}

void Universe::InsertIDCore(std::shared_ptr<UniverseObject> obj, int id) {
    if (!obj)
        return;

    auto valid = m_object_id_allocator->UpdateIDAndCheckIfOwned(id);
    if (!valid) {
        ErrorLogger() << "An object has not been inserted into the universe because it's id = " << id << " is invalid.";
        obj->SetID(INVALID_OBJECT_ID);
        return;
    }

    obj->SetID(id);
    m_objects.Insert(std::forward<std::shared_ptr<UniverseObject>>(obj));
}

bool Universe::InsertShipDesign(ShipDesign* ship_design) {
    if (!ship_design
        || (ship_design->ID() != INVALID_DESIGN_ID && m_ship_designs.count(ship_design->ID())))
    { return false; }

    return InsertShipDesignID(ship_design, boost::none, GenerateDesignID());
}

bool Universe::InsertShipDesignID(ShipDesign* ship_design, boost::optional<int> empire_id, int id) {
    if (!ship_design)
        return false;

    if (!m_design_id_allocator->UpdateIDAndCheckIfOwned(id)) {
        ErrorLogger() << "Ship design id " << id << " is invalid.";
        return false;
    }

    ship_design->SetID(id);
    m_ship_designs[id] = ship_design;
    return true;
}

bool Universe::DeleteShipDesign(int design_id) {
    auto it = m_ship_designs.find(design_id);
    if (it != m_ship_designs.end()) {
        m_ship_designs.erase(it);
        return true;
    } else { return false; }
}

void Universe::ResetAllObjectMeters(bool target_max_unpaired, bool active) {
    for (const auto& object : m_objects) {
        if (target_max_unpaired)
            object->ResetTargetMaxUnpairedMeters();
        if (active)
            object->ResetPairedActiveMeters();
    }
}

void Universe::ResetObjectMeters(const std::vector<std::shared_ptr<UniverseObject>>& objects,
                                 bool target_max_unpaired, bool active)
{
    for (const auto& object : objects) {
        if (target_max_unpaired)
            object->ResetTargetMaxUnpairedMeters();
        if (active)
            object->ResetPairedActiveMeters();
    }
}

void Universe::ApplyAllEffectsAndUpdateMeters(bool do_accounting) {
    ScopedTimer timer("Universe::ApplyAllEffectsAndUpdateMeters");

    if (do_accounting) {
        // override if option disabled
        do_accounting = GetOptionsDB().Get<bool>("effects.accounting.enabled");
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
    ResetAllObjectMeters(true, true);
    for (auto& entry : Empires())
        entry.second->ResetMeters();

    ExecuteEffects(targets_causes, do_accounting, false, false, true);
    // clamp max meters to [DEFAULT_VALUE, LARGE_VALUE] and current meters to [DEFAULT_VALUE, max]
    // clamp max and target meters to [DEFAULT_VALUE, LARGE_VALUE] and current meters to [DEFAULT_VALUE, max]
    for (const auto& object : m_objects)
        object->ClampMeters();
}

void Universe::ApplyMeterEffectsAndUpdateMeters(const std::vector<int>& object_ids, bool do_accounting) {
    if (object_ids.empty())
        return;
    ScopedTimer timer("Universe::ApplyMeterEffectsAndUpdateMeters on " + std::to_string(object_ids.size()) + " objects");
    if (do_accounting) {
        // override if disabled
        do_accounting = GetOptionsDB().Get<bool>("effects.accounting.enabled");
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
    ResetObjectMeters(objects, true, true);
    // could also reset empire meters here, but unless all objects have meters
    // recalculated, some targets that lead to empire meters being modified may
    // be missed, and estimated empire meters would be inaccurate

    ExecuteEffects(targets_causes, do_accounting, true);

    for (auto& object : objects)
        object->ClampMeters();
}

void Universe::ApplyMeterEffectsAndUpdateMeters(bool do_accounting) {
    ScopedTimer timer("Universe::ApplyMeterEffectsAndUpdateMeters on all objects");
    if (do_accounting) {
        // override if disabled
        do_accounting = GetOptionsDB().Get<bool>("effects.accounting.enabled");
    }

    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes);

    TraceLogger(effects) << "Universe::ApplyMeterEffectsAndUpdateMeters resetting...";
    for (const auto& object : m_objects) {
        TraceLogger(effects) << "object " << object->Name() << " (" << object->ID() << ") before resetting meters: ";
        for (auto const& meter_pair : object->Meters()) {
            TraceLogger(effects) << "    meter: " << boost::lexical_cast<std::string>(meter_pair.first)
                                 << "  value: " << meter_pair.second.Current();
        }
        object->ResetTargetMaxUnpairedMeters();
        object->ResetPairedActiveMeters();
        TraceLogger(effects) << "object " << object->Name() << " (" << object->ID() << ") after resetting meters: ";
        for (auto const& meter_pair : object->Meters()) {
            TraceLogger(effects) << "    meter: " << boost::lexical_cast<std::string>(meter_pair.first)
                                 << "  value: " << meter_pair.second.Current();
        }
    }
    for (auto& entry : Empires())
        entry.second->ResetMeters();
    ExecuteEffects(targets_causes, do_accounting, true, false, true);

    for (const auto& object : m_objects)
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

    // save starting meter vales
    Effect::DiscrepancyMap starting_current_meter_values;
    for (const auto& obj : m_objects)
        for (const auto& meter_pair : obj->Meters())
            starting_current_meter_values[obj->ID()][meter_pair.first] = meter_pair.second.Current();


    // generate new estimates (normally uses discrepancies, but in this case will find none)
    UpdateMeterEstimates();


    DebugLogger() << "IMEAD: determining discrepancies";
    // determine meter max discrepancies
    for (auto& entry : m_effect_accounting_map) {
        int object_id = entry.first;
        // skip destroyed objects
        if (m_destroyed_object_ids.count(object_id))
            continue;
        // get object
        auto obj = m_objects.Object(object_id);
        if (!obj) {
            ErrorLogger() << "Universe::InitMeterEstimatesAndDiscrepancies couldn't find an object that was in the effect accounting map...?";
            continue;
        }

        TraceLogger(effects) << "... discrepancies for " << obj->Name() << " (" << obj->ID() << "):";

        // every meter has a value at the start of the turn, and a value after
        // updating with known effects
        for (auto& meter_pair : obj->Meters()) {
            MeterType type = meter_pair.first;

            // skip paired active meters, as differences in these are expected
            // and persistent, and not a "discrepancy"
            if (type >= METER_POPULATION && type <= METER_TROOPS)
                continue;

            // discrepancy is the difference between expected and actual meter
            // values at start of turn. here "expected" is what the meter value
            // was before updating the meters, and actual is what it is now
            // after updating the meters based on the known universe.
            Meter& meter = meter_pair.second;
            float discrepancy = starting_current_meter_values[object_id][type] - meter.Current();
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

            TraceLogger(effects) << "... ... " << boost::lexical_cast<std::string>(type) << ": " << discrepancy;
        }
    }
}

void Universe::UpdateMeterEstimates()
{ UpdateMeterEstimates(GetOptionsDB().Get<bool>("effects.accounting.enabled")); }

void Universe::UpdateMeterEstimates(bool do_accounting) {
    for (int obj_id : m_objects.FindExistingObjectIDs())
        m_effect_accounting_map[obj_id].clear();
    // update meters for all objects.
    UpdateMeterEstimatesImpl(std::vector<int>(), do_accounting);
}

void Universe::UpdateMeterEstimates(int object_id, bool update_contained_objects) {
    // ids of the object and all valid contained objects
    std::unordered_set<int> collected_ids;

    // Collect objects ids to update meter for.  This may be a single object, a
    // group of related objects. Return true if all collected ids are valid.
    std::function<bool (int, int)> collect_ids =
        [this, &collected_ids, update_contained_objects, &collect_ids]
        (int cur_id, int container_id)
    {
        // Ignore if already in the set
        if (collected_ids.count(cur_id))
            return true;

        auto cur_object = m_objects.Object(cur_id);
        if (!cur_object) {
            ErrorLogger() << "Universe::UpdateMeterEstimates tried to get an invalid object for id " << cur_id
                          << " in container " << container_id
                          << ". All meter estimates will be updated.";
            UpdateMeterEstimates();
            return false;
        }

        // add object
        collected_ids.insert(cur_id);

        // add contained objects to list of objects to process, if requested.
        if (update_contained_objects)
            for (const auto& contained_id : cur_object->ContainedObjectIDs())
                if (!collect_ids(contained_id, cur_id))
                    return false;
        return true;
    };

    if (!collect_ids(object_id, INVALID_OBJECT_ID))
        return;

    if (collected_ids.empty())
        return;

    // Clear ids that will be updated
    for (auto cur_id : collected_ids)
        m_effect_accounting_map[cur_id].clear();

    // Convert to a vector
    std::vector<int> objects_vec;
    objects_vec.reserve(collected_ids.size());
    std::copy(collected_ids.begin(), collected_ids.end(), std::back_inserter(objects_vec));
    UpdateMeterEstimatesImpl(objects_vec, GetOptionsDB().Get<bool>("effects.accounting.enabled"));
}

void Universe::UpdateMeterEstimates(const std::vector<int>& objects_vec) {
    std::set<int> objects_set;  // ensures no duplicates

    for (int object_id : objects_vec) {
        // skip destroyed objects
        if (m_destroyed_object_ids.count(object_id))
            continue;
        m_effect_accounting_map[object_id].clear();
        objects_set.insert(object_id);
    }
    std::vector<int> final_objects_vec;
    std::copy(objects_set.begin(), objects_set.end(), std::back_inserter(final_objects_vec));
    if (!final_objects_vec.empty())
        UpdateMeterEstimatesImpl(final_objects_vec, GetOptionsDB().Get<bool>("effects.accounting.enabled"));
}

void Universe::UpdateMeterEstimatesImpl(const std::vector<int>& objects_vec, bool do_accounting) {
    ScopedTimer timer("Universe::UpdateMeterEstimatesImpl on " + std::to_string(objects_vec.size()) + " objects", true);

    // get all pointers to objects once, to avoid having to do so repeatedly
    // when iterating over the list in the following code
    auto object_ptrs = m_objects.FindObjects(objects_vec);
    if (objects_vec.empty()) {
        object_ptrs.reserve(m_objects.ExistingObjects().size());
        std::transform(Objects().ExistingObjects().begin(), Objects().ExistingObjects().end(),
                       std::back_inserter(object_ptrs),
                       boost::bind(&std::map<int, std::shared_ptr<UniverseObject>>::value_type::second, _1));
    }

    for (auto& obj : object_ptrs) {
        int obj_id = obj->ID();

        // Reset max meters to DEFAULT_VALUE and current meters to initial value
        // at start of this turn
        obj->ResetTargetMaxUnpairedMeters();
        obj->ResetPairedActiveMeters();

        if (!do_accounting)
            continue;

        // record current value(s) of meters after resetting
        for (MeterType type = MeterType(0); type != NUM_METER_TYPES;
             type = MeterType(type + 1))
        {
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

    TraceLogger(effects) << "UpdateMeterEstimatesImpl after resetting meters objects:";
    for (auto& obj : object_ptrs)
        TraceLogger(effects) << obj->Dump();

    // cache all activation and scoping condition results before applying Effects, since the application of
    // these Effects may affect the activation and scoping evaluations
    Effect::TargetsCauses targets_causes;
    GetEffectsAndTargets(targets_causes, objects_vec);

    // Apply and record effect meter adjustments
    ExecuteEffects(targets_causes, do_accounting, true, false, false, false);

    TraceLogger(effects) << "UpdateMeterEstimatesImpl after executing effects objects:";
    for (auto& obj : object_ptrs)
        TraceLogger(effects) << obj->Dump();

    // Apply known discrepancies between expected and calculated meter maxes at start of turn.  This
    // accounts for the unknown effects on the meter, and brings the estimate in line with the actual
    // max at the start of the turn
    if (!m_effect_discrepancy_map.empty() && do_accounting) {
        for (auto& obj : object_ptrs) {
            int obj_id = obj->ID();

            // check if this object has any discrepancies
            auto dis_it = m_effect_discrepancy_map.find(obj_id);
            if (dis_it == m_effect_discrepancy_map.end())
                continue;   // no discrepancy, so skip to next object

            // apply all meters' discrepancies
            for (auto& entry : dis_it->second) {
                MeterType type = entry.first;
                double discrepancy = entry.second;

                //if (discrepancy == 0.0) continue;

                Meter* meter = obj->GetMeter(type);
                if (!meter)
                    continue;

                TraceLogger(effects) << "object " << obj_id << " has meter " << type
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

    // clamp meters to valid range of max values, and so current is less than max
    for (auto& obj : object_ptrs) {
        // currently this clamps all meters, even if not all meters are being processed by this function...
        // but that shouldn't be a problem, as clamping meters that haven't changed since they were last
        // updated should have no effect
        obj->ClampMeters();
    }

    TraceLogger(effects) << "UpdateMeterEstimatesImpl after discrepancies and clamping objects:";
    for (auto& obj : object_ptrs)
        TraceLogger(effects) << obj->Dump();

}

void Universe::BackPropagateObjectMeters(const std::vector<int>& object_ids) {
    // copy current meter values to initial values
    for (auto& obj : m_objects.FindObjects(object_ids))
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
            std::map<const Condition::ConditionBase*, std::pair<bool, Effect::TargetSet>> m_entries;
            boost::shared_mutex m_mutex;
            boost::condition_variable_any m_state_changed;
        };
        StoreTargetsAndCausesOfEffectsGroupsWorkItem(
            const std::shared_ptr<Effect::EffectsGroup>&            the_effects_group,
            const std::vector<std::shared_ptr<const UniverseObject>>& the_sources,
            EffectsCauseType                                        the_effect_cause_type,
            const std::string&                                      the_specific_cause_name,
            Effect::TargetSet&                                      the_target_objects,
            Effect::TargetsCauses&                                  the_targets_causes,
            std::map<int, std::shared_ptr<ConditionCache>>&         the_source_cached_condition_matches,
            ConditionCache&                                         the_invariant_cached_condition_matches,
            boost::shared_mutex&                                    the_global_mutex
        );
        void operator ()();

        /** Return a report of that state of this work item. */
        std::string GenerateReport() const;

    private:
        // WARNING: do NOT copy the shared_pointers! Use raw pointers, shared_ptr may not be thread-safe.
        std::shared_ptr<Effect::EffectsGroup>                   m_effects_group;
        const std::vector<std::shared_ptr<const UniverseObject>>* m_sources;
        EffectsCauseType                                        m_effect_cause_type;
        const std::string                                       m_specific_cause_name;
        Effect::TargetSet*                                      m_target_objects;
        Effect::TargetsCauses*                                  m_targets_causes;
        std::map<int, std::shared_ptr<ConditionCache>>*         m_source_cached_condition_matches;
        ConditionCache*                                         m_invariant_cached_condition_matches;
        boost::shared_mutex*                                    m_global_mutex;

        static Effect::TargetSet& GetConditionMatches(
            const Condition::ConditionBase*    cond,
            ConditionCache&                    cached_condition_matches,
            std::shared_ptr<const UniverseObject> source,
            const ScriptingContext&            source_context,
            Effect::TargetSet&                 target_objects,
            std::string&                       match_log);
    };

    StoreTargetsAndCausesOfEffectsGroupsWorkItem::StoreTargetsAndCausesOfEffectsGroupsWorkItem(
            const std::shared_ptr<Effect::EffectsGroup>&            the_effects_group,
            const std::vector<std::shared_ptr<const UniverseObject>>& the_sources,
            EffectsCauseType                                        the_effect_cause_type,
            const std::string&                                      the_specific_cause_name,
            Effect::TargetSet&                                      the_target_objects,
            Effect::TargetsCauses&                                  the_targets_causes,
            std::map<int, std::shared_ptr<ConditionCache>>&         the_source_cached_condition_matches,
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

    std::pair<bool, Effect::TargetSet>*
    StoreTargetsAndCausesOfEffectsGroupsWorkItem::ConditionCache::Find(
        const Condition::ConditionBase* cond, bool insert)
    {
        // have to iterate through cached condition matches, rather than using
        // find, since there is no operator< for comparing conditions by value
        // and by pointer is irrelivant.
        boost::unique_lock<boost::shared_mutex> unique_guard(m_mutex, boost::defer_lock_t());
        boost::shared_lock<boost::shared_mutex> shared_guard(m_mutex, boost::defer_lock_t());

        if (insert)
            unique_guard.lock();
        else
            shared_guard.lock();

        for (auto& entry : m_entries) {
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

    void StoreTargetsAndCausesOfEffectsGroupsWorkItem::ConditionCache::MarkComplete(
        std::pair<bool, Effect::TargetSet>* cache_entry)
    {
        boost::unique_lock<boost::shared_mutex> cache_guard(m_mutex); // make sure threads are waiting for completion, not checking for completion
        cache_entry->first = true;
        m_state_changed.notify_all(); // signal cachefill
    }

    void StoreTargetsAndCausesOfEffectsGroupsWorkItem::ConditionCache::LockShared(
        boost::shared_lock<boost::shared_mutex>& guard)
    {
        boost::shared_lock<boost::shared_mutex> tmp_guard(m_mutex);
        guard.swap(tmp_guard);
    }

    Effect::TargetSet EMPTY_TARGET_SET;

    Effect::TargetSet& StoreTargetsAndCausesOfEffectsGroupsWorkItem::GetConditionMatches(
        const Condition::ConditionBase*                                 cond,
        StoreTargetsAndCausesOfEffectsGroupsWorkItem::ConditionCache&   cached_condition_matches,
        std::shared_ptr<const UniverseObject>                           source,
        const ScriptingContext&                                         source_context,
        Effect::TargetSet&                                              target_objects,
        std::string&                                                    match_log)
    {
        if (!cond)
            return EMPTY_TARGET_SET;

        // the passed-in cached_condition_matches are here expected be specific for the current source object
        std::pair<bool, Effect::TargetSet>* cache_entry = cached_condition_matches.Find(cond, false);
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

        // log condition scope matches, except for Source
        if (!(dynamic_cast<const Condition::Source*>(cond))) {
            std::stringstream ss;
            ss << "\nGenerated new target set, for Condition: " << cond->Dump() << "\n    targets: (";
            for (const auto& obj : *target_set)
                ss << obj->Name() << " (" << std::to_string(obj->ID()) << ")  ";
            ss << ")";
            match_log.append(ss.str());
        }

        cached_condition_matches.MarkComplete(cache_entry);

        return *target_set;
    }

    std::string StoreTargetsAndCausesOfEffectsGroupsWorkItem::GenerateReport() const {
        boost::unique_lock<boost::shared_mutex> guard(*m_global_mutex);
        std::stringstream ss;
        ss << "StoreTargetsAndCausesOfEffectsGroups: effects_group: "
           << m_effects_group->AccountingLabel()
           << "  specific_cause: " << m_specific_cause_name
           << "  sources: ";
        for (const auto& obj : *m_sources)
            ss << obj->Name() << " (" << std::to_string(obj->ID()) << ")  ";
        ss << ")";
        return ss.str();
    }

    void StoreTargetsAndCausesOfEffectsGroupsWorkItem::operator()() {
        ScopedTimer timer("StoreTargetsAndCausesOfEffectsGroups");

        // get objects matched by scope
        const auto* scope = m_effects_group->Scope();
        if (!scope) {
            TraceLogger(effects) << GenerateReport();
            return;
        }

        // create temporary container for concurrent work
        Effect::TargetSet target_objects(*m_target_objects);
        std::string match_log;
        // process all sources in set provided
        for (auto& source : *m_sources) {
            ScriptingContext source_context(source);
            int source_object_id = (source ? source->ID() : INVALID_OBJECT_ID);
            ScopedTimer update_timer("... StoreTargetsAndCausesOfEffectsGroups done processing source " +
                                     std::to_string(source_object_id) +
                                     " cause: " + m_specific_cause_name);

            // skip inactive sources
            // FIXME: is it safe to move this out of the loop?
            // Activation condition must not contain "Source" subconditions in that case
            const auto* activation = m_effects_group->Activation();
            if (activation && !activation->Eval(source_context, source))
                continue;

            // if scope is source-invariant, use the source-invariant cache of condition results.
            // if scope depends on the source object, use a cache of condition results for that souce object.
            bool source_invariant = !source || scope->SourceInvariant();
            ConditionCache* condition_cache = source_invariant ?
                m_invariant_cached_condition_matches :
                (*m_source_cached_condition_matches)[source_object_id].get();

            // look up scope condition in the cache. if not found, calculate it
            // and store in the cache. either way, return the result.
            auto& target_set = GetConditionMatches(scope, *condition_cache,
                                                   source, source_context,
                                                   target_objects, match_log);

            {
                // As of this writing, this code is 4-5 years old and its author
                // (cami) is long gone. I don't really understand the purpose
                // of this lock and check of target_set, but my guess after a
                // bit of looking at the code is that the above call to
                // GetConditionMatches can be happening in several threads,
                // and each could attempt to evaluate the same scope and source
                // combination in parallel. Here, getting the shared_lock
                // ensures that no other threads are currently modifying the
                // cache. It should then be safe to check if the targets are
                // empty, as it is assured that the emptiness is the result
                // of evaluating the scope condition, and not an intermediate
                // state of another thread's incomplete evaluation. After any
                // thread has evaluated a scope/source combination, that result
                // shouldn't be modified by any other thread, so can be used in
                // the code below without access guards.

                boost::shared_lock<boost::shared_mutex> cache_guard;
                condition_cache->LockShared(cache_guard);

                if (target_set.empty())
                    continue;
            }

            {
                TraceLogger(effects) << GenerateReport() << match_log;

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
                m_targets_causes->push_back({sourced_effects_group, target_and_cause});
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

    TraceLogger(effects) << "target objects:";
    for (auto& obj : all_potential_targets)
        TraceLogger(effects) << obj->Dump();

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
    TraceLogger(effects) << "Universe::GetEffectsAndTargets for SPECIES";
    type_timer.restart();

    // find each species planets in single pass, maintaining object map order per-species
    std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>> species_objects;
    for (auto& planet : m_objects.FindObjects<Planet>()) {
        if (m_destroyed_object_ids.count(planet->ID()))
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
    for (auto& ship : m_objects.FindObjects<Ship>()) {
        if (m_destroyed_object_ids.count(ship->ID()))
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
    for (const auto& entry : GetSpeciesManager()) {
        const std::string& species_name = entry.first;
        const auto& species = entry.second;
        auto species_objects_it = species_objects.find(species_name);

        if (species_objects_it == species_objects.end())
            continue;

        for (auto& effects_group : species->Effects()) {
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
    TraceLogger(effects) << "Universe::GetEffectsAndTargets for SPECIALS";
    type_timer.restart();
    std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>> specials_objects;
    // determine objects with specials in a single pass
    for (const auto& obj : m_objects) {
        if (m_destroyed_object_ids.count(obj->ID()))
            continue;
        for (const auto& entry : obj->Specials()) {
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
        const Special* special = GetSpecial(special_name);
        auto specials_objects_it = specials_objects.find(special_name);

        if (specials_objects_it == specials_objects.end())
            continue;

        for (auto& effects_group : special->Effects()) {
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
    TraceLogger(effects) << "Universe::GetEffectsAndTargets for TECHS";
    type_timer.restart();
    std::list<std::vector<std::shared_ptr<const UniverseObject>>> tech_sources;
    for (auto& entry : Empires()) {
        const Empire* empire = entry.second;
        auto source = empire->Source();
        if (!source)
            continue;

        tech_sources.push_back(std::vector<std::shared_ptr<const UniverseObject>>(1U, source));
        for (const auto tech_entry : empire->ResearchedTechs()) {
            const Tech* tech = GetTech(tech_entry.first);
            if (!tech) continue;

            for (auto& effects_group : tech->Effects()) {
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
    TraceLogger(effects) << "Universe::GetEffectsAndTargets for BUILDINGS";
    type_timer.restart();

    // determine buildings of each type in a single pass
    std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>> buildings_by_type;
    for (auto& building : m_objects.FindObjects<Building>()) {
        if (m_destroyed_object_ids.count(building->ID()))
            continue;
        const std::string& building_type_name = building->BuildingTypeName();
        const BuildingType* building_type = GetBuildingType(building_type_name);
        if (!building_type) {
            ErrorLogger() << "GetEffectsAndTargets couldn't get BuildingType " << building->BuildingTypeName();
            continue;
        }

        buildings_by_type[building_type_name].push_back(building);
    }

    // enforce building types effects order
    for (const auto& entry : GetBuildingTypeManager()) {
        const std::string& building_type_name = entry.first;
        const BuildingType* building_type = entry.second.get();
        auto buildings_by_type_it = buildings_by_type.find(building_type_name);

        if (buildings_by_type_it == buildings_by_type.end())
            continue;

        for (auto& effects_group : building_type->Effects()) {
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
    TraceLogger(effects) << "Universe::GetEffectsAndTargets for SHIPS hulls and parts";
    type_timer.restart();
    // determine ship hulls and parts of each type in a single pass
    // the same ship might be added multiple times if it contains the part multiple times
    // recomputing targets for the same ship and part is kind of silly here, but shouldn't hurt
    std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>> ships_by_hull_type;
    std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>> ships_by_part_type;
    for (auto& ship : m_objects.FindObjects<Ship>()) {
        if (m_destroyed_object_ids.count(ship->ID()))
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
    for (const auto& entry : GetHullTypeManager()) {
        const std::string& hull_type_name = entry.first;
        const auto& hull_type = entry.second;
        auto ships_by_hull_type_it = ships_by_hull_type.find(hull_type_name);

        if (ships_by_hull_type_it == ships_by_hull_type.end())
            continue;

        for (auto& effects_group : hull_type->Effects()) {
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
    for (const auto& entry : GetPartTypeManager()) {
        const std::string& part_type_name = entry.first;
        const auto& part_type = entry.second;
        auto ships_by_part_type_it = ships_by_part_type.find(part_type_name);

        if (ships_by_part_type_it == ships_by_part_type.end())
            continue;

        for (auto& effects_group : part_type->Effects()) {
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
    TraceLogger(effects) << "Universe::GetEffectsAndTargets for FIELDS";
    type_timer.restart();
    // determine fields of each type in a single pass
    std::map<std::string, std::vector<std::shared_ptr<const UniverseObject>>> fields_by_type;
    for (auto& field : m_objects.FindObjects<Field>()) {
        if (m_destroyed_object_ids.count(field->ID()))
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
    for (const auto& entry : GetFieldTypeManager()) {
        const std::string& field_type_name = entry.first;
        const auto& field_type = entry.second;
        auto fields_by_type_it = fields_by_type.find(field_type_name);

        if (fields_by_type_it == fields_by_type.end())
            continue;

        for (auto effects_group : field_type->Effects()) {
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
    std::map<std::string, std::set<int>> executed_nonstacking_effects;

    // grouping targets causes by effects group
    // sorting by effects group has already been done in GetEffectsAndTargets()
    // FIXME: GetEffectsAndTargets already produces this separation, exploit that
    std::map<int, std::vector<std::pair<Effect::EffectsGroup*, Effect::TargetsCauses>>> dispatched_targets_causes;
    {
        const Effect::EffectsGroup* last_effects_group   = nullptr;
        Effect::TargetsCauses*      group_targets_causes = nullptr;

        for (const auto& targets_cause : targets_causes) {
            const Effect::SourcedEffectsGroup& sourced_effects_group = targets_cause.first;
            Effect::EffectsGroup* effects_group = sourced_effects_group.effects_group.get();

            if (effects_group != last_effects_group) {
                last_effects_group = effects_group;
                dispatched_targets_causes[effects_group->Priority()].push_back({effects_group, Effect::TargetsCauses()});
                group_targets_causes = &dispatched_targets_causes[effects_group->Priority()].back().second;
            }
            group_targets_causes->push_back(targets_cause);
        }
    }

    // execute each effects group one by one
    for (auto& priority_group : dispatched_targets_causes) {
        for (auto& effect_group_entry : priority_group.second) {
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

                for (auto targets_it = group_targets_causes.begin();
                     targets_it != group_targets_causes.end();)
                {
                    Effect::TargetsAndCause& targets_and_cause = targets_it->second;
                    Effect::TargetSet& targets = targets_and_cause.target_set;

                    // this is a set difference/union algorithm:
                    // targets              -= non_stacking_targets
                    // non_stacking_targets += targets
                    for (auto object_it = targets.begin();
                         object_it != targets.end();)
                    {
                        int object_id = (*object_it)->ID();
                        auto it = non_stacking_targets.find(object_id);

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

            TraceLogger(effects) << "\n\n * * * * * * * * * * * (new effects group log entry)(" << effects_group->TopLevelContent()
                                 << " " << effects_group->AccountingLabel() << " " << effects_group->StackingGroup() << ")";

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

    for (auto& entry : m_marked_destroyed) {
        int obj_id = entry.first;
        auto obj = GetUniverseObject(obj_id);
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
        switch (obj->ObjectType()) {
        case OBJ_PLANET: {
            auto obj_planet = std::static_pointer_cast<const Planet>(obj);
            return obj_planet->SpeciesName();
            break;
        }
        case OBJ_SHIP: {
            auto obj_ship = std::static_pointer_cast<const Ship>(obj);
            return obj_ship->SpeciesName();
            break;
        }
        default:
            return EMPTY_STRING;
        }
    }

    int GetDesignIDFromObject(std::shared_ptr<const UniverseObject> obj) {
        if (obj->ObjectType() != OBJ_SHIP)
            return INVALID_DESIGN_ID;
        auto shp = std::static_pointer_cast<const Ship>(obj);
        return shp->DesignID();
    }
}

void Universe::CountDestructionInStats(int object_id, int source_object_id) {
    auto obj = GetUniverseObject(object_id);
    if (!obj)
        return;
    auto source = GetUniverseObject(source_object_id);
    if (!source)
        return;

    const std::string& species_for_obj = GetSpeciesFromObject(obj);

    int empire_for_obj_id = obj->Owner();
    int empire_for_source_id = source->Owner();

    int design_for_obj_id = GetDesignIDFromObject(obj);

    if (Empire* source_empire = GetEmpire(empire_for_source_id)) {
        source_empire->EmpireShipsDestroyed()[empire_for_obj_id]++;

        if (design_for_obj_id != INVALID_DESIGN_ID)
            source_empire->ShipDesignsDestroyed()[design_for_obj_id]++;

        if (species_for_obj.empty())
            source_empire->SpeciesShipsDestroyed()[species_for_obj]++;
    }

    if (Empire* obj_empire = GetEmpire(empire_for_obj_id)) {
        if (!species_for_obj.empty())
            obj_empire->SpeciesShipsLost()[species_for_obj]++;

        if (design_for_obj_id != INVALID_DESIGN_ID)
            obj_empire->ShipDesignsLost()[design_for_obj_id]++;
    }
}

void Universe::SetEffectDerivedVisibility(int empire_id, int object_id, int source_id,
                                          const ValueRef::ValueRefBase<Visibility>* vis)
{
    if (empire_id == ALL_EMPIRES)
        return;
    if (object_id <= INVALID_OBJECT_ID)
        return;
    if (!vis)
        return;
    m_effect_specified_empire_object_visibilities[empire_id][object_id].push_back({source_id, vis});
}

void Universe::ApplyEffectDerivedVisibilities() {
    EmpireObjectVisibilityMap new_empire_object_visibilities;
    // for each empire with a visibility map
    for (auto& empire_entry : m_effect_specified_empire_object_visibilities) {
        if (empire_entry.first == ALL_EMPIRES)
            continue;   // can't set a non-empire's visibility
        for (const auto& object_entry : empire_entry.second) {
            if (object_entry.first <= INVALID_OBJECT_ID)
                continue;   // can't set a non-object's visibility
            auto target = GetUniverseObject(object_entry.first);
            if (!target)
                continue;   // don't need to set a non-gettable object's visibility

            // if already have an entry in new_empire_object_visibilities then
            // use that as the target initial visibility for purposes of
            // evaluating this ValueRef. If not, use the object's current
            // in-universe Visibility for the specified empire
            Visibility target_initial_vis =
                m_empire_object_visibility[empire_entry.first][object_entry.first];
            auto neov_it = new_empire_object_visibilities[empire_entry.first].find(object_entry.first);
            if (neov_it != new_empire_object_visibilities[empire_entry.first].end())
                target_initial_vis = neov_it->second;

            // evaluate valuerefs and and store visibility of object
            for (auto& source_ref_entry : object_entry.second) {
                // set up context for executing ValueRef to determine visibility to set
                auto source = GetUniverseObject(source_ref_entry.first);
                ScriptingContext context(source, target, target_initial_vis);

                const auto val_ref = source_ref_entry.second;

                // evaluate and store actual new visibility level
                Visibility vis = val_ref->Eval(context);
                target_initial_vis = vis;   // store for next iteration's context
                new_empire_object_visibilities[empire_entry.first][object_entry.first] = vis;
            }
        }
    }

    // copy newly determined visibility levels into actual gamestate, without
    // erasing visibilities that aren't affected by the effects
    for (auto empire_entry : new_empire_object_visibilities) {
        int empire_id = empire_entry.first;
        for (auto object_entry : empire_entry.second) {
            int object_id = object_entry.first;
            Visibility vis = object_entry.second;
            m_empire_object_visibility[empire_id][object_id] = vis;
        }
    }
}

void Universe::ForgetKnownObject(int empire_id, int object_id) {
    // Note: Client calls this with empire_id == ALL_EMPIRES to
    // immediately forget information without waiting for the turn update.
    ObjectMap& objects(EmpireKnownObjects(empire_id));

    if (objects.Empty())
        return;

    auto obj = objects.Object(object_id);
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

    // Remove all contained objects to avoid breaking fleet+ship, system+planet invariants
    auto contained_ids = obj->ContainedObjectIDs();
    for (int child_id : contained_ids)
        ForgetKnownObject(empire_id, child_id);

    int container_id = obj->ContainerObjectID();
    if (container_id != INVALID_OBJECT_ID) {
        if (auto container = objects.Object(container_id)) {
            if (auto system = std::dynamic_pointer_cast<System>(container))
                system->Remove(object_id);
            else if (auto planet = std::dynamic_pointer_cast<Planet>(container))
                planet->RemoveBuilding(object_id);
            else if (auto fleet = std::dynamic_pointer_cast<Fleet>(container)) {
                fleet->RemoveShips({object_id});
                if (fleet->Empty())
                    objects.Remove(fleet->ID());
            }
        }
    }

    objects.Remove(object_id);
}

void Universe::SetEmpireObjectVisibility(int empire_id, int object_id, Visibility vis) {
    if (empire_id == ALL_EMPIRES || object_id == INVALID_OBJECT_ID)
        return;

    // get visibility map for empire and find object in it
    auto& vis_map = m_empire_object_visibility[empire_id];
    auto vis_map_it = vis_map.find(object_id);

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
        if (auto ship = GetShip(object_id))
            SetEmpireKnowledgeOfShipDesign(ship->DesignID(), empire_id);
    }
}

void Universe::SetEmpireSpecialVisibility(int empire_id, int object_id,
                                          const std::string& special_name,
                                          bool visible/* = true*/)
{
    if (empire_id == ALL_EMPIRES || special_name.empty() || object_id == INVALID_OBJECT_ID)
        return;
    //auto obj = GetUniverseObject(object_id);
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
    std::map<int, std::map<std::pair<double, double>, float>> GetEmpiresPositionDetectionRanges() {
        std::map<int, std::map<std::pair<double, double>, float>> retval;

        for (const auto& obj : Objects()) {
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
                auto ship = std::dynamic_pointer_cast<const Ship>(obj);
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
            auto& retval_empire_pos_range = retval[object_owner_empire_id];
            auto retval_pos_it = retval_empire_pos_range.find(object_pos);
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
            for (const auto& empire_entry : Empires()) {
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
    std::map<int, std::map<std::pair<double, double>, std::vector<int>>>
        GetEmpiresPositionsPotentiallyDetectableObjects(const ObjectMap& objects, int empire_id = ALL_EMPIRES)
    {
        std::map<int, std::map<std::pair<double, double>, std::vector<int>>> retval;

        auto empire_detection_strengths = GetEmpiresDetectionStrengths(empire_id);

        // filter objects as detectors for this empire or detectable objects
        for (auto object_it = objects.const_begin();
             object_it != objects.const_end(); ++object_it)
        {
            auto obj = *object_it;
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
            for (const auto& empire_entry : empire_detection_strengths) {
                if (object_stealth <= empire_entry.second || object_stealth == 0.0f || obj->OwnedBy(empire_entry.first))
                    retval[empire_entry.first][object_pos].push_back(object_id);
            }
        }
        return retval;
    }

    /** filters set of objects at locations by which of those locations are
      * within range of a set of detectors and ranges */
    std::vector<int> FilterObjectPositionsByDetectorPositionsAndRanges(
        const std::map<std::pair<double, double>, std::vector<int>>& object_positions,
        const std::map<std::pair<double, double>, float>& detector_position_ranges)
    {
        std::vector<int> retval;
        // check each detector position and range against each object position
        for (const auto& object_position_entry : object_positions) {
            const auto& object_pos = object_position_entry.first;
            const auto& objects = object_position_entry.second;
            // search through detector positions until one is found in range
            for (const auto& detector_position_entry : detector_position_ranges) {
                // check range for this detector location for this detectables location
                float detector_range2 = detector_position_entry.second * detector_position_entry.second;
                const auto& detector_pos = detector_position_entry.first;
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
                                           const std::map<int, std::set<int>>& empire_known_destroyed_object_ids)
    {
        if (empire_id == ALL_EMPIRES)
            return;
        for (auto it = object_ids.begin(); it != object_ids.end();) {
            int object_id = *it;
            auto obj_it = empire_known_destroyed_object_ids.find(object_id);
            if (obj_it == empire_known_destroyed_object_ids.end()) {
                ++it;
                continue;
            }
            const std::set<int>& empires_that_know = obj_it->second;
            if (!empires_that_know.count(empire_id)) {
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
        const std::map<int, std::map<std::pair<double, double>, float>>&
            empire_location_detection_ranges,
        const ObjectMap& objects)
    {
        Universe& universe = GetUniverse();

        for (const auto& detecting_empire_entry : empire_location_detection_ranges) {
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
            const auto& detector_position_ranges = detecting_empire_entry.second;

            // for each field, try to find a detector position in range for this empire
            for (auto& field : objects.FindObjects<Field>()) {
                if (field->GetMeter(METER_STEALTH)->Current() > detection_strength)
                    continue;
                double field_size = field->GetMeter(METER_SIZE)->Current();
                const std::pair<double, double> object_pos(field->X(), field->Y());

                // search through detector positions until one is found in range
                for (const auto& detector_position_entry : detector_position_ranges) {
                    // check range for this detector location, for field of this
                    // size, against distance between field and detector
                    float detector_range = detector_position_entry.second;
                    const auto& detector_pos = detector_position_entry.first;
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
        const std::map<int, std::map<std::pair<double, double>, float>>&
            empire_location_detection_ranges,
        const std::map<int, std::map<std::pair<double, double>, std::vector<int>>>&
            empire_location_potentially_detectable_objects)
    {
        Universe& universe = GetUniverse();

        for (const auto& detecting_empire_entry : empire_location_detection_ranges) {
            int detecting_empire_id = detecting_empire_entry.first;
            // get empire's locations of detection ability
            const auto& detector_position_ranges = detecting_empire_entry.second;
            // for this empire, get objects it could potentially detect
            const auto empire_detectable_objects_it =
                empire_location_potentially_detectable_objects.find(detecting_empire_id);
            if (empire_detectable_objects_it == empire_location_potentially_detectable_objects.end())
                continue;   // empire can't detect anything!
            const auto& detectable_position_objects = empire_detectable_objects_it->second;
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
        for (const auto& obj : Objects()) {
            if (obj->Unowned())
                continue;
            universe.SetEmpireObjectVisibility(obj->Owner(), obj->ID(), VIS_FULL_VISIBILITY);
        }
    }

    /** sets all objects visible to all empires */
    void SetAllObjectsVisibleToAllEmpires() {
        Universe& universe = GetUniverse();
        // set every object visible to all empires
        for (auto obj_it = Objects().const_begin();
             obj_it != Objects().const_end(); ++obj_it)
        {
            for (auto& empire_entry : Empires()) {
                // objects
                universe.SetEmpireObjectVisibility(empire_entry.first, obj_it->ID(), VIS_FULL_VISIBILITY);
                // specials on objects
                for (const auto& special_entry : obj_it->Specials()) {
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
        std::map<int, std::set<int>> empires_systems_with_owned_objects;
        // get systems where empires have owned objects
        for (auto it = objects.const_begin(); it != objects.const_end(); ++it) {
            auto obj = *it;
            if (obj->Unowned() || obj->SystemID() == INVALID_OBJECT_ID)
                continue;
            empires_systems_with_owned_objects[obj->Owner()].insert(obj->SystemID());
        }

        // set system visibility
        for (const auto& empire_entry : empires_systems_with_owned_objects) {
            int empire_id = empire_entry.first;

            for (int system_id : empire_entry.second) {
                universe.SetEmpireObjectVisibility(empire_id, system_id, VIS_PARTIAL_VISIBILITY);
            }
        }

        // get planets, check their locations...
        for (const auto& planet : objects.FindObjects<Planet>()) {
            int system_id = planet->SystemID();
            if (system_id == INVALID_OBJECT_ID)
                continue;

            int planet_id = planet->ID();
            for (const auto& empire_entry : empires_systems_with_owned_objects) {
                int empire_id = empire_entry.first;
                const auto& empire_systems = empire_entry.second;
                if (!empire_systems.count(system_id))
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
        for (auto container_object_it = objects.const_begin();
             container_object_it != objects.const_end(); ++container_object_it)
        {
            int container_obj_id = container_object_it->ID();

            // get container object
            auto container_obj = *container_object_it;
            if (!container_obj)
                continue;   // shouldn't be necessary, but I like to be safe...

            // check if container object is a fleet, for special case later...
            bool container_fleet = container_obj->ObjectType() == OBJ_FLEET;

            //DebugLogger() << "Container object " << container_obj->Name() << " (" << container_obj->ID() << ")";

            // for each contained object within container
            for (int contained_obj_id : container_obj->ContainedObjectIDs()) {
                //DebugLogger() << " ... contained object (" << contained_obj_id << ")";

                // for each empire with a visibility map
                for (auto& empire_entry : empire_object_visibility) {
                    auto& vis_map = empire_entry.second;

                    //DebugLogger() << " ... ... empire id " << empire_entry.first;

                    // find current empire's visibility entry for current container object
                    auto container_vis_it = vis_map.find(container_obj_id);
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
                    auto contained_vis_it = vis_map.find(contained_obj_id);
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
        for (auto& system : objects.FindObjects<System>()) {
            int system_id = system->ID();

            // for each empire with a visibility map
            for (auto& empire_entry : empire_object_visibility) {
                auto& vis_map = empire_entry.second;

                // find current system's visibility
                auto system_vis_it = vis_map.find(system_id);
                if (system_vis_it == vis_map.end())
                    continue;

                // skip systems that aren't at least partially visible; they can't propagate visibility along starlanes
                Visibility system_vis = system_vis_it->second;
                if (system_vis <= VIS_BASIC_VISIBILITY)
                    continue;

                // get all starlanes emanating from this system, and loop through them
                for (auto& lane : system->StarlanesWormholes()) {
                    bool is_wormhole = lane.second;
                    if (is_wormhole)
                        continue;

                    // find entry for system on other end of starlane in visibility
                    // map, and upgrade to basic visibility if not already at that
                    // leve, so that starlanes will be visible if either system it
                    // ends at is partially visible or better
                    int lane_end_sys_id = lane.first;
                    auto lane_end_vis_it = vis_map.find(lane_end_sys_id);
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
        for (auto& obj : objects.FindObjects(MovingFleetVisitor())) {
            if (obj->Unowned() || obj->SystemID() == INVALID_OBJECT_ID || obj->ObjectType() != OBJ_FLEET)
                continue;
            auto fleet = std::dynamic_pointer_cast<const Fleet>(obj);
            if (!fleet)
                continue;

            int prev = fleet->PreviousSystemID();
            int next = fleet->NextSystemID();

            // ensure fleet's owner has at least basic visibility of the next
            // and previous systems on the fleet's path
            auto& vis_map = empire_object_visibility[fleet->Owner()];

            auto system_vis_it = vis_map.find(prev);
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
        for (auto& empire_entry : Empires()) {
            int empire_id = empire_entry.first;
            auto& obj_vis_map = empire_object_visibility[empire_id];
            auto& obj_specials_map = empire_object_visible_specials[empire_id];

            const Empire* empire = empire_entry.second;
            const Meter* detection_meter = empire->GetMeter("METER_DETECTION_STRENGTH");
            if (!detection_meter)
                continue;
            float detection_strength = detection_meter->Current();

            // every object empire has visibility of might have specials
            for (auto& obj_entry : obj_vis_map) {
                if (obj_entry.second <= VIS_NO_VISIBILITY)
                    continue;

                int object_id = obj_entry.first;
                auto obj = objects.Object(object_id);
                if (!obj)
                    continue;

                if (obj->Specials().empty())
                    continue;

                auto& visible_specials = obj_specials_map[object_id];

                // check all object's specials.
                for (const auto& special_entry : obj->Specials()) {
                    const Special* special = GetSpecial(special_entry.first);
                    if (!special)
                        continue;

                    float stealth = 0.0f;
                    const auto special_stealth = special->Stealth();
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
        auto input_eov_copy = empire_object_visibility;
        // unused variable auto input_eovs_copy = empire_object_visible_specials;
        Universe& universe = GetUniverse();

        for (auto& empire_entry : Empires()) {
            int empire_id = empire_entry.first;
            // output maps for this empire
            auto& obj_vis_map = empire_object_visibility[empire_id];
            // unused variable Universe::ObjectSpecialsMap& obj_specials_map = empire_object_visible_specials[empire_id];

            for (auto allied_empire_id : Empires().GetEmpireIDsWithDiplomaticStatusWithEmpire(empire_id, DIPLO_ALLIED)) {
                if (empire_id == allied_empire_id) {
                    ErrorLogger() << "ShareVisbilitiesBetweenAllies : Empire apparent allied with itself!";
                    continue;
                }

                // input maps for this ally empire
                auto& allied_obj_vis_map = input_eov_copy[allied_empire_id];
                // unused variable Universe::ObjectSpecialsMap& allied_obj_specials_map = input_eovs_copy[allied_empire_id];

                // add allied visibilities to outer-loop empire visibilities
                // whenever the ally has better visibility of an object
                // (will do the reverse in another loop iteration)
                for (auto const& allied_obj_id_vis_pair : allied_obj_vis_map) {
                    int obj_id = allied_obj_id_vis_pair.first;
                    Visibility allied_vis = allied_obj_id_vis_pair.second;
                    auto it = obj_vis_map.find(obj_id);
                    if (it == obj_vis_map.end() || it->second < allied_vis) {
                        obj_vis_map[obj_id] = allied_vis;
                        if (allied_vis < VIS_PARTIAL_VISIBILITY)
                            continue;
                        if (auto ship = GetShip(obj_id))
                            universe.SetEmpireKnowledgeOfShipDesign(ship->DesignID(), empire_id);
                    }
                }
            }
        }
    }
}

void Universe::UpdateEmpireObjectVisibilities() {
    // ensure Universe knows empires have knowledge of designs the empire is specifically remembering
    for (auto& empire_entry : Empires()) {
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

    auto empire_position_detection_ranges = GetEmpiresPositionDetectionRanges();

    auto empire_position_potentially_detectable_objects =
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
    for (const auto& full_object : m_objects) {
        if (!full_object) {
            ErrorLogger() << "UpdateEmpireLatestKnownObjectsAndVisibilityTurns found null object in m_objects";
            continue;
        }
        int object_id = full_object->ID();

        // for each empire with a visibility map
        for (auto& empire_entry : m_empire_object_visibility) {
            // can empire see object?
            const auto& vis_map = empire_entry.second;    // stores level of visibility empire has for each object it can detect this turn
            auto vis_it = vis_map.find(object_id);
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
            if (auto known_obj = known_object_map.Object(object_id)) {
                known_obj->Copy(full_object, empire_id);                    // already a stored version of this object for this empire.  update it, limited by visibility this empire has for this object this turn
            } else {
                if (auto new_obj = std::shared_ptr<UniverseObject>(full_object->Clone(empire_id)))    // no previously-recorded version of this object for this empire.  create a new one, copying only the information limtied by visibility, leaving the rest as default values
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

    const auto empire_location_detection_ranges = GetEmpiresPositionDetectionRanges();

    for (const auto& empire_entry : m_empire_latest_known_objects) {
        int empire_id = empire_entry.first;
        const ObjectMap& latest_known_objects = empire_entry.second;
        const ObjectVisibilityMap& vis_map = m_empire_object_visibility[empire_id];
        std::set<int>& stale_set = m_empire_stale_knowledge_object_ids[empire_id];
        const std::set<int>& destroyed_set = m_empire_known_destroyed_object_ids[empire_id];

        // remove stale marking for any known destroyed or currently visible objects
        for (auto stale_it = stale_set.begin(); stale_it != stale_set.end();) {
            int object_id = *stale_it;
            if (vis_map.count(object_id) || destroyed_set.count(object_id))
                stale_it = stale_set.erase(stale_it);
            else
                ++stale_it;
        }


        // get empire latest known objects that are potentially detectable
        auto empires_latest_known_objects_that_should_be_detectable =
            GetEmpiresPositionsPotentiallyDetectableObjects(latest_known_objects, empire_id);
        auto& empire_latest_known_should_be_still_detectable_objects =
            empires_latest_known_objects_that_should_be_detectable[empire_id];


        // get empire detection ranges
        auto empire_detectors_it = empire_location_detection_ranges.find(empire_id);
        if (empire_detectors_it == empire_location_detection_ranges.end())
            continue;
        const auto& empire_detector_positions_ranges = empire_detectors_it->second;


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
            auto vis_it = vis_map.find(object_id);
            if (vis_it == vis_map.end() || vis_it->second < VIS_BASIC_VISIBILITY) {
                // object not visible even though the latest known info about it
                // for this empire suggests it should be.  info is stale.
                stale_set.insert(object_id);
            }
        }


        // fleets that are not visible and that contain no ships or only stale ships are stale
        for (auto obj_it = latest_known_objects.const_begin();
             obj_it != latest_known_objects.const_end(); ++obj_it)
        {
            if (obj_it->ObjectType() != OBJ_FLEET)
                continue;
            if (obj_it->GetVisibility(empire_id) >= VIS_BASIC_VISIBILITY)
                continue;
            auto fleet = std::dynamic_pointer_cast<const Fleet>(*obj_it);
            if (!fleet)
                continue;
            int fleet_id = obj_it->ID();

            // destroyed? not stale
            if (destroyed_set.count(fleet_id)) {
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
                auto ship = latest_known_objects.Object<Ship>(ship_id);

                // if ship doesn't think it's in this fleet, doesn't count.
                if (!ship || ship->FleetID() != fleet_id)
                    continue;

                // if ship is destroyed, doesn't count
                if (destroyed_set.count(ship_id))
                    continue;

                // is contained ship visible? If so, fleet is not stale.
                auto vis_it = vis_map.find(ship_id);
                if (vis_it != vis_map.end() && vis_it->second > VIS_NO_VISIBILITY) {
                    fleet_stale = false;
                    break;
                }

                // is contained ship not visible and not stale? if so, fleet is not stale
                if (!stale_set.count(ship_id)) {
                    fleet_stale = false;
                    break;
                }
            }
            if (fleet_stale)
                stale_set.insert(fleet_id);
        }

        //for (int stale_id : stale_set) {
        //    auto obj = latest_known_objects.Object(stale_id);
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
    if (ship_design_id == INVALID_DESIGN_ID) {
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
    auto obj = m_objects.Object(object_id);
    if (!obj) {
        ErrorLogger() << "Universe::Destroy called for nonexistant object with id: " << object_id;
        return;
    }

    m_destroyed_object_ids.insert(object_id);

    if (update_destroyed_object_knowers) {
        // record empires that know this object has been destroyed
        for (auto& empire_entry : Empires()) {
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

    auto obj = m_objects.Object(object_id);
    if (!obj) {
        DebugLogger() << "Universe::RecursiveDestroy asked to destroy nonexistant object with id " << object_id;
        return retval;
    }

    auto system = GetSystem(obj->SystemID());

    if (auto ship = std::dynamic_pointer_cast<Ship>(obj)) {
        // if a ship is being deleted, and it is the last ship in its fleet, then the empty fleet should also be deleted
        auto fleet = GetFleet(ship->FleetID());
        if (fleet) {
            fleet->RemoveShips({ship->ID()});
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

    } else if (auto obj_fleet = std::dynamic_pointer_cast<Fleet>(obj)) {
        for (int ship_id : obj_fleet->ShipIDs()) {
            if (system)
                system->Remove(ship_id);
            Destroy(ship_id);
            retval.insert(ship_id);
        }
        if (system)
            system->Remove(object_id);
        Destroy(object_id);
        retval.insert(object_id);

    } else if (auto obj_planet = std::dynamic_pointer_cast<Planet>(obj)) {
        for (int building_id : obj_planet->BuildingIDs()) {
            if (system)
                system->Remove(building_id);
            Destroy(building_id);
            retval.insert(building_id);
        }
        if (system)
            system->Remove(object_id);
        Destroy(object_id);
        retval.insert(object_id);

    } else if (auto obj_system = std::dynamic_pointer_cast<System>(obj)) {
        // destroy all objects in system
        for (int system_id : obj_system->ObjectIDs()) {
            Destroy(system_id);
            retval.insert(system_id);
        }

        // remove any starlane connections to this system
        int this_sys_id = obj_system->ID();
        for (auto& sys : m_objects.FindObjects<System>()) {
            sys->RemoveStarlane(this_sys_id);
        }

        // remove fleets / ships moving along destroyed starlane
        for (auto& fleet : m_objects.FindObjects<Fleet>()) {
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

    } else if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
        auto planet = GetPlanet(building->PlanetID());
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
    auto obj = m_objects.Object(object_id);
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
    if (m_marked_destroyed.count(object_id))
        return;
    m_marked_destroyed[object_id].insert(source_object_id);
}

void Universe::InitializeSystemGraph(int for_empire_id) {
    auto system_ids = ::EmpireKnownObjects(for_empire_id).FindObjectIDs<System>();
    std::vector<std::shared_ptr<const System>> systems;
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
    for (const auto& empire_entry : Empires()) {
        if (empire_entry.second->Eliminated())
            continue;
        auto source = empire_entry.second->Source();
        if (!source) {
            ErrorLogger() << "Universe::UpdateStatRecords() unable to find source for empire, id = "
                          <<  empire_entry.second->EmpireID();
            continue;
        }
        empire_sources[empire_entry.first] = source;
    }

    // process each stat
    for (const auto& stat_entry : EmpireStats()) {
        const std::string& stat_name = stat_entry.first;

        const auto& value_ref = stat_entry.second;
        if (!value_ref)
            continue;

        auto& stat_records = m_stat_records[stat_name];

        // calculate stat for each empire, store in records for current turn
        for (const auto& entry : empire_sources) {
            int empire_id = entry.first;

            if (value_ref->SourceInvariant()) {
                stat_records[empire_id][current_turn] = value_ref->Eval();
            } else if (entry.second) {
                stat_records[empire_id][current_turn] = value_ref->Eval(ScriptingContext(entry.second));
            }
        }
    }
}

void Universe::GetShipDesignsToSerialize(ShipDesignMap& designs_to_serialize,
                                         int encoding_empire) const
{
    if (encoding_empire == ALL_EMPIRES) {
        designs_to_serialize = m_ship_designs;
    } else {
        designs_to_serialize.clear();

        // add generic monster ship designs so they always appear in players' pedias
        for (const auto& ship_design_entry : m_ship_designs) {
            ShipDesign* design = ship_design_entry.second;
            if (design->IsMonster() && design->DesignedByEmpire() == ALL_EMPIRES)
                designs_to_serialize[design->ID()] = design;
        }

        // get empire's known ship designs
        auto it = m_empire_known_ship_design_ids.find(encoding_empire);
        if (it == m_empire_known_ship_design_ids.end())
            return; // no known designs to serialize

        const std::set<int>& empire_designs = it->second;

        // add all ship designs of ships this empire knows about
        for (int design_id : empire_designs) {
            auto universe_design_it = m_ship_designs.find(design_id);
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
        auto it = m_empire_latest_known_objects.find(encoding_empire);
        if (it == m_empire_latest_known_objects.end())
            return;                 // empire has no object knowledge, so there is nothing to send

        //the empire_latest_known_objects are already processed for visibility, so can be copied streamlined
        objects.CopyForSerialize(it->second);

        auto destroyed_ids_it = m_empire_known_destroyed_object_ids.find(encoding_empire);
        bool map_avail = (destroyed_ids_it != m_empire_known_destroyed_object_ids.end());
        const auto& destroyed_object_ids = map_avail ?
            destroyed_ids_it->second : std::set<int>();

        objects.AuditContainment(destroyed_object_ids);
    }
}

void Universe::GetDestroyedObjectsToSerialize(std::set<int>& destroyed_object_ids,
                                              int encoding_empire) const
{
    if (&destroyed_object_ids == &m_destroyed_object_ids)
        return;

    if (encoding_empire == ALL_EMPIRES) {
        // all destroyed objects
        destroyed_object_ids = m_destroyed_object_ids;
    } else {
        destroyed_object_ids.clear();
        // get empire's known destroyed objects
        auto it = m_empire_known_destroyed_object_ids.find(encoding_empire);
        if (it != m_empire_known_destroyed_object_ids.end())
            destroyed_object_ids = it->second;
    }
}

void Universe::GetEmpireKnownObjectsToSerialize(EmpireObjectMap& empire_latest_known_objects,
                                                int encoding_empire) const
{
    if (&empire_latest_known_objects == &m_empire_latest_known_objects)
        return;

    DebugLogger() << "GetEmpireKnownObjectsToSerialize";

    for (auto& entry : empire_latest_known_objects)
        entry.second.Clear();

    empire_latest_known_objects.clear();

    if (!ENABLE_VISIBILITY_EMPIRE_MEMORY)
        return;

    if (encoding_empire == ALL_EMPIRES) {
        // copy all ObjectMaps' contents
        for (const auto& entry : m_empire_latest_known_objects) {
            int empire_id = entry.first;
            const ObjectMap& map = entry.second;
            //the maps in m_empire_latest_known_objects are already processed for visibility, so can be copied fully
            empire_latest_known_objects[empire_id].CopyForSerialize(map);
        }
        return;
    }
}

void Universe::GetEmpireObjectVisibilityMap(EmpireObjectVisibilityMap& empire_object_visibility,
                                            int encoding_empire) const
{
    if (encoding_empire == ALL_EMPIRES) {
        empire_object_visibility = m_empire_object_visibility;
        return;
    }

    // include just requested empire's visibility for each object it has better
    // than no visibility of.  TODO: include what requested empire knows about
    // other empires' visibilites of objects
    empire_object_visibility.clear();
    for (auto it = m_objects.const_begin(); it != m_objects.const_end(); ++it) {
        int object_id = it->ID();
        Visibility vis = GetObjectVisibilityByEmpire(object_id, encoding_empire);
        if (vis > VIS_NO_VISIBILITY)
            empire_object_visibility[encoding_empire][object_id] = vis;
    }
}

void Universe::GetEmpireObjectVisibilityTurnMap(EmpireObjectVisibilityTurnMap& empire_object_visibility_turns,
                                                int encoding_empire) const
{
    if (encoding_empire == ALL_EMPIRES) {
        empire_object_visibility_turns = m_empire_object_visibility_turns;
        return;
    }

    // include just requested empire's visibility turn information
    empire_object_visibility_turns.clear();
    auto it = m_empire_object_visibility_turns.find(encoding_empire);
    if (it != m_empire_object_visibility_turns.end())
        empire_object_visibility_turns[encoding_empire] = it->second;
}

void Universe::GetEmpireKnownDestroyedObjects(ObjectKnowledgeMap& empire_known_destroyed_object_ids,
                                              int encoding_empire) const
{
    if (&empire_known_destroyed_object_ids == &m_empire_known_destroyed_object_ids)
        return;

    if (encoding_empire == ALL_EMPIRES) {
        empire_known_destroyed_object_ids = m_empire_known_destroyed_object_ids;
        return;
    }

    empire_known_destroyed_object_ids.clear();

    // copy info about what encoding empire knows
    auto it = m_empire_known_destroyed_object_ids.find(encoding_empire);
    if (it != m_empire_known_destroyed_object_ids.end())
        empire_known_destroyed_object_ids[encoding_empire] = it->second;
}

void Universe::GetEmpireStaleKnowledgeObjects(ObjectKnowledgeMap& empire_stale_knowledge_object_ids,
                                              int encoding_empire) const
{
    if (&empire_stale_knowledge_object_ids == &m_empire_stale_knowledge_object_ids)
        return;

    if (encoding_empire == ALL_EMPIRES) {
        empire_stale_knowledge_object_ids = m_empire_stale_knowledge_object_ids;
        return;
    }

    empire_stale_knowledge_object_ids.clear();

    // copy stale data for this empire
    auto it = m_empire_stale_knowledge_object_ids.find(encoding_empire);
    if (it != m_empire_stale_knowledge_object_ids.end())
        empire_stale_knowledge_object_ids[encoding_empire] = it->second;
}

std::map<std::string, unsigned int> CheckSumContent() {
    std::map<std::string, unsigned int> checksums;

    // add entries for various content managers...
    checksums["BuildingTypeManager"] = GetBuildingTypeManager().GetCheckSum();
    checksums["Encyclopedia"] = GetEncyclopedia().GetCheckSum();
    checksums["FieldTypeManager"] = GetFieldTypeManager().GetCheckSum();
    checksums["HullTypeManager"] = GetHullTypeManager().GetCheckSum();
    checksums["PartTypeManager"] = GetPartTypeManager().GetCheckSum();
    checksums["PredefinedShipDesignManager"] = GetPredefinedShipDesignManager().GetCheckSum();
    checksums["SpeciesManager"] = GetSpeciesManager().GetCheckSum();
    checksums["TechManager"] = GetTechManager().GetCheckSum();

    return checksums;
}
