#include "Ship.h"

#include "Condition.h"
#include "Conditions.h"
#include "Fighter.h"
#include "Fleet.h"
#include "ScriptingContext.h"
#include "ShipDesign.h"
#include "ShipHull.h"
#include "ShipPart.h"
#include "Species.h"
#include "Universe.h"
#include "ValueRef.h"
#include "../combat/CombatDamage.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../util/AppInterface.h"
#include "../util/GameRules.h"
#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/i18n.h"
#include <numeric>


Ship::Ship(int empire_id, int design_id, std::string species_name,
           const Universe& universe, const SpeciesManager& species,
           int produced_by_empire_id, int current_turn) :
    UniverseObject{UniverseObjectType::OBJ_SHIP, "", empire_id, current_turn},
    m_species_name(std::move(species_name)),
    m_design_id(design_id),
    m_produced_by_empire_id(produced_by_empire_id),
    m_arrived_on_turn(current_turn),
    m_last_resupplied_on_turn(current_turn)
{
    const ShipDesign* design = universe.GetShipDesign(design_id);

    if (!design)
        DebugLogger() << "Constructing a ship with an invalid design ID: " << design_id
                      << "  ... could happen if copying from a ship seen only with basic vis...";

    const auto ship_species = species.GetSpecies(m_species_name);
    if (!m_species_name.empty() && !ship_species)
        DebugLogger() << "Ship created with invalid species name: " << m_species_name;

    AddMeters(ship_meter_types);

    if (!design)
        return;

    m_part_meters.reserve(design->Parts().size() * 2); // guesstimate
    for (const std::string& part_name : design->Parts()) {
        if (!part_name.empty()) {
            const ShipPart* part = GetShipPart(part_name);
            if (!part) [[unlikely]] {
                ErrorLogger() << "Ship::Ship couldn't get part with name " << part_name;
                continue;
            }

#if defined(__cpp_using_enum)
            using enum MeterType;
            using enum ShipPartClass;
#else
            static constexpr auto METER_CAPACITY = MeterType::METER_CAPACITY;
            static constexpr auto METER_MAX_CAPACITY = MeterType::METER_MAX_CAPACITY;
            static constexpr auto METER_SECONDARY_STAT = MeterType::METER_SECONDARY_STAT;
            static constexpr auto METER_MAX_SECONDARY_STAT = MeterType::METER_MAX_SECONDARY_STAT;
            static constexpr auto PC_COLONY = ShipPartClass::PC_COLONY;
            static constexpr auto PC_TROOPS = ShipPartClass::PC_TROOPS;
            static constexpr auto PC_DIRECT_WEAPON = ShipPartClass::PC_DIRECT_WEAPON;
            static constexpr auto PC_FIGHTER_HANGAR = ShipPartClass::PC_FIGHTER_HANGAR;
            static constexpr auto PC_FIGHTER_BAY = ShipPartClass::PC_FIGHTER_BAY;
#endif
            switch (part->Class()) {
            case PC_COLONY:
            case PC_TROOPS: {
                m_part_meters[{part_name, METER_CAPACITY}];
                break;
            }
            case PC_DIRECT_WEAPON:      // capacity is damage, secondary stat is shots per attack
            case PC_FIGHTER_HANGAR: {   // capacity is how many fighters contained, secondary stat is damage per fighter attack
                m_part_meters[{part_name, METER_SECONDARY_STAT}];
                m_part_meters[{part_name, METER_MAX_SECONDARY_STAT}];
            }
            [[fallthrough]];
            case PC_FIGHTER_BAY: {      // capacity is how many fighters launched per combat round
                m_part_meters[{part_name, METER_CAPACITY}];
                m_part_meters[{part_name, METER_MAX_CAPACITY}];
                break;
            }
            default:
                break;
            }
        }
    }
}

std::shared_ptr<UniverseObject> Ship::Clone(const Universe& universe, int empire_id) const {
    const Visibility vis = empire_id == ALL_EMPIRES ?
        Visibility::VIS_FULL_VISIBILITY : universe.GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= Visibility::VIS_BASIC_VISIBILITY && vis <= Visibility::VIS_FULL_VISIBILITY))
        return nullptr;

    auto retval = std::make_shared<Ship>();
    retval->Copy(*this, universe, empire_id);
    return retval;
}

