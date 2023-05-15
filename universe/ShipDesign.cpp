#include "ShipDesign.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "Condition.h"
#include "Effect.h"
#include "Planet.h"
#include "ScriptingContext.h"
#include "ShipHull.h"
#include "ShipPart.h"
#include "Ship.h"
#include "Species.h"
#include "ValueRef.h"
#include "../util/AppInterface.h"
#include "../util/CheckSums.h"
#include "../util/GameRules.h"
#include "../util/i18n.h"


namespace {
    void AddRules(GameRules& rules) {
        // makes all ships cost 1 PP and take 1 turn to produce
        rules.Add<bool>(UserStringNop("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION"),
                        UserStringNop("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION_DESC"),
                        "TEST", false, true);
    }
    bool temp_bool = RegisterGameRules(&AddRules);

    const std::string EMPTY_STRING;
    constexpr float ARBITRARY_LARGE_COST = 999999.9f;

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
// CommonParams
////////////////////////////////////////////////
CommonParams::CommonParams(std::unique_ptr<ValueRef::ValueRef<double>>&& production_cost_,
                           std::unique_ptr<ValueRef::ValueRef<int>>&& production_time_,
                           bool producible_,
                           std::set<std::string>& tags_,
                           std::unique_ptr<Condition::Condition>&& location_,
                           std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects_,
                           ConsumptionMap<MeterType>&& production_meter_consumption_,
                           ConsumptionMap<std::string>&& production_special_consumption_,
                           std::unique_ptr<Condition::Condition>&& enqueue_location_) :
    production_cost(std::move(production_cost_)),
    production_time(std::move(production_time_)),
    producible(producible_),
    tags(tags_.begin(), tags_.end()),
    production_meter_consumption(std::move(production_meter_consumption_)),
    production_special_consumption(std::move(production_special_consumption_)),
    location(std::move(location_)),
    enqueue_location(std::move(enqueue_location_)),
    effects(std::move(effects_))
{
    std::transform(tags.begin(), tags.end(), tags.begin(), [](const auto& t) { return boost::to_upper_copy(t); } );
}

CommonParams::~CommonParams() = default;


/////////////////////////////////////
//       ParsedShipDesign          //
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
    m_icon(std::move(icon)),
    m_3D_model(std::move(model)),
    m_is_monster(monster),
    m_name_desc_in_stringtable(name_desc_in_stringtable)
{}


////////////////////////////////////////////////
// ShipDesign
////////////////////////////////////////////////
ShipDesign::ShipDesign() = default;

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
    m_icon(std::move(icon)),
    m_3D_model(std::move(model)),
    m_is_monster(monster),
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

const std::string& ShipDesign::Name(bool stringtable_lookup) const {
    if (m_name_desc_in_stringtable && stringtable_lookup)
        return UserString(m_name);
    else
        return m_name;
}

void ShipDesign::SetName(std::string name) noexcept {
    if (!name.empty() && !m_name.empty())
        m_name = std::move(name);
}

const std::string& ShipDesign::Description(bool stringtable_lookup) const {
    if (m_name_desc_in_stringtable && stringtable_lookup)
        return UserString(m_description);
    else
        return m_description;
}

void ShipDesign::SetDescription(const std::string& description) // TODO: pass by value with move
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

float ShipDesign::ProductionCost(int empire_id, int location_id, const ScriptingContext& context) const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION"))
        return 1.0f;

    float cost_accumulator = 0.0f;
    if (const ShipHull* hull = GetShipHull(m_hull))
        cost_accumulator += hull->ProductionCost(empire_id, location_id, context, m_id);

    int part_count = 0;
    for (const std::string& part_name : m_parts) {
        if (const ShipPart* part = GetShipPart(part_name)) {
            cost_accumulator += part->ProductionCost(empire_id, location_id, context, m_id);
            part_count++;
        }
    }

