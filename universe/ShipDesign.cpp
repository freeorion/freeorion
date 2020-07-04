#include "ShipDesign.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "Planet.h"
#include "Ship.h"
#include "ShipHull.h"
#include "ShipPart.h"
#include "Species.h"
#include "../focs/Condition.h"
#include "../focs/Effect.h"
#include "../focs/ScriptingContext.h"
#include "../focs/ValueRef.h"
#include "../util/AppInterface.h"
#include "../util/CheckSums.h"
#include "../util/GameRules.h"
#include "../util/i18n.h"


extern FO_COMMON_API const int INVALID_DESIGN_ID = -1;

//using boost::io::str;

namespace {
    void AddRules(GameRules& rules) {
        // makes all ships cost 1 PP and take 1 turn to produce
        rules.Add<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION",
                        "RULE_CHEAP_AND_FAST_SHIP_PRODUCTION_DESC",
                        "", false, true);
    }
    bool temp_bool = RegisterGameRules(&AddRules);

    const std::string EMPTY_STRING;
    const float ARBITRARY_LARGE_COST = 999999.9f;

    bool DesignsTheSame(const ShipDesign& one, const ShipDesign& two) {
        return (
            one.Name()              == two.Name() &&
            one.Description()       == two.Description() &&
            one.DesignedOnTurn()    == two.DesignedOnTurn() &&
            one.Hull()              == two.Hull() &&
            one.Parts()             == two.Parts() &&
            one.Icon()              == two.Icon() &&
            one.Model()             == two.Model()
        );
        // not checking that IDs are the same, since the purpose of this is to
        // check if a design that might be added to the universe (which doesn't
        // have an ID yet) is the same as one that has already been added
        // (which does have an ID)
    }
}

////////////////////////////////////////////////
// Free Functions                             //
////////////////////////////////////////////////
const ShipDesign* GetShipDesign(int ship_design_id)
{ return GetUniverse().GetShipDesign(ship_design_id); }


////////////////////////////////////////////////
// CommonParams
////////////////////////////////////////////////
CommonParams::CommonParams() {}

CommonParams::CommonParams(std::unique_ptr<focs::ValueRef<double>>&& production_cost_,
                           std::unique_ptr<focs::ValueRef<int>>&& production_time_,
                           bool producible_,
                           const std::set<std::string>& tags_,
                           std::unique_ptr<focs::Condition>&& location_,
                           std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects_,
                           ConsumptionMap<MeterType>&& production_meter_consumption_,
                           ConsumptionMap<std::string>&& production_special_consumption_,
                           std::unique_ptr<focs::Condition>&& enqueue_location_) :
    production_cost(std::move(production_cost_)),
    production_time(std::move(production_time_)),
    producible(producible_),
    production_meter_consumption(std::move(production_meter_consumption_)),
    production_special_consumption(std::move(production_special_consumption_)),
    location(std::move(location_)),
    enqueue_location(std::move(enqueue_location_)),
    effects(std::move(effects_))
{
    for (const std::string& tag : tags_)
        tags.insert(boost::to_upper_copy<std::string>(tag));
}

CommonParams::~CommonParams() {}


/////////////////////////////////////
// ParsedShipDesign     //
/////////////////////////////////////
ParsedShipDesign::ParsedShipDesign(
    std::string&& name, std::string&& description, int designed_on_turn,
    int designed_by_empire, std::string&& hull, std::vector<std::string>&& parts,
    std::string&& icon, std::string&& model, bool name_desc_in_stringtable,
    bool monster, boost::uuids::uuid uuid) :
    m_name(std::move(name)),
    m_description(std::move(description)),
    m_uuid(std::move(uuid)),
    m_designed_on_turn(designed_on_turn),
    m_designed_by_empire(designed_by_empire),
    m_hull(std::move(hull)),
    m_parts(std::move(parts)),
    m_is_monster(monster),
    m_icon(std::move(icon)),
    m_3D_model(std::move(model)),
    m_name_desc_in_stringtable(name_desc_in_stringtable)
{}

////////////////////////////////////////////////
// ShipDesign
////////////////////////////////////////////////
ShipDesign::ShipDesign() :
    m_uuid(boost::uuids::nil_generator()())
{}