void Ship::Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id) {
    if (&copied_object == this)
        return;

    if (copied_object.ObjectType() != UniverseObjectType::OBJ_SHIP) {
        ErrorLogger() << "Ship::Copy passed an object that wasn't a Ship";
        return;
    }

    Copy(static_cast<const Ship&>(copied_object), universe, empire_id);
}

void Ship::Copy(const Ship& copied_ship, const Universe& universe, int empire_id) {
    if (&copied_ship == this)
        return;

    const int copied_object_id = copied_ship.ID();
    const Visibility vis = empire_id == ALL_EMPIRES ?
        Visibility::VIS_FULL_VISIBILITY : universe.GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    const auto visible_specials = universe.GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_ship, vis, visible_specials, universe);

    if (vis >= Visibility::VIS_BASIC_VISIBILITY) {
        this->m_fleet_id =                      copied_ship.m_fleet_id;

        if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
            if (this->Unowned())
                this->m_name =                  copied_ship.m_name;

            this->m_design_id =                 copied_ship.m_design_id;
            this->m_part_meters =               copied_ship.m_part_meters;
            this->m_species_name =              copied_ship.m_species_name;

            this->m_last_turn_active_in_combat= copied_ship.m_last_turn_active_in_combat;
            this->m_produced_by_empire_id =     copied_ship.m_produced_by_empire_id;
            this->m_arrived_on_turn =           copied_ship.m_arrived_on_turn;
            this->m_last_resupplied_on_turn =   copied_ship.m_last_resupplied_on_turn;

            if (vis >= Visibility::VIS_FULL_VISIBILITY) {
                this->m_ordered_scrapped =          copied_ship.m_ordered_scrapped;
                this->m_ordered_colonize_planet_id= copied_ship.m_ordered_colonize_planet_id;
                this->m_ordered_invade_planet_id  = copied_ship.m_ordered_invade_planet_id;
                this->m_ordered_bombard_planet_id = copied_ship.m_ordered_bombard_planet_id;
            }
        }
    }
}

bool Ship::HostileToEmpire(int empire_id, const EmpireManager& empires) const {
    if (OwnedBy(empire_id))
        return false;
    return empire_id == ALL_EMPIRES || Unowned() ||
        empires.GetDiplomaticStatus(Owner(), empire_id) == DiplomaticStatus::DIPLO_WAR;
}

bool Ship::HasTag(std::string_view name, const ScriptingContext& context) const {
    const ShipDesign* design = context.ContextUniverse().GetShipDesign(m_design_id);
    if (design && design->HasTag(name))
        return true;
    const Species* species = context.species.GetSpecies(m_species_name);
    return species && species->HasTag(name);
}

UniverseObject::TagVecs Ship::Tags(const ScriptingContext& context) const {
    const ShipDesign* design = context.ContextUniverse().GetShipDesign(m_design_id);
    const Species* species = context.species.GetSpecies(m_species_name);

    if (design && species)
        return TagVecs{design->Tags(), species->Tags()};
    else if (design)
        return TagVecs{design->Tags()};
    else if (species)
        return TagVecs{species->Tags()};
    else return {};
}

bool Ship::ContainedBy(int object_id) const noexcept {
    return object_id != INVALID_OBJECT_ID
        && (    object_id == m_fleet_id
            ||  object_id == this->SystemID());
}

std::string Ship::Dump(uint8_t ntabs) const {
    std::string retval = UniverseObject::Dump(ntabs);
    retval.reserve(2048); // guesstimate
    retval.append(" design id: ").append(std::to_string(m_design_id))
          .append(" fleet id: ").append(std::to_string(m_fleet_id))
          .append(" species name: ").append(m_species_name)
          .append(" produced by empire id: ").append(std::to_string(m_produced_by_empire_id))
          .append(" arrived on turn: ").append(std::to_string(m_arrived_on_turn))
          .append(" last resupplied on turn: ").append(std::to_string(m_last_resupplied_on_turn));
    if (!m_part_meters.empty()) {
        retval.append(" part meters: ");
        for (const auto& [meter_type_part_name, meter] : m_part_meters) {
            const auto& [part_name, meter_type] = meter_type_part_name;
            retval.append(part_name).append(" ")
                  .append(to_string(meter_type))
                  .append(": ").append(std::to_string(meter.Current())).append("  ");
        }
    }

    return retval;
}

