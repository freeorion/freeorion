#include "ShipDesign.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/ParserUtil.h"
#include "../util/OptionsDB.h"
#include "Condition.h"

#include <fstream>

namespace {
    struct store_part_type_impl {
        template <class T1, class T2>
        struct result {typedef void type;};
        template <class T>
        void operator()(std::map<std::string, PartType*>& part_types, const T& part_type) const {
            if (part_types.find(part_type->Name()) != part_types.end()) {
                std::string error_str = "ERROR: More than one ship part in ship_parts.txt has the name " + part_type->Name();
                throw std::runtime_error(error_str.c_str());
            }
            part_types[part_type->Name()] = part_type;
        }
    };

    const phoenix::function<store_part_type_impl> store_part_type_;

    struct store_hull_type_impl {
        template <class T1, class T2>
        struct result {typedef void type;};
        template <class T>
        void operator()(std::map<std::string, HullType*>& hull_types, const T& hull_type) const {
            if (hull_types.find(hull_type->Name()) != hull_types.end()) {
                std::string error_str = "ERROR: More than one ship hull in ship_hulls.txt has the name " + hull_type->Name();
                throw std::runtime_error(error_str.c_str());
            }
            hull_types[hull_type->Name()] = hull_type;
        }
    };

    const phoenix::function<store_hull_type_impl> store_hull_type_;
}

////////////////////////////////////////////////
// Free Functions                             //
////////////////////////////////////////////////
const PartTypeManager& GetPartTypeManager() {
    return PartTypeManager::GetPartTypeManager();
}

const PartType* GetPartType(const std::string& name) {
    return GetPartTypeManager().GetPartType(name);
}

const HullTypeManager& GetHullTypeManager() {
    return HullTypeManager::GetHullTypeManager();
}

const HullType* GetHullType(const std::string& name) {
    return GetHullTypeManager().GetHullType(name);
}

const ShipDesign* GetShipDesign(int ship_design_id) {
    return GetUniverse().GetShipDesign(ship_design_id);
}


/////////////////////////////////////
// PartTypeManager                 //
/////////////////////////////////////
// static
PartTypeManager* PartTypeManager::s_instance = 0;

PartTypeManager::PartTypeManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one PartTypeManager.");
    s_instance = this;

    std::string settings_dir = GetOptionsDB().Get<std::string>("settings-dir");
    if (!settings_dir.empty() && settings_dir[settings_dir.size() - 1] != '/')
        settings_dir += '/';
    std::string filename = settings_dir + "ship_parts.txt";
    std::ifstream ifs(filename.c_str());
    
    std::string input;
    std::getline(ifs, input, '\0');
    ifs.close();
    using namespace boost::spirit;
    using namespace phoenix;
    parse_info<const char*> result =
        parse(input.c_str(),
              as_lower_d[*part_p[store_part_type_(var(m_parts), arg1)]]
              >> end_p,
              skip_p);
    if (!result.full)
        ReportError(std::cerr, input.c_str(), result);
}

PartTypeManager::~PartTypeManager()
{
    for (std::map<std::string, PartType*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
        delete it->second;
    }
}

const PartType* PartTypeManager::GetPartType(const std::string& name) const {
    std::map<std::string, PartType*>::const_iterator it = m_parts.find(name);
    return it != m_parts.end() ? it->second : 0;
}

const PartTypeManager& PartTypeManager::GetPartTypeManager() {
    static PartTypeManager manager;
    return manager;
}

PartTypeManager::iterator PartTypeManager::begin() const {
    return m_parts.begin();
}

PartTypeManager::iterator PartTypeManager::end() const {
    return m_parts.end();
}


////////////////////////////////////////////////
// PartType
////////////////////////////////////////////////
PartType::PartType() :
    m_name("invalid part type"),
    m_description("indescribable"),
    m_class(INVALID_SHIP_PART_CLASS),
    m_power(1.0),
    m_cost(1.0),
    m_build_time(1),
    m_mountable_slot_types(),
    m_location(0),
    m_effects(0),
    m_graphic("")
{}