ShipDesign::ShipDesign(const boost::optional<std::invalid_argument>& should_throw,
                       std::string name, std::string description,
                       int designed_on_turn, int designed_by_empire,
                       std::string hull, std::vector<std::string> parts,
                       std::string icon, std::string model,
                       bool name_desc_in_stringtable, bool monster,
                       boost::uuids::uuid uuid) :
    m_name(std::move(name)),
    m_description(std::move(description)),
    m_uuid(std::move(uuid)),
    m_designed_on_turn(designed_on_turn),
    m_designed_by_empire(designed_by_empire),
    m_hull(std::move(hull)),
    m_parts(std::move(parts)),
    m_is_monster(monster),
    m_icon(std::move(icon)),
    m_3D_model(std::move(model)),
    m_name_desc_in_stringtable(name_desc_in_stringtable)
{
    // Either force a valid design and log about it or just throw std::invalid_argument
    ForceValidDesignOrThrow(should_throw, !should_throw);
    BuildStatCaches();
}

ShipDesign::ShipDesign(const ParsedShipDesign& design) :
    ShipDesign(boost::none, design.m_name, design.m_description,
               design.m_designed_on_turn, design.m_designed_by_empire,
               design.m_hull, design.m_parts,
               design.m_icon, design.m_3D_model, design.m_name_desc_in_stringtable,
               design.m_is_monster, design.m_uuid)
{}

const std::string& ShipDesign::Name(bool stringtable_lookup /* = true */) const {
    if (m_name_desc_in_stringtable && stringtable_lookup)
        return UserString(m_name);
    else
        return m_name;
}

void ShipDesign::SetName(const std::string& name) {
    if (!name.empty() && !m_name.empty()) {
        m_name = name;
    }
}

void ShipDesign::SetUUID(const boost::uuids::uuid& uuid)
{ m_uuid = uuid; }

const std::string& ShipDesign::Description(bool stringtable_lookup /* = true */) const {
    if (m_name_desc_in_stringtable && stringtable_lookup)
        return UserString(m_description);
    else
        return m_description;
}

void ShipDesign::SetDescription(const std::string& description)
{ m_description = description; }

bool ShipDesign::ProductionCostTimeLocationInvariant() const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION"))
        return true;
    // as seen in ShipDesign::ProductionCost, the production location is passed
    // as the local candidate in the ScriptingContext

    // check hull and all parts
    if (const ShipHull* hull = GetShipHull(m_hull))
        if (!hull->ProductionCostTimeLocationInvariant())
            return false;

    for (const std::string& part_name : m_parts)
        if (const ShipPart* part = GetShipPart(part_name))
            if (!part->ProductionCostTimeLocationInvariant())
                return false;

    // if hull and all parts are invariant, so is whole design
    return true;
}

float ShipDesign::ProductionCost(int empire_id, int location_id) const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION"))
        return 1.0f;

    float cost_accumulator = 0.0f;
    if (const ShipHull* hull = GetShipHull(m_hull))
        cost_accumulator += hull->ProductionCost(empire_id, location_id, m_id);

    int part_count = 0;
    for (const std::string& part_name : m_parts) {
        if (const ShipPart* part = GetShipPart(part_name)) {
            cost_accumulator += part->ProductionCost(empire_id, location_id, m_id);
            part_count++;
        }
    }

    // Assuming no reasonable combination of parts and hull will add up to more
    // than ARBITRARY_LARGE_COST. Truncating cost here to return it to indicate
    // an uncalculable cost (ie. due to lacking a valid location object)

    return std::min(std::max(0.0f, cost_accumulator), ARBITRARY_LARGE_COST);
}

float ShipDesign::PerTurnCost(int empire_id, int location_id) const
{ return ProductionCost(empire_id, location_id) / std::max(1, ProductionTime(empire_id, location_id)); }

int ShipDesign::ProductionTime(int empire_id, int location_id) const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION"))
        return 1;

    int time_accumulator = 1;
    if (const ShipHull* hull = GetShipHull(m_hull))
        time_accumulator = std::max(time_accumulator, hull->ProductionTime(empire_id, location_id));

    for (const std::string& part_name : m_parts)
        if (const ShipPart* part = GetShipPart(part_name))
            time_accumulator = std::max(time_accumulator, part->ProductionTime(empire_id, location_id));

    // assuming that ARBITRARY_LARGE_TURNS is larger than any reasonable turns,
    // so the std::max calls will preserve it be returned

    return std::max(1, time_accumulator);
}

bool ShipDesign::CanColonize() const {
    for (const std::string& part_name : m_parts) {
        if (part_name.empty())
            continue;
        if (const ShipPart* part = GetShipPart(part_name))
            if (part->Class() == PC_COLONY)
                return true;
    }
    return false;
}

float ShipDesign::Defense() const {
    // accumulate defense from defensive parts in design.
    float total_defense = 0.0f;
    const ShipPartManager& part_manager = GetShipPartManager();
    for (const std::string& part_name : Parts()) {
        const ShipPart* part = part_manager.GetShipPart(part_name);
        if (part && (part->Class() == PC_SHIELD || part->Class() == PC_ARMOUR))
            total_defense += part->Capacity();
    }
    return total_defense;
}