bool Ship::IsMonster(const Universe& universe) const {
    if (const ShipDesign* design = universe.GetShipDesign(m_design_id))
        return design->IsMonster();
    else
        return false;
}

bool Ship::CanDamageShips(const ScriptingContext& context, float target_shields) const
{ return TotalWeaponsShipDamage(context, target_shields, true) > 0.0f; }

bool Ship::CanDestroyFighters(const ScriptingContext& context) const
{ return TotalWeaponsFighterDamage(context, true) > 0.0f; }

bool Ship::IsArmed(const ScriptingContext& context) const {
    bool has_fighters = HasFighters(context.ContextUniverse());

    for (auto& [meter_type_part, meter] : m_part_meters) {
        auto& [part_name, meter_type] = meter_type_part;

        const ShipPart* part = GetShipPart(part_name);
        if (!part)
            continue;

        if (meter_type == MeterType::METER_CAPACITY &&
            part->Class() == ShipPartClass::PC_DIRECT_WEAPON &&
            meter.Current() > 0.0f)
        {
            return true; // ship has a direct weapon that can do damage

        } else if (meter_type == MeterType::METER_SECONDARY_STAT &&
                   has_fighters &&
                   part->Class() == ShipPartClass::PC_FIGHTER_HANGAR &&
                   meter.Current() > 0.0f)
        {
            return true; // ship has fighters and those fighters can do damage
        }
    }

    return false;
}

bool Ship::HasFighters(const Universe& universe) const {
    const ShipDesign* design = universe.GetShipDesign(m_design_id);
    if (!design || !design->HasFighters())  // ensures ship has ability to launch fighters
        return false;

    // ensure ship currently has fighters to launch
    for (auto& [meter_type_part, meter] : m_part_meters) {
        auto& [part_name, meter_type] = meter_type_part;
        if (meter_type != MeterType::METER_CAPACITY)
            continue;
        const ShipPart* part = GetShipPart(part_name);
        if (!part || part->Class() != ShipPartClass::PC_FIGHTER_HANGAR)
            continue;
        if (meter.Current() > 0.0f)
            return true;
    }

    return false;
}

bool Ship::CanColonize(const Universe& universe, const SpeciesManager& sm) const {
    if (m_species_name.empty())
        return false;
    const ShipDesign* design = universe.GetShipDesign(m_design_id);
    if (design && design->CanColonize()) {
        if (design->ColonyCapacity() == 0.0f) // zero-capacity colony ships count as outpost ships
            return true;
        // for establishing a colony, the species needs to be able to colonize
        const Species* species = sm.GetSpecies(m_species_name);
        if (species && species->CanColonize())
            return true;
    }
    return false;
}

bool Ship::HasTroops(const Universe& universe) const
{ return this->TroopCapacity(universe) > 0.0f; }

bool Ship::CanBombard(const Universe& universe) const {
    const ShipDesign* design = universe.GetShipDesign(m_design_id);
    return design && design->CanBombard();
}

float Ship::Speed() const
{ return GetMeter(MeterType::METER_SPEED)->Initial(); }

float Ship::ColonyCapacity(const Universe& universe) const {
    float retval = 0.0f;
    // find which colony parts are present in design (one copy of name for each instance of a part, allowing duplicate names to appear)
    const ShipDesign* design = universe.GetShipDesign(m_design_id);
    if (!design)
        return retval;

    for (const std::string& part_name : design->Parts()) {
        if (part_name.empty())
            continue;
        const ShipPart* part = GetShipPart(part_name);
        if (!part || part->Class() != ShipPartClass::PC_COLONY)
            continue;
        // add capacity for all instances of colony parts to accumulator
        retval += this->InitialPartMeterValue(MeterType::METER_CAPACITY, part_name);
    }

    return retval;
}

