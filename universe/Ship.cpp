#include "Ship.h"

#include "Fleet.h"
#include "ShipDesign.h"
#include "ShipHull.h"
#include "ShipPart.h"
#include "Species.h"
#include "UniverseObjectVisitor.h"
#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../util/AppInterface.h"
#include "../util/GameRules.h"
#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/i18n.h"


Ship::Ship(int empire_id, int design_id, std::string species_name, int produced_by_empire_id) :
    m_design_id(design_id),
    m_species_name(std::move(species_name)),
    m_produced_by_empire_id(produced_by_empire_id),
    m_arrived_on_turn(CurrentTurn()),
    m_last_resupplied_on_turn(CurrentTurn())
{
    if (!GetShipDesign(design_id))
        throw std::invalid_argument("Attempted to construct a Ship with an invalid design id");

    if (!m_species_name.empty() && !GetSpecies(m_species_name))
        DebugLogger() << "Ship created with invalid species name: " << m_species_name;

    SetOwner(empire_id);

    UniverseObject::Init();

    AddMeter(MeterType::METER_FUEL);
    AddMeter(MeterType::METER_MAX_FUEL);
    AddMeter(MeterType::METER_SHIELD);
    AddMeter(MeterType::METER_MAX_SHIELD);
    AddMeter(MeterType::METER_DETECTION);
    AddMeter(MeterType::METER_STRUCTURE);
    AddMeter(MeterType::METER_MAX_STRUCTURE);
    AddMeter(MeterType::METER_SPEED);
    AddMeter(MeterType::METER_TARGET_INDUSTRY);
    AddMeter(MeterType::METER_INDUSTRY);
    AddMeter(MeterType::METER_TARGET_RESEARCH);
    AddMeter(MeterType::METER_RESEARCH);
    AddMeter(MeterType::METER_TARGET_INFLUENCE);
    AddMeter(MeterType::METER_INFLUENCE);

    for (const std::string& part_name : Design()->Parts()) {
        if (!part_name.empty()) {
            const ShipPart* part = GetShipPart(part_name);
            if (!part) {
                ErrorLogger() << "Ship::Ship couldn't get part with name " << part_name;
                continue;
            }

            switch (part->Class()) {
            case ShipPartClass::PC_COLONY:
            case ShipPartClass::PC_TROOPS: {
                m_part_meters[{MeterType::METER_CAPACITY, part_name}];
                break;
            }
            case ShipPartClass::PC_DIRECT_WEAPON:      // capacity is damage, secondary stat is shots per attack
            case ShipPartClass::PC_FIGHTER_HANGAR: {   // capacity is how many fighters contained, secondary stat is damage per fighter attack
                m_part_meters[{MeterType::METER_SECONDARY_STAT, part_name}];
                m_part_meters[{MeterType::METER_MAX_SECONDARY_STAT, part_name}];
                // intentionally no break here
            }
            case ShipPartClass::PC_FIGHTER_BAY: {      // capacity is how many fighters launched per combat round
                m_part_meters[{MeterType::METER_CAPACITY, part_name}];
                m_part_meters[{MeterType::METER_MAX_CAPACITY, part_name}];
                break;
            }
            default:
                break;
            }
        }
    }
}

Ship* Ship::Clone(int empire_id) const {
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= Visibility::VIS_BASIC_VISIBILITY && vis <= Visibility::VIS_FULL_VISIBILITY))
        return nullptr;

    Ship* retval = new Ship();
    retval->Copy(shared_from_this(), empire_id);
    return retval;
}