PartType::PartType(const std::string& name, const std::string& description, ShipPartClass part_class,
                   double power, double cost, int build_time,
                   std::vector<ShipSlotType> mountable_slot_types, const Condition::ConditionBase* location,
                   const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects,
                   const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_class(part_class),
    m_power(power),
    m_cost(cost),
    m_build_time(build_time),
    m_mountable_slot_types(mountable_slot_types),
    m_location(location),
    m_effects(effects),
    m_graphic(graphic)
{}

PartType::~PartType()
{ delete m_location; }

const std::string& PartType::Name() const {
    return m_name;
}

const std::string& PartType::Description() const {
    return m_description;
}

ShipPartClass PartType::Class() const {
    return m_class;
}

double PartType::Power() const {
    return m_power;
}

bool PartType::CanMountInSlotType(ShipSlotType slot_type) const {
    if (INVALID_SHIP_SLOT_TYPE == slot_type)
        return false;
    for (std::vector<ShipSlotType>::const_iterator it = m_mountable_slot_types.begin(); it != m_mountable_slot_types.end(); ++it)
        if (*it == slot_type)
            return true;
    return false;
}

double PartType::Cost() const {
    return m_cost;
}

int PartType::BuildTime() const {
    return m_build_time;
}

const std::string& PartType::Graphic() const {
    return m_graphic;
}

const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& PartType::Effects() const {
    return m_effects;
}

const Condition::ConditionBase* PartType::Location() const {
    return m_location;
}


////////////////////////////////////////////////
// HullType
////////////////////////////////////////////////
HullType::HullType() :
    m_name("generic hull type"),
    m_description("indescribable"),
    m_speed(1.0),
    m_cost(1.0),
    m_build_time(1),
    m_slots(),
    m_location(0),
    m_effects(0),
    m_graphic("")
{}

HullType::HullType(const std::string& name, const std::string& description, double speed, double cost,
                   int build_time, const std::vector<Slot>& slots,
                   const Condition::ConditionBase* location,
                   const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects,
                   const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_speed(speed),
    m_cost(cost),
    m_build_time(build_time),
    m_slots(slots),
    m_location(location),
    m_effects(effects),
    m_graphic(graphic)
{}

HullType::~HullType()
{ delete m_location; }

const std::string& HullType::Name() const {
    return m_name;
}

const std::string& HullType::Description() const {
    return m_description;
}

double HullType::Speed() const {
    return m_speed;
}

double HullType::Cost() const {
    return m_cost;
}

int HullType::BuildTime() const {
    return m_build_time;
}

unsigned int HullType::NumSlots() const {
    return m_slots.size();
}

unsigned int HullType::NumSlots(ShipSlotType slot_type) const {
    unsigned int count = 0;
    for (std::vector<Slot>::const_iterator it = m_slots.begin(); it != m_slots.end(); ++it)
        if (it->type == slot_type)
            ++count;
    return count;
}

const std::vector<HullType::Slot>& HullType::Slots() const {
    return m_slots;
}

const Condition::ConditionBase* HullType::Location() const {
    return m_location;
}

const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& HullType::Effects() const {
    return m_effects;
}

const std::string& HullType::Graphic() const {
    return m_graphic;
}


/////////////////////////////////////
// HullTypeManager                 //
/////////////////////////////////////
// static
HullTypeManager* HullTypeManager::s_instance = 0;

HullTypeManager::HullTypeManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one HullTypeManager.");
    s_instance = this;

    std::string settings_dir = GetOptionsDB().Get<std::string>("settings-dir");
    if (!settings_dir.empty() && settings_dir[settings_dir.size() - 1] != '/')
        settings_dir += '/';
    std::string filename = settings_dir + "ship_hulls.txt";
    std::ifstream ifs(filename.c_str());

    std::string input;
    std::getline(ifs, input, '\0');
    ifs.close();
    using namespace boost::spirit;
    using namespace phoenix;
    parse_info<const char*> result =
        parse(input.c_str(),
              as_lower_d[*hull_p[store_hull_type_(var(m_hulls), arg1)]]
              >> end_p,
              skip_p);
    if (!result.full)
        ReportError(std::cerr, input.c_str(), result);
}

HullTypeManager::~HullTypeManager()
{
    for (std::map<std::string, HullType*>::iterator it = m_hulls.begin(); it != m_hulls.end(); ++it) {
        delete it->second;
    }
}

const HullType* HullTypeManager::GetHullType(const std::string& name) const {
    std::map<std::string, HullType*>::const_iterator it = m_hulls.find(name);
    return it != m_hulls.end() ? it->second : 0;
}

const HullTypeManager& HullTypeManager::GetHullTypeManager()
{
    static HullTypeManager manager;
    return manager;
}

HullTypeManager::iterator HullTypeManager::begin() const {
    return m_hulls.begin();
}

HullTypeManager::iterator HullTypeManager::end() const {
    return m_hulls.end();
}


////////////////////////////////////////////////
// ShipDesign
////////////////////////////////////////////////
ShipDesign::ShipDesign() :
    m_id(UniverseObject::INVALID_OBJECT_ID),
    m_name(""),
    m_designed_by_empire_id(-1),
    m_designed_on_turn(UniverseObject::INVALID_OBJECT_AGE),
    m_hull(""),
    m_parts(),
    m_graphic(""),
    m_3D_model("")
{}

ShipDesign::ShipDesign(const std::string& name, const std::string& description, int designed_by_empire_id,
                       int designed_on_turn, const std::string& hull, const std::vector<std::string>& parts,
                       const std::string& graphic, const std::string& model) :
    m_id(UniverseObject::INVALID_OBJECT_ID),
    m_name(name),
    m_description(description),
    m_designed_by_empire_id(designed_by_empire_id),
    m_designed_on_turn(designed_on_turn),
    m_hull(hull),
    m_parts(parts),
    m_graphic(graphic),
    m_3D_model(model)
{
    if (!ValidDesign(m_hull, m_parts))
        Logger().errorStream() << "constructing an invalid ShipDesign!";
}

int ShipDesign::ID() const {
    return m_id;
}

const std::string& ShipDesign::Name() const
{
    return m_name;
}

int ShipDesign::DesignedByEmpire() const
{
    return m_designed_by_empire_id;
}

void ShipDesign::SetID(int id)
{
    m_id = id;
}

void ShipDesign::Rename(const std::string& name)
{
    m_name = name;
}

const std::string& ShipDesign::Graphic() const {
    return m_graphic;
}

const std::string& ShipDesign::Description() const
{
    return m_description;
}

int ShipDesign::DesignedOnTurn() const {
    return m_designed_on_turn;
}

double ShipDesign::StarlaneSpeed() const {
    return GetHull()->Speed();
}

double ShipDesign::BattleSpeed() const {
    return GetHull()->Speed();
}

const std::string& ShipDesign::Hull() const {
    return m_hull;
}

const HullType* ShipDesign::GetHull() const {
    return GetHullTypeManager().GetHullType(m_hull);
}

const std::vector<std::string>& ShipDesign::Parts() const {
    return m_parts;
}

std::vector<std::string> ShipDesign::Parts(ShipSlotType slot_type) const {
    std::vector<std::string> retval;

    const HullType* hull = GetHull();
    assert(hull);
    const std::vector<HullType::Slot>& slots = hull->Slots();

    unsigned int size = m_parts.size();
    assert(size == hull->NumSlots());

    // add to output vector each part that is in a slot of the indicated ShipSlotType 
    for (unsigned int i = 0; i < size; ++i)
        if (slots[i].type == slot_type)
            retval.push_back(m_parts[i]);

    return retval;
}

const std::string& ShipDesign::Model() const {
    return m_3D_model;
}

bool ShipDesign::ProductionLocation(int empire_id, int location_id) const {
    Condition::ObjectSet locations;
    Condition::ObjectSet non_locations;

    Universe& universe = GetUniverse();

    UniverseObject* loc = universe.Object(location_id);
    if (!loc) return false;

    Empire * empire = Empires().Lookup(empire_id);
    if (!empire) {
        Logger().debugStream() << "ShipDesign::ProductionLocation: Unable to get pointer to empire " << empire_id;
        return false;
    }

    UniverseObject * source = universe.Object(empire->CapitolID());
    if (!source) return false;

    locations.insert(loc);

    // apply hull location conditions to potential location
    const HullType* hull = GetHull();
    if (!hull)
        throw std::runtime_error("ShipDesign couldn't get its own hull...?");
    hull->Location()->Eval(source, locations, non_locations, Condition::TARGETS);
    if (locations.empty())
        return false;

    // apply external and internal parts' location conditions to potential location
    for (std::vector<std::string>::const_iterator part_it = m_parts.begin(); part_it != m_parts.end(); ++part_it) {
        std::string part_name = *part_it;
        if (part_name.empty())
            continue;       // empty slots don't limit build location

        const PartType* part = GetPartType(part_name);
        if (!part)
            throw std::runtime_error("ShipDesign couldn't get one of its own part: '" + part_name + "'");
        part->Location()->Eval(source, locations, non_locations, Condition::TARGETS);
        if (locations.empty())
            return false;
    }
    // location matched all hull and part conditions, so is a valid build location
    return true;
}

bool ShipDesign::ValidDesign(const std::string& hull, const std::vector<std::string>& parts) {
    // ensure hull type exists and has exactly enough slots for passed parts
    const HullType* hull_type = GetHullTypeManager().GetHullType(hull);
    if (!hull_type)
        return false;

    unsigned int size = parts.size();
    if (size != hull_type->NumSlots())
        return false;

    const std::vector<HullType::Slot>& slots = hull_type->Slots();

    // ensure all passed parts can be mounted in slots of type they were passed for
    const PartTypeManager& part_manager = GetPartTypeManager();
    for (unsigned int i = 0; i < size; ++i) {
        const std::string& part_name = parts[i];
        if (part_name.empty())
            continue;   // if part slot is empty, ignore - doesn't invalidate design

        const PartType* part = part_manager.GetPartType(part_name);
        if (!part)
            return false;

        // verify part can mount in indicated slot
        ShipSlotType slot_type = slots[i].type;
        if (!(part->CanMountInSlotType(slot_type)))
            return false;
    }

    return true;
}

bool ShipDesign::ValidDesign(const ShipDesign& design) {
    return ValidDesign(design.m_hull, design.m_parts);
}

//// TEMPORARY
double ShipDesign::Defense() const {
    // accumulate defense from defensive parts in design.
    double total_defense = 0.0;
    const PartTypeManager& part_manager = GetPartTypeManager();
    std::vector<std::string> all_parts = Parts();
    for (std::vector<std::string>::const_iterator it = all_parts.begin(); it != all_parts.end(); ++it) {
        const PartType* part = part_manager.GetPartType(*it);
        if (part && (part->Class() == PC_SHIELD || part->Class() == PC_ARMOUR))
            total_defense += part->Power();
    }
    return total_defense;
}

double ShipDesign::Speed() const {
    return GetHull()->Speed();
}

double ShipDesign::Attack() const {
    // accumulate attack power from all weapon parts in design
    const PartTypeManager& manager = GetPartTypeManager();

    double total_attack = 0.0;
    std::vector<std::string> all_parts = Parts();
    for (std::vector<std::string>::const_iterator it = all_parts.begin(); it != all_parts.end(); ++it) {
        const PartType* part = manager.GetPartType(*it);
        if (part && (part->Class() == PC_SHORT_RANGE || part->Class() == PC_MISSILES || 
                     part->Class() == PC_FIGHTERS || part->Class() == PC_POINT_DEFENSE)) {
            total_attack += part->Power();
        }
    }
    return total_attack;
}

bool ShipDesign::CanColonize() const {
    const PartTypeManager& part_manager = GetPartTypeManager();
    for (std::vector<std::string>::const_iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
        const PartType* part = part_manager.GetPartType(*it);
        if (part && part->Class() == PC_COLONY)
            return true;
    }

    // KLUDGE!!! REMOVE THIS!!!
    if (m_name == "Colony Ship")
        return true;
    // END KLUDGE!!!

    return false;
}

bool ShipDesign::IsArmed() const {
    //const PartTypeManager& part_manager = GetPartTypeManager();
    //for (std::vector<std::string>::const_iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
    //    const PartType* part = part_manager.GetPartType(*it);
    //    if (part && part->Class() == PC_COLONY)   // TODO: check if any part is a weapon
    //        return true;
    //}

    // KLUDGE!!! REMOVE THIS!!!
    return Attack() > 0;
    // END KLUDGE!!!
}

double ShipDesign::Cost() const {
    // accumulate cost from hull and all parts in design
    double total_cost = 0.0;

    const PartTypeManager& part_manager = GetPartTypeManager();
    for (std::vector<std::string>::const_iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
        const PartType* part = part_manager.GetPartType(*it);
        if (part)
            total_cost += part->Cost();
    }

    const HullTypeManager& hull_manager = GetHullTypeManager();
    const HullType* hull = hull_manager.GetHullType(m_hull);
    if (hull)
        total_cost += hull->Cost();

    return total_cost;
}

int ShipDesign::BuildTime() const {
    // accumulate time from hull and all parts in design
    int total_turns = 0;

    const PartTypeManager& part_manager = GetPartTypeManager();
    std::vector<std::string> all_parts = Parts();
    for (std::vector<std::string>::const_iterator it = all_parts.begin(); it != all_parts.end(); ++it) {
        const PartType* part = part_manager.GetPartType(*it);
        if (part)
            total_turns += part->BuildTime();
    }

    const HullTypeManager& hull_manager = GetHullTypeManager();
    const HullType* hull = hull_manager.GetHullType(m_hull);
    if (hull)
        total_turns += hull->BuildTime();

    return total_turns;
}