float Ship::TroopCapacity(const Universe& universe) const {
    float retval = 0.0f;
    // find which troop parts are present in design (one copy of name for each instance of a part, allowing duplicate names to appear)
    const ShipDesign* design = universe.GetShipDesign(m_design_id);
    if (!design)
        return retval;

    for (const std::string& part_name : design->Parts()) {
        if (part_name.empty())
            continue;
        const ShipPart* part = GetShipPart(part_name);
        if (!part)
            continue;
        ShipPartClass part_class = part->Class();
        if (part_class != ShipPartClass::PC_TROOPS)
            continue;
        // add capacity for all instances of colony parts to accumulator
        retval += this->InitialPartMeterValue(MeterType::METER_CAPACITY, part_name);
    }

    return retval;
}

bool Ship::CanHaveTroops(const Universe& universe) const {
    const ShipDesign* design = universe.GetShipDesign(m_design_id);
    return design ? design->HasTroops() : false;
}

const std::string& Ship::PublicName(int empire_id, const Universe& universe) const {
    // Disclose real ship name only to fleet owners. Rationale: a player who
    // doesn't know the design for a particular ship can easily guess it if the
    // ship's name is "Scout"
    // An exception is made for unowned monsters.
    if (empire_id == ALL_EMPIRES || OwnedBy(empire_id) || (IsMonster(universe) && Unowned()))  // TODO: Check / apply GameRule for all objects visible?
        return Name();
    if (const ShipDesign* design = universe.GetShipDesign(m_design_id))
        return design->Name();
    else if (IsMonster(universe))
        return UserString("SM_MONSTER");
    else if (!Unowned())
        return UserString("FW_FOREIGN_SHIP");
    else if (Unowned() && GetVisibility(empire_id, universe) > Visibility::VIS_NO_VISIBILITY)
        return UserString("FW_ROGUE_SHIP");
    else
        return UserString("OBJ_SHIP");
}

const std::string& Ship::PublicName(int empire_id) const {
    if (empire_id == ALL_EMPIRES || OwnedBy(empire_id))
        return Name();
    else if (!Unowned())
        return UserString("FW_FOREIGN_SHIP");
    else if (Unowned())
        return UserString("FW_ROGUE_SHIP");
    else
        return UserString("OBJ_SHIP");
}

const Meter* Ship::GetPartMeter(MeterType type, const std::string& part_name) const {
    const Meter* retval = nullptr;
    const auto it = std::find_if(m_part_meters.begin(), m_part_meters.end(),
                                 [type, &part_name](const auto& name_type)
                                 { return name_type.first.first == part_name && name_type.first.second == type; });
    if (it != m_part_meters.end())
        retval = &it->second;
    return retval;
}

Meter* Ship::GetPartMeter(MeterType type, const std::string& part_name) {
    Meter* retval = nullptr;
    const auto it = std::find_if(m_part_meters.begin(), m_part_meters.end(),
                                 [type, &part_name](const auto& name_type)
                                 { return name_type.first.first == part_name && name_type.first.second == type; });
    if (it != m_part_meters.end())
        retval = &it->second;
    return retval;
}

float Ship::CurrentPartMeterValue(MeterType type, const std::string& part_name) const {
    if (const Meter* meter = GetPartMeter(type, part_name))
        return meter->Current();
    return 0.0f;
}

float Ship::InitialPartMeterValue(MeterType type, const std::string& part_name) const {
    if (const Meter* meter = GetPartMeter(type, part_name))
        return meter->Initial();
    return 0.0f;
}