void Ship::Copy(std::shared_ptr<const UniverseObject> copied_object, int empire_id) {
    if (copied_object.get() == this)
        return;
    auto copied_ship = std::dynamic_pointer_cast<const Ship>(copied_object);
    if (!copied_ship) {
        ErrorLogger() << "Ship::Copy passed an object that wasn't a Ship";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    auto visible_specials = GetUniverse().GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(std::move(copied_object), vis, visible_specials);;

    if (vis >= Visibility::VIS_BASIC_VISIBILITY) {
        if (this->m_fleet_id != copied_ship->m_fleet_id) {
            // as with other containers, removal from the old container is triggered by the contained Object; removal from System is handled by UniverseObject::Copy
            if (auto old_fleet = Objects().get<Fleet>(this->m_fleet_id))
                old_fleet->RemoveShips({this->ID()});
            this->m_fleet_id = copied_ship->m_fleet_id; // as with other containers (Systems), actual insertion into fleet ships set is handled by the fleet
        }

        if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
            if (this->Unowned())
                this->m_name =                  copied_ship->m_name;

            this->m_design_id =                 copied_ship->m_design_id;
            this->m_part_meters =               copied_ship->m_part_meters;
            this->m_species_name =              copied_ship->m_species_name;

            this->m_last_turn_active_in_combat= copied_ship->m_last_turn_active_in_combat;
            this->m_produced_by_empire_id =     copied_ship->m_produced_by_empire_id;
            this->m_arrived_on_turn =           copied_ship->m_arrived_on_turn;
            this->m_last_resupplied_on_turn =   copied_ship->m_last_resupplied_on_turn;

            if (vis >= Visibility::VIS_FULL_VISIBILITY) {
                this->m_ordered_scrapped =          copied_ship->m_ordered_scrapped;
                this->m_ordered_colonize_planet_id= copied_ship->m_ordered_colonize_planet_id;
                this->m_ordered_invade_planet_id  = copied_ship->m_ordered_invade_planet_id;
                this->m_ordered_bombard_planet_id = copied_ship->m_ordered_bombard_planet_id;
            }
        }
    }
}

bool Ship::HostileToEmpire(int empire_id) const
{
    if (OwnedBy(empire_id))
        return false;
    return empire_id == ALL_EMPIRES || Unowned() ||
           Empires().GetDiplomaticStatus(Owner(), empire_id) == DiplomaticStatus::DIPLO_WAR;
}

std::set<std::string> Ship::Tags() const {
    const ShipDesign* design = GetShipDesign(m_design_id);
    if (!design)
        return {};

    const ShipHull* hull = ::GetShipHull(design->Hull());
    if (!hull)
        return {};

    std::set<std::string> retval{hull->Tags().begin(), hull->Tags().end()};

    const std::vector<std::string>& parts = design->Parts();
    if (parts.empty())
        return retval;

    for (const std::string& part_name : parts) {
        if (const ShipPart* part = GetShipPart(part_name)) {
            retval.insert(part->Tags().begin(), part->Tags().end());
        }
    }

    return retval;
}

bool Ship::HasTag(const std::string& name) const {
    const ShipDesign* design = GetShipDesign(m_design_id);
    if (design) {
        // check hull for tag
        const ShipHull* hull = ::GetShipHull(design->Hull());
        if (hull && hull->Tags().count(name))
            return true;

        // check parts for tag
        for (const std::string& part_name : design->Parts()) {
            const ShipPart* part = GetShipPart(part_name);
            if (part && part->Tags().count(name))
                return true;
        }
    }
    // check species for tag
    const Species* species = GetSpecies(SpeciesName());
    if (species && species->Tags().count(name))
        return true;

    return false;
}

UniverseObjectType Ship::ObjectType() const
{ return UniverseObjectType::OBJ_SHIP; }

bool Ship::ContainedBy(int object_id) const {
    return object_id != INVALID_OBJECT_ID
        && (    object_id == m_fleet_id
            ||  object_id == this->SystemID());
}

std::string Ship::Dump(unsigned short ntabs) const {
    std::stringstream os;
    os << UniverseObject::Dump(ntabs);
    os << " design id: " << m_design_id
       << " fleet id: " << m_fleet_id
       << " species name: " << m_species_name
       << " produced by empire id: " << m_produced_by_empire_id
       << " arrived on turn: " << m_arrived_on_turn
       << " last resupplied on turn: " << m_last_resupplied_on_turn;
    if (!m_part_meters.empty()) {
        os << " part meters: ";
        for (const auto& entry : m_part_meters) {
            const std::string part_name = entry.first.second;
            MeterType meter_type = entry.first.first;
            const Meter& meter = entry.second;
            os << part_name << " " << meter_type << ": " << meter.Current() << "  ";
        }
    }
    return os.str();
}