    // Assuming no reasonable combination of parts and hull will add up to more
    // than ARBITRARY_LARGE_COST. Truncating cost here to return it to indicate
    // an uncalculable cost (ie. due to lacking a valid location object)

    return std::min(std::max(0.0f, cost_accumulator), ARBITRARY_LARGE_COST);
}

float ShipDesign::PerTurnCost(int empire_id, int location_id, const ScriptingContext& context) const {
    return ProductionCost(empire_id, location_id, context) /
        std::max(1, ProductionTime(empire_id, location_id, context));
}

int ShipDesign::ProductionTime(int empire_id, int location_id, const ScriptingContext& context) const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION"))
        return 1;

    int time_accumulator = 1;
    if (const ShipHull* hull = GetShipHull(m_hull))
        time_accumulator = std::max(time_accumulator, hull->ProductionTime(empire_id, location_id, context));

    for (const std::string& part_name : m_parts)
        if (const ShipPart* part = GetShipPart(part_name))
            time_accumulator = std::max(time_accumulator,
                                        part->ProductionTime(empire_id, location_id, context));

    // assuming that ARBITRARY_LARGE_TURNS is larger than any reasonable turns,
    // so the std::max calls will preserve it be returned

    return std::max(1, time_accumulator);
}

bool ShipDesign::CanColonize() const {
    for (const auto& part_name : m_parts) {
        if (part_name.empty())
            continue;
        if (const ShipPart* part = GetShipPart(part_name))
            if (part->Class() == ShipPartClass::PC_COLONY)
                return true;
    }
    return false;
}

float ShipDesign::Defense() const {
    // accumulate defense from defensive parts in design.
    float total_defense = 0.0f;
    const ShipPartManager& part_manager = GetShipPartManager();
    for (const auto& part_name : m_parts) {
        const ShipPart* part = part_manager.GetShipPart(part_name);
        if (part && (part->Class() == ShipPartClass::PC_SHIELD || part->Class() == ShipPartClass::PC_ARMOUR))
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
        if (part_class == ShipPartClass::PC_DIRECT_WEAPON) {
            float part_attack = part->Capacity();
            if (part_attack > shield)
                direct_attack += (part_attack - shield)*part->SecondaryStat();  // here, secondary stat is number of shots per round
        } else if (part_class == ShipPartClass::PC_FIGHTER_HANGAR) {
            available_fighters = part->Capacity();                              // stacked meter
        } else if (part_class == ShipPartClass::PC_FIGHTER_BAY) {
            fighter_launch_capacity += part->Capacity();
            fighter_damage = part->SecondaryStat();                             // here, secondary stat is fighter damage per shot
        }
    }

    int fighter_shots = std::min(available_fighters, fighter_launch_capacity);  // how many fighters launched in bout 1
    available_fighters -= fighter_shots;
    int launched_fighters = fighter_shots;
    int num_bouts = GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS"); // TODO: get from ScriptingContext?
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
    retval.reserve(m_parts.size());
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
        if (part_class == ShipPartClass::PC_DIRECT_WEAPON || part_class == ShipPartClass::PC_FIGHTER_BAY)
            retval.push_back(part_name);
    }
    return retval;
}

int ShipDesign::PartCount() const {
    int count = 0;
    for (auto& entry : m_num_part_classes)
         count += entry.second;
    return count;
}