float Ship::SumCurrentPartMeterValuesForPartClass(MeterType type, ShipPartClass part_class,
                                                  const Universe& universe) const
{
    float retval = 0.0f;

    const ShipDesign* design = universe.GetShipDesign(m_design_id);
    if (!design)
        return retval;

    const auto& parts = design->Parts();
    if (parts.empty())
        return retval;

    std::map<std::string, int> part_counts;
    for (const std::string& part : parts)
        part_counts[part]++;

    for (const auto& [meter_type_part, meter] : m_part_meters) {
        if (meter_type_part.second != type)
            continue;
        const std::string& part_name = meter_type_part.first;
        const auto part_count = part_counts[part_name];
        if (part_count < 1)
            continue;
        if (const ShipPart* part = GetShipPart(part_name))
            if (part_class == part->Class())
                retval += meter.Current() * part_count;
    }

    return retval;
}

float Ship::FighterCount() const {
    float retval = 0.0f;
    for (auto& [meter_type_part, meter] : m_part_meters) {
        const auto& [part_name, meter_type] = meter_type_part;
        if (meter_type != MeterType::METER_CAPACITY)
            continue;
        if (const ShipPart* part = GetShipPart(part_name))
            if (part->Class() == ShipPartClass::PC_FIGHTER_HANGAR)
                retval += meter.Current();
    }

    return retval;
}

float Ship::FighterMax() const {
    float retval = 0.0f;
    for (auto& [meter_type_part, meter] : m_part_meters) {
        const auto& [part_name, meter_type] = meter_type_part;
        if (meter_type != MeterType::METER_MAX_CAPACITY)
            continue;
        if (const ShipPart* part = GetShipPart(part_name))
            if (part->Class() == ShipPartClass::PC_FIGHTER_HANGAR)
                retval += meter.Current();
    }

    return retval;
}

float Ship::WeaponPartFighterDamage(const ShipPart* part, const ScriptingContext& context) const {
    if (!part || (part->Class() != ShipPartClass::PC_DIRECT_WEAPON))
        return 0.0f;

    // usually a weapon part destroys one fighter per shot, but that can be overridden
    if (part->TotalFighterDamage()) {
        return part->TotalFighterDamage()->Eval(context);
    } else {
        const int num_bouts_with_fighter_targets = GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS") - 1;
        return CurrentPartMeterValue(MeterType::METER_SECONDARY_STAT, part->Name()) * num_bouts_with_fighter_targets;  // used within loop that updates meters, so need current, not initial values
    }
}

float Ship::WeaponPartShipDamage(const ShipPart* part, const ScriptingContext& context) const {
    if (!part || (part->Class() != ShipPartClass::PC_DIRECT_WEAPON))
        return 0.0f;

    // usually a weapon part does damage*shots ship damage, but that can be overridden
    if (part->TotalShipDamage()) {
        return part->TotalShipDamage()->Eval(context);
    } else {
        const float part_attack = CurrentPartMeterValue(MeterType::METER_CAPACITY, part->Name());  // used within loop that updates meters, so need current, not initial values
        const float part_shots = CurrentPartMeterValue(MeterType::METER_SECONDARY_STAT, part->Name());
        float target_shield = 0.0f;
        if (context.effect_target) {
            const Ship* target = static_cast<const Ship*>(context.effect_target);
            target_shield = target->GetMeter(MeterType::METER_SHIELD)->Current();
        }
        if (part_attack > target_shield) {
            const int num_bouts = GetGameRules().Get<int>("RULE_NUM_COMBAT_ROUNDS");
            return (part_attack - target_shield) * part_shots * num_bouts;
        } else {
            return 0.0f;
        }
    }
}

float Ship::TotalWeaponsFighterDamage(const ScriptingContext& context, bool launch_fighters) const {
    // sum up all individual weapons' attack strengths
    const auto all_weapons_shots = AllWeaponsFighterDamage(context, launch_fighters);
    return std::accumulate(all_weapons_shots.begin(), all_weapons_shots.end(), 0.0f);
}

float Ship::TotalWeaponsShipDamage(const ScriptingContext& context, float shield_DR,
                                   bool launch_fighters) const
{
    // sum up all individual weapons' attack strengths
    const auto all_weapons_damage = AllWeaponsShipDamage(context, shield_DR, launch_fighters);
    return std::accumulate(all_weapons_damage.begin(), all_weapons_damage.end(), 0.0f);
}