float ShipDesign::Attack() const {
    // total damage against a target with the no shield.
    return AdjustedAttack(0.0f);
}

float ShipDesign::AdjustedAttack(float shield) const {
    // total damage against a target with the given shield (damage reduction)
    // assuming full load of fighters that are not destroyed during the battle
    int available_fighters = 0;
    int fighter_launch_capacity = 0;
    float fighter_damage = 0.0f;
    float direct_attack = 0.0f;

    for (const std::string& part_name : m_parts) {
        const ShipPart* part = GetShipPart(part_name);
        if (!part)
            continue;
        ShipPartClass part_class = part->Class();

        // direct weapon and fighter-related parts all handled differently...
        if (part_class == PC_DIRECT_WEAPON) {
            float part_attack = part->Capacity();
            if (part_attack > shield)
                direct_attack += (part_attack - shield)*part->SecondaryStat();  // here, secondary stat is number of shots per round
        } else if (part_class == PC_FIGHTER_HANGAR) {
            available_fighters = part->Capacity();                              // stacked meter
        } else if (part_class == PC_FIGHTER_BAY) {
            fighter_launch_capacity += part->Capacity();
            fighter_damage = part->SecondaryStat();                             // here, secondary stat is fighter damage per shot
        }
    }

    int fighter_shots = std::min(available_fighters, fighter_launch_capacity);  // how many fighters launched in bout 1
    available_fighters -= fighter_shots;
    int launched_fighters = fighter_shots;
    int num_bouts = GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS");
    int remaining_bouts = num_bouts - 2;  // no attack for first round, second round already added
    while (remaining_bouts > 0) {
        int fighters_launched_this_bout = std::min(available_fighters, fighter_launch_capacity);
        available_fighters -= fighters_launched_this_bout;
        launched_fighters += fighters_launched_this_bout;
        fighter_shots += launched_fighters;
        --remaining_bouts;
    }

    // how much damage does a fighter shot do?
    fighter_damage = std::max(0.0f, fighter_damage);

    return direct_attack + fighter_shots*fighter_damage/num_bouts;   // divide by bouts because fighter calculation is for a full combat, but direct firefor one attack
}

std::vector<std::string> ShipDesign::Parts(ShipSlotType slot_type) const {
    std::vector<std::string> retval;

    const ShipHull* hull = GetShipHullManager().GetShipHull(m_hull);
    if (!hull) {
        ErrorLogger() << "Design hull not found: " << m_hull;
        return retval;
    }
    const auto& slots = hull->Slots();

    if (m_parts.empty())
        return retval;

    // add to output vector each part that is in a slot of the indicated ShipSlotType
    for (unsigned int i = 0; i < m_parts.size(); ++i)
        if (slots[i].type == slot_type)
            retval.push_back(m_parts[i]);

    return retval;
}

std::vector<std::string> ShipDesign::Weapons() const {
    std::vector<std::string> retval;
    retval.reserve(m_parts.size());
    for (const auto& part_name : m_parts) {
        const ShipPart* part = GetShipPart(part_name);
        if (!part)
            continue;
        ShipPartClass part_class = part->Class();
        if (part_class == PC_DIRECT_WEAPON || part_class == PC_FIGHTER_BAY)
        { retval.push_back(part_name); }
    }
    return retval;
}

int ShipDesign::PartCount() const {
    int count = 0;
    for (auto& entry : m_num_part_classes)
         count += entry.second;
    return count;
}

bool ShipDesign::ProductionLocation(int empire_id, int location_id) const {
    Empire* empire = GetEmpire(empire_id);
    if (!empire) {
        DebugLogger() << "ShipDesign::ProductionLocation: Unable to get pointer to empire " << empire_id;
        return false;
    }

    // must own the production location...
    auto location = Objects().get(location_id);
    if (!location) {
        WarnLogger() << "ShipDesign::ProductionLocation unable to get location object with id " << location_id;
        return false;
    }
    if (!location->OwnedBy(empire_id))
        return false;

    auto planet = std::dynamic_pointer_cast<const Planet>(location);
    std::shared_ptr<const Ship> ship;
    if (!planet)
        ship = std::dynamic_pointer_cast<const Ship>(location);
    if (!planet && !ship)
        return false;

    // ships can only be produced by species that are not planetbound
    const std::string& species_name = planet ? planet->SpeciesName() : (ship ? ship->SpeciesName() : EMPTY_STRING);
    if (species_name.empty())
        return false;
    const Species* species = GetSpecies(species_name);
    if (!species)
        return false;

    if (!species->CanProduceShips())
        return false;
    // also, species that can't colonize can't produce colony ships
    if (this->CanColonize() && !species->CanColonize())
        return false;

    // apply hull location conditions to potential location
    const ShipHull* hull = GetShipHull(m_hull);
    if (!hull) {
        ErrorLogger() << "ShipDesign::ProductionLocation  ShipDesign couldn't get its own hull with name " << m_hull;
        return false;
    }
    // evaluate using location as the source, as it should be an object owned by this empire.
    ScriptingContext location_as_source_context(location, location);
    if (!hull->Location()->Eval(location_as_source_context, location))
        return false;

    // apply external and internal parts' location conditions to potential location
    for (const std::string& part_name : m_parts) {
        if (part_name.empty())
            continue;       // empty slots don't limit build location

        const ShipPart* part = GetShipPart(part_name);
        if (!part) {
            ErrorLogger() << "ShipDesign::ProductionLocation  ShipDesign couldn't get part with name " << part_name;
            return false;
        }
        if (!part->Location()->Eval(location_as_source_context, location))
            return false;
    }
    // location matched all hull and part conditions, so is a valid build location
    return true;
}

