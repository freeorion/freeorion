#include "Ship.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/AppInterface.h"
#include "Fleet.h"
#include "Predicates.h"
#include "ShipDesign.h"
#include "Species.h"
#include "Universe.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include <boost/lexical_cast.hpp>

class Species;
const Species* GetSpecies(const std::string& name);

Ship::Ship() :
    UniverseObject(),
    m_design_id(ShipDesign::INVALID_DESIGN_ID),
    m_fleet_id(INVALID_OBJECT_ID),
    m_ordered_scrapped(false),
    m_ordered_colonize_planet_id(INVALID_OBJECT_ID),
    m_ordered_invade_planet_id(INVALID_OBJECT_ID),
    m_ordered_bombard_planet_id(INVALID_OBJECT_ID),
    m_last_turn_active_in_combat(INVALID_GAME_TURN),
    m_produced_by_empire_id(ALL_EMPIRES),
    m_arrived_on_turn(INVALID_GAME_TURN)
{}

Ship::Ship(int empire_id, int design_id, const std::string& species_name,
           int produced_by_empire_id/* = ALL_EMPIRES*/) :
    UniverseObject(),
    m_design_id(design_id),
    m_fleet_id(INVALID_OBJECT_ID),
    m_ordered_scrapped(false),
    m_ordered_colonize_planet_id(INVALID_OBJECT_ID),
    m_ordered_invade_planet_id(INVALID_OBJECT_ID),
    m_ordered_bombard_planet_id(INVALID_OBJECT_ID),
    m_last_turn_active_in_combat(INVALID_GAME_TURN),
    m_species_name(species_name),
    m_produced_by_empire_id(produced_by_empire_id),
    m_arrived_on_turn(CurrentTurn())
{
    if (!GetShipDesign(design_id))
        throw std::invalid_argument("Attempted to construct a Ship with an invalid design id");

    if (!m_species_name.empty() && !GetSpecies(m_species_name))
        DebugLogger() << "Ship created with invalid species name: " << m_species_name;

    SetOwner(empire_id);

    UniverseObject::Init();

    AddMeter(METER_FUEL);
    AddMeter(METER_MAX_FUEL);
    AddMeter(METER_SHIELD);
    AddMeter(METER_MAX_SHIELD);
    AddMeter(METER_DETECTION);
    AddMeter(METER_STRUCTURE);
    AddMeter(METER_MAX_STRUCTURE);
    AddMeter(METER_SPEED);
    AddMeter(METER_TARGET_INDUSTRY);
    AddMeter(METER_INDUSTRY);
    AddMeter(METER_TARGET_RESEARCH);
    AddMeter(METER_RESEARCH);
    AddMeter(METER_TARGET_TRADE);
    AddMeter(METER_TRADE);

    const std::vector<std::string>& part_names = Design()->Parts();
    for (std::size_t i = 0; i < part_names.size(); ++i) {
        if (part_names[i] != "") {
            const PartType* part = GetPartType(part_names[i]);
            if (!part) {
                ErrorLogger() << "Ship::Ship couldn't get part with name " << part_names[i];
                continue;
            }

            switch (part->Class()) {
            case PC_COLONY:
            case PC_TROOPS: {
                m_part_meters[std::make_pair(METER_CAPACITY, part->Name())];
                break;
            }
            case PC_DIRECT_WEAPON:      // capacity is damage, secondary stat is shots per attack
            case PC_FIGHTER_HANGAR: {   // capacity is how many fighters contained, secondary stat is damage per fighter attack
                m_part_meters[std::make_pair(METER_SECONDARY_STAT,      part->Name())];
                m_part_meters[std::make_pair(METER_MAX_SECONDARY_STAT,  part->Name())];
                // intentionally no break here
            }
            case PC_FIGHTER_BAY: {      // capacity is how many fighters launched per combat round
                m_part_meters[std::make_pair(METER_CAPACITY,            part->Name())];
                m_part_meters[std::make_pair(METER_MAX_CAPACITY,        part->Name())];
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

    if (!(vis >= VIS_BASIC_VISIBILITY && vis <= VIS_FULL_VISIBILITY))
        return 0;

    Ship* retval = new Ship();
    retval->Copy(TemporaryFromThis(), empire_id);
    return retval;
}

void Ship::Copy(TemporaryPtr<const UniverseObject> copied_object, int empire_id) {
    if (copied_object == this)
        return;
    TemporaryPtr<const Ship> copied_ship = boost::dynamic_pointer_cast<const Ship>(copied_object);
    if (!copied_ship) {
        ErrorLogger() << "Ship::Copy passed an object that wasn't a Ship";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    std::set<std::string> visible_specials = GetUniverse().GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis, visible_specials);;

    if (vis >= VIS_BASIC_VISIBILITY) {
        if (this->m_fleet_id != copied_ship->m_fleet_id) {
            // as with other containers, removal from the old container is triggered by the contained Object; removal from System is handled by UniverseObject::Copy
            if (TemporaryPtr<Fleet> oldFleet = GetFleet(this->m_fleet_id)) 
                oldFleet->RemoveShip(this->ID());
            this->m_fleet_id =              copied_ship->m_fleet_id; // as with other containers (Systems), actual insertion into fleet ships set is handled by the fleet
        }

        if (vis >= VIS_PARTIAL_VISIBILITY) {
            if (this->Unowned())
                this->m_name =                  copied_ship->m_name;

            this->m_design_id =                 copied_ship->m_design_id;
            for (PartMeterMap::const_iterator it = copied_ship->m_part_meters.begin();
                 it != copied_ship->m_part_meters.end(); ++it)
            { this->m_part_meters = copied_ship->m_part_meters; }
            this->m_species_name =              copied_ship->m_species_name;

            this->m_last_turn_active_in_combat= copied_ship->m_last_turn_active_in_combat;
            this->m_produced_by_empire_id =     copied_ship->m_produced_by_empire_id;
            this->m_arrived_on_turn =           copied_ship->m_arrived_on_turn;

            if (vis >= VIS_FULL_VISIBILITY) {
                this->m_ordered_scrapped =          copied_ship->m_ordered_scrapped;
                this->m_ordered_colonize_planet_id= copied_ship->m_ordered_colonize_planet_id;
                this->m_ordered_invade_planet_id  = copied_ship->m_ordered_invade_planet_id;
                this->m_ordered_bombard_planet_id = copied_ship->m_ordered_bombard_planet_id;
            }
        }
    }
}

std::set<std::string> Ship::Tags() const {
    std::set<std::string> retval;

    const ShipDesign* design = GetShipDesign(m_design_id);
    if (!design)
        return retval;

    const HullType* hull = ::GetHullType(design->Hull());
    if (!hull)
        return retval;
    retval.insert(hull->Tags().begin(), hull->Tags().end());

    const std::vector<std::string>& parts = design->Parts();
    if (parts.empty())
        return retval;

    for (std::vector<std::string>::const_iterator part_it = parts.begin(); part_it != parts.end(); ++part_it) {
        if (const PartType* part = GetPartType(*part_it)) {
            retval.insert(part->Tags().begin(), part->Tags().end());
        }
    }

    return retval;
}

bool Ship::HasTag(const std::string& name) const {
    const ShipDesign* design = GetShipDesign(m_design_id);
    if (design) {
        // check hull for tag
        const HullType* hull = ::GetHullType(design->Hull());
        if (hull && hull->Tags().count(name))
            return true;

        // check parts for tag
        const std::vector<std::string>& parts = design->Parts();
        for (std::vector<std::string>::const_iterator part_it = parts.begin(); part_it != parts.end(); ++part_it) {
            const PartType* part = GetPartType(*part_it);
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
{ return OBJ_SHIP; }

bool Ship::ContainedBy(int object_id) const {
    return object_id != INVALID_OBJECT_ID
        && (    object_id == m_fleet_id
            ||  object_id == this->SystemID());
}

std::string Ship::Dump() const {
    std::stringstream os;
    os << UniverseObject::Dump();
    os << " design id: " << m_design_id
       << " fleet id: " << m_fleet_id
       << " species name: " << m_species_name
       << " produced by empire id: " << m_produced_by_empire_id
       << " arrived on turn: " << m_arrived_on_turn;
    if (!m_part_meters.empty()) {
        os << " part meters: ";
        for (PartMeterMap::const_iterator it = m_part_meters.begin(); it != m_part_meters.end();) {
        const std::string part_name = it->first.second;
        MeterType meter_type = it->first.first;
        const Meter& meter = it->second;
        ++it;
        os << UserString(part_name) << " "
           << UserString(EnumToString(meter_type))
           << ": " << meter.Current() << "  ";
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
    const ShipDesign* design = Design();
    if (design)
        return design->IsArmed();
    else
        return false;
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
{ return CurrentMeterValue(METER_SPEED); }

float Ship::ColonyCapacity() const {
    float retval = 0.0f;
    // find which colony parts are present in design (one copy of name for each instance of a part, allowing duplicate names to appear)
    const ShipDesign* design = Design();
    if (!design)
        return retval;

    const std::vector<std::string>& parts = design->Parts();
    for (std::vector<std::string>::const_iterator part_it = parts.begin();
         part_it != parts.end(); ++part_it)
    {
        const std::string& part_name = *part_it;
        if (part_name.empty())
            continue;
        const PartType* part_type = GetPartType(part_name);
        if (!part_type)
            continue;
        ShipPartClass part_class = part_type->Class();
        if (part_class != PC_COLONY)
            continue;
        // add capacity for all instances of colony parts to accumulator
        retval += this->CurrentPartMeterValue(METER_CAPACITY, part_name);
    }

    return retval;
}

float Ship::TroopCapacity() const {
    float retval = 0.0f;
    // find which troop parts are present in design (one copy of name for each instance of a part, allowing duplicate names to appear)
    const ShipDesign* design = Design();
    if (!design)
        return retval;

    const std::vector<std::string>& parts = design->Parts();
    for (std::vector<std::string>::const_iterator part_it = parts.begin();
         part_it != parts.end(); ++part_it)
    {
        const std::string& part_name = *part_it;
        if (part_name.empty())
            continue;
        const PartType* part_type = GetPartType(part_name);
        if (!part_type)
            continue;
        ShipPartClass part_class = part_type->Class();
        if (part_class != PC_TROOPS)
            continue;
        // add capacity for all instances of colony parts to accumulator
        retval += this->CurrentPartMeterValue(METER_CAPACITY, part_name);
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
    if (GetUniverse().AllObjectsVisible() || empire_id == ALL_EMPIRES || OwnedBy(empire_id) || (IsMonster() && Owner() == ALL_EMPIRES))
        return Name();
    const ShipDesign* design = Design();
    if (design)
        return design->Name();
    else if (IsMonster())
        return UserString("SM_MONSTER");
    else if (!Unowned())
        return UserString("FW_FOREIGN_SHIP");
    else if (Unowned() && GetVisibility(empire_id) > VIS_NO_VISIBILITY)
        return UserString("FW_ROGUE_SHIP");
    else
        return UserString("OBJ_SHIP");
}

TemporaryPtr<UniverseObject> Ship::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(boost::const_pointer_cast<Ship>(boost::static_pointer_cast<const Ship>(TemporaryFromThis()))); }

float Ship::NextTurnCurrentMeterValue(MeterType type) const {
    const Meter* meter = UniverseObject::GetMeter(type);
    if (!meter)
        throw std::invalid_argument("Ship::NextTurnCurrentMeterValue passed meter type that the Ship does not have: " + boost::lexical_cast<std::string>(type));
    float current_meter_value = meter->Current();

    //if (type == METER_FUEL) {
    //    // todo: consider fleet movement or being stationary, which may partly replenish fuel
    //    // todo: consider fleet passing through or being in a supplied system, which replenishes fuel
    //}

    if (type == METER_SHIELD) {
        if (m_last_turn_active_in_combat >= CurrentTurn())
            return std::max(0.0f,   // battle just happened. shields limited to max shield, but don't regen
                            std::min(current_meter_value,
                                     UniverseObject::GetMeter(METER_MAX_SHIELD)->Current()));
        else                        // shields regneerate to max shield
            return UniverseObject::GetMeter(METER_MAX_SHIELD)->Current();
    }


    // ResourceCenter-like resource meter growth...

    MeterType target_meter_type = INVALID_METER_TYPE;
    switch (type) {
    case METER_TARGET_INDUSTRY:
    case METER_TARGET_RESEARCH:
    case METER_TARGET_TRADE:
        return current_meter_value;
        break;
    case METER_INDUSTRY:    target_meter_type = METER_TARGET_INDUSTRY;      break;
    case METER_RESEARCH:    target_meter_type = METER_TARGET_RESEARCH;      break;
    case METER_TRADE:       target_meter_type = METER_TARGET_TRADE;         break;
    default:
        return UniverseObject::NextTurnCurrentMeterValue(type);
    }

    const Meter* target_meter = UniverseObject::GetMeter(target_meter_type);
    if (!target_meter)
        throw std::runtime_error("Ship::NextTurnCurrentMeterValue dealing with invalid meter type: " + boost::lexical_cast<std::string>(type));
    float target_meter_value = target_meter->Current();

    // meter growth or decay towards target is one per turn.
    if (target_meter_value > current_meter_value)
        return std::min(current_meter_value + 1.0f, target_meter_value);
    else if (target_meter_value < current_meter_value)
        return std::max(target_meter_value, current_meter_value - 1.0f);
    else
        return current_meter_value;
}

const Meter* Ship::GetPartMeter(MeterType type, const std::string& part_name) const
{ return const_cast<Ship*>(this)->GetPartMeter(type, part_name); }

Meter* Ship::GetPartMeter(MeterType type, const std::string& part_name) {
    Meter* retval = 0;
    PartMeterMap::iterator it = m_part_meters.find(std::make_pair(type, part_name));
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

float Ship::TotalWeaponsDamage(float shield_DR, bool include_fighters) const {
    // sum up all individual weapons' attack strengths
    float total_attack = 0.0f;
    std::vector<float> all_weapons_damage = AllWeaponsDamage(shield_DR, include_fighters);
    for (std::vector<float>::iterator it = all_weapons_damage.begin(); it != all_weapons_damage.end(); ++it)
        total_attack += *it;
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

        MeterType METER = max ? METER_MAX_CAPACITY : METER_CAPACITY;
        MeterType SECONDARY_METER = max ? METER_MAX_SECONDARY_STAT : METER_SECONDARY_STAT;

        float fighter_damage = 0.0f;
        int fighter_launch_capacity = 0;
        int available_fighters = 0;

        // for each weapon part, get its damage meter value
        for (std::vector<std::string>::const_iterator part_it = parts.begin();
             part_it != parts.end(); ++part_it)
        {
            const std::string& part_name = *part_it;
            const PartType* part = GetPartType(part_name);
            if (!part)
                continue;
            ShipPartClass part_class = part->Class();

            // get the attack power for each weapon part
            if (part_class == PC_DIRECT_WEAPON) {
                float part_attack = ship->CurrentPartMeterValue(METER, part_name);
                float part_shots = ship->CurrentPartMeterValue(SECONDARY_METER, part_name);
                if (part_attack > DR)
                    retval.push_back((part_attack - DR)*part_shots);

            } else if (part_class == PC_FIGHTER_BAY && include_fighters) {
                // launch capacity determined by capacity of bay
                fighter_launch_capacity += static_cast<int>(ship->CurrentPartMeterValue(METER, part_name));

            } else if (part_class == PC_FIGHTER_HANGAR && include_fighters) {
                // attack strength of a ship's fighters determined by the hangar...
                fighter_damage = ship->CurrentPartMeterValue(SECONDARY_METER, part_name) - DR;  // assuming all hangars have the same damage...
                available_fighters += std::max(0, static_cast<int>(ship->CurrentPartMeterValue(METER, part_name)));
            }
        }

        if (!include_fighters || fighter_damage <= 0.0f || available_fighters <= 0 || fighter_launch_capacity <= 0)
            return retval;

        // assuming 3 combat "bouts"
        int fighter_shots = std::min(available_fighters, fighter_launch_capacity);  // how many fighters launched in bout 1
        available_fighters -= fighter_shots;
        fighter_shots *= 2;                                                         // first-bout-launched fighters attack in bouts 2 and 3
        fighter_shots += std::min(available_fighters, fighter_launch_capacity);     // second-bout-launched fighters attack only in bout 3

        // how much damage does a fighter shot do?
        fighter_damage = std::max(0.0f, fighter_damage);

        retval.push_back(fighter_damage * fighter_shots / 3.0f);    // divide by 3 because fighter calculation is for a full combat, but direct firefor one attack

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
    for (PartMeterMap::iterator it = m_part_meters.begin(); it != m_part_meters.end(); ++it)
        it->second.BackPropagate();
}

void Ship::Resupply() {
    Meter* fuel_meter = UniverseObject::GetMeter(METER_FUEL);
    const Meter* max_fuel_meter = UniverseObject::GetMeter(METER_MAX_FUEL);
    if (!fuel_meter || !max_fuel_meter) {
        ErrorLogger() << "Ship::Resupply couldn't get fuel meters!";
    } else {
        fuel_meter->SetCurrent(max_fuel_meter->Current());
        fuel_meter->BackPropagate();
    }

    // set all part capacities equal to any associated max capacity
    // this "upgrades" any direct-fire weapon parts to their latest-allowed
    // strengths, and replaces any lost fighters
    for (PartMeterMap::iterator it = m_part_meters.begin(); it != m_part_meters.end(); ++it) {
        PartMeterMap::iterator max_it = m_part_meters.end();
        if (it->first.first == METER_CAPACITY) {
            max_it = m_part_meters.find(std::make_pair(METER_MAX_CAPACITY, it->first.second));
        } else if (it->first.first == METER_SECONDARY_STAT) {
            max_it = m_part_meters.find(std::make_pair(METER_MAX_SECONDARY_STAT, it->first.second));
        }
        if (max_it == m_part_meters.end())
            continue;

        const Meter& max_meter = max_it->second;

        it->second.SetCurrent(max_meter.Current());
        it->second.BackPropagate();
    }
}

void Ship::SetSpecies(const std::string& species_name) {
    if (!GetSpecies(species_name))
        ErrorLogger() << "Ship::SetSpecies couldn't get species with name " << species_name;
    m_species_name = species_name;
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

    UniverseObject::GetMeter(METER_MAX_FUEL)->ResetCurrent();
    UniverseObject::GetMeter(METER_MAX_SHIELD)->ResetCurrent();
    UniverseObject::GetMeter(METER_MAX_STRUCTURE)->ResetCurrent();
    UniverseObject::GetMeter(METER_TARGET_INDUSTRY)->ResetCurrent();
    UniverseObject::GetMeter(METER_TARGET_RESEARCH)->ResetCurrent();
    UniverseObject::GetMeter(METER_TARGET_TRADE)->ResetCurrent();

    UniverseObject::GetMeter(METER_DETECTION)->ResetCurrent();
    UniverseObject::GetMeter(METER_SPEED)->ResetCurrent();
    //UniverseObject::GetMeter(METER_STEALTH)->ResetCurrent(); redundant with base class function

    for (PartMeterMap::iterator it = m_part_meters.begin(); it != m_part_meters.end(); ++it) {
        PartMeterMap::iterator max_it = m_part_meters.end();
        if (it->first.first == METER_CAPACITY) {
            max_it = m_part_meters.find(std::make_pair(METER_MAX_CAPACITY, it->first.second));
        } else if (it->first.first == METER_SECONDARY_STAT) {
            max_it = m_part_meters.find(std::make_pair(METER_MAX_SECONDARY_STAT, it->first.second));
        }
        if (max_it != m_part_meters.end())
            continue;
        it->second.ResetCurrent();
    }
}

void Ship::ResetPairedActiveMeters() {
    UniverseObject::ResetPairedActiveMeters();

    for (PartMeterMap::iterator it = m_part_meters.begin(); it != m_part_meters.end(); ++it) {
        PartMeterMap::iterator max_it = m_part_meters.end();
        if (it->first.first == METER_CAPACITY) {
            max_it = m_part_meters.find(std::make_pair(METER_MAX_CAPACITY, it->first.second));
        } else if (it->first.first == METER_SECONDARY_STAT) {
            max_it = m_part_meters.find(std::make_pair(METER_MAX_SECONDARY_STAT, it->first.second));
        }
        if (max_it != m_part_meters.end())
            it->second.SetCurrent(it->second.Initial());
    }
}

void Ship::SetShipMetersToMax() {
    UniverseObject::GetMeter(METER_MAX_FUEL)->SetCurrent(Meter::LARGE_VALUE);
    UniverseObject::GetMeter(METER_MAX_SHIELD)->SetCurrent(Meter::LARGE_VALUE);
    UniverseObject::GetMeter(METER_MAX_STRUCTURE)->SetCurrent(Meter::LARGE_VALUE);
    UniverseObject::GetMeter(METER_FUEL)->SetCurrent(Meter::LARGE_VALUE);
    UniverseObject::GetMeter(METER_SHIELD)->SetCurrent(Meter::LARGE_VALUE);
    UniverseObject::GetMeter(METER_STRUCTURE)->SetCurrent(Meter::LARGE_VALUE);

    // some part capacity meters may have an associated max capacity...
    for (PartMeterMap::iterator it = m_part_meters.begin(); it != m_part_meters.end(); ++it) {
        PartMeterMap::iterator max_it = m_part_meters.end();
        if (it->first.first == METER_CAPACITY) {
            max_it = m_part_meters.find(std::make_pair(METER_MAX_CAPACITY, it->first.second));
        } else if (it->first.first == METER_SECONDARY_STAT) {
            max_it = m_part_meters.find(std::make_pair(METER_MAX_SECONDARY_STAT, it->first.second));
        }
        if (max_it == m_part_meters.end())
            continue;
        max_it->second.SetCurrent(Meter::LARGE_VALUE);
        it->second.SetCurrent(Meter::LARGE_VALUE);
    }
}

void Ship::PopGrowthProductionResearchPhase() {
    UniverseObject::PopGrowthProductionResearchPhase();

    UniverseObject::GetMeter(METER_SHIELD)->SetCurrent(Ship::NextTurnCurrentMeterValue(METER_SHIELD));
    UniverseObject::GetMeter(METER_INDUSTRY)->SetCurrent(Ship::NextTurnCurrentMeterValue(METER_INDUSTRY));
    UniverseObject::GetMeter(METER_RESEARCH)->SetCurrent(Ship::NextTurnCurrentMeterValue(METER_RESEARCH));
    UniverseObject::GetMeter(METER_TRADE)->SetCurrent(Ship::NextTurnCurrentMeterValue(METER_TRADE));

    // part capacity meters set to max only by being in supply

    StateChangedSignal();
}

void Ship::ClampMeters() {
    UniverseObject::ClampMeters();

    UniverseObject::GetMeter(METER_MAX_FUEL)->ClampCurrentToRange();
    UniverseObject::GetMeter(METER_FUEL)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(METER_MAX_FUEL)->Current());
    UniverseObject::GetMeter(METER_MAX_SHIELD)->ClampCurrentToRange();
    UniverseObject::GetMeter(METER_SHIELD)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(METER_MAX_SHIELD)->Current());
    UniverseObject::GetMeter(METER_MAX_STRUCTURE)->ClampCurrentToRange();
    UniverseObject::GetMeter(METER_STRUCTURE)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(METER_MAX_STRUCTURE)->Current());
    UniverseObject::GetMeter(METER_TARGET_INDUSTRY)->ClampCurrentToRange();
    UniverseObject::GetMeter(METER_INDUSTRY)->ClampCurrentToRange();
    UniverseObject::GetMeter(METER_TARGET_RESEARCH)->ClampCurrentToRange();
    UniverseObject::GetMeter(METER_RESEARCH)->ClampCurrentToRange();
    UniverseObject::GetMeter(METER_TARGET_TRADE)->ClampCurrentToRange();
    UniverseObject::GetMeter(METER_TRADE)->ClampCurrentToRange();

    UniverseObject::GetMeter(METER_DETECTION)->ClampCurrentToRange();
    UniverseObject::GetMeter(METER_SPEED)->ClampCurrentToRange();

    // clamp most part meters to basic range limits
    for (PartMeterMap::iterator it = m_part_meters.begin(); it != m_part_meters.end(); ++it) {
        if (it->first.first == METER_CAPACITY || it->first.first == METER_SECONDARY_STAT)
            continue;
        it->second.ClampCurrentToRange();
    }

    // special case extra clamping for paired CAPACTY meters dependent on their associated MAX_CAPACITY meter...
    for (PartMeterMap::iterator it = m_part_meters.begin(); it != m_part_meters.end(); ++it) {
        PartMeterMap::iterator max_it = m_part_meters.end();
        if (it->first.first == METER_CAPACITY) {
            max_it = m_part_meters.find(std::make_pair(METER_MAX_CAPACITY, it->first.second));
        } else if (it->first.first == METER_SECONDARY_STAT) {
            max_it = m_part_meters.find(std::make_pair(METER_MAX_SECONDARY_STAT, it->first.second));
        }

        if (max_it == m_part_meters.end()) {
            // no found max meter, revert to normal clamping for this meter
            it->second.ClampCurrentToRange();
            continue;
        }

        const Meter& max_meter = max_it->second;
        it->second.ClampCurrentToRange(Meter::DEFAULT_VALUE, max_meter.Current());
    }
}

////////////////////
// Free Functions //
////////////////////
std::string NewMonsterName() {
    static std::vector<std::string> monster_names;
    static std::map<std::string, int> monster_names_used;
    if (monster_names.empty()) {
        // load monster names from stringtable
        std::list<std::string> monster_names_list;
        UserStringList("MONSTER_NAMES", monster_names_list);

        monster_names.reserve(monster_names_list.size());
        std::copy(monster_names_list.begin(), monster_names_list.end(), std::back_inserter(monster_names));
        if (monster_names.empty()) // safety check to ensure not leaving list empty in case of stringtable failure
            monster_names.push_back(UserString("MONSTER"));
    }

    // select name randomly from list
    int monster_name_index = RandSmallInt(0, static_cast<int>(monster_names.size()) - 1);
    std::string result = monster_names[monster_name_index];
    if (monster_names_used[result]++) {
        result += " " + RomanNumber(monster_names_used[result]);
    }
    return result;
}