std::vector<float> Ship::AllWeaponsFighterDamage(const ScriptingContext& context,
                                                 bool launch_fighters) const
{
    return Combat::WeaponDamageImpl(context, *this, /*target_shield_DR*/0, /*max meters*/false,
                                    launch_fighters, /*target_ships*/false);
}

std::vector<float> Ship::AllWeaponsShipDamage(const ScriptingContext& context, float shield_DR,
                                              bool launch_fighters) const
{ return Combat::WeaponDamageImpl(context, *this, shield_DR, false, launch_fighters, true); }

std::vector<float> Ship::AllWeaponsMaxShipDamage(const ScriptingContext& context, float shield_DR,
                                                 bool launch_fighters) const
{
    std::vector<float> retval;

    const ShipDesign* design = context.ContextUniverse().GetShipDesign(m_design_id);
    if (!design)
        return retval;

    return Combat::WeaponDamageImpl(context, *this, shield_DR, true, launch_fighters);
}

std::size_t Ship::SizeInMemory() const {
    std::size_t retval = UniverseObject::SizeInMemory();
    retval += sizeof(Ship) - sizeof(UniverseObject);

    retval += sizeof(PartMeterMap::value_type)*m_part_meters.capacity();
    for (const auto& name : m_part_meters | range_keys | range_keys)
        retval += name.capacity()*sizeof(std::decay_t<decltype(name)>::value_type);
    retval += sizeof(decltype(m_species_name)::value_type)*m_species_name.capacity();

    return retval;
}

void Ship::SetFleetID(int fleet_id) {
    if (m_fleet_id != fleet_id) {
        m_fleet_id = fleet_id;
        StateChangedSignal();
    }
}

void Ship::SetArrivedOnTurn(int turn) {
    if (m_arrived_on_turn != turn) {
        m_arrived_on_turn = turn;
        StateChangedSignal();
    }
}

void Ship::BackPropagateMeters() noexcept {
    UniverseObject::BackPropagateMeters();

    // ship part meter back propagation, since base class function doesn't do this...
    for (auto& entry : m_part_meters)
        entry.second.BackPropagate();
}

namespace {
    // specifically for ship part meters
    constexpr auto ToPairedMeterType(MeterType mt) noexcept {
        return
            mt == MeterType::METER_CAPACITY ? MeterType::METER_MAX_CAPACITY :
            mt == MeterType::METER_SECONDARY_STAT ? MeterType::METER_MAX_SECONDARY_STAT :
            MeterType::INVALID_METER_TYPE;
    };

    // specifically for ship part meters
    constexpr bool IsMaxMeterType(MeterType mt) noexcept
    { return mt == MeterType::METER_MAX_CAPACITY || mt == MeterType::METER_MAX_SECONDARY_STAT; }
}

void Ship::Resupply(int turn) {
    m_last_resupplied_on_turn = turn;

    Meter* fuel_meter = UniverseObject::GetMeter(MeterType::METER_FUEL);
    const Meter* max_fuel_meter = UniverseObject::GetMeter(MeterType::METER_MAX_FUEL);
    if (!fuel_meter || !max_fuel_meter) [[unlikely]] {
        ErrorLogger() << "Ship::Resupply couldn't get fuel meters!";
    } else {
        fuel_meter->SetCurrent(max_fuel_meter->Current());
        fuel_meter->BackPropagate();
    }

    // set all part capacities equal to any associated max capacity
    // this "upgrades" any direct-fire weapon parts to their latest-allowed
    // strengths, and replaces any lost fighters
    for (auto& [type_str, meter] : m_part_meters) {
        const auto& [part_name, meter_type] = type_str;
        const MeterType paired_meter_type = ToPairedMeterType(meter_type);
        if (paired_meter_type == MeterType::INVALID_METER_TYPE)
            continue;

        const auto max_it = m_part_meters.find(std::pair{std::string_view{part_name}, paired_meter_type});
        if (max_it != m_part_meters.end()) [[likely]] {
            const Meter& max_meter = max_it->second;
            meter.SetCurrent(max_meter.Current());
            meter.BackPropagate();
        }
    }
}