void ShipDesign::SetID(int id)
{ m_id = id; }

bool ShipDesign::ValidDesign(const std::string& hull, const std::vector<std::string>& parts_in) {
    auto parts = parts_in;
    return !MaybeInvalidDesign(hull, parts, true);
}

boost::optional<std::pair<std::string, std::vector<std::string>>>
ShipDesign::MaybeInvalidDesign(const std::string& hull_in,
                               std::vector<std::string>& parts_in,
                               bool produce_log)
{
    bool is_valid = true;

    auto hull = hull_in;
    auto parts = parts_in;

    // ensure hull type exists
    auto ship_hull = GetShipHullManager().GetShipHull(hull);
    if (!ship_hull) {
        is_valid = false;
        if (produce_log)
            WarnLogger() << "Invalid ShipDesign hull not found: " << hull;

        const auto hull_it = GetShipHullManager().begin();
        if (hull_it != GetShipHullManager().end()) {
            hull = hull_it->first;
            ship_hull = hull_it->second.get();
            if (produce_log)
                WarnLogger() << "Invalid ShipDesign hull falling back to: " << hull;
        } else {
            if (produce_log)
                ErrorLogger() << "Invalid ShipDesign no available hulls ";
            hull = "";
            parts.clear();
            return std::make_pair(hull, parts);
        }
    }

    // ensure hull type has at least enough slots for passed parts
    if (parts.size() > ship_hull->NumSlots()) {
        is_valid = false;
        if (produce_log)
            WarnLogger() << "Invalid ShipDesign given " << parts.size() << " parts for hull with "
                         << ship_hull->NumSlots() << " slots.  Truncating last "
                         << (parts.size() - ship_hull->NumSlots()) << " parts.";
    }

    // If parts is smaller than the full hull size pad it and the incoming parts
    if (parts.size() < ship_hull->NumSlots())
        parts_in.resize(ship_hull->NumSlots(), "");

    // Truncate or pad with "" parts.
    parts.resize(ship_hull->NumSlots(), "");

    const auto& slots = ship_hull->Slots();

    // check hull exclusions against all parts...
    const auto& hull_exclusions = ship_hull->Exclusions();
    for (auto& part_name : parts) {
        if (part_name.empty())
            continue;
        if (hull_exclusions.count(part_name)) {
            is_valid = false;
            if (produce_log)
                WarnLogger() << "Invalid ShipDesign part \"" << part_name << "\" is excluded by \""
                             << ship_hull->Name() << "\". Removing \"" << part_name <<"\"";
            part_name.clear();
        }
    }

    // check part exclusions against other parts and hull
    std::unordered_map<std::string, unsigned int> component_name_counts;
    component_name_counts[hull] = 1;
    for (auto part_name : parts)
        component_name_counts[part_name]++;
    component_name_counts.erase("");

    for (std::size_t ii = 0; ii < parts.size(); ++ii) {
        const auto part_name = parts[ii];
        // Ignore empty slots, which are valid.
        if (part_name.empty())
            continue;

        // Parts must exist...
        const auto ship_part = GetShipPart(part_name);
        if (!ship_part) {
            if (produce_log)
                WarnLogger() << "Invalid ShipDesign part \"" << part_name << "\" not found"
                             << ". Removing \"" << part_name <<"\"";
            is_valid = false;
            continue;
        }

        for (const auto& excluded : ship_part->Exclusions()) {
            // confict if a different excluded part is present, or if there are
            // two or more of a part that excludes itself
            if ((excluded == part_name && component_name_counts[excluded] > 1) ||
                (excluded != part_name && component_name_counts[excluded] > 0))
            {
                is_valid = false;
                if (produce_log)
                    WarnLogger() << "Invalid ShipDesign part " << part_name << " conflicts with \""
                                 << excluded << "\". Removing \"" << part_name <<"\"";
                continue;
            }
        }

        // verify part can mount in indicated slot
        const ShipSlotType& slot_type = slots[ii].type;

        if (!ship_part->CanMountInSlotType(slot_type)) {
            if (produce_log)
                DebugLogger() << "Invalid ShipDesign part \"" << part_name << "\" can't be mounted in "
                              << slot_type << " slot. Removing \"" << part_name <<"\"";
            is_valid = false;
            continue;
        }
    }

    if (is_valid)
        return boost::none;
    else
        return std::make_pair(hull, parts);
}