bool ShipDesign::ProductionLocation(int empire_id, int location_id, const ScriptingContext& context) const {
    auto empire = context.GetEmpire(empire_id);
    if (!empire) {
        DebugLogger() << "ShipDesign::ProductionLocation: Unable to get pointer to empire " << empire_id;
        return false;
    }

    // must own the production location...
    auto location = context.ContextObjects().getRaw(location_id);
    if (!location) {
        WarnLogger() << "ShipDesign::ProductionLocation unable to get location object with id " << location_id;
        return false;
    }
    if (!location->OwnedBy(empire_id))
        return false;

    std::string_view species_name = "";
    if (location->ObjectType() == UniverseObjectType::OBJ_PLANET)
        species_name = static_cast<const Planet*>(location)->SpeciesName();
    else if (location->ObjectType() == UniverseObjectType::OBJ_SHIP)
        species_name = static_cast<const Ship*>(location)->SpeciesName();

    if (species_name.empty())
        return false;
    const Species* species = context.species.GetSpecies(species_name);
    if (!species)
        return false;

    // ships can only be produced by species that are not planetbound
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
    const ScriptingContext location_as_source_context{location, context};
    if (!hull->Location()->EvalOne(location_as_source_context, location))
        return false;

    // apply external and internal parts' location conditions to potential location
    for (const auto& part_name : m_parts) {
        if (part_name.empty())
            continue;       // empty slots don't limit build location

        const ShipPart* part = GetShipPart(part_name);
        if (!part) {
            ErrorLogger() << "ShipDesign::ProductionLocation  ShipDesign couldn't get part with name " << part_name;
            return false;
        }
        if (!part->Location()->EvalOne(location_as_source_context, location))
            return false;
    }
    // location matched all hull and part conditions, so is a valid build location
    return true;
}

void ShipDesign::SetID(int id)
{ m_id = id; }

bool ShipDesign::ValidDesign(const std::string& hull, const std::vector<std::string>& parts_in)
{ return !MaybeInvalidDesign(hull, std::vector<std::string>(parts_in), true); }

boost::optional<std::pair<std::string, std::vector<std::string>>>
ShipDesign::MaybeInvalidDesign(std::string hull, std::vector<std::string> parts, bool produce_log)
{
    bool is_valid = true;

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
            hull.clear();
            parts.clear();
            return std::pair(std::move(hull), std::move(parts));
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
        parts.resize(ship_hull->NumSlots(), "");

    // Truncate or pad with "" parts.
    parts.resize(ship_hull->NumSlots(), "");

    const auto& slots = ship_hull->Slots();

    // check hull exclusions against all parts...
    const auto& hull_exclusions = ship_hull->Exclusions();
    for (auto& part_name : parts) {
        if (part_name.empty())
            continue;
        if (std::count(hull_exclusions.begin(), hull_exclusions.end(), part_name)) {
            is_valid = false;
            if (produce_log)
                WarnLogger() << "Invalid ShipDesign part \"" << part_name << "\" is excluded by \""
                             << ship_hull->Name() << "\". Removing \"" << part_name <<"\"";
            part_name.clear();
        }
    }

    // check part exclusions against other parts and hull

    // find how many of each part and hull are present...
    boost::container::flat_map<std::string_view, uint8_t> component_counts;
    component_counts.reserve(parts.size() + 1);
    component_counts.emplace(hull, 1);
    for (auto& part : parts)
        component_counts[part]++;

    auto has_component = [&component_counts](std::string_view sv) -> bool
    { return component_counts.contains(sv); };
    auto has_multiples_of_component = [&component_counts](std::string_view sv) -> bool
    { return component_counts.contains(sv); };


    // check each part's existance and exclusions
    for (std::size_t ii = 0; ii < parts.size(); ++ii) {
        std::string_view part_name = parts[ii];
        if (part_name.empty()) // ignore empty slots, which are always valid
            continue;

        // Parts must exist...
        const auto ship_part = GetShipPart(part_name);
        if (!ship_part) {
            is_valid = false;
            if (produce_log)
                WarnLogger() << "Invalid ShipDesign part \"" << part_name << "\" not found"
                             << ". Removing \"" << part_name <<"\"";
            continue;
        }

        for (const auto& excluded : ship_part->Exclusions()) {
            // confict if a different excluded part is present, or if there are
            // two or more of a part that excludes itself
            if ((excluded == part_name && has_multiples_of_component(excluded)) ||
                (excluded != part_name && has_component(excluded)))
            {
                is_valid = false;
                if (produce_log)
                    WarnLogger() << "Invalid ShipDesign part " << part_name << " conflicts with \""
                                 << excluded << "\". Removing \"" << part_name <<"\"";
                else
                    break; // don't break if logging, so all conflicts will be logged
            }
        }
        if (!is_valid && !produce_log)
            continue; // if not logging, don't also need to check slot moutability

        // verify part can mount in indicated slot
        const auto slot_type = slots[ii].type;

        if (!ship_part->CanMountInSlotType(slot_type)) {
            if (produce_log)
                DebugLogger() << "Invalid ShipDesign part \"" << part_name << "\" can't be mounted in "
                              << slot_type << " slot. Removing \"" << part_name <<"\"";
            is_valid = false;
        }
    }

    if (is_valid) // if valid, return none to indicate no modifications needed
        return boost::none;
    else
        return std::pair(std::move(hull), std::move(parts)); // return modified design
}