const ShipDesign* Ship::Design() const
{ return GetShipDesign(m_design_id); }

bool Ship::IsMonster() const {
    const ShipDesign* design = Design();
    if (design)
        return design->IsMonster();
    else
        return false;
}

bool Ship::IsArmed() const {
    if (TotalWeaponsDamage(0.0f, false) > 0.0f)
        return true;    // has non-fighter weapons
    if (HasFighters() && (TotalWeaponsDamage(0.0f, true) > 0.0f))
        return true;    // has no non-fighter weapons but has launchable fighters that do damage
    return false;
}

bool Ship::HasFighters() const {
    const ShipDesign* design = Design();
    if (!design || !design->HasFighters())  // ensures ship has ability to launch fighters
        return false;
    return FighterCount() >= 1.0f;          // ensures ship currently has fighters to launch
}

bool Ship::CanColonize() const {
    if (m_species_name.empty())
        return false;
    const Species* species = GetSpecies(m_species_name);
    if (!species)
        return false;
    if (!species->CanColonize())
        return false;
    const ShipDesign* design = this->Design();
    return design && design->CanColonize(); // use design->CanColonize because zero-capacity colony ships still count as outpost ships, can "can colonize" as far as order / the UI are concerned
}

bool Ship::HasTroops() const
{ return this->TroopCapacity() > 0.0f; }

bool Ship::CanBombard() const {
    const ShipDesign* design = Design();
    return design && design->CanBombard();
}

float Ship::Speed() const
{ return GetMeter(MeterType::METER_SPEED)->Initial(); }

float Ship::ColonyCapacity() const {
    float retval = 0.0f;
    // find which colony parts are present in design (one copy of name for each instance of a part, allowing duplicate names to appear)
    const ShipDesign* design = Design();
    if (!design)
        return retval;

    for (const std::string& part_name : design->Parts()) {
        if (part_name.empty())
            continue;
        const ShipPart* part = GetShipPart(part_name);
        if (!part)
            continue;
        ShipPartClass part_class = part->Class();
        if (part_class != ShipPartClass::PC_COLONY)
            continue;
        // add capacity for all instances of colony parts to accumulator
        retval += this->InitialPartMeterValue(MeterType::METER_CAPACITY, part_name);
    }

    return retval;
}

float Ship::TroopCapacity() const {
    float retval = 0.0f;
    // find which troop parts are present in design (one copy of name for each instance of a part, allowing duplicate names to appear)
    const ShipDesign* design = Design();
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

bool Ship::CanHaveTroops() const {
    const ShipDesign* design = Design();
    return design ? design->HasTroops() : false;
}

const std::string& Ship::PublicName(int empire_id) const {
    // Disclose real ship name only to fleet owners. Rationale: a player who
    // doesn't know the design for a particular ship can easily guess it if the
    // ship's name is "Scout"
    // An exception is made for unowned monsters.
    if (empire_id == ALL_EMPIRES || OwnedBy(empire_id) || (IsMonster() && Owner() == ALL_EMPIRES))  // TODO: GameRule for all objects visible
        return Name();
    const ShipDesign* design = Design();
    if (design)
        return design->Name();
    else if (IsMonster())
        return UserString("SM_MONSTER");
    else if (!Unowned())
        return UserString("FW_FOREIGN_SHIP");
    else if (Unowned() && GetVisibility(empire_id) > Visibility::VIS_NO_VISIBILITY)
        return UserString("FW_ROGUE_SHIP");
    else
        return UserString("OBJ_SHIP");
}

std::shared_ptr<UniverseObject> Ship::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(std::const_pointer_cast<Ship>(std::static_pointer_cast<const Ship>(shared_from_this()))); }