void ShipDesign::ForceValidDesignOrThrow(const boost::optional<std::invalid_argument>& should_throw,
                                         bool  produce_log)
{
    auto force_valid = MaybeInvalidDesign(m_hull, m_parts, produce_log);
    if (!force_valid)
        return;

    if (!produce_log && should_throw)
        throw std::invalid_argument("ShipDesign: Bad hull or parts");

    std::stringstream ss;

    bool no_hull_available = force_valid->first.empty();
    if (no_hull_available)
        ss << "ShipDesign has no valid hull and there are no other hulls available." << std::endl;

    ss << "Invalid ShipDesign:" << std::endl;
    ss << Dump() << std::endl;

    std::tie(m_hull, m_parts) = *force_valid;

    ss << "ShipDesign was made valid as:" << std::endl;
    ss << Dump() << std::endl;

    if (no_hull_available)
        ErrorLogger() << ss.str();
    else
        WarnLogger() << ss.str();

    if (should_throw)
        throw std::invalid_argument("ShipDesign: Bad hull or parts");
}

void ShipDesign::BuildStatCaches() {
    const ShipHull* hull = GetShipHull(m_hull);
    if (!hull) {
        ErrorLogger() << "ShipDesign::BuildStatCaches couldn't get hull with name " << m_hull;
        return;
    }

    m_producible =      hull->Producible();
    m_detection =       hull->Detection();
    m_colony_capacity = hull->ColonyCapacity();
    m_troop_capacity =  hull->TroopCapacity();
    m_stealth =         hull->Stealth();
    m_fuel =            hull->Fuel();
    m_shields =         hull->Shields();
    m_structure =       hull->Structure();
    m_speed =           hull->Speed();

    bool has_fighter_bays = false;
    bool has_fighter_hangars = false;
    bool has_armed_fighters = false;
    bool can_launch_fighters = false;

    for (const std::string& part_name : m_parts) {
        if (part_name.empty())
            continue;

        const ShipPart* part = GetShipPart(part_name);
        if (!part) {
            ErrorLogger() << "ShipDesign::BuildStatCaches couldn't get part with name " << part_name;
            continue;
        }

        if (!part->Producible())
            m_producible = false;

        ShipPartClass part_class = part->Class();

        switch (part_class) {
        case PC_DIRECT_WEAPON:
            m_has_direct_weapons = true;
            if (part->Capacity() > 0.0f)
                m_is_armed = true;
            break;
        case PC_FIGHTER_BAY:
            has_fighter_bays = true;
            if (part->Capacity() >= 1.0f)
                can_launch_fighters = true;
            break;
        case PC_FIGHTER_HANGAR:
            has_fighter_hangars = true;
            if (part->SecondaryStat() > 0.0f && part->Capacity() >= 1.0f)
                has_armed_fighters = true;
            break;
        case PC_COLONY:
            m_colony_capacity += part->Capacity();
            break;
        case PC_TROOPS:
            m_troop_capacity += part->Capacity();
            break;
        case PC_STEALTH:
            m_stealth += part->Capacity();
            break;
        case PC_SPEED:
            m_speed += part->Capacity();
            break;
        case PC_SHIELD:
            m_shields += part->Capacity();
            break;
        case PC_FUEL:
            m_fuel += part->Capacity();
            break;
        case PC_ARMOUR:
            m_structure += part->Capacity();
            break;
        case PC_DETECTION:
            m_detection += part->Capacity();
            break;
        case PC_BOMBARD:
            m_can_bombard = true;
            break;
        case PC_RESEARCH:
            m_research_generation += part->Capacity();
            break;
        case PC_INDUSTRY:
            m_industry_generation += part->Capacity();
            break;
        case PC_INFLUENCE:
            m_influence_generation += part->Capacity();
            break;
        case PC_PRODUCTION_LOCATION:
            m_is_production_location = true;
            break;
        default:
            break;
        }
        m_has_fighters = has_fighter_bays && has_fighter_hangars;
        m_is_armed = m_is_armed || (can_launch_fighters && has_armed_fighters);

        m_num_ship_parts[part_name]++;
        if (part_class > INVALID_SHIP_PART_CLASS && part_class < NUM_SHIP_PART_CLASSES)
            m_num_part_classes[part_class]++;
    }
}