void Ship::SetSpecies(std::string species_name, const SpeciesManager& sm) {
    if (!sm.GetSpecies(species_name)) [[unlikely]] 
        ErrorLogger() << "Ship::SetSpecies couldn't get species with name " << species_name;
    m_species_name = std::move(species_name);
}

void Ship::SetOrderedScrapped(bool b) {
    if (b == m_ordered_scrapped) return;
    m_ordered_scrapped = b;
    StateChangedSignal();
}

void Ship::SetColonizePlanet(int planet_id) {
    if (planet_id == m_ordered_colonize_planet_id) return;
    m_ordered_colonize_planet_id = planet_id;
    StateChangedSignal();
}

void Ship::ClearColonizePlanet()
{ SetColonizePlanet(INVALID_OBJECT_ID); }

void Ship::SetInvadePlanet(int planet_id) {
    if (planet_id == m_ordered_invade_planet_id) return;
    m_ordered_invade_planet_id = planet_id;
    StateChangedSignal();
}

void Ship::ClearInvadePlanet()
{ SetInvadePlanet(INVALID_OBJECT_ID); }

void Ship::SetBombardPlanet(int planet_id) {
    if (planet_id == m_ordered_bombard_planet_id) return;
    m_ordered_bombard_planet_id = planet_id;
    StateChangedSignal();
}

void Ship::ClearBombardPlanet()
{ SetBombardPlanet(INVALID_OBJECT_ID); }

void Ship::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();

    UniverseObject::GetMeter(MeterType::METER_MAX_FUEL)->ResetCurrent();
    UniverseObject::GetMeter(MeterType::METER_MAX_SHIELD)->ResetCurrent();
    UniverseObject::GetMeter(MeterType::METER_MAX_STRUCTURE)->ResetCurrent();
    UniverseObject::GetMeter(MeterType::METER_TARGET_INDUSTRY)->ResetCurrent();
    UniverseObject::GetMeter(MeterType::METER_TARGET_RESEARCH)->ResetCurrent();
    UniverseObject::GetMeter(MeterType::METER_TARGET_INFLUENCE)->ResetCurrent();

    UniverseObject::GetMeter(MeterType::METER_DETECTION)->ResetCurrent();
    UniverseObject::GetMeter(MeterType::METER_SPEED)->ResetCurrent();
    //UniverseObject::GetMeter(MeterType::METER_STEALTH)->ResetCurrent(); redundant with base class function


    // meters with no associated paired meter are (target/max/unpaired)
    for (auto& [type_str, meter] : m_part_meters) {
        const auto& [part_name, meter_type] = type_str;
        const MeterType paired_meter_type = ToPairedMeterType(meter_type);
        if (paired_meter_type != MeterType::INVALID_METER_TYPE && // meter type with a paired meter type is not max or target
            m_part_meters.find(std::pair{std::string_view{part_name}, paired_meter_type}) != m_part_meters.end()) [[unlikely]] // no paired meter found, so is unpaired
        { continue; }

        // reset target/max/unpaired
        meter.ResetCurrent();
    }
}

void Ship::ResetPairedActiveMeters() {
    UniverseObject::ResetPairedActiveMeters();

    // meters are paired only if they are not max/target meters, and there is an
    // associated max/target meter
    for (auto& [type_str, meter] : m_part_meters) {
        const auto& [part_name, meter_type] = type_str;
        const MeterType paired_meter_type = ToPairedMeterType(meter_type);
        if (paired_meter_type == MeterType::INVALID_METER_TYPE ||
            m_part_meters.find(std::pair{std::string_view{part_name}, paired_meter_type}) == m_part_meters.end()) [[likely]]
        { continue; }

        // has an associated max/target meter.
        meter.SetCurrent(meter.Initial());
    }
}