void ShipDesign::ForceValidDesignOrThrow(const boost::optional<std::invalid_argument>& should_throw,
                                         bool produce_log)
{
    auto force_valid = MaybeInvalidDesign(m_hull, m_parts, produce_log);
    if (!force_valid)
        return;

    if (!produce_log && should_throw)
        throw std::invalid_argument("ShipDesign: Bad hull or parts");

    std::stringstream ss;

    bool no_hull_available = force_valid->first.empty();
    if (no_hull_available)
        ss << "ShipDesign has no valid hull and there are no other hulls available.\n";

    std::tie(m_hull, m_parts) = *force_valid;

    ss << "ShipDesign was made valid as:\n";
    ss << Dump() << "\n";

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

    std::vector<std::string_view> tags(hull->Tags().begin(), hull->Tags().end());

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

        std::copy(part->Tags().begin(), part->Tags().end(), std::back_inserter(tags));

        if (!part->Producible())
            m_producible = false;

        ShipPartClass part_class = part->Class();

        switch (part_class) {
        case ShipPartClass::PC_DIRECT_WEAPON:
            m_has_direct_weapons = true;
            if (part->Capacity() > 0.0f)
                m_is_armed = true;
            break;
        case ShipPartClass::PC_FIGHTER_BAY:
            has_fighter_bays = true;
            if (part->Capacity() >= 1.0f)
                can_launch_fighters = true;
            break;
        case ShipPartClass::PC_FIGHTER_HANGAR:
            has_fighter_hangars = true;
            if (part->SecondaryStat() > 0.0f && part->Capacity() >= 1.0f)
                has_armed_fighters = true;
            break;
        case ShipPartClass::PC_COLONY:
            m_colony_capacity += part->Capacity();
            break;
        case ShipPartClass::PC_TROOPS:
            m_troop_capacity += part->Capacity();
            break;
        case ShipPartClass::PC_STEALTH:
            m_stealth += part->Capacity();
            break;
        case ShipPartClass::PC_SPEED:
            m_speed += part->Capacity();
            break;
        case ShipPartClass::PC_SHIELD:
            m_shields += part->Capacity();
            break;
        case ShipPartClass::PC_FUEL:
            m_fuel += part->Capacity();
            break;
        case ShipPartClass::PC_ARMOUR:
            m_structure += part->Capacity();
            break;
        case ShipPartClass::PC_DETECTION:
            m_detection += part->Capacity();
            break;
        case ShipPartClass::PC_BOMBARD:
            m_can_bombard = true;
            break;
        case ShipPartClass::PC_RESEARCH:
            m_research_generation += part->Capacity();
            break;
        case ShipPartClass::PC_INDUSTRY:
            m_industry_generation += part->Capacity();
            break;
        case ShipPartClass::PC_INFLUENCE:
            m_influence_generation += part->Capacity();
            break;
        case ShipPartClass::PC_PRODUCTION_LOCATION:
            m_is_production_location = true;
            break;
        default:
            break;
        }
        m_has_fighters = has_fighter_bays && has_fighter_hangars;
        m_is_armed = m_is_armed || (can_launch_fighters && has_armed_fighters);

        m_num_ship_parts[part_name]++;
        if (part_class > ShipPartClass::INVALID_SHIP_PART_CLASS &&
            part_class < ShipPartClass::NUM_SHIP_PART_CLASSES)
        { m_num_part_classes[part_class]++; }
    }

    // collect unique tags
    std::sort(tags.begin(), tags.end());
    auto last = std::unique(tags.begin(), tags.end());

    // compile concatenated tags into contiguous storage
    // TODO: transform_reduce when available on all platforms...
    std::size_t tags_sz = 0;
    std::for_each(tags.begin(), last, [&tags_sz](auto str) { tags_sz += str.size(); });

    m_tags_concatenated.reserve(tags_sz);
    m_tags.clear();
    m_tags.reserve(tags.size());

    std::for_each(tags.begin(), last, [this](auto str) {
        auto next_start = m_tags_concatenated.size();
        m_tags_concatenated.append(str);
        m_tags.push_back(std::string_view{m_tags_concatenated}.substr(next_start));
    });
}