std::string ShipDesign::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "ShipDesign\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "uuid = \"" + boost::uuids::to_string(m_uuid) + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";

    if (!m_name_desc_in_stringtable)
        retval += DumpIndent(ntabs+1) + "NoStringtableLookup\n";
    retval += DumpIndent(ntabs+1) + "hull = \"" + m_hull + "\"\n";
    retval += DumpIndent(ntabs+1) + "parts = ";
    if (m_parts.empty()) {
        retval += "[]\n";
    } else if (m_parts.size() == 1) {
        retval += "\"" + *m_parts.begin() + "\"\n";
    } else {
        retval += "[\n";
        for (const std::string& part_name : m_parts) {
            retval += DumpIndent(ntabs+2) + "\"" + part_name + "\"\n";
        }
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    if (!m_icon.empty())
        retval += DumpIndent(ntabs+1) + "icon = \"" + m_icon + "\"\n";
    retval += DumpIndent(ntabs+1) + "model = \"" + m_3D_model + "\"\n";
    return retval;
}

unsigned int ShipDesign::GetCheckSum() const {
    unsigned int retval{0};
    CheckSums::CheckSumCombine(retval, m_id);
    CheckSums::CheckSumCombine(retval, m_uuid);
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_designed_on_turn);
    CheckSums::CheckSumCombine(retval, m_designed_by_empire);
    CheckSums::CheckSumCombine(retval, m_hull);
    CheckSums::CheckSumCombine(retval, m_parts);
    CheckSums::CheckSumCombine(retval, m_is_monster);
    CheckSums::CheckSumCombine(retval, m_icon);
    CheckSums::CheckSumCombine(retval, m_3D_model);
    CheckSums::CheckSumCombine(retval, m_name_desc_in_stringtable);

    return retval;
}

bool operator ==(const ShipDesign& first, const ShipDesign& second) {
    if (first.Hull() != second.Hull())
        return false;

    std::map<std::string, int> first_parts;
    std::map<std::string, int> second_parts;

    // don't care if order is different, as long as the types and numbers of parts is the same
    for (const std::string& part_name : first.Parts())
    { ++first_parts[part_name]; }

    for (const std::string& part_name : second.Parts())
    { ++second_parts[part_name]; }

    return first_parts == second_parts;
}

/////////////////////////////////////
// PredefinedShipDesignManager     //
/////////////////////////////////////
// static(s)
PredefinedShipDesignManager* PredefinedShipDesignManager::s_instance = nullptr;

PredefinedShipDesignManager::PredefinedShipDesignManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one PredefinedShipDesignManager.");

    // Only update the global pointer on sucessful construction.
    s_instance = this;
}

namespace {
    void AddDesignToUniverse(std::unordered_map<std::string, int>& design_generic_ids,
                             const std::unique_ptr<ShipDesign>& design, bool monster)
    {
        if (!design)
            return;

        Universe& universe = GetUniverse();
        /* check if there already exists this same design in the universe. */
        for (auto it = universe.beginShipDesigns();
             it != universe.endShipDesigns(); ++it)
        {
            const ShipDesign* existing_design = it->second;
            if (!existing_design) {
                ErrorLogger() << "PredefinedShipDesignManager::AddShipDesignsToUniverse found an invalid design in the Universe";
                continue;
            }

            if (DesignsTheSame(*existing_design, *design)) {
                WarnLogger() << "AddShipDesignsToUniverse found an exact duplicate of ship design "
                             << design->Name() << "to be added, so is not re-adding it";
                design_generic_ids[design->Name(false)] = existing_design->ID();
                return; // design already added; don't need to do so again
            }
        }

        // duplicate design to add to Universe
        ShipDesign* copy = new ShipDesign(*design);

        bool success = universe.InsertShipDesign(copy);
        if (!success) {
            ErrorLogger() << "Empire::AddShipDesign Unable to add new design to universe";
            delete copy;
            return;
        }

        auto new_design_id = copy->ID();
        design_generic_ids[design->Name(false)] = new_design_id;
        TraceLogger() << "AddShipDesignsToUniverse added ship design "
                      << design->Name() << " to universe.";
    };
}

void PredefinedShipDesignManager::AddShipDesignsToUniverse() const {
    CheckPendingDesignsTypes();
    m_design_generic_ids.clear();

    for (const auto& uuid : m_ship_ordering)
        AddDesignToUniverse(m_design_generic_ids, m_designs.at(uuid), false);

    for (const auto& uuid : m_monster_ordering)
        AddDesignToUniverse(m_design_generic_ids, m_designs.at(uuid), true);
}