void Ship::SetShipMetersToMax() {
    UniverseObject::GetMeter(MeterType::METER_MAX_FUEL)->SetCurrent(Meter::LARGE_VALUE);
    UniverseObject::GetMeter(MeterType::METER_MAX_SHIELD)->SetCurrent(Meter::LARGE_VALUE);
    UniverseObject::GetMeter(MeterType::METER_MAX_STRUCTURE)->SetCurrent(Meter::LARGE_VALUE);
    UniverseObject::GetMeter(MeterType::METER_FUEL)->SetCurrent(Meter::LARGE_VALUE);
    UniverseObject::GetMeter(MeterType::METER_SHIELD)->SetCurrent(Meter::LARGE_VALUE);
    UniverseObject::GetMeter(MeterType::METER_STRUCTURE)->SetCurrent(Meter::LARGE_VALUE);

    // some part capacity meters may have an associated max capacity...
    for (auto& entry : m_part_meters)
        entry.second.SetCurrent(Meter::LARGE_VALUE);
}

void Ship::ClampMeters() {
    UniverseObject::ClampMeters();

    UniverseObject::GetMeter(MeterType::METER_MAX_FUEL)->ClampCurrentToRange();
    UniverseObject::GetMeter(MeterType::METER_FUEL)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(MeterType::METER_MAX_FUEL)->Current());
    UniverseObject::GetMeter(MeterType::METER_MAX_SHIELD)->ClampCurrentToRange();
    UniverseObject::GetMeter(MeterType::METER_SHIELD)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(MeterType::METER_MAX_SHIELD)->Current());
    UniverseObject::GetMeter(MeterType::METER_MAX_STRUCTURE)->ClampCurrentToRange();
    UniverseObject::GetMeter(MeterType::METER_STRUCTURE)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(MeterType::METER_MAX_STRUCTURE)->Current());
    UniverseObject::GetMeter(MeterType::METER_TARGET_INDUSTRY)->ClampCurrentToRange();
    UniverseObject::GetMeter(MeterType::METER_INDUSTRY)->ClampCurrentToRange();
    UniverseObject::GetMeter(MeterType::METER_TARGET_RESEARCH)->ClampCurrentToRange();
    UniverseObject::GetMeter(MeterType::METER_RESEARCH)->ClampCurrentToRange();
    //UniverseObject::GetMeter(MeterType::METER_TARGET_INFLUENCE)->ClampCurrentToRange(-Meter::LARGE_VALUE, Meter::LARGE_VALUE);
    //UniverseObject::GetMeter(MeterType::METER_INFLUENCE)->ClampCurrentToRange(-Meter::LARGE_VALUE, Meter::LARGE_VALUE);

    UniverseObject::GetMeter(MeterType::METER_DETECTION)->ClampCurrentToRange();
    UniverseObject::GetMeter(MeterType::METER_SPEED)->ClampCurrentToRange();

    // clamp most part meters to basic range limits
    for (auto& [type_str, meter] : m_part_meters) {
        if (IsMaxMeterType(type_str.second))
            meter.ClampCurrentToRange();
    }

    // special case extra clamping for paired active meters dependent
    // on their associated max meter...
    for (auto& [type_str, meter] : m_part_meters) {
        const auto& [part_name, meter_type] = type_str;
        const MeterType paired_meter_type = ToPairedMeterType(meter_type);
        if (paired_meter_type == MeterType::INVALID_METER_TYPE)
            continue;
        const auto max_it = m_part_meters.find(std::pair{std::string_view{part_name}, paired_meter_type});
        if (max_it != m_part_meters.end()) {
            const Meter& max_meter = max_it->second;
            meter.ClampCurrentToRange(Meter::DEFAULT_VALUE, max_meter.Current());
        }
    }
}

////////////////////
// Free Functions //
////////////////////
std::string NewMonsterName() {
    auto monster_names = UserStringList("MONSTER_NAMES");
    static std::unordered_map<std::string, std::size_t> monster_names_used;

    if (monster_names.empty())
        monster_names.push_back(UserString("MONSTER"));

    // select name randomly from list
    int monster_name_index = RandInt(0, static_cast<int>(monster_names.size()) - 1);
    std::string result = monster_names[monster_name_index];
    if (monster_names_used[result]++)
        result += " " + RomanNumber(monster_names_used[result]);
    return result;
}