std::string ShipDesign::Dump(uint8_t ntabs) const {
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
        retval += "\"" + m_parts.front() + "\"\n";
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

uint32_t ShipDesign::GetCheckSum() const {
    uint32_t retval{0};
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
        ++first_parts[part_name];

    for (const std::string& part_name : second.Parts())
        ++second_parts[part_name];

    return first_parts == second_parts;
}


/////////////////////////////////////
// PredefinedShipDesignManager     //
/////////////////////////////////////
PredefinedShipDesignManager* PredefinedShipDesignManager::s_instance = nullptr;

PredefinedShipDesignManager::PredefinedShipDesignManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one PredefinedShipDesignManager.");

    // Only update the global pointer on sucessful construction.
    s_instance = this;
}

namespace {
    void AddDesignToUniverse(Universe& universe, std::unordered_map<std::string, int>& design_generic_ids,
                             const std::unique_ptr<ShipDesign>& design, bool monster)
    {
        if (!design)
            return;

        /* check if there already exists this same design in the universe. */
        for (const auto& [existing_id, existing_design] : universe.ShipDesigns()) {
            if (DesignsTheSame(existing_design, *design)) {
                WarnLogger() << "AddShipDesignsToUniverse found an exact duplicate of ship design "
                             << design->Name() << "to be added, so is not re-adding it";
                design_generic_ids[design->Name(false)] = existing_id;
                return; // design already added; don't need to do so again
            }
        }

        // duplicate design to add to Universe
        const auto new_design_id = universe.InsertShipDesign(*design);
        if (new_design_id == INVALID_DESIGN_ID) {
            ErrorLogger() << "Empire::AddShipDesign Unable to add new design to universe";
            return;
        }

        design_generic_ids[design->Name(false)] = new_design_id;
        TraceLogger() << "AddShipDesignsToUniverse added ship design " << design->Name() << " to universe.";
    };
}

void PredefinedShipDesignManager::AddShipDesignsToUniverse(Universe& universe) const {
    CheckPendingDesignsTypes();
    m_design_generic_ids.clear();

    for (const auto& uuid : m_ship_ordering)
        AddDesignToUniverse(universe, m_design_generic_ids, m_designs.at(uuid), false);

    for (const auto& uuid : m_monster_ordering)
        AddDesignToUniverse(universe, m_design_generic_ids, m_designs.at(uuid), true);
}

PredefinedShipDesignManager& PredefinedShipDesignManager::GetPredefinedShipDesignManager() {
    static PredefinedShipDesignManager manager;
    return manager;
}

std::vector<const ShipDesign*> PredefinedShipDesignManager::GetOrderedShipDesigns() const {
    CheckPendingDesignsTypes();
    std::vector<const ShipDesign*> retval;
    retval.reserve(m_ship_ordering.size());
    for (const auto& uuid : m_ship_ordering)
        retval.push_back(m_designs.at(uuid).get());
    return retval;
}