PredefinedShipDesignManager& PredefinedShipDesignManager::GetPredefinedShipDesignManager() {
    static PredefinedShipDesignManager manager;
    return manager;
}


std::vector<const ShipDesign*> PredefinedShipDesignManager::GetOrderedShipDesigns() const {
    CheckPendingDesignsTypes();
    std::vector<const ShipDesign*> retval;
    for (const auto& uuid : m_ship_ordering)
        retval.push_back(m_designs.at(uuid).get());
    return retval;
}

std::vector<const ShipDesign*> PredefinedShipDesignManager::GetOrderedMonsterDesigns() const {
    CheckPendingDesignsTypes();
    std::vector<const ShipDesign*> retval;
    for (const auto& uuid : m_monster_ordering)
        retval.push_back(m_designs.at(uuid).get());
    return retval;
}

int PredefinedShipDesignManager::GetDesignID(const std::string& name) const {
    CheckPendingDesignsTypes();
    const auto& it = m_design_generic_ids.find(name);
    if (it == m_design_generic_ids.end())
        return INVALID_DESIGN_ID;
    return it->second;
}

unsigned int PredefinedShipDesignManager::GetCheckSum() const {
    CheckPendingDesignsTypes();
    unsigned int retval{0};

    auto build_checksum = [&retval, this](const std::vector<boost::uuids::uuid>& ordering){
        for (auto const& uuid : ordering) {
            auto it = m_designs.find(uuid);
            if (it != m_designs.end())
                CheckSums::CheckSumCombine(retval, std::make_pair(it->second->Name(false), *it->second));
        }
        CheckSums::CheckSumCombine(retval, ordering.size());
    };

    build_checksum(m_ship_ordering);
    build_checksum(m_monster_ordering);

    DebugLogger() << "PredefinedShipDesignManager checksum: " << retval;
    return retval;
}


void PredefinedShipDesignManager::SetShipDesignTypes(
    Pending::Pending<ParsedShipDesignsType>&& pending_designs)
{ m_pending_designs = std::move(pending_designs); }

void PredefinedShipDesignManager::SetMonsterDesignTypes(
    Pending::Pending<ParsedShipDesignsType>&& pending_designs)
{ m_pending_monsters = std::move(pending_designs); }

namespace {
    template <typename Map1, typename Map2, typename Ordering>
    void FillDesignsOrderingAndNameTables(
        PredefinedShipDesignManager::ParsedShipDesignsType& parsed_designs,
        Map1& designs, Ordering& ordering, Map2& name_to_uuid)
    {
        // Remove the old designs
        for (const auto& name_and_uuid: name_to_uuid)
            designs.erase(name_and_uuid.second);
        name_to_uuid.clear();

        auto inconsistent_and_map_and_order_ships =
            LoadShipDesignsAndManifestOrderFromParseResults(parsed_designs);

        ordering = std::get<2>(inconsistent_and_map_and_order_ships);

        auto& disk_designs = std::get<1>(inconsistent_and_map_and_order_ships);

        for (auto& uuid_and_design : disk_designs) {
            auto& design = uuid_and_design.second.first;

            if (designs.count(design->UUID())) {
                ErrorLogger() << design->Name() << " ship design does not have a unique UUID for "
                              << "its type monster or pre-defined. "
                              << designs[design->UUID()]->Name() << " has the same UUID.";
                continue;
            }

            if (name_to_uuid.count(design->Name())) {
                ErrorLogger() << design->Name() << " ship design does not have a unique name for "
                              << "its type monster or pre-defined.";
                continue;
            }

            name_to_uuid.insert({design->Name(), design->UUID()});
            designs[design->UUID()] = std::move(design);
        }
    }

    template <typename PendingShips, typename Map1, typename Map2, typename Ordering>
    void CheckPendingAndFillDesignsOrderingAndNameTables(
        PendingShips& pending, Map1& designs, Ordering& ordering, Map2& name_to_uuid, bool are_monsters)
    {
        if (!pending)
            return;

        auto parsed = Pending::WaitForPending(pending);
        if (!parsed)
            return;

        DebugLogger() << "Populating pre-defined ships with "
                      << std::string(are_monsters ? "monster" : "ship") << " designs.";

        FillDesignsOrderingAndNameTables(
            *parsed, designs, ordering, name_to_uuid);

        // Make the monsters monstrous
        if (are_monsters)
            for (const auto& uuid : ordering)
                designs[uuid]->SetMonster(true);

        TraceLogger() << [&designs, name_to_uuid]() {
            std::stringstream ss;
            ss << "Predefined Ship Designs:";
            for (const auto& entry : name_to_uuid)
                ss << " ... " << designs[entry.second]->Name();
            return ss.str();
        }();
    }
}