const Meter* Ship::GetPartMeter(MeterType type, const std::string& part_name) const
{ return const_cast<Ship*>(this)->GetPartMeter(type, part_name); }

Meter* Ship::GetPartMeter(MeterType type, const std::string& part_name) {
    Meter* retval = nullptr;
    auto it = m_part_meters.find({type, part_name});
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

float Ship::SumCurrentPartMeterValuesForPartClass(MeterType type, ShipPartClass part_class) const {
    float retval = 0.0f;

    const ShipDesign* design = GetShipDesign(m_design_id);
    if (!design)
        return retval;

    const auto& parts = design->Parts();
    if (parts.empty())
        return retval;

    std::map<std::string, int> part_counts;
    for (const std::string& part : parts)
        part_counts[part]++;

    for (const auto& part_meter : m_part_meters) {
        if (part_meter.first.first != type)
            continue;
        const std::string& part_name = part_meter.first.second;
        if (part_counts[part_name] < 1)
            continue;
        const ShipPart* part = GetShipPart(part_name);
        if (!part)
            continue;
        if (part_class == part->Class())
            retval += part_meter.second.Current() * part_counts[part_name];
    }

    return retval;
}

float Ship::FighterCount() const {
    float retval = 0.0f;
    for (const auto& entry : m_part_meters) {
        if (entry.first.first != MeterType::METER_CAPACITY)
            continue;
        const ShipPart* part = GetShipPart(entry.first.second);
        if (!part || part->Class() != ShipPartClass::PC_FIGHTER_HANGAR)
            continue;
        retval += entry.second.Current();
    }

    return retval;
}

float Ship::FighterMax() const {
    float retval = 0.0f;
    for (const auto& entry : m_part_meters) {
        //std::map<std::pair<MeterType, std::string>, Meter>
        if (entry.first.first != MeterType::METER_MAX_CAPACITY)
            continue;
        const ShipPart* part = GetShipPart(entry.first.second);
        if (!part || part->Class() != ShipPartClass::PC_FIGHTER_HANGAR)
            continue;
        retval += entry.second.Current();
    }

    return retval;
}

float Ship::TotalWeaponsDamage(float shield_DR, bool include_fighters) const {
    // sum up all individual weapons' attack strengths
    float total_attack = 0.0f;
    auto all_weapons_damage = AllWeaponsDamage(shield_DR, include_fighters);
    for (float attack : all_weapons_damage)
        total_attack += attack;
    return total_attack;
}

namespace {
    std::vector<float> WeaponDamageImpl(const Ship* ship, const ShipDesign* design,
                                        float DR, bool max, bool include_fighters)
    {
        std::vector<float> retval;

        if (!ship || !design)
            return retval;
        const std::vector<std::string>& parts = design->Parts();
        if (parts.empty())
            return retval;

        MeterType METER = max ? MeterType::METER_MAX_CAPACITY : MeterType::METER_CAPACITY;
        MeterType SECONDARY_METER = max ? MeterType::METER_MAX_SECONDARY_STAT : MeterType::METER_SECONDARY_STAT;

        float fighter_damage = 0.0f;
        int fighter_launch_capacity = 0;
        int available_fighters = 0;

        retval.reserve(parts.size() + 1);
        // for each weapon part, get its damage meter value
        for (const auto& part_name : parts) {
            const ShipPart* part = GetShipPart(part_name);
            if (!part)
                continue;
            ShipPartClass part_class = part->Class();

            // get the attack power for each weapon part.
            if (part_class == ShipPartClass::PC_DIRECT_WEAPON) {
                float part_attack = ship->CurrentPartMeterValue(METER, part_name);  // used within loop that updates meters, so need current, not initial values
                float part_shots = ship->CurrentPartMeterValue(SECONDARY_METER, part_name);
                if (part_attack > DR)
                    retval.emplace_back((part_attack - DR)*part_shots);

            } else if (part_class == ShipPartClass::PC_FIGHTER_BAY && include_fighters) {
                // launch capacity determined by capacity of bay
                fighter_launch_capacity += static_cast<int>(ship->CurrentPartMeterValue(METER, part_name));

            } else if (part_class == ShipPartClass::PC_FIGHTER_HANGAR && include_fighters) {
                // attack strength of a ship's fighters determined by the hangar...
                fighter_damage = ship->CurrentPartMeterValue(SECONDARY_METER, part_name);  // assuming all hangars have the same damage...
                available_fighters = std::max(0, static_cast<int>(ship->CurrentPartMeterValue(METER, part_name)));  // stacked meter
            }
        }

        if (!include_fighters || fighter_damage <= 0.0f || available_fighters <= 0 || fighter_launch_capacity <= 0)
            return retval;

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

        retval.emplace_back(fighter_damage * fighter_shots / num_bouts); // divide by bouts because fighter calculation is for a full combat, but direct firefor one attack

        return retval;
    }
}

std::vector<float> Ship::AllWeaponsDamage(float shield_DR, bool include_fighters) const {
    std::vector<float> retval;

    const ShipDesign* design = GetShipDesign(m_design_id);
    if (!design)
        return retval;

    return WeaponDamageImpl(this, design, shield_DR, false, include_fighters);
}

std::vector<float> Ship::AllWeaponsMaxDamage(float shield_DR , bool include_fighters) const {
    std::vector<float> retval;

    const ShipDesign* design = GetShipDesign(m_design_id);
    if (!design)
        return retval;

    return WeaponDamageImpl(this, design, shield_DR, true, include_fighters);
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

void Ship::BackPropagateMeters() {
    UniverseObject::BackPropagateMeters();

    // ship part meter back propagation, since base class function doesn't do this...
    for (auto& entry : m_part_meters)
        entry.second.BackPropagate();
}

void Ship::Resupply() {
    m_last_resupplied_on_turn = CurrentTurn();

    Meter* fuel_meter = UniverseObject::GetMeter(MeterType::METER_FUEL);
    const Meter* max_fuel_meter = UniverseObject::GetMeter(MeterType::METER_MAX_FUEL);
    if (!fuel_meter || !max_fuel_meter) {
        ErrorLogger() << "Ship::Resupply couldn't get fuel meters!";
    } else {
        fuel_meter->SetCurrent(max_fuel_meter->Current());
        fuel_meter->BackPropagate();
    }

    // set all part capacities equal to any associated max capacity
    // this "upgrades" any direct-fire weapon parts to their latest-allowed
    // strengths, and replaces any lost fighters
    for (auto& entry : m_part_meters) {
        const auto& part_name = entry.first.second;
        MeterType meter_type = entry.first.first;
        MeterType paired_meter_type = MeterType::INVALID_METER_TYPE;
        switch(meter_type) {
        case MeterType::METER_CAPACITY:       paired_meter_type = MeterType::METER_MAX_CAPACITY;         break;
        case MeterType::METER_SECONDARY_STAT: paired_meter_type = MeterType::METER_MAX_SECONDARY_STAT;   break;
        default:
            break;
        }
        if (paired_meter_type == MeterType::INVALID_METER_TYPE)
            continue;
        auto max_it = m_part_meters.find({paired_meter_type, part_name});
        if (max_it == m_part_meters.end())
            continue;

        const Meter& max_meter = max_it->second;

        entry.second.SetCurrent(max_meter.Current());
        entry.second.BackPropagate();
    }
}

void Ship::SetSpecies(std::string species_name) {
    if (!GetSpecies(species_name))
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

    // max meters are always treated as target/max meters.
    // other meters may be unpaired if there is no associated max or target meter
    for (auto& entry : m_part_meters) {
        const auto& part_name = entry.first.second;
        MeterType meter_type = entry.first.first;
        MeterType paired_meter_type = MeterType::INVALID_METER_TYPE;

        switch(meter_type) {
        case MeterType::METER_MAX_CAPACITY:
        case MeterType::METER_MAX_SECONDARY_STAT:
            entry.second.ResetCurrent();
            continue;
            break;
        case MeterType::METER_CAPACITY:        paired_meter_type = MeterType::METER_MAX_CAPACITY;         break;
        case MeterType::METER_SECONDARY_STAT:  paired_meter_type = MeterType::METER_MAX_SECONDARY_STAT;   break;
        default:
            continue;
            break;
        }

        auto max_it = m_part_meters.find({paired_meter_type, part_name});
        if (max_it != m_part_meters.end())
            continue;   // is a max/target meter associated with the meter, so don't treat this a target/max

        // no associated target/max meter, so treat this meter as unpaired
        entry.second.ResetCurrent();
    }
}

void Ship::ResetPairedActiveMeters() {
    UniverseObject::ResetPairedActiveMeters();

    // meters are paired only if they are not max/target meters, and there is an
    // associated max/target meter
    for (auto& entry : m_part_meters) {
        const auto& part_name = entry.first.second;
        MeterType meter_type = entry.first.first;
        MeterType paired_meter_type = MeterType::INVALID_METER_TYPE;

        switch(meter_type) {
        case MeterType::METER_MAX_CAPACITY:
        case MeterType::METER_MAX_SECONDARY_STAT:
            continue;   // is a max/target meter
            break;
        case MeterType::METER_CAPACITY:       paired_meter_type = MeterType::METER_MAX_CAPACITY;         break;
        case MeterType::METER_SECONDARY_STAT: paired_meter_type = MeterType::METER_MAX_SECONDARY_STAT;   break;
        default:
            continue;   // no associated max/target meter
            break;
        }

        auto max_it = m_part_meters.find({paired_meter_type, part_name});
        if (max_it == m_part_meters.end())
            continue;   // no associated max/target meter

        // has an associated max/target meter.
        //std::map<std::pair<MeterType, std::string>, Meter>::iterator
        entry.second.SetCurrent(entry.second.Initial());
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
    for (auto& entry : m_part_meters) {
        MeterType meter_type = entry.first.first;
        switch(meter_type) {
        case MeterType::METER_MAX_CAPACITY:
        case MeterType::METER_MAX_SECONDARY_STAT:
            entry.second.ClampCurrentToRange();
        default:
            break;
        }
    }

    // special case extra clamping for paired active meters dependent
    // on their associated max meter...
    for (auto& entry : m_part_meters) {
        const auto& part_name = entry.first.second;
        MeterType meter_type = entry.first.first;
        MeterType paired_meter_type = MeterType::INVALID_METER_TYPE;
        switch(meter_type) {
        case MeterType::METER_CAPACITY:        paired_meter_type = MeterType::METER_MAX_CAPACITY;         break;
        case MeterType::METER_SECONDARY_STAT:  paired_meter_type = MeterType::METER_MAX_SECONDARY_STAT;   break;
        default:
            break;
        }
        if (paired_meter_type == MeterType::INVALID_METER_TYPE)
            continue;
        auto max_it = m_part_meters.find({paired_meter_type, part_name});
        if (max_it == m_part_meters.end())
            continue;

        const Meter& max_meter = max_it->second;
        entry.second.ClampCurrentToRange(Meter::DEFAULT_VALUE, max_meter.Current());
    }
}

////////////////////
// Free Functions //
////////////////////
std::string NewMonsterName() {
    static std::vector<std::string> monster_names = UserStringList("MONSTER_NAMES");
    static std::map<std::string, int> monster_names_used;

    if (monster_names.empty())
        monster_names.emplace_back(UserString("MONSTER"));

    // select name randomly from list
    int monster_name_index = RandInt(0, static_cast<int>(monster_names.size()) - 1);
    std::string result = monster_names[monster_name_index];
    if (monster_names_used[result]++)
        result += " " + RomanNumber(monster_names_used[result]);
    return result;
}