std::vector<const ShipDesign*> PredefinedShipDesignManager::GetOrderedMonsterDesigns() const {
    CheckPendingDesignsTypes();
    std::vector<const ShipDesign*> retval;
    retval.reserve(m_monster_ordering.size());
    for (const auto& uuid : m_monster_ordering)
        retval.push_back(m_designs.at(uuid).get());
    return retval;
}

int PredefinedShipDesignManager::GetDesignID(const std::string& name) const {
    CheckPendingDesignsTypes();
    const auto it = m_design_generic_ids.find(name);
    if (it == m_design_generic_ids.end())
        return INVALID_DESIGN_ID;
    return it->second;
}

uint32_t PredefinedShipDesignManager::GetCheckSum() const {
    CheckPendingDesignsTypes();
    uint32_t retval{0};

    auto build_checksum = [&retval, this](const std::vector<boost::uuids::uuid>& ordering){
        for (auto const& uuid : ordering) {
            auto it = m_designs.find(uuid);
            if (it != m_designs.end())
                CheckSums::CheckSumCombine(retval, std::pair(it->second->Name(false), *it->second));
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

            if (designs.contains(design->UUID())) {
                ErrorLogger() << design->Name() << " ship design does not have a unique UUID for "
                              << "its type monster or pre-defined. "
                              << designs[design->UUID()]->Name() << " has the same UUID.";
                continue;
            }

            if (name_to_uuid.contains(design->Name())) {
                ErrorLogger() << design->Name() << " ship design does not have a unique name for "
                              << "its type monster or pre-defined.";
                continue;
            }

            auto uuid = design->UUID();
            name_to_uuid.emplace(design->Name(), uuid);
            designs.emplace(std::move(uuid), std::move(design));
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
                       boost::hash<boost::uuids::uuid>> saved_designs;

    auto& designs_and_paths = designs_paths_and_ordering.first;
    auto& disk_ordering = designs_paths_and_ordering.second;

    for (auto& design_and_path : designs_and_paths) {
        auto design = std::make_unique<ShipDesign>(*design_and_path.first);

        // If the UUID is nil this is a legacy design that needs a new UUID
        if (design->UUID().is_nil()) {
            design->SetUUID(boost::uuids::random_generator()());
            DebugLogger() << "Converted legacy ship design file by adding  UUID " << design->UUID()
                          << " for name " << design->Name();
        }

        // Make sure the design is an out of universe object
        // This should not be needed.
        if (design->ID() != INVALID_OBJECT_ID) {
            design->SetID(INVALID_OBJECT_ID);
            ErrorLogger() << "Loaded ship design has an id implying it is in an ObjectMap for UUID "
                          << design->UUID() << " for name " << design->Name();
        }

        if (!saved_designs.contains(design->UUID())) {
            TraceLogger() << "Added saved design UUID " << design->UUID()
                          << " with name " << design->Name();
            auto uuid = design->UUID();
            saved_designs.emplace(std::move(uuid), std::pair(std::move(design), design_and_path.second));
        } else {
            WarnLogger() << "Duplicate ship design UUID " << design->UUID()
                         << " found for ship design " << design->Name()
                         << " and " << saved_designs[design->UUID()].first->Name();
        }
    }

    // Verify that all UUIDs in ordering exist
    std::vector<boost::uuids::uuid> ordering;
    ordering.reserve(disk_ordering.size());
    bool ship_manifest_inconsistent = false;
    for (auto& uuid : disk_ordering) {
        // Skip the nil UUID.
        if (uuid.is_nil())
            continue;

        if (!saved_designs.contains(uuid)) {
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
        for (auto& uuid_to_design_and_filename : saved_designs) {
            if (uuids_in_ordering.contains(uuid_to_design_and_filename.first))
                continue;
            ship_manifest_inconsistent = true;
            missing_uuids_sorted_by_name.emplace(
                uuid_to_design_and_filename.second.first->Name(),
                uuid_to_design_and_filename.first);
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