void PredefinedShipDesignManager::CheckPendingDesignsTypes() const {
    CheckPendingAndFillDesignsOrderingAndNameTables(
        m_pending_designs, m_designs, m_ship_ordering, m_name_to_ship_design, false);

    CheckPendingAndFillDesignsOrderingAndNameTables(
        m_pending_monsters, m_designs, m_monster_ordering, m_name_to_monster_design, true);
 }

///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
PredefinedShipDesignManager& GetPredefinedShipDesignManager()
{ return PredefinedShipDesignManager::GetPredefinedShipDesignManager(); }

const ShipDesign* GetPredefinedShipDesign(const std::string& name)
{ return GetUniverse().GetGenericShipDesign(name); }

std::tuple<
    bool,
    std::unordered_map<boost::uuids::uuid,
                       std::pair<std::unique_ptr<ShipDesign>, boost::filesystem::path>,
                       boost::hash<boost::uuids::uuid>>,
    std::vector<boost::uuids::uuid>>
LoadShipDesignsAndManifestOrderFromParseResults(
    PredefinedShipDesignManager::ParsedShipDesignsType& designs_paths_and_ordering)
{
    std::unordered_map<boost::uuids::uuid,
                       std::pair<std::unique_ptr<ShipDesign>,
                                 boost::filesystem::path>,
                       boost::hash<boost::uuids::uuid>>  saved_designs;

    auto& designs_and_paths = designs_paths_and_ordering.first;
    auto& disk_ordering = designs_paths_and_ordering.second;

    for (auto&& design_and_path : designs_and_paths) {
        auto design = std::make_unique<ShipDesign>(*design_and_path.first);

        // If the UUID is nil this is a legacy design that needs a new UUID
        if(design->UUID() == boost::uuids::uuid{{0}}) {
            design->SetUUID(boost::uuids::random_generator()());
            DebugLogger() << "Converted legacy ship design file by adding  UUID " << design->UUID()
                          << " for name " << design->Name();
        }

        // Make sure the design is an out of universe object
        // This should not be needed.
        if(design->ID() != INVALID_OBJECT_ID) {
            design->SetID(INVALID_OBJECT_ID);
            ErrorLogger() << "Loaded ship design has an id implying it is in an ObjectMap for UUID "
                          << design->UUID() << " for name " << design->Name();
        }

        if (!saved_designs.count(design->UUID())) {
            TraceLogger() << "Added saved design UUID " << design->UUID()
                          << " with name " << design->Name();
            auto uuid = design->UUID();
            saved_designs[uuid] = std::make_pair(std::move(design), design_and_path.second);
        } else {
            WarnLogger() << "Duplicate ship design UUID " << design->UUID()
                         << " found for ship design " << design->Name()
                         << " and " << saved_designs[design->UUID()].first->Name();
        }
    }

    // Verify that all UUIDs in ordering exist
    std::vector<boost::uuids::uuid> ordering;
    bool ship_manifest_inconsistent = false;
    for (auto& uuid: disk_ordering) {
        // Skip the nil UUID.
        if(uuid == boost::uuids::uuid{{0}})
            continue;

        if (!saved_designs.count(uuid)) {
            WarnLogger() << "UUID " << uuid << " is in ship design manifest for "
                         << "a ship design that does not exist.";
            ship_manifest_inconsistent = true;
            continue;
        }
        ordering.push_back(uuid);
    }

    // Verify that every design in saved_designs is in ordering.
    if (ordering.size() != saved_designs.size()) {
        // Add any missing designs in alphabetical order to the end of the list
        std::unordered_set<boost::uuids::uuid, boost::hash<boost::uuids::uuid>>
            uuids_in_ordering{ordering.begin(), ordering.end()};
        std::map<std::string, boost::uuids::uuid> missing_uuids_sorted_by_name;
        for (auto& uuid_to_design_and_filename: saved_designs) {
            if (uuids_in_ordering.count(uuid_to_design_and_filename.first))
                continue;
            ship_manifest_inconsistent = true;
            missing_uuids_sorted_by_name.insert(
                std::make_pair(uuid_to_design_and_filename.second.first->Name(),
                               uuid_to_design_and_filename.first));
        }

        for (auto& name_and_uuid: missing_uuids_sorted_by_name) {
            WarnLogger() << "Missing ship design " << name_and_uuid.second
                         << " called " << name_and_uuid.first
                         << " added to the manifest.";
            ordering.push_back(name_and_uuid.second);
        }
    }

    return std::make_tuple(ship_manifest_inconsistent, std::move(saved_designs), ordering);
}
